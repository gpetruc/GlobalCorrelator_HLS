#ifndef UTILS_PATTERNSERIALIZER_H
#define UTILS_PATTERNSERIALIZER_H

#include <cstdio>
#include <vector>
#include "../dataformats/layer1_objs.h"
#include "../dataformats/layer1_multiplicities.h"
#include "../dataformats/pf.h"
#include "../dataformats/puppi.h"


#if defined(PACKING_DATA_SIZE)
class PatternSerializer {
    public:
        /** 
         *  each event corresponds to N of PACKING_DATA_SIZE bits
         *
         *  each event is serialized as <NMUX> frames, where the first <NMUX> words are send on the first link, etc.
         *  e.g. if event[i] = [ w0[i] w1[i] ... w(N-1)[i] ] then
         *  at NMUX = 1, the output uses N channels
         *      payload[t = 0] = [ w0[0] w1[0] ... w(N-1)[0] ]
         *      payload[t = 1] = [ w0[1] w1[1] ... w(N-1)[1] ]
         *  at NMUX = 2, the output uses N/2 channels
         *      payload[t = 0] = [ w0[0] w2[0] ... w(N-2)[0] ]
         *      payload[t = 1] = [ w1[0] w3[0] ... w(N-1)[0] ]
         *      payload[t = 2] = [ w0[1] w2[1] ... w(N-2)[1] ]                       
         *      payload[t = 3] = [ w1[1] w3[1] ... w(N-1)[1] ]
         *  at NMUX = 4, the output uses N/4 channels
         *      payload[t = 0] = [ w0[0] w4[0] ... w(N-4)[0] ]   
         *      payload[t = 1] = [ w1[0] w5[0] ... w(N-3)[0] ]
         *      payload[t = 2] = [ w2[0] w6[0] ... w(N-2)[0] ]   
         *      payload[t = 3] = [ w3[0] w7[0] ... w(N-1)[0] ]
         *      payload[t = 4] = [ w0[1] w4[1] ... w(N-4)[1] ]   
         *      payload[t = 5] = [ w1[1] w5[1] ... w(N-3)[1] ]
         *
         * In addition, it's possible to insert <NZERO> empty frames of zeros after the last frame of an event.
         * The "valid" bit in these patterns can be set to (either 0 or 1, depending on <ZERO_VALID>)
         * NMUX = 1, NZERO = 1
         *      payload[t = 0] = [ w0[0] w1[0] ... w(N-1)[0] ]
         *      payload[t = 1] = [  0     0    ...     0     ]
         *      payload[t = 2] = [ w0[1] w1[1] ... w(N-1)[1] ]
         *      payload[t = 3] = [  0     0    ...     0     ]
         * NMUX = 1, NZERO = 2
         *      payload[t = 0] = [ w0[0] w1[0] ... w(N-1)[0] ]
         *      payload[t = 1] = [  0     0    ...     0     ]
         *      payload[t = 2] = [  0     0    ...     0     ]
         *      payload[t = 3] = [ w0[1] w1[1] ... w(N-1)[1] ]
         *      payload[t = 4] = [  0     0    ...     0     ]
         *      payload[t = 5] = [  0     0    ...     0     ]
         * NMUX = 2, NZERO = 1
         *      payload[t = 0] = [ w0[0] w2[0] ... w(N-2)[0] ]
         *      payload[t = 1] = [ w1[0] w3[0] ... w(N-1)[0] ]
         *      payload[t = 2] = [  0     0    ...     0     ]
         *      payload[t = 3] = [ w0[1] w2[1] ... w(N-2)[1] ]                       
         *      payload[t = 4] = [ w1[1] w3[1] ... w(N-1)[1] ]
         *      payload[t = 5] = [  0     0    ...     0     ]
         *
         *
         * The serializer can also add <NPREFIX> frames of zero values with valid bit off at the beginning and <NPOSTFIX> frames and end of the file
         *
        */

