#include "pattern_serializer.h"
#include <cassert>
#include <string>
#include <cstdlib>
#include <cassert>

#if defined(PACKING_DATA_SIZE)

PatternSerializer::PatternSerializer(const std::string &fname, unsigned int nchann, unsigned int nmux, unsigned int nzero, bool zero_valid, unsigned int nprefix, unsigned int npostfix, const std::string &boardName) :
    fname_(fname), nin_(nchann), nout_(nchann/nmux), nmux_(nmux), nzero_(nzero), nprefix_(nprefix), npostfix_(npostfix), zerovalid_(zero_valid), file_(nullptr), ipattern_(0) 
{
    if (!fname.empty()) {
        const unsigned int extra_space = (PACKING_DATA_SIZE-32)/4;
        char extra_spacer[extra_space+1];
        std::fill(extra_spacer, &extra_spacer[extra_space], ' ');
        extra_spacer[extra_space] = '\0';
        file_ = fopen(fname.c_str(), "w");
        fprintf(file_, "Board %s\n", boardName.c_str());
        fprintf(file_, " Quad/Chan :    ");
        for (unsigned int i = 0; i < nout_; ++i) fprintf(file_, "q%02dc%1d%s      ", i/4, i % 4, extra_spacer);
        fprintf(file_, "\n      Link :     ");
        for (unsigned int i = 0; i < nout_; ++i) fprintf(file_, "%02d%s         ", i, extra_spacer);
        fprintf(file_, "\n");
    }
    if (nmux_ > 1) {
        assert(nchann % nmux_ == 0);
    }

    if (nprefix_ || npostfix_ || nzero_) {
        zeroframe_.resize(nout_);
        for (unsigned int j = 0; j < nout_; ++j) zeroframe_[j] = 0;
    }
    for (unsigned int j = 0; j < nprefix_; ++j) {
      print(zeroframe_, false);
    }
}

PatternSerializer::~PatternSerializer() 
{
    for (unsigned int j = 0; j < npostfix_; ++j) {
      print(zeroframe_, false);
    }
    if (file_) {
        fclose(file_); file_ = nullptr;
        printf("Saved %u patterns to %s.\n", ipattern_, fname_.c_str());
    }
}

void PatternSerializer::operator()(const ap_uint<PACKING_DATA_SIZE> event[], bool valid) 
{
    if (!file_) return;
    for (unsigned int j = 0; j < nmux_; ++j) {
        print(event, valid, j, nmux_);
    }
    for (unsigned int j = 0; j < nzero_; ++j) {
      print(zeroframe_, zerovalid_);
    }
}

void PatternSerializer::operator()(const ap_uint<PACKING_DATA_SIZE> event[], const bool valid[]) 
{
    if (!file_) return;
    for (unsigned int j = 0; j < nmux_; ++j) {
        printv(event, valid, j, nmux_);
    }
    for (unsigned int j = 0; j < nzero_; ++j) {
      print(zeroframe_, zerovalid_);
    }
}

template<typename T> void PatternSerializer::print(const T & event, bool valid, unsigned int ifirst, unsigned int stride) 
{
    assert(PACKING_DATA_SIZE == 32 || PACKING_DATA_SIZE == 64);

    fprintf(file_, "Frame %04u :", ipattern_);
    for (unsigned int i = 0, j = ifirst; i < nout_; ++i, j += stride) {
#if PACKING_DATA_SIZE == 32
        fprintf(file_, " %dv%08x", int(valid), event[j].to_uint32());
#else 
        fprintf(file_, " %dv%016llx", int(valid), event[j].to_uint64());
#endif
    }
    fprintf(file_, "\n");
    ipattern_++;
}


template<typename T, typename TV> void PatternSerializer::printv(const T & event, const TV & valid, unsigned int ifirst, unsigned int stride) 
{
    assert(PACKING_DATA_SIZE == 32 || PACKING_DATA_SIZE == 64);

    fprintf(file_, "Frame %04u :", ipattern_);
    for (unsigned int i = 0, j = ifirst; i < nout_; ++i, j += stride) {
#if PACKING_DATA_SIZE == 32
        fprintf(file_, " %dv%08x", valid[j] ? 1 : 0, event[j].to_uint32());
#else 
        fprintf(file_, " %dv%016llx", valid[j] ? 1 : 0, event[j].to_uint64());
#endif
    }
    fprintf(file_, "\n");
    ipattern_++;
}


