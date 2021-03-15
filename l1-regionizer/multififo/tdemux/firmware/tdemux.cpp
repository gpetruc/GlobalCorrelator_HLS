#include "tdemux.h"
#include <cassert>
#ifndef __SYNTHESIS__
#include <cstdio>
#endif
void tdemux(const w65 links[NLINKS], w65 out[NLINKS]) {
    tdemux_errflags errs; 
    tdemux_istate   istate; 
    tdemux_ostate   ostate;
    tdemux_full(links, out, errs, istate, ostate);
}


void tdemux_full(const w65 links[NLINKS], w65 out[NLINKS], tdemux_errflags & errs, tdemux_istate & istate, tdemux_ostate & ostate) {
    assert(NLINKS==3);
    #pragma HLS PIPELINE ii=1
    #pragma HLS ARRAY_PARTITION variable=links complete
    #pragma HLS ARRAY_PARTITION variable=out complete
    #pragma HLS INTERFACE ap_none port=out
    #pragma HLS INTERFACE ap_none port=errs
    #pragma HLS INTERFACE ap_none port=istate
    #pragma HLS INTERFACE ap_none port=ostate

    typedef ap_uint<9> offs_t;    // must count up to TMUX_IN * NLCK * 2
    typedef ap_uint<9> counter_t; // must count up to TMUX_IN * NLCK * 2
    typedef ap_uint<2> robin_t;   // must count up to NLINKS
 
    static counter_t   wcounter = 0, readcount = 0, toread = 0, lastread = PAGESIZE-1; 
    static offs_t      offs[NLINKS] = { 0, BLKSIZE*5/3, BLKSIZE*7/3 };
    #pragma HLS ARRAY_PARTITION variable=offs complete
    static robin_t     robin = 0, readrobin = 0;

    static bool wasvalid[NLINKS] = { false, false, false };
    #pragma HLS ARRAY_PARTITION variable=wasvalid complete

    static w65 buffer[NLINKS][PAGESIZE];
    #pragma HLS ARRAY_PARTITION variable=buffer dim=1 complete
    #pragma HLS DEPENDENCE variable=buffer intra false

    static enum { Wait=0, Run=1, Check=2, Finish=3 } inputState;
    static bool outputRun = false;

    if (outputRun) {
        assert(0 <= toread && toread < PAGESIZE);
        for (int i = 0; i < NLINKS; ++i) {
            out[i] = buffer[(i+readrobin)%NLINKS][toread];
        }

        if ((inputState == Finish || inputState == Wait) && toread == lastread) {
            outputRun = false;
        }

        if (toread == PAGESIZE-1) {
            toread = 0;
        } else {
            toread++;
        }

        if (readcount == BLKSIZE - 1) {
            readcount = 0;
            if (readrobin == NLINKS - 1) {
                readrobin = 0;
            } else {
                readrobin++;
            }
        } else {
            readcount++; 
        }
    } else {
        for (int i = 0; i < NLINKS; ++i) {
            out[i] = 0;
        }
        if (inputState == Run && wcounter == 2*BLKSIZE) {
            outputRun = true;
        } 
    }

    assert(0*BLKSIZE <= offs[0] && offs[0] < 1 * BLKSIZE); 
    assert(1*BLKSIZE <= offs[1] && offs[1] < 2 * BLKSIZE); 
    assert(2*BLKSIZE <= offs[2] && offs[2] < 3 * BLKSIZE); 
    switch(robin) {
        case 0:
            buffer[0][offs[0]] = links[0];
            buffer[1][offs[1]] = links[1];
            buffer[2][offs[2]] = links[2];
            break;
        case 1:
            buffer[1][offs[0]] = links[0];
            buffer[2][offs[1]] = links[1];
            buffer[0][offs[2]] = links[2];
            break;
        case 2:
            buffer[2][offs[0]] = links[0];
            buffer[0][offs[1]] = links[1];
            buffer[1][offs[2]] = links[2];
            break;
    }

    // error checking
    errs.multistart = (inputState == Wait) && links[0][64] && (links[1][64] || links[2][64]);
    errs.badstart0 = (links[0][64] && !wasvalid[0]) && !(inputState == Wait || inputState == Check);
    errs.badstart1 = (links[1][64] && !wasvalid[1]) && ( !(inputState == Run || inputState == Finish) || (wcounter != 1 * BLKSIZE));
    errs.badstart2 = (links[2][64] && !wasvalid[2]) && ( !(inputState == Run || inputState == Finish) || (wcounter != 2 * BLKSIZE));

    if (inputState != Wait || links[0][64]) {
        // update counters and state after write
        if (robin != NLINKS-1) {
            robin++;
        } else {
            robin = 0;
            for (int i = 0; i < NLINKS; ++i) {
                if (offs[i] == (i+1)*BLKSIZE-1) {
                    offs[i] = i*BLKSIZE;
                } else {    
                    offs[i]++;
                }
            }
        }
        bool lastpage = (wcounter == PAGESIZE-1);
        if (!lastpage) {
            wcounter++;
        } else {
            wcounter = 0;
        }
        switch (inputState) {
            case Wait:
                inputState = Run; 
                break;
            case Run: 
                if (lastpage) inputState = Check; 
                break;
            case Check:
                inputState = links[0][64] ? Run : Finish; 
                break;
            case Finish:
                if (wcounter == (NLINKS-1)*BLKSIZE) inputState = Wait;
                break;
        }

    } else {
        offs[0] = 0;
        offs[1] = BLKSIZE*5/3;
        offs[2] = BLKSIZE*7/3;
        robin = 0; 
        wcounter = 0;
    }


    for (int i = 0; i < NLINKS; ++i) {
        wasvalid[i] = links[i][64];
    }

#if 0 //ndef __SYNTHESIS__
    printf("final: input %c, robin %d, wcount %3d offs %3d/%3d/%3d: output %c, robin %d, toread %3d : wrote %d, read %d, available %3d ||  ",
            (inputState == Run ? 'R' : (inputState == Check ? 'C' : (inputState == Finish ? 'F' : 'W'))), 
            robin.to_int(), wcounter.to_int(), 
            offs[0].to_int(), offs[1].to_int(), offs[2].to_int(),
            (outputRun ? 'W' : '-'),
            readrobin.to_int(), toread.to_int(),
            int(wrote), int(read), available.to_int());
#endif
    istate.state = inputState;
    istate.robin = robin;
    istate.counter = wcounter;
    istate.offs0 = offs[0];
    istate.offs1 = offs[1];
    istate.offs2 = offs[2];

    ostate.running = outputRun;
    ostate.robin = readrobin;
    ostate.toread = toread;
    ostate.lastread = lastread;
}

#ifndef __SYNTHESIS__
const char * tdemux_errflags::msg() const {
    static char buff[5];
    snprintf(buff,5,"%c%c%c%c", 
                multistart ? 'M' : '.', 
                badstart0 ? '0' : '.',
                badstart1 ? '1' : '.',
                badstart2 ? '2' : '.');
    return buff;
}


const char * tdemux_istate::msg() const {
    static char buff[255];
    snprintf(buff,254,"state %c robin %d counter %3d offs %3d/%3d/%3d",
                (state == Run ? 'R' : (state == Check ? 'C' : (state == Finish ? 'F' : 'W'))),
                robin.to_int(), counter.to_int(), offs0.to_int(), offs1.to_int(), offs2.to_int());
    return buff;
}

const char * tdemux_ostate::msg() const {
    static char buff[255];
    snprintf(buff,254,"state %c robin %d toread %3d lastread %3d",
                (running ? 'R' : 'W'),
                robin.to_int(), toread.to_int(), lastread.to_int());
    return buff;
}

#endif
