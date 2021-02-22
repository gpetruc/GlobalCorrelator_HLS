#ifndef multififo_regionizer_router_ref_h
#define multififo_regionizer_router_ref_h

#include <cassert>
#include "../../dataformats/layer1_emulator.h"

bool tk_router_ref(bool newevent, const l1ct::TkObjEmu tracks_in[NTKSECTORS][NTKFIBERS], l1ct::TkObjEmu tracks_out[NTKOUT]) ;
bool calo_router_ref(bool newevent, const l1ct::HadCaloObjEmu calo_in[NCALOSECTORS][NCALOFIBERS], l1ct::HadCaloObjEmu calo_out[NCALOOUT]) ;
bool mu_router_ref(bool newevent, const l1ct::glbeta_t etaCenter, const l1ct::MuObjEmu mu_in[NMUFIBERS], l1ct::MuObjEmu mu_out[NMUOUT]) ;


bool tk_router_ref(bool newevent, const std::vector<l1ct::TkObjEmu> & links_in, std::vector<l1ct::TkObjEmu> & out) ;
bool calo_router_ref(bool newevent, const std::vector<l1ct::HadCaloObjEmu> & links_in, std::vector<l1ct::HadCaloObjEmu> & out) ;
bool mu_router_ref(bool newevent, const l1ct::glbeta_t etaCenter, const std::vector<l1ct::MuObjEmu> & links_in, std::vector<l1ct::MuObjEmu> & out) ;

#endif