#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HumanReadablePatternSerializer::HumanReadablePatternSerializer(const std::string &fname, bool zerosuppress) :
    fname_(fname), zerosuppress_(zerosuppress), file_(nullptr), ipattern_(0) 
{
    if (!fname.empty()) {
        if (fname == "-") {
            file_ = stdout;
        } else {
            file_ = fopen(fname.c_str(), "w");
        }
    }
}

HumanReadablePatternSerializer::~HumanReadablePatternSerializer() 
{
    if (file_ && (file_ != stdout)) {
        fclose(file_); 
        printf("Saved %u human readable patterns to %s.\n", ipattern_, fname_.c_str());
    }
}

bool HumanReadablePatternSerializer::startframe() {
    if (!file_) return false;
    fprintf(file_, "Frame %04u:\n", ipattern_);
    return true;
}
void HumanReadablePatternSerializer::endframe() {
    fprintf(file_, "\n");
    if (file_ == stdout) fflush(file_);
    ipattern_++;
}
void HumanReadablePatternSerializer::operator()(const l1ct::EmCaloObj emcalo[NEMCALO], const l1ct::HadCaloObj hadcalo[NCALO], const l1ct::TkObj track[NTRACK], const l1ct::MuObj mu[NMU], const l1ct::PFChargedObj outch[NTRACK], const l1ct::PFNeutralObj outpho[NPHOTON], const l1ct::PFNeutralObj outne[NSELCALO], const l1ct::PFChargedObj outmu[NMU]) 
{
    if (!startframe()) return;
    dump_inputs(emcalo,hadcalo,track,mu);
    dump_outputs(outch,outpho,outne,outmu);
    endframe();
}

void HumanReadablePatternSerializer::operator()(const l1ct::HadCaloObj calo[NCALO], const l1ct::TkObj track[NTRACK], const l1ct::MuObj mu[NMU], const l1ct::PFChargedObj outch[NTRACK], const l1ct::PFNeutralObj outne[NSELCALO], const l1ct::PFChargedObj outmu[NMU]) 
{
    if (!startframe()) return;
    dump_inputs(calo,track,mu);
    dump_outputs(outch,outne,outmu);
    endframe();
}

void HumanReadablePatternSerializer::dump_inputs(const l1ct::EmCaloObj emcalo[NEMCALO], const l1ct::HadCaloObj hadcalo[NCALO], const l1ct::TkObj track[NTRACK], const l1ct::MuObj mu[NMU]) {
    dump_hadcalo(hadcalo);
    dump_emcalo(emcalo);
    dump_track(track);
    dump_mu(mu);
}
void HumanReadablePatternSerializer::dump_inputs(const l1ct::HadCaloObj calo[NCALO], const l1ct::TkObj track[NTRACK], const l1ct::MuObj mu[NMU]) {
    dump_hadcalo(calo);
    dump_track(track);
    dump_mu(mu);
}

void HumanReadablePatternSerializer::dump_outputs(const l1ct::PFChargedObj outch[NTRACK], const l1ct::PFNeutralObj outpho[NPHOTON], const l1ct::PFNeutralObj outne[NSELCALO], const l1ct::PFChargedObj outmu[NMU]) 
{
    dump_pf(NTRACK,   "charged pf", outch);
    dump_pf(NPHOTON,  "photon  pf", outpho);
    dump_pf(NSELCALO, "neutral pf", outne);
    dump_pf(NMU,      "muon    pf", outmu);
}

void HumanReadablePatternSerializer::dump_outputs(const l1ct::PFChargedObj outch[NTRACK], const l1ct::PFNeutralObj outne[NSELCALO], const l1ct::PFChargedObj outmu[NMU]) 
{
    dump_pf(NTRACK,   "charged pf", outch);
    dump_pf(NSELCALO, "neutral pf", outne);
    dump_pf(NMU,      "muon    pf", outmu);
}


