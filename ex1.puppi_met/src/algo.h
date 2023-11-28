#ifndef ALGO_H
#define ALGO_H

#include "data.h"

#define NPUPPI_MAX 216

void compute_sums_l1t(const Puppi in[NPUPPI_MAX], Sum & out, Sum & out_nomu) ;

void compute_sums_alveo(unsigned int N, const uint64_t *in, uint64_t out[2]) ;

#endif