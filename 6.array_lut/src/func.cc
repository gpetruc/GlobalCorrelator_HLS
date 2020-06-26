#include "func.h"
#include <cassert>


void myfunc(const ap_int<16> a[NDATA], const bool b[NDATA], ap_int<24> c[NDATA]) {
    #pragma HLS pipeline II=1
    #pragma HLS array_partition variable=a complete
    #pragma HLS array_partition variable=b complete
    #pragma HLS array_partition variable=c complete

    const int otherFactor[2] = { 42, 137 };

    for (unsigned int i = 0; i < NDATA; ++i) {
        ap_int<10> shift = b[i] ? otherFactor[0] : otherFactor[1];
        c[i] = a[i] * shift;
    }
}

void myfunc2(const ap_int<16> a[NDATA], const ap_uint<2> b[NDATA], ap_int<24> c[NDATA]) {
    #pragma HLS pipeline II=1
    #pragma HLS array_partition variable=a complete
    #pragma HLS array_partition variable=b complete
    #pragma HLS array_partition variable=c complete

    const ap_int<10> otherFactor[4] = { 42, 137, 25, -18 };

    for (unsigned int i = 0; i < NDATA; ++i) {
        int idx = b[i]; assert(idx >= 0 && idx < 4);
        ap_int<10> shift = otherFactor[idx];
        c[i] = a[i] * shift;
    }
}
