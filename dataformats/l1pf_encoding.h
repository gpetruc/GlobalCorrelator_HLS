#ifndef FIRMWARE_L1PF_ENCODING_H
#define FIRMWARE_L1PF_ENCODING_H

#include <cassert>

template <unsigned int N, unsigned int OFFS=0, typename T, int NB>
inline void l1pf_pattern_pack(const T objs[N], ap_uint<NB> data[]) {
#pragma HLS inline
  assert(T::BITWIDTH <= NB);
  for (unsigned int i = 0; i < N; ++i) {
    data[i + OFFS] = objs[i].pack();
  }
}

template <unsigned int N, unsigned int OFFS=0, typename T, int NB>
inline void l1pf_pattern_unpack(const ap_uint<NB> data[], T objs[N]) {
#pragma HLS inline
  assert(T::BITWIDTH <= NB);
  for (unsigned int i = 0; i < N; ++i) {
    objs[i] = T::unpack(data[i + OFFS]);
  }
}

#endif
