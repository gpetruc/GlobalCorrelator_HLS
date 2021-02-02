#ifndef utils_test_utils_h
#define utils_test_utils_h

#include "../dataformats/layer1_objs.h"
#include "../dataformats/pf.h"
#include "../dataformats/puppi.h"

bool track_equals(const l1ct::TkObj &out_ref, const l1ct::TkObj &out, const char *what, int idx) ;
bool had_equals(const l1ct::HadCaloObj &out_ref, const l1ct::HadCaloObj &out, const char *what, int idx) ;
bool em_equals(const l1ct::EmCaloObj &out_ref, const l1ct::EmCaloObj &out, const char *what, int idx) ;
bool mu_equals(const l1ct::MuObj &out_ref, const l1ct::MuObj &out, const char *what, int idx) ;
bool pf_equals(const l1ct::PFChargedObj &out_ref, const l1ct::PFChargedObj &out, const char *what, int idx) ;
bool pf_equals(const l1ct::PFNeutralObj &out_ref, const l1ct::PFNeutralObj &out, const char *what, int idx) ;
bool puppi_equals(const l1ct::PuppiObj &out_ref, const l1ct::PuppiObj &out, const char *what, int idx) ;

template<typename T> 
unsigned int count_nonzero(T objs[], unsigned int NMAX) {
    unsigned int ret = 0;
    for (unsigned int i = 0; i < NMAX; ++i) ret += (objs[i].hwPt > 0);
    return ret;
}

#endif