void HumanReadablePatternSerializer::dump_hadcalo(const l1ct::HadCaloObj hadcalo[NCALO], unsigned int N) {
    for (int i = 0; i < N; ++i) {
        if (zerosuppress_ && !hadcalo[i].hwPt) continue;
        fprintf(file_, "   calo  %3d, hwPt % 7d   hwEmPt  % 7d    hwEta %+7d   hwPhi %+7d   hwIsEM %1d\n", i, hadcalo[i].intPt(), hadcalo[i].intEmPt(), hadcalo[i].intEta(), hadcalo[i].intPhi(), int(hadcalo[i].hwIsEM));
    }
    if (file_ == stdout) fflush(file_);
}
void HumanReadablePatternSerializer::dump_emcalo(const l1ct::EmCaloObj emcalo[NEMCALO], unsigned int N) {
    for (int i = 0; i < N; ++i) {
        if (zerosuppress_ && !emcalo[i].hwPt) continue;
        fprintf(file_, "   em    %3d, hwPt % 7d   hwPtErr % 7d    hwEta %+7d   hwPhi %+7d\n", i, emcalo[i].intPt(), emcalo[i].intPtErr(), emcalo[i].intEta(), emcalo[i].intPhi());
    }
    if (file_ == stdout) fflush(file_);
}
void HumanReadablePatternSerializer::dump_track(const l1ct::TkObj track[NTRACK], unsigned int N) {
    for (int i = 0; i < N; ++i) {
        if (zerosuppress_ && !track[i].hwPt) continue;
        fprintf(file_, "   track %3d, hwPt % 7d                      hwEta %+7d   hwPhi %+7d     hwZ0 %+7d   tight %1d\n", i, track[i].intPt(), track[i].intEta(), track[i].intPhi(), int(track[i].hwZ0), int(track[i].isPFTight()));
    }
    if (file_ == stdout) fflush(file_);
}
void HumanReadablePatternSerializer::dump_mu(const l1ct::MuObj mu[NMU], unsigned int N) {
    for (int i = 0; i < N; ++i) {
        if (zerosuppress_ && !mu[i].hwPt) continue;
        fprintf(file_, "   muon  %3d, hwPt % 7d                      hwEta %+7d   hwPhi %+7d\n", i, mu[i].intPt(), mu[i].intEta(), mu[i].intPhi());
    }
    if (file_ == stdout) fflush(file_);
}

void HumanReadablePatternSerializer::dump_pf(unsigned int N, const char *label, const l1ct::PFChargedObj outch[/*N*/]) 
{
    for (int i = 0; i < N; ++i) {
        if (zerosuppress_ && !outch[i].hwPt) continue;
        fprintf(file_, "   %s %3d, hwPt % 7d   hwEta %+7d   hwPhi %+7d   hwId %1d      hwZ0 %+7d\n", label, i, 
                outch[i].intPt(), outch[i].intEta(), outch[i].intPhi(), outch[i].oldId(), int(outch[i].hwZ0));
    }
}
void HumanReadablePatternSerializer::dump_pf(unsigned int N, const char *label, const l1ct::PFNeutralObj outne[/*N*/]) 
{
    for (int i = 0; i < N; ++i) {
        if (zerosuppress_ && !outne[i].hwPt) continue;
        fprintf(file_, "   %s %3d, hwPt % 7d   hwEta %+7d   hwPhi %+7d   hwId %1d\n", label, i,
                outne[i].intPt(), outne[i].intEta(), outne[i].intPhi(), outne[i].oldId());
    }
    if (file_ == stdout) fflush(file_);
}

template<typename TV> 
void HumanReadablePatternSerializer::dump_puppi_(unsigned int N, const char *label, const TV outpuppi) 
{
    for (int i = 0; i < N; ++i) {
        if (zerosuppress_ && !outpuppi[i].hwPt) continue; 
        if (outpuppi[i].hwId.charged()) {
            fprintf(file_, "   %s %3d, hwPt % 7d   hwEta %+7d   hwPhi %+7d   hwId %1d      hwZ0     %+7d\n", label, i, 
                    outpuppi[i].intPt(), outpuppi[i].intEta(), outpuppi[i].intPhi(), outpuppi[i].oldId(), int(outpuppi[i].hwZ0()));
        } else {
            fprintf(file_, "   %s %3d, hwPt % 7d   hwEta %+7d   hwPhi %+7d   hwId %1d      hwPuppiW % 7d\n", label, i, 
                    outpuppi[i].intPt(), outpuppi[i].intEta(), outpuppi[i].intPhi(), outpuppi[i].oldId(), int(outpuppi[i].hwPuppiW()));
        }
    }
}

void HumanReadablePatternSerializer::dump_puppi(unsigned int N, const char *label, const l1ct::PuppiObj outpuppi[/*N*/]) 
{
    dump_puppi_(N, label, outpuppi);
}

void HumanReadablePatternSerializer::dump_puppi(const char *label, const std::vector<l1ct::PuppiObjEmu> & outpuppi/*N*/) 
{
    dump_puppi_(outpuppi.size(), label, outpuppi);
}



