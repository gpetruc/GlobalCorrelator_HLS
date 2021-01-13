#include "test_utils.h"
#include <cstdio>

bool had_equals(const HadCaloObj &out_ref, const HadCaloObj &out, const char *what, int idx) {
    bool ret = (out_ref == out);
    if  (!ret) {
        printf("Mismatch at %s[%d] ref vs test, hwPt % 7d % 7d   hwEmPt % 7d % 7d   hwEta %+7d %+7d   hwPhi %+7d %+7d   hwId %1d %1d\n", what, idx,
                out_ref.intPt(), out.intPt(), out_ref.intEmPt(), out.intEmPt(), int(out_ref.hwEta), int(out.hwEta), int(out_ref.hwPhi), int(out.hwPhi), int(out_ref.hwIsEM), int(out.hwIsEM));
    }
    return ret;
}
bool em_equals(const EmCaloObj &out_ref, const EmCaloObj &out, const char *what, int idx) {
    bool ret = (out_ref == out);
    if  (!ret) {
        printf("Mismatch at %s[%d] ref vs test, hwPt % 7d % 7d   hwPtErr % 7d % 7d   hwEta %+7d %+7d   hwPhi %+7d %+7d\n", what, idx,
                out_ref.intPt(), out.intPt(), out_ref.intPtErr(), out.intPtErr(), int(out_ref.hwEta), int(out.hwEta), int(out_ref.hwPhi), int(out.hwPhi));
    }
    return ret;
}

bool track_equals(const TkObj &out_ref, const TkObj &out, const char *what, int idx) {
    bool ret = (out_ref == out);
    if  (!ret) {
        printf("Mismatch at %s[%d] ref vs test, hwPt % 7d % 7d   hwPtErr % 7d % 7d   hwEta %+7d %+7d   hwPhi %+7d %+7d   hwDEta %+7d %+7d  hwDPhi %+7d %+7d  hwZ0 %+7d %+7d  hwDxy %+7d %+7d    hwCharge %1d %1d   hwQuality %1d %1d\n", what, idx,
                out_ref.intPt(), out.intPt(), out_ref.intPtErr(), out.intPtErr(),
                int(out_ref.hwEta), int(out.hwEta), int(out_ref.hwPhi), int(out.hwPhi), 
                int(out_ref.hwDEta), int(out.hwDEta), int(out_ref.hwDPhi), int(out.hwDPhi), 
                int(out_ref.hwZ0), int(out.hwZ0), int(out_ref.hwDxy), int(out.hwDxy), int(out_ref.hwCharge), int(out.hwCharge), int(out_ref.hwQuality), int(out.hwQuality));
    }
    return ret;
}
bool mu_equals(const MuObj &out_ref, const MuObj &out, const char *what, int idx) {
    bool ret = (out_ref == out);
    if  (!ret) {
        printf("Mismatch at %s[%d] ref vs test, hwPt % 7d % 7d   hwEta %+7d %+7d   hwPhi %+7d %+7d   hwDEta %+7d %+7d  hwDPhi %+7d %+7d  hwZ0 %+7d %+7d  hwDxy %+7d %+7d    hwCharge %1d %1d   hwQuality %1d %1d\n", what, idx,
                out_ref.intPt(), out.intPt(),
                int(out_ref.hwEta), int(out.hwEta), int(out_ref.hwPhi), int(out.hwPhi), 
                int(out_ref.hwDEta), int(out.hwDEta), int(out_ref.hwDPhi), int(out.hwDPhi), 
                int(out_ref.hwZ0), int(out.hwZ0), int(out_ref.hwDxy), int(out.hwDxy), int(out_ref.hwCharge), int(out.hwCharge), int(out_ref.hwQuality), int(out.hwQuality));
    }
    return ret;
}

bool pf_equals(const PFChargedObj &out_ref, const PFChargedObj &out, const char *what, int idx) {
    bool ret = (out_ref == out);
    if  (!ret) {
        printf("Mismatch at %s[%d] ref vs test, hwPt % 7d % 7d   hwEta %+7d %+7d   hwPhi %+7d %+7d   hwId %1d %1d      hwDEta %+7d %+7d  hwDPhi %+7d %+7d  hwZ0 %+7d %+7d  hwDxy %+7d %+7d  hwTkQuality %1d %1d  \n", what, idx,
                out_ref.intPt(), out.intPt(),
                int(out_ref.hwEta), int(out.hwEta),
                int(out_ref.hwPhi), int(out.hwPhi),
                out_ref.hwId.rawId(), out.hwId.rawId(),
                int(out_ref.hwDEta), int(out.hwDEta), int(out_ref.hwDPhi), int(out.hwDPhi), 
                int(out_ref.hwZ0), int(out.hwZ0), int(out_ref.hwDxy), int(out.hwDxy), int(out_ref.hwTkQuality), int(out.hwTkQuality));
    }
    return ret;
}
bool pf_equals(const PFNeutralObj &out_ref, const PFNeutralObj &out, const char *what, int idx) {
    bool ret = (out_ref == out);
    if  (!ret) {
        printf("Mismatch at %s[%d] ref vs test, hwPt % 7d % 7d   hwEta %+7d %+7d   hwPhi %+7d %+7d   hwId %1d %1d     hwEmPt  % 7d % 7d      hwEmID %7d %7d  hwPUID  %7d %7d\n", what, idx,
                out_ref.intPt(), out.intPt(),
                int(out_ref.hwEta), int(out.hwEta),
                int(out_ref.hwPhi), int(out.hwPhi),
                out_ref.hwId.rawId(), out.hwId.rawId(),
                out_ref.intEmPt(), out.intEmPt(),
                int(out_ref.hwEmID), int(out.hwEmID), int(out_ref.hwPUID), int(out.hwPUID));
    }
    return ret;
}
bool puppi_equals(const PuppiObj &out_ref, const PuppiObj &out, const char *what, int idx) {
    bool ret = (out_ref == out);
    if  (!ret) {
        printf("Mismatch at %s[%d] ref vs test, hwPt % 7d % 7d   hwEta %+7d %+7d   hwPhi %+7d %+7d   hwId %1d %1d      hwData % 7d % 7d   \n", what, idx,
                out_ref.intPt(), out.intPt(),
                int(out_ref.hwEta), int(out.hwEta),
                int(out_ref.hwPhi), int(out.hwPhi),
                out_ref.hwId.rawId(), out.hwId.rawId(),
                int(out_ref.hwData), int(out.hwData));
    }
    return ret;
}

