#ifndef FIRMWARE_LINPUPPI_H
#define FIRMWARE_LINPUPPI_H

#include <cmath>
#include "../../dataformats/layer1_objs.h"
#include "../../dataformats/layer1_multiplicities.h"
#include "../../dataformats/pf.h"
#include "../../dataformats/puppi.h"
#include "linpuppi_bits.h"

// charged
void linpuppi_chs(const l1ct::PFRegion & region, l1ct::z0_t pvZ0, const l1ct::PFChargedObj pfch[NTRACK], l1ct::PuppiObj outallch[NTRACK]) ;

// neutrals, in the tracker
void linpuppiNoCrop(const l1ct::PFRegion & region, const l1ct::TkObj track[NTRACK], l1ct::z0_t pvZ0, const l1ct::PFNeutralObj pfallne[NALLNEUTRALS], l1ct::PuppiObj outallne[NALLNEUTRALS]) ;
void linpuppi(const l1ct::PFRegion & region, const l1ct::TkObj track[NTRACK], l1ct::z0_t pvZ0, const l1ct::PFNeutralObj pfallne[NALLNEUTRALS], l1ct::PuppiObj outselne[NNEUTRALS]) ;

// streaming versions, taking one object at a time 
struct linpuppi_refobj { ap_uint<17> pt2_shift; l1ct::eta_t hwEta; l1ct::phi_t hwPhi; };
linpuppi_refobj linpuppi_prepare_track(const l1ct::TkObj & track, l1ct::z0_t pvZ0);
l1ct::PuppiObj linpuppi_one(const l1ct::PFRegion & region, const l1ct::PFNeutralObj & in, const linpuppi_refobj sel_track[NTRACK]);
l1ct::PuppiObj linpuppi_chs_one(const l1ct::PFRegion & region, const l1ct::PFChargedObj pfch, l1ct::z0_t pvZ0) ;
void linpuppiNoCrop_streamed(const l1ct::PFRegion & region, const l1ct::TkObj track[NTRACK], l1ct::z0_t pvZ0, const l1ct::PFNeutralObj pfallne[NALLNEUTRALS], l1ct::PuppiObj outallne[NALLNEUTRALS]) ;
void linpuppi_chs_streamed(const l1ct::PFRegion & region, l1ct::z0_t pvZ0, const l1ct::PFChargedObj pfch[NTRACK], l1ct::PuppiObj outallch[NTRACK]) ;

// neutrals, forward
void fwdlinpuppi(const l1ct::PFRegion & region, const l1ct::HadCaloObj caloin[NCALO], l1ct::PuppiObj pfselne[NNEUTRALS]);
void fwdlinpuppiNoCrop(const l1ct::PFRegion & region, const l1ct::HadCaloObj caloin[NCALO], l1ct::PuppiObj pfallne[NCALO]);

#define LINPUPPI_DATA_SIZE_IN 72
#define LINPUPPI_DATA_SIZE_OUT 64
#define LINPUPPI_DATA_SIZE_FWD 64
#define LINPUPPI_NCHANN_IN (2+NTRACK+NALLNEUTRALS)
#define LINPUPPI_NCHANN_OUTNC (NALLNEUTRALS)
#define LINPUPPI_NCHANN_OUT (NNEUTRALS)
#define LINPUPPI_CHS_NCHANN_IN (2+NTRACK)
#define LINPUPPI_CHS_NCHANN_OUT (NTRACK)
#define LINPUPPI_NCHANN_FWD_IN (1+NCALO)
#define LINPUPPI_NCHANN_FWD_OUTNC (NCALO)
#define LINPUPPI_NCHANN_FWD_OUT (NNEUTRALS)

void packed_fwdlinpuppi(const ap_uint<LINPUPPI_DATA_SIZE_FWD> input[LINPUPPI_NCHANN_FWD_IN], ap_uint<LINPUPPI_DATA_SIZE_FWD> output[LINPUPPI_NCHANN_FWD_OUT]) ;
void packed_fwdlinpuppiNoCrop(const ap_uint<LINPUPPI_DATA_SIZE_FWD> input[LINPUPPI_NCHANN_FWD_IN], ap_uint<LINPUPPI_DATA_SIZE_FWD> output[LINPUPPI_NCHANN_FWD_OUTNC]) ;

