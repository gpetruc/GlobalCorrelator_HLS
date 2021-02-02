#include "pfalgo2hgc_ref.h"


#include <cmath>
#include <cstdio>
#include <algorithm>
#include <memory>

namespace l1ct {
    int g_pfalgo2hgc_debug_ref_ = 0;
}


void l1ct::pfalgo2hgc_ref_set_debug(int debug) { g_pfalgo2hgc_debug_ref_ = debug; }

void l1ct::pfalgo2hgc_ref(const l1ct::pfalgo_config &cfg, const l1ct::PFRegion & region, const l1ct::HadCaloObj calo[/*cfg.nCALO*/], const l1ct::TkObj track[/*cfg.nTRACK*/], const l1ct::MuObj mu[/*cfg.nMU*/], l1ct::PFChargedObj outch[/*cfg.nTRACK*/], l1ct::PFNeutralObj outne[/*cfg.nSELCALO*/], l1ct::PFChargedObj outmu[/*cfg.nMU*/]) {

    if (g_pfalgo2hgc_debug_ref_) {
        for (unsigned int i = 0; i < cfg.nTRACK; ++i) { if (track[i].hwPt == 0) continue;
            printf("FW  \t track %3d: pt %8.2f [ %8d ]  calo eta %+5.2f [ %+7d ]  calo phi %+5.2f [ %+7d ]  quality %d\n", 
                                i, track[i].floatPt(), track[i].intPt(), track[i].floatEta(), track[i].intEta(), track[i].floatPhi(), track[i].intPhi(), int(track[i].hwQuality));
        }
        for (unsigned int i = 0; i < cfg.nCALO; ++i) { if (calo[i].hwPt == 0) continue;
            printf("FW  \t calo  %3d: pt %8.2f [ %8d ]  calo eta %+5.2f [ %+7d ]  calo phi %+5.2f [ %+7d ]  calo emPt %8.2f [ %6d ]   isEM %d \n", 
                                i, calo[i].floatPt(), calo[i].intPt(), calo[i].floatEta(), calo[i].intEta(), calo[i].floatPhi(), calo[i].intPhi(), calo[i].floatEmPt(), calo[i].intEmPt(), int(calo[i].hwIsEM));
        } 
        for (unsigned int i = 0; i < cfg.nMU; ++i) { if (mu[i].hwPt == 0) continue;
            printf("FW  \t muon  %3d: pt %8.2f [ %8d ]  calo eta %+5.2f [ %+7d ]  calo phi %+5.2f [ %+7d ]   \n", 
                                i, mu[i].floatPt(), mu[i].intPt(), mu[i].floatEta(), mu[i].intEta(), mu[i].floatPhi(), mu[i].intPhi());
        } 
    }

    // constants
    const pt_t     TKPT_MAX_LOOSE = cfg.tk_MAXINVPT_LOOSE;
    const pt_t     TKPT_MAX_TIGHT = cfg.tk_MAXINVPT_TIGHT;
    const int      DR2MAX         = cfg.dR2MAX_TK_CALO;

    ////////////////////////////////////////////////////
    // TK-MU Linking
    std::unique_ptr<bool[]> isMu(new bool[cfg.nTRACK]);
    pfalgo_mu_ref(cfg, track, mu, &isMu[0], outmu, g_pfalgo2hgc_debug_ref_ );


    ////////////////////////////////////////////////////
    // TK-HAD Linking

    // initialize sum track pt
    std::vector<pt_t> calo_sumtk(cfg.nCALO), calo_subpt(cfg.nCALO);
    std::vector<pt2_t>  calo_sumtkErr2(cfg.nCALO);
    for (unsigned int ic = 0; ic < cfg.nCALO; ++ic) { calo_sumtk[ic] = 0;  calo_sumtkErr2[ic] = 0;}

    // initialize good track bit
    std::unique_ptr<bool[]> track_good(new bool[cfg.nTRACK]);
    std::unique_ptr<bool[]> isEle(new bool[cfg.nTRACK]);
    for (unsigned int it = 0; it < cfg.nTRACK; ++it) { 
        track_good[it] = (track[it].hwPt < ((track[it].hwQuality & TkObj::PFTIGHT) ? TKPT_MAX_TIGHT : TKPT_MAX_LOOSE) || isMu[it]); 
        isEle[it] = false;
    }

    // initialize output
    for (unsigned int ipf = 0; ipf < cfg.nTRACK; ++ipf) clear(outch[ipf]); 
    for (unsigned int ipf = 0; ipf < cfg.nSELCALO; ++ipf) clear(outne[ipf]);

    // for each track, find the closest calo
    for (unsigned int it = 0; it < cfg.nTRACK; ++it) {
        if (track[it].hwPt > 0 && !isMu[it]) {
            pt_t tkCaloPtErr = ptErr_ref(cfg, region, track[it]);
            int  ibest = best_match_with_pt_ref<HadCaloObj>(cfg.nCALO, DR2MAX, calo, track[it], tkCaloPtErr);
            if (ibest != -1) {
                if (g_pfalgo2hgc_debug_ref_) printf("FW  \t track  %3d pt %8.2f matched to calo %3d pt %8.2f\n", it, track[it].floatPt(), ibest, calo[ibest].floatPt());
                track_good[it] = 1;
                isEle[it] = calo[ibest].hwIsEM;
                calo_sumtk[ibest]    += track[it].hwPt;
                calo_sumtkErr2[ibest] += tkCaloPtErr*tkCaloPtErr;
            }
        }
    }

    for (unsigned int ic = 0; ic < cfg.nCALO; ++ic) {
        if (calo_sumtk[ic] > 0) {
            pt_t ptdiff = calo[ic].hwPt - calo_sumtk[ic];
            pt2_t sigmamult = calo_sumtkErr2[ic]; //  + (calo_sumtkErr2[ic] >> 1)); // this multiplies by 1.5 = sqrt(1.5)^2 ~ (1.2)^2
            if (g_pfalgo2hgc_debug_ref_ && (calo[ic].hwPt > 0)) {
                printf("FW  \t calo  %3d pt %8.2f [ %7d ] eta %+5.2f [ %+7d ] has a sum track pt %8.2f, difference %7.2f +- %.2f \n",
                            ic, calo[ic].floatPt(), calo[ic].intPt(), calo[ic].floatEta(), calo[ic].intEta(),  
                                Scales::floatPt(calo_sumtk[ic]), Scales::floatPt(ptdiff), std::sqrt(Scales::floatPt(calo_sumtkErr2[ic])));
                        
            }
            if (ptdiff > 0 && ptdiff*ptdiff > sigmamult) {
                calo_subpt[ic] = ptdiff;
            } else {
                calo_subpt[ic] = 0;
            }
        } else {
            calo_subpt[ic] = calo[ic].hwPt;
        }
        if (g_pfalgo2hgc_debug_ref_ && (calo[ic].hwPt > 0)) printf("FW  \t calo'  %3d pt %8.2f ---> %8.2f \n", ic, calo[ic].floatPt(), Scales::floatPt(calo_subpt[ic]));

    }

    // copy out charged hadrons
    for (unsigned int it = 0; it < cfg.nTRACK; ++it) {
        if (track[it].hwPt > 0 && track_good[it]) {
            assert(!(isEle[it] && isMu[it]));
            outch[it].hwPt = track[it].hwPt;
            outch[it].hwEta = track[it].hwEta;
            outch[it].hwPhi = track[it].hwPhi;
            outch[it].hwDEta = track[it].hwDEta;
            outch[it].hwDPhi = track[it].hwDPhi;
            outch[it].hwZ0 = track[it].hwZ0;
            outch[it].hwDxy = track[it].hwDxy;
            outch[it].hwTkQuality = track[it].hwQuality;
            if (isMu[it]) {
                outch[it].hwId = ParticleID::mkMuon(track[it].hwCharge);
            } else if (isEle[it]) {
                outch[it].hwId = ParticleID::mkElectron(track[it].hwCharge);
            } else {
                outch[it].hwId = ParticleID::mkChHad(track[it].hwCharge);
            }
        }
    }

    // copy out neutral hadrons with sorting and cropping
    std::vector<PFNeutralObj> outne_all(cfg.nCALO);
    for (unsigned int ipf = 0; ipf < cfg.nCALO; ++ipf) clear(outne_all[ipf]);
    for (unsigned int ic = 0; ic < cfg.nCALO; ++ic) {
        if (calo_subpt[ic] > 0) {
            outne_all[ic].hwPt  = calo_subpt[ic];
            outne_all[ic].hwEta = calo[ic].hwEta;
            outne_all[ic].hwPhi = calo[ic].hwPhi;
            outne_all[ic].hwId  = ParticleID(calo[ic].hwIsEM ? ParticleID::PHOTON : ParticleID::HADZERO);
            outne_all[ic].hwEmPt  = calo[ic].hwIsEM ? calo_subpt[ic] : pt_t(0); // FIXME
            outne_all[ic].hwEmID  = calo[ic].hwIsEM;
            outne_all[ic].hwPUID  = 0;
        }
    }


    if (cfg.nCALO == cfg.nSELCALO) {
        for (unsigned int ic = 0; ic < cfg.nCALO; ++ic) outne[ic] = outne_all[ic];
    } else {
        ptsort_ref(cfg.nCALO, cfg.nSELCALO, outne_all, outne);
    }

}
