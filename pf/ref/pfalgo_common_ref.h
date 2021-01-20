#ifndef PFALGO_COMMON_REF_H
#define PFALGO_COMMON_REF_H

#include "../../dataformats/pf.h"
#include "../firmware/pfalgo_common.h"
#include <algorithm>
#include <vector>

template <typename T> inline int sqr(const T & t) { return t*t; }

template<typename CO_t>
int best_match_with_pt_ref(int nCAL, int dR2MAX, const CO_t calo[/*nCAL*/], const TkObj & track) ;
 
template<typename T>
void ptsort_ref(int nIn, int nOut, const T in[/*nIn*/], T out[/*nOut*/]) ;

struct pfalgo_config {
    unsigned int nTRACK, nCALO, nMU;
    unsigned int nSELCALO;
    unsigned int dR2MAX_TK_MU;
    unsigned int dR2MAX_TK_CALO;
    pt_t tk_MAXINVPT_LOOSE, tk_MAXINVPT_TIGHT;

    struct ptErrBin { glbeta_t abseta; ptErrScale_t scale; ptErrOffs_t offs; };
    std::vector<ptErrBin> ptErrBins;
    
    pfalgo_config(unsigned int nTrack, unsigned int nCalo, unsigned int nMu, unsigned int nSelCalo, 
                  unsigned int dR2Max_Tk_Mu, unsigned int dR2Max_Tk_Calo, 
                  pt_t tk_MaxInvPt_Loose, pt_t tk_MaxInvPt_Tight) :
        nTRACK(nTrack), nCALO(nCalo), nMU(nMu), nSELCALO(nSelCalo), dR2MAX_TK_MU(dR2Max_Tk_Mu), dR2MAX_TK_CALO(dR2Max_Tk_Calo), tk_MAXINVPT_LOOSE(tk_MaxInvPt_Loose), tk_MAXINVPT_TIGHT(tk_MaxInvPt_Tight) {}

    void loadPtErrBins(unsigned int nbins, const float absetas[], const float scales[], const float offs[]) ;
};

pt_t ptErr_ref(const pfalgo_config & cfg, const PFRegion & region, const TkObj & track) ;

void pfalgo_mu_ref(const pfalgo_config &cfg, const TkObj track[/*cfg.nTRACK*/], const MuObj mu[/*cfg.nMU*/], bool isMu[/*cfg.nTRACK*/], PFChargedObj outmu[/*cfg.nMU*/], bool debug) ;

//=== begin implementation part

template<typename CO_t>
int best_match_with_pt_ref(int nCAL, int dR2MAX, const CO_t calo[/*nCAL*/], const TkObj & track, const pt_t & trackCaloPtErr) {
    pt_t caloPtMin = track.hwPt - 2*trackCaloPtErr;
    if (caloPtMin < 0) caloPtMin = 0;
    float ptErr = std::max<float>(Scales::INTPT_LSB, Scales::floatPt(trackCaloPtErr));
    ptscale_t dptscale = float(dR2MAX)/(ptErr*ptErr);
    int dr2min = 0, ibest = -1;
    for (int ic = 0; ic < nCAL; ++ic) {
            if (calo[ic].hwPt <= caloPtMin) continue;
            int dr2 = dr2_int(track.hwEta, track.hwPhi, calo[ic].hwEta, calo[ic].hwPhi);
            if (dr2 >= dR2MAX) continue;
            pt_t dpt = track.hwPt - calo[ic].hwPt;
            dr2 += int((dpt*dpt)*dptscale);
            if (ibest == -1 || dr2 < dr2min) { dr2min = dr2; ibest = ic; }
    }
    return ibest;
}

template<typename T, typename TV>
void ptsort_ref(int nIn, int nOut, const TV & in/*[nIn]*/, T out[/*nOut*/]) {
    for (int iout = 0; iout < nOut; ++iout) {
        out[iout].hwPt = 0;
    }
    for (int it = 0; it < nIn; ++it) {
        for (int iout = 0; iout < nOut; ++iout) {
            if (in[it].hwPt >= out[iout].hwPt) {
                for (int i2 = nOut-1; i2 > iout; --i2) {
                    out[i2] = out[i2-1];
                }
                out[iout] = in[it];
                break;
            }
        }
    }
}

#endif
