#ifndef FIRMWARE_DATA_H
#define FIRMWARE_DATA_H

#include <ap_int.h>
#include <cassert>
#include <cmath>

typedef ap_ufixed<14,12,AP_TRN,AP_SAT> pt_t;
typedef ap_fixed<16,14,AP_TRN,AP_SAT> dpt_t;
typedef ap_ufixed<28,24,AP_TRN,AP_SAT> pt2_t;
typedef ap_int<10> eta_t;
typedef ap_int<10> phi_t;
typedef ap_int<6> tkdeta_t;
typedef ap_uint<7> tkdphi_t;
typedef ap_int<12> glbeta_t;
typedef ap_int<11> glbphi_t;
typedef ap_int<5> vtx_t;
typedef ap_int<10> z0_t;        // 40cm / 0.1
typedef ap_int<8> dxy_t;        // tbd
typedef ap_uint<3> tkquality_t; // tbd
typedef ap_uint<9> puppiWgt_t;  // 256 = 1.0
typedef ap_uint<14> tk2em_dr_t;
typedef ap_uint<14> tk2calo_dr_t;
typedef ap_uint<10> em2calo_dr_t;
typedef ap_uint<13> tk2calo_dq_t;

struct ParticleID {
  ap_uint<3> bits;
  enum PID {
    NONE = 0,
    HADZERO = 0,
    PHOTON = 1,
    HADMINUS = 2,
    HADPLUS = 3,
    ELEMINUS = 4,
    ELEPLUS = 5,
    MUMINUS = 6,
    MUPLUS = 7
  };
  enum PTYPE { HAD = 0, EM = 1, MU = 2 };

  ParticleID(PID val = NONE) : bits(val) {}
  ParticleID &operator=(PID val) {
    bits = val;
    return *this;
  }

  int rawId() const { return bits.to_int(); }
  bool isPhoton() const {
#ifndef __SYNTHESIS__
    assert(neutral());
#endif
    return bits[0];
  }
  bool isMuon() const { return bits[2] && bits[1]; }
  bool isElectron() const { return bits[2] && !bits[1]; }
  bool charge() const {
#ifndef __SYNTHESIS__
    assert(charged());
#endif
    return bits[0]; /* 1 if positive, 0 if negative */
  }

  bool chargeOrNull() const { // doesn't throw on null id
    return bits[0]; 
  }
  bool charged() const { return bits[1] || bits[2]; };
  bool neutral() const { return !charged(); }
  void clear() { bits = 0; }

  static ParticleID mkChHad(bool charge) {
    return ParticleID(charge ? HADPLUS : HADMINUS);
  }
  static ParticleID mkElectron(bool charge) {
    return ParticleID(charge ? ELEPLUS : ELEMINUS);
  }
  static ParticleID mkMuon(bool charge) {
    return ParticleID(charge ? MUPLUS : MUMINUS);
  }

  inline bool operator==(const ParticleID & other) const {
      return bits == other.bits;
  }

  inline int pdgId() const {
      switch(bits.to_int()) {
        case HADZERO: return 130;
        case PHOTON: return 22;
        case HADMINUS: return -211;
        case HADPLUS: return +211;
        case ELEMINUS: return +11;
        case ELEPLUS: return -11;
        case MUMINUS: return +13;
        case MUPLUS: return -13;
      }
      return 0;
  }

  inline int oldId() const {
      //{ PID_Charged=0, PID_Neutral=1, PID_Photon=2, PID_Electron=3, PID_Muon=4 };
      switch(bits.to_int()) {
        case HADZERO: return 1;
        case PHOTON: return 2;
        case HADMINUS: return 0;
        case HADPLUS: return 0;
        case ELEMINUS: return 3;
        case ELEPLUS: return 3;
        case MUMINUS: return 4;
        case MUPLUS: return 4;
      }
      return -1;
  }

};

