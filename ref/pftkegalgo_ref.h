#ifndef PFTKEGALGO_REF_H
#define PFTKEGALGO_REF_H
#ifndef CMSSW_GIT_HASH
  #include "../DiscretePFInputs.h"
#else
  #include "../../interface/DiscretePFInputs.h"
#endif

#include "../firmware/pfalgo2hgc.h"
#include "pfalgo_common_ref.h"



namespace pFTkEGAlgo {

  struct pftkegalgo_config {
      unsigned int nTRACK;
      unsigned int nEMCALO;
      unsigned int nEMCALOSEL_EGIN;
      unsigned int nEM_EGOUT;

      bool filterHwQuality;
      bool doBremRecovery;
      
      pftkegalgo_config(unsigned int nTrack, 
                        unsigned int nEmCalo, 
                        unsigned int nEmCaloSel_in, 
                        unsigned int nEmOut,
                        bool filterHwQuality,
                        bool doBremRecovery) :
          nTRACK(nTrack), 
          nEMCALO(nEmCalo), 
          nEMCALOSEL_EGIN(nEmCaloSel_in), 
          nEM_EGOUT(nEmOut),
          filterHwQuality(filterHwQuality),
          doBremRecovery(doBremRecovery) {}
  };

  struct Region {
    std::vector<l1tpf_impl::CaloCluster> emcalo;
    std::vector<l1tpf_impl::PropagatedTrack> track;
    std::vector<EGIsoParticle> egphotons;
    std::vector<EGIsoEleParticle> egeles;
  };

  // void pftkegalgo_ref_set_debug(int debug) ;

  bool hwpt_sort(const l1tpf_impl::CaloCluster &i, const l1tpf_impl::CaloCluster &j);



  class PFTkEGAlgo {
  public:
    
    PFTkEGAlgo(const pftkegalgo_config& config) :
    cfg(config),
    filterHwQuality_(config.filterHwQuality),
    doBremRecovery_(config.doBremRecovery) {}

    void pftkegalgo_ref(const pftkegalgo_config &cfg,
                        const EmCaloObj emcalo[/*cfg.nCALO*/],
                        const TkObj track[/*cfg.nTRACK*/],
                        int emCalo2tk_ref[],
                        int emCalo2emcalo_ref[],
                        EGIsoParticle egphs_ref[],
                        EGIsoEleParticle egele_ref[]) ;

    void link_emCalo2emCalo(const std::vector<l1tpf_impl::CaloCluster>& emcalo,
                            std::vector<int> &emCalo2emCalo);

    void link_emCalo2tk(const std::vector<l1tpf_impl::CaloCluster> &emcalo,
                        const std::vector<l1tpf_impl::PropagatedTrack> &track,
                        std::vector<int> &emCalo2tk);

    float deltaPhi(float phi1, float phi2);


    void sel_emCalo_ref(const EmCaloObj emcalo[/*cfg.nCALO*/],
                        EmCaloObj emcalo_sel_ref[/*cfg.nCALO*/]);

    void eg_algo(Region &r,
                 const std::vector<int> &emCalo2emCalo,
                 const std::vector<int> &emCalo2tk);

     EGIsoParticle &addEGIsoToPF(std::vector<EGIsoParticle> &egobjs,
                                 const l1tpf_impl::CaloCluster &calo,
                                 const int hwQual,
                                 const int ptCorr);

     EGIsoEleParticle &addEGIsoEleToPF(std::vector<EGIsoEleParticle> &egobjs,
                                       const l1tpf_impl::CaloCluster &calo,
                                       const l1tpf_impl::PropagatedTrack &track,
                                       const int hwQual,
                                       const int ptCorr);

     void addEgObjsToPF(Region &r, const int calo_idx, const int hwQual, const int ptCorr, const int tk_idx);

     //FIXME: this is hugly but we will soon change the interface
     void setRegionEtaCenter(float etacenter) {
       reg_eta_center_ = etacenter;
     }

   private:
     
     
     float globalAbsEta(float localEta) {
       return std::abs(localEta + reg_eta_center_);
     }

     
     
     pftkegalgo_config cfg;
     bool debug_ = 1;
     bool filterHwQuality_;
     bool doBremRecovery_;

     int caloHwQual_ = 4;
     float dEtaMaxBrem_ = 0.02;
     float dPhiMaxBrem_ = 0.1;
     // FIXME: need region center in FW, we don't have it for now hence we keep 1 value for the barrel
     // std::vector<float> absEtaBoundaries_{0.0, 0.9, 1.5};
     //  // FIXME: should be {0.025, 0.015, 0.01}  but 0.01 it is too small give eta precision. We use  0.0174533 hwEta: 4
     // std::vector<float> dEtaValues_{0.025, 0.015, 0.0174533};
     // std::vector<float> dPhiValues_{0.07, 0.07, 0.07};
     std::vector<float> absEtaBoundaries_{0.0, 1.5};
      // FIXME: should be {0.025, 0.015, 0.01}  but 0.01 it is too small give eta precision. We use  0.0174533 hwEta: 4
     std::vector<float> dEtaValues_{0.015, 0.0174533};
     std::vector<float> dPhiValues_{0.07, 0.07};

     float trkQualityPtMin_ = 10;
     float reg_eta_center_;
   };
}

#endif
