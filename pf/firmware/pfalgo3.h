#ifndef FIRMWARE_PFALGO3_H
#define FIRMWARE_PFALGO3_H

#ifndef REG_Barrel
  #ifndef CMSSW_GIT_HASH
    #warning "REG_Barrel not defined in PFALGO3: not validated"
  #endif
#endif

#include "pfalgo_common.h"

void pfalgo3(const l1ct::PFRegion & region, const l1ct::EmCaloObj emcalo[NEMCALO], const l1ct::HadCaloObj hadcalo[NCALO], const l1ct::TkObj track[NTRACK], const l1ct::MuObj mu[NMU], l1ct::PFChargedObj outch[NTRACK], l1ct::PFNeutralObj outpho[NPHOTON], l1ct::PFNeutralObj outne[NSELCALO], l1ct::PFChargedObj outmu[NMU]) ;

#define PFALGO3_DATA_SIZE 72
#define PFALGO3_NCHANN_IN (1+NTRACK+NEMCALO+NCALO+NMU)
#define PFALGO3_NCHANN_OUT (NTRACK+NPHOTON+NSELCALO+NMU)
void packed_pfalgo3(const ap_uint<PFALGO3_DATA_SIZE> input[PFALGO3_NCHANN_IN], ap_uint<PFALGO3_DATA_SIZE> output[PFALGO3_NCHANN_OUT]) ;
void pfalgo3_pack_in(const l1ct::PFRegion & region, const l1ct::EmCaloObj emcalo[NEMCALO], const l1ct::HadCaloObj hadcalo[NCALO], const l1ct::TkObj track[NTRACK], const l1ct::MuObj mu[NMU], ap_uint<PFALGO3_DATA_SIZE> input[PFALGO3_NCHANN_IN]) ;
void pfalgo3_unpack_in(const ap_uint<PFALGO3_DATA_SIZE> input[PFALGO3_NCHANN_IN], l1ct::PFRegion & region, l1ct::EmCaloObj emcalo[NEMCALO], l1ct::HadCaloObj hadcalo[NCALO], l1ct::TkObj track[NTRACK], l1ct::MuObj mu[NMU]) ;
void pfalgo3_pack_out(const l1ct::PFChargedObj outch[NTRACK], const l1ct::PFNeutralObj outpho[NPHOTON], const l1ct::PFNeutralObj outne[NSELCALO], const l1ct::PFChargedObj outmu[NMU], ap_uint<PFALGO3_DATA_SIZE> output[PFALGO3_NCHANN_OUT]) ;
void pfalgo3_unpack_out(const ap_uint<PFALGO3_DATA_SIZE> output[PFALGO3_NCHANN_OUT], l1ct::PFChargedObj outch[NTRACK], l1ct::PFNeutralObj outpho[NPHOTON], l1ct::PFNeutralObj outne[NSELCALO], l1ct::PFChargedObj outmu[NMU]) ;


void pfalgo3_set_debug(bool debug);

#ifndef CMSSW_GIT_HASH
#define PFALGO_DR2MAX_TK_CALO 1182
#define PFALGO_DR2MAX_EM_CALO 525
#define PFALGO_DR2MAX_TK_EM   84
#define PFALGO_TK_MAXINVPT_LOOSE    40
#define PFALGO_TK_MAXINVPT_TIGHT    80
#endif

#endif