namespace Scales {
    constexpr float INTPT_LSB = 0.25;
    constexpr float ETAPHI_LSB = M_PI/720;
    constexpr float Z0_LSB = 0.05;
    constexpr float DXY_LSB = 0.05;
    constexpr float PUPPIW_LSB = 1.0/256;
    inline float floatPt(pt_t pt) { return pt.to_float(); }
    inline float floatPt(dpt_t pt) { return pt.to_float(); }
    inline float floatPt(pt2_t pt2) { return pt2.to_float(); }
    inline int intPt(pt_t pt) { return (ap_ufixed<16,14>(pt) << 2).to_int(); }
    inline int intPt(dpt_t pt) { return (ap_fixed<18,16>(pt) << 2).to_int(); }
    inline float floatEta(eta_t eta) { return eta.to_float() * ETAPHI_LSB; }
    inline float floatPhi(phi_t phi) { return phi.to_float() * ETAPHI_LSB; }
    inline float floatEta(tkdeta_t eta) { return eta.to_float() * ETAPHI_LSB; }
    inline float floatPhi(tkdphi_t phi) { return phi.to_float() * ETAPHI_LSB; }
    inline float floatEta(glbeta_t eta) { return eta.to_float() * ETAPHI_LSB; }
    inline float floatPhi(glbphi_t phi) { return phi.to_float() * ETAPHI_LSB; }
    inline float floatZ0(z0_t z0) { return z0.to_float() * Z0_LSB; }
    inline float floatDxy(dxy_t dxy) { return dxy.to_float() * DXY_LSB; }
    inline float floatPuppiW(puppiWgt_t puppiw) { return puppiw.to_float() * PUPPIW_LSB; }

    inline pt_t makePt(int pt) { return ap_ufixed<16,14>(pt) >> 2; }
    inline dpt_t makeDPt(int dpt) { return ap_fixed<18,16>(dpt) >> 2; }
    inline pt_t makePtFromFloat(float pt) { return pt_t(pt); }
    inline dpt_t makeDPtFromFloat(float dpt) { return dpt_t(dpt); }

    inline ap_uint<pt_t::width> ptToInt(pt_t pt) {
        // note: this can be synthethized, e.g. when pT is used as intex in a LUT
        ap_uint<pt_t::width> ret = 0;
        ret(pt_t::width-1,0) = pt(pt_t::width-1,0);
        return ret;
    }

    inline ap_int<dpt_t::width> ptToInt(dpt_t pt) {
        // note: this can be synthethized, e.g. when pT is used as intex in a LUT
        ap_uint<dpt_t::width> ret = 0;
        ret(dpt_t::width-1,0) = pt(dpt_t::width-1,0);
        return ret;
    }

    inline eta_t makeEta(float eta) { return round(eta / ETAPHI_LSB); }
    inline glbeta_t makeGlbEta(float eta) { return round(eta / ETAPHI_LSB); }

};

// DEFINE MULTIPLICITIES
#if defined(REG_HGCal)
#define NTRACK 30
#define NCALO 20
#define NMU 4
#define NSELCALO 20
#define NALLNEUTRALS NSELCALO
// dummy
#define NEMCALO 1
#define NPHOTON NEMCALO
// not used but must be there because used in header files
#define NNEUTRALS 1
//--------------------------------
#elif defined(REG_HGCalNoTK)
#define NCALO 12
#define NNEUTRALS 8
#define NALLNEUTRALS NCALO
// dummy
#define NMU 1
#define NTRACK 1
#define NEMCALO 1
#define NPHOTON NEMCALO
#define NSELCALO 1
//--------------------------------
#elif defined(REG_HF)
#define NCALO 18
#define NNEUTRALS 10
#define NALLNEUTRALS NCALO
// dummy
#define NMU 1
#define NTRACK 1
#define NEMCALO 1
#define NPHOTON NEMCALO
#define NSELCALO 1
//--------------------------------
#else // BARREL
#ifndef REG_Barrel
#ifndef CMSSW_GIT_HASH
#warning                                                                       \
    "No region defined, assuming it's barrel (#define REG_Barrel to suppress this)"
#endif
#endif
#if defined(BOARD_MP7)
#warning "MP7 NOT SUPPORTED ANYMORE"
#define NTRACK 14
#define NCALO 10
#define NMU 2
#define NEMCALO 10
#define NPHOTON NEMCALO
#define NSELCALO 10
#define NALLNEUTRALS (NPHOTON + NSELCALO)
#define NNEUTRALS 15
#elif defined(BOARD_CTP7)
#error "NOT SUPPORTED ANYMORE"
#elif defined(BOARD_KU15P)
#define NTRACK 14
#define NCALO 110
#define NMU 2
#define NEMCALO 10
#define NPHOTON NEMCALO
#define NSELCALO 10
#define NALLNEUTRALS (NPHOTON + NSELCALO)
#define NNEUTRALS 15
#elif defined(BOARD_VCU118)
#define NTRACK 22
#define NCALO 15
#define NEMCALO 13
#define NMU 2
#define NPHOTON NEMCALO
#define NSELCALO 10
#define NALLNEUTRALS (NPHOTON + NSELCALO)
#define NNEUTRALS 25
#else
#define NTRACK 22
#define NCALO 15
#define NEMCALO 13
#define NMU 2
#define NPHOTON NEMCALO
#define NSELCALO 10
#define NALLNEUTRALS (NPHOTON + NSELCALO)
#define NNEUTRALS 25
#endif

#endif // region

