#ifndef multififo_regionizer_tmux18_utils_h
#define multififo_regionizer_tmux18_utils_h

#include <cstdlib>
#include <cstdio>
#include <vector>
#include <queue>
#include <cassert>
#include <algorithm>

#include <ap_int.h>


template<typename T, unsigned int TM6CLOCKS>
struct TM18LinkTriplet {
    std::queue<std::pair<T,bool>> links[3];
    
    TM18LinkTriplet() { 
        init();
    }

    void init() {
        for (auto & l : links) { while (!l.empty()) l.pop(); }
        for (int i = 0; i <  TM6CLOCKS; ++i)  links[1].emplace(T(0), false);
        for (int i = 0; i < 2*TM6CLOCKS; ++i) links[2].emplace(T(0), false);
    }

    template<typename C>
    void push_event(unsigned int iev, const C & objs) {
        auto & q = links[iev % 3];
        unsigned int n = std::min<unsigned int>(3*TM6CLOCKS-3, objs.size()); // let's leave 3 empty frames at the end, so they get reassebled as 1 row of nulls
        for (unsigned int i = 0; i < n; ++i) {
            q.emplace(objs[i], true);
        }

        for (unsigned int i = n; i < 3*TM6CLOCKS; ++i) {
            q.emplace(T(0), i < 3*TM6CLOCKS-3);
        }
    }

    void push_pause(unsigned int nclocks, bool valid=false) {
        for (auto & q : links) {
            for (unsigned int i = 0; i < nclocks; ++i) {
                q.emplace(T(0), false);
            }
        }
    }

    template<typename C>
    void pop_frame(C & channels, unsigned int start=0, unsigned int stride=1, bool maybenull=false) {
        for (int i = 0; i < 3; ++i) {
            if (maybenull && links[i].empty()) {
                channels.data [start+i*stride] = 0;
                channels.valid[start+i*stride] = false;
                continue;
            }
            if (links[i].empty()) { 
                printf("ERROR: link %d is empty (start = %u, stride = %u)\n", i, start, stride); fflush(stdout); 
                continue;
            }
            assert(!links[i].empty());
            auto obj = links[i].front();
            channels.data [start+i*stride] = obj.first;
            channels.valid[start+i*stride] = obj.second;
            links[i].pop();
        }
    }

    bool anyEmpty() {
        for (auto & q : links) { 
            if (q.empty()) return true; 
        }
        return false;
    }

};

template<typename T, unsigned int TM6CLOCKS>
class TM18LinkMultiplet {

    public:
        TM18LinkMultiplet(unsigned int N) :
            nlinks_(N), links_(N) {}

        void init() {
            for (auto & l : links_) l.init();
        }

        template<typename C, typename E>
        void push_links(unsigned int iev, std::vector<C> objs, const E & enc) {
            unsigned int nsec = objs.size();
            assert((nsec >= 1) && (nlinks_ >= nsec) && (nlinks_ % nsec == 0));
            unsigned int links_per_sec = nlinks_ / nsec;
            if (links_per_sec == 1) {
                for (unsigned int i = 0; i < nlinks_; ++i) {
                    links_[i].push_event(iev, enc(objs[i]));
                }
            } else {
                std::vector<C> redist(links_per_sec);
                for (unsigned int i = 0; i < nsec; ++i) {
                    //std::cout << "Pushing ev " << iev << " sec " << i << "/" << nsec << " for " << objs[i].size() << " objects." << std::endl;
                    for (unsigned int j = 0, nj = objs[i].size(); j < nj; ++j) {
                        redist[j % links_per_sec].obj.push_back(objs[i][j]);
                    }
                    for (unsigned int j = 0; j < links_per_sec; ++j) {
                        //if (nsec == 3) std::cout << "Pushing ev " << iev << " sec " << i << "/" << nsec << "  link " << j << "/" << links_per_sec <<
                        //                 " on global link triplet " << (i*links_per_sec+j) << " or " << redist[j].size() << " objects." << std::endl;
                        links_[i*links_per_sec+j].push_event(iev, enc(redist[j]));
                    }
                    for (auto & r : redist) r.clear();
                }
            }
        }

        template<typename C, typename E>
        void push_link(unsigned int iev, const C objs, const E & enc) {
            assert(nlinks_ == 1);
            links_.front().push_event(iev, enc(objs));
        }

        void push_pause(unsigned int nclocks, bool valid=false) {
            for (auto & l : links_) {
                l.push_pause(nclocks, valid);
            }
        }


        template<typename C>
        void pop_frame(C & channels, unsigned int start=0, bool group_by_link=true, bool maybenull=false) {
            unsigned int stride = group_by_link ? 1 : nlinks_;
            for (unsigned int i = 0; i < nlinks_; ++i) {
                links_[i].pop_frame(channels, start + (group_by_link ? 3*i : i), stride, maybenull);
            }
        }

        bool anyEmpty() {
            for (auto & l : links_) if (l.anyEmpty()) return true;
            return false;
        }
    private: 
        unsigned int nlinks_;
        std::vector<TM18LinkTriplet<T,TM6CLOCKS>> links_;
};

class DelayQueue {
    public:
        DelayQueue(unsigned int n) : n_(n), data_(n, 0), ptr_(0) {}
        ap_uint<65> operator()(const ap_uint<65> & in) ;
        void operator()(const ap_uint<64> & in,  const bool & in_valid,
                              ap_uint<64> & out,       bool & out_valid) ;
    private:
        unsigned int n_, ptr_;
        std::vector<ap_uint<65>> data_;
};



#endif
