#ifndef PUPPI_PuppiChecker_h
#define PUPPI_PuppiChecker_h
#include <cstdio>
#include <cmath>

class PuppiChecker {
    public:
        PuppiChecker() : 
            npt2_(0), nok_(0), n1bit_(0), nalmostok_(0), nbad_(0),
            sumDiff_(0), sumAbsDiff_(0) {}

        template<typename T>
        void checkIntVsFloat(const std::vector<T> & input, const std::vector<l1ct::PuppiObjEmu> & puppi, const std::vector<l1ct::PuppiObjEmu> & puppi_flt, bool verbose) ;

        template<unsigned int N>
        bool check(const l1ct::PuppiObj puppi[N], const std::vector<l1ct::PuppiObjEmu> & puppi_ref, const std::vector<l1ct::PuppiObjEmu> & puppi_flt) ;

        template<unsigned int N>
        bool checkChs(l1ct::z0_t pvZ0, const l1ct::PuppiObj puppi[N], const std::vector<l1ct::PuppiObjEmu> & puppi_ref);

        void printIntVsFloatReport() {
            int nmiss = n1bit_ + nalmostok_ + nbad_, nall = nok_ + nmiss;
            float fall = 1.0/std::max(nall,1), fmiss = 1.0/std::max(nmiss,1);
            printf("  - all        : %6d\n", nall);
            printf("  -   filled   : %6d  (%6.2f%% )   [ pT >= 2 GeV ]\n", npt2_,  npt2_ * 100.0 * fall);
            printf("  - exact match: %6d  (%6.2f%% )\n", nok_,   nok_ * 100.0 * fall);
            printf("  - mismatch   : %6d  (%6.2f%% )\n", nmiss, nmiss * 100.0 * fall);
            printf("  -   by 1*LSB : %6d  (%6.2f%% )   [ 1 unit, %.2f GeV ]\n", n1bit_, n1bit_ * 100.0 * fall, l1ct::Scales::INTPT_LSB);
            printf("  -      small : %6d  (%6.2f%% )   [ %.2f < delta(pt) <= 1 GeV + 1% ]\n", nalmostok_, nalmostok_ * 100.0 * fall, l1ct::Scales::INTPT_LSB);
            printf("  -      big   : %6d  (%6.2f%% )   [ delta(pt) > 1 GeV + 1% ]\n", (nbad_), (nbad_) * 100.0 * fall);
            printf("  - average pT  diff   %+8.4f  (on all)    %+8.4f  (on mismatch)\n", sumDiff_*fall, sumDiff_*fmiss);
            printf("  - average pT |diff|  % 8.4f  (on all)    % 8.4f  (on mismatch)\n", sumAbsDiff_*fall, sumAbsDiff_*fmiss);
        }

    private:
        int npt2_, nok_, n1bit_, nalmostok_, nbad_;
        float sumDiff_, sumAbsDiff_;
};

template<typename T>
void PuppiChecker::checkIntVsFloat(const std::vector<T> & input, const std::vector<l1ct::PuppiObjEmu> & puppi, const std::vector<l1ct::PuppiObjEmu> & puppi_flt, bool verbose) {
    assert(input.size() == puppi.size() && input.size() == puppi_flt.size());
    for (int i = 0, n = input.size(); i < n; ++i){
        if (input[i].hwPt > 0) {
            if (puppi_flt[i].floatPt() >= 2) npt2_++;

            l1ct::dpt_t hwPtDiff = l1ct::dpt_t(puppi_flt[i].hwPt) - l1ct::dpt_t(puppi[i].hwPt);
            float ptDiff = l1ct::Scales::floatPt(hwPtDiff);

            int warn = 0;
            if (hwPtDiff == 0) {
                nok_++; 
            } else if (std::abs(l1ct::Scales::intPt(hwPtDiff)) == 1) {
                n1bit_++;
            } else if (std::abs(ptDiff)< 1 + 0.01 * puppi_flt[i].floatPt()) {
                nalmostok_++;
                warn = 1;
            } else {
                nbad_++;
                warn = 2;
            }
            sumDiff_ += ptDiff;
            sumAbsDiff_ += std::abs(ptDiff);
            if (verbose)  printf("particle %02d pT %7.2f :  puppiPt_int %7.2f   puppiPt_flt %7.2f    diff %+7.2f %s\n",
                    i, input[i].floatPt(), 
                    puppi[i].floatPt(), puppi_flt[i].floatPt(), ptDiff,
                    warn ? (warn == 1 ? "small" : "LARGE") : ""); 
        }
    }
}


