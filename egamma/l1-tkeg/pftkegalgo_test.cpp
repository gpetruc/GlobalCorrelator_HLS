#include <cstdio>
#include "firmware/pftkegalgo.h"

#include "ref/pftkegalgo_ref.h"

#include "../../utils/DumpFileReader.h"
#include "../../utils/pattern_serializer.h"
#include "../../utils/test_utils.h"

#define NTEST 10000

using namespace l1ct;


int main() {
    HumanReadablePatternSerializer debugHR("-", /*zerosuppress=*/true); // this will print on stdout, we'll use it for errors

#if defined(REG_HGCal)
    std::cout << "REG_Hgcal" << std::endl;
    DumpFileReader inputs("TTbar_PU200_HGCal.dump");    
    bool filterHwQuality = true;
    bool doBremRecovery = true;
    float emClusterPtMin = 0.;
#endif

#if defined(REG_Barrel)
    std::cout << "REG_Barrel" << std::endl;
    DumpFileReader inputs("TTbar_PU200_Barrel.dump");
    bool filterHwQuality = false;
    bool doBremRecovery = false;
    float emClusterPtMin = 2.;
#endif

  int caloHwQual = 4;
  float dEtaMaxBrem = 0.02;
  float dPhiMaxBrem = 0.1;
  std::vector<float> absEtaBoundaries={0.0, 1.5};
  std::vector<float> dEtaValues={0.015, 0.0174533};
  std::vector<float> dPhiValues={0.07, 0.07};
  
  // FIXME: need region center in FW, we don't have it for now hence we keep 1 value for the barrel
  // std::vector<float> absEtaBoundaries_{0.0, 0.9, 1.5};
  //  // FIXME: should be {0.025, 0.015, 0.01}  but 0.01 it is too small give eta precision. We use  0.0174533 hwEta: 4
  // std::vector<float> dEtaValues_{0.025, 0.015, 0.0174533};
  // std::vector<float> dPhiValues_{0.07, 0.07, 0.07};
  
  float trkQualityPtMin = 10.;

  // input TP objects
  PFRegion region;
  EmCaloObj emcalo[NEMCALO]; TkObj track[NTRACK]; 

  // z0_t hwZPV;
  // MuObj mu[NMU];

  // output PF objects
  l1ct::OutputRegion out_ref; // emulator
  EGIsoObj egphs_ref[NEM_EGOUT];
  EGIsoEleObj egele_ref[NEM_EGOUT];
  EGIsoObj egphs[NEM_EGOUT];
  EGIsoEleObj egele[NEM_EGOUT];


  l1ct::pftkegalgo_config cfg(
    NTRACK, NEMCALO, NEMCALO_EGIN, NEM_EGOUT, 
    filterHwQuality, doBremRecovery, caloHwQual, emClusterPtMin,
    dEtaMaxBrem, dPhiMaxBrem, absEtaBoundaries, dEtaValues, dPhiValues, trkQualityPtMin);

  l1ct::PFTkEGAlgoEmulator emulator(cfg);
  // -----------------------------------------
  // run multiple tests
  int passed_tests = 0;
  int nEles_tests = 0;
  int nPhotons_tests = 0;

  int tot_errors = 0;
  for (int test = 1; test <= NTEST; ++test) {
    // std::cout << "------------ TEST: " << test << std::endl;
      // get the inputs from the input object
      if (!inputs.nextPFRegion()) break;

      int debug=0;
      if(test < 1) debug=5;
      
      emulator.setDebug(debug);

      emulator.toFirmware(inputs.pfregion(), region, emcalo, track);

      int errors = 0;
      
      emulator.run(inputs.pfregion(), out_ref);
      emulator.toFirmware(out_ref, egphs_ref, egele_ref);

      pftkegalgo(region, emcalo, track, egphs, egele) ;

      for(int it = 0; it < NEM_EGOUT; it++) {
        if(!egisoele_equals(egele_ref[it], egele[it], "EG Iso Ele", it)) errors++;
        if(egele_ref[it].hwPt > 0) nEles_tests++;
      }

      for(int it = 0; it < NEM_EGOUT; it++) {
        if(!egiso_equals(egphs_ref[it], egphs[it], "EG Iso", it)) errors++;
        if(egphs_ref[it].hwPt > 0) nPhotons_tests++;
      }
      
      if (errors != 0) {
        tot_errors += errors;
          printf("Error in computing test %d (%d)\n", test, errors);
          printf("    Passed %d tests (%d electrons, %d photons)\n", passed_tests, nEles_tests, nPhotons_tests);

      //     printf("Inputs: \n"); debugHR.dump_inputs(calo, track, mu);
      //     printf("Reference output: \n"); debugHR.dump_outputs(outch_ref, outne_ref, outmupf_ref);
      //     printf("Current output: \n"); debugHR.dump_outputs(outch, outne, outmupf);
        return 1;
      } else {
        passed_tests++;
        // printf("Passed test %d\n", test);
      }
  }
  std::cout << "# of errors: " << tot_errors << std::endl;
  printf("Passed %d tests (%d electrons, %d photons)\n", passed_tests, nEles_tests, nPhotons_tests);

  return 0;
}
