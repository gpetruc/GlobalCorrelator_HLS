#ifndef my_func_h
#define my_func_h

#include <ap_int.h>
#include <ap_fixed.h>

#define NDATA 12


typedef ap_int<24> aterm_b; 
typedef ap_int<12> bterm_b;
typedef ap_int<16> cterm_b;
typedef ap_int<16> ret_b;
typedef ap_int<30> acc_b;
// implied decimal bits
#define a_dec 8
#define b_dec 10
#define c_dec 8
#define r_dec 4

typedef ap_fixed<aterm_b::width,aterm_b::width-a_dec> aterm_f;
typedef ap_fixed<bterm_b::width,bterm_b::width-b_dec> bterm_f;
typedef ap_fixed<cterm_b::width,cterm_b::width-c_dec> cterm_f;
typedef ap_fixed<ret_b::width,ret_b::width-r_dec> ret_f;
typedef ap_fixed<30,30-c_dec> acc_f;


typedef ap_fixed<aterm_f::width,aterm_f::iwidth,AP_TRN,AP_SAT> aterm_s;
typedef ap_fixed<bterm_f::width,bterm_f::iwidth,AP_TRN,AP_SAT> bterm_s;
typedef ap_fixed<cterm_f::width,cterm_f::iwidth,AP_TRN,AP_SAT> cterm_s;
typedef ap_fixed<ret_f::width,ret_f::iwidth,AP_TRN,AP_SAT> ret_s;
typedef ap_fixed<30,30-c_dec,AP_TRN,AP_SAT> acc_s;


ret_b op_int(const aterm_b a[NDATA], const bterm_b b[NDATA], const cterm_b c[NDATA]) ;
ret_f op_fix(const aterm_f a[NDATA], const bterm_f b[NDATA], const cterm_f c[NDATA]) ;
ret_s op_sat(const aterm_s a[NDATA], const bterm_s b[NDATA], const cterm_s c[NDATA]) ;

ret_b op_int_redux(const aterm_b a[NDATA], const bterm_b b[NDATA], const cterm_b c[NDATA]) ;
ret_f op_fix_redux(const aterm_f a[NDATA], const bterm_f b[NDATA], const cterm_f c[NDATA]) ;
ret_s op_sat_redux(const aterm_s a[NDATA], const bterm_s b[NDATA], const cterm_s c[NDATA]) ;


#endif
