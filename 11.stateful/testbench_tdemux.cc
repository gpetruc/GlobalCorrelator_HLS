#include "src/tdemux.h"
#include <cstdio>
#include <cstdlib>

int main() {
    srand(125);
    const int NDATA = TMUX_IN*NCLK*2+3;
    w64 data[NLINKS][NDATA], in[NLINKS], out[NLINKS], refout[NLINKS];
    //for (unsigned int itest = 0, ntest = 20; itest <= ntest; ++itest) {
        // create some input data
    bool isok = true;
    for (unsigned int j = 0; j < NLINKS; ++j) {
        for (unsigned int i = 0; i < NDATA; ++i) {
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
        }
    }
    for (unsigned int j = 0; j < NLINKS; ++j) {
        printf("L[%d]: ", j);
        for (unsigned int i = 0; i < NDATA; ++i) printf("%5d | ", int(data[j][i]));
        printf("\n");
    }

    for (unsigned int iclock = 0; iclock <  NDATA; ++iclock) {
        for (unsigned int j = 0; j < NLINKS; ++j) {
            in[j] = data[j][iclock];
        }
        bool ret = tdemux_simple(iclock == 0, in, out);
        bool ref = tdemux_simple_ref(iclock == 0, in, refout);
        printf("%04d |  ", iclock);
        for (unsigned int j = 0; j < NLINKS; ++j) printf("%5d ", int(in[j]));
        printf(" | v%d  ", ret ? 1 : 0);
        for (unsigned int j = 0; j < NLINKS; ++j) printf("%5d ", int(out[j]));
        printf(" | v%d  ", ref ? 1 : 0);
        for (unsigned int j = 0; j < NLINKS; ++j) printf("%5d ", int(refout[j]));

        bool ok = true; // (ret == ref);
        for (unsigned int j = 0; j < NLINKS; ++j) ok = ok && (out[j] == refout[j]);
        if (!ok)  {
            printf("   <=== ERROR \n");
            isok = false;
        } else {
            printf("\n");
        }
    }
    if (!isok) {
        printf("\ntest failed\n");
        return 1;
    } else {
        printf("\ntest passed\n");
        return 0;
    }
    //}
}
