#include "algo.h"
#include <cmath>
#include <cassert>
#ifndef __SYNTHESIS__
#include <cstdio>
#endif

num_t algo_main(num_t threshold, hls::stream<num_t> & input) {
    #pragma HLS pipeline II=6
    num_t ret = 0;
    for (int i = 0; i < NITEMS; ++i) {
        num_t val = input.read();
        if (val > threshold) ret += val;
    }
    return ret;
}
