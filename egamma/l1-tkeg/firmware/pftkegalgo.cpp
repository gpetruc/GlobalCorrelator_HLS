#include "pftkegalgo.h"
#include <cassert>
#include "hls_math.h"

#include "../../../dataformats/layer1_emulator.h"
#include "../../../pf/firmware/pfalgo_types.h"

using namespace l1ct;
#include "../../../pf/firmware/pfalgo_common.h"
#include "../../../pf/firmware/pfalgo_common.icc"


dpt_t ell_dpt_int_cap(glbeta_t abseta_ref, eta_t eta1, phi_t phi1, eta_t eta2, phi_t phi2, pt_t pt1, pt_t pt2, dpt_t max) {
#pragma HLS INLINE

#if defined(REG_HGCal)
    // const ap_uint<10> cdeta_int = 16;
    const ap_ufixed<12,10> cdeta = 16;
#endif

#if defined(REG_Barrel)
    // const ap_uint<10> cdeta_int = (hls::abs(eta1) > 3) ? 22 : 8;
    // //FIXME: global absolute eta > 0.9 
    // const ap_ufixed<10,12> cdeta = (hls::abs(eta1) > 206) ? 21.75 : 7.75;
    // const ap_uint<10> cdeta = 22;
    const ap_ufixed<12,10> cdeta = 21.75;
    // const ap_ufixed<12,10> cdeta = (abseta_ref > 206) ? 21.75 : 7.75;
#endif

    const ap_uint<10> cm = 256;
    // const ap_uint<10> cm_int = 256;

    ap_int<eta_t::width+1> d_eta = (eta1-eta2);
    ap_int<phi_t::width+1> d_phi = (phi1-phi2);

    ap_uint<22> ell = d_phi*d_phi + d_eta*d_eta*cdeta;

    std::cout << "[FW] ell: " << ell << " cm: " << cm << " match:" << (ell <= int(cm)) <<std::endl;
    
    dpt_t d_pt = hls::abs(pt1 - pt2);
    return (ell <= int(cm)) ? d_pt : max;
}



template<int DPTMAX>
void calo2tk_ellipticdptvals(const l1ct::PFRegion & region, const EmCaloObj &em, const TkObj track[NTRACK], dpt_t calo_track_dptval[NTRACK]) {
// Not inlining this allows to save DSPs with II != 1
// #pragma HLS INLINE
    const dpt_t eDPTMAX = DPTMAX; 
    const pt_t trkQualityPtMin_ = 10.; // 10 GeV
    glbeta_t abseta = hls::abs(region.hwGlbEta(em.hwEta));
    
    track_loop: for (int itk = 0; itk < NTRACK; ++itk) {
      if (track[itk].hwPt < trkQualityPtMin_ || em.hwPt == 0) {
        calo_track_dptval[itk] = eDPTMAX;
      } else {
        calo_track_dptval[itk] = ell_dpt_int_cap(abseta, em.hwEta, em.hwPhi, track[itk].hwEta, track[itk].hwPhi, em.hwPt, track[itk].hwPt, eDPTMAX);
        std::cout << "[" << itk << "] dpt: " << calo_track_dptval[itk] << std::endl;
      }
    }

}



void link_emCalo2emCalo(const EmCaloObj emcalo[NEMCALO_EGIN], ap_uint<NEMCALO_EGIN> emCalo2emcalo_bit[NEMCALO_EGIN]) {
  #pragma HLS ARRAY_PARTITION variable=emcalo complete dim=1
  #pragma HLS ARRAY_PARTITION variable=emCalo2emcalo_bit complete dim=1
  #pragma HLS INLINE

  const ap_int<eta_t::width+1>  dEtaMaxBrem_ = 5; // 0.02; -> round(0.02*4*180/3.14)
  const ap_int<phi_t::width+1>  dPhiMaxBrem_ = 23; // 0.1; -> round(0.1*4*180/3.14)

  // NOTE: we assume the input to be sorted!!!
  brem_reco_outer_loop: for (int ic = 0; ic < NEMCALO_EGIN; ++ic) {
    auto &calo = emcalo[ic];
    brem_reco_inner_loop: for (int jc = ic + 1; jc < NEMCALO_EGIN; ++jc) {
      auto &otherCalo = emcalo[jc];
      if (calo.hwPt != 0 && otherCalo.hwPt != 0 &&
        hls::abs(otherCalo.hwEta - calo.hwEta) < dEtaMaxBrem_ &&
          hls::abs(otherCalo.hwPhi - calo.hwPhi) < dPhiMaxBrem_) {
            emCalo2emcalo_bit[ic][jc] = 1;
            emCalo2emcalo_bit[jc][jc] = 1; // use diagonal bit to mark the cluster as already used
            std::cout << "[FW] BREM: set to be used " << ic << " " << jc << std::endl;
            std::cout << "[FW] BREM: set to skip " << jc << " " << jc << std::endl;
      }
    }
  }
}





