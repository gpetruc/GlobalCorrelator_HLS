#ifndef multitap_sr_h
#define multitap_sr_h

#include <ap_int.h>
#include <hls_stream.h>

typedef ap_uint<64> w64;

#define TMUX_IN 18
#define TMUX_OUT 6
#define NLINKS   3 
#define NCLK     9 // clocks per BX (8 = 320 MHz, 9 = 360 MHz)
#define BLKSIZE  (NCLK*TMUX_OUT)
#define PAGESIZE (NCLK*TMUX_IN)

bool tdemux_simple(bool newEvent, const w64 links[NLINKS], w64 out[NLINKS]) ;
bool tdemux_simple_ref(bool newEvent, const w64 links[NLINKS], w64 out[NLINKS]) ;

#endif
