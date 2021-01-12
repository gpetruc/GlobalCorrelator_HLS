#ifndef FIRMWARE_L1PF_ENCODING_H
#define FIRMWARE_L1PF_ENCODING_H

inline ap_uint<64> l1pf_pattern_pack_one(const EmCaloObj &emcalo) {
#pragma HLS inline
  ap_uint<64> data = 0;
  data(31 + 32, 16 + 32) = emcalo.hwPtErr;
  data(15 + 32, 0 + 32) = emcalo.hwPt;
  data(9, 0) = emcalo.hwEta;
  data(19, 10) = emcalo.hwPhi;
  return data;
}

template <unsigned int N, unsigned int OFFS>
inline void l1pf_pattern_pack(const EmCaloObj emcalo[N], ap_uint<64> data[]) {
#pragma HLS inline
  for (unsigned int i = 0; i < N; ++i) {
    data[i + OFFS] = l1pf_pattern_pack_one(emcalo[i]);
  }
}
inline ap_uint<64> l1pf_pattern_pack_one(const HadCaloObj &hadcalo) {
#pragma HLS inline
  ap_uint<64> data = 0;
  data(31 + 32, 16 + 32) = hadcalo.hwEmPt;
  data(15 + 32, 0 + 32) = hadcalo.hwPt;
  data(9, 0) = hadcalo.hwEta;
  data(19, 10) = hadcalo.hwPhi;
  data[20] = hadcalo.hwIsEM;
  return data;
}

template <unsigned int N, unsigned int OFFS>
inline void l1pf_pattern_pack(const HadCaloObj hadcalo[N], ap_uint<64> data[]) {
#pragma HLS inline
  for (unsigned int i = 0; i < N; ++i) {
    data[i + OFFS] = l1pf_pattern_pack_one(hadcalo[i]);
  }
}

inline ap_uint<64> l1pf_pattern_pack_one(const TkObj &track) {
#pragma HLS inline
  ap_uint<64> data = 0;
  data(31 + 32, 16 + 32) = track.hwPtErr;
  data(15 + 32, 0 + 32) = track.hwPt;
  data(9, 0) = track.hwEta;
  data(19, 10) = track.hwPhi;
  data(29, 20) = track.hwZ0;
  data[30] = (track.hwQuality & TkObj::PFTIGHT) ? 1 : 0;
  data[31] = track.hwCharge;
  return data;
}

template <unsigned int N, unsigned int OFFS>
inline void l1pf_pattern_pack(const TkObj track[N], ap_uint<64> data[]) {
#pragma HLS inline
  for (unsigned int i = 0; i < N; ++i) {
    data[i + OFFS] = l1pf_pattern_pack_one(track[i]);
  }
}

inline ap_uint<64> l1pf_pattern_pack_one(const MuObj &mu) {
  ap_uint<64> data = 0;
  //data(31 + 32, 16 + 32) = mu.hwPtErr;
  data(15 + 32, 0 + 32) = mu.hwPt;
  data(9, 0) = mu.hwEta;
  data(19, 10) = mu.hwPhi;
  return data;
}

template <unsigned int N, unsigned int OFFS>
inline void l1pf_pattern_pack(const MuObj mu[N], ap_uint<64> data[]) {
#pragma HLS inline
  for (unsigned int i = 0; i < N; ++i) {
    data[i + OFFS] = l1pf_pattern_pack_one(mu[i]);
  }
}

inline ap_uint<64> l1pf_pattern_pack_one(const PFChargedObj pfch) {
#pragma HLS inline
  ap_uint<64> data = 0;
  data(18 + 32, 16 + 32) = pfch.hwId.bits;
  data(15 + 32, 0 + 32) = pfch.hwPt;
  data(9, 0) = pfch.hwEta;
  data(19, 10) = pfch.hwPhi;
  data(29, 20) = pfch.hwZ0;
  return data;
}

template <unsigned int N, unsigned int OFFS>
inline void l1pf_pattern_pack(const PFChargedObj pfch[N], ap_uint<64> data[]) {
#pragma HLS inline
  for (unsigned int i = 0; i < N; ++i) {
    data[i + OFFS] = l1pf_pattern_pack_one(pfch[i]);
  }
}

inline ap_uint<64> l1pf_pattern_pack_one(const PFNeutralObj pfne) {
#pragma HLS inline
  ap_uint<64> data = 0;
  data(15 + 32, 0 + 32) = pfne.hwPt;
  data(9, 0) = pfne.hwEta;
  data(19, 10) = pfne.hwPhi;
  data(22, 20) = pfne.hwId.bits;
  return data;
}

template <unsigned int N, unsigned int OFFS>
inline void l1pf_pattern_pack(const PFNeutralObj pfne[N], ap_uint<64> data[]) {
#pragma HLS inline
  for (unsigned int i = 0; i < N; ++i) {
    data[i + OFFS] = l1pf_pattern_pack_one(pfne[i]);
  }
}

inline ap_uint<64> l1pf_pattern_pack_one(const PuppiObj pup) {
#pragma HLS inline
  ap_uint<64> data = 0;
  data(18 + 32, 16 + 32) = pup.hwId.bits;
  data(15 + 32, 0 + 32) = pup.hwPt;
  data(9, 0) = pup.hwEta;
  data(19, 10) = pup.hwPhi;
  data(31, 20) = pup.hwData;
  return data;
}