template<unsigned int N>
bool PuppiChecker::check(const l1ct::PuppiObj puppi[N], const std::vector<l1ct::PuppiObjEmu> & puppi_ref, const std::vector<l1ct::PuppiObjEmu> & puppi_flt) {
    if (puppi_ref.size() > N || puppi_flt.size() > N) { 
        printf("Size overflow in ref: hardware %d (static)  ref %d   flt %d\n", N, int(puppi_ref.size()), int(puppi_flt.size()));
        return false;
    }
    bool ret = true;
    l1ct::PuppiObjEmu zero; zero.clear();
    for (int i = 0; i < N; ++i){
        const l1ct::PuppiObjEmu & ref = ( i < puppi_ref.size() ? puppi_ref[i] : zero );
        if (!puppi_equals(ref, puppi[i], "Puppi", i)) {
            ret = false;
        }
    }
    if (!ret) {
        for (int i = 0; i < N; ++i){
            const l1ct::PuppiObjEmu & ref = ( i < puppi_ref.size() ? puppi_ref[i] : zero );
            const l1ct::PuppiObjEmu & flt = ( i < puppi_flt.size() ? puppi_flt[i] : zero );
            printf("particle %02d:  puppiPt_hw %7.2f eta %+5d phi %+5d    puppiPt_ref %7.2f eta %+5d phi %+5d   puppiPt_flt %7.2f eta %+5d phi %+5d\n", i,
                    puppi[i].floatPt(),     int(puppi[i].hwEta), int(puppi[i].hwPhi),
                    ref.floatPt(), int(ref.hwEta), int(ref.hwPhi), 
                    flt.floatPt(), int(flt.hwEta), int(flt.hwPhi));
        }
    }
    return ret;
}

template<unsigned int N>
bool PuppiChecker::checkChs(l1ct::z0_t pvZ0, const l1ct::PuppiObj puppi[N], const std::vector<l1ct::PuppiObjEmu> & puppi_ref) {
    if (puppi_ref.size() > N) { 
        printf("Size overflow in ref: hardware %d (static)  ref %d\n", N, int(puppi_ref.size()));
        return false;
    }
    bool ret = true;
    l1ct::PuppiObjEmu zero; zero.clear();
    for (int i = 0; i < N; ++i){
        const l1ct::PuppiObjEmu & ref = ( i < puppi_ref.size() ? puppi_ref[i] : zero );
        if (!puppi_equals(ref, puppi[i], "PFCHS", i)) {
            ret = false;
        }
    }
    if (!ret) {
        for (int i = 0; i < N; ++i){
        const l1ct::PuppiObjEmu & ref = ( i < puppi_ref.size() ? puppi_ref[i] : zero );
            printf("particle %02d:  puppiPt_hw %7.2f eta %+5d phi %+5d dz %+5d   puppiPt_ref %7.2f eta %+5d phi %+5d dz %+5d\n", i,
                    puppi[i].floatPt(),     int(puppi[i].hwEta), int(puppi[i].hwPhi), int(puppi[i].hwZ0() - pvZ0),
                    ref.floatPt(), int(ref.hwEta), int(ref.hwPhi), int(ref.hwZ0() - pvZ0)); 
        }
    }
    return ret;
}




#endif
