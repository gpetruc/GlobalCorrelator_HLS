#ifndef FIRMWARE_PFTKEGALGO_H
#define FIRMWARE_PFTKEGALGO_H

#include "pfalgo_common.h"
#include "data.h"

void pftkegalgo(const EmCaloObj emcalo[NEMCALO], const TkObj track[NTRACK],
  EGIsoParticle photons[NEM_EGOUT],
  EGIsoEleParticle eles[NEM_EGOUT]
) ;

void link_emCalo2tk(const EmCaloObj emcalo[NEMCALOSEL_EGIN], const TkObj track[NTRACK], ap_uint<NTRACK> emCalo2tk_bit[NEMCALOSEL_EGIN]);

void link_emCalo2emCalo(const EmCaloObj emcalo[NEMCALOSEL_EGIN], ap_uint<NEMCALOSEL_EGIN> emCalo2emcalo_bit[NEMCALOSEL_EGIN]);

void sel_emCalo(const EmCaloObj emcalo[NEMCALO], EmCaloObj emcalo_sel[NEMCALOSEL_EGIN]);

// FIXME: I added this to have pfalgo_common.icc compile...
#ifndef CMSSW_GIT_HASH
#define PFALGO_DR2MAX_TK_CALO 525
#define PFALGO_TK_MAXINVPT_LOOSE    40
#define PFALGO_TK_MAXINVPT_TIGHT    80
#endif


#endif
