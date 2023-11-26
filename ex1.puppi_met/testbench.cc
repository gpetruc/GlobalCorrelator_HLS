#include "src/algo.h"
#include <cstdio>
#include <cstdlib>
#include <fstream>

int main(int argc, char **argv) {
    uint64_t header, data[NPUPPI_MAX], dout[2];
    Puppi puppi[NPUPPI_MAX];
    Sum out, out_nomu;
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
            float s = std::sin(puppi[i].floatPhi());
            float c = std::sin(puppi[i].floatPhi());
            sumpt += puppi[i].floatPt();
            sumpx += puppi[i].floatPt()*c;
            sumpy += puppi[i].floatPt()*s;
            if (puppi[i].hwID < Puppi::MuMinus) {
                nomu_sumpt += puppi[i].floatPt();
                nomu_sumpx += puppi[i].floatPt()*c;
                nomu_sumpy += puppi[i].floatPt()*s;
            }
        }
        for (unsigned int i = npuppi; i < NPUPPI_MAX; ++i) {
            puppi[i].clear();
        }
        pt = std::hypot(sumpx, sumpy);
        phi = std::atan2(sumpy, sumpx);
        nomu_pt = std::hypot(nomu_sumpx, nomu_sumpy);
        nomu_phi = std::atan2(nomu_sumpy, nomu_sumpx);

        #ifdef ON_L1T
        compute_sums_l1t(puppi, out, out_nomu);
        dout[0] = out.pack();
        dout[1] = out_nomu.pack();
        #elif ON_ALVEO
        compute_sums_alveo(npuppi, data, dout);
        out.unpack(dout[0]);
        out_nomu.unpack(dout[1]);
        #endif

        printf("Results for test %d: \n", itest);
        printf("  pt:        %8.3f (float)  %8.3f (hw)   diff %+5.3f \n", pt, out.floatPt(), pt - out.floatPt());
        printf("  phi:       %8.3f (float)  %8.3f (hw)   diff %+5.3f \n", phi, out.floatPhi(), phi - out.floatPhi());
        printf("  tot:       %8.3f (float)  %8.3f (hw)   diff %+5.3f \n", sumpt, out.floatPtTot(), sumpt - out.floatPtTot());
        printf("  nomu pt:   %8.3f (float)  %8.3f (hw)   diff %+5.3f \n", nomu_pt, out_nomu.floatPt(), nomu_pt - out_nomu.floatPt());
        printf("  nomu phi:  %8.3f (float)  %8.3f (hw)   diff %+5.3f \n", nomu_phi, out_nomu.floatPhi(), nomu_phi - out_nomu.floatPhi());
        printf("  nomu tot:  %8.3f (float)  %8.3f (hw)   diff %+5.3f \n", nomu_sumpt, out_nomu.floatPtTot(), nomu_sumpt - out_nomu.floatPtTot());
    }

}