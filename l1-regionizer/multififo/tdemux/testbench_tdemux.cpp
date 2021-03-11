#include "firmware/tdemux.h"
#include <cstdio>
#include <cstdlib>
#include "tdemux_ref.h"


int main() {
    srand(125);
    const int NDATA = TMUX_IN*NCLK*4+10;
    w65 data[NLINKS][NDATA], in[NLINKS], out[NLINKS], refout[NLINKS];

    FILE * f_patterns_in, * f_patterns_out; char fnbuff[25]; 

    unsigned int maxlen = TMUX_IN*NCLK;

    for (unsigned int itest = 0, ntest = 20; itest <= ntest; ++itest) {
        TDemuxRef tdemux_ref;
        // create some input data
        bool isok = true; int pktlen;
        for (unsigned int j = 0; j < NLINKS; ++j) {
            for (unsigned int i = 0; i < NDATA; ++i) {
                if (i % (TMUX_IN*NCLK) == 0) pktlen = (maxlen/5) + rand() % (maxlen*4/5);
                int iclock = i - j * TMUX_OUT * NCLK;
                if ((iclock >= 0) && ((iclock % maxlen) < pktlen)) {
                    if (itest == 0){ // special case, human readable pattern
                        if (NCLK > 1) {
                            int sub = iclock % NCLK;
                            int bx  = (iclock / NCLK) % TMUX_IN;
                            int ev  = (iclock / (TMUX_IN * NCLK)) * NLINKS + j;
                            data[j][i](63,0) = 1 + sub + 10*bx + 1000 * ev;
                        } else {
                            int bx  = iclock % TMUX_IN;
                            int ev  = iclock / TMUX_IN * NLINKS + j;
                            data[j][i](63,0) = 1  + bx + 100 * ev;
                        }
                    } else {
                        data[j][i](63,0) = ap_uint<64>(rand() & 0xFFFFFF);
                    }
                    data[j][i][64] = 1;
                } else {
                    data[j][i] = 0;
                }
            }
        }
#ifdef VERBOSE
        if (itest <= 5) {
            for (unsigned int j = 0; j < NLINKS; ++j) {
                printf("L[%d]: ", j);
                for (unsigned int i = 0; i < NDATA; ++i) printf("%dv%5d | ", int(data[j][i][64]), int(data[j][i](63,0)));
                printf("\n");
            }
        }
#endif

        snprintf(fnbuff, 25, "patterns-in-%d.txt", itest);
        f_patterns_in = fopen(fnbuff, "w");
        snprintf(fnbuff, 25, "patterns-out-%d.txt", itest);
        f_patterns_out = fopen(fnbuff, "w");


        for (unsigned int iclock = 0; iclock <  NDATA; ++iclock) {

            fprintf(f_patterns_in,  "Frame %04u :", iclock);
            fprintf(f_patterns_out, "Frame %04u :", iclock);

            for (unsigned int j = 0; j < NLINKS; ++j) {
                in[j] = data[j][iclock];
                fprintf(f_patterns_in, " %1dv%016llx", int(in[j][64]), in[j](63,0).to_uint64());
            }

            //bool newevt = (iclock == 0);
            bool newevt = (iclock % (NCLK*TMUX_IN)) == 0;
            tdemux(newevt, in, out);
            tdemux_ref(newevt, in, refout);

            for (unsigned int j = 0; j < NLINKS; ++j) {
                fprintf(f_patterns_out, " %1dv%016llx", int(refout[j][64]), refout[j](63,0).to_uint64());
            }

            bool ok = true; //(ret == ref);
            for (unsigned int j = 0; j < NLINKS; ++j) {
                ok = ok && (out[j] == refout[j]);
            }

#ifdef VERBOSE
            if (itest <= 2) {
                printf("%04d |  ", iclock);
                for (unsigned int j = 0; j < NLINKS; ++j) printf("%dv%9d ", int(in[j][64]), int(in[j](63,0)));
                printf(" |   ");
                for (unsigned int j = 0; j < NLINKS; ++j) printf("%dv%9d ", int(out[j][64]), int(out[j](63,0)));
                printf(" |   ");
                for (unsigned int j = 0; j < NLINKS; ++j) printf("%dv%9d ", int(refout[j][64]), int(refout[j](63,0)));
                printf(ok ? "\n" : "   <=== ERROR \n");
            }
#endif

            if (!ok) isok = false;

            fprintf(f_patterns_in, "\n");
            fprintf(f_patterns_out, "\n");
        }
        if (!isok) {
            printf("\ntest %d failed\n", itest);
            return 1;
        } else {
            printf("\ntest %d passed\n", itest);
        }
        fclose(f_patterns_in);
        fclose(f_patterns_out);
        // feed nulls into the tdemux to clean up the statics before the next test
        for (int i = 0; i < NLINKS; ++i) in[i] = 0;
        for (int i = 0; i < 6*PAGESIZE+10; ++i) {
            tdemux(i == 0, in, out);
            tdemux_ref(i == 0, in, out);

        }
    }
    return 0;
}