void link_emCalo2tk(const l1ct::PFRegion & region, 
                    const EmCaloObj emcalo[NEMCALO_EGIN],
                    const TkObj track[NTRACK],
                    ap_uint<NTRACK> emCalo2tk_bit[NEMCALO_EGIN]) {
  #pragma HLS INLINE

  #pragma HLS ARRAY_PARTITION variable=emcalo complete dim=1
  #pragma HLS ARRAY_PARTITION variable=track complete dim=1
  #pragma HLS ARRAY_PARTITION variable=emCalo2tk_bit complete dim=1

  const int DPTMAX = 16384; // dpt_t = ap_fixed<16,14,AP_TRN,AP_SAT> ->  max =(2^(16) - 1)/2^2 = 16383.75
  calo_loop: for (int ic = 0; ic < NEMCALO_EGIN; ++ic) {
    dpt_t dptvals[NTRACK];
    calo2tk_ellipticdptvals<DPTMAX>(region, emcalo[ic], track, dptvals);
    emCalo2tk_bit[ic] = pick_closest<DPTMAX,NTRACK,dpt_t>(dptvals);
  }

}

#if defined(REG_HGCal)
void sel_emCalo(const EmCaloObj emcalo[NEMCALO], EmCaloObj emcalo_sel[NEMCALO_EGIN]) {
  #pragma HLS INLINE
  // #pragma HLS INLINE REGION

  EmCaloObj emcalo_sel_temp[NEMCALO];
  #pragma HLS ARRAY_PARTITION variable=emcalo_sel complete dim=1
  #pragma HLS ARRAY_PARTITION variable=emcalo_sel_temp complete dim=1
  EmCaloObj emcalo_zero;
  clear(emcalo_zero);
  in_select_loop: for(int ic = 0; ic < NEMCALO; ++ic) {
    emcalo_sel_temp[ic] = (emcalo[ic].hwFlags == 4) ? emcalo[ic] : emcalo_zero;
    if(emcalo[ic].hwPt > 0) std::cout << "[FW] IN emcalo[" << ic << "] with pt: " << emcalo[ic].hwPt << " qual: " << emcalo[ic].hwFlags << " eta: " << emcalo[ic].hwEta << " phi " << emcalo[ic].hwPhi << std::endl;
    if(emcalo_sel_temp[ic].hwPt > 0)std::cout << "[FW] SEL emcalo with pt: " << emcalo_sel_temp[ic].hwPt << " qual: " << emcalo_sel_temp[ic].hwFlags << " eta: " << emcalo_sel_temp[ic].hwEta << " phi " << emcalo_sel_temp[ic].hwPhi << std::endl;

  }
  ptsort_hwopt<EmCaloObj,NEMCALO,NEMCALO_EGIN>(emcalo_sel_temp, emcalo_sel);  
}
#endif


#if defined(REG_Barrel)
// NOTE: for now this is a placeholder more than anything else
void sel_emCalo(const EmCaloObj emcalo[NEMCALO], EmCaloObj emcalo_sel[NEMCALO_EGIN]) {
  #pragma HLS INLINE
  // #pragma HLS INLINE REGION

  EmCaloObj emcalo_sel_temp[NEMCALO];
  #pragma HLS ARRAY_PARTITION variable=emcalo_sel complete dim=1
  #pragma HLS ARRAY_PARTITION variable=emcalo_sel_temp complete dim=1
  EmCaloObj emcalo_zero;
  clear(emcalo_zero);
  in_select_loop: for(int ic = 0; ic < NEMCALO; ++ic) {
    // we require pt>2GeV
    emcalo_sel_temp[ic] = (emcalo[ic].hwPt >= 2.) ? emcalo[ic] : emcalo_zero;
    if(emcalo[ic].hwPt > 0) std::cout << "[FW] IN emcalo [" << ic << "] with pt: " << emcalo[ic].hwPt << " qual: " << emcalo[ic].hwFlags << " eta: " << emcalo[ic].hwEta << " phi " << emcalo[ic].hwPhi << std::endl;
    if(emcalo_sel_temp[ic].hwPt > 0)std::cout << "[FW] SEL emcalo with pt: " << emcalo_sel_temp[ic].hwPt << " qual: " << emcalo_sel_temp[ic].hwFlags << " eta: " << emcalo_sel_temp[ic].hwEta << " phi " << emcalo_sel_temp[ic].hwPhi << std::endl;

  }
  ptsort_hwopt<EmCaloObj,NEMCALO,NEMCALO_EGIN>(emcalo_sel_temp, emcalo_sel);
}
#endif


