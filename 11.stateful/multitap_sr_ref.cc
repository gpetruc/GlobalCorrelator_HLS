#include "src/multitap_sr.h"
#include <algorithm>

bool sorting_multitap_sr_push_simple_ref(bool newRecord, const ap_uint<16> newValue, ap_uint<16> tap[NTAPS]) {
    static ap_uint<5> nitems = 0;
    static ap_uint<16> cells[NTAPS+1];
    if (newRecord) {
        for (unsigned int i = 0; i <= NTAPS; ++i) cells[i] = 0;
        nitems = 0;
    } 
    cells[0] = newValue; 
    nitems++;
    std::sort(&cells[0], &cells[NTAPS+1]);

    for (unsigned int i = 0; i < NTAPS; ++i) {
        tap[i] = cells[NTAPS-i];
    }
    return (nitems >= NTAPS);
 
}


