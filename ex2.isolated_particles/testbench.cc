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
        compute_isolated_ref(npuppi, puppi, out_ref, out_absiso_ref, itest == 0);

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