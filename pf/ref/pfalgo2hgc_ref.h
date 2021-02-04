#ifndef PFALGO2HGC_REF_H
#define PFALGO2HGC_REF_H

#include "pfalgo_common_ref.h"

namespace l1ct {

    class PFAlgo2HGCEmulator : public PFAlgoEmulatorBase {
        public:
            PFAlgo2HGCEmulator(unsigned int nTrack, unsigned int nCalo, unsigned int nMu, unsigned int nSelCalo,
                  unsigned int dR2Max_Tk_Mu, unsigned int dR2Max_Tk_Calo,
                  pt_t tk_MaxInvPt_Loose, pt_t tk_MaxInvPt_Tight) :
                PFAlgoEmulatorBase(nTrack, nCalo, nMu, nSelCalo, dR2Max_Tk_Mu, dR2Max_Tk_Calo, tk_MaxInvPt_Loose, tk_MaxInvPt_Tight) {}

            virtual ~PFAlgo2HGCEmulator() override {}

            void pfalgo2hgc_ref(const PFRegion & region, const HadCaloObj calo[/*cfg.nCALO*/], const TkObj track[/*cfg.nTRACK*/], const MuObj mu[/*cfg.nMU*/], PFChargedObj outch[/*cfg.nTRACK*/], PFNeutralObj outne[/*cfg.nSELCALO*/], PFChargedObj outmu[/*cfg.nMU*/]) const ;
    };

} // namespace

#endif
