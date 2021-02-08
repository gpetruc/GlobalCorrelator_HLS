#include <cstdio>
#include "firmware/linpuppi.h"
#include "linpuppi_ref.h"
#include "../utils/DumpFileReader.h"
#include "../utils/pattern_serializer.h"
#include "../utils/test_utils.h"
#include "puppi_checker.h"

#if defined(REG_Barrel)
    #include "../pf/firmware/pfalgo3.h"
    #include "../pf/ref/pfalgo3_ref.h"
#elif defined(REG_HGCal)
    #include "../pf/firmware/pfalgo2hgc.h"
    #include "../pf/ref/pfalgo2hgc_ref.h"
#endif

#define NTEST 1000

using namespace l1ct;

int main() {
#if defined(REG_Barrel)
    DumpFileReader inputs("TTbar_PU200_Barrel.dump");
    PFAlgo3Emulator pfEmulator(NTRACK,NEMCALO,NCALO,NMU, 
                         NPHOTON,NSELCALO,NALLNEUTRALS,
                         PFALGO_DR2MAX_TK_MU, PFALGO_DR2MAX_TK_EM, PFALGO_DR2MAX_EM_CALO, PFALGO_DR2MAX_TK_CALO,
                         PFALGO_TK_MAXINVPT_LOOSE, PFALGO_TK_MAXINVPT_TIGHT);
    LinPuppiEmulator puEmulator(NTRACK, NALLNEUTRALS, NNEUTRALS,
                          LINPUPPI_DR2MIN, LINPUPPI_DR2MAX, LINPUPPI_ptMax, LINPUPPI_dzCut,
                          LINPUPPI_ptSlopeNe, LINPUPPI_ptSlopePh, LINPUPPI_ptZeroNe, LINPUPPI_ptZeroPh, 
                          LINPUPPI_alphaSlope, LINPUPPI_alphaZero, LINPUPPI_alphaCrop, 
                          LINPUPPI_priorNe, LINPUPPI_priorPh,
                          Scales::makePt(LINPUPPI_ptCut));
    printf("Multiplicities per region: Tk %d, EmCalo %d, HadCalo %d, Mu %d, PFCharged %d, PFPhoton %d, PFNeutral %d, PFMu %d, Puppi All %d => Sel %d\n",
        NTRACK, NEMCALO, NCALO, NMU, NTRACK, NPHOTON, NSELCALO, NMU, NALLNEUTRALS, NNEUTRALS);
#elif defined(REG_HGCal)
    DumpFileReader inputs("TTbar_PU200_HGCal.dump");
    PFAlgo2HGCEmulator pfEmulator(NTRACK,NCALO,NMU, NSELCALO,
                        PFALGO_DR2MAX_TK_MU, PFALGO_DR2MAX_TK_CALO,
                        PFALGO_TK_MAXINVPT_LOOSE, PFALGO_TK_MAXINVPT_TIGHT);
    LinPuppiEmulator puEmulator(NTRACK, NALLNEUTRALS, NNEUTRALS,
                          LINPUPPI_DR2MIN, LINPUPPI_DR2MAX, LINPUPPI_ptMax, LINPUPPI_dzCut,
                          LINPUPPI_etaCut, LINPUPPI_invertEta,
                          LINPUPPI_ptSlopeNe, LINPUPPI_ptSlopeNe_1, LINPUPPI_ptSlopePh, LINPUPPI_ptSlopePh_1, 
                          LINPUPPI_ptZeroNe, LINPUPPI_ptZeroNe_1, LINPUPPI_ptZeroPh, LINPUPPI_ptZeroPh_1, 
                          LINPUPPI_alphaSlope, LINPUPPI_alphaSlope_1, LINPUPPI_alphaZero, LINPUPPI_alphaZero_1, LINPUPPI_alphaCrop, LINPUPPI_alphaCrop_1, 
                          LINPUPPI_priorNe, LINPUPPI_priorNe_1, LINPUPPI_priorPh, LINPUPPI_priorPh_1,
                          Scales::makePt(LINPUPPI_ptCut), Scales::makePt(LINPUPPI_ptCut_1));
#endif
    
    // input TP objects and PV
    //PFRegion region;
    TkObj track[NTRACK]; 
    z0_t hwZPV;

    // PF objects
    l1ct::OutputRegion pfout;
    PFChargedObj pfch[NTRACK];
    PFNeutralObj pfallne[NALLNEUTRALS];

    // Puppi objects
    PuppiObj outallch[NTRACK];
    PuppiObj outallne[NALLNEUTRALS];
    PuppiObj outselne[NNEUTRALS];
    std::vector<PuppiObjEmu> outallch_ref;
    std::vector<PuppiObjEmu> outallne_ref_nocut, outallne_ref, outallne_flt_nocut, outallne_flt;
    std::vector<PuppiObjEmu> outselne_ref, outselne_flt;

#if !defined(BOARD_none) && !defined(TEST_PUPPI_STREAM)
    printf("Packing bit sizes: EmCalo %d, HadCalo %d, Tk %d, Mu %d, PFCharged %d, PFNeutral %d, Puppi %d\n",
        EmCaloObj::BITWIDTH, HadCaloObj::BITWIDTH, TkObj::BITWIDTH, MuObj::BITWIDTH, PFChargedObj::BITWIDTH, PFNeutralObj::BITWIDTH, PuppiObj::BITWIDTH);

    const unsigned int nchann64_in = 2*LINPUPPI_NCHANN_IN, nchann64_chs_in = 2*LINPUPPI_CHS_NCHANN_IN, nmux = 1;
    PatternSerializer serPatternsIn("linpuppi_input_patterns.txt", nchann64_in, nmux), serPatternsOut("linpuppi_output_patterns.txt", LINPUPPI_NCHANN_OUTNC, nmux);
    PatternSerializer serPatternsChsIn("linpuppi_chs_input_patterns.txt", nchann64_chs_in, nmux), serPatternsChsOut("linpuppi_chs_output_patterns.txt", LINPUPPI_CHS_NCHANN_OUT, nmux);
    ap_uint<LINPUPPI_DATA_SIZE_IN> packed_input[LINPUPPI_NCHANN_IN], packed_input_chs[LINPUPPI_CHS_NCHANN_IN];
    ap_uint<LINPUPPI_DATA_SIZE_OUT> packed_output[LINPUPPI_NCHANN_OUTNC], packed_output_chs[LINPUPPI_CHS_NCHANN_OUT];
    std::fill(packed_input, packed_input+LINPUPPI_NCHANN_IN, 0);
    std::fill(packed_output, packed_output+LINPUPPI_NCHANN_OUTNC, 0);
    std::fill(packed_input_chs, packed_input_chs+LINPUPPI_CHS_NCHANN_IN, 0);
    std::fill(packed_output_chs, packed_output_chs+LINPUPPI_CHS_NCHANN_OUT, 0);
#endif
    HumanReadablePatternSerializer debugDump("linpuppi_output.txt",true);

    PuppiChecker checker;

    for (int test = 1; test <= NTEST; ++test) {
        // get the inputs from the input object
        if (!inputs.nextPFRegion()) break;

        const PVObjEmu & pv = inputs.event().pvs.front();
        hwZPV = pv.hwZ0;
        
#ifdef TEST_PT_CUT
        float minpt = 0;
        for (const auto & tk : inputs.pfregion().track)  minpt += tk.floatPt();
        if (minpt < TEST_PT_CUT) { 
            //std::cout << "Skipping region with total calo pt " << minpt << " below threshold." << std::endl; 
            --test; continue; 
        }
#endif

        pfEmulator.run(inputs.pfregion(), pfout);
        pfEmulator.mergeNeutrals(pfout);
        l1ct::toFirmware(inputs.pfregion().track, NTRACK, track);
        l1ct::toFirmware(pfout.pfcharged, NTRACK, pfch);
        l1ct::toFirmware(pfout.pfneutral, NALLNEUTRALS, pfallne);

        bool verbose = (test <= 15);
        if (verbose) printf("test case %d\n", test);
        linpuppi_set_debug(verbose);
        puEmulator.setDebug(verbose);

#if defined(TEST_PUPPI_STREAM)
    #if !defined(BOARD_none)
        packed_linpuppiNoCrop_streamed(track, hwZPV, pfallne, outallne);
        packed_linpuppi_chs_streamed(hwZPV, pfch, outallch);  // we call this again, with the streamed version
    #else
        linpuppiNoCrop_streamed(track, hwZPV, pfallne, outallne);
        linpuppi_chs_streamed(hwZPV, pfch, outallch);  // we call this again, with the streamed version
    #endif
#elif !defined(BOARD_none)
        linpuppi_chs_pack_in(hwZPV, pfch, packed_input_chs); 
        linpuppi_pack_in(track, hwZPV, pfallne, packed_input); 
        serPatternsChsIn.packAndWrite(LINPUPPI_CHS_NCHANN_IN, packed_input_chs);
        serPatternsIn.packAndWrite(LINPUPPI_NCHANN_IN, packed_input);
        packed_linpuppi_chs(packed_input_chs, packed_output_chs);
    #if defined(TEST_PUPPI_NOCROP)
        packed_linpuppiNoCrop(packed_input, packed_output);
        serPatternsOut.packAndWrite(NALLNEUTRALS, packed_output); 
        l1pf_pattern_unpack<NALLNEUTRALS,0>(packed_output, outallne);
    #else
        packed_linpuppi(packed_input, packed_output);
        serPatternsOut.packAndWrite(NNEUTRALS, packed_output); 
        l1pf_pattern_unpack<NNEUTRALS,0>(packed_output, outselne);
    #endif
        l1pf_pattern_unpack<NTRACK,0>(packed_output_chs, outallch);
        serPatternsChsOut.packAndWrite(NTRACK,packed_output_chs);
#else
        linpuppi_chs(hwZPV, pfch, outallch);
    #if defined(TEST_PUPPI_NOCROP)
        linpuppiNoCrop(track, hwZPV, pfallne, outallne);
    #elif defined(TEST_PUPPI_STREAM)
        linpuppiNoCrop_streamed(track, hwZPV, pfallne, outallne);
        linpuppi_chs_streamed(hwZPV, pfch, outallch); // we call this again, with the streamed version
    #else
        linpuppi(track, hwZPV, pfallne, outselne);
    #endif
#endif

        puEmulator.linpuppi_chs_ref(pv, pfout.pfcharged, outallch_ref);
        puEmulator.linpuppi_ref(inputs.pfregion().track, pv, pfout.pfneutral, outallne_ref_nocut, outallne_ref, outselne_ref);
        puEmulator.linpuppi_flt(inputs.pfregion().track, pv, pfout.pfneutral, outallne_flt_nocut, outallne_flt, outselne_flt);

        // validate numerical accuracy 
        checker.checkIntVsFloat(pfout.pfneutral, outallne_ref_nocut, outallne_flt_nocut, verbose);

        bool ok = checker.checkChs<NTRACK>(hwZPV, outallch, outallch_ref) && 
#if defined(TEST_PUPPI_NOCROP) or defined(TEST_PUPPI_STREAM)
                  checker.check<NALLNEUTRALS>(outallne, outallne_ref, outallne_flt);
#else
                  checker.check<NNEUTRALS>(outselne, outselne_ref, outselne_flt);
#endif

#if defined(TEST_PUPPI_NOCROP) or defined(TEST_PUPPI_STREAM)
        debugDump.dump_puppi(NALLNEUTRALS, "all    ", outallne);
#else
        debugDump.dump_puppi(NNEUTRALS,    "sel    ", outselne);
#endif
        debugDump.dump_puppi("all rnc", outallne_ref_nocut);
        debugDump.dump_puppi("all flt", outallne_flt_nocut);

        if (!ok) {
            printf("FAILED test %d\n", test);
            HumanReadablePatternSerializer dumper("-", true);
#if defined(TEST_PUPPI_NOCROP) or defined(TEST_PUPPI_STREAM)
            dumper.dump_puppi(NALLNEUTRALS, "all    ", outallne);
            dumper.dump_puppi("all ref", outallne_ref);
#else
            dumper.dump_puppi(NNEUTRALS,    "sel    ", outselne);
            dumper.dump_puppi("sel ref", outselne_ref);
#endif
            dumper.dump_puppi("all rnc", outallne_ref_nocut);
            dumper.dump_puppi("all flt", outallne_flt_nocut);
            return 1;
        }

        if (verbose) printf("\n");
        else         printf("passed test %d\n", test);

    }

    printf("Report for %d regions (cropped at N=%d):\n", NTEST, NALLNEUTRALS);
    checker.printIntVsFloatReport();
    return 0;
}
