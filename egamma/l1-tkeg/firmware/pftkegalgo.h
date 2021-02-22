#ifndef FIRMWARE_PFTKEGALGO_H
#define FIRMWARE_PFTKEGALGO_H

#include "../../../dataformats/layer1_emulator.h"
#include "../../../dataformats/layer1_multiplicities.h"

void pftkegalgo(const l1ct::PFRegion & region, 
  const l1ct::EmCaloObj emcalo[NEMCALO], const l1ct::TkObj track[NTRACK],
  l1ct::EGIsoObj photons[NEM_EGOUT], l1ct::EGIsoEleObj eles[NEM_EGOUT]
) ;

void link_emCalo2tk(const l1ct::PFRegion & region, const l1ct::EmCaloObj emcalo[NEMCALO_EGIN], const l1ct::TkObj track[NTRACK], ap_uint<NTRACK> emCalo2tk_bit[NEMCALO_EGIN]);

void link_emCalo2emCalo(const l1ct::EmCaloObj emcalo[NEMCALO_EGIN], ap_uint<NEMCALO_EGIN> emCalo2emcalo_bit[NEMCALO_EGIN]);

void sel_emCalo(const l1ct::EmCaloObj emcalo[NEMCALO], l1ct::EmCaloObj emcalo_sel[NEMCALO_EGIN]);

#endif
