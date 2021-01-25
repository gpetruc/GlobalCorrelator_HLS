#ifndef FIRMWARE_PFALGO2HGC_H
#define FIRMWARE_PFALGO2HGC_H

#ifndef REG_HGCal
  #ifndef CMSSW_GIT_HASH
    #warning "REG_HGCal is not #defined, but this algorithm has only been tested there"
  #endif
#endif

#include "pfalgo_common.h"

void pfalgo2hgc(const PFRegion & region, const HadCaloObj calo[NCALO], const TkObj track[NTRACK], const MuObj mu[NMU], PFChargedObj outch[NTRACK], PFNeutralObj outne[NSELCALO], PFChargedObj outmu[NMU]) ;

#define PFALGO2HGC_DATA_SIZE 72
#define PFALGO2HGC_NCHANN_IN (1+NTRACK+NCALO+NMU)
#define PFALGO2HGC_NCHANN_OUT (NTRACK+NSELCALO+NMU)
void packed_pfalgo2hgc(const ap_uint<PFALGO2HGC_DATA_SIZE> input[PFALGO2HGC_NCHANN_IN], ap_uint<PFALGO2HGC_DATA_SIZE> output[PFALGO2HGC_NCHANN_OUT]) ;
void pfalgo2hgc_pack_in(const PFRegion & region, const HadCaloObj calo[NCALO], const TkObj track[NTRACK], const MuObj mu[NMU], ap_uint<PFALGO2HGC_DATA_SIZE> input[PFALGO2HGC_NCHANN_IN]) ;
void pfalgo2hgc_unpack_in(const ap_uint<PFALGO2HGC_DATA_SIZE> input[PFALGO2HGC_NCHANN_IN], PFRegion & region, HadCaloObj calo[NCALO], TkObj track[NTRACK], MuObj mu[NMU]) ;
void pfalgo2hgc_pack_out(const PFChargedObj outch[NTRACK], const PFNeutralObj outne[NSELCALO], const PFChargedObj outmu[NMU], ap_uint<PFALGO2HGC_DATA_SIZE> output[PFALGO2HGC_NCHANN_OUT]) ;
void pfalgo2hgc_unpack_out(const ap_uint<PFALGO2HGC_DATA_SIZE> output[PFALGO2HGC_NCHANN_OUT], PFChargedObj outch[NTRACK], PFNeutralObj outne[NSELCALO], PFChargedObj outmu[NMU]) ;

#ifndef CMSSW_GIT_HASH
#define PFALGO_DR2MAX_TK_CALO 525
#define PFALGO_TK_MAXINVPT_LOOSE    40
#define PFALGO_TK_MAXINVPT_TIGHT    80
#endif

#endif
