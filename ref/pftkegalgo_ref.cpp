#include "pftkegalgo_ref.h"


#include "../utils/Firmware2DiscretePF.h"
#include "../utils/DiscretePF2Firmware.h"

#include <cmath>
#include <cstdio>
#include <algorithm>
#include <memory>
#include <iostream>



int g_pftkegalgo_debug_ref_ = 0;

using namespace pFTkEGAlgo;


float PFTkEGAlgo::deltaPhi(float phi1, float phi2) {
  // reduce to [-pi,pi]
  float x = phi1 - phi2;
  float o2pi = 1. / (2. * M_PI);
  if (std::abs(x) <= float(M_PI))
      return x;
  float n = std::round(x * o2pi);
  return x - n * float(2. * M_PI);
}

void PFTkEGAlgo::link_emCalo2emCalo(const std::vector<l1tpf_impl::CaloCluster>& emcalo,
                        std::vector<int> &emCalo2emCalo) {
  // NOTE: we assume the input to be sorted!!!
  for (int ic = 0, nc = emcalo.size(); ic < nc; ++ic) {
    auto &calo = emcalo[ic];
    if (filterHwQuality_ && calo.hwFlags != caloHwQual_)
      continue;

    if (emCalo2emCalo[ic] != -1)
      continue;

    for (int jc = ic + 1; jc < nc; ++jc) {
      if (emCalo2emCalo[jc] != -1)
        continue;

      auto &otherCalo = emcalo[jc];
      if (filterHwQuality_ && otherCalo.hwFlags != caloHwQual_)
        continue;

      if (fabs(otherCalo.floatEta() - calo.floatEta()) < dEtaMaxBrem_ &&
          fabs(deltaPhi(otherCalo.floatPhi(), calo.floatPhi())) < dPhiMaxBrem_) {
        emCalo2emCalo[jc] = ic;
      }
    }
  }
}



void PFTkEGAlgo::link_emCalo2tk(const std::vector<l1tpf_impl::CaloCluster> &emcalo,
                                const std::vector<l1tpf_impl::PropagatedTrack> &track,
                                std::vector<int> &emCalo2tk) {

  if (debug_ > 10)
    std::cout << "[link_emCalo2tk]" << std::endl;

  for (int ic = 0, nc = emcalo.size(); ic < nc; ++ic) {
    auto &calo = emcalo[ic];

    if (filterHwQuality_ && calo.hwFlags != caloHwQual_)
      continue;

    if (debug_ > 10) {
      std::cout << "[REF] EM calo [" << ic << "] hwFlags: " << calo.hwFlags << std::endl;
      std::cout << "--- calo: pt: " << calo.floatPt() << " eta: " << calo.floatEta() << " phi: " << calo.floatPhi()
                << std::endl;
    }

    float dPtMin = 999;
    for (int itk = 0, ntk = track.size(); itk < ntk; ++itk) {
      const auto &tk = track[itk];
      if (tk.floatPt() < trkQualityPtMin_)
        continue;

      if (debug_ > 10) {
        std::cout << "[REF] tk [" << itk << "] tk: pt: " << tk.floatPt() << " eta: " << tk.floatEta() << " phi: " << tk.floatPhi() << " z0: " << tk.hwZ0
        << std::endl;
      }

      float d_phi = deltaPhi(tk.floatPhi(), calo.floatPhi());
      float d_eta = tk.floatEta() - calo.floatEta();  // We only use it squared

      auto eta_index =
          std::distance(
              absEtaBoundaries_.begin(),
              std::lower_bound(absEtaBoundaries_.begin(), absEtaBoundaries_.end(), globalAbsEta(calo.floatEta()))) -
          1;
      float dEtaMax = dEtaValues_[eta_index];
      float dPhiMax = dPhiValues_[eta_index];
      
      std::cout << "eta: " << calo.floatEta() << " globa eta: " << globalAbsEta(calo.floatEta()) << " deta_max: " << dEtaMax << " dphimax: " << dPhiMax << std::endl;
      // if (debug_ > 0)
      //   std::cout << " deta: " << fabs(d_eta) << " dphi: " << d_phi
      //             << " ell: " << ((d_phi / dPhiMax) * (d_phi / dPhiMax)) + ((d_eta / dEtaMax) * (d_eta / dEtaMax))
      //             << std::endl;
      // std::cout << "Global abs eta: " << r.globalAbsEta(calo.floatEta())
      //           << " abs eta: " << fabs(calo.floatEta()) << std::endl;

      if ((((d_phi / dPhiMax) * (d_phi / dPhiMax)) + ((d_eta / dEtaMax) * (d_eta / dEtaMax))) < 1.) {
        if (debug_ > 0) {
              if (debug_ <= 10){
          std::cout << "[REF] EM calo [" << ic << "] hwFlags: " << calo.hwFlags << std::endl;
          std::cout << "--- calo: pt: " << calo.floatPt() << " eta: " << calo.floatEta() << " phi: " << calo.floatPhi()
                    << std::endl;
          std::cout << "[REF] tk [" << itk << " tk: pt: " << tk.floatPt() << " eta: " << tk.floatEta() << " phi: " << tk.floatPhi() << " z0: " << tk.hwZ0
                    << std::endl;
                  }
          std::cout << "    pass elliptic " << std::endl;
        }
        // NOTE: for now we implement only best pt match. This is NOT what is done in the L1TkElectronTrackProducer
        if (fabs(tk.floatPt() - calo.floatPt()) < dPtMin) {
          if (debug_ > 0)
            std::cout << "     best pt match: " << fabs(tk.floatPt() - calo.floatPt()) << std::endl;
          emCalo2tk[ic] = itk;
          dPtMin = fabs(tk.floatPt() - calo.floatPt());
        }
      }
    }
  }
}





