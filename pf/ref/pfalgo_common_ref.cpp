#include "pfalgo_common_ref.h"

#include <cmath>
#include <cstdio>

void pfalgo_mu_ref(const pfalgo_config &cfg, const TkObj track[/*cfg.nTRACK*/], const MuObj mu[/*cfg.nMU*/], bool isMu[/*cfg.nTRACK*/], PFChargedObj outmu[/*cfg.nMU*/], bool debug) {

    // init
    for (unsigned int ipf = 0; ipf < cfg.nMU; ++ipf) clear(outmu[ipf]);
    for (unsigned int it = 0; it < cfg.nTRACK; ++it) isMu[it] = 0;

        // for each muon, find the closest track
    for (unsigned int im = 0; im < cfg.nMU; ++im) {
        if (mu[im].hwPt > 0) {
            int ibest = -1;
            pt_t dptmin = mu[im].hwPt >> 1;
            for (unsigned int it = 0; it < cfg.nTRACK; ++it) {
                unsigned int dr = dr2_int(mu[im].hwEta, mu[im].hwPhi, track[it].hwEta, track[it].hwPhi);
                //printf("deltaR2(mu %d float pt %5.1f, tk %2d float pt %5.1f) = int %d  (float deltaR = %.3f); int cut at %d\n", im, 0.25*int(mu[im].hwPt), it, 0.25*int(track[it].hwPt), dr, std::sqrt(float(dr))/229.2, cfg.dR2MAX_TK_MU);
                if (dr < cfg.dR2MAX_TK_MU) { 
                    dpt_t dpt = (dpt_t(track[it].hwPt) - dpt_t(mu[im].hwPt));
                    pt_t absdpt = dpt >= 0 ? pt_t(dpt) : pt_t(-dpt);
                    if (absdpt < dptmin) {
                        dptmin = absdpt; ibest = it; 
                    }
                }
            }
            if (ibest != -1) {
                outmu[im].hwPt = track[ibest].hwPt;
                outmu[im].hwEta = track[ibest].hwEta;
                outmu[im].hwPhi = track[ibest].hwPhi;
                outmu[im].hwId  = ParticleID::mkMuon(track[ibest].hwCharge);
                outmu[im].hwDEta = track[ibest].hwDEta;
                outmu[im].hwDPhi = track[ibest].hwDPhi;
                outmu[im].hwZ0 = track[ibest].hwZ0;
                outmu[im].hwDxy = track[ibest].hwDxy;
                outmu[im].hwTkQuality = track[ibest].hwQuality;
                isMu[ibest] = 1;
                if (debug) printf("FW  \t muon %3d linked to track %3d \n", im, ibest);
            } else {
                if (debug) printf("FW  \t muon %3d not linked to any track\n", im);
            }
        }
    }
}