#if defined(BOARD_MP7)
#define PACKING_DATA_SIZE 32
#define PACKING_NCHANN 72
#elif defined(BOARD_KU15P)
#define PACKING_DATA_SIZE 64
#define PACKING_NCHANN 42
#elif defined(BOARD_VCU118)
#define PACKING_DATA_SIZE 64
#define PACKING_NCHANN 120
#elif defined(BOARD_APD1)
#define PACKING_DATA_SIZE 64
#define PACKING_NCHANN 96
#endif

template <int N> struct ct_log2_ceil {
  enum { value = ct_log2_ceil<(N / 2) + (N % 2)>::value + 1 };
};
template <> struct ct_log2_ceil<2> {
  enum { value = 1 };
};
template <> struct ct_log2_ceil<1> {
  enum { value = 0 };
};

template <typename U, typename T> 
void _pack_into_bits(U & u, unsigned int & start, const T & data) {
    unsigned int w = T::width;
    u(start+w-1,start) = data(w-1,0);
    start += w;
}
template <typename U, typename T> 
void _unpack_from_bits(const U & u, unsigned int & start, T & data) {
    unsigned int w = T::width;
    data(w-1,0) = u(start+w-1,start);
    start += w;
}
template <typename U> 
void _pack_bool_into_bits(U & u, unsigned int & start, bool data) {
    u[start++] = data;
}
template <typename U> 
void _unpack_bool_from_bits(const U & u, unsigned int & start, bool & data) {
    data = u[start++];
}





struct HadCaloObj {
  pt_t hwPt;
  eta_t hwEta; // relative to the region center, at calo
  phi_t hwPhi; // relative to the region center, at calo
  pt_t hwEmPt;
  bool hwIsEM;

  inline bool operator==(const HadCaloObj & other) const {
    return hwPt == other.hwPt && 
      hwEta == other.hwEta && 
      hwPhi == other.hwPhi && 
      hwEmPt == other.hwEmPt && 
      hwIsEM == other.hwIsEM;
  }

  inline void clear() {
    hwPt = 0;
    hwEta = 0;
    hwPhi = 0;
    hwEmPt = 0;
    hwIsEM = false;
  }

  int intPt() const { return Scales::intPt(hwPt); }
  int intEmPt() const { return Scales::intPt(hwEmPt); }
  int intEta() const { return hwEta.to_int(); }
  int intPhi() const { return hwPhi.to_int(); }
  float floatPt() const { return Scales::floatPt(hwPt); }
  float floatEmPt() const { return Scales::floatPt(hwEmPt); }
  float floatEta() const { return Scales::floatEta(hwEta); }
  float floatPhi() const { return Scales::floatPhi(hwPhi); }


  static const int BITWIDTH = pt_t::width + eta_t::width + phi_t::width + pt_t::width + 1;
  inline ap_uint<BITWIDTH> pack() const {
        ap_uint<BITWIDTH> ret; unsigned int start = 0;
        _pack_into_bits(ret, start, hwPt);
        _pack_into_bits(ret, start, hwEta);
        _pack_into_bits(ret, start, hwPhi);
        _pack_into_bits(ret, start, hwEmPt);
        _pack_bool_into_bits(ret, start, hwIsEM);
        return ret;
  }
  inline static HadCaloObj unpack(const ap_uint<BITWIDTH> & src) {
        HadCaloObj ret; unsigned int start = 0;
        _unpack_from_bits(src, start, ret.hwPt);
        _unpack_from_bits(src, start, ret.hwEta);
        _unpack_from_bits(src, start, ret.hwPhi);
        _unpack_from_bits(src, start, ret.hwEmPt);
        _unpack_bool_from_bits(src, start, ret.hwIsEM);
        return ret;
  }

};

inline void clear(HadCaloObj &c) {
  c.clear();
}

struct EmCaloObj {
  pt_t hwPt, hwPtErr;
  eta_t hwEta; // relative to the region center, at calo
  phi_t hwPhi; // relative to the region center, at calo

  inline bool operator==(const EmCaloObj & other) const {
    return hwPt == other.hwPt && 
      hwEta == other.hwEta && 
      hwPhi == other.hwPhi && 
      hwPtErr == other.hwPtErr;
  }

  inline void clear() {
    hwPt = 0;
    hwPtErr = 0;
    hwEta = 0;
    hwPhi = 0;
  }

  int intPt() const { return Scales::intPt(hwPt); }
  int intPtErr() const { return Scales::intPt(hwPtErr); }
  int intEta() const { return hwEta.to_int(); }
  int intPhi() const { return hwPhi.to_int(); }
  float floatPt() const { return Scales::floatPt(hwPt); }
  float floatPtErr() const { return Scales::floatPt(hwPtErr); }
  float floatEta() const { return Scales::floatEta(hwEta); }
  float floatPhi() const { return Scales::floatPhi(hwPhi); }

