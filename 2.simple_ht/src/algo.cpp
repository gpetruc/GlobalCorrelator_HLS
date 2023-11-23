#include "algo.h"
#include <cmath>

// == version with a loop ==
pt_t algo_main(Particle particles[NPARTICLES]) {
    #pragma HLS ARRAY_PARTITION variable=particles complete
    #pragma HLS pipeline II=1

    pt_t sum = 0;
    for (unsigned int i = 0; i < NPARTICLES; ++i) {
        #pragma HLS unroll
        //====  Naive version, with an if guarding the sum
        if (-240 <= particles[i].hwEta && particles[i].hwEta <= 240) {
            sum += particles[i].hwPt;
        }
        //====  Version where the sum is always computed, but we add zero in some entries
        //bool central = (-240 <= particles[i].hwEta && particles[i].hwEta <= 240);
        //sum += (central ? particles[i].hwPt : pt_t(0));
    }
    return sum;
}

// == version with recursive templates ==

// recursive template to process N items:
// process each half and combine
template<unsigned int N>
pt_t partial_ht(const Particle particles[N]) {
   return partial_ht<N/2>(particles) +
          partial_ht<N-N/2>(&particles[N/2]);

}

// what to do when we get to a single item
template<>
pt_t partial_ht<1>(const Particle particles[1]) {
    if (-240 <= particles[0].hwEta && particles[0].hwEta <= 240) {
        return particles[0].hwPt;
    } else {
        return pt_t(0);
    }
}

pt_t algo_main_recurisve(Particle particles[NPARTICLES]) {
    #pragma HLS ARRAY_PARTITION variable=particles complete
    #pragma HLS pipeline II=1
    return partial_ht<NPARTICLES>(particles);
}
