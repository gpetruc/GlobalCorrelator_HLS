#include "src/data.h"
#include "src/algo.h"
#include <cmath>

num_t algo_main_ref(num_t threshold, hls::stream<num_t> & input) {
    num_t ret = 0;
    for (int i = 0; i < NITEMS; ++i) {
        num_t val = input.read();
        if (val > threshold) ret += val;
    }
    return ret;
}