  static const int BITWIDTH = pt_t::width + eta_t::width + phi_t::width + pt_t::width;
  inline ap_uint<BITWIDTH> pack() const {
        ap_uint<BITWIDTH> ret; unsigned int start = 0;
        _pack_into_bits(ret, start, hwPt);
        _pack_into_bits(ret, start, hwEta);
        _pack_into_bits(ret, start, hwPhi);
        _pack_into_bits(ret, start, hwPtErr);
        return ret;
  }
  inline static EmCaloObj unpack(const ap_uint<BITWIDTH> & src) {
        EmCaloObj ret; unsigned int start = 0;
        _unpack_from_bits(src, start, ret.hwPt);
        _unpack_from_bits(src, start, ret.hwEta);
        _unpack_from_bits(src, start, ret.hwPhi);
        _unpack_from_bits(src, start, ret.hwPtErr);
        return ret;
  }

};
inline void clear(EmCaloObj &c) {
  c.clear();
}

struct TkObj {
  pt_t hwPt;
  eta_t hwEta;   // relative to the region center, at calo
  phi_t hwPhi;   // relative to the region center, at calo
  tkdeta_t hwDEta; //  vtx - calo
  tkdphi_t hwDPhi; // |vtx - calo| (sign is derived by the charge)
  bool hwCharge; // 1 = positive, 0 = negative
  z0_t hwZ0;
  dxy_t hwDxy;
  tkquality_t hwQuality;
  
  enum TkQuality { PFLOOSE = 1, PFTIGHT = 2 };
  bool isPFLoose() const { return hwQuality[0]; }
  bool isPFTight() const { return hwQuality[1]; }
  phi_t hwVtxPhi() const { return hwCharge ? hwPhi + hwDPhi : hwPhi - hwDPhi; }
  eta_t hwVtxEta() const { return hwEta + hwDEta; }
  inline bool operator==(const TkObj & other) const {
    return hwPt == other.hwPt &&
      hwEta == other.hwEta &&
      hwPhi == other.hwPhi &&
      hwDEta == other.hwDEta &&
      hwDPhi == other.hwDPhi &&
      hwZ0 == other.hwZ0 &&
      hwDxy == other.hwDxy &&
      hwCharge == other.hwCharge &&
      hwQuality == other.hwQuality;
  }
  inline void clear() {
    hwPt = 0;
    hwEta = 0;
    hwPhi = 0;
    hwDEta = 0;
    hwDPhi = 0;
    hwZ0 = 0;
    hwDxy = 0;
    hwCharge = false;
    hwQuality = 0;
  }

  int intPt() const { return Scales::intPt(hwPt); }
  int intEta() const { return hwEta.to_int(); }
  int intPhi() const { return hwPhi.to_int(); }
  float floatPt() const { return Scales::floatPt(hwPt); }
  float floatEta() const { return Scales::floatEta(hwEta); }
  float floatPhi() const { return Scales::floatPhi(hwPhi); }
  float floatDEta() const { return Scales::floatEta(hwDEta); }
  float floatDPhi() const { return Scales::floatPhi(hwDPhi); }
  float floatVtxEta() const { return Scales::floatEta(hwVtxEta()); }
  float floatVtxPhi() const { return Scales::floatPhi(hwVtxPhi()); }
  float floatZ0() const { return Scales::floatZ0(hwZ0); }
  float floatDxy() const { return Scales::floatDxy(hwDxy); }

  static const int BITWIDTH = pt_t::width + eta_t::width + phi_t::width + tkdeta_t::width + tkdphi_t::width + 1 + z0_t::width + dxy_t::width + tkquality_t::width;
  inline ap_uint<BITWIDTH> pack() const {
        ap_uint<BITWIDTH> ret; unsigned int start = 0;
        _pack_into_bits(ret, start, hwPt);
        _pack_into_bits(ret, start, hwEta);
        _pack_into_bits(ret, start, hwPhi);
        _pack_into_bits(ret, start, hwDEta);
        _pack_into_bits(ret, start, hwDPhi);
        _pack_bool_into_bits(ret, start, hwCharge);
        _pack_into_bits(ret, start, hwZ0);
        _pack_into_bits(ret, start, hwDxy);
        _pack_into_bits(ret, start, hwQuality);
        return ret;
  }
  inline static TkObj unpack(const ap_uint<BITWIDTH> & src) {
        TkObj ret; unsigned int start = 0;
        _unpack_from_bits(src, start, ret.hwPt);
        _unpack_from_bits(src, start, ret.hwEta);
        _unpack_from_bits(src, start, ret.hwPhi);
        _unpack_from_bits(src, start, ret.hwDEta);
        _unpack_from_bits(src, start, ret.hwDPhi);
        _unpack_bool_from_bits(src, start, ret.hwCharge);
        _unpack_from_bits(src, start, ret.hwZ0);
        _unpack_from_bits(src, start, ret.hwDxy);
        _unpack_from_bits(src, start, ret.hwQuality);
        return ret;
  }

};
inline void clear(TkObj &c) {
  c.clear();
}

