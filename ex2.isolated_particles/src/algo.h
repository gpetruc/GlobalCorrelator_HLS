#ifndef ALGO_H
#define ALGO_H

#include "data.h"

#define NPUPPI_MAX 208
#define NISO_MAX 12

typedef ap_uint<24> dr2_t;
inline dr2_t drToHwDr2(float dr) { return dr2_t(round(std::pow(dr/Puppi::ETAPHI_LSB,2))); }
inline float hwDr2ToDr(dr2_t dr2) { return std::sqrt(dr2.to_int()*Puppi::ETAPHI_LSB*Puppi::ETAPHI_LSB); }

inline dr2_t deltaR2(const Puppi & p1, const Puppi & p2) {
    auto dphi = p1.hwPhi - p2.hwPhi;
    if (dphi > Puppi::INT_PI) dphi -= Puppi::INT_2PI;
    else if (dphi < -Puppi::INT_PI) dphi += Puppi::INT_2PI;
    auto deta = p1.hwEta - p2.hwEta;
    return dphi*dphi + deta*deta;
}


void compute_isolated_l1t(const Puppi in[NPUPPI_MAX], Puppi out[NISO_MAX], Puppi::pt_t out_absiso[NISO_MAX]) ;

void compute_isolated_alveo(unsigned int N, const uint64_t *in, unsigned int & nout, uint64_t *out, uint16_t *out_absiso) ;

#endif