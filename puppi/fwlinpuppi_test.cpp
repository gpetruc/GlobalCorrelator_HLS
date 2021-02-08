#include <cstdio>
#include "firmware/linpuppi.h"
#include "linpuppi_ref.h"
#include "../utils/DumpFileReader.h"
#include "../utils/pattern_serializer.h"
#include "../utils/test_utils.h"
#include "puppi_checker.h"

#define NTEST 500

using namespace l1ct;

int main() {
#if defined(REG_HGCalNoTK)
    DumpFileReader inputs("TTbar_PU200_HGCalNoTK.dump");
    //DumpFileReader inputs("VBFHToBB_PU200_HGCalNoTK.dump");
#elif defined(REG_HF)
    DumpFileReader inputs("TTbar_PU200_HF.dump");
    //DumpFileReader inputs("VBFHToBB_PU200_HF.dump");
#endif

    LinPuppiEmulator puEmulator(NTRACK, NCALO, NNEUTRALS,
                        LINPUPPI_DR2MIN, LINPUPPI_DR2MAX, LINPUPPI_ptMax, LINPUPPI_dzCut,
                        LINPUPPI_ptSlopeNe, LINPUPPI_ptSlopePh, LINPUPPI_ptZeroNe, LINPUPPI_ptZeroPh, 
                        LINPUPPI_alphaSlope, LINPUPPI_alphaZero, LINPUPPI_alphaCrop, 
                        LINPUPPI_priorNe, LINPUPPI_priorPh,
                        Scales::makePt(LINPUPPI_ptCut));
    
    // input TP objects (used)
    //PFRegion region;
    HadCaloObj calo[NCALO];

    // input/output PFPUPPI objects
    PuppiObj outselne[NNEUTRALS];
    std::vector<PuppiObjEmu> outselne_ref, outselne_flt;
    PuppiObj outallne[NCALO];
    std::vector<PuppiObjEmu> outallne_ref_nocut, outallne_ref;
    std::vector<PuppiObjEmu> outallne_flt_nocut, outallne_flt;

#ifndef BOARD_none
    PatternSerializer serPatternsIn("fwlinpuppi_input_patterns.txt", LINPUPPI_NCHANN_FWDNC);
    PatternSerializer serPatternsOut("fwlinpuppi_output_patterns.txt", LINPUPPI_NCHANN_FWDNC);
    ap_uint<LINPUPPI_DATA_SIZE_FWD> packed_input[LINPUPPI_NCHANN_FWDNC], packed_output[LINPUPPI_NCHANN_FWDNC];
    for (unsigned int i = 0; i < LINPUPPI_NCHANN_FWDNC; ++i) { packed_input[i] = 0; packed_output[i] = 0; }
#endif
    HumanReadablePatternSerializer debugDump("linpuppi_output.txt",true);

    PuppiChecker checker;

    for (int test = 1; test <= NTEST; ++test) {
        // get the inputs from the input object
        if (!inputs.nextPFRegion()) break;
#ifdef TEST_PT_CUT
        float minpt = 0;
        for (const auto & calo : inputs.pfregion().hadcalo) minpt += calo.floatPt();
        if (minpt < TEST_PT_CUT) { 
            //std::cout << "Skipping region with total calo pt " << minpt << " below threshold." << std::endl; 
            --test; continue; 
        }
#endif

        //region = inputs.pfregion().region;
        l1ct::toFirmware(inputs.pfregion().hadcalo, NCALO, calo);

        bool verbose = 0;
        if (verbose) printf("test case %d\n", test);
        linpuppi_set_debug(verbose);
        puEmulator.setDebug(verbose);

#ifndef BOARD_none
        l1pf_pattern_pack<NCALO,0>(calo, packed_input);
        serPatternsIn(packed_input);
  #if defined(TEST_PUPPI_NOCROP)
        packed_fwdlinpuppiNoCrop(packed_input, packed_output);
        l1pf_pattern_unpack<NCALO,0>(packed_output, outallne);
  #else
        packed_fwdlinpuppi(packed_input, packed_output);
        l1pf_pattern_unpack<NNEUTRALS,0>(packed_output, outselne);
  #endif
        serPatternsOut(packed_output);
#else
  #if defined(TEST_PUPPI_NOCROP)
        fwdlinpuppiNoCrop(calo, outallne);
  #else
        fwdlinpuppi(calo, outselne);
  #endif
#endif

        puEmulator.fwdlinpuppi_ref(inputs.pfregion().hadcalo, outallne_ref_nocut, outallne_ref, outselne_ref);
        puEmulator.fwdlinpuppi_flt(inputs.pfregion().hadcalo, outallne_flt_nocut, outallne_flt, outselne_flt);

        // validate numerical accuracy 
        checker.checkIntVsFloat(inputs.pfregion().hadcalo, outallne_ref_nocut, outallne_flt_nocut, verbose);

#if defined(TEST_PUPPI_NOCROP)
        debugDump.dump_puppi(NALLNEUTRALS, "all    ", outallne);
#else
        debugDump.dump_puppi(NNEUTRALS,    "sel    ", outselne);
#endif
        debugDump.dump_puppi("all rnc", outallne_ref_nocut);
        debugDump.dump_puppi("all flt", outallne_flt_nocut);


        // check vs reference
#if defined(TEST_PUPPI_NOCROP)
        bool ok = checker.check<NCALO>(outallne, outallne_ref, outallne_flt);
#else
        bool ok = checker.check<NSELCALO>(outselne, outselne_ref, outselne_flt);
#endif
        if (!ok) {
            printf("FAILED test %d\n", test);
            HumanReadablePatternSerializer dumper("-", true);
            dumper.dump_puppi(NCALO, "    ", outallne);
            dumper.dump_puppi("ref ", outallne_ref);
            dumper.dump_puppi("rnc ", outallne_ref_nocut);
            dumper.dump_puppi("flt ", outallne_flt_nocut);
            return 1;
        }

        if (verbose) printf("\n");
        else         printf("passed test %d\n", test);

    }

    printf("Report for %d regions (cropped at NCALO=%d):\n", NTEST, NCALO);
    checker.printIntVsFloatReport();
    return 0;
}