// void PFTkEGAlgo::pftkegalgo_ref_set_debug(int debug) { g_pftkegalgo_debug_ref_ = debug; }

bool pFTkEGAlgo::hwpt_sort(const l1tpf_impl::CaloCluster &i, const l1tpf_impl::CaloCluster &j) {
  return (i.hwPt > j.hwPt);
}


void PFTkEGAlgo::sel_emCalo_ref(const EmCaloObj emcalo[/*cfg.nCALO*/],
  EmCaloObj emcalo_sel_ref[/*cfg.nCALO*/]) {


  std::vector<l1tpf_impl::CaloCluster> emcalos(NEMCALO);
  for(int ic = 0; ic < NEMCALO; ic++) {
    // if (emcalo[i].hwPt == 0) continue;
    l1tpf_impl::CaloCluster c;
    fw2dpf::convert(emcalo[ic], c);
    // if(emcalo[ic].hwPt != 0)
    // std::cout << "[" << ic << "] EmCaloObj pt: " << emcalo[ic].hwPt << " CaloCluster pt: " << c.hwPt << std::endl;
    emcalos.push_back(c);
  }
  std::sort(emcalos.begin(), emcalos.end(), hwpt_sort);

  for(int ic = 0; ic < NEMCALOSEL_EGIN; ic++) {
    clear(emcalo_sel_ref[ic]);
  }

  int jc = 0;
  for(int ic = 0; ic < NEMCALO; ic++) {
    if(emcalos[ic].hwFlags == 4) {
      // std::cout << "[" << ic << "] EmCaloObj pt: " << emcalos[ic].hwPt << " hwflags: " << emcalos[ic].hwFlags << std::endl;
      dpf2fw::convert(emcalos[ic], emcalo_sel_ref[jc++]);
      if(jc == NEMCALOSEL_EGIN) break;
    }
  }
}



void PFTkEGAlgo::pftkegalgo_ref(const pftkegalgo_config &cfg,
                    const EmCaloObj emcalo[/*cfg.nCALO*/],
                    const TkObj track[/*cfg.nTRACK*/],
                    int emCalo2tk_ref[],
                    int emCalo2emcalo_ref[],
                    EGIsoParticle egphs_ref[],
                    EGIsoEleParticle egele_ref[]) {

  // printf("[pftkegalgo_ref]\n");
  //     if (g_pftkegalgo_debug_ref_) {
  // #ifdef L1Trigger_Phase2L1ParticleFlow_DiscretePFInputs_MORE
  pFTkEGAlgo::Region reg;

  for (unsigned int i = 0; i < cfg.nTRACK; ++i) { if (track[i].hwPt == 0) continue;
      l1tpf_impl::PropagatedTrack tk; fw2dpf::convert(track[i], tk);
      // printf("FW  \t track %3d: pt %8d [ %7.2f ]  calo eta %+7d [ %+5.2f ]  calo phi %+7d [ %+5.2f ]  calo ptErr %6d [ %7.2f ]   tight %d\n",
      //                     i, tk.hwPt, tk.floatPt(), tk.hwEta, tk.floatEta(), tk.hwPhi, tk.floatPhi(), tk.hwCaloPtErr, tk.floatCaloPtErr(), int(track[i].hwTightQuality));
      reg.track.push_back(tk);
  }
  for (unsigned int i = 0; i < cfg.nEMCALO; ++i) { if (emcalo[i].hwPt == 0) continue;
      l1tpf_impl::CaloCluster c; fw2dpf::convert(emcalo[i], c);
      // printf("FW  \t EM calo  %3d: pt %8d [ %7.2f ]  calo eta %+7d [ %+5.2f ]  calo phi %+7d [ %+5.2f ]  calo emPt %7d [ %7.2f ]   isEM %d \n",
      //                     i, c.hwPt, c.floatPt(), c.hwEta, c.floatEta(), c.hwPhi, c.floatPhi(), c.hwEmPt, c.floatEmPt(), c.isEM);
      reg.emcalo.push_back(c);
  }

  // printf("# tracks: %3d # emcalos: %3d\n", tracks.size(), emcalos.size());


  std::vector<int> emCalo2emCalo(reg.emcalo.size(), -1);
  if (doBremRecovery_)
    link_emCalo2emCalo(reg.emcalo, emCalo2emCalo);
  // convert to array
  for(int id = 0; id < cfg.nEMCALO && id < emCalo2emCalo.size(); id++) {
      emCalo2emcalo_ref[id] = emCalo2emCalo[id];
    }


  std::vector<int> emCalo2tk(reg.emcalo.size(), -1);
  link_emCalo2tk(reg.emcalo, reg.track, emCalo2tk);

  // convert to array
  for(int id = 0; id < cfg.nEMCALO && id < emCalo2tk.size(); id++) {
    emCalo2tk_ref[id] = emCalo2tk[id];
  }


  eg_algo(reg, emCalo2emCalo, emCalo2tk);

  // std::cout << "# EG photons: " << reg.egphotons.size() << " # eles: " << reg.egeles.size() << std::endl;

  // initialize and fill the output arrays
  for(int ic = 0; ic != cfg.nEM_EGOUT; ++ic) {
    clear(egphs_ref[ic]);
    clear(egele_ref[ic]);
  }

  for(int ic = 0; ic != reg.egphotons.size(); ++ic) {
    egphs_ref[ic] = reg.egphotons[ic];
  }
  for(int ic = 0; ic != reg.egeles.size(); ++ic) {
    egele_ref[ic] = reg.egeles[ic];
  }

}


