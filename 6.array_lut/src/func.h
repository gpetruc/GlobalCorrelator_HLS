#ifndef my_func_h
#define my_func_h

#include <ap_int.h>

#define NDATA 12

void myfunc(const ap_int<16> a[NDATA], const bool b[NDATA], ap_int<24> c[NDATA]) ;
void myfunc2(const ap_int<16> a[NDATA], const ap_uint<2> b[NDATA], ap_int<24> c[NDATA]) ;

#endif
