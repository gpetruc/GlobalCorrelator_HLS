#include "src/func.h"
#include <cstdio>
#include <cstdlib>

int main() {
    srand(125);
    for (unsigned int itest = 0, ntest = 20; itest <= ntest; ++itest) {
        ap_int<16> a = (rand() & 0xFFF);
        ap_int<10> b = (rand() & 0xFF);
        bool want_sum = rand() & 1;
        printf("test %d: a = %d, b = %d, want_sum = %d:\n",
            itest,
            a.to_int(),
            b.to_int(),
            int(want_sum));

        bool ok = true;

        ap_int<16> sum_ref = a + b;
        ap_int<16> sum1_fw = basic_sum(a, b);
        printf(" - basic_sum = %d (expected %d), %s\n", 
            sum1_fw.to_int(), sum_ref.to_int(), 
            sum1_fw == sum_ref ? " ok " : "FAIL");
        ok = ok && (sum1_fw == sum_ref);

        ap_int<24> prod_ref = a * b;
        ap_int<24> prod1_fw = basic_mul(a, b);
        printf(" - basic_mul = %d (expected %d), %s\n", 
            prod1_fw.to_int(), 
            prod_ref.to_int(), 
            prod1_fw == prod_ref ? " ok " : "FAIL");
        ok = ok && (prod1_fw == prod_ref);

        ap_int<24> prod2_fw = pipelined_mul(a, b);
        printf(" - basic_mul = %d (expected %d), %s\n", 
            prod2_fw.to_int(), 
            prod_ref.to_int(), 
            prod2_fw == prod_ref ? " ok " : "FAIL");
        ok = ok && (prod2_fw == prod_ref);

        ap_int<16> sum3_fw;
        ap_int<24> prod3_fw;
        sum_and_mul(a, b, sum3_fw, prod3_fw);
        printf(" - sum_and_mul = %d %d (expected %d %d), %s\n", 
            sum3_fw.to_int(), prod3_fw.to_int(), 
            sum_ref.to_int(), prod_ref.to_int(), 
            (prod3_fw == prod_ref && sum3_fw == sum_ref) ? " ok " : "FAIL");
        ok = ok && (prod3_fw == prod_ref && sum3_fw == sum_ref);

        ap_int<24> choice_ref = want_sum ? ap_int<24>(sum_ref) : prod_ref;
        ap_int<24> choice_fw = sum_or_mul(want_sum, a, b);
        printf(" - sum_or_mul = %d (expected %d) %s\n", 
            choice_fw.to_int(), choice_ref.to_int(),
            choice_fw == choice_ref ? " ok " : "FAIL");
        ok = ok && (choice_fw == choice_ref); 

        ap_int<24> div_ref = a / b;
        ap_int<24> div_fw = basic_div(a, b);
        printf(" - basic_div = %d (expected %d) %s\n", 
            div_fw.to_int(), div_ref.to_int(),
            div_fw == div_ref ? " ok " : "FAIL");
        ok = ok && (div_fw == div_ref); 

        // fixed precision numbers with 4 decimal bits (i.e. LSB=2^-4 = 0.0625)
        ap_fixed<16,12> afix;
        ap_fixed<10,6> bfix;
        // copy the bits from a and b
        afix(15,0) = a(15,0);
        bfix(9,0) = b(9,0);
        // print out the numbers
        printf("fixed point a = %.4f (expected %.4f), b = %.4f (expected %.4f):\n",
            itest,
            afix.to_float(), a.to_int() * 0.0625,
            bfix.to_float(), b.to_int() * 0.0625);

        ap_fixed<16,12> sum_fix_ref = afix + bfix;
        ap_fixed<16,12> sum_fix_fw = fix_sum(afix, bfix);
        printf(" - fix_sum = %.4f (expected %.4f), %s\n", 
            sum_fix_fw.to_float(), sum_fix_ref.to_float(), 
            sum_fix_fw == sum_fix_ref ? " ok " : "FAIL");
        ok = ok && (sum_fix_fw == sum_fix_ref);

        ap_fixed<16,12,AP_TRN,AP_SAT> asat = afix;
        ap_fixed<10,6,AP_TRN,AP_SAT> bsat = bfix;
        printf("fixed point a = %.4f (expected %.4f), b = %.4f (expected %.4f):\n",
            itest,
            asat.to_float(), a.to_int() * 0.0625,
            asat.to_float(), b.to_int() * 0.0625);

        ap_fixed<16,12,AP_TRN,AP_SAT> sum_sat_ref = asat + bsat;
        ap_fixed<16,12,AP_TRN,AP_SAT> sum_sat_fw = fix_sum_sat(asat, bsat);
        printf(" - fix_sum_sat = %.4f (expected %.4f), %s\n", 
            sum_sat_fw.to_float(), sum_sat_ref.to_float(), 
            sum_sat_fw == sum_sat_ref ? " ok " : "FAIL");
        ok = ok && (sum_sat_fw == sum_sat_ref);

        if (!ok) return 1;
        printf("\n");
    }
    return 0;
}
