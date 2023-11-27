#include "src/algo.h"
#include <cstdio>
#include <cstdlib>
#include <fstream>

int main(int argc, char **argv) {
    uint64_t header, data[NPUPPI_MAX], dout[2];
    Puppi puppi[NPUPPI_MAX];
    Puppi out_ref[NISO_MAX], out[NISO_MAX];
    Puppi::pt_t out_absiso_ref[NISO_MAX], out_absiso[NISO_MAX];
    std::fstream in("Puppi.dump", std::ios::in | std::ios::binary);
    for (int itest = 0, ntest = 10; itest < ntest && in.good(); ++itest) {
        in.read(reinterpret_cast<char *>(&header), sizeof(uint64_t));
        unsigned int npuppi = header & 0xFF;
        assert(npuppi <= NPUPPI_MAX);
        if (npuppi == 0) continue;
        in.read(reinterpret_cast<char *>(data), npuppi*sizeof(uint64_t));
        float sumpt = 0, sumpx = 0, sumpy = 0, pt, phi;
        float nomu_sumpt = 0, nomu_sumpx = 0, nomu_sumpy = 0, nomu_pt, nomu_phi;
        for (unsigned int i = 0; i < npuppi; ++i) {
            puppi[i].unpack(data[i]);
            if (itest == 0) {
              printf("Particle %3u/%3u %016lx pT %8.3f eta %+6.3f phi %+6.3f pid %1u\n",
                     i, npuppi, data[i], puppi[i].floatPt(), puppi[i].floatEta(),
                     puppi[i].floatPhi(), puppi[i].hwID.to_uint());
            }
        }
        for (unsigned int i = npuppi; i < NPUPPI_MAX; ++i) {
            puppi[i].clear();
        }

        // REFERENCE CODE GOES HERE
        unsigned niso_ref = 0;
        bool masked[NPUPPI_MAX];
        auto dr2_max = drToHwDr2(0.4), dr2_veto = drToHwDr2(0.1);
        for (int i = 0; i < npuppi; ++i) masked[i] = (puppi[i].hwID < 2);
        for (int iter = 0; iter < NISO_MAX; ++iter) {
            int iseed = -1;
            for (int i = 0; i < npuppi; ++i) {
                if (!masked[i] && (iseed == -1 || puppi[iseed].hwPt < puppi[i].hwPt)) {
                    iseed = i;
                }
            }
            if (iseed == -1) break;
            out_ref[niso_ref] = puppi[iseed];
            out_absiso_ref[niso_ref] = 0;
            for (int i = 0; i < npuppi; ++i) {
                auto dr2 = deltaR2(puppi[iseed], puppi[i]);
                if (dr2 < dr2_max) {
                    masked[i] = true;
                    if (dr2 > dr2_veto) out_absiso_ref[niso_ref] += puppi[i].hwPt;
                }
            }
            if (itest == 0) {
                printf("Seed %3u pT %8.3f eta %+6.3f phi %+6.3f pid %1u: abs iso %8.3f\n",
                    iseed, puppi[iseed].floatPt(), puppi[iseed].floatEta(),  
            puppi[iseed].floatPhi(), puppi[iseed].hwID.to_uint(), Puppi::floatPt(out_absiso_ref[niso_ref]));
            }
            if (out_absiso_ref[niso_ref] < puppi[iseed].hwPt) {
                niso_ref++;
            }
        }
        for (int i = niso_ref; i < NISO_MAX; ++i) {
            out_ref[i].clear();
            out_absiso_ref[i] = 0;
        }

        // CALL TO FIRMWARE
        compute_isolated_l1t(puppi, out, out_absiso);

        // COMPARE
        bool ok = true;
        for (int i = 0; i < NISO_MAX; ++i) {
            ok = ok && (out[i].pack() == out_ref[i].pack() && out_absiso[i] == out_absiso_ref[i]); 
        }
        if (!ok) {
            printf("Mismatch in test %u!\n", itest);
            for (int i = 0; i < NISO_MAX; ++i) { 
                printf("REF pT %8.3f eta %+6.3f phi %+6.3f pid %1u abs iso %8.3f ",
                    out_ref[i].floatPt(), out_ref[i].floatEta(), out_ref[i].floatPhi(), out_ref[i].hwID.to_int(), Puppi::floatPt(out_absiso_ref[i])); 
                printf("FW  pT %8.3f eta %+6.3f phi %+6.3f pid %1u abs iso %8.3f \n",
                    out[i].floatPt(), out[i].floatEta(), out[i].floatPhi(), out[i].hwID.to_int(), Puppi::floatPt(out_absiso[i])); 
            }
            return 1;
        } else {
            printf("Test %u passed\n", itest);
        }
    }
    return 0;
}