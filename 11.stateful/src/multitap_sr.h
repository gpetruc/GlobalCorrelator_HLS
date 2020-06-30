#ifndef multitap_sr_h
#define multitap_sr_h

#include <ap_int.h>
#include <hls_stream.h>

#define NTAPS 20

bool multitap_sr_push_simple(bool newRecord, const ap_uint<16> newValue, ap_uint<16> tap[NTAPS]);
bool sorting_multitap_sr_push_simple(bool newRecord, const ap_uint<16> newValue, ap_uint<16> tap[NTAPS]);
bool sorting_multitap_sr_push_simple_ref(bool newRecord, const ap_uint<16> newValue, ap_uint<16> tap[NTAPS]);

#endif
