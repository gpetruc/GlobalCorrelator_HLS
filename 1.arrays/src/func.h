#ifndef my_func_h
#define my_func_h

#include <ap_fixed.h>

#define NDATA 12

ap_int<24> mul_add_basic(const ap_int<16> a[NDATA], const ap_int<16> b[NDATA]) ;

ap_int<24> mul_add_pipelined(const ap_int<16> a[NDATA], const ap_int<16> b[NDATA]) ;

ap_int<24> mul_add_partitioned(const ap_int<16> a[NDATA], const ap_int<16> b[NDATA]) ;

#endif
