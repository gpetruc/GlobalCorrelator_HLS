#ifndef multififo_regionizer_tdemux_h
#define multififo_regionizer_tdemux_h

#include <ap_int.h>

typedef ap_uint<65> w65; // bit 64 is used for the valid bit

#define TMUX_IN 18
#define TMUX_OUT 6
#define NLINKS   3 
#define NCLK     9 // clocks per BX (8 = 320 MHz, 9 = 360 MHz)
#define BLKSIZE  (NCLK*TMUX_OUT)
#define PAGESIZE (NCLK*TMUX_IN)

struct tdemux_errflags {
    bool multistart;       // more than one link is at 1 at the start of a frame
    bool badstart0, badstart1, badstart2;
    const char * msg() const ; // not thread safe
};
struct tdemux_istate {
    enum { Wait=0, Run=1, Check=2, Finish=3 };
    ap_uint<2> state;
    ap_uint<2> robin;
    ap_uint<9> counter;
    ap_uint<9> offs0, offs1, offs2;
    const char * msg() const ; // not thread safe
};
struct tdemux_ostate {
    bool running;
    ap_uint<2> robin;
    ap_uint<9> toread, lastread;
    const char * msg() const ; // not thread safe
};


void tdemux_full(const w65 links[NLINKS], w65 out[NLINKS], tdemux_errflags & errs, tdemux_istate & istate, tdemux_ostate & ostate) ;
void tdemux(const w65 links[NLINKS], w65 out[NLINKS]) ;

#endif
