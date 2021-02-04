#include "pfalgo3_ref.h"

#include <cmath>
#include <cstdio>
#include <algorithm>
#include <vector>
#include <memory>

int l1ct::PFAlgo3Emulator::tk_best_match_ref(unsigned int nCAL, unsigned int dR2MAX, const l1ct::EmCaloObj calo[/*nCAL*/], const l1ct::TkObj & track) const {
    int  drmin = dR2MAX, ibest = -1;
    for (unsigned int ic = 0; ic < nCAL; ++ic) {
            if (calo[ic].hwPt <= 0) continue;
            int dr = dr2_int(track.hwEta, track.hwPhi, calo[ic].hwEta, calo[ic].hwPhi);
            if (dr < drmin) { drmin = dr; ibest = ic; }
    }
    return ibest;
}
int l1ct::PFAlgo3Emulator::em_best_match_ref(unsigned int nCAL, unsigned int dR2MAX, const l1ct::HadCaloObj calo[/*nCAL*/], const l1ct::EmCaloObj & em) const {
    pt_t emPtMin = em.hwPt >> 1;
    int  drmin = dR2MAX, ibest = -1;
    for (unsigned int ic = 0; ic < nCAL; ++ic) {
            if (calo[ic].hwEmPt <= emPtMin) continue;
            int dr = dr2_int(em.hwEta, em.hwPhi, calo[ic].hwEta, calo[ic].hwPhi);
            if (dr < drmin) { drmin = dr; ibest = ic; }
    }
    return ibest;
}


