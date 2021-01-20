#include "pfalgo3_ref.h"

#include <cmath>
#include <cstdio>
#include <algorithm>
#include <vector>
#include <memory>


int g_pfalgo3_debug_ref_ = 0;

void pfalgo3_ref_set_debug(int debug) { g_pfalgo3_debug_ref_ = debug; }

int tk_best_match_ref(unsigned int nCAL, unsigned int dR2MAX, const EmCaloObj calo[/*nCAL*/], const TkObj & track) {
    int  drmin = dR2MAX, ibest = -1;
    for (unsigned int ic = 0; ic < nCAL; ++ic) {
            if (calo[ic].hwPt <= 0) continue;
            int dr = dr2_int(track.hwEta, track.hwPhi, calo[ic].hwEta, calo[ic].hwPhi);
            if (dr < drmin) { drmin = dr; ibest = ic; }
    }
    return ibest;
}
int em_best_match_ref(unsigned int nCAL, unsigned int dR2MAX, const HadCaloObj calo[/*nCAL*/], const EmCaloObj & em) {
    pt_t emPtMin = em.hwPt >> 1;
    int  drmin = dR2MAX, ibest = -1;
    for (unsigned int ic = 0; ic < nCAL; ++ic) {
            if (calo[ic].hwEmPt <= emPtMin) continue;
            int dr = dr2_int(em.hwEta, em.hwPhi, calo[ic].hwEta, calo[ic].hwPhi);
            if (dr < drmin) { drmin = dr; ibest = ic; }
    }
    return ibest;
}