void PFTkEGAlgo::eg_algo(Region &r,
                         const std::vector<int> &emCalo2emCalo,
                         const std::vector<int> &emCalo2tk) {

  for (int ic = 0, nc = r.emcalo.size(); ic < nc; ++ic) {
    auto &calo = r.emcalo[ic];
    if (filterHwQuality_ && calo.hwFlags != caloHwQual_)
      continue;

    int itk = emCalo2tk[ic];


    // check if brem recovery is on
    if (!doBremRecovery_) {
      // 1. create EG objects before brem recovery
      addEgObjsToPF(r, ic, calo.hwFlags, calo.hwPt, itk);
      continue;
    }

    // check if the cluster has already been used in a brem reclustering
    if (emCalo2emCalo[ic] != -1)
      continue;

    int ptBremReco = calo.hwPt;

    for (int jc = ic; jc < nc; ++jc) {
      if (emCalo2emCalo[jc] == ic) {
        auto &otherCalo = r.emcalo[jc];
        ptBremReco += otherCalo.hwPt;
      }
    }

    // 2. create EG objects with brem recovery
    // FIXME: duplicating the object is suboptimal but this is done for keeping things as in TDR code...
    addEgObjsToPF(r, ic, calo.hwFlags + 1, ptBremReco, itk);
  }
}

EGIsoParticle &PFTkEGAlgo::addEGIsoToPF(std::vector<EGIsoParticle> &egobjs,
                                        const l1tpf_impl::CaloCluster &calo,
                                        const int hwQual,
                                        const int ptCorr) {
  EGIsoParticle egiso;
  egiso.hwPt = ptCorr;
  egiso.hwEta = calo.hwEta;
  egiso.hwPhi = calo.hwPhi;
  // egiso.hwQual = hwQual;
  // egiso.hwIso = 0;
  egobjs.push_back(egiso);
  return egobjs.back();
}

EGIsoEleParticle &PFTkEGAlgo::addEGIsoEleToPF(std::vector<EGIsoEleParticle> &egobjs,
                                              const l1tpf_impl::CaloCluster &calo,
                                              const l1tpf_impl::PropagatedTrack &track,
                                              const int hwQual,
                                              const int ptCorr) {
  EGIsoEleParticle egiso;
  egiso.hwPt = ptCorr;
  egiso.hwEta = calo.hwEta;
  egiso.hwPhi = calo.hwPhi;

  // egiso.hwVtxEta = track.hwVtxEta;
  // egiso.hwVtxPhi = track.hwVtxPhi;
  egiso.hwZ0 = track.hwZ0;
  // egiso.hwCharge = track.hwCharge;

  // egiso.hwQual = hwQual;
  // egiso.hwIso = 0;
  egobjs.push_back(egiso);
  return egobjs.back();
}

void PFTkEGAlgo::addEgObjsToPF(
    Region &r, const int calo_idx, const int hwQual, const int ptCorr, const int tk_idx) {
  EGIsoParticle &egobj = addEGIsoToPF(r.egphotons, r.emcalo[calo_idx], hwQual, ptCorr);
  if (tk_idx != -1) {
    // egobj.ele_idx = r.egeles.size();
    addEGIsoEleToPF(r.egeles, r.emcalo[calo_idx], r.track[tk_idx], hwQual, ptCorr);
  }
}