void pftkegalgo(const l1ct::PFRegion & region, const EmCaloObj emcalo[NCALO], const TkObj track[NTRACK],
  EGIsoObj photons[NEM_EGOUT], EGIsoEleObj eles[NEM_EGOUT]) {
    #ifdef HLS_pipeline_II
     #if HLS_pipeline_II == 1
        #pragma HLS pipeline II=1
     #elif HLS_pipeline_II == 2
        #pragma HLS pipeline II=2
     #elif HLS_pipeline_II == 3
        #pragma HLS pipeline II=3
     #elif HLS_pipeline_II == 4
        #pragma HLS pipeline II=4
     #elif HLS_pipeline_II == 6
        #pragma HLS pipeline II=6
     #endif
    #else
        #pragma HLS pipeline II=2
    #endif
  // #pragma HLS PIPELINE II=HLS_pipeline_II
  #pragma HLS ARRAY_PARTITION variable=emcalo complete dim=1
  #pragma HLS ARRAY_PARTITION variable=track complete dim=1

  #pragma HLS ARRAY_PARTITION variable=photons complete dim=1
  #pragma HLS ARRAY_PARTITION variable=eles complete dim=1

  EmCaloObj emcalo_sel[NEMCALO_EGIN];
  sel_emCalo(emcalo, emcalo_sel);

  // FIXME: shall we forseen the same selection step for tracks?
  ap_uint<NTRACK> emCalo2tk_bit[NEMCALO_EGIN];
  ap_uint<NEMCALO_EGIN> emCalo2emcalo_bit[NEMCALO_EGIN];
  #pragma HLS ARRAY_PARTITION variable=emCalo2tk_bit complete dim=1
  #pragma HLS ARRAY_PARTITION variable=emCalo2emcalo_bit complete dim=1

  // initialize
  init_loop: for (int ic = 0; ic < NEMCALO_EGIN; ++ic) {
    emCalo2tk_bit[ic] = 0;
    emCalo2emcalo_bit[ic] = 0;
  }

  #if defined(DOBREMRECOVERY)
    link_emCalo2emCalo(emcalo_sel, emCalo2emcalo_bit);
  #endif
  //
  link_emCalo2tk(region, emcalo_sel, track, emCalo2tk_bit);


  EGIsoObj photons_temp[NEMCALO_EGIN];
  EGIsoEleObj eles_temp[NEMCALO_EGIN];
  #pragma HLS ARRAY_PARTITION variable=photons_temp complete dim=1
  #pragma HLS ARRAY_PARTITION variable=eles_temp complete dim=1

  loop_calo: for (int ic = 0; ic < NEMCALO_EGIN; ++ic) {
    if(emcalo_sel[ic].hwPt > 0)std::cout << "[FW] emcalo [" << ic << "]  with pt: " << emcalo_sel[ic].hwPt << " qual: " << emcalo_sel[ic].hwFlags << " eta: " << emcalo_sel[ic].hwEta << " phi " << emcalo_sel[ic].hwPhi << std::endl;

    clear(photons_temp[ic]);
    clear(eles_temp[ic]);
    int track_id = -1;
    loop_track_matched: for(int it = 0; it < NTRACK; ++it) {
      if(emCalo2tk_bit[ic][it]) {
        track_id = it;
        break;
      }
    }

    pt_t ptcorr = emcalo_sel[ic].hwPt;
    #if defined(DOBREMRECOVERY)
      if(emCalo2emcalo_bit[ic][ic] != 1) {
        // FIXME: we should set the quality bit as "brem-recovery performed"
        loop_calo_brem_reco: for (int ioc = 0; ioc < NEMCALO_EGIN; ++ioc) {
          if(emCalo2emcalo_bit[ic][ioc]) {
            ptcorr += emcalo_sel[ioc].hwPt;
          }
        }
      } else {
        // This cluster has alread been used in brem reclustering
        // shall we just set the quality to a different value???
        std::cout << "   skip!" << std::endl;
        continue;
      }
    #endif

    photons_temp[ic].hwPt = ptcorr;
    photons_temp[ic].hwEta = emcalo_sel[ic].hwEta;
    photons_temp[ic].hwPhi = emcalo_sel[ic].hwPhi;
    if(photons_temp[ic].hwPt) std::cout << "[FW] Add EGIsoObj with pt: " << ptcorr << " qual: " << "NULL" << " eta: " << photons_temp[ic].hwEta << " phi " << photons_temp[ic].hwPhi << std::endl;

    if(emCalo2tk_bit[ic]) {
      eles_temp[ic].hwPt = ptcorr;
      eles_temp[ic].hwEta = emcalo_sel[ic].hwEta;
      eles_temp[ic].hwPhi = emcalo_sel[ic].hwPhi;
      // FIXME: add track properties @ vertex using track[track_id]
      eles_temp[ic].hwZ0 = track[track_id].hwZ0;
      if(eles_temp[ic].hwPt) std::cout << "[FW] Add EGIsoEleObj with pt: " << ptcorr << " qual: " << "NULL" << " eta: " << eles_temp[ic].hwEta << " phi " << eles_temp[ic].hwPhi << std::endl;

    } 

  }
  
  ptsort_hwopt<EGIsoObj,NEMCALO_EGIN,NEM_EGOUT>(photons_temp, photons);
  ptsort_hwopt<EGIsoEleObj,NEMCALO_EGIN,NEM_EGOUT>(eles_temp, eles);

}
