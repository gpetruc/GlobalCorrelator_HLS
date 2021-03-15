#include "firmware/tdemux.h"
#include <cstdio>
#include <cstdlib>
#include "tdemux_ref.h"

#define NEVTEST 5
int main() {
    srand(125);
    const int NDATA = TMUX_IN*NCLK*NEVTEST+10;
    std::vector<w65> data[NLINKS];
    w65 in[NLINKS], out[NLINKS], refout[NLINKS];

    FILE * f_patterns_in, * f_patterns_out; char fnbuff[25]; 

    unsigned int maxlen = TMUX_IN*NCLK;

    for (unsigned int itest = 0, ntest = 20; itest <= ntest; ++itest) {
        TDemuxRef tdemux_ref;
        // create some input data
        bool isok = true;
        for (unsigned int j = 0; j < NLINKS; ++j) {
            data[j].clear();
            for (unsigned int i = 0; i < j*NCLK*TMUX_OUT; ++i) {
                data[j].push_back(0);
            }
            for (unsigned int iev = 0; iev < NEVTEST; ++iev) {
                unsigned int pktlen = (PAGESIZE/5) + rand() % (PAGESIZE*4/5);
                for (unsigned int iclock = 0; iclock < TMUX_IN*NCLK; ++iclock) {
                    w65 word = 0;
                    if (iclock < pktlen) {
                        word[64] = 1;
                        if (itest <= 3){ // special case, human readable pattern
                            int sub = iclock % NCLK;
                            int bx  = (iclock / NCLK) % TMUX_IN;
                            int ev  = iev * NLINKS + j;
                            word(63,0) = 1 + sub + 10*bx + 1000 * ev;
                        } else {
                            word(63,0) = ap_uint<64>(rand() & 0xFFFFFF);
                        }
                    }
                    data[j].push_back(word);
                }
                if (itest == 1) break;
                if (itest == 2 && iev == 0) {
                    for (unsigned int i = 0; i < 200; ++i) {
                        data[j].push_back(0);
                    }
                }
            }
        }

        snprintf(fnbuff, 25, "patterns-in-%d.txt", itest);
        f_patterns_in = fopen(fnbuff, "w");
        snprintf(fnbuff, 25, "patterns-out-%d.txt", itest);
        f_patterns_out = fopen(fnbuff, "w");

        tdemux_errflags errs; 
        tdemux_istate   istate; 
        tdemux_ostate   ostate;

        for (unsigned int iclock = 0; iclock <  NDATA; ++iclock) {

            fprintf(f_patterns_in,  "Frame %04u :", iclock);
            fprintf(f_patterns_out, "Frame %04u :", iclock);

            for (unsigned int j = 0; j < NLINKS; ++j) {
                in[j] = (iclock < data[j].size() ? data[j][iclock] : w65(0));
                fprintf(f_patterns_in, " %1dv%016llx", int(in[j][64]), in[j](63,0).to_uint64());
            }

            //bool newevt = (iclock == 0);
            tdemux_full(in, out, errs, istate, ostate);
            tdemux_ref(in, refout);

            for (unsigned int j = 0; j < NLINKS; ++j) {
                fprintf(f_patterns_out, " %1dv%016llx", int(refout[j][64]), refout[j](63,0).to_uint64());
            }

            bool ok = true;
            for (unsigned int j = 0; j < NLINKS; ++j) {
                ok = ok && (out[j] == refout[j]);
            }

#ifdef VERBOSE
            if (itest <= 5) {
                printf("%04d | in   ", iclock);
                for (unsigned int j = 0; j < NLINKS; ++j) printf("%dv%9d ", int(in[j][64]), int(in[j](63,0)));
                printf(" | ref   ");
                for (unsigned int j = 0; j < NLINKS; ++j) printf("%dv%9d ", int(refout[j][64]), int(refout[j](63,0)));
                printf(" | out   ");
                for (unsigned int j = 0; j < NLINKS; ++j) printf("%dv%9d ", int(out[j][64]), int(out[j](63,0)));
                printf(" : in %s : out %s : err %s :", istate.msg(), ostate.msg(), errs.msg());
                printf(ok ? "\n" : "   <=== ERROR \n");
            }
#endif

            if (!ok) isok = false;

            fprintf(f_patterns_in, "\n");
            fprintf(f_patterns_out, "\n");
        }
        if (!isok) {
            printf("\ntest %d failed\n", itest);
            //return 1;
        } else {
            printf("\ntest %d passed\n", itest);
        }
        fclose(f_patterns_in);
        fclose(f_patterns_out);
        // feed nulls into the tdemux to clean up the statics before the next test
        for (int i = 0; i < NLINKS; ++i) in[i] = 0;
        for (int i = 0; i < 2*PAGESIZE+1; ++i) {
            tdemux_full(in, out, errs, istate, ostate); //printf("\n");
            #ifdef VERBOSE
            if (itest <= 5) {
                printf("+%03d | in   ", i);
                for (unsigned int j = 0; j < NLINKS; ++j) printf("%dv%9d ", int(in[j][64]), int(in[j](63,0)));
                printf(" | out   ");
                for (unsigned int j = 0; j < NLINKS; ++j) printf("%dv%9d ", int(out[j][64]), int(out[j](63,0)));
                printf(" : in %s : out %s : err %s \n", istate.msg(), ostate.msg(), errs.msg());
            }
            #endif
        }
    }
    return 0;
}
