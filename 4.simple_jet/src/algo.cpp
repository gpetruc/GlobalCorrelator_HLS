#include "algo.h"
#include <cmath>
#include <cassert>

#ifndef __SYNTHESIS__
#include <cstdio>
#endif

void _invert_lut_init(ap_uint<18> table[1024]) {
    for (int i = 0; i < 1024; ++i) {
        int val = i > 16 ? (1 << 22)/i : 0; 
        assert(val >= 0 && val < (1 << 18));
        table[i] = val;
    }
}
void reconstruct_jet(etaphi_t seed_eta, etaphi_t seed_phi, 
                     ap_uint<15> sum_pt, ap_int<22> sum_pt_eta, ap_int<22> sum_pt_phi, ap_uint<5> count, Jet & jet) {
    ap_uint<18> _inv_table[1024]; _invert_lut_init(_inv_table);

    ap_uint<10> den; ap_int<17> num_eta, num_phi;
    if (sum_pt[14] || sum_pt[13] || sum_pt[12]) { // sum_pt > 4*1024
        den     = sum_pt >> 5;
        num_eta = sum_pt_eta >> 5;
        num_phi = sum_pt_phi >> 5;
    } else if (sum_pt[11] || sum_pt[10]) {
        den     = sum_pt >> 2;
        num_eta = sum_pt_eta >> 2;
        num_phi = sum_pt_phi >> 2;
    } else {
        den     = sum_pt;
        num_eta = sum_pt_eta;
        num_phi = sum_pt_phi;
    }
    assert((sum_pt < JET_PT_CUT) || den > 16);
    ap_uint<18> inv_den = _inv_table[den];
    etaphi_t jet_eta = seed_eta + etaphi_t((num_eta * inv_den) >> 22);
    etaphi_t jet_phi = seed_phi + etaphi_t((num_phi * inv_den) >> 22);
    bool fiducial = (sum_pt >= JET_PT_CUT) &&
        (-FIDUCIAL_ETA_PHI <= jet_eta && jet_eta <= FIDUCIAL_ETA_PHI) &&
        (-FIDUCIAL_ETA_PHI <= jet_phi && jet_phi <= FIDUCIAL_ETA_PHI);
    if (fiducial) {
#ifndef __SYNTHESIS__
        //printf("HW algo         found jet pt %.2f, eta %+5.3f, phi %+5.3f\n", sum_pt * 0.25, jet_eta*0.01, jet_phi*0.01);
#endif
        jet.hwPt  = sum_pt;
        jet.hwEta = jet_eta;
        jet.hwPhi = jet_phi;
        jet.nCand = count;
        jet.iSeed = 0;
    } else {
        clear(jet);
    }

}

template<typename T, int NIn, int NOut>
void sort_and_crop(const T in[NIn], T out[NOut]) {
    T tmp[NOut];

    for (int iout = 0; iout < NOut; ++iout) {
        clear(tmp[iout]);
    }

    for (int it = 0; it < NIn; ++it) {
        for (int iout = NOut-1; iout >= 0; --iout) {
            if (tmp[iout].hwPt <= in[it].hwPt) {
                if (iout == 0 || tmp[iout-1].hwPt > in[it].hwPt) {
                    tmp[iout] = in[it];
                } else {
                    tmp[iout] = tmp[iout-1];
                }
            }
        }
    }

    for (int iout = 0; iout < NOut; ++iout) {
        out[iout] = tmp[iout];
    }
}

Particle Max(Particle a, Particle b){
    return a.hwPt >= b.hwPt ? a : b;
}

template<int width>
Particle MaxReduce(const Particle x[width]){
    // Tree reduce from https://github.com/definelicht/hlslib/blob/master/include/hlslib/xilinx/TreeReduce.h
    #pragma HLS inline
    static constexpr int halfWidth = width / 2;
    static constexpr int reducedSize = halfWidth + width % 2;
    Particle reduced[reducedSize];
    #pragma HLS array_partition variable=reduced complete
    Reduce:
    for(int i = 0; i < halfWidth; ++i){
        #pragma HLS unroll
        reduced[i] = Max(x[i*2], x[i*2 + 1]);
    }
    if(halfWidth != reducedSize){
        reduced[reducedSize - 1] = x[width - 1];
    }
    return MaxReduce<reducedSize>(reduced);
}

template<>
Particle MaxReduce<2>(const Particle x[2]){
    #pragma HLS inline
    return Max(x[0], x[1]);
}

template<>
Particle MaxReduce<1>(const Particle x[1]){
    #pragma HLS inline
    return x[0];   
}

void algo_main(const Particle particles[NPARTICLES], Jet jet[NJETS]) {
    #pragma HLS array_partition variable=particles complete
    #pragma HLS array_partition variable=jet complete
    #pragma HLS interface ap_none port=jet 
    #pragma HLS pipeline II=1

    Jet myjet[NJETS+MOREJETS];
    #pragma HLS array_partition variable=myjet complete
    
    Particle work[NPARTICLES];
    #pragma HLS array_partition variable=work complete

    for (unsigned int i = 0; i < NPARTICLES; ++i) {
        work[i] = particles[i];
    }

    for (unsigned int j = 0; j < NJETS+MOREJETS; ++j) {
        // Pick the highest pT particle as the seed
        Particle seedp = MaxReduce<NPARTICLES>(work);
        etaphi_t seed_eta = seedp.hwEta, seed_phi = seedp.hwPhi;
     
#ifndef __SYNTHESIS__
        //printf("HW algo iter %d: seed %2d of pt %.2f, eta %+.2f, phi %+.2f\n", j, int(-1), seedp.hwPt*0.25, seedp.hwEta*0.01, seedp.hwPhi*0.01);
#endif
        // this block builds the jet out of the seed, and zeroes out the used candidates
        ap_uint<15> sum_pt = 0;
        ap_int<22> sum_pt_eta = 0, sum_pt_phi = 0;
        ap_uint<5> count = 0;
        for (unsigned int i = 0; i < NPARTICLES-j; ++i) {
            int deta = work[i].hwEta - seed_eta;
            int dphi = work[i].hwPhi - seed_phi;
            bool incone = deta*deta + dphi*dphi < R2CONE;
            ap_uint<15> maybePt =  incone ? ap_uint<15>(work[i].hwPt(14,0)) : ap_uint<15>(0);
            sum_pt     += maybePt;
            sum_pt_eta += maybePt * ap_int<7>(deta); // range is bounded since |deta| < 0.04 = 40 units
            sum_pt_phi += maybePt * ap_int<7>(dphi);
            count += (maybePt > 0);
            work[i].hwPt  = incone ? pt_t(0) : work[i].hwPt;
            work[i].hwEta = work[i].hwEta;
            work[i].hwPhi = work[i].hwPhi;
#ifndef __SYNTHESIS__
            //if (incone) printf("                add cand %d, pt %.2f, eta %+.2f, phi %+.2f, cluster pt now %.2f, ncand %d\n", i, work[i].hwPt*0.25, work[i].hwEta*0.01, work[i].hwPhi*0.01,sum_pt*0.25, int(count));
#endif
        } 
        for (unsigned int i = NPARTICLES-j-1; i < NPARTICLES; ++i) {
            clear(work[i]);
        }
#ifndef __SYNTHESIS__
        //printf("HW algo iter %d: cluster of pt %.2f, ncand %d\n", j, sum_pt*0.25, int(count));
#endif
        reconstruct_jet(seed_eta, seed_phi, sum_pt, sum_pt_eta, sum_pt_phi, count, myjet[j]);
    }

    sort_and_crop<Jet,NJETS+MOREJETS,NJETS>(myjet, jet);

}


