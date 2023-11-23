#include "func.h"

ap_int<16> basic_sum(ap_int<16> a, ap_int<10> b) {
    return a+b;
}

ap_int<24> basic_mul(ap_int<16> a, ap_int<10> b) {
    return a*b;
}

ap_int<24> pipelined_mul(ap_int<16> a, ap_int<10> b) {
    #pragma HLS pipeline II=1
    return a*b;
}

void sum_and_mul(ap_int<16> a, ap_int<10> b, ap_int<16> &out_sum, ap_int<24> &out_prod) {
    #pragma HLS pipeline II=1
    out_sum = a + b;
    out_prod = a * b;
}

ap_int<24> sum_or_mul(bool want_sum, ap_int<16> a, ap_int<10> b) {
	#pragma HLS pipeline II=1
    return want_sum ? ap_int<24>(a + b) : ap_int<24>(a * b);
}

ap_int<16> basic_div(ap_int<16> a, ap_int<10> b) {
	#pragma HLS pipeline II=1
    return a / b;
}

ap_fixed<16,12> fix_sum(ap_fixed<16,12> a, ap_fixed<10,6> b) {
	#pragma HLS pipeline II=1
    return a + b;
}

ap_fixed<16,12,AP_TRN,AP_SAT> fix_sum_sat(ap_fixed<16,12,AP_TRN,AP_SAT> a, ap_fixed<10,6,AP_TRN,AP_SAT> b) {
    #pragma HLS pipeline II=1
    return a + b;
}