struct MuObj {
  pt_t hwPt;
  eta_t hwEta;   // relative to the region center, at calo
  phi_t hwPhi;   // relative to the region center, at calo
  tkdeta_t hwDEta; //  vtx - calo
  tkdphi_t hwDPhi; // |vtx - calo| (sign is derived by the charge)
  bool hwCharge; // 1 = positive, 0 = negative
  z0_t hwZ0;
  dxy_t hwDxy;
  ap_uint<3> hwQuality;
  phi_t hwVtxPhi() const { return hwCharge ? hwPhi + hwDPhi : hwPhi - hwDPhi; }
  eta_t hwVtxEta() const { return hwEta + hwDEta; }

  inline bool operator==(const MuObj &other) const {
    return hwPt == other.hwPt &&
      hwEta == other.hwEta &&
      hwPhi == other.hwPhi &&
      hwDEta == other.hwDEta &&
      hwDPhi == other.hwDPhi &&
      hwZ0 == other.hwZ0 &&
      hwDxy == other.hwDxy &&
      hwCharge == other.hwCharge &&
      hwQuality == other.hwQuality;
  }

  inline void clear() {
    hwPt = 0;
    hwEta = 0;
    hwPhi = 0;
    hwDEta = 0;
    hwDPhi = 0;
    hwZ0 = 0;
    hwDxy = 0;
    hwCharge = false;
    hwQuality = 0;
  }

  int intPt() const { return Scales::intPt(hwPt); }
  int intEta() const { return hwEta.to_int(); }
  int intPhi() const { return hwPhi.to_int(); }
  float floatPt() const { return Scales::floatPt(hwPt); }
  float floatEta() const { return Scales::floatEta(hwEta); }
  float floatPhi() const { return Scales::floatPhi(hwPhi); }
  float floatDEta() const { return Scales::floatEta(hwDEta); }
  float floatDPhi() const { return Scales::floatPhi(hwDPhi); }
  float floatVtxEta() const { return Scales::floatEta(hwVtxEta()); }
  float floatVtxPhi() const { return Scales::floatPhi(hwVtxPhi()); }
  float floatZ0() const { return Scales::floatZ0(hwZ0); }
  float floatDxy() const { return Scales::floatDxy(hwDxy); }

  static const int BITWIDTH = pt_t::width + eta_t::width + phi_t::width + tkdeta_t::width + tkdphi_t::width + 1 + z0_t::width + dxy_t::width + ap_uint<3>::width;
  inline ap_uint<BITWIDTH> pack() const {
        ap_uint<BITWIDTH> ret; unsigned int start = 0;
        _pack_into_bits(ret, start, hwPt);
        _pack_into_bits(ret, start, hwEta);
        _pack_into_bits(ret, start, hwPhi);
        _pack_into_bits(ret, start, hwDEta);
        _pack_into_bits(ret, start, hwDPhi);
        _pack_bool_into_bits(ret, start, hwCharge);
        _pack_into_bits(ret, start, hwZ0);
        _pack_into_bits(ret, start, hwDxy);
        _pack_into_bits(ret, start, hwQuality);
        return ret;
  }
  inline static MuObj unpack(const ap_uint<BITWIDTH> & src) {
        MuObj ret; unsigned int start = 0;
        _unpack_from_bits(src, start, ret.hwPt);
        _unpack_from_bits(src, start, ret.hwEta);
        _unpack_from_bits(src, start, ret.hwPhi);
        _unpack_from_bits(src, start, ret.hwDEta);
        _unpack_from_bits(src, start, ret.hwDPhi);
        _unpack_bool_from_bits(src, start, ret.hwCharge);
        _unpack_from_bits(src, start, ret.hwZ0);
        _unpack_from_bits(src, start, ret.hwDxy);
        _unpack_from_bits(src, start, ret.hwQuality);
        return ret;
  }


};
inline void clear(MuObj &c) {
  c.clear();
}

struct PFCommonObj {
  pt_t hwPt;
  eta_t hwEta; // relative to the region center, at calo
  phi_t hwPhi; // relative to the region center, at calo
  ParticleID hwId;