template <unsigned int N, unsigned int OFFS>
inline void l1pf_pattern_pack(const PuppiObj pup[N], ap_uint<64> data[]) {
#pragma HLS inline
  for (unsigned int i = 0; i < N; ++i) {
    data[i + OFFS] = l1pf_pattern_pack_one(pup[i]);
  }
}

inline void l1pf_pattern_unpack_one(const ap_uint<64> &data,
                                    EmCaloObj &emcalo) {
#pragma HLS inline
  emcalo.hwPt = data(15 + 32, 0 + 32);
  emcalo.hwPtErr = data(31 + 32, 16 + 32);
  emcalo.hwEta = data(9, 0);
  emcalo.hwPhi = data(19, 10);
}

template <unsigned int N, unsigned int OFFS>
inline void l1pf_pattern_unpack(const ap_uint<64> data[], EmCaloObj emcalo[N]) {
#pragma HLS inline
  for (unsigned int i = 0; i < N; ++i) {
    l1pf_pattern_unpack_one(data[i + OFFS], emcalo[i]);
  }
}

inline void l1pf_pattern_unpack_one(const ap_uint<64> &data,
                                    HadCaloObj &hadcalo) {
#pragma HLS inline
  hadcalo.hwPt = data(15 + 32, 0 + 32);
  hadcalo.hwEmPt = data(31 + 32, 16 + 32);
  hadcalo.hwEta = data(9, 0);
  hadcalo.hwPhi = data(19, 10);
  hadcalo.hwIsEM = data[20];
}

template <unsigned int N, unsigned int OFFS>
inline void l1pf_pattern_unpack(const ap_uint<64> data[],
                                HadCaloObj hadcalo[N]) {
#pragma HLS inline
  for (unsigned int i = 0; i < N; ++i) {
    l1pf_pattern_unpack_one(data[i + OFFS], hadcalo[i]);
  }
}

inline void l1pf_pattern_unpack_one(const ap_uint<64> &data, TkObj &track) {
#pragma HLS inline
  track.hwPtErr = data(31 + 32, 16 + 32);
  track.hwPt = data(15 + 32, 0 + 32);
  track.hwEta = data(9, 0);
  track.hwPhi = data(19, 10);
  track.hwZ0 = data(29, 20);
  track.hwQuality = TkObj::PFTIGHT + (data[30] ? TkObj::PFTIGHT : 0);
  track.hwCharge = data[31];
}

template <unsigned int N, unsigned int OFFS>
inline void l1pf_pattern_unpack(const ap_uint<64> data[], TkObj track[N]) {
#pragma HLS inline
  for (unsigned int i = 0; i < N; ++i) {
    l1pf_pattern_unpack_one(data[i + OFFS], track[i]);
  }
}

inline void l1pf_pattern_unpack_one(const ap_uint<64> &data, MuObj &mu) {
#pragma HLS inline
  mu.hwPt = data(15 + 32, 0 + 32);
  //mu.hwPtErr = data(31 + 32, 16 + 32);
  mu.hwEta = data(9, 0);
  mu.hwPhi = data(19, 10);
}

template <unsigned int N, unsigned int OFFS>
inline void l1pf_pattern_unpack(const ap_uint<64> data[], MuObj mu[N]) {
#pragma HLS inline
  for (unsigned int i = 0; i < N; ++i) {
    l1pf_pattern_unpack_one(data[i + OFFS], mu[i]);
  }
}

inline void l1pf_pattern_unpack_one(const ap_uint<64> &data,
                                    PFChargedObj &pfch) {
#pragma HLS inline
  pfch.hwId.bits = data(18 + 32, 16 + 32);
  pfch.hwPt = data(15 + 32, 0 + 32);
  pfch.hwEta = data(9, 0);
  pfch.hwPhi = data(19, 10);
  pfch.hwZ0 = data(29, 20);
}

template <unsigned int N, unsigned int OFFS>
inline void l1pf_pattern_unpack(const ap_uint<64> data[],
                                PFChargedObj pfch[N]) {
#pragma HLS inline
  for (unsigned int i = 0; i < N; ++i) {
    l1pf_pattern_unpack_one(data[i + OFFS], pfch[i]);
  }
}

inline void l1pf_pattern_unpack_one(const ap_uint<64> &data,
                                    PFNeutralObj &pfne) {
#pragma HLS inline
  pfne.hwPt = data(15 + 32, 0 + 32);
  pfne.hwId.bits = data(22, 20);
  pfne.hwEta = data(9, 0);
  pfne.hwPhi = data(19, 10);
}

template <unsigned int N, unsigned int OFFS>
inline void l1pf_pattern_unpack(const ap_uint<64> data[],
                                PFNeutralObj pfne[N]) {
#pragma HLS inline
  for (unsigned int i = 0; i < N; ++i) {
    l1pf_pattern_unpack_one(data[i + OFFS], pfne[i]);
  }
}

inline void l1pf_pattern_unpack_one(const ap_uint<64> &data, PuppiObj &pupp) {
#pragma HLS inline
  pupp.hwId.bits = data(18 + 32, 16 + 32);
  pupp.hwPt = data(15 + 32, 0 + 32);
  pupp.hwEta = data(9, 0);
  pupp.hwPhi = data(19, 10);
  pupp.hwData = data(31, 20);
}

template <unsigned int N, unsigned int OFFS>
inline void l1pf_pattern_unpack(const ap_uint<64> data[], PuppiObj pupp[N]) {
#pragma HLS inline
  for (unsigned int i = 0; i < N; ++i) {
    l1pf_pattern_unpack_one(data[i + OFFS], pupp[i]);
  }
}

#endif
