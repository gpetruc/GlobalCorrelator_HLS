#include "src/func.h"
#include <cstdio>
#include <cstdlib>

int main() {
    srand(125);
    ap_int<16> a[NDATA]; bool b[NDATA]; ap_uint<2> b2[NDATA]; ap_int<24> c[NDATA], c2[NDATA], c_ref[NDATA], c2_ref[NDATA];
    const int otherFactor[4] = { 42, 137, 25, -18 };
    for (unsigned int itest = 0, ntest = 20; itest <= ntest; ++itest) {
        // create some input data
        for (unsigned int i = 0; i < NDATA; ++i) {
            a[i] = ap_int<16>(rand() & 0xFFFF);
            b[i] = i % 3;
            b2[i] = i % 4;
            c_ref[i] = a[i] * (b[i] ? otherFactor[0] : otherFactor[1]);
            c2_ref[i] = a[i] * otherFactor[int(b2[i])];
        } 
        // call the function
        myfunc(a,b,c);
        myfunc2(a,b2,c2);
        // check the results (here we check only the total, for lazyness)
        for (unsigned int i = 0; i < NDATA; ++i) {
            if (c[i] != c_ref[i]) {
                printf("ERROR in myfunc: a = %d, b = %d, c = %d, c_ref = %d\n", int(a[i]), int(b[i]), int(c[i]), int(c_ref[i]));
                return 1;
            }
            if (c2[i] != c2_ref[i]) {
                printf("ERROR in myfunc: a = %d, b2 = %d, c2 = %d, c2_ref = %d\n", int(a[i]), int(b2[i]), int(c2[i]), int(c2_ref[i]));
                return 1;
            }
        }
    }
    return 0;
}
