#include "src/func.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>

int main() {
    srand(125);
    aterm_b a_b[NDATA]; bterm_b b_b[NDATA]; cterm_b c_b[NDATA]; ret_b fret_b, fretr_b;
    aterm_f a_f[NDATA]; bterm_f b_f[NDATA]; cterm_f c_f[NDATA]; ret_f fret_f, fretr_f;
    aterm_s a_s[NDATA]; bterm_s b_s[NDATA]; cterm_s c_s[NDATA]; ret_s fret_s, fretr_s;
    const float a_max = std::pow(2, aterm_b::width - 1 - a_dec); 
    const float b_max = std::pow(2, bterm_b::width - 1 - b_dec);
    const float c_max = std::pow(2, cterm_b::width - 1 - c_dec);
    for (unsigned int itest = 0, ntest = 50; itest <= ntest; ++itest) {
        // create some input data
        int check_sum_b = 0;
        double check_sum_d = 0;
        for (unsigned int i = 0; i < NDATA; ++i) {

            // generate a, b, c so that once in a while we get an overflow in the sum
            float a_val = a_max * rand()/float(RAND_MAX) * ((itest*7+i) % 37 ? 0.001 : 1);
            float b_val = b_max * rand()/float(RAND_MAX+1.f);
            float c_val = c_max * rand()/float(RAND_MAX) / 2;
            assert(a_val >= 0 && b_val >= 0 && c_val >= 0);

            check_sum_d += (a_val * b_val ) + c_val;
        
            // integer math
            a_b[i] = aterm_b( std::floor( a_val * (1<<a_dec) ) );
            b_b[i] = bterm_b( std::floor( b_val * (1<<b_dec) ) );
            c_b[i] = cterm_b( std::floor( c_val * (1<<c_dec) ) );
            assert(a_b[i] >= 0 && b_b[i] >= 0 && c_b[i] >= 0);

            int term_b = (int64_t(a_b[i]) * int64_t(b_b[i])) >> (a_dec + b_dec - c_dec); // upcast needed to avoid temporary overflow
            assert(term_b >= 0);
            check_sum_b += term_b + int(c_b[i]);

            // fixed-prec (no sat)
            a_f[i] = aterm_f( a_val );
            b_f[i] = bterm_f( b_val );
            c_f[i] = cterm_f( c_val );
            assert(a_f[i] >= 0 && b_f[i] >= 0 && c_f[i] >= 0);

            // fixed-prec (no sat)
            a_s[i] = aterm_s( a_val );
            b_s[i] = bterm_s( b_val );
            c_s[i] = cterm_s( c_val );
            assert(a_s[i] >= 0 && b_s[i] >= 0 && c_s[i] >= 0);
        }

        //printf("test %3d: true result before truncation: %9.5f\n", itest, check_sum_d);
        assert(check_sum_b >= 0);
        check_sum_b = check_sum_b >> (c_dec - r_dec);

        const int sum_max_b = (1<<(ret_b::width-1)) - 1;
        bool overflow = check_sum_b > sum_max_b;
        check_sum_b = std::min<int>(check_sum_b, sum_max_b);
        // call the function
        fret_b = op_int(a_b,b_b,c_b);
        fretr_b = op_int_redux(a_b,b_b,c_b);
        if (fret_b != check_sum_b) {
            printf("int test %3d: sum = %9d  expected %9d  true %9.1f overflow? %d ---> ERROR \n", itest, int(fret_b), check_sum_b, check_sum_d * (1<<r_dec), int(overflow)); 
            return 1;
        } else if (fret_b != fretr_b) {
            printf("int test %3d: sum = %9d  expected %9d  true %9.1f overflow? %d ---> REDUX ERROR\n", itest, int(fret_b), int(fretr_b), check_sum_d * (1<<r_dec), int(overflow)); 
            return 2;
        } else {
            printf("int test %3d: sum = %9d  expected %9d  true %9.1f overflow? %d ---> ok \n", itest, int(fret_b), check_sum_b, check_sum_d * (1<<r_dec), int(overflow)); 
        }

        // call the function
        fret_f = op_fix(a_f,b_f,c_f);
        fretr_f = op_fix_redux(a_f,b_f,c_f);
        float fret_ff = fret_f.to_float(), fretr_ff = fretr_f.to_float(), fexp_ff =  float(fret_b)/(1<<r_dec);
        if (fret_ff != fexp_ff) {
            printf("fix test %3d: sum = %9.3f  expected %9.3f  true %9.3f overflow? %d ---> ERROR \n", itest, fret_ff, fexp_ff, check_sum_d, int(overflow)); 
            return 1;
        } else if (fret_f != fretr_f) {
            printf("fix test %3d: sum = %9.3f  expected %9.3f  true %9.3f overflow? %d ---> REDUX ERROR \n", itest, fret_ff, fretr_ff, check_sum_d, int(overflow)); 
            return 2;
        } else {
            printf("fix test %3d: sum = %9.3f  expected %9.3f  true %9.3f overflow? %d ---> ok \n", itest, fret_ff, fexp_ff, check_sum_d, int(overflow)); 
        }

        // call the function
        fret_s = op_sat(a_s,b_s,c_s);
        fretr_s = op_sat_redux(a_s,b_s,c_s);
        float fret_sf = fret_s.to_float(), fretr_sf = fretr_s.to_float();
        if (fret_sf != fexp_ff) {
            printf("sat test %3d: sum = %9.3f  expected %9.3f  true %9.3f overflow? %d ---> ERROR \n", itest, fret_sf, fexp_ff, check_sum_d, int(overflow)); 
            return 1;
        } else if (fret_f != fretr_s) {
            printf("fix test %3d: sum = %9.3f  expected %9.3f  true %9.3f overflow? %d ---> REDUX ERROR \n", itest, fret_sf, fretr_sf, check_sum_d, int(overflow)); 
            return 2;
        } else {
            printf("sat test %3d: sum = %9.3f  expected %9.3f  true %9.3f overflow? %d ---> ok \n", itest, fret_sf, fexp_ff, check_sum_d, int(overflow)); 
        }
 
    }
}
