#include "algo.h"
#include <cmath>
#include <cassert>
#ifndef __SYNTHESIS__
#include <cstdio>
#endif

#define STRINGIFY(a) # a
#define REQ_HLS_PIPELINE(N) \
    _Pragma( STRINGIFY(HLS pipeline II=N) )

num_t algo_main(num_t threshold, hls::stream<num_t> & input) {
    REQ_HLS_PIPELINE(NITEMS)
    num_t ret = 0;
    for (int i = 0; i < NITEMS; ++i) {
        num_t val = input.read();
        if (val > threshold) ret += val;
    }
    return ret;
}