void l1ct::PFAlgo3Emulator::pfalgo3_em_ref(const l1ct::EmCaloObj emcalo[/*nEMCALO_*/], const l1ct::HadCaloObj hadcalo[/*nCALO_*/], const l1ct::TkObj track[/*nTRACK_*/], const bool isMu[/*nTRACK_*/], bool isEle[/*nTRACK_*/], l1ct::PFNeutralObj outpho[/*nPHOTON_*/], l1ct::HadCaloObj hadcalo_out[/*nCALO_*/]) const {
    // constants
    const int DR2MAX_TE = dR2MAX_TK_EM_;
    const int DR2MAX_EH = dR2MAX_EM_CALO_;

    // initialize sum track pt
    std::vector<pt_t> calo_sumtk(nEMCALO_);
    for (unsigned int ic = 0; ic < nEMCALO_; ++ic) {  calo_sumtk[ic] = 0; }
    std::vector<int> tk2em(nTRACK_); 
    std::vector<bool> isEM(nEMCALO_);
    // for each track, find the closest calo
    for (unsigned int it = 0; it < nTRACK_; ++it) {
        if (track[it].hwPt > 0 && !isMu[it]) {
            tk2em[it] = tk_best_match_ref(nEMCALO_, DR2MAX_TE, emcalo, track[it]);
            if (tk2em[it] != -1) {
                if (debug_) printf("FW  \t track  %3d pt %8.2f matched to em calo %3d pt %8.2f\n", it, track[it].floatPt(), tk2em[it], emcalo[tk2em[it]].floatPt());
                calo_sumtk[tk2em[it]] += track[it].hwPt;
            }
        } else {
        	tk2em[it] = -1;
        }
    }

    if (debug_) {
        for (unsigned int ic = 0; ic < nEMCALO_; ++ic) {  if (emcalo[ic].hwPt > 0) printf("FW  \t emcalo %3d pt %8.2f has sumtk %8.2fd\n", ic, emcalo[ic].floatPt(), Scales::floatPt(calo_sumtk[ic])); }
    }

    for (unsigned int ic = 0; ic < nEMCALO_; ++ic) {
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
                if (debug_) printf("FW  \t emcalo %3d pt %8.2f ptdiff %8.2f [match window: -%.2f / +%.2f] flagged as electron\n", ic, emcalo[ic].floatPt(), Scales::floatPt(ptdiff), std::sqrt(Scales::floatPt(sigma2Lo)), std::sqrt(float(sigma2Hi)));
            } else if (ptdiff > 0) {
                // electron + photon
                photonPt = ptdiff; 
                isEM[ic] = true;
                if (debug_) printf("FW  \t emcalo %3d pt %8.2f ptdiff %8.2f [match window: -%.2f / +%.2f] flagged as electron + photon of pt %8.2f\n", ic, emcalo[ic].floatPt(), Scales::floatPt(ptdiff), std::sqrt(Scales::floatPt(sigma2Lo)), std::sqrt(float(sigma2Hi)), int(photonPt));
            } else {
                // pion
                photonPt = 0;
                isEM[ic] = false;
                if (debug_) printf("FW  \t emcalo %3d pt %8.2f ptdiff %8.2f [match window: -%.2f / +%.2f] flagged as pion\n", ic, emcalo[ic].floatPt(), Scales::floatPt(ptdiff), std::sqrt(Scales::floatPt(sigma2Lo)), std::sqrt(Scales::floatPt(sigma2Hi)));
            }
        } else {
            // photon
            isEM[ic] = true;
            photonPt = emcalo[ic].hwPt;
            if (debug_ && emcalo[ic].hwPt > 0) printf("FW  \t emcalo %3d pt %8.2f flagged as photon\n", ic, emcalo[ic].floatPt());
        }
        outpho[ic].hwPt  = photonPt;
        outpho[ic].hwEta = photonPt ? emcalo[ic].hwEta : eta_t(0);
        outpho[ic].hwPhi = photonPt ? emcalo[ic].hwPhi : phi_t(0);
        outpho[ic].hwId  = ParticleID(photonPt ? ParticleID::PHOTON : ParticleID::HADZERO);
        outpho[ic].hwEmPt  = photonPt;
        outpho[ic].hwEmID  = photonPt ? 1 : 0;
        outpho[ic].hwPUID  = 0;

    }

    for (unsigned int it = 0; it < nTRACK_; ++it) {
        isEle[it] = (tk2em[it] != -1) && isEM[tk2em[it]];
        if (debug_ && isEle[it]) printf("FW  \t track  %3d pt %8.2f flagged as electron.\n", it, track[it].floatPt());
    }

    std::vector<int> em2calo(nEMCALO_);
    for (unsigned int ic = 0; ic < nEMCALO_; ++ic) {
        em2calo[ic] = em_best_match_ref(nCALO_, DR2MAX_EH, hadcalo, emcalo[ic]);
        if (debug_ && (emcalo[ic].hwPt > 0)) {
             printf("FW  \t emcalo %3d pt %8.2f isEM %d matched to hadcalo %3d pt %8.2f emPt %8.2f isEM %d\n", 
                                ic, emcalo[ic].floatPt(), int(isEM[ic]), em2calo[ic], (em2calo[ic] >= 0 ? hadcalo[em2calo[ic]].floatPt() : -1), 
                                (em2calo[ic] >= 0 ? hadcalo[em2calo[ic]].floatEmPt() : -1), (em2calo[ic] >= 0 ? int(hadcalo[em2calo[ic]].hwIsEM) : 0));
        }
    }
    
    for (unsigned int ih = 0; ih < nCALO_; ++ih) {
        hadcalo_out[ih] = hadcalo[ih];
        dpt_t sub = 0; bool keep = false;
        for (unsigned int ic = 0; ic < nEMCALO_; ++ic) {
            if (em2calo[ic] == int(ih)) {
                if (isEM[ic]) sub += emcalo[ic].hwPt;
                else keep = true;
            }
        }
        dpt_t emdiff  = dpt_t(hadcalo[ih].hwEmPt) - sub; // ok to saturate at zero here
        dpt_t alldiff = dpt_t(hadcalo[ih].hwPt) - sub;
        if (debug_ && (hadcalo[ih].hwPt > 0)) {
            printf("FW  \t calo   %3d pt %8.2f has a subtracted pt of %8.2f, empt %8.2f -> %8.2f   isem %d mustkeep %d \n",
                        ih, hadcalo[ih].floatPt(), Scales::floatPt(alldiff), hadcalo[ih].floatEmPt(), Scales::floatPt(emdiff), int(hadcalo[ih].hwIsEM), keep);
                    
        }
        if (alldiff <= ( hadcalo[ih].hwPt >>  4 ) ) {
            hadcalo_out[ih].hwPt = 0;   // kill
            hadcalo_out[ih].hwEmPt = 0; // kill
            if (debug_ && (hadcalo[ih].hwPt > 0)) printf("FW  \t calo   %3d pt %8.2f --> discarded (zero pt)\n", ih, hadcalo[ih].floatPt());
        } else if ((hadcalo[ih].hwIsEM && emdiff <= ( hadcalo[ih].hwEmPt >> 3 )) && !keep) {
            hadcalo_out[ih].hwPt = 0;   // kill
            hadcalo_out[ih].hwEmPt = 0; // kill
            if (debug_ && (hadcalo[ih].hwPt > 0)) printf("FW  \t calo   %3d pt %8.2f --> discarded (zero em)\n", ih, hadcalo[ih].floatPt());
        } else {
            hadcalo_out[ih].hwPt   = alldiff;   
            hadcalo_out[ih].hwEmPt = (emdiff > 0 ? pt_t(emdiff) : pt_t(0)); 
        }
    }
}

