#ifndef PFALGO3_REF_H
#define PFALGO3_REF_H

#include "pfalgo_common_ref.h"

namespace l1ct {

    class PFAlgo3Emulator : public PFAlgoEmulatorBase {
        public:
            PFAlgo3Emulator(unsigned int nTrack, unsigned int nEmCalo, unsigned int nCalo, unsigned int nMu, 
                    unsigned int nPhoton, unsigned int nSelCalo, unsigned int nAllNeutral, 
                    unsigned int dR2Max_Tk_Mu, unsigned int dR2Max_Tk_Em, unsigned int dR2Max_Em_Calo, unsigned int dR2Max_Tk_Calo,
                    pt_t tk_MaxInvPt_Loose, pt_t tk_MaxInvPt_Tight): 
                PFAlgoEmulatorBase(nTrack, nCalo, nMu, nSelCalo, dR2Max_Tk_Mu, dR2Max_Tk_Calo, tk_MaxInvPt_Loose, tk_MaxInvPt_Tight),
                nEMCALO_(nEmCalo), nPHOTON_(nPhoton), nALLNEUTRAL_(nAllNeutral), dR2MAX_TK_EM_(dR2Max_Tk_Em), dR2MAX_EM_CALO_(dR2Max_Em_Calo) {}

            virtual ~PFAlgo3Emulator() override {} 

            void pfalgo3_em_ref(const EmCaloObj emcalo[/*cfg.nEMCALO*/], const HadCaloObj hadcalo[/*cfg.nCALO*/], const TkObj track[/*cfg.nTRACK*/], const bool isMu[/*cfg.nTRACK*/], bool isEle[/*cfg.nTRACK*/], PFNeutralObj outpho[/*cfg.nPHOTON*/], HadCaloObj hadcalo_out[/*cfg.nCALO*/]) const ;
            void pfalgo3_ref(const PFRegion & region, const EmCaloObj emcalo[/*cfg.nEMCALO*/], const HadCaloObj hadcalo[/*cfg.nCALO*/], const TkObj track[/*cfg.nTRACK*/], const MuObj mu[/*cfg.nMU*/], PFChargedObj outch[/*cfg.nTRACK*/], PFNeutralObj outpho[/*cfg.nPHOTON*/], PFNeutralObj outne[/*cfg.nSELCALO*/], PFChargedObj outmu[/*cfg.nMU*/]) const ;

            void pfalgo3_merge_neutrals_ref(const PFNeutralObj pho[/*cfg.nPHOTON*/], const PFNeutralObj ne[/*cfg.nSELCALO*/], PFNeutralObj allne[/*cfg.nALLNEUTRALS*/]) const ;

        protected:
            unsigned int nEMCALO_, nPHOTON_, nALLNEUTRAL_;
            unsigned int dR2MAX_TK_EM_;
            unsigned int dR2MAX_EM_CALO_;

            int tk_best_match_ref(unsigned int nCAL, unsigned int dR2MAX, const l1ct::EmCaloObj calo[/*nCAL*/], const l1ct::TkObj & track) const ;
            int em_best_match_ref(unsigned int nCAL, unsigned int dR2MAX, const l1ct::HadCaloObj calo[/*nCAL*/], const l1ct::EmCaloObj & em) const ;
};


} // namespace

#endif
