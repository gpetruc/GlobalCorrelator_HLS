#include "algo.h"
#include <cassert>
#include <algorithm>

#define NSINCOS 1024
typedef ap_fixed<9,1,AP_RND,AP_SAT> sincos_t; 
// saturation is important to avoid wrap-around (a value of exactly +1 can't be represented,
// and we want it to be approximated as 0.99x rather than wrapping around to -1)

typedef ap_fixed<15,13,AP_RND,AP_SAT> pxy_t;
typedef ap_ufixed<12,0,AP_RND,AP_SAT> invpt6_t;

void _lut_sincos_init(ap_uint<18> table_sincos[NSINCOS]) {
    for (int i = 0; i < NSINCOS; ++i) {
        float alpha = Puppi::floatPhi(Puppi::phi_t(i));
        sincos_t is = sin(alpha), ic = cos(alpha);
        table_sincos[i](8,0) = is(8,0);
        table_sincos[i](17,9) = ic(8,0);
    }
}
void toCartesian(Puppi::pt_t pt, Puppi::phi_t phi, pxy_t & px, pxy_t & py) {
    ap_uint<18> _table_sincos[NSINCOS];
    _lut_sincos_init(_table_sincos);
    int iphi = phi;
    if (phi < 0) iphi = -iphi;
    assert(iphi >= 0 && iphi < NSINCOS);
    ap_uint<18> both = _table_sincos[iphi];
    sincos_t s; s(8,0) = both(8,0);
    if (phi < 0) s = -s;
    sincos_t c; c(8,0) = both(17,9);
    px = pt * c;
    py = pt * s;
}

#define SQRT_TABLE_SIZE (1<<12)
void _lut_sqrt_init(ap_uint<6> table[SQRT_TABLE_SIZE]) {
    for (int i = 0; i < SQRT_TABLE_SIZE; ++i) {
        float pt2f = float(i), ptf = round(std::sqrt(pt2f));
        table[i] = std::min<int>(round(ptf), (1<<6)-1);
    }
}
#define INV_TABLE_SIZE (1<<6)
void _lut_invert_init(invpt6_t table[INV_TABLE_SIZE]) {
    for (int i = 0; i < INV_TABLE_SIZE; ++i) {
        float x = 1.0/std::max(i,1);
        table[i] = x;
    }
}
#define ACOS_TABLE_SIZE (1<<10)
void _lut_acos_init(ap_ufixed<12,1> table[ACOS_TABLE_SIZE]) {
    for (int i = 0; i < ACOS_TABLE_SIZE; ++i) {
        ap_uint<10> x_uint = i;
        ap_fixed<10,1> x_ap;
        x_ap(9,0) = x_uint(9,0);
        float cosphi = x_ap.to_float();
        float phi = std::acos(cosphi);
        ap_ufixed<12,1,AP_RND,AP_WRAP> phi_ap = phi/M_PI;  
        table[i] = phi_ap;
    }
}

void px_py_to_pt_phi(pxy_t px, pxy_t py, Sum::pt_t &pt, Sum::phi_t &phi) {
    ap_uint<6> _sqrt_table[SQRT_TABLE_SIZE];
    _lut_sqrt_init(_sqrt_table);
    invpt6_t _inv_table[INV_TABLE_SIZE];
    _lut_invert_init(_inv_table);
    ap_ufixed<12,1> _acos_table[ACOS_TABLE_SIZE];
    _lut_acos_init(_acos_table);
    // max |px| or |py| is 2^12 --> max |px|^2 = 2^24 -> max |pt|^2 = 2^25
    ap_uint<25> pt2 = (px * px) + (py * py); 

    ap_uint<12> index ; 
    ap_uint<3> shift;
    if (pt2[24]) { 
        index[11] = 0;
        index(10,0) = pt2(24,14);
        shift = 7;
    } else if (pt2[23] || pt2[22]) {
        index = pt2(23,12);
        shift = 6;
    } else if (pt2[21] || pt2[20]) {
        index = pt2(21,10);
        shift = 5;
    } else if (pt2[19] || pt2[18]) {
        index = pt2(19,8);
        shift = 4;
    } else if (pt2[17] || pt2[16]) {
        index = pt2(17,6);
        shift = 3;
    } else if (pt2[15] || pt2[14]) {
        index = pt2(15,4);
        shift = 2;
    } else if (pt2[13] || pt2[12]) {
        index = pt2(13,2);
        shift = 1;
    } else {
        index = pt2(11,0);
        shift = 0;
    }

    ap_uint<6> pt6 = _sqrt_table[index];
    pt = (ap_uint<13>(pt6) << shift);

    invpt6_t invpt6 = _inv_table[pt6]; 
    ap_fixed<10,1,AP_RND,AP_SAT> cosphi = (px >> shift) * invpt6;
    ap_uint<10> icosphi; icosphi(9,0) = cosphi(9,0);
    ap_ufixed<12,1> absphi = _acos_table[icosphi];
    phi = (py >= 0 ? Sum::phi_t(absphi) : Sum::phi_t(-absphi));

    #ifndef __SYNTHESIS__
      printf("HW  px = %+8.3f, py = %+8.3f\n", px.to_float(), py.to_float());
      printf("HW  pt2 = %8.2f, pt = %8.3f, 1/pt = %.6f\n", pt2.to_float(), pt.to_float(), invpt6.to_float()/(1<<shift));
      printf("HW  cosphi = %7.4f, iphi = %u, phi = %+8.3f\n", cosphi.to_float(), icosphi.to_uint(), phi.to_float());
    #endif
}

