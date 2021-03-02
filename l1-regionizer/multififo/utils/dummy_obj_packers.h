#ifndef multififo_regionizer_obj_packers_h
#define multififo_regionizer_obj_packers_h

#include "../firmware/regionizer.h"
#include "../../../dataformats/layer1_emulator.h"
#include <vector>

std::vector<ap_uint<64>> pack_tracks(const l1ct::DetectorSector<l1ct::TkObjEmu> & tracks) ; 
std::vector<ap_uint<64>> pack_hgcal(const l1ct::DetectorSector<l1ct::HadCaloObjEmu> & calo) ;
std::vector<ap_uint<64>> pack_muons(const l1ct::DetectorSector<l1ct::MuObjEmu> & mu) ; 

#endif
