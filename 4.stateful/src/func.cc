#include "func.h"
#ifndef __SYNTHESIS__
#include <cstdio>
#endif

void tkmu_receiver(bool newEvent, ap_uint<64> data, TkMu out[12],
                   bool &newEventOut) {
    #pragma HLS pipeline ii=1
    #pragma HLS array_partition variable=out complete

    static ap_uint<5> counter_ = 0; 
    static ap_uint<2> counter3_ = 0; 
    static ap_uint<96> words_[12];
    #pragma HLS array_partition variable=words_ complete
#ifndef __SYNTHESIS__
    //printf("in %1d %016lx | state %2d %2d ", int(newEvent), data.to_uint64(), counter_.to_int(), counter3_.to_int());
    //for (int i = 0; i < 12; ++i) printf(" %26s", words_[i].to_string(16).c_str());
    //printf("\n");
#endif
    newEventOut = false;
    if (newEvent) {
        words_[10](63,0) = data;
        counter_ = 1;
        counter3_ = 2;
    } else {
        switch (counter3_) {
            case 0:
                break;
            case 1:
                for (int i = 0; i < 10; ++i) {
                    words_[i] = words_[i+2];
                }
                words_[10](63,0) = data;
                counter3_ = 2;
                break;
            case 2:
                words_[10](95,64) = data(31,0);
                words_[11](95,64) = data(63,32);
                counter3_ = 3;
                break;
            case 3:
                words_[11](63,0) = data;
                if (counter_ == 18) {
                    newEventOut = true;
                    counter3_ = 0;
                } else {
                    counter3_ = 1;
                }
                break;
        }
    }
    for (int i = 0; i < 12; ++i)
        out[i].unpack(words_[i]);
    counter_ = counter3_ == 0 ? counter_ : ap_uint<5>(counter_ + 1);
}