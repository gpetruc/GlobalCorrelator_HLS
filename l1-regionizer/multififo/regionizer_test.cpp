#include "firmware/regionizer.h"
#include "../../utils/pattern_serializer.h"
#include "../../utils/test_utils.h"
#include "../../utils/DumpFileReader.h"
#include "regionizer_ref.h"
#include "regionizer_new_ref.h"

#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <vector>


#define TLEN REGIONIZERNCLOCKS 

using namespace l1ct;

int main(int argc, char **argv) {
    DumpFileReader inputs("TTbar_PU200_HGCal.dump");
    FILE *fin_tk   = fopen("input-tk.txt", "w");
    FILE *fin_calo = fopen("input-calo.txt", "w");
    FILE *fin_mu = fopen("input-mu.txt", "w");
    FILE *fout_tk   = fopen("output-tk.txt", "w");
    FILE *fout_calo = fopen("output-calo.txt", "w");
    FILE *fout_mu = fopen("output-mu.txt", "w");
    FILE *fref_tk   = fopen("output-ref-tk.txt", "w");
    FILE *fref_calo = fopen("output-ref-calo.txt", "w");
    FILE *fref_mu = fopen("output-ref-mu.txt", "w");
    FILE *fold_tk   = fopen("output-old-tk.txt", "w");
    FILE *fold_calo = fopen("output-old-calo.txt", "w");
    FILE *fold_mu = fopen("output-old-mu.txt", "w");

#if 0
    PatternSerializer serPatternsIn("input-emp.txt"), serPatternsOut("output-emp.txt"), serPatternsRef("output-ref-emp.txt");
    assert(PACKING_NCHANN >= NTKSECTORS*NTKFIBERS + NCALOSECTORS*NCALOFIBERS + NMUFIBERS);
    assert(PACKING_NCHANN >= NTKOUT + NCALOOUT + NMUOUT);
    ap_uint<64> all_channels_in[PACKING_NCHANN], all_channels_ref[PACKING_NCHANN], all_channels_out[PACKING_NCHANN];
    for (unsigned int i = 0; i < PACKING_NCHANN; ++i) {
        all_channels_in[i] = 0; all_channels_ref[i] = 0; all_channels_out[i] = 0; 
    }
    serPatternsIn(all_channels_in, false); // prepend one null frame at the beginning
#endif

    int frame = 0, pingpong = 1; 
    int tk_latency = -1, calo_latency = -1, mu_latency = -1;

    bool ok = true;

    l1ct::MultififoRegionizerEmulator emulator(/*nendcaps=*/1, REGIONIZERNCLOCKS, NTRACK, NCALO, /*NEM=*/0, NMU, /*streaming=*/false, 6);

    for (int itest = 0; itest < 10; ++itest) {
        TkObj tk_output[NTKOUT][TLEN], tk_output_ref[NTKOUT][2*TLEN];
        HadCaloObj calo_output[NCALOOUT][TLEN], calo_output_ref[NCALOOUT][2*TLEN];
        MuObj mu_output[NMUOUT][TLEN], mu_output_ref[NMUOUT][2*TLEN];

        if (!inputs.nextEvent()) break;
        const auto & decodedObjs = inputs.event().decoded;
        // now we make a single endcap setup
        //
        //
        RegionizerDecodedInputs in; std::vector<PFInputRegion> pfin;
        for (auto & sec : inputs.event().decoded.track) if (sec.region.floatEtaCenter() >= 0) in.track.push_back(sec);
        for (auto & sec : inputs.event().decoded.hadcalo) if (sec.region.floatEtaCenter() >= 0) in.hadcalo.push_back(sec);
        for (auto & sec : inputs.event().decoded.emcalo) if (sec.region.floatEtaCenter() >= 0) in.emcalo.push_back(sec);
        in.muon = inputs.event().decoded.muon;
        for (auto & reg : inputs.event().pfinputs) if (reg.region.floatEtaCenter() >= 0) pfin.push_back(reg);
        const glbeta_t etaCenter = 2*PFREGION_ETA_SIZE; // eta = +2.0

        if (itest == 0) emulator.initSectorsAndRegions(in, pfin);


        for (int i = 0; i < TLEN; ++i, ++frame) {
            std::vector<l1ct::TkObjEmu> tk_links_in_emu, tk_out_oldemu, tk_out_emu;
            std::vector<l1ct::HadCaloObjEmu> calo_links_in_emu, calo_out_oldemu, calo_out_emu;
            std::vector<l1ct::MuObjEmu> mu_links_in_emu, mu_out_oldemu, mu_out_emu;

            emulator.fillLinks(i, in, tk_links_in_emu);
            emulator.fillLinks(i, in, calo_links_in_emu);
            emulator.fillLinks(i, in, mu_links_in_emu);

            l1ct::TkObj tk_links_in[NTKSECTORS][NTKFIBERS];
            l1ct::HadCaloObj    calo_links_in[NCALOSECTORS][NCALOFIBERS];
            l1ct::MuObj    mu_links_in[NMUFIBERS];

            emulator.toFirmware(tk_links_in_emu, tk_links_in);
            emulator.toFirmware(calo_links_in_emu, calo_links_in);
            emulator.toFirmware(mu_links_in_emu, mu_links_in);

            TkObj tk_links_out[NTKOUT];
            HadCaloObj calo_links_out[NCALOOUT];
            MuObj mu_links_out[NMUOUT];

            bool calo_newevt_out, tk_newevt_out, mu_newevt_out, newevt_ref = (i == 0);
            bool tk_ref_good   = emulator.step(newevt_ref, tk_links_in_emu, tk_out_emu);
            bool calo_ref_good = emulator.step(newevt_ref, calo_links_in_emu, calo_out_emu);
            bool mu_ref_good = emulator.step(newevt_ref, mu_links_in_emu, mu_out_emu);
            bool tk_old_good   = tk_router_ref(newevt_ref, tk_links_in_emu, tk_out_oldemu);
            bool calo_old_good = calo_router_ref(newevt_ref, calo_links_in_emu, calo_out_oldemu);
            bool mu_old_good = mu_router_ref(newevt_ref, etaCenter, mu_links_in_emu, mu_out_oldemu);

#if defined(ROUTER_NOMERGE) || defined(ROUTER_NOMUX)
            bool tk_good   = tk_router(i == 0, tk_links64_in, tk_links64_out, tk_newevt_out);
            bool calo_good = calo_router(i == 0, calo_links64_in, calo_links64_out, calo_newevt_out);
            bool mu_good = mu_router(i == 0, etaCenter, mu_links64_in, mu_links64_out, mu_newevt_out);
#else
            // the routers are not implemented in this pattern, and may segfault due to size of the arrays
            bool tk_good = false, calo_good = false, mu_good = false;
#endif


            fprintf(fin_tk,   "%05d %1d   ", frame, int(i==0));
            fprintf(fin_calo, "%05d %1d   ", frame, int(i==0));
            fprintf(fin_mu, "%05d %1d   ", frame, int(i==0));
            for (int s = 0; s < NTKSECTORS; ++s) {
                for (int f = 0; f < NTKFIBERS; ++f) {
                    printTrack(fin_tk, tk_links_in[s][f]);
                }
            }
            for (int s = 0; s < NCALOSECTORS; ++s) {
                for (int f = 0; f < NCALOFIBERS; ++f) {
                    printCalo(fin_calo, calo_links_in[s][f]);
                }
            }
            for (int f = 0; f < NMUFIBERS; ++f) {
                printMu(fin_mu, mu_links_in[f]);
            }
            fprintf(fin_tk, "\n");
            fprintf(fin_calo, "\n");
            fprintf(fin_mu, "\n");
            fprintf(fout_tk, "%5d %1d %1d   ", frame, int(tk_good), int(tk_newevt_out));
            fprintf(fref_tk, "%5d %1d %1d   ", frame, int(tk_ref_good), int(newevt_ref));
            fprintf(fold_tk, "%5d %1d %1d   ", frame, int(tk_old_good), int(newevt_ref));
            fprintf(fout_calo, "%5d %1d %1d   ", frame, int(calo_good), int(calo_newevt_out));
            fprintf(fref_calo, "%5d %1d %1d   ", frame, int(calo_ref_good), int(newevt_ref));
            fprintf(fold_calo, "%5d %1d %1d   ", frame, int(calo_old_good), int(newevt_ref));
            fprintf(fout_mu, "%5d %1d %1d   ", frame, int(mu_good), int(mu_newevt_out));
            fprintf(fref_mu, "%5d %1d %1d   ", frame, int(mu_ref_good), int(newevt_ref));
            fprintf(fold_mu, "%5d %1d %1d   ", frame, int(mu_old_good), int(newevt_ref));
            for (int r = 0; r < NTKOUT; ++r) printTrack(fout_tk, tk_links_out[r]);
            for (int r = 0; r < NTKOUT; ++r) printTrack(fref_tk, tk_out_emu[r]);
            for (int r = 0; r < NTKOUT; ++r) printTrack(fold_tk, tk_out_oldemu[r]);
            for (int r = 0; r < NCALOOUT; ++r) printCalo(fout_calo, calo_links_out[r]);
            for (int r = 0; r < NCALOOUT; ++r) printCalo(fref_calo, calo_out_emu[r]);
            for (int r = 0; r < NCALOOUT; ++r) printCalo(fold_calo, calo_out_oldemu[r]);
            for (int r = 0; r < NMUOUT; ++r) printMu(fout_mu, mu_links_out[r]);
            for (int r = 0; r < NMUOUT; ++r) printMu(fref_mu, mu_out_emu[r]);
            for (int r = 0; r < NMUOUT; ++r) printMu(fold_mu, mu_out_oldemu[r]);
            fprintf(fout_tk, "\n");
            fprintf(fout_calo, "\n");
            fprintf(fout_mu, "\n");
            fprintf(fref_tk, "\n");
            fprintf(fref_calo, "\n");
            fprintf(fref_mu, "\n");
            fprintf(fold_tk, "\n");
            fprintf(fold_calo, "\n");
            fprintf(fold_mu, "\n");

#ifdef NO_VALIDATE
            continue;
#endif
            // validation: common
            if (newevt_ref) {
                pingpong = 1-pingpong;
                for (int r = 0; r < NTKOUT; ++r)   for (int k = 0; k < TLEN; ++k) tk_output_ref[r][TLEN*pingpong+k].clear();
                for (int r = 0; r < NCALOOUT; ++r) for (int k = 0; k < TLEN; ++k) calo_output_ref[r][TLEN*pingpong+k].clear();
                for (int r = 0; r < NMUOUT; ++r) for (int k = 0; k < TLEN; ++k) mu_output_ref[r][TLEN*pingpong+k].clear();
            }
            for (int r = 0; r < NTKOUT; ++r) tk_output_ref[r][TLEN*pingpong+i] = tk_out_emu[r];
            for (int r = 0; r < NCALOOUT; ++r) calo_output_ref[r][TLEN*pingpong+i] = calo_out_emu[r];
            for (int r = 0; r < NMUOUT; ++r) mu_output_ref[r][TLEN*pingpong+i] = mu_out_emu[r];
            // validation: tracks
            if (tk_newevt_out) {
                if (tk_latency == -1) { tk_latency = i; printf("Detected tk_latency = %d\n", tk_latency); } 
                if (i != tk_latency) { printf("ERROR in tk_latency\n"); ok = false; break; }
                if (itest >= 1) { 
                    for (int r = 0; r < NTKOUT; ++r) {
                        for (int k = 0; k < TLEN; ++k) {
                            if (!track_equals(tk_output[r][k], tk_output_ref[r][TLEN*(1-pingpong)+k], "track 100*region+obj ", 100*(r+1)+k)) { ok = false; break; }
                        }
                    }
                }
                for (int r = 0; r < NTKOUT; ++r) for (int k = 0; k < TLEN; ++k) clear(tk_output[r][k]);
            }
            if (tk_latency >= 0 && frame >= tk_latency) {
                for (int r = 0; r < NTKOUT; ++r) tk_output[r][(frame-tk_latency) % TLEN] = tk_links_out[r];
            }
            // validation: calo
            if (calo_newevt_out) {
                if (calo_latency == -1) { calo_latency = i; printf("Detected calo_latency = %d\n", calo_latency); } 
                if (i != calo_latency) { printf("ERROR in calo_latency\n"); ok = false; break; }
                if (itest >= 1) { 
                    for (int r = 0; r < NCALOOUT; ++r) {
                        for (int k = 0; k < TLEN; ++k) {
                            if (!had_equals(calo_output[r][k], calo_output_ref[r][TLEN*(1-pingpong)+k], "calo 100*region+obj ", 100*(r+1)+k)) { ok = false; break; }
                        }
                    }
                }
                for (int r = 0; r < NCALOOUT; ++r) for (int k = 0; k < TLEN; ++k) clear(calo_output[r][k]);
            }
            if (calo_latency >= 0 && frame >= calo_latency) {
                for (int r = 0; r < NCALOOUT; ++r) calo_output[r][(frame-calo_latency) % TLEN] = calo_links_out[r];
            }
            // validation: mu
            if (mu_newevt_out) {
                if (mu_latency == -1) { mu_latency = i; printf("Detected mu_latency = %d\n", mu_latency); } 
                if (i != mu_latency) { printf("ERROR in mu_latency\n"); ok = false; break; }
                if (itest >= 1) { 
                    for (int r = 0; r < NMUOUT; ++r) {
                        for (int k = 0; k < TLEN; ++k) {
                            if (!mu_equals(mu_output[r][k], mu_output_ref[r][TLEN*(1-pingpong)+k], "mu 100*region+obj ", 100*(r+1)+k)) { ok = false; break; }
                        }
                    }
                }
                for (int r = 0; r < NMUOUT; ++r) for (int k = 0; k < TLEN; ++k) clear(mu_output[r][k]);
            }
            if (mu_latency >= 0 && frame >= mu_latency) {
                for (int r = 0; r < NMUOUT; ++r) mu_output[r][(frame-mu_latency) % TLEN] = mu_links_out[r];
            }
            // end validation
            if (!ok) break; 

        }
        if (!ok) break;
    } 

    fclose(fin_tk);
    fclose(fref_tk);
    fclose(fold_tk);
    fclose(fout_tk);
    fclose(fin_calo);
    fclose(fref_calo);
    fclose(fold_calo);
    fclose(fout_calo);
    fclose(fin_mu);
    fclose(fref_mu);
    fclose(fold_mu);
    fclose(fout_mu);

    return ok ? 0 : 1;
}