template<unsigned int N>
pxy_t sumpxy(const pxy_t values[N]) {
   return sumpxy<N/2>(values) +
          sumpxy<N-N/2>(&values[N/2]);

}

template<unsigned int N>
pxy_t sumpxy(const pxy_t values[N], bool ismu[N]) {
   return sumpxy<N/2>(values, ismu) +
          sumpxy<N-N/2>(&values[N/2], &ismu[N/2]);

}

template<unsigned int N>
Puppi::pt_t sumpt(const Puppi values[N]) {
   return sumpt<N/2>(values) +
          sumpt<N-N/2>(&values[N/2]);

}

template<unsigned int N>
Puppi::pt_t sumpt(const Puppi values[N], bool ismu[N]) {
   return sumpt<N/2>(values, ismu) +
          sumpt<N-N/2>(&values[N/2], &ismu[N/2]);

}

template<>
pxy_t sumpxy<1>(const pxy_t values[1]) {
   return values[0];
}

template<>
pxy_t sumpxy<1>(const pxy_t values[1], bool ismu[1]) {
   return ismu[0] ? pxy_t(0) : values[0];
}


template<>
Puppi::pt_t sumpt<1>(const Puppi values[1]) {
   return values[0].hwPt;
}

template<>
Puppi::pt_t sumpt<1>(const Puppi values[1], bool ismu[1]) {
   return ismu[0] ? Puppi::pt_t(0) : values[0].hwPt;
}


void compute_sums_l1t(const Puppi in[NPUPPI_MAX], Sum & out, Sum & out_nomu) {
    #pragma HLS ARRAY_PARTITION variable=in complete
    #pragma HLS pipeline II=54
    pxy_t px[NPUPPI_MAX], py[NPUPPI_MAX];
    bool ismu[NPUPPI_MAX];
    #pragma HLS ARRAY_PARTITION variable=px complete
    #pragma HLS ARRAY_PARTITION variable=py complete
    #pragma HLS ARRAY_PARTITION variable=ismu complete
    for (unsigned int i = 0; i < NPUPPI_MAX; ++i) {
       toCartesian(in[i].hwPt, in[i].hwPhi, px[i], py[i]);
       ismu[i] = in[i].hwID >= 6;
    }
    pxy_t sumx = sumpxy<NPUPPI_MAX>(px), sumy = sumpxy<NPUPPI_MAX>(py);
    pxy_t sumx_nomu = sumpxy<NPUPPI_MAX>(px, ismu), sumy_nomu = sumpxy<NPUPPI_MAX>(py, ismu);
    Puppi::pt_t sum = sumpt<NPUPPI_MAX>(in), sum_nomu = sumpt<NPUPPI_MAX>(in, ismu);
    out.valid = 1;
    px_py_to_pt_phi(-sumx, -sumy, out.hwPt, out.hwPhi);
    out.hwPtTot = sum;
    px_py_to_pt_phi(-sumx_nomu, -sumy_nomu, out_nomu.hwPt, out_nomu.hwPhi);
    out_nomu.hwPtTot = sum_nomu;
}

void compute_sums_alveo(unsigned int N, const uint64_t *in, uint64_t out[2])  {  
    #pragma hls interface mode=m_axi port=in offset=slave bundle=gmem
    #pragma hls interface mode=m_axi port=out offset=slave bundle=gmem
    Puppi puppi[NPUPPI_MAX];
    #pragma HLS ARRAY_PARTITION variable=puppi complete
    Sum sum, sum_nomu;
    assert(N < NPUPPI_MAX);
    for (unsigned int i = 0; i < N; ++i) {
        puppi[i].unpack(in[i]);
    }
    for (unsigned int i = N; i < NPUPPI_MAX; ++i) {
        puppi[i].clear();
    }
    compute_sums_l1t(puppi, sum, sum_nomu);
    out[0] = sum.pack();
    out[1] = sum_nomu.pack();
}

