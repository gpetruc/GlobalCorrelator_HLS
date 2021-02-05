#ifndef utils_DumpFileReader_h
#define utils_DumpFileReader_h

#include <vector>
#include <cassert>
#include "../dataformats/layer1_emulator.h"
#include <fstream>

class DumpFileReader {
    public:
        DumpFileReader(const char *fileName) : file_(fileName,std::ios::in|std::ios::binary), event_(), ipfregion_(0) {
            if (!file_.is_open()) { std::cout << "ERROR: cannot read '" << fileName << "'" << std::endl; }
            assert(file_.is_open());
            std::cout << "INFO: opening "  << fileName << std::endl;
        }
        // for region-by-region approach
        bool nextPFRegion() {
            ipfregion_++;
            while(true) {
                if (event_.event == 0 || ipfregion_ == event_.pfinputs.size()) {
                    if (file_.eof()) { std::cout << "END OF FILE" << std::endl; return false; }
                    if (!event_.read(file_)) return false;
                    //printf("Beginning of run %u, lumi %u, event %lu \n", event_.run, event_.lumi, event_.event);
                    ipfregion_ = 0;
                }
               return true;
            }
        }
        // for full event approach (don't mix with the above)
        bool nextEvent() {
            if (!file_) return false;
            if (!event_.read(file_)) return false;
            return true;
        }
        const l1ct::Event & event() const { return event_; }
        const l1ct::PFInputRegion & pfregion() const { return event_.pfinputs[ipfregion_]; }

    private:

        std::fstream file_;
        l1ct::Event event_;
        unsigned int ipfregion_;
};
#endif
