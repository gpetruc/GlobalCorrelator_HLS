#include "src/algo.h"
#include <cstdio>
#include <cstdlib>
#include <fstream>

float deltaPhi(float phi1, float phi2) {
    auto dphi = phi1 - phi2;
    if (dphi > M_PI) dphi -= 2*M_PI;
    if (dphi < -M_PI) dphi += 2*M_PI;
    return dphi;
}

int main(int argc, char **argv) {
    uint64_t header, data[NPUPPI_MAX], dout[2];
    Puppi puppi[NPUPPI_MAX];
    Sum out, out_nomu;
    std::fstream in("Puppi.dump", std::ios::in | std::ios::binary);

    double sum_dpt = 0, sum_dpt2 = 0, sum_dphi = 0, sum_dphi2 = 0, sum_dptt = 0, sum_dptt2 = 0;
    double nomu_sum_dpt = 0, nomu_sum_dpt2 = 0, nomu_sum_dphi = 0, nomu_sum_dphi2 = 0, nomu_sum_dptt = 0, nomu_sum_dptt2 = 0;
    unsigned int ntest = 10;
    for (int itest = 0; itest < ntest && in.good(); ++itest) {
        in.read(reinterpret_cast<char *>(&header), sizeof(uint64_t));
        unsigned int npuppi = header & 0xFF;
        assert(npuppi <= NPUPPI_MAX);
        if (npuppi == 0) continue;
        in.read(reinterpret_cast<char *>(data), npuppi*sizeof(uint64_t));
        float sumpt = 0, sumpx = 0, sumpy = 0, pt, phi;
        float nomu_sumpt = 0, nomu_sumpx = 0, nomu_sumpy = 0, nomu_pt, nomu_phi;
        for (unsigned int i = 0; i < npuppi; ++i) {
            puppi[i].unpack(data[i]);
            float s = std::sin(puppi[i].floatPhi());
            float c = std::cos(puppi[i].floatPhi());
            if (itest == 0) {
              printf("Particle %3u/%3u %016lx pT %8.3f eta %+6.3f phi %+6.3f pid %1u  -> px = %+8.3f, py = %+8.3f \n",
                     i, npuppi, data[i], puppi[i].floatPt(), puppi[i].floatEta(),
                     puppi[i].floatPhi(), puppi[i].hwID.to_uint(),
                     puppi[i].floatPt()*c, puppi[i].floatPt()*s);
            }
            sumpt += puppi[i].floatPt();
            sumpx += puppi[i].floatPt()*c;
            sumpy += puppi[i].floatPt()*s;
            if (puppi[i].hwID < 6) {
                nomu_sumpt += puppi[i].floatPt();
                nomu_sumpx += puppi[i].floatPt()*c;
                nomu_sumpy += puppi[i].floatPt()*s;
            }
        }
        for (unsigned int i = npuppi; i < NPUPPI_MAX; ++i) {
            puppi[i].clear();
        }
        pt = std::hypot(sumpx, sumpy);
        phi = std::atan2(-sumpy, -sumpx);
        nomu_pt = std::hypot(nomu_sumpx, nomu_sumpy);
        nomu_phi = std::atan2(-nomu_sumpy, -nomu_sumpx);
        printf("EMU px = %+8.3f, py = %+8.3f\n", -sumpx, -sumpy);
        printf("EMU pt2 = %8.2f, pt = %8.3f, 1/pt = %.6f\n", pt*pt, pt, 1.0/pt);
        printf("EMU cosphi = %7.4f, phi = %+8.3f\n", -sumpx/pt, phi);
        printf("EMU px = %+8.3f, py = %+8.3f\n", -nomu_sumpx, -nomu_sumpy);
        printf("EMU pt2 = %8.2f, pt = %8.3f, 1/pt = %.6f\n", nomu_pt*nomu_pt, nomu_pt, 1.0/nomu_pt);
        printf("EMU cosphi = %7.4f, phi = %+8.3f\n", -nomu_sumpx/nomu_pt, nomu_phi);

        #ifdef ON_L1T
        compute_sums_l1t(puppi, out, out_nomu);
        dout[0] = out.pack();
        dout[1] = out_nomu.pack();
        #elif ON_ALVEO
        compute_sums_alveo(npuppi, data, dout);
        out.unpack(dout[0]);
        out_nomu.unpack(dout[1]);
        #endif

        float dpt = pt - out.floatPt();
        float dphi = phi - out.floatPhi();
        float dptt = sumpt - out.floatPtTot();
        float nomu_dpt = nomu_pt - out_nomu.floatPt();
        float nomu_dphi = nomu_phi - out_nomu.floatPhi();
        float nomu_dptt = nomu_sumpt - out_nomu.floatPtTot();

        printf("Results for test %d: \n", itest);
        printf("  pt:        %8.3f (float)  %8.3f (hw)   diff %+5.3f \n", pt, out.floatPt(), dpt);
        printf("  phi:       %8.3f (float)  %8.3f (hw)   diff %+5.3f \n", phi, out.floatPhi(), dphi);
        printf("  tot:       %8.3f (float)  %8.3f (hw)   diff %+5.3f \n", sumpt, out.floatPtTot(), dptt);
        printf("  nomu pt:   %8.3f (float)  %8.3f (hw)   diff %+5.3f \n", nomu_pt, out_nomu.floatPt(), nomu_dpt);
        printf("  nomu phi:  %8.3f (float)  %8.3f (hw)   diff %+5.3f \n", nomu_phi, out_nomu.floatPhi(), nomu_dphi);
        printf("  nomu tot:  %8.3f (float)  %8.3f (hw)   diff %+5.3f \n", nomu_sumpt, out_nomu.floatPtTot(), nomu_dptt);

        sum_dpt += dpt;
        sum_dpt2 += dpt*dpt;
        sum_dphi += dphi;
        sum_dphi2 += dphi*dphi;
        sum_dptt += dptt;
        sum_dptt2 += dptt*dptt;

        nomu_sum_dpt += nomu_dpt;
        nomu_sum_dpt2 += nomu_dpt*nomu_dpt;
        nomu_sum_dphi += nomu_dphi;
        nomu_sum_dphi2 += nomu_dphi*nomu_dphi;
        nomu_sum_dptt += nomu_dptt;
        nomu_sum_dptt2 += nomu_dptt*nomu_dptt;
    }

    printf("dpt   average %8.3f  rms %7.3f\n", sum_dpt/ntest, std::sqrt(sum_dpt2/ntest));
    printf("dphi  average %8.3f  rms %7.3f\n", sum_dphi/ntest, std::sqrt(sum_dphi2/ntest));
    printf("sumpt average %8.3f  rms %7.3f\n", sum_dptt/ntest, std::sqrt(sum_dptt2/ntest));
    printf("nomu dpt   average %8.3f  rms %7.3f\n", nomu_sum_dpt/ntest, std::sqrt(nomu_sum_dpt2/ntest));
    printf("nomu dphi  average %8.3f  rms %7.3f\n", nomu_sum_dphi/ntest, std::sqrt(nomu_sum_dphi2/ntest));
    printf("nomu sumpt average %8.3f  rms %7.3f\n", nomu_sum_dptt/ntest, std::sqrt(nomu_sum_dptt2/ntest));
}