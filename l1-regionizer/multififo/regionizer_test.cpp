#include "firmware/regionizer.h"
#include "../../utils/pattern_serializer.h"
#include "../../utils/test_utils.h"
#include "../../utils/DumpFileReader.h"
#include "multififo_regionizer_ref.h"

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

    const unsigned int nchann_in = NTKSECTORS*NTKFIBERS + NCALOSECTORS*NCALOFIBERS + NMUFIBERS;
    #ifdef ROUTER_NOMUX
    const unsigned int nchann_out = NTKOUT + NCALOOUT + NMUOUT;
    #else
    const unsigned int nchann_out = NTKOUT + NCALOOUT + NMUOUT + 1;
    #endif
    PatternSerializer serPatternsIn("input-emp.txt",2*nchann_in), serPatternsOut("output-emp.txt",2*nchann_out), serPatternsRef("output-ref-emp.txt",2*nchann_out);
    ap_uint<PackedTkObj::width> all_channels_in[nchann_in], all_channels_ref[nchann_out], all_channels_out[nchann_out];
    for (unsigned int i = 0; i < nchann_in; ++i) all_channels_in[i] = 0;
    for (unsigned int i = 0; i < nchann_out; ++i) {
        all_channels_ref[i] = 0; all_channels_out[i] = 0; 
    }
    serPatternsIn.packAndWrite(nchann_in, all_channels_in, false); // prepend one null frame at the beginning

    int frame = 0, pingpong = 1; 
    int tk_latency = -1, calo_latency = -1, mu_latency = -1;

    bool ok = true, first = true;

    const bool mux = ROUTER_ISMUX, stream = ROUTER_ISSTREAM;
    l1ct::MultififoRegionizerEmulator emulator(/*nendcaps=*/1, REGIONIZERNCLOCKS, NTRACK, NCALO, /*NEM=*/0, NMU, /*streaming=*/stream, /*ii=*/(stream?4:6));

    for (int itest = 0; itest < 100; ++itest) {
        TkObj tk_output[NTKOUT][TLEN], tk_output_ref[NTKOUT][2*TLEN];
        HadCaloObj calo_output[NCALOOUT][TLEN], calo_output_ref[NCALOOUT][2*TLEN];
        MuObj mu_output[NMUOUT][TLEN], mu_output_ref[NMUOUT][2*TLEN];

        if (!inputs.nextEvent()) break;
        const auto & decodedObjs = inputs.event().decoded;

        // now we make a single endcap setup
        RegionizerDecodedInputs in; std::vector<PFInputRegion> pfin;
        for (auto & sec : inputs.event().decoded.track) if (sec.region.floatEtaCenter() >= 0) in.track.push_back(sec);
        for (auto & sec : inputs.event().decoded.hadcalo) if (sec.region.floatEtaCenter() >= 0) in.hadcalo.push_back(sec);
        for (auto & sec : inputs.event().decoded.emcalo) if (sec.region.floatEtaCenter() >= 0) in.emcalo.push_back(sec);
        in.muon = inputs.event().decoded.muon;
        for (auto & reg : inputs.event().pfinputs) if (reg.region.floatEtaCenter() >= 0) pfin.push_back(reg);
        const glbeta_t etaCenter = l1ct::Scales::makeGlbEta(2.0);

        if (first) { emulator.initSectorsAndRegions(in, pfin); first = false; }

        for (int i = 0; i < TLEN; ++i, ++frame) {
            std::vector<l1ct::TkObjEmu> tk_links_in_emu, tk_out_emu;
            std::vector<l1ct::HadCaloObjEmu> calo_links_in_emu, calo_out_emu;
            std::vector<l1ct::MuObjEmu> mu_links_in_emu, mu_out_emu;

            emulator.fillLinks(i, in, tk_links_in_emu);
            emulator.fillLinks(i, in, calo_links_in_emu);
            emulator.fillLinks(i, in, mu_links_in_emu);

            l1ct::TkObj tk_links_in[NTKSECTORS][NTKFIBERS];
            l1ct::HadCaloObj    calo_links_in[NCALOSECTORS][NCALOFIBERS];
            l1ct::MuObj    mu_links_in[NMUFIBERS];
            PackedTkObj tk_links64_in[NTKSECTORS][NTKFIBERS];
            PackedCaloObj    calo_links64_in[NCALOSECTORS][NCALOFIBERS];
            PackedMuObj    mu_links64_in[NMUFIBERS];

            emulator.toFirmware(tk_links_in_emu, tk_links_in);
            emulator.toFirmware(calo_links_in_emu, calo_links_in);
            emulator.toFirmware(mu_links_in_emu, mu_links_in);

            l1pf_pattern_pack<NTKSECTORS*NTKFIBERS>(&tk_links_in[0][0], &tk_links64_in[0][0]);
            l1pf_pattern_pack<NCALOSECTORS*NCALOFIBERS>(&calo_links_in[0][0], &calo_links64_in[0][0]);
            l1pf_pattern_pack<NMUFIBERS>(mu_links_in, mu_links64_in);

            unsigned int ilink = 0;
            for (int s = 0; s < NTKSECTORS; ++s) 
                for (int f = 0; f < NTKFIBERS; ++f) 
                    all_channels_in[ilink++] = tk_links64_in[s][f];
            for (int s = 0; s < NCALOSECTORS; ++s) 
                for (int f = 0; f < NCALOFIBERS; ++f) 
                    all_channels_in[ilink++] = calo_links64_in[s][f];
            for (int f = 0; f < NMUFIBERS; ++f) 
                all_channels_in[ilink++] = mu_links64_in[f];

            TkObj tk_links_out[NTKOUT];
            HadCaloObj calo_links_out[NCALOOUT];
            MuObj mu_links_out[NMUOUT];
            PackedTkObj tk_links64_out[NTKOUT];
            PackedCaloObj calo_links64_out[NCALOOUT];
            PackedMuObj mu_links64_out[NMUOUT];

            bool calo_newevt_out, tk_newevt_out, mu_newevt_out, newevt_ref = (i == 0);
            bool tk_ref_good   = emulator.step(newevt_ref, tk_links_in_emu, tk_out_emu, mux);
            bool calo_ref_good = emulator.step(newevt_ref, calo_links_in_emu, calo_out_emu, mux);
            bool mu_ref_good = emulator.step(newevt_ref, mu_links_in_emu, mu_out_emu, mux);

#if defined(ROUTER_NOMERGE) || defined(ROUTER_NOMUX)
            bool tk_good   = tk_router(i == 0, tk_links64_in, tk_links64_out, tk_newevt_out);
            bool calo_good = calo_router(i == 0, calo_links64_in, calo_links64_out, calo_newevt_out);
            bool mu_good = mu_router(i == 0, etaCenter, mu_links64_in, mu_links64_out, mu_newevt_out);
#else
            // the routers are not implemented in this pattern, and may segfault due to size of the arrays
            bool tk_good = false, calo_good = false, mu_good = false;
#endif
            l1pf_pattern_unpack<NTKOUT>(tk_links64_out, tk_links_out);
            l1pf_pattern_unpack<NCALOOUT>(calo_links64_out, calo_links_out);
            l1pf_pattern_unpack<NMUOUT>(mu_links64_out, mu_links_out);

            ilink = 0;
            for (int r = 0; r < NTKOUT; ++r) all_channels_ref[ilink++] = tk_out_emu[r].pack();
            for (int r = 0; r < NCALOOUT; ++r) all_channels_ref[ilink++] = calo_out_emu[r].pack();
            for (int r = 0; r < NMUOUT; ++r) all_channels_ref[ilink++] = mu_out_emu[r].pack();
            unsigned int iregion = i / (stream ? 4 : 6); bool region_valid = iregion < pfin.size();
            all_channels_ref[ilink++] = region_valid ? pfin[iregion].region.pack() : ap_uint<l1ct::PFRegion::BITWIDTH>(0);

            ilink = 0;
            for (int r = 0; r < NTKOUT; ++r) all_channels_out[ilink++] = tk_links64_out[r];
            for (int r = 0; r < NCALOOUT; ++r) all_channels_out[ilink++] = calo_links64_out[r];
            for (int r = 0; r < NMUOUT; ++r) all_channels_out[ilink++] = mu_links64_out[r];

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
            fprintf(fout_calo, "%5d %1d %1d   ", frame, int(calo_good), int(calo_newevt_out));
            fprintf(fref_calo, "%5d %1d %1d   ", frame, int(calo_ref_good), int(newevt_ref));
            fprintf(fout_mu, "%5d %1d %1d   ", frame, int(mu_good), int(mu_newevt_out));
            fprintf(fref_mu, "%5d %1d %1d   ", frame, int(mu_ref_good), int(newevt_ref));
            for (int r = 0; r < NTKOUT; ++r) printTrack(fout_tk, tk_links_out[r]);
            for (int r = 0; r < NTKOUT; ++r) printTrack(fref_tk, tk_out_emu[r]);
            for (int r = 0; r < NCALOOUT; ++r) printCalo(fout_calo, calo_links_out[r]);
            for (int r = 0; r < NCALOOUT; ++r) printCalo(fref_calo, calo_out_emu[r]);
            for (int r = 0; r < NMUOUT; ++r) printMu(fout_mu, mu_links_out[r]);
            for (int r = 0; r < NMUOUT; ++r) printMu(fref_mu, mu_out_emu[r]);
            fprintf(fout_tk, "\n");
            fprintf(fout_calo, "\n");
            fprintf(fout_mu, "\n");
            fprintf(fref_tk, "\n");
            fprintf(fref_calo, "\n");
            fprintf(fref_mu, "\n");

            serPatternsIn.packAndWrite(nchann_in, all_channels_in, i < TLEN-1);
            serPatternsOut.packAndWrite(nchann_out, all_channels_out);
            serPatternsRef.packAndWrite(nchann_out, all_channels_ref);
             

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
    fclose(fout_tk);
    fclose(fin_calo);
    fclose(fref_calo);
    fclose(fout_calo);
    fclose(fin_mu);
    fclose(fref_mu);
    fclose(fout_mu);

    return ok ? 0 : 1;
}