void l1ct::PFAlgo3Emulator::pfalgo3_ref(const l1ct::PFRegion & region, const l1ct::EmCaloObj emcalo[/*nEMCALO_*/], const l1ct::HadCaloObj hadcalo[/*nCALO_*/], const l1ct::TkObj track[/*nTRACK_*/], const l1ct::MuObj mu[/*nMU_*/], l1ct::PFChargedObj outch[/*nTRACK_*/], l1ct::PFNeutralObj outpho[/*nPHOTON_*/], l1ct::PFNeutralObj outne[/*nSELCALO_*/], l1ct::PFChargedObj outmu[/*nMU_*/]) const {

    if (debug_) {
        for (unsigned int i = 0; i < nTRACK_; ++i) { if (track[i].hwPt == 0) continue;
            printf("FW  \t track %3d: pt %8.2f [ %8d ]  calo eta %+5.2f [ %+7d ]  calo phi %+5.2f [ %+7d ]  quality %d\n", 
                                i, track[i].floatPt(), track[i].intPt(), track[i].floatEta(), track[i].intEta(), track[i].floatPhi(), track[i].intPhi(), int(track[i].hwQuality));
        }
        for (unsigned int i = 0; i < nEMCALO_; ++i) { if (emcalo[i].hwPt == 0) continue;
            printf("FW  \t EM    %3d: pt %8.2f [ %8d ]  calo eta %+5.2f [ %+7d ]  calo phi %+5.2f [ %+7d ]  calo ptErr %8.2f [ %6d ] \n", 
                                i, emcalo[i].floatPt(), emcalo[i].intPt(), emcalo[i].floatEta(), emcalo[i].intEta(), emcalo[i].floatPhi(), emcalo[i].intPhi(), emcalo[i].floatPtErr(), emcalo[i].intPtErr());
        } 
        for (unsigned int i = 0; i < nCALO_; ++i) { if (hadcalo[i].hwPt == 0) continue;
            printf("FW  \t calo  %3d: pt %8.2f [ %8d ]  calo eta %+5.2f [ %+7d ]  calo phi %+5.2f [ %+7d ]  calo emPt %8.2f [ %6d ]   isEM %d \n", 
                                i, hadcalo[i].floatPt(), hadcalo[i].intPt(), hadcalo[i].floatEta(), hadcalo[i].intEta(), hadcalo[i].floatPhi(), hadcalo[i].intPhi(), hadcalo[i].floatEmPt(), hadcalo[i].intEmPt(), int(hadcalo[i].hwIsEM));
        } 
        for (unsigned int i = 0; i < nMU_; ++i) { if (mu[i].hwPt == 0) continue;
            printf("FW  \t muon  %3d: pt %8.2f [ %8d ]  calo eta %+5.2f [ %+7d ]  calo phi %+5.2f [ %+7d ]   \n", 
                                i, mu[i].floatPt(), mu[i].intPt(), mu[i].floatEta(), mu[i].intEta(), mu[i].floatPhi(), mu[i].intPhi());
        } 
    }

    // constants
    const pt_t TKPT_MAX_LOOSE = tk_MAXINVPT_LOOSE_; 
    const pt_t TKPT_MAX_TIGHT = tk_MAXINVPT_TIGHT_; 
    const int  DR2MAX = dR2MAX_TK_CALO_;

    ////////////////////////////////////////////////////
    // TK-MU Linking
    // // we can't use std::vector here because it's specialized
    std::unique_ptr<bool[]> isMu(new bool[nTRACK_]);
    pfalgo_mu_ref(track, mu, &isMu[0], outmu);

    ////////////////////////////////////////////////////
    // TK-EM Linking
    std::unique_ptr<bool[]> isEle(new bool[nTRACK_]);
    std::vector<HadCaloObj> hadcalo_subem(nCALO_);
    pfalgo3_em_ref(emcalo, hadcalo, track, &isMu[0], &isEle[0], outpho, &hadcalo_subem[0]);

    ////////////////////////////////////////////////////
    // TK-HAD Linking

    // initialize sum track pt
    std::vector<pt_t> calo_sumtk(nCALO_), calo_subpt(nCALO_);
    std::vector<pt2_t>  calo_sumtkErr2(nCALO_);
    for (unsigned int ic = 0; ic < nCALO_; ++ic) { calo_sumtk[ic] = 0;  calo_sumtkErr2[ic] = 0;}

    // initialize good track bit
    std::unique_ptr<bool[]>  track_good(new bool[nTRACK_]);
    for (unsigned int it = 0; it < nTRACK_; ++it) { 
        track_good[it] = (track[it].hwPt < ((track[it].hwQuality & TkObj::PFTIGHT) ? TKPT_MAX_TIGHT : TKPT_MAX_LOOSE) || isEle[it] || isMu[it]); 
    }

    // initialize output
    for (unsigned int ipf = 0; ipf < nTRACK_; ++ipf) { clear(outch[ipf]); }
    for (unsigned int ipf = 0; ipf < nSELCALO_; ++ipf) { clear(outne[ipf]); }

    // for each track, find the closest calo
    for (unsigned int it = 0; it < nTRACK_; ++it) {
        if (track[it].hwPt > 0 && !isEle[it] && !isMu[it]) {
            pt_t tkCaloPtErr = ptErr_ref(region, track[it]);
            int  ibest = best_match_with_pt_ref<HadCaloObj>(nCALO_, DR2MAX, &hadcalo_subem[0], track[it], tkCaloPtErr);
            if (ibest != -1) {
                if (debug_) printf("FW  \t track  %3d pt %8.2f matched to calo %3d pt %8.2f\n", it, track[it].floatPt(), ibest, hadcalo_subem[ibest].floatPt());
                track_good[it] = 1;
                calo_sumtk[ibest]    += track[it].hwPt;
                calo_sumtkErr2[ibest] += tkCaloPtErr*tkCaloPtErr;
            }
        }
    }

    for (unsigned int ic = 0; ic < nCALO_; ++ic) {
        if (calo_sumtk[ic] > 0) {
            dpt_t ptdiff = dpt_t(hadcalo_subem[ic].hwPt) - dpt_t(calo_sumtk[ic]); 
            pt2_t sigmamult = calo_sumtkErr2[ic]; // before we did (calo_sumtkErr2[ic] + (calo_sumtkErr2[ic] >> 1)); to multiply by 1.5 = sqrt(1.5)^2 ~ (1.2)^2
            if (debug_ && (hadcalo_subem[ic].hwPt > 0)) {
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
        if (debug_ && (hadcalo_subem[ic].hwPt > 0)) printf("FW  \t calo  %3d pt %8.2f ---> %8.2f \n", ic, hadcalo_subem[ic].floatPt(), Scales::floatPt(calo_subpt[ic]));
    }

    // copy out charged hadrons
    for (unsigned int it = 0; it < nTRACK_; ++it) {
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
    std::vector<PFNeutralObj> outne_all(nCALO_);
    for (unsigned int ipf = 0; ipf < nCALO_; ++ipf) { clear(outne_all[ipf]); }
    for (unsigned int ic = 0; ic < nCALO_; ++ic) {
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

    ptsort_ref(nCALO_, nSELCALO_, outne_all, outne);

    if (debug_) {
        for (unsigned int i = 0; i < nTRACK_; ++i) { if (outch[i].hwPt == 0) continue;
            printf("FW  \t outch %3d: pt %8.2f [ %8d ]  calo eta %+5.2f [ %+7d ]  calo phi %+5.2f [ %+7d ]  pid %d\n", 
                                i, outch[i].floatPt(), outch[i].intPt(), outch[i].floatEta(), outch[i].intEta(), outch[i].floatPhi(), outch[i].intPhi(), outch[i].intId());
        }
        for (unsigned int i = 0; i < nPHOTON_; ++i) { if (outpho[i].hwPt == 0) continue;
            printf("FW  \t outph %3d: pt %8.2f [ %8d ]  calo eta %+5.2f [ %+7d ]  calo phi %+5.2f [ %+7d ]  pid %d\n", 
                                i, outpho[i].floatPt(), outpho[i].intPt(), outpho[i].floatEta(), outpho[i].intEta(), outpho[i].floatPhi(), outpho[i].intPhi(), outpho[i].intId());
        }
        for (unsigned int i = 0; i < nSELCALO_; ++i) { if (outne[i].hwPt == 0) continue;
            printf("FW  \t outne %3d: pt %8.2f [ %8d ]  calo eta %+5.2f [ %+7d ]  calo phi %+5.2f [ %+7d ]  pid %d\n", 
                                i, outne[i].floatPt(), outne[i].intPt(), outne[i].floatEta(), outne[i].intEta(), outne[i].floatPhi(), outne[i].intPhi(), outne[i].intId());
        }
    }


}

void l1ct::PFAlgo3Emulator::pfalgo3_merge_neutrals_ref(const l1ct::PFNeutralObj pho[/*nPHOTON_*/], const l1ct::PFNeutralObj ne[/*nSELCALO_*/], l1ct::PFNeutralObj allne[/*nALLNEUTRALS_*/]) const
{
    int j = 0;
    for (unsigned int i = 0; i < nPHOTON_;  ++i, ++j) allne[j] = pho[i];
    for (unsigned int i = 0; i < nSELCALO_; ++i, ++j) allne[j] = ne[i];
}

