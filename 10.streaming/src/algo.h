#ifndef ALGO_H
#define ALGO_H

#include "data.h"
#include "hls_stream.h"

// implementation to be synthethised
num_t algo_main(num_t threshold, hls::stream<num_t> & input) ;

// reference implementation for validation
num_t algo_main_ref(num_t threshold, hls::stream<num_t> & input) ;

#endif