  int intPt() const { return Scales::intPt(hwPt); }
  int intEta() const { return hwEta.to_int(); }
  int intPhi() const { return hwPhi.to_int(); }
  float floatPt() const { return Scales::floatPt(hwPt); }
  float floatEta() const { return Scales::floatEta(hwEta); }
  float floatPhi() const { return Scales::floatPhi(hwPhi); }
  int intId() const { return hwId.rawId(); }
  int oldId() const { return hwPt > 0 ? hwId.oldId() : 0; }
  int pdgId() const { return hwId.pdgId(); }

  static const int _PFCOMMON_BITWIDTH = pt_t::width + eta_t::width + phi_t::width + 3;
  template<typename U>
  inline void _pack_common(U & out, unsigned int & start) const {
        _pack_into_bits(out, start, hwPt);
        _pack_into_bits(out, start, hwEta);
        _pack_into_bits(out, start, hwPhi);
        _pack_into_bits(out, start, hwId.bits);
  }
  template<typename U>
  inline void _unpack_common(const U & src, unsigned int & start) {
        _unpack_from_bits(src, start, hwPt);
        _unpack_from_bits(src, start, hwEta);
        _unpack_from_bits(src, start, hwPhi);
        _unpack_from_bits(src, start, hwId.bits);
  }

};

struct PFChargedObj : public PFCommonObj {
  tkdeta_t hwDEta; // relative to the region center, at calo
  tkdphi_t hwDPhi; // relative to the region center, at calo
  z0_t hwZ0;
  dxy_t hwDxy;
  tkquality_t hwTkQuality;

  phi_t hwVtxPhi() const {
    return hwId.chargeOrNull() ? hwPhi + hwDPhi : hwPhi - hwDPhi;
  }
  eta_t hwVtxEta() const { return hwEta + hwDEta; }

  inline bool operator==(const PFChargedObj & other) const {
    return hwPt == other.hwPt &&
      hwEta == other.hwEta &&
      hwPhi == other.hwPhi &&
      hwId == other.hwId &&
      hwDEta == other.hwDEta &&
      hwDPhi == other.hwDPhi &&
      hwZ0 == other.hwZ0 &&
      hwDxy == other.hwDxy &&
      hwTkQuality == other.hwTkQuality;
  }

  inline void clear() {
    hwPt = 0;
    hwEta = 0;
    hwPhi = 0;
    hwId.clear();
    hwDEta = 0;
    hwDPhi = 0;
    hwZ0 = 0;
    hwDxy = 0;
    hwTkQuality = 0;
  }

  float floatDEta() const { return Scales::floatEta(hwDEta); }
  float floatDPhi() const { return Scales::floatPhi(hwDPhi); }
  float floatVtxEta() const { return Scales::floatEta(hwVtxEta()); }
  float floatVtxPhi() const { return Scales::floatPhi(hwVtxPhi()); }
  float floatZ0() const { return Scales::floatZ0(hwZ0); }
  float floatDxy() const { return Scales::floatDxy(hwDxy); }

  static const int BITWIDTH = _PFCOMMON_BITWIDTH + tkdeta_t::width + tkdphi_t::width + z0_t::width + dxy_t::width + tkquality_t::width;
  inline ap_uint<BITWIDTH> pack() const {
        ap_uint<BITWIDTH> ret; unsigned int start = 0;
        _pack_common(ret, start);
        _pack_into_bits(ret, start, hwDEta);
        _pack_into_bits(ret, start, hwDPhi);
        _pack_into_bits(ret, start, hwZ0);
        _pack_into_bits(ret, start, hwDxy);
        _pack_into_bits(ret, start, hwTkQuality);
        return ret;
  }
  inline static PFChargedObj unpack(const ap_uint<BITWIDTH> & src) {
        PFChargedObj ret; unsigned int start = 0;
        ret._unpack_common(src, start);
        _unpack_from_bits(src, start, ret.hwDEta);
        _unpack_from_bits(src, start, ret.hwDPhi);
        _unpack_from_bits(src, start, ret.hwZ0);
        _unpack_from_bits(src, start, ret.hwDxy);
        _unpack_from_bits(src, start, ret.hwTkQuality);
        return ret;
  }

};
inline void clear(PFChargedObj &c) {
  c.clear();
 }

struct PFNeutralObj : public PFCommonObj {
  pt_t hwEmPt;
  ap_uint<6> hwEmID;
  ap_uint<6> hwPUID;

  inline bool operator==(const PFNeutralObj & other) const {
    return hwPt == other.hwPt &&
      hwEta == other.hwEta &&
      hwPhi == other.hwPhi &&
      hwId == other.hwId &&
      hwEmPt == other.hwEmPt &&
      hwEmID == other.hwEmID &&
      hwPUID == other.hwPUID;
  }
  
