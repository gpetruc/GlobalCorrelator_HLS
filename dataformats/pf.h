#ifndef FIRMWARE_DATA_H
#define FIRMWARE_DATA_H

#include <ap_int.h>
#include <cassert>

typedef ap_int<16> pt_t; // FIXME to go to uint
typedef ap_int<16> dpt_t;
typedef ap_int<10> eta_t;
typedef ap_int<10> phi_t;
typedef ap_int<7> tkdeta_t;
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
#define NCALO 10
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

struct CaloObj {
  pt_t hwPt;
  eta_t hwEta; // relative to the region center, at calo
  phi_t hwPhi; // relative to the region center, at calo
};
struct HadCaloObj : public CaloObj {
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

};
inline void clear(EmCaloObj &c) {
  c.clear();
}

struct TkObj {
  pt_t hwPt;
  pt_t hwPtErr;
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
      hwPtErr == other.hwPtErr &&
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
    hwPtErr = 0;
    hwEta = 0;
    hwPhi = 0;
    hwDEta = 0;
    hwDPhi = 0;
    hwZ0 = 0;
    hwDxy = 0;
    hwCharge = false;
    hwQuality = false;
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

};
inline void clear(MuObj &c) {
  c.clear();
}

struct PFCommonObj {
  pt_t hwPt;
  eta_t hwEta; // relative to the region center, at calo
  phi_t hwPhi; // relative to the region center, at calo
  ParticleID hwId;
};

struct PFChargedObj : public PFCommonObj {
  eta_t hwDEta; // relative to the region center, at calo
  phi_t hwDPhi; // relative to the region center, at calo
  z0_t hwZ0;
  dxy_t hwDxy;
  tkquality_t hwTkQuality;

  phi_t hwVtxPhi() const {
    return hwId.charge() ? hwPhi + hwDPhi : hwPhi - hwDPhi;
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

};

inline void clear(PFNeutralObj &c) {
  c.clear();
}

struct PuppiObj {
  pt_t hwPt;
  glbeta_t hwEta; // wider range to support global coordinates
  glbphi_t hwPhi;
  ParticleID hwId;

  static const int BITS_Z0_START = 0;
  static const int BITS_DXY_START = BITS_Z0_START + z0_t::width;
  static const int BITS_TKQUAL_START = BITS_DXY_START + dxy_t::width;
  static const int BITS_TOTAL = BITS_TKQUAL_START + tkquality_t::width;

  static const int BITS_PUPPIW_START = 0;

  ap_uint<BITS_TOTAL> hwData;

  inline z0_t hwZ0() const {
#ifndef __SYNTHESIS__
    assert(hwId.charged());
#endif
    return z0_t(hwData(BITS_Z0_START + z0_t::width - 1, BITS_Z0_START));
  }

  inline void setHwZ0(z0_t z0) {
#ifndef __SYNTHESIS__
    assert(hwId.charged());
#endif
    hwData(BITS_Z0_START + z0_t::width - 1, BITS_Z0_START) =
        z0(z0_t::width - 1, 0);
  }

  inline dxy_t hwDxy() const {
#ifndef __SYNTHESIS__
    assert(hwId.charged());
#endif
    return dxy_t(hwData(BITS_DXY_START + dxy_t::width - 1, BITS_DXY_START));
  }

  inline void setHwDxy(dxy_t dxy) {
#ifndef __SYNTHESIS__
    assert(hwId.charged());
#endif
    hwData(BITS_DXY_START + dxy_t::width - 1, BITS_DXY_START) = dxy(7, 0);
  }

  inline tkquality_t hwTkQuality() const {
#ifndef __SYNTHESIS__
    assert(hwId.charged());
#endif
    return tkquality_t(
        hwData(BITS_TKQUAL_START + tkquality_t::width - 1, BITS_TKQUAL_START));
  }

  inline void setHwTkQuality(tkquality_t qual) {
#ifndef __SYNTHESIS__
    assert(hwId.charged());
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

};

inline void clear(PuppiObj &c) {
  c.clear();
}

inline void fill(PuppiObj &out, const PFChargedObj &src) {
  out.hwEta = src.hwVtxEta();
  out.hwPhi = src.hwVtxPhi();
  out.hwId = src.hwId;
  out.hwPt = src.hwPt;
  out.hwData = 0;
  out.setHwZ0(src.hwZ0);
  out.setHwDxy(src.hwDxy);
  out.setHwTkQuality(src.hwTkQuality);
}
inline void fill(PuppiObj &out, const PFNeutralObj &src, pt_t puppiPt,
                 puppiWgt_t puppiWgt) {
  out.hwEta = src.hwEta;
  out.hwPhi = src.hwPhi;
  out.hwId = src.hwId;
  out.hwPt = puppiPt;
  out.hwData = 0;
  out.setHwPuppiW(puppiWgt);
}
inline void fill(PuppiObj &out, const HadCaloObj &src, pt_t puppiPt,
                 puppiWgt_t puppiWgt) {
  out.hwEta = src.hwEta;
  out.hwPhi = src.hwPhi;
  out.hwId = src.hwIsEM ? ParticleID::PHOTON : ParticleID::HADZERO;
  out.hwPt = puppiPt;
  out.hwData = 0;
  out.setHwPuppiW(puppiWgt);
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