void pfalgo3_em_ref(const pfalgo3_config &cfg, const EmCaloObj emcalo[/*cfg.nEMCALO*/], const HadCaloObj hadcalo[/*cfg.nCALO*/], const TkObj track[/*cfg.nTRACK*/], const bool isMu[/*cfg.nTRACK*/], bool isEle[/*cfg.nTRACK*/], PFNeutralObj outpho[/*cfg.nPHOTON*/], HadCaloObj hadcalo_out[/*cfg.nCALO*/]) {
    // constants
    const int DR2MAX_TE = cfg.dR2MAX_TK_EM;
    const int DR2MAX_EH = cfg.dR2MAX_EM_CALO;

    // initialize sum track pt
    std::vector<pt_t> calo_sumtk(cfg.nEMCALO);
    for (unsigned int ic = 0; ic < cfg.nEMCALO; ++ic) {  calo_sumtk[ic] = 0; }
    std::vector<int> tk2em(cfg.nTRACK); 
    std::vector<bool> isEM(cfg.nEMCALO);
    // for each track, find the closest calo
    for (unsigned int it = 0; it < cfg.nTRACK; ++it) {
        if (track[it].hwPt > 0 && !isMu[it]) {
            tk2em[it] = tk_best_match_ref(cfg.nEMCALO, DR2MAX_TE, emcalo, track[it]);
            if (tk2em[it] != -1) {
                if (g_pfalgo3_debug_ref_) printf("FW  \t track  %3d pt %8.2f matched to em calo %3d pt %8.2f\n", it, track[it].floatPt(), tk2em[it], emcalo[tk2em[it]].floatPt());
                calo_sumtk[tk2em[it]] += track[it].hwPt;
            }
        } else {
        	tk2em[it] = -1;
        }
    }

    if (g_pfalgo3_debug_ref_) {
        for (unsigned int ic = 0; ic < cfg.nEMCALO; ++ic) {  if (emcalo[ic].hwPt > 0) printf("FW  \t emcalo %3d pt %8.2f has sumtk %8.2fd\n", ic, emcalo[ic].floatPt(), Scales::floatPt(calo_sumtk[ic])); }
    }

    for (unsigned int ic = 0; ic < cfg.nEMCALO; ++ic) {
        pt_t photonPt;
        if (calo_sumtk[ic] > 0) {
            dpt_t ptdiff = dpt_t(emcalo[ic].hwPt) - dpt_t(calo_sumtk[ic]);
            pt2_t sigma2 = emcalo[ic].hwPtErr*emcalo[ic].hwPtErr;
            pt2_t sigma2Lo = 4*sigma2, sigma2Hi = sigma2; // + (sigma2>>1); // cut at 1 sigma instead of old cut at sqrt(1.5) sigma's
            pt2_t ptdiff2 = ptdiff*ptdiff;
            if ((ptdiff >= 0 && ptdiff2 <= sigma2Hi) || (ptdiff < 0 && ptdiff2 < sigma2Lo)) {
                // electron
                photonPt = 0; 
                isEM[ic] = true;
                if (g_pfalgo3_debug_ref_) printf("FW  \t emcalo %3d pt %8.2f ptdiff %8.2f [match window: -%.2f / +%.2f] flagged as electron\n", ic, emcalo[ic].floatPt(), Scales::floatPt(ptdiff), std::sqrt(Scales::floatPt(sigma2Lo)), std::sqrt(float(sigma2Hi)));
            } else if (ptdiff > 0) {
                // electron + photon
                photonPt = ptdiff; 
                isEM[ic] = true;
                if (g_pfalgo3_debug_ref_) printf("FW  \t emcalo %3d pt %8.2f ptdiff %8.2f [match window: -%.2f / +%.2f] flagged as electron + photon of pt %8.2f\n", ic, emcalo[ic].floatPt(), Scales::floatPt(ptdiff), std::sqrt(Scales::floatPt(sigma2Lo)), std::sqrt(float(sigma2Hi)), int(photonPt));
            } else {
                // pion
                photonPt = 0;
                isEM[ic] = false;
                if (g_pfalgo3_debug_ref_) printf("FW  \t emcalo %3d pt %8.2f ptdiff %8.2f [match window: -%.2f / +%.2f] flagged as pion\n", ic, emcalo[ic].floatPt(), Scales::floatPt(ptdiff), std::sqrt(Scales::floatPt(sigma2Lo)), std::sqrt(Scales::floatPt(sigma2Hi)));
            }
        } else {
            // photon
            isEM[ic] = true;
            photonPt = emcalo[ic].hwPt;
            if (g_pfalgo3_debug_ref_ && emcalo[ic].hwPt > 0) printf("FW  \t emcalo %3d pt %8.2f flagged as photon\n", ic, emcalo[ic].floatPt());
        }
        outpho[ic].hwPt  = photonPt;
        outpho[ic].hwEta = photonPt ? emcalo[ic].hwEta : eta_t(0);
        outpho[ic].hwPhi = photonPt ? emcalo[ic].hwPhi : phi_t(0);
        outpho[ic].hwId  = ParticleID(photonPt ? ParticleID::PHOTON : ParticleID::HADZERO);
        outpho[ic].hwEmPt  = photonPt;
        outpho[ic].hwEmID  = photonPt ? 1 : 0;
        outpho[ic].hwPUID  = 0;

    }

    for (unsigned int it = 0; it < cfg.nTRACK; ++it) {
        isEle[it] = (tk2em[it] != -1) && isEM[tk2em[it]];
        if (g_pfalgo3_debug_ref_ && isEle[it]) printf("FW  \t track  %3d pt %8.2f flagged as electron.\n", it, track[it].floatPt());
    }

    std::vector<int> em2calo(cfg.nEMCALO);
    for (unsigned int ic = 0; ic < cfg.nEMCALO; ++ic) {
        em2calo[ic] = em_best_match_ref(cfg.nCALO, DR2MAX_EH, hadcalo, emcalo[ic]);
        if (g_pfalgo3_debug_ref_ && (emcalo[ic].hwPt > 0)) {
             printf("FW  \t emcalo %3d pt %8.2f isEM %d matched to hadcalo %3d pt %8.2f emPt %8.2f isEM %d\n", 
                                ic, emcalo[ic].floatPt(), int(isEM[ic]), em2calo[ic], (em2calo[ic] >= 0 ? hadcalo[em2calo[ic]].floatPt() : -1), 
                                (em2calo[ic] >= 0 ? hadcalo[em2calo[ic]].floatEmPt() : -1), (em2calo[ic] >= 0 ? int(hadcalo[em2calo[ic]].hwIsEM) : 0));
        }
    }
    
    for (unsigned int ih = 0; ih < cfg.nCALO; ++ih) {
        hadcalo_out[ih] = hadcalo[ih];
        dpt_t sub = 0; bool keep = false;
        for (unsigned int ic = 0; ic < cfg.nEMCALO; ++ic) {
            if (em2calo[ic] == int(ih)) {
                if (isEM[ic]) sub += emcalo[ic].hwPt;
                else keep = true;
            }
        }
        dpt_t emdiff  = dpt_t(hadcalo[ih].hwEmPt) - sub; // ok to saturate at zero here
        dpt_t alldiff = dpt_t(hadcalo[ih].hwPt) - sub;
        if (g_pfalgo3_debug_ref_ && (hadcalo[ih].hwPt > 0)) {
            printf("FW  \t calo   %3d pt %8.2f has a subtracted pt of %8.2f, empt %8.2f -> %8.2f   isem %d mustkeep %d \n",
                        ih, hadcalo[ih].floatPt(), Scales::floatPt(alldiff), hadcalo[ih].floatEmPt(), Scales::floatPt(emdiff), int(hadcalo[ih].hwIsEM), keep);
                    
        }
        if (alldiff <= ( hadcalo[ih].hwPt >>  4 ) ) {
            hadcalo_out[ih].hwPt = 0;   // kill
            hadcalo_out[ih].hwEmPt = 0; // kill
            if (g_pfalgo3_debug_ref_ && (hadcalo[ih].hwPt > 0)) printf("FW  \t calo   %3d pt %8.2f --> discarded (zero pt)\n", ih, hadcalo[ih].floatPt());
        } else if ((hadcalo[ih].hwIsEM && emdiff <= ( hadcalo[ih].hwEmPt >> 3 )) && !keep) {
            hadcalo_out[ih].hwPt = 0;   // kill
            hadcalo_out[ih].hwEmPt = 0; // kill
            if (g_pfalgo3_debug_ref_ && (hadcalo[ih].hwPt > 0)) printf("FW  \t calo   %3d pt %8.2f --> discarded (zero em)\n", ih, hadcalo[ih].floatPt());
        } else {
            hadcalo_out[ih].hwPt   = alldiff;   
            hadcalo_out[ih].hwEmPt = (emdiff > 0 ? pt_t(emdiff) : pt_t(0)); 
        }
    }
}

