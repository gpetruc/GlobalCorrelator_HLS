#ifndef multififo_regionizer_tdemux_ref_h
#define multififo_regionizer_tdemux_ref_h

#include "firmware/tdemux.h"
#include <queue>

typedef ap_uint<64> w64;

class TDemuxRef {
    public:
        TDemuxRef() ;
        void operator()(const w65 links[NLINKS], w65 out[NLINKS]) ;
        void operator()(const w64 links[NLINKS], const bool valid[NLINKS], 
                              w64 out[NLINKS],         bool vout[NLINKS]) ;
        void clear() ;
    private:
        enum State { Wait, Run, Check, Finish } inputState_, outputState_;
        unsigned int readLink_, firstWriteLink_, nWritten_, nRead_, nToWrite_;
        std::queue<w65> buffer_[NLINKS];

};

#endif