void packed_linpuppi_chs(const ap_uint<LINPUPPI_DATA_SIZE_IN> input[LINPUPPI_CHS_NCHANN_IN], ap_uint<LINPUPPI_DATA_SIZE_OUT> output[LINPUPPI_CHS_NCHANN_OUT]);
void packed_linpuppi(const ap_uint<LINPUPPI_DATA_SIZE_IN> input[LINPUPPI_NCHANN_IN], ap_uint<LINPUPPI_DATA_SIZE_OUT> output[LINPUPPI_NCHANN_OUT]);
void packed_linpuppiNoCrop(const ap_uint<LINPUPPI_DATA_SIZE_IN> input[LINPUPPI_NCHANN_IN], ap_uint<LINPUPPI_DATA_SIZE_OUT> output[LINPUPPI_NCHANN_OUTNC]);

void linpuppi_pack_in(const l1ct::PFRegion & region, const l1ct::TkObj track[NTRACK], l1ct::z0_t pvZ0, const l1ct::PFNeutralObj pfallne[NALLNEUTRALS], ap_uint<LINPUPPI_DATA_SIZE_IN> input[LINPUPPI_CHS_NCHANN_IN]);
void linpuppi_unpack_in(const ap_uint<LINPUPPI_DATA_SIZE_IN> input[LINPUPPI_CHS_NCHANN_IN], l1ct::PFRegion & region, l1ct::TkObj track[NTRACK], l1ct::z0_t & pvZ0, l1ct::PFNeutralObj pfallne[NALLNEUTRALS]);
void linpuppi_chs_pack_in(const l1ct::PFRegion & region, l1ct::z0_t pvZ0, const l1ct::PFChargedObj pfch[NTRACK], ap_uint<LINPUPPI_DATA_SIZE_IN> input[LINPUPPI_CHS_NCHANN_IN]);
void linpuppi_chs_unpack_in(const ap_uint<LINPUPPI_DATA_SIZE_IN> input[LINPUPPI_CHS_NCHANN_IN], l1ct::PFRegion & region, l1ct::z0_t & pvZ0, l1ct::PFChargedObj pfch[NTRACK]);

typedef ap_uint<17+l1ct::eta_t::width+l1ct::phi_t::width> packed_linpuppi_refobj;
inline linpuppi_refobj linpuppi_refobj_unpack(const packed_linpuppi_refobj & src) {
    linpuppi_refobj ret;
    ret.pt2_shift(16,0)        = src(16,0);
    ret.hwEta(l1ct::eta_t::width-1, 0) = src(17+l1ct::eta_t::width-1,17);
    ret.hwPhi(l1ct::phi_t::width-1, 0) = src(17+l1ct::eta_t::width+l1ct::phi_t::width-1,17+l1ct::eta_t::width);
    return ret;
}
inline packed_linpuppi_refobj linpuppi_refobj_pack(const linpuppi_refobj & src) {
    packed_linpuppi_refobj ret;
    ret(16,0) = src.pt2_shift;
    ret(17+l1ct::eta_t::width-1,17) = src.hwEta(l1ct::eta_t::width-1, 0);
    ret(17+l1ct::eta_t::width+l1ct::phi_t::width-1,17+l1ct::eta_t::width) = src.hwPhi(l1ct::phi_t::width-1, 0);
    return ret;
}

packed_linpuppi_refobj packed_linpuppi_prepare_track(const ap_uint<l1ct::TkObj::BITWIDTH> & track, const l1ct::z0_t & pvZ0);
ap_uint<l1ct::PuppiObj::BITWIDTH> packed_linpuppi_one(const ap_uint<l1ct::PFRegion::BITWIDTH> & region, const ap_uint<l1ct::PFNeutralObj::BITWIDTH> & in, const packed_linpuppi_refobj sel_tracks[NTRACK]);
ap_uint<l1ct::PuppiObj::BITWIDTH> packed_linpuppi_chs_one(const ap_uint<l1ct::PFRegion::BITWIDTH> & region, const ap_uint<l1ct::PFChargedObj::BITWIDTH> & pfch, const l1ct::z0_t & pvZ0) ;

// these two call the packed versions internally, and are used for valiation (they are not synthethized directly)
void packed_linpuppiNoCrop_streamed(const l1ct::PFRegion & region, const l1ct::TkObj track[NTRACK], l1ct::z0_t pvZ0, const l1ct::PFNeutralObj pfallne[NALLNEUTRALS], l1ct::PuppiObj outallne[NALLNEUTRALS]) ;
void packed_linpuppi_chs_streamed(const l1ct::PFRegion & region, l1ct::z0_t pvZ0, const l1ct::PFChargedObj pfch[NTRACK], l1ct::PuppiObj outallch[NTRACK]) ;