void pfalgo3_ref(const pfalgo3_config &cfg, const PFRegion & region, const EmCaloObj emcalo[/*cfg.nEMCALO*/], const HadCaloObj hadcalo[/*cfg.nCALO*/], const TkObj track[/*cfg.nTRACK*/], const MuObj mu[/*cfg.nMU*/], PFChargedObj outch[/*cfg.nTRACK*/], PFNeutralObj outpho[/*cfg.nPHOTON*/], PFNeutralObj outne[/*cfg.nSELCALO*/], PFChargedObj outmu[/*cfg.nMU*/]) {

    if (g_pfalgo3_debug_ref_) {
        for (unsigned int i = 0; i < cfg.nTRACK; ++i) { if (track[i].hwPt == 0) continue;
            printf("FW  \t track %3d: pt %8.2f [ %8d ]  calo eta %+5.2f [ %+7d ]  calo phi %+5.2f [ %+7d ]  quality %d\n", 
                                i, track[i].floatPt(), track[i].intPt(), track[i].floatEta(), track[i].intEta(), track[i].floatPhi(), track[i].intPhi(), int(track[i].hwQuality));
        }
        for (unsigned int i = 0; i < cfg.nEMCALO; ++i) { if (emcalo[i].hwPt == 0) continue;
            printf("FW  \t EM    %3d: pt %8.2f [ %8d ]  calo eta %+5.2f [ %+7d ]  calo phi %+5.2f [ %+7d ]  calo ptErr %8.2f [ %6d ] \n", 
                                i, emcalo[i].floatPt(), emcalo[i].intPt(), emcalo[i].floatEta(), emcalo[i].intEta(), emcalo[i].floatPhi(), emcalo[i].intPhi(), emcalo[i].floatPtErr(), emcalo[i].intPtErr());
        } 
        for (unsigned int i = 0; i < cfg.nCALO; ++i) { if (hadcalo[i].hwPt == 0) continue;
            printf("FW  \t calo  %3d: pt %8.2f [ %8d ]  calo eta %+5.2f [ %+7d ]  calo phi %+5.2f [ %+7d ]  calo emPt %8.2f [ %6d ]   isEM %d \n", 
                                i, hadcalo[i].floatPt(), hadcalo[i].intPt(), hadcalo[i].floatEta(), hadcalo[i].intEta(), hadcalo[i].floatPhi(), hadcalo[i].intPhi(), hadcalo[i].floatEmPt(), hadcalo[i].intEmPt(), int(hadcalo[i].hwIsEM));
        } 
        for (unsigned int i = 0; i < cfg.nMU; ++i) { if (mu[i].hwPt == 0) continue;
            printf("FW  \t muon  %3d: pt %8.2f [ %8d ]  calo eta %+5.2f [ %+7d ]  calo phi %+5.2f [ %+7d ]   \n", 
                                i, mu[i].floatPt(), mu[i].intPt(), mu[i].floatEta(), mu[i].intEta(), mu[i].floatPhi(), mu[i].intPhi());
        } 
    }

    // constants
    const pt_t TKPT_MAX_LOOSE = cfg.tk_MAXINVPT_LOOSE; 
    const pt_t TKPT_MAX_TIGHT = cfg.tk_MAXINVPT_TIGHT; 
    const int  DR2MAX = cfg.dR2MAX_TK_CALO;

    ////////////////////////////////////////////////////
    // TK-MU Linking
    // // we can't use std::vector here because it's specialized
    std::unique_ptr<bool[]> isMu(new bool[cfg.nTRACK]);
    pfalgo_mu_ref(cfg, track, mu, &isMu[0], outmu, g_pfalgo3_debug_ref_);

    ////////////////////////////////////////////////////
    // TK-EM Linking
    std::unique_ptr<bool[]> isEle(new bool[cfg.nTRACK]);
    std::vector<HadCaloObj> hadcalo_subem(cfg.nCALO);
    pfalgo3_em_ref(cfg, emcalo, hadcalo, track, &isMu[0], &isEle[0], outpho, &hadcalo_subem[0]);

    ////////////////////////////////////////////////////
    // TK-HAD Linking

    // initialize sum track pt
    std::vector<pt_t> calo_sumtk(cfg.nCALO), calo_subpt(cfg.nCALO);
    std::vector<pt2_t>  calo_sumtkErr2(cfg.nCALO);
    for (unsigned int ic = 0; ic < cfg.nCALO; ++ic) { calo_sumtk[ic] = 0;  calo_sumtkErr2[ic] = 0;}

    // initialize good track bit
    std::unique_ptr<bool[]>  track_good(new bool[cfg.nTRACK]);
    for (unsigned int it = 0; it < cfg.nTRACK; ++it) { 
        track_good[it] = (track[it].hwPt < ((track[it].hwQuality & TkObj::PFTIGHT) ? TKPT_MAX_TIGHT : TKPT_MAX_LOOSE) || isEle[it] || isMu[it]); 
    }

    // initialize output
    for (unsigned int ipf = 0; ipf < cfg.nTRACK; ++ipf) { clear(outch[ipf]); }
    for (unsigned int ipf = 0; ipf < cfg.nSELCALO; ++ipf) { clear(outne[ipf]); }

    // for each track, find the closest calo
    for (unsigned int it = 0; it < cfg.nTRACK; ++it) {
        if (track[it].hwPt > 0 && !isEle[it] && !isMu[it]) {
            pt_t tkCaloPtErr = ptErr_ref(cfg, region, track[it]);
            int  ibest = best_match_with_pt_ref<HadCaloObj>(cfg.nCALO, DR2MAX, &hadcalo_subem[0], track[it], tkCaloPtErr);
            if (ibest != -1) {
                if (g_pfalgo3_debug_ref_) printf("FW  \t track  %3d pt %8.2f matched to calo %3d pt %8.2f\n", it, track[it].floatPt(), ibest, hadcalo_subem[ibest].floatPt());
                track_good[it] = 1;
                calo_sumtk[ibest]    += track[it].hwPt;
                calo_sumtkErr2[ibest] += tkCaloPtErr*tkCaloPtErr;
            }
        }
    }

    for (unsigned int ic = 0; ic < cfg.nCALO; ++ic) {
        if (calo_sumtk[ic] > 0) {
            dpt_t ptdiff = dpt_t(hadcalo_subem[ic].hwPt) - dpt_t(calo_sumtk[ic]); 
            pt2_t sigmamult = calo_sumtkErr2[ic]; // before we did (calo_sumtkErr2[ic] + (calo_sumtkErr2[ic] >> 1)); to multiply by 1.5 = sqrt(1.5)^2 ~ (1.2)^2
            if (g_pfalgo3_debug_ref_ && (hadcalo_subem[ic].hwPt > 0)) {
                printf("FW  \t calo  %3d pt %8.2f [ %7d ] eta %+5.2f [ %+7d ] has a sum track pt %8.2f, difference %7.2f +- %.2f \n",
                            ic, hadcalo_subem[ic].floatPt(), hadcalo_subem[ic].intPt(), hadcalo_subem[ic].floatEta(), hadcalo_subem[ic].intEta(),  
                                Scales::floatPt(calo_sumtk[ic]), Scales::floatPt(ptdiff), std::sqrt(Scales::floatPt(calo_sumtkErr2[ic])));
                        
            }
            if (ptdiff > 0 && ptdiff*ptdiff > sigmamult) {
                calo_subpt[ic] = pt_t(ptdiff);
            } else {
                calo_subpt[ic] = 0;
            }
        } else {
            calo_subpt[ic] = hadcalo_subem[ic].hwPt;
        }
        if (g_pfalgo3_debug_ref_ && (hadcalo_subem[ic].hwPt > 0)) printf("FW  \t calo  %3d pt %8.2f ---> %8.2f \n", ic, hadcalo_subem[ic].floatPt(), Scales::floatPt(calo_subpt[ic]));
    }

    // copy out charged hadrons
    for (unsigned int it = 0; it < cfg.nTRACK; ++it) {
        if (track_good[it]) {
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

    // copy out neutral hadrons
    std::vector<PFNeutralObj> outne_all(cfg.nCALO);
    for (unsigned int ipf = 0; ipf < cfg.nCALO; ++ipf) { clear(outne_all[ipf]); }
    for (unsigned int ic = 0; ic < cfg.nCALO; ++ic) {
        if (calo_subpt[ic] > 0) {
            outne_all[ic].hwPt  = calo_subpt[ic];
            outne_all[ic].hwEta = hadcalo_subem[ic].hwEta;
            outne_all[ic].hwPhi = hadcalo_subem[ic].hwPhi;
            //outne_all[ic].hwId  = ParticleID(hadcalo_subem[ic].hwIsEM ? ParticleID::PHOTON : ParticleID::HADZERO);
            outne_all[ic].hwId  = ParticleID::HADZERO;
            outne_all[ic].hwEmPt  = hadcalo_subem[ic].hwIsEM ? calo_subpt[ic] : pt_t(0); // FIXME
            outne_all[ic].hwEmID  = hadcalo_subem[ic].hwIsEM;
            outne_all[ic].hwPUID  = 0;
        }
    }

    ptsort_ref(cfg.nCALO, cfg.nSELCALO, outne_all, outne);

    if (g_pfalgo3_debug_ref_) {
        for (unsigned int i = 0; i < cfg.nTRACK; ++i) { if (outch[i].hwPt == 0) continue;
            printf("FW  \t outch %3d: pt %8.2f [ %8d ]  calo eta %+5.2f [ %+7d ]  calo phi %+5.2f [ %+7d ]  pid %d\n", 
                                i, outch[i].floatPt(), outch[i].intPt(), outch[i].floatEta(), outch[i].intEta(), outch[i].floatPhi(), outch[i].intPhi(), outch[i].intId());
        }
        for (unsigned int i = 0; i < cfg.nPHOTON; ++i) { if (outpho[i].hwPt == 0) continue;
            printf("FW  \t outph %3d: pt %8.2f [ %8d ]  calo eta %+5.2f [ %+7d ]  calo phi %+5.2f [ %+7d ]  pid %d\n", 
                                i, outpho[i].floatPt(), outpho[i].intPt(), outpho[i].floatEta(), outpho[i].intEta(), outpho[i].floatPhi(), outpho[i].intPhi(), outpho[i].intId());
        }
        for (unsigned int i = 0; i < cfg.nSELCALO; ++i) { if (outne[i].hwPt == 0) continue;
            printf("FW  \t outne %3d: pt %8.2f [ %8d ]  calo eta %+5.2f [ %+7d ]  calo phi %+5.2f [ %+7d ]  pid %d\n", 
                                i, outne[i].floatPt(), outne[i].intPt(), outne[i].floatEta(), outne[i].intEta(), outne[i].floatPhi(), outne[i].intPhi(), outne[i].intId());
        }
    }


}

void pfalgo3_merge_neutrals_ref(const pfalgo3_config &cfg, const PFNeutralObj pho[/*cfg.nPHOTON*/], const PFNeutralObj ne[/*cfg.nSELCALO*/], PFNeutralObj allne[/*cfg.nALLNEUTRALS*/]) 
{
    int j = 0;
    for (unsigned int i = 0; i < cfg.nPHOTON;  ++i, ++j) allne[j] = pho[i];
    for (unsigned int i = 0; i < cfg.nSELCALO; ++i, ++j) allne[j] = ne[i];
}