  inline void clear() {
    hwPt = 0;
    hwEta = 0;
    hwPhi = 0;
    hwId.clear();
    hwEmPt = 0;
    hwEmID = 0;
    hwPUID = 0;
  }


  int intEmPt() const { return Scales::intPt(hwEmPt); }
  float floatEmPt() const { return Scales::floatPt(hwEmPt); }

  static const int BITWIDTH = _PFCOMMON_BITWIDTH + pt_t::width + ap_uint<6>::width + ap_uint<6>::width;
  inline ap_uint<BITWIDTH> pack() const {
        ap_uint<BITWIDTH> ret; unsigned int start = 0;
        _pack_common(ret, start);
        _pack_into_bits(ret, start, hwEmPt);
        _pack_into_bits(ret, start, hwEmID);
        _pack_into_bits(ret, start, hwPUID);
        return ret;
  }
  inline static PFNeutralObj unpack(const ap_uint<BITWIDTH> & src) {
        PFNeutralObj ret; unsigned int start = 0;
        ret._unpack_common(src, start);
        _unpack_from_bits(src, start, ret.hwEmPt);
        _unpack_from_bits(src, start, ret.hwEmID);
        _unpack_from_bits(src, start, ret.hwPUID);
        return ret;
  }

};

inline void clear(PFNeutralObj &c) {
  c.clear();
}

struct PFRegion {
    glbeta_t hwEtaCenter;

    inline glbeta_t hwGlbEta(eta_t hwEta) const {
        return hwEtaCenter + hwEta;
    }

    inline float floatEtaCenter() const {
        return Scales::floatEta(hwEtaCenter);
    }
    inline float floatGlbEta(eta_t hwEta) const {
        return Scales::floatEta(hwGlbEta(hwEta));
    }

    static const int BITWIDTH = glbeta_t::width;
    inline ap_uint<BITWIDTH> pack() const {
        ap_uint<BITWIDTH> ret; unsigned int start = 0;
        _pack_into_bits(ret, start, hwEtaCenter);
        return ret;
    }
    inline static PFRegion unpack(const ap_uint<BITWIDTH> & src) {
        PFRegion ret; unsigned int start = 0;
        _unpack_from_bits(src, start, ret.hwEtaCenter);
        return ret;
    }


};

struct PuppiObj {
  pt_t hwPt;
  glbeta_t hwEta; // wider range to support global coordinates
  glbphi_t hwPhi;
  ParticleID hwId;

  static const int BITS_Z0_START = 0;
  static const int BITS_DXY_START = BITS_Z0_START + z0_t::width;
  static const int BITS_TKQUAL_START = BITS_DXY_START + dxy_t::width;
  static const int DATA_BITS_TOTAL = BITS_TKQUAL_START + tkquality_t::width;

  static const int BITS_PUPPIW_START = 0;

  ap_uint<DATA_BITS_TOTAL> hwData;

  inline z0_t hwZ0() const {
#ifndef __SYNTHESIS__
    assert(hwId.charged() || hwPt == 0);
#endif
    return z0_t(hwData(BITS_Z0_START + z0_t::width - 1, BITS_Z0_START));
  }

  inline void setHwZ0(z0_t z0) {
#ifndef __SYNTHESIS__
    assert(hwId.charged() || hwPt == 0);
#endif
    hwData(BITS_Z0_START + z0_t::width - 1, BITS_Z0_START) =
        z0(z0_t::width - 1, 0);
  }

  inline dxy_t hwDxy() const {
#ifndef __SYNTHESIS__
    assert(hwId.charged() || hwPt == 0);
#endif
    return dxy_t(hwData(BITS_DXY_START + dxy_t::width - 1, BITS_DXY_START));
  }

  inline void setHwDxy(dxy_t dxy) {
#ifndef __SYNTHESIS__
    assert(hwId.charged() || hwPt == 0);
#endif
    hwData(BITS_DXY_START + dxy_t::width - 1, BITS_DXY_START) = dxy(7, 0);
  }

  inline tkquality_t hwTkQuality() const {
#ifndef __SYNTHESIS__
    assert(hwId.charged() || hwPt == 0);
#endif
    return tkquality_t(
        hwData(BITS_TKQUAL_START + tkquality_t::width - 1, BITS_TKQUAL_START));
  }

  inline void setHwTkQuality(tkquality_t qual) {
#ifndef __SYNTHESIS__
    assert(hwId.charged() || hwPt == 0);
#endif
    hwData(BITS_TKQUAL_START + tkquality_t::width - 1, BITS_TKQUAL_START) =
        qual(tkquality_t::width - 1, 0);
  }

