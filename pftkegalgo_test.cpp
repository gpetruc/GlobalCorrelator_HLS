#include <cstdio>
#include "firmware/pftkegalgo.h"

#include "ref/pftkegalgo_ref.h"

#include "utils/DiscretePFInputsReader.h"
#include "utils/pattern_serializer.h"
#include "utils/test_utils.h"

#define NTEST 10000

bool emcalo_equal(const EmCaloObj &emcalo1, const EmCaloObj &emcalo2) {
  return emcalo1.hwPt == emcalo2.hwPt && emcalo1.hwEta == emcalo2.hwEta && emcalo1.hwPhi == emcalo2.hwPhi;
}

bool compare(const EGIsoEleParticle&ele1, const EGIsoEleParticle&ele2) {
  // FIXME: add other parameters
  return (ele1.hwPt == ele2.hwPt &&
          ele1.hwEta == ele2.hwEta &&
          ele1.hwPhi == ele2.hwPhi &&
          ele1.hwZ0 == ele2.hwZ0);
}

bool compare(const EGIsoParticle&ele1, const EGIsoParticle&ele2) {
  // FIXME: add other parameters
  return (ele1.hwPt == ele2.hwPt &&
          ele1.hwEta == ele2.hwEta &&
          ele1.hwPhi == ele2.hwPhi);
}


