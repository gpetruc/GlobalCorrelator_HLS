#ifndef utils_DumpFileReader_h
#define utils_DumpFileReader_h

#include <vector>
#include <cassert>
#include "../dataformats/layer1_emulator.h"
#include <fstream>

class DumpFileReader {
    public:
        DumpFileReader(const char *fileName) : file_(fileName,std::ios::in|std::ios::binary), event_(), iregion_(0) {
            if (!file_.is_open()) { std::cout << "ERROR: cannot read '" << fileName << "'" << std::endl; }
            assert(file_.is_open());
            std::cout << "INFO: opening "  << fileName << std::endl;
        }
        // for region-by-region approach
        bool nextRegion(l1ct::PFRegion & region, l1ct::HadCaloObj calo[NCALO], l1ct::EmCaloObj emcalo[NEMCALO], l1ct::TkObj track[NTRACK], l1ct::MuObj mu[NMU], l1ct::z0_t & hwZPV) {
            if (!nextRegion()) return false;
            const auto &r = event_.pfinputs[iregion_];
            region = r.region;

            l1ct::toFirmware(r.track, NTRACK, track);
            l1ct::toFirmware(r.hadcalo, NCALO, calo);
            l1ct::toFirmware(r.emcalo, NEMCALO, emcalo);
            l1ct::toFirmware(r.muon, NMU, mu);
            hwZPV = event_.pvs.front().hwZ0;

            //printf("Read region %u with %lu tracks, %lu em calo, %lu had calo, %lu muons\n", iregion_, r.track.size(), r.emcalo.size(), r.hadcalo.size(), r.muon.size());
            iregion_++;
            return true;
        }
        // for full event approach (don't mix with the above)
        bool nextEvent() {
            if (!file_) return false;
            if (!event_.read(file_)) return false;
            return true;
        }
        const l1ct::Event & event() const { return event_; }
        const l1ct::PFInputRegion & region() const { return event_.pfinputs[iregion_-1]; }

    private:
        bool nextRegion() {
            while(true) {
                if (event_.event == 0 || iregion_ == event_.pfinputs.size()) {
                    if (file_.eof()) { std::cout << "END OF FILE" << std::endl; return false; }
                    if (!event_.read(file_)) return false;
                    //printf("Beginning of run %u, lumi %u, event %lu \n", event_.run, event_.lumi, event_.event);
                    iregion_ = 0;
                }
               return true;
            }
        }

        std::fstream file_;
        l1ct::Event event_;
        unsigned int iregion_;
};
#endif
