#include "dummy_obj_packers.h"

std::vector<ap_uint<64>> pack_tracks(const l1ct::DetectorSector<l1ct::TkObjEmu> & tracks) {
    std::vector<ap_uint<64>> ret;
    for (unsigned int i = 0, n = tracks.size(); i < n; ++i) {
        // simulate 96 bit objects
        ap_uint<96> packedtk = tracks[i].pack();
        if (i % 2 == 0) {
            ret.emplace_back(packedtk(63,0));
            ret.emplace_back((packedtk(95,64), ap_uint<32>(0)));
        } else {
            ret.back()(31,0) = packedtk(63,32);
            ret.emplace_back((packedtk(31, 0),packedtk(95,64)));
        }
    }
    return ret;
}

std::vector<ap_uint<64>> pack_hgcal(const l1ct::DetectorSector<l1ct::HadCaloObjEmu> & calo) {
    std::vector<ap_uint<64>> ret;
    for (unsigned int i = 0, n = calo.size(); i < n; ++i) {
        // simulate 128 bit objects on a 16 G link
        ap_uint<128> packed = calo[i].pack();
        ret.emplace_back(packed( 63, 0));
        ret.emplace_back(packed(127,64));
        ret.push_back(0); // third zero frame, that will have the strobe bit off
    }
    return ret;

}

std::vector<ap_uint<64>> pack_muons(const l1ct::DetectorSector<l1ct::MuObjEmu> & mu) {
    std::vector<ap_uint<64>> ret;
    for (unsigned int i = 0, n = mu.size(); i < n; ++i) {
        // simulate 128 bit objects on a 25 G link
        ap_uint<128> packed = mu[i].pack();
        ret.emplace_back(packed( 63, 0));
        ret.emplace_back(packed(127,64));
    }
    return ret;
}

