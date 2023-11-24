#include "src/func.h"
#include <cstdio>
#include <cstdlib>

#define NMU 12
#define NCLK 24 // for how long we simulate

int main() {
    srand(125);
    for (unsigned int itest = 0, ntest = 2; itest <= ntest; ++itest) {
        TkMu tkMu[NMU], tkMu_out[NMU];
        ap_uint<96> tkMuWords_ref[NMU], tkMuWords_out[NMU];
        ap_uint<64> data_in[NCLK];

        for (int i = 0; i < NCLK; ++i)
            data_in[i] = 0;

        for (int i = 0, iword = 0; i < 12; ++i) {
            if (itest == 0) {
                // made-up words
                tkMuWords_ref[i](95,64) = 0x300 + (i+1) + 0x1000*itest;
                tkMuWords_ref[i](63,32) = 0x200 + (i+1) + 0x1000*itest;
                tkMuWords_ref[i](31, 0) = 0x100 + (i+1) + 0x1000*itest;
            } else {
                tkMu[i].valid = true;
                tkMu[i].hwPt = TkMu::toHwPt(rand()/float(RAND_MAX)*50+3);
                tkMu[i].hwEta = TkMu::toHwEta(rand()/float(RAND_MAX)*4.8-2.4);
                tkMu[i].hwPhi = TkMu::toHwPhi(rand()/float(RAND_MAX)*2*M_PI - M_PI);
                tkMu[i].hwZ0 = (rand() & 1023) - 515;
                tkMu[i].hwD0 = (rand() & 1023) - 512;
                tkMu[i].hwCharge = rand() & 1;
                tkMu[i].hwQuality = (rand() & 0xFE) + 1;
                tkMu[i].hwIsolation = rand() % 4;
                tkMu[i].hwBeta = std::min<int>(16*(1-std::pow(rand()/float(RAND_MAX),3)), 15);
                tkMu[i].spare = rand() & ((1<<15)-1);
                tkMuWords_ref[i] = tkMu[i].pack();
            }
            if (i % 2 == 0) {
                data_in[iword+0](63, 0) = tkMuWords_ref[i](63,0);
                data_in[iword+1](31, 0) = tkMuWords_ref[i](95,64);
            } else {
                data_in[iword+1](63,32) = tkMuWords_ref[i](95,64);
                data_in[iword+2](63, 0) = tkMuWords_ref[i](63,0);
                iword += 3;
            }
        }

        bool ok = true;
        for (int iclock = 0; iclock < NCLK; ++iclock) {
            bool newEventOut = false, newEventOut_ref = (iclock == (3*NMU)/2-1);
            tkmu_receiver(iclock == 0, data_in[iclock], tkMu_out, newEventOut);
            ok = ok && (newEventOut == newEventOut_ref);
            if (newEventOut) {
                printf("iclock %2d: newEventOut %d\n", iclock, int(newEventOut));
                for (int i = 0; i < NMU; ++i) {
                  tkMuWords_out[i] = tkMu_out[i].pack();
                  printf("TkMu %26s ref %26s: %s\n",
                         tkMuWords_out[i].to_string(16).c_str(),
                         tkMuWords_ref[i].to_string(16).c_str(),
                         tkMuWords_out[i] == tkMuWords_ref[i] ? "ok" : "FAIL");
                  ok = ok && (tkMuWords_out[i] == tkMuWords_ref[i]);
                } 
            }
        }
        if (!ok) return 1;
    }
    return 0;
}
