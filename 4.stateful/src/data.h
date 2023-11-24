#ifndef ALGO_DATA_H
#define ALGO_DATA_H

#include "ap_int.h"
#include "ap_fixed.h"
#include "math.h"

struct TkMu {
    // define data types
    typedef ap_ufixed<16,11,AP_RND,AP_SAT>  pt_t; // in GeV, LSB = 31.25 MeV; max = 2 TeV
    typedef ap_fixed<13,1,AP_RND,AP_WRAP>  phi_t; // in units of pi, LSB = pi/2^12
    typedef ap_fixed<14,2,AP_RND,AP_WRAP>  eta_t; // in units of pi, LSB = pi/2^12
    typedef ap_int<10>  z0_t; // LSB = 0.05 cm
    typedef ap_int<10>  d0_t; // LSB = 0.03 cm
    // datamembers
    bool valid;
    pt_t hwPt;
    eta_t hwEta; 
    phi_t hwPhi;
    z0_t hwZ0;
    d0_t hwD0;
    bool hwCharge; // zero is positive
    ap_uint<8> hwQuality;
    ap_uint<4> hwIsolation;
    ap_uint<4> hwBeta;
    ap_uint<15> spare;
    // pack and unpack
    ap_uint<96> pack() const {
        ap_uint<96> ret;
        ret[0] = valid;
        ret(16,1) = hwPt(15,0);
        ret(29,17) = hwPhi(12,0);
        ret(43,30) = hwEta(13,0);
        ret(53,44) = hwZ0(9,0);
        ret(63,54) = hwD0(9,0);
        ret[64] = hwCharge;
        ret(72,65) = hwQuality(7,0);
        ret(76,73) = hwIsolation(3,0);
        ret(80,77) = hwBeta(3,0);
        ret(95,81) = spare(14,0);
        return ret;
    }
    TkMu & unpack(ap_uint<96> packed) {
        valid = packed[0];
        hwPt(15,0) = packed(16,1);
        hwPhi(12,0) = packed(29,17);
        hwEta(13,0) = packed(43,30);
        hwZ0(9,0) = packed(53,44);
        hwD0(9,0) = packed(63,54);
        hwCharge = packed[64];
        hwQuality(7,0) = packed(72,65);
        hwIsolation(3,0) = packed(76,73);
        hwBeta(3,0) = packed(80,77);
        spare(14,0) = packed(95,81);
        return *this;        
    }
    // covenience functions for printout
    float floatPt() const { return floatPt(hwPt); }
    float floatEta() const { return floatEta(hwEta); }
    float floatPhi() const { return floatPhi(hwPhi); }
    int intCharge() const { return hwCharge ? -1 : +1; }
    // helpers
    static pt_t toHwPt(float pt) { return pt_t(pt); }
    static float floatPt(pt_t hwPt) { return hwPt.to_float(); }
    static eta_t toHwEta(float eta) { return eta_t(eta/M_PI); }
    static float floatEta(eta_t hwEta) { return hwEta.to_float() * M_PI; }
    static phi_t toHwPhi(float phi) { return phi_t(phi/M_PI); }
    static float floatPhi(phi_t hwPhi) { return hwPhi.to_float() * M_PI; }

};

#endif
