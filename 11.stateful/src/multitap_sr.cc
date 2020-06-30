#include "multitap_sr.h"

bool multitap_sr_push_simple(bool newRecord, const ap_uint<16> newValue, ap_uint<16> tap[NTAPS]) {
    #pragma HLS PIPELINE ii=1
    #pragma HLS ARRAY_PARTITION variable=tap complete
    #pragma HLS INTERFACE ap_none port=tap
    static ap_uint<5> nitems = 0;
    static ap_uint<16> cells[NTAPS];
    #pragma HLS ARRAY_PARTITION variable=cells complete

    nitems = newRecord ? 1 : int(nitems)+1;
    for (unsigned int i = NTAPS-1; i > 0; --i) {
        #pragma HLS unroll
        cells[i] = newRecord ? ap_uint<16>(0) : cells[i-1];
    }
    cells[0] = newValue;
    // push output
    for (unsigned int i = 0; i < NTAPS; ++i) {
        #pragma HLS unroll
        tap[i] = cells[NTAPS-1-i];
    }
    return (nitems >= NTAPS);
}

bool sorting_multitap_sr_push_simple(bool newRecord, const ap_uint<16> newValue, ap_uint<16> tap[NTAPS]) {
    #pragma HLS PIPELINE ii=1
    #pragma HLS ARRAY_PARTITION variable=tap complete
    #pragma HLS INTERFACE ap_none port=tap
    static ap_uint<5> nitems = 0;
    static ap_uint<16> cells[NTAPS];
    #pragma HLS ARRAY_PARTITION variable=cells complete

#if 1
    bool below[NTAPS];
    #pragma HLS ARRAY_PARTITION variable=below complete
    for (int i = 0; i < NTAPS; ++i) below[i] = !newRecord && (cells[i] <= newValue);

#if 0
    for (int i = NTAPS-1; i >= 1; --i) {
        if      (below[i])  cells[i] = (below[i-1] ? cells[i-1] : newValue);
        else if (newRecord) cells[i] = 0;
    }
    if (newRecord || below[0]) cells[0] = newValue;
#else
    for (int i = NTAPS-1; i >= 1; --i) {
        cells[i] = below[i] ? (below[i-1] ? cells[i-1]     : newValue) :
                              (newRecord  ? ap_uint<16>(0) : cells[i]);
    }
    cells[0] = (newRecord || below[0]) ? newValue : cells[0];
#endif
    nitems = newRecord ? 1 : int(nitems)+1;
#else
    if (newRecord) {
        for (int i = NTAPS-1; i >= 0; --i) {
            cells[i] = i ? ap_uint<16>(0) : newValue;
        }
        nitems = 1;
    } else {
        // insertion in sorted list
        for (int i = NTAPS-1; i >= 0; --i) {
            if (cells[i] <= newValue) {
                if (i > 0 && cells[i-1] <= newValue) {
                    cells[i] = cells[i-1];
                } else {
                    cells[i] = newValue;
                }
            }
        }
        nitems++;
    }
#endif
    // push output
    for (unsigned int i = 0; i < NTAPS; ++i) {
        #pragma HLS unroll
        tap[i] = cells[i];
    }
    return (nitems >= NTAPS);
}

