#ifndef UTILS_PATTERNMULTIPLEXER_H
#define UTILS_PATTERNMULTIPLEXER_H

// relative path to what? 
#include "../utils/pattern_serializer.h"

#include <vector>
#include <queue>
#include <bitset>

typedef unsigned __int128 uint128_t;

#define MAXMUX 3

class PatternMultiplexer: public PatternSerializer {
/*
this is intended to use with NMUX=1 int Pattern Serializer. Do we need to extend?;
*/

    public:
        PatternMultiplexer(const std::string& fname);
        ~PatternMultiplexer(){flush();}

        typedef std::bitset<PACKING_NCHANN> mask_t; //uint128_t?
        // add a multiplexing offset:
        // all channels in maskin will be moved to maskout when () called with imux, 
        // column maskout will be added [time_offset] invalid null words
        //
        void add_tmux(int imux, mask_t maskin,mask_t maskto, int time_offset);

        void operator()(const Word event[PACKING_NCHANN], bool valid,int imux) ;

        // reset the queues and flush files
        void flush() ;
        bool printone();  // if true / printed something
        void print(){while(printone()){};};  // print all available

	// print queue status on screen
	// - short: print only the number of elements in each queue
        void debug_print(bool Short=false) const; 

    protected:
        // column valid/word
        std::queue<std::pair<bool,Word>>  _queue[PACKING_NCHANN]; 

            
        // mask container per imux -> [ (maskin, maskout), .... ]
        std::vector<std::pair<mask_t,mask_t>> _masks[MAXMUX];
};

#endif

#ifndef UTILS_PATTERNMULTIPLEXER_CC
#define UTILS_PATTERNMULTIPLEXER_CC

PatternMultiplexer::PatternMultiplexer( const std::string& fname) :
    PatternSerializer(fname)
{
}

void PatternMultiplexer::add_tmux(int imux,mask_t maskin,mask_t maskto, int time_offset){
    // check
    int nin=0, nto=0;
    for( unsigned ich =0 ; ich<PACKING_NCHANN ;++ich)
    {
        //if (maskto & (mask_t(1)<<ich) ) 
        if (maskto[ich]) {
            ++nto;
            // check that the channels to offset, are not already offset or unempty for any other reasons
            assert(_queue[ich].empty());
            for(int it=0;it<time_offset;++it)_queue[ich].push(std::pair<bool,Word>(false,0));
        }
        //if (maskin & (mask_t(1)<<ich) ) 
        if (maskin[ich]) {
            ++nin;
        }
    }
    // check that the channels to shift match in number
    assert(nin==nto);
    _masks[imux] . push_back(std::pair<mask_t,mask_t>(maskin,maskto));
    return;
}

// write an event in per time
void PatternMultiplexer::operator()(const Word event[PACKING_NCHANN], bool valid,int imux) {
    int shiftin=0;
    for (unsigned ich =0 ; ich<PACKING_NCHANN;++ich){
        //unsigned toch=ich;
        int toch=-1; // fill only the one with mask
        for( const auto & p : _masks[imux]){
            //if (p.first & (mask_t(1)<<ich))
            if (p.first[ich]){
                ++shiftin;
                // set toch to the non-zero shiftin bit
                int shiftto=0;
                for(unsigned ibit =0; ibit< PACKING_NCHANN ;++ibit)
                {
                    //if (p.second & (mask_t(1)<<ibit))
                    if (p.second[ibit]){
                        ++shiftto;
                        if (shiftto == shiftin) {
                            toch = ibit;
                            break;
                        }
                    }
                }
            } // found where to put
        }
        // if found match fill it, else fill the channel when calling imux=0;
        if (toch >=0 ) _queue[toch] . push( std::pair<bool,Word> (valid,event[ich]) );
        if (toch <0 and imux==0) _queue[ich] . push( std::pair<bool,Word> (valid,event[ich]) );
    } 
  return ;
}

// changing the one from PatternSerializer to print a valid status word dependent. makes sense?
bool PatternMultiplexer::printone(){
    assert(PACKING_DATA_SIZE == 32 || PACKING_DATA_SIZE == 64);
    // check that all the queues are not empty
    for (unsigned ich =0; ich<PACKING_NCHANN;ich++)
    {
        if (_queue[ich].empty()) {return false;}
    }
    
    fprintf(file_, "Frame %04u :", ipattern_);
    for (unsigned int ich = 0; ich < PACKING_NCHANN; ++ich) {
        assert(not _queue[ich].empty()); // undefined state
        std::pair<bool,Word> p = _queue[ich].front();
        _queue[ich].pop();
#if PACKING_DATA_SIZE == 32
        fprintf(file_, " %dv%08x", int(p.first), p.second.to_uint32());
#else 
        fprintf(file_, " %dv%016llx", int(p.first), p.second.to_uint64());
#endif
    }
    fprintf(file_, "\n");
    ipattern_++;
    return true;
}
void PatternMultiplexer::flush() {
    int max=0;
    for (unsigned int ich = 0; ich < PACKING_NCHANN; ++ich) {
        max=std::max(max, int(_queue[ich].size()));
    }
    for (unsigned int ich = 0; ich < PACKING_NCHANN; ++ich) {
            while(_queue[ich].size()<max)_queue[ich].push(std::pair<bool,Word>(false,0));
    }
    print();
    //make sure all queues are empty
    for (unsigned int ich = 0; ich < PACKING_NCHANN; ++ich) {
        assert(_queue[ich].empty());
    }
}

void PatternMultiplexer::debug_print(bool Short) const
{
    
    std::queue<std::pair<bool,Word>>  myqueue[PACKING_NCHANN]; 
    unsigned maxsize=0;
    std::cout<<" ---------------- PATTERM MULTIPLEXER STATUS --------------"<<std::endl;
    std::cout<<"N: ";
    for (unsigned int ich = 0; ich < PACKING_NCHANN; ++ich) {
        std::cout<<_queue[ich].size()<<" "; 
        maxsize = std::max( maxsize, unsigned(_queue[ich].size()));
        // make a copy
        myqueue[ich]=_queue[ich]; 
    }
    std::cout<<std::endl;
    if (Short) return;
    for( unsigned int iel =0 ; iel< maxsize ;++iel) {
        std::cout<<iel<<") ";
        for (unsigned int ich = 0; ich < PACKING_NCHANN; ++ich) {
            if (myqueue[ich].empty()) std::cout<<"**"<<std::setw(16)<<std::setfill('*')<<std::right<<"*"<<" ";
            else {
                std::pair<bool,Word> p = myqueue[ich].front();
                myqueue[ich].pop();
                std::cout<< int(p.first)<<"v"<<std::hex<<std::setw(16)<<std::right<<std::setfill('0')<<p.second.to_uint64()<<" "<<std::dec;
            }
        }
        std::cout<<std::endl;
    }
    //fprintf(file_, " %dv%016llx", int(p.first), p.second.to_uint64());
    
    std::cout<<" ----------------------------------------------------------"<<std::endl;
}


#endif
