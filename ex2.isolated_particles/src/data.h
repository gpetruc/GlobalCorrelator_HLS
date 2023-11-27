#ifndef ALGO_DATA_H
#define ALGO_DATA_H

#include "ap_int.h"
#include "ap_fixed.h"
#include <cstdint>
#include "math.h"

struct Puppi {
    // data types and constants
    typedef ap_ufixed<14,12,AP_RND,AP_SAT>  pt_t; 
    typedef ap_int<12> eta_t;
    typedef ap_int<11> phi_t;
    static constexpr int INT_PI = 720;
    static constexpr int INT_2PI = 2*INT_PI;
    static constexpr float ETAPHI_LSB = M_PI/INT_PI;
    enum PID { H0=0, Gamma=1, HMinus=2, HPlus=3, EMinus=4, EPlus=5, MuMinus=6, MuPlus=7 };
    // data members
    pt_t hwPt;
    eta_t hwEta; 
    phi_t hwPhi;
    ap_uint<3> hwID;
    // pack and unpack
    uint64_t pack() const {
        ap_uint<64> ret;
        ret(13,0) = hwPt(13,0);
        ret(25,14) = hwEta(11,0);
        ret(36,26) = hwPhi(10,0);
        ret(39,37) = hwID(2,0);
        return ret.to_uint64();
    }
    Puppi & unpack(uint64_t packed) { return unpack(ap_uint<64>(packed)); }
    Puppi & unpack(ap_uint<64> packed) {
       hwPt(13,0) = packed(13,0);
       hwEta(11,0) = packed(25,14);
       hwPhi(10,0) = packed(36,26);
       hwID(2,0) = packed(39,37);
       return *this;
    }
    void clear() {
        hwPt = 0;
        hwEta = 0;
        hwPhi = 0;
        hwID = 0;
    }
    // covenience functions for printout
    // methods
    float floatPt() const { return floatPt(hwPt); }
    float floatEta() const { return floatEta(hwEta); }
    float floatPhi() const { return floatPhi(hwPhi); }
    // helpers
    static pt_t toHwPt(float pt) { return pt_t(pt); }
    static float floatPt(pt_t hwPt) { return hwPt.to_float(); }
    static eta_t toHwEta(float eta) { return eta_t(round(eta/ETAPHI_LSB)); }
    static float floatEta(eta_t hwEta) { return hwEta.to_int() * ETAPHI_LSB; }
    static phi_t toHwPhi(float phi) { return phi_t(round(phi/ETAPHI_LSB));  }
    static float floatPhi(phi_t hwPhi) { return hwPhi.to_int() * ETAPHI_LSB; }    
};

#endif