        typedef ap_uint<PACKING_DATA_SIZE> Word;

        PatternSerializer(const std::string &fname, unsigned int nchann, unsigned int nmux=1, unsigned int nzero=0, bool zero_valid=true, unsigned int nprefix=0, unsigned int npostfix=0, const std::string &boardName = "Board L1PF") ;
        ~PatternSerializer() ;
        
        void operator()(const Word event[], bool valid=true) ;
        void operator()(const Word event[], const bool valid[]) ;

        template<int NB>
        void packAndWrite(unsigned int N, const ap_uint<NB> event[], bool valid=true) ;

        template<int NB>
        void packAndWrite(unsigned int N, const ap_uint<NB> event[], const bool valid[]) ;

    
        template<typename T> void print(const T & event, bool valid = true, unsigned int ifirst = 0, unsigned int stride = 1);
        template<typename T, typename TV> void printv(const T & event, const TV & valid, unsigned int ifirst = 0, unsigned int stride = 1);
        
    protected:
        const std::string fname_;
        const unsigned int nin_, nout_, nmux_, nzero_, nprefix_, npostfix_;
        const bool zerovalid_;
        FILE *file_;
        unsigned int ipattern_;
        std::vector<Word> zeroframe_;
};



template<int NB>
void PatternSerializer::packAndWrite(unsigned int N, const ap_uint<NB> event[], bool valid) {
    if (NB <= PACKING_DATA_SIZE) {
        if (N > nin_) { 
            printf("ERROR: %s : Would like to pack %u items of %d bits, and I have %u channels of %d bits\n", fname_.c_str(), N, NB, nin_, PACKING_DATA_SIZE);
            assert(false);
        }
        std::unique_ptr<Word[]> words(new Word[nin_]);

        for (unsigned int i = 0; i < nin_; ++i) {
            words[i] = (i < N ? Word(event[i]) : Word(0));
        }

        this->operator()(words.get(), valid);
    } else if (NB <= 2*PACKING_DATA_SIZE) {
        if (2*N > nin_) { 
            printf("ERROR: %s :Would like to pack %u items of %d bits, and I have %u channels of %d bits\n", fname_.c_str(), N, NB, nin_, PACKING_DATA_SIZE);
            assert(false);
        }
        std::unique_ptr<Word[]> words(new Word[nin_]);

        unsigned int rest = NB - PACKING_DATA_SIZE;
        for (unsigned int i = 0; i < N; ++i) {
            words[2*i+0](PACKING_DATA_SIZE-1, 0) = event[i](PACKING_DATA_SIZE-1, 0);
            words[2*i+1] = 0;
            words[2*i+1](rest-1, 0) = event[i](NB-1, PACKING_DATA_SIZE);
        }
        for (unsigned int i = 2*N; i < nin_; ++i) {
            words[i] = 0;
        }

        this->operator()(words.get(), valid);
    } else {
        assert(false);
    }
}