  inline puppiWgt_t hwPuppiW() const {
#ifndef __SYNTHESIS__
    assert(hwId.neutral());
#endif
    return puppiWgt_t(
        hwData(BITS_PUPPIW_START + puppiWgt_t::width - 1, BITS_PUPPIW_START));
  }

  inline void setHwPuppiW(puppiWgt_t w) {
#ifndef __SYNTHESIS__
    assert(hwId.neutral());
#endif
    hwData(BITS_PUPPIW_START + puppiWgt_t::width - 1, BITS_PUPPIW_START) =
        w(puppiWgt_t::width - 1, 0);
  }

  inline bool operator==(const PuppiObj &other) const {
    return hwPt == other.hwPt &&
      hwEta == other.hwEta &&
      hwPhi == other.hwPhi &&
      hwId == other.hwId &&
      hwData == other.hwData;
  }

  inline void clear() {
    hwPt = 0;
    hwEta = 0;
    hwPhi = 0;
    hwId.clear();
    hwData = 0;
  }


  inline void fill(const PFChargedObj &src) {
    hwEta = src.hwVtxEta(); 
    hwPhi = src.hwVtxPhi();
    hwId = src.hwId;
    hwPt = src.hwPt;
    hwData = 0;
    setHwZ0(src.hwZ0);
    setHwDxy(src.hwDxy);
    setHwTkQuality(src.hwTkQuality);
  }
  inline void fill(const PFNeutralObj &src, pt_t puppiPt,
                   puppiWgt_t puppiWgt) {
    hwEta = src.hwEta;
    hwPhi = src.hwPhi;
    hwId = src.hwId;
    hwPt = puppiPt;
    hwData = 0;
    setHwPuppiW(puppiWgt);
  }
  inline void fill(const HadCaloObj &src, pt_t puppiPt,
                   puppiWgt_t puppiWgt) {
    hwEta = src.hwEta;
    hwPhi = src.hwPhi;
    hwId = src.hwIsEM ? ParticleID::PHOTON : ParticleID::HADZERO;
    hwPt = puppiPt;
    hwData = 0;
    setHwPuppiW(puppiWgt);
  }


  int intPt() const { return Scales::intPt(hwPt); }
  int intEta() const { return hwEta.to_int(); }
  int intPhi() const { return hwPhi.to_int(); }
  float floatPt() const { return Scales::floatPt(hwPt); }
  float floatEta() const { return Scales::floatEta(hwEta); }
  float floatPhi() const { return Scales::floatPhi(hwPhi); }
  int intId() const { return hwId.rawId(); }
  int pdgId() const { return hwId.pdgId(); }
  int oldId() const { return hwPt > 0 ? hwId.oldId() : 0; }
  float floatZ0() const { return Scales::floatZ0(hwZ0()); }
  float floatDxy() const { return Scales::floatDxy(hwDxy()); }

  static const int BITWIDTH = pt_t::width + glbeta_t::width + glbphi_t::width + 3 + DATA_BITS_TOTAL;
  inline ap_uint<BITWIDTH> pack() const {
        ap_uint<BITWIDTH> ret; unsigned int start = 0;
        _pack_into_bits(ret, start, hwPt);
        _pack_into_bits(ret, start, hwEta);
        _pack_into_bits(ret, start, hwPhi);
        _pack_into_bits(ret, start, hwId.bits);
        _pack_into_bits(ret, start, hwData);
        return ret;
  }
  inline static PuppiObj unpack(const ap_uint<BITWIDTH> & src) {
        PuppiObj ret; unsigned int start = 0;
        _unpack_from_bits(src, start, ret.hwPt);
        _unpack_from_bits(src, start, ret.hwEta);
        _unpack_from_bits(src, start, ret.hwPhi);
        _unpack_from_bits(src, start, ret.hwId.bits);
        _unpack_from_bits(src, start, ret.hwData);
        return ret;
  }


};

inline void clear(PuppiObj &c) {
  c.clear();
}

// TMUX
#define NETA_TMUX 2
#define NPHI_TMUX 1
/* #define TMUX_IN 36 */
/* #define TMUX_OUT 18 */
#define TMUX_IN 18
#define TMUX_OUT 6
#define NTRACK_TMUX (NTRACK * TMUX_OUT * NETA_TMUX * NPHI_TMUX)
#define NCALO_TMUX (NCALO * TMUX_OUT * NETA_TMUX * NPHI_TMUX)
#define NEMCALO_TMUX (NEMCALO * TMUX_OUT * NETA_TMUX * NPHI_TMUX)
#define NMU_TMUX (NMU * TMUX_OUT * NETA_TMUX * NPHI_TMUX)

#endif
