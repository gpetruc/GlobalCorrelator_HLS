#include "src/tdemux.h"
#include <cstdio>
#include <cstdlib>

int main() {
    srand(125);
    const int NDATA = TMUX_IN*NCLK*4+10;
    w64 data[NLINKS][NDATA], in[NLINKS], out[NLINKS], refout[NLINKS];
    for (unsigned int itest = 0, ntest = 20; itest <= ntest; ++itest) {
        // create some input data
        bool isok = true;
        for (unsigned int j = 0; j < NLINKS; ++j) {
            for (unsigned int i = 0; i < NDATA; ++i) {
                if (itest == 0){ // special case, human readable pattern
                    int iclock = i - j * TMUX_OUT * NCLK;
                    if (iclock >= 0) {
                        if (NCLK > 1) {
                            int sub = iclock % NCLK;
                            int bx  = (iclock / NCLK) % TMUX_IN;
                            int ev  = (iclock / (TMUX_IN * NCLK)) * NLINKS + j;
                            data[j][i] = 1 + sub + 10*bx + 1000 * ev;
                        } else {
                            int bx  = iclock % TMUX_IN;
                            int ev  = iclock / TMUX_IN * NLINKS + j;
                            data[j][i] = 1  + bx + 100 * ev;
                        }
                    } else {
                        data[j][i] = 0;
                    }
                } else {
                    data[j][i] = ap_uint<64>(rand() & 0xFFFFFF);
                }
            }
        }
        if (itest == 0) {
            for (unsigned int j = 0; j < NLINKS; ++j) {
                printf("L[%d]: ", j);
                for (unsigned int i = 0; i < NDATA; ++i) printf("%5d | ", int(data[j][i]));
                printf("\n");
            }
        }

        for (unsigned int iclock = 0; iclock <  NDATA; ++iclock) {
            for (unsigned int j = 0; j < NLINKS; ++j) {
                in[j] = data[j][iclock];
            }
            bool ret = tdemux_simple(iclock == 0, in, out);
            bool ref = tdemux_simple_ref(iclock == 0, in, refout);
            bool ok = (ret == ref);
            if (ok) {
                for (unsigned int j = 0; j < NLINKS; ++j) {
                    ok = ok && (out[j] == refout[j]);
                }
            }

            if (itest == 0) {
                printf("%04d |  ", iclock);
                for (unsigned int j = 0; j < NLINKS; ++j) printf("%6d ", int(in[j]));
                printf(" | v%d  ", ret ? 1 : 0);
                for (unsigned int j = 0; j < NLINKS; ++j) printf("%6d ", int(out[j]));
                printf(" | v%d  ", ref ? 1 : 0);
                for (unsigned int j = 0; j < NLINKS; ++j) printf("%6d ", int(refout[j]));
                printf(isok ? "\n" : "   <=== ERROR \n");
            }

            if (!ok) isok = false;
        }
        if (!isok) {
            printf("\ntest %d failed\n", itest);
            return 1;
        } else {
            printf("\ntest %d passed\n", itest);
        }
    }
    return 0;
}
