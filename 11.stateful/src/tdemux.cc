#include "tdemux.h"
#include <cassert>

bool tdemux_simple(bool newEvent, const w64 links[NLINKS], w64 out[NLINKS]) {
    #pragma HLS PIPELINE ii=1
    #pragma HLS ARRAY_PARTITION variable=links complete
    #pragma HLS ARRAY_PARTITION variable=out complete
    //#pragma HLS INTERFACE ap_none port=out

    typedef ap_uint<9> offs_t;    // must count up to TMUX_IN * NLCK * 2
    typedef ap_uint<9> counter_t; // must count up to TMUX_IN * NLCK * 2
    typedef ap_uint<2> robin_t;   // must count up to NLINKS

    static counter_t   elcounter = 0, readcount = 0, toread = 0; 
    static bool        fold[NLINKS];    // we use twice as much memory, to avoid contentions
    static offs_t      offs[NLINKS];
    static robin_t     robin, readrobin, blkcounter;
    static bool        readvalid = false, pre_readvalid = false;
    #pragma HLS ARRAY_PARTITION variable=offs complete

    const unsigned int MEMSIZE = 2*PAGESIZE;
    static w64 buffer[NLINKS][MEMSIZE];
    #pragma HLS ARRAY_PARTITION variable=buffer dim=1 complete
    #pragma HLS DEPENDENCE variable=buffer intra false

    if (newEvent) {
        elcounter = 0; blkcounter = 1;
        robin = 0; readvalid = false; pre_readvalid = false;
        for (int i = 0; i < NLINKS; ++i) {
            //fold[i] = (i == 0 ? 1 : 0);
            offs[i]  =  i * BLKSIZE + (i == 0 ? PAGESIZE : 0);
        }
        toread = -1;
#ifndef __SYNTHESIS__
        //printf("FW: Initialized\n");
#endif
    }


#if 1
    assert(NLINKS==3);
    switch(robin) {
        case 0:
            buffer[0][offs[0]] = links[0];
            buffer[1][offs[1]] = links[1];
            buffer[2][offs[2]] = links[2];
            break;
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
#else
    for (int i = 0; i < NLINKS; ++i) {
        buffer[(robin+i)%NLINKS][offs[i]] = links[i];
    }
#endif
    robin++;
    elcounter++;

#if 0 && defined(__SYNTHESIS__)
    printf("FW: robin %d, elcounter %4d, blkcounter %4d, toread %6d, readrobin %d, readcount %3d\n", 
                int(elcounter), int(blkcounter), int(robin), int(toread), int(readrobin), int(readcount));
    for (unsigned int j = 0; j < NLINKS; ++j) {
        printf("FW: M%d  fold %d  offs %5d : ", j, fold[j] ? 1 : 0, int(offs[j]));
        for (unsigned int i = 0; i < MEMSIZE; ++i) {
            printf("%5d%c | ", int(buffer[j][i]), i == offs[(NLINKS-int(robin)+1+j)%NLINKS] ? '!' : ' ');
        }
        printf("\n");
    } 
#endif

    if (robin == NLINKS) {
        robin = 0;
        if ((elcounter == BLKSIZE)) {
            int completed = blkcounter; // index of the link whose buffer we have filled
#ifndef __SYNTHESIS__
            //printf("FW: Swap block for %d as counter %d is %d\n", completed, int(elcounter), BLKSIZE);
#endif

#if 1          
            for (int i = 0; i < NLINKS; ++i) {
                if (i == blkcounter) {
                    if (offs[i] > PAGESIZE) {
                        offs[i] = i*BLKSIZE;
                    } else {
                        offs[i] = i*BLKSIZE + PAGESIZE;
                    }
                } else {
                    offs[i]++;
                } 
           }
  
#else
            fold[completed] = !fold[completed];  // fold that one around
            for (int i = 0; i < NLINKS; ++i) {
                if (i == completed) {
                    offs[i] = i * BLKSIZE + (fold[i] ? PAGESIZE : 0);
                } else {
                    offs[i]++;
                }
            }
#endif

            elcounter = 0;

            blkcounter++; if (blkcounter == NLINKS) blkcounter = 0;

        } else  {
#ifndef __SYNTHESIS__
            //printf("FW: Increase offset as counter %d is not multiple of %d\n", int(elcounter), BLKSIZE);
#endif
            for (int i = 0; i < NLINKS; ++i) {
                offs[i]++;
            }
        }
    }
    if (!readvalid) {
        if (elcounter == 0 && blkcounter == 0) { 
#ifndef __SYNTHESIS__
            //printf("FW: elcounter %4d, blkcounter %6d, robin %d -> trigger pre_readvalid\n", int(elcounter), int(blkcounter), int(robin));
#endif
            pre_readvalid = true;
        } else if (pre_readvalid) {
#ifndef __SYNTHESIS__
            //printf("FW: elcounter %4d, blkcounter %6d, robin %d -> trigger readvalid\n", int(elcounter), int(blkcounter), int(robin));
#endif
            readvalid = true;
            toread = PAGESIZE;
            readrobin = 0; readcount = 0;
        } else {
#ifndef __SYNTHESIS__
            //printf("FW: elcounter %4d, blkcounter %6d, robin %d -> wait readvalid\n", int(elcounter), int(blkcounter), int(robin));
#endif
        }
        for (int i = 0; i < NLINKS; ++i) {
            out[i] = 0;
        }
        return false;
    } else {
#ifndef __SYNTHESIS__
        //printf("FW: toread %6d, readrobin %d, readcount %3d\n", int(toread), int(readrobin), int(readcount));
#endif
        for (int i = 0; i < NLINKS; ++i) {
            out[i] = buffer[(i+readrobin)%NLINKS][toread];
        }

        toread++; if (toread == MEMSIZE) toread = 0;

        readcount++; 
        if (readcount == BLKSIZE) {
            readcount = 0;
            readrobin++;
            if (readrobin == NLINKS) readrobin = 0;
        }
        return true;
    }
}


bool tdemux_simple_ref(bool newEvent, const w64 links[NLINKS], w64 out[NLINKS]) {
    static int   counter; // must count up to TMUX_IN * NLCK
    static bool  fold[NLINKS];    // we use twice as much memory, to avoid contentions
    static int   offs[NLINKS];
    static int   robin;
    static int   toread = -1, readrobin = 0, readcount = 0;

    const unsigned int MEMSIZE = 2*PAGESIZE;
    static w64 buffer[NLINKS][MEMSIZE];

    if (newEvent) {
        counter  = 0;
        robin = 0;
        for (int i = 0; i < NLINKS; ++i) {
            fold[i] = (i == 0 ? 1 : 0);
            offs[i]  =  i * BLKSIZE + (fold[i] ? PAGESIZE : 0);
        }
        toread = -1;
        //printf("Initialized\n");
    }
    for (int i = 0; i < NLINKS; ++i) {
        buffer[(robin+i)%NLINKS][offs[i]] = links[i];
    }
    robin++;
    counter++;

    /*printf("counter %4d, robin %d, toread %6d, readrobin %d, readcount %3d\n", counter, robin, toread, readrobin, readcount);
    for (unsigned int j = 0; j < NLINKS; ++j) {
        printf("M%d  fold %d  offs %5d : ", j, fold[j] ? 1 : 0, offs[j]);
        for (unsigned int i = 0; i < MEMSIZE; ++i) {
            printf("%5d%c | ", int(buffer[j][i]), i == offs[(NLINKS-robin+1+j)%NLINKS] ? '!' : ' ');
        }
        printf("\n");
    }*/

    if (robin == NLINKS) {
        robin = 0;
        if ((counter % BLKSIZE) == 0) {
            int completed = (counter/BLKSIZE)%NLINKS; // index of the link whose buffer we have filled
            //printf("Swap block for %d as counter %d is multiple of %d, %d\n", completed, counter, BLKSIZE, (counter % BLKSIZE));
            fold[completed] = !fold[completed];  // fold that one around
            for (int i = 0; i < NLINKS; ++i) {
                offs[i]  =  (i == completed ? i * BLKSIZE + (fold[i] ? PAGESIZE : 0) : offs[i]+1);
            }
        } else  {
            //printf("Increase offset as counter %d is not multiple of %d\n", counter, BLKSIZE);
            for (int i = 0; i < NLINKS; ++i) {
                offs[i]++;
            }
        }
    }
    if (toread == -1) {
        //printf("counter %4d, threshold %4d, toread %6d, readrobin %d, readcount %3d\n", counter, (NLINKS-1)*BLKSIZE, toread, readrobin, readcount);
        if (counter == (NLINKS-1)*BLKSIZE + 1) { 
            toread = PAGESIZE;
            readrobin = 0; readcount = 0;
        }
        for (int i = 0; i < NLINKS; ++i) {
            out[i] = 0;
        }
        return false;
    } else {
        for (int i = 0; i < NLINKS; ++i) {
            out[i] = buffer[(i+readrobin)%NLINKS][toread];
        }
        toread    = (toread+1)    % MEMSIZE;
        readcount = (readcount+1) % BLKSIZE;
        if (readcount == 0) readrobin = (readrobin+1) % NLINKS;
        return true;
    }
}


