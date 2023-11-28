#include "algo.h"
#ifndef __SYNTHESIS__
#include <cstdio>
#endif

Puppi BestSeed(Puppi a, bool a_masked, Puppi b, bool b_masked) {
    bool bestByPt = (a.hwPt >= b.hwPt);
    return (!a_masked && (bestByPt || b_masked)) ? a : b;
}

template<int width>
Puppi BestSeedReduce(const Puppi x[width], const bool masked[width]){
    // Tree reduce from https://github.com/definelicht/hlslib/blob/master/include/hlslib/xilinx/TreeReduce.h
    #pragma HLS inline
    static constexpr int halfWidth = width / 2;
    static constexpr int reducedSize = halfWidth + width % 2;
    Puppi reduced[reducedSize];
    bool maskreduced[reducedSize]; 
    #pragma HLS array_partition variable=reduced complete
    for(int i = 0; i < halfWidth; ++i) {
        #pragma HLS unroll
        reduced[i] = BestSeed(x[i*2], masked[i*2], x[i*2+1], masked[i*2+1]);
        maskreduced[i] = (masked[i*2] && masked[i*2+1]);
    }
    if(halfWidth != reducedSize){
        reduced[reducedSize - 1] = x[width - 1];
        maskreduced[reducedSize - 1] = masked[width - 1];
    }
    return BestSeedReduce<reducedSize>(reduced, maskreduced);
}

template<>
Puppi BestSeedReduce<2>(const Puppi x[2], const bool masked[2]){
    #pragma HLS inline
    return BestSeed(x[0], masked[0], x[1], masked[1]);
}

template<int N>
Puppi::pt_t SumReduce(const Puppi::pt_t in[N]) {
       return SumReduce<N/2>(in) + SumReduce<N-N/2>(&in[N/2]);
}
template<>
Puppi::pt_t SumReduce<1>(const Puppi::pt_t in[1]) {
    return in[0];
}

Puppi find_seed(const Puppi in[NPUPPI_MAX], const bool masked[NPUPPI_MAX]) {
    return BestSeedReduce<NPUPPI_MAX>(in, masked);
}

ap_int<Puppi::eta_t::width+1> deltaEta(Puppi::eta_t eta1, Puppi::eta_t eta2) {
    #pragma HLS latency min=1
    #pragma HLS inline off
    return eta1 - eta2;
}

inline dr2_t deltaR2_slow(const Puppi & p1, const Puppi & p2) {
    auto dphi = p1.hwPhi - p2.hwPhi;
    if (dphi > Puppi::INT_PI) dphi -= Puppi::INT_2PI;
    else if (dphi < -Puppi::INT_PI) dphi += Puppi::INT_2PI;
    auto deta = deltaEta(p1.hwEta, p2.hwEta);
    return dphi*dphi + deta*deta;
}

void one_iteration(const Puppi in[NPUPPI_MAX], const bool masked[NPUPPI_MAX], bool masked_out[NPUPPI_MAX], Puppi & seed, Puppi::pt_t & absiso) {
    #pragma HLS ARRAY_PARTITION variable=in complete
    #pragma HLS ARRAY_PARTITION variable=masked complete
    #pragma HLS ARRAY_PARTITION variable=masked_out complete
    #pragma HLS pipeline II=9

    #pragma HLS inline off

    const dr2_t dr2_max = drToHwDr2(0.4), dr2_veto = drToHwDr2(0.1);

    Puppi::pt_t tosum[NPUPPI_MAX];
    #pragma HLS ARRAY_PARTITION variable=tosum complete

    seed = BestSeedReduce<NPUPPI_MAX>(in, masked);
    for (unsigned int i = 0; i < NPUPPI_MAX; ++i) {
        dr2_t dr2 = deltaR2_slow(seed, in[i]);
        bool inside = (dr2 < dr2_max);
        masked_out[i] = masked[i] || inside;
        tosum[i] =  inside && (dr2 > dr2_veto) ? in[i].hwPt : Puppi::pt_t(0);
    }
    absiso = SumReduce<NPUPPI_MAX>(tosum);
}


void compute_isolated_l1t(const Puppi in[NPUPPI_MAX], Puppi out[NISO_MAX], Puppi::pt_t out_absiso[NISO_MAX])  {
    #pragma HLS ARRAY_PARTITION variable=in complete
    #pragma HLS ARRAY_PARTITION variable=out complete
    #pragma HLS ARRAY_PARTITION variable=out_absiso complete
    #pragma HLS pipeline II=9

    const dr2_t dr2_max = drToHwDr2(0.4), dr2_veto = drToHwDr2(0.1);
    bool masked[NPUPPI_MAX], masked_out[NPUPPI_MAX];
    #pragma HLS ARRAY_PARTITION variable=masked complete
    #pragma HLS ARRAY_PARTITION variable=masked_out complete
    Puppi::pt_t tosum[NPUPPI_MAX];
    #pragma HLS ARRAY_PARTITION variable=tosum complete
    for (unsigned int i = 0; i < NPUPPI_MAX; ++i) {
        masked[i] = in[i].hwID <= 1; // neutral particles
    }
    for (unsigned int j = 0; j < NISO_MAX; ++j) {
        out[j].clear();
        out_absiso[j] = 0;
    }
    ap_uint<4> niso = 0;
    for (unsigned int j = 0; j < NISO_MAX; ++j) {
        Puppi seed;
        Puppi::pt_t iso_sum;
        one_iteration(in, masked, masked_out, seed, iso_sum);
        for (unsigned int i = 0; i < NPUPPI_MAX; ++i) {
            masked[i] = masked_out[i];
        }
#ifndef __SYNTHESIS__
      //printf("HW Seed pT %8.3f eta %+6.3f phi %+6.3f pid %1u: abs iso %8.3f\n",
      //       seed.floatPt(), seed.floatEta(), seed.floatPhi(), seed.hwID.to_uint(), Puppi::floatPt(iso_sum));
#endif
        if (iso_sum < seed.hwPt) {
            out[niso] = seed;
            out_absiso[niso] = iso_sum;
            niso++;
        }
    }
    
}
void compute_isolated_alveo(unsigned int N, const uint64_t *in, unsigned int & nout, uint64_t *out, uint16_t *out_absiso) {
    // left as exercise
}