int main() {
    HumanReadablePatternSerializer debugHR("-", /*zerosuppress=*/true); // this will print on stdout, we'll use it for errors
#if defined(REG_HGCal)
std::cout << "REG_Hgcal" << std::endl;

    DiscretePFInputsReader inputs("DoubleElectron_PU200_HGCal.dump");
    bool filterHwQuality = true;
    bool doBremRecovery = true;
#endif

#if defined(REG_Barrel)
std::cout << "REG_Barrel" << std::endl;
    DiscretePFInputsReader inputs("DoubleElectron_PU200_Barrel.dump");
    bool filterHwQuality = false;
    bool doBremRecovery = false;
#endif
    // input TP objects
    HadCaloObj calo[NCALO]; EmCaloObj emcalo[NEMCALO]; TkObj track[NTRACK]; z0_t hwZPV;
    MuObj mu[NMU];
    float reg_eta_center;
    // output PF objects
    PFChargedObj outch[NTRACK], outch_ref[NTRACK];
    PFNeutralObj outne[NSELCALO], outne_ref[NSELCALO];
    PFChargedObj outmupf[NMU], outmupf_ref[NMU];

    pFTkEGAlgo::pftkegalgo_config cfg(NTRACK, NEMCALO, NEMCALOSEL_EGIN, NEM_EGOUT, filterHwQuality, doBremRecovery);
    
    pFTkEGAlgo::PFTkEGAlgo algo(cfg);
    // -----------------------------------------
    // run multiple tests
    int passed_tests = 0;
    int nEles_tests = 0;
    int nPhotons_tests = 0;

    for (int test = 1; test <= NTEST; ++test) {
      // std::cout << "------------ TEST: " << test << std::endl;
        // get the inputs from the input object
        if (!inputs.nextRegion(calo, emcalo, track, mu, hwZPV, reg_eta_center)) break;

        // for(auto region: inputs.event().regions) {
        //   printf("# of EM objects: %u\n", region.emcalo.size());
        //   for(auto emc: region.emcalo) {
        //     printf("   - hwFlags: %u\n", emc.hwFlags);
        //   }
        // }
        algo.setRegionEtaCenter(reg_eta_center);
        std::cout<< "Region eta center: " <<   reg_eta_center << std::endl;      
        // 0 - input cluster selection
        EmCaloObj emcalo_sel[NEMCALOSEL_EGIN];
        EmCaloObj emcalo_sel_ref[NEMCALOSEL_EGIN];
        sel_emCalo(emcalo, emcalo_sel);
        algo.sel_emCalo_ref(emcalo, emcalo_sel_ref);


        // FIXME: determine output depth
        int emCalo2tk_ref[NEMCALOSEL_EGIN], emCalo2tk[NEMCALOSEL_EGIN];
        for(int id = 0; id < NEMCALOSEL_EGIN; id++) {
          emCalo2tk_ref[id] = -1;
          emCalo2tk[id] = -1;
        }


        int emCalo2emcalo_ref[NEMCALOSEL_EGIN], emCalo2emcalo[NEMCALOSEL_EGIN];
        for(int id = 0; id < NEMCALOSEL_EGIN; id++) {
          emCalo2emcalo_ref[id] = -1;
          emCalo2emcalo[id] = -1;
        }

        // pftkegalgo(emcalo, track);
        EGIsoParticle egphs_ref[NEM_EGOUT];
        EGIsoEleParticle egele_ref[NEM_EGOUT];
        algo.pftkegalgo_ref(cfg, emcalo_sel_ref, track, emCalo2tk_ref, emCalo2emcalo_ref, egphs_ref, egele_ref);

        ap_uint<NTRACK> emCalo2tk_bit[NEMCALOSEL_EGIN];
        ap_uint<NEMCALOSEL_EGIN> emCalo2emcalo_bit[NEMCALOSEL_EGIN];
        EGIsoParticle egphs[NEM_EGOUT];
        EGIsoEleParticle egele[NEM_EGOUT];

        for(int i = 0; i < NEMCALOSEL_EGIN; i++) {
          emCalo2tk_bit[i] = 0;
          emCalo2emcalo_bit[i] = 0;
        }

        #ifndef REG_Barrel
        link_emCalo2emCalo(emcalo_sel, emCalo2emcalo_bit);
        #endif
        
        link_emCalo2tk(emcalo_sel, track, emCalo2tk_bit);

        pftkegalgo(emcalo, track, egphs, egele) ;

        for(int ic = 0; ic < NEMCALOSEL_EGIN; ic++) {
          for(int it = 0; it < NTRACK; it++) {
            if(emCalo2tk_bit[ic][it])
              emCalo2tk[ic] = it;
          }
        }

        for(int ic = 0; ic < NEMCALOSEL_EGIN; ic++) {
          if(emCalo2emcalo_bit[ic][ic]) continue;
          for(int it = 0; it < NEMCALOSEL_EGIN; it++) {
            if(emCalo2emcalo_bit[ic][it])
              emCalo2emcalo[it] = ic;
          }
        }

        // -----------------------------------------
        // validation against the reference algorithm
        int errors = 0;
        // int ntot = 0, nch = 0, nneu = 0, nmu = 0;
        for(int i = 0; i < NEMCALOSEL_EGIN; i++) {
          if (!emcalo_equal(emcalo_sel_ref[i], emcalo_sel[i])) {
            std::cout << "emcalo_sel [" << i << "]" << " REF: " << emcalo_sel_ref[i].hwPt << " FW: " << emcalo_sel[i].hwPt << std::endl;
          errors++;
          }
        }

        for(int i = 0; i < NEMCALOSEL_EGIN; i++) {
          if (emCalo2tk_ref[i] != emCalo2tk[i]) {
            std::cout << "emCalo2tk [" << i << "]" << " REF: " << emCalo2tk_ref[i] << " FW: " << emCalo2tk[i] << std::endl;
            errors++;
          }
        }

        for(int i = 0; i < NEMCALOSEL_EGIN; i++) {
          if (emCalo2emcalo_ref[i] != emCalo2emcalo[i]) {
            std::cout << "emCalo2emcalo [" << i << "]" << " REF: " << emCalo2emcalo_ref[i] << " FW: " << emCalo2emcalo[i] << std::endl;
            errors++;
          }
        }

        for(int it = 0; it < NEM_EGOUT; it++) {
          if(egele_ref[it].hwPt > 0) {
            nEles_tests++;
            if(!compare(egele_ref[it], egele[it])) {
              errors++;
              std::cout << "[" << it << "] REF ele pt: " << egele_ref[it].hwPt << " FW ele pt: " << egele[it].hwPt << std::endl;
              std::cout << "    REF ele Z0: " << egele_ref[it].hwZ0 << " FW ele z0: " << egele[it].hwZ0 << std::endl;
            }
          }
        }

        for(int it = 0; it < NEM_EGOUT; it++) {
          if(egphs_ref[it].hwPt > 0) {
            nPhotons_tests++;
            if(!compare(egphs_ref[it], egphs[it])) {
              errors++;
              std::cout << "[" << it << "] REF photon pt: " << egphs_ref[it].hwPt << " FW photon pt: " << egphs[it].hwPt << std::endl;
            }

          }
        }


        if (errors != 0) {
            printf("Error in computing test %d (%d)\n", test, errors);
        //     printf("Inputs: \n"); debugHR.dump_inputs(calo, track, mu);
        //     printf("Reference output: \n"); debugHR.dump_outputs(outch_ref, outne_ref, outmupf_ref);
        //     printf("Current output: \n"); debugHR.dump_outputs(outch, outne, outmupf);
            return 1;
        } else {
          passed_tests++;
            // printf("Passed test %d\n", test);
        }
    }
    printf("Passed %d tests (%d electrons, %d photons)\n", passed_tests, nEles_tests, nPhotons_tests);

    return 0;
}