void linpuppi_set_debug(bool debug);


//=================================================
#if defined(REG_Barrel)

#define LINPUPPI_DR2MAX  4727 // 0.3 cone
#define LINPUPPI_DR2MIN   257 // 0.07 cone
#define LINPUPPI_dzCut     10
#define LINPUPPI_ptMax    200 // 50.0/LINPUPPI_ptLSB 

#define LINPUPPI_ptSlopeNe  0.3
#define LINPUPPI_ptSlopePh  0.3
#define LINPUPPI_ptZeroNe   4.0
#define LINPUPPI_ptZeroPh   2.5
#define LINPUPPI_alphaSlope 0.7
#define LINPUPPI_alphaZero  6.0
#define LINPUPPI_alphaCrop  4.0
#define LINPUPPI_priorNe    5.0
#define LINPUPPI_priorPh    1.0

#define LINPUPPI_ptCut        4 // 1.0/LINPUPPI_ptLSB

//=================================================
#elif defined(REG_HGCal) 

#define LINPUPPI_etaBins 2
#define LINPUPPI_etaCut  2.0 // abseta

#define LINPUPPI_DR2MAX  4727 // 0.3 cone
#define LINPUPPI_DR2MIN    84 // 0.04 cone
#define LINPUPPI_dzCut     40
#define LINPUPPI_ptMax    200 // 50.0/LINPUPPI_ptLSB 

#define LINPUPPI_ptSlopeNe  0.3 
#define LINPUPPI_ptSlopePh  0.4 
#define LINPUPPI_ptZeroNe   5.0 
#define LINPUPPI_ptZeroPh   3.0 
#define LINPUPPI_alphaSlope 1.5 
#define LINPUPPI_alphaZero  6.0 
#define LINPUPPI_alphaCrop  3.0 
#define LINPUPPI_priorNe    5.0 
#define LINPUPPI_priorPh    1.5 

#define LINPUPPI_ptSlopeNe_1  0.3 
#define LINPUPPI_ptSlopePh_1  0.4 
#define LINPUPPI_ptZeroNe_1   7.0 
#define LINPUPPI_ptZeroPh_1   4.0 
#define LINPUPPI_alphaSlope_1 1.5 
#define LINPUPPI_alphaZero_1  6.0 
#define LINPUPPI_alphaCrop_1  3.0 
#define LINPUPPI_priorNe_1    5.0 
#define LINPUPPI_priorPh_1    1.5 


#define LINPUPPI_ptCut        4 // 1.0/LINPUPPI_ptLSB
#define LINPUPPI_ptCut_1      8 // 2.0/LINPUPPI_ptLSB

//=================================================
#elif defined(REG_HGCalNoTK)

#define LINPUPPI_DR2MAX  4727 // 0.3 cone
#define LINPUPPI_DR2MIN    84 // 0.04 cone
#define LINPUPPI_dzCut     40 // unused
#define LINPUPPI_ptMax    200 // 50.0/LINPUPPI_ptLSB 

#define LINPUPPI_ptSlopeNe  0.3
#define LINPUPPI_ptSlopePh  0.4
#define LINPUPPI_ptZeroNe   9.0
#define LINPUPPI_ptZeroPh   5.0
#define LINPUPPI_alphaSlope 2.2
#define LINPUPPI_alphaZero  9.0
#define LINPUPPI_alphaCrop  4.0
#define LINPUPPI_priorNe    7.0
#define LINPUPPI_priorPh    5.0

#define LINPUPPI_ptCut       16 // 4.0/LINPUPPI_ptLSB

//=================================================
#elif defined(REG_HF)

#define LINPUPPI_DR2MAX  4727 // 0.3 cone
#define LINPUPPI_DR2MIN   525 // 0.1 cone
#define LINPUPPI_dzCut     40 // unused

#define LINPUPPI_ptMax    400 // 100.0/LINPUPPI_ptLSB 

#define LINPUPPI_ptSlopeNe  0.25
#define LINPUPPI_ptSlopePh  0.25
#define LINPUPPI_ptZeroNe   14.
#define LINPUPPI_ptZeroPh   14.
#define LINPUPPI_alphaSlope 0.6
#define LINPUPPI_alphaZero  9.0
#define LINPUPPI_alphaCrop  4.0
#define LINPUPPI_priorNe    6.0
#define LINPUPPI_priorPh    6.0

#define LINPUPPI_ptCut      40  // 10.0/LINPUPPI_ptLSB

#endif

#endif