template<int NB>
void PatternSerializer::packAndWrite(unsigned int N, const ap_uint<NB> event[], const bool valid[]) {
    if (NB <= PACKING_DATA_SIZE) {
        if (N > nin_) { 
            printf("ERROR: %s : Would like to pack %u items of %d bits, and I have %u channels of %d bits\n", fname_.c_str(), N, NB, nin_, PACKING_DATA_SIZE);
            assert(false);
        }
        std::unique_ptr<Word[]> words(new Word[nin_]);
        std::unique_ptr<bool[]> bits(new bool[nin_]);

        for (unsigned int i = 0; i < nin_; ++i) {
            words[i] = (i < N ? event[i] : Word(0));
            bits[i]  = (i < N ? valid[i] : false  );
        }

        this->operator()(words.get(), bits.get());
    } else if (NB <= 2*PACKING_DATA_SIZE) {
        if (2*N > nin_) { 
            printf("ERROR: %s : Would like to pack %u items of %d bits, and I have %u channels of %d bits\n", fname_.c_str(), N, NB, nin_, PACKING_DATA_SIZE);
            assert(false);
        }
        std::unique_ptr<Word[]> words(new Word[nin_]);
        std::unique_ptr<bool[]> bits(new bool[nin_]);

        unsigned int rest = NB - PACKING_DATA_SIZE;
        for (unsigned int i = 0; i < N; ++i) {
            words[2*i+0](PACKING_DATA_SIZE-1, 0) = event[i](PACKING_DATA_SIZE-1, 0);
            words[2*i+1] = 0;
            words[2*i+1](rest-1, 0) = event[i](NB-1, PACKING_DATA_SIZE);
            bits[2*i+0] = valid[i];
            bits[2*i+1] = valid[i];
        }
        for (unsigned int i = 2*N; i < nin_; ++i) {
            words[i] = 0; bits[i] = false;
        }

        this->operator()(words.get(), bits.get());
    } else {
        assert(false);
    }
}

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class HumanReadablePatternSerializer {
    public:
        HumanReadablePatternSerializer(const std::string &fname, bool zerosuppress=false) ;
        ~HumanReadablePatternSerializer() ;
        
        void operator()(const l1ct::EmCaloObj emcalo[NEMCALO], const l1ct::HadCaloObj hadcalo[NCALO], const l1ct::TkObj track[NTRACK], const l1ct::MuObj mu[NMU], const l1ct::PFChargedObj outch[NTRACK], const l1ct::PFNeutralObj outpho[NPHOTON], const l1ct::PFNeutralObj outne[NSELCALO], const l1ct::PFChargedObj outmu[NMU]) ;
        void operator()(const l1ct::HadCaloObj calo[NCALO], const l1ct::TkObj track[NTRACK], const l1ct::MuObj mu[NMU], const l1ct::PFChargedObj outch[NTRACK], const l1ct::PFNeutralObj outne[NSELCALO], const l1ct::PFChargedObj outmu[NMU]) ;
        // PF all-in-one
        void dump_inputs(const l1ct::EmCaloObj emcalo[NEMCALO], const l1ct::HadCaloObj hadcalo[NCALO], const l1ct::TkObj track[NTRACK], const l1ct::MuObj mu[NMU]) ;
        void dump_inputs(const l1ct::HadCaloObj calo[NCALO], const l1ct::TkObj track[NTRACK], const l1ct::MuObj mu[NMU]) ;
        void dump_outputs(const l1ct::PFChargedObj outch[NTRACK], const l1ct::PFNeutralObj outpho[NPHOTON], const l1ct::PFNeutralObj outne[NSELCALO], const l1ct::PFChargedObj outmu[NMU]) ;
        void dump_outputs(const l1ct::PFChargedObj outch[NTRACK], const l1ct::PFNeutralObj outne[NSELCALO], const l1ct::PFChargedObj outmu[NMU]) ;
        // Individual pieces
        void dump_emcalo(const l1ct::EmCaloObj emcalo[NEMCALO], unsigned int N = NEMCALO) ;
        void dump_hadcalo(const l1ct::HadCaloObj hadcalo[NCALO], unsigned int N = NCALO) ;
        void dump_track(const l1ct::TkObj track[NTRACK], unsigned int N = NTRACK) ;
        void dump_mu(const l1ct::MuObj mu[NMU], unsigned int N = NMU) ;
        void dump_pf(unsigned int N, const char *label, const l1ct::PFChargedObj outch[/*N*/]) ;
        void dump_pf(unsigned int N, const char *label, const l1ct::PFNeutralObj outch[/*N*/]) ;
        void dump_puppi(unsigned int N, const char *label, const l1ct::PuppiObj outpuppi[/*N*/]) ;
        bool startframe();
        void endframe();
    protected:
        const std::string fname_;
        bool zerosuppress_;
        FILE *file_; // may be stdout
        unsigned int ipattern_;
    
};

#endif

