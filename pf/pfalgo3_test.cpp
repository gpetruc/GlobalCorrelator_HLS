#include <cstdio>
#include "firmware/pfalgo3.h"
#include "ref/pfalgo3_ref.h"
#include "../utils/DumpFileReader.h"
#include "../utils/pattern_serializer.h"
#include "../utils/test_utils.h"

#define NTEST 1000

using namespace l1ct;

int main() {
    HumanReadablePatternSerializer debugHR("-", /*zerosuppress=*/true); // this will print on stdout, we'll use it for errors

    DumpFileReader inputs("TTbar_PU200_Barrel.dump");
    
    // input TP objects
    PFRegion region;
    HadCaloObj hadcalo[NCALO]; EmCaloObj emcalo[NEMCALO]; TkObj track[NTRACK]; z0_t hwZPV;
    MuObj mu[NMU];

    // output PF objects
    PFChargedObj outch[NTRACK], outch_ref[NTRACK];
    PFNeutralObj outne[NSELCALO], outne_ref[NSELCALO];
    PFNeutralObj outpho[NPHOTON], outpho_ref[NPHOTON];
    PFChargedObj outmupf[NMU], outmupf_ref[NMU];

    pfalgo3_config cfg(NTRACK,NEMCALO,NCALO,NMU, 
                       NPHOTON,NSELCALO,NALLNEUTRALS,
                       PFALGO_DR2MAX_TK_MU, PFALGO_DR2MAX_TK_EM, PFALGO_DR2MAX_EM_CALO, PFALGO_DR2MAX_TK_CALO,
                       Scales::makePt(PFALGO_TK_MAXINVPT_LOOSE), Scales::makePt(PFALGO_TK_MAXINVPT_TIGHT));
    const float ptErr_edges[PTERR_BINS]  = PTERR_EDGES;
    const float ptErr_offss[PTERR_BINS]  = PTERR_OFFS;
    const float ptErr_scales[PTERR_BINS] = PTERR_SCALE;
    cfg.loadPtErrBins(PTERR_BINS, ptErr_edges, ptErr_scales, ptErr_offss);   
 
#ifndef BOARD_none
    printf("Multiplicities per region: Region %d, Tk %d, EmCalo %d, HadCalo %d, Mu %d => %d, PFCharged %d, PFPhoton %d, PFNeutral %d, PFMu %d => %d\n",
        1, NTRACK, NEMCALO, NCALO, NMU, PFALGO3_NCHANN_IN,
        NTRACK, NPHOTON, NSELCALO, NMU, PFALGO3_NCHANN_OUT);

    printf("Packing bit sizes: EmCalo %3d, HadCalo %3d, Tk %3d, Mu %3d, PFCharged %3d, PFNeutral %3d\n",
        EmCaloObj::BITWIDTH, HadCaloObj::BITWIDTH, TkObj::BITWIDTH, MuObj::BITWIDTH, PFChargedObj::BITWIDTH, PFNeutralObj::BITWIDTH);

    const unsigned int nchann64_in = 2*PFALGO3_NCHANN_IN, nchann64_out = 2*PFALGO3_NCHANN_OUT, nmux = 1;
    PatternSerializer serPatternsIn("pfalgo3_input_patterns.txt", nchann64_in, nmux), serPatternsOut("pfalgo3_output_patterns.txt", nchann64_out, nmux);
    ap_uint<PFALGO3_DATA_SIZE> packed_input[PFALGO3_NCHANN_IN], packed_output[PFALGO3_NCHANN_OUT];
    std::fill(packed_input, packed_input+PFALGO3_NCHANN_IN, 0);
    std::fill(packed_output, packed_output+PFALGO3_NCHANN_OUT, 0);
#endif
    HumanReadablePatternSerializer debugDump("pfalgo3_debug.txt");

    // -----------------------------------------
    // run multiple tests
    for (int test = 1; test <= NTEST; ++test) {
        // get the inputs from the input object
        if (!inputs.nextRegion(region, hadcalo, emcalo, track, mu, hwZPV)) break;

        bool verbose = (test < 10); // can set this on to get detailed printout of some test
        pfalgo3_set_debug(verbose);
        pfalgo3_ref_set_debug(verbose);

#ifndef BOARD_none
        pfalgo3_pack_in(region, emcalo, hadcalo, track, mu, packed_input); 
        serPatternsIn.packAndWrite(PFALGO3_NCHANN_IN, packed_input);
        packed_pfalgo3(packed_input, packed_output);
        serPatternsOut.packAndWrite(PFALGO3_NCHANN_OUT, packed_output);
        pfalgo3_unpack_out(packed_output, outch, outpho, outne, outmupf);
#else
        pfalgo3(region, emcalo, hadcalo, track, mu, outch, outpho, outne, outmupf);
#endif

        pfalgo3_ref(cfg, region, emcalo, hadcalo, track, mu, outch_ref, outpho_ref, outne_ref, outmupf_ref);

        // -----------------------------------------
        // validation against the reference algorithm
        int errors = 0; int ntot = 0, nch = 0, npho = 0, nneu = 0, nmu = 0;

        for (int i = 0; i < NTRACK; ++i) {
            if (!pf_equals(outch_ref[i], outch[i], "PF Charged", i)) errors++;
            if (outch_ref[i].hwPt > 0) { ntot++; nch++; }
        }
        for (int i = 0; i < NPHOTON; ++i) {
            if (!pf_equals(outpho_ref[i], outpho[i], "Photon", i)) errors++;
            if (outpho_ref[i].hwPt > 0) { ntot++; npho++; }
        }        
        for (int i = 0; i < NSELCALO; ++i) {
            if (!pf_equals(outne_ref[i], outne[i], "PF Neutral", i)) errors++;
            if (outne_ref[i].hwPt > 0) { ntot++; nneu++; }
        }
        for (int i = 0; i < NMU; ++i) {
            if (!pf_equals(outmupf_ref[i], outmupf[i], "PF Muon", i)) errors++;
            if (outmupf_ref[i].hwPt > 0) { ntot++; nmu++; }
        }        

        debugDump.dump_inputs(emcalo, hadcalo, track, mu);
        debugDump.dump_outputs(outch, outpho, outne, outmupf);

        if (errors != 0) {
            printf("Error in computing test %d (%d)\n", test, errors);
            printf("Inputs: \n"); debugHR.dump_inputs(emcalo, hadcalo, track, mu);
            printf("Reference output: \n"); debugHR.dump_outputs(outch_ref, outpho_ref, outne_ref, outmupf_ref);
            printf("Current output: \n"); debugHR.dump_outputs(outch, outpho, outne, outmupf);
            return 1;
        } else {
            printf("Passed test %d (%d, %d, %d, %d)\n", test, ntot, nch, npho, nneu);
        }

    }
    return 0;
}
