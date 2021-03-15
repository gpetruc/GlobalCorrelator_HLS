#include "firmware/tdemux.h"
#include "tdemux_ref.h"
#include <algorithm>
#include <cstdio>

TDemuxRef::TDemuxRef() {
    inputState_ = Wait;
    nWritten_ = 0;
    outputState_ = Wait;
    nRead_ = 0;
    nToWrite_ = 0;
    readLink_ = 0;
    firstWriteLink_ = 0;
}

void TDemuxRef::clear() {
    for (int i = 0; i < NLINKS; ++i) { 
        while (!buffer_[i].empty()) buffer_[i].pop(); 
    }
    inputState_ = Wait;
    nWritten_ = 0;
    outputState_ = Wait;
    nRead_ = 0;
    nToWrite_ = 0;
    readLink_ = 0;
    firstWriteLink_ = 0;
}
void TDemuxRef::operator()(const w65 links[NLINKS], w65 out[NLINKS]) {
    // ----------------
    if (inputState_ == Wait) {
        int nOn = 0, iOn = -1;
        for (int i = 0; i < NLINKS; ++i) {
            if (links[i][64]) { nOn++; iOn = i; }
        }
        assert(nOn <= 1);
        if (nOn == 1) {
            inputState_ = Run;
            firstWriteLink_ = iOn;
            nWritten_ = 0;
        }
    } else if (inputState_ == Check) {
        if (links[firstWriteLink_][64]) { // new train
            inputState_ = Run;
        } else {
            inputState_ = Finish;
            nToWrite_ = TMUX_OUT*NCLK*(NLINKS-1)+1;
        }
    } 
    // intentionally not part of the previous IF, as we can get there if the previous ones set inputState = Run
    if (inputState_ == Run) {
        for (int i = 0; i < NLINKS; ++i) {
            int j = (firstWriteLink_ + i) % NLINKS;
            bool linkEnabled = (nWritten_ >= TMUX_OUT*NCLK*i);
            if (linkEnabled) buffer_[j].push(links[j]);
        }
        nWritten_++;
        if (nWritten_ % PAGESIZE == 0) {
            inputState_ = Check; // we may be at the end of a train
        }
    } else if (inputState_ == Finish) {
        for (int i = 0; i < NLINKS; ++i) {
            int j = (firstWriteLink_ + i) % NLINKS;
            bool linkEnabled = (nToWrite_ > TMUX_OUT*NCLK*(NLINKS-1-i)+1);
            if (linkEnabled) buffer_[j].push(links[j]);
        }
        nToWrite_--;
        if (nToWrite_ == 0) {
            inputState_ = Wait;
        }
    }
    // ----------------
    if (outputState_ == Wait) {
        if (nWritten_ > (NLINKS-1)*TMUX_OUT*NCLK+1) {
            outputState_ = Run;
            readLink_ = firstWriteLink_;
            nRead_ = 0;
        }
    }
    if (outputState_ == Run) {
        for (int i = 0; i < NLINKS; ++i) {
            if (buffer_[readLink_].empty()) {
                printf("ERROR: buffer[%d] is empty. nWritten %u, nRead %u, firstWriteLink %d, ", readLink_, nWritten_, nRead_, firstWriteLink_);
                for (int j = 0; j < NLINKS; ++j) {
                    printf("buffer[%d].size %lu, ", j, buffer_[j].size());
                }
                printf("\n"); fflush(stdout);

            }
            assert(!buffer_[readLink_].empty());
            out[i] = buffer_[readLink_].front(); 
            buffer_[readLink_].pop();
        }
        nRead_++;
        if ((nRead_ % (TMUX_OUT*NCLK)) == 0) {
            readLink_ = (readLink_+1) % NLINKS;
        }
        if (nRead_ == nWritten_) {
            outputState_ = Wait;
            nWritten_ = 0;
            nRead_ = 0;
        }
    } else {
        for (int i = 0; i < NLINKS; ++i) out[i] = 0;
    }
    /*
    printf("after: inputState %1d, nWritten %3u, nToWrite %3u, firstWriteLink %d, outputState %1d, nRead %3u, readLink %d ", 
            int(inputState_), nWritten_, nToWrite_, firstWriteLink_, int(outputState_), nRead_, readLink_);
    for (int j = 0; j < NLINKS; ++j) {
        printf("buff[%d] %4lu, ", j, buffer_[j].size());
    }
    printf(""); fflush(stdout);
    */
}

void TDemuxRef::operator()(const w64 links[NLINKS], const bool valid[NLINKS], 
                                   w64 out[NLINKS],         bool vout[NLINKS]) {
    w65 links65[NLINKS], out65[NLINKS];
    for (int i = 0; i < NLINKS; ++i) {
        links65[i](63,0) = links[i];
        links65[i][64]   = valid[i];
    }
    (*this)(links65, out65);
    for (int i = 0; i < NLINKS; ++i) {
        out[i]  = out65[i](63,0);
        vout[i] = out65[i][64];
    }
}
