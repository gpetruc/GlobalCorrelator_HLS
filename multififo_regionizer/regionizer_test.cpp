#include "firmware/regionizer.h"
#include "../utils/pattern_serializer.h"
#include "../utils/test_utils.h"

#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <vector>

#define TLEN REGIONIZERNCLOCKS 

bool tk_router_ref(bool newevent, const TkObj tracks_in[NTKSECTORS][NTKFIBERS], TkObj tracks_out[NTKOUT]) ;
bool calo_router_ref(bool newevent, const HadCaloObj calo_in[NCALOSECTORS][NCALOFIBERS], HadCaloObj calo_out[NCALOOUT]) ;
bool readEventTk(FILE *file, std::vector<TkObj> inputs[NTKSECTORS][NTKFIBERS], uint32_t &run, uint32_t &lumi, uint64_t &event) ;
bool readEventCalo(FILE *file, std::vector<HadCaloObj> inputs[NCALOSECTORS][NCALOFIBERS], bool zside, uint32_t &run, uint32_t &lumi, uint64_t &event) ;

int main(int argc, char **argv) {
    FILE *fMC_calo  = fopen("caloDump_hgcal.TTbar_PU200.txt", "r");
    FILE *fMC_tk  = fopen("trackDump_hgcalPos.TTbar_PU200.txt", "r");
    if (!fMC_calo || !fMC_tk) return 2;
    FILE *fin_tk   = fopen("input-tk.txt", "w");
    FILE *fin_calo = fopen("input-calo.txt", "w");
    FILE *fout_tk   = fopen("output-tk.txt", "w");
    FILE *fout_calo = fopen("output-calo.txt", "w");
    FILE *fref_tk   = fopen("output-ref-tk.txt", "w");
    FILE *fref_calo = fopen("output-ref-calo.txt", "w");

    PatternSerializer serPatternsIn("input-emp.txt"), serPatternsOut("output-emp.txt"), serPatternsRef("output-ref-emp.txt");
    assert(PACKING_NCHANN >= NTKSECTORS*NTKFIBERS + NCALOSECTORS*NCALOFIBERS);
    assert(PACKING_NCHANN >= NTKOUT + NCALOOUT);
    ap_uint<64> all_channels_in[PACKING_NCHANN], all_channels_ref[PACKING_NCHANN], all_channels_out[PACKING_NCHANN];
    for (unsigned int i = 0; i < PACKING_NCHANN; ++i) {
        all_channels_in[i] = 0; all_channels_ref[i] = 0; all_channels_out[i] = 0; 
    }

    int frame = 0, pingpong = 1; 
    int tk_latency = -1, calo_latency = -1;

    bool ok = true;
    for (int itest = 0; itest < 10; ++itest) {
        std::vector<TkObj> tk_inputs[NTKSECTORS][NTKFIBERS];
        std::vector<HadCaloObj> calo_inputs[NCALOSECTORS][NCALOFIBERS];
        TkObj tk_output[NTKOUT][TLEN], tk_output_ref[NTKOUT][2*TLEN];
        HadCaloObj calo_output[NCALOOUT][TLEN], calo_output_ref[NCALOOUT][2*TLEN];

        uint32_t run = 0, lumi = 0; uint64_t event = 0;
        if (!readEventTk(fMC_tk, tk_inputs, run, lumi, event) || 
                !readEventCalo(fMC_calo, calo_inputs, /*zside=*/true, run, lumi, event)) break;

        for (int i = 0; i < TLEN; ++i, ++frame) {
            TkObj tk_links_in[NTKSECTORS][NTKFIBERS];
            PackedTkObj tk_links64_in[NTKSECTORS][NTKFIBERS];

            unsigned int ilink = 0;

            for (int s = 0; s < NTKSECTORS; ++s) {
                for (int f = 0; f < NTKFIBERS; ++f) {
                    clear(tk_links_in[s][f]);
                    if (i < TLEN-1 && i < int(tk_inputs[s][f].size())) { // emp protocol, must leave one null frame at the end
                        tk_links_in[s][f]  = tk_inputs[s][f][i];
                    }
                    tk_links64_in[s][f] = l1pf_pattern_pack_one(tk_links_in[s][f]);
                    all_channels_in[ilink++] = tk_links64_in[s][f];
                }
            }

            HadCaloObj    calo_links_in[NCALOSECTORS][NCALOFIBERS];
            PackedCaloObj calo_links64_in[NCALOSECTORS][NCALOFIBERS];
            for (int s = 0; s < NCALOSECTORS; ++s) {
                for (int f = 0; f < NCALOFIBERS; ++f) {
                    clear(calo_links_in[s][f]);
                    if (i < TLEN-1 && i < int(calo_inputs[s][f].size())) { // emp protocol, must leave one null frame at the end
                        calo_links_in[s][f]  = calo_inputs[s][f][i];
                    }
                    calo_links64_in[s][f] = l1pf_pattern_pack_one(calo_links_in[s][f]);
                    all_channels_in[ilink++] = tk_links64_in[s][f];
                }
            }

            TkObj tk_links_out[NTKOUT], tk_links_ref[NTKOUT]; PackedTkObj tk_links64_out[NTKOUT];
            HadCaloObj calo_links_out[NCALOOUT], calo_links_ref[NCALOOUT]; PackedCaloObj calo_links64_out[NCALOOUT];

            bool calo_newevt_out, tk_newevt_out, newevt_ref = (i == 0);
            bool tk_ref_good   = tk_router_ref(i == 0, tk_links_in, tk_links_ref);
            bool calo_ref_good = calo_router_ref(i == 0, calo_links_in, calo_links_ref);

            bool tk_good   = tk_router(i == 0, tk_links64_in, tk_links64_out, tk_newevt_out);
            bool calo_good = calo_router(i == 0, calo_links64_in, calo_links64_out, calo_newevt_out);

            l1pf_pattern_unpack<NTKOUT,0>(tk_links64_out, tk_links_out);
            l1pf_pattern_unpack<NCALOOUT,0>(calo_links64_out, calo_links_out);

            for (unsigned int i = 0; i < NTKOUT;   ++i) all_channels_out[i]        = tk_links64_out[i];
            for (unsigned int i = 0; i < NCALOOUT; ++i) all_channels_out[i+NTKOUT] = calo_links64_out[i];
            l1pf_pattern_pack<NTKOUT,  0     >(tk_links_ref,   all_channels_ref);
            l1pf_pattern_pack<NCALOOUT,NTKOUT>(calo_links_ref, all_channels_ref);

            serPatternsIn(all_channels_in, (i < TLEN-1));
            serPatternsOut(all_channels_out);
            serPatternsRef(all_channels_ref);

            fprintf(fin_tk,   "%05d %1d   ", frame, int(i==0));
            fprintf(fin_calo, "%05d %1d   ", frame, int(i==0));
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
            fprintf(fin_tk, "\n");
            fprintf(fin_calo, "\n");
            fprintf(fout_tk, "%5d %1d %1d   ", frame, int(tk_good), int(tk_newevt_out));
            fprintf(fref_tk, "%5d %1d %1d   ", frame, int(tk_ref_good), int(newevt_ref));
            fprintf(fout_calo, "%5d %1d %1d   ", frame, int(calo_good), int(calo_newevt_out));
            fprintf(fref_calo, "%5d %1d %1d   ", frame, int(calo_ref_good), int(newevt_ref));
            for (int r = 0; r < NTKOUT; ++r) printTrack(fout_tk, tk_links_out[r]);
            for (int r = 0; r < NTKOUT; ++r) printTrack(fref_tk, tk_links_ref[r]);
            for (int r = 0; r < NCALOOUT; ++r) printCalo(fout_calo, calo_links_out[r]);
            for (int r = 0; r < NCALOOUT; ++r) printCalo(fref_calo, calo_links_ref[r]);
            fprintf(fout_tk, "\n");
            fprintf(fout_calo, "\n");
            fprintf(fref_tk, "\n");
            fprintf(fref_calo, "\n");

#ifdef NO_VALIDATE
            continue;
#endif
            // validation: common
            if (newevt_ref) {
                pingpong = 1-pingpong;
                for (int r = 0; r < NTKOUT; ++r)   for (int k = 0; k < TLEN; ++k) clear(tk_output_ref[r][TLEN*pingpong+k]);
                for (int r = 0; r < NCALOOUT; ++r) for (int k = 0; k < TLEN; ++k) clear(calo_output_ref[r][TLEN*pingpong+k]);
            }
            for (int r = 0; r < NTKOUT; ++r) tk_output_ref[r][TLEN*pingpong+i] = tk_links_ref[r];
            for (int r = 0; r < NCALOOUT; ++r) calo_output_ref[r][TLEN*pingpong+i] = calo_links_ref[r];
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

            // end validation
            if (!ok) break; 

        }
        if (!ok) break;
    } 

    fclose(fMC_tk);
    fclose(fMC_calo);
    fclose(fin_tk);
    fclose(fref_tk);
    fclose(fout_tk);
    fclose(fin_calo);
    fclose(fref_calo);
    fclose(fout_calo);

    return ok ? 0 : 1;
}
