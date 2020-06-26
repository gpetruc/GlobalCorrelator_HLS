#include "func.h"
#include <cassert>
#include <cmath>

ret_b op_int(const aterm_b a[NDATA], const bterm_b b[NDATA], const cterm_b c[NDATA]) {
    #pragma HLS pipeline II=1
    #pragma HLS array_partition variable=a complete
    #pragma HLS array_partition variable=b complete
    #pragma HLS array_partition variable=c complete

    acc_b sum = 0;
    for (unsigned int i = 0; i < NDATA; ++i) {
        acc_b prod = (a[i] * b[i]) >> (a_dec + b_dec - c_dec);
        sum += prod + c[i];
    }
    int ret = sum >> (c_dec - r_dec);
    return ret_b(ret >= (1<<(ret_b::width-1)) ? ((1<<(ret_b::width-1))-1) : ret);
}

ret_f op_fix(const aterm_f a[NDATA], const bterm_f b[NDATA], const cterm_f c[NDATA]) {
    #pragma HLS pipeline II=1
    #pragma HLS array_partition variable=a complete
    #pragma HLS array_partition variable=b complete
    #pragma HLS array_partition variable=c complete

    acc_f sum = 0;
    for (unsigned int i = 0; i < NDATA; ++i) {
        sum += (a[i] * b[i]) + c[i];
        assert(sum >= 0);
    }

    const ret_f maxret = (std::pow(2.f, ret_f::width-1)-1) / std::pow(2.f, ret_f::width-ret_f::iwidth);
    return ( sum > maxret ? maxret : ret_f(sum) );
}

ret_s op_sat(const aterm_s a[NDATA], const bterm_s b[NDATA], const cterm_s c[NDATA]) {
    #pragma HLS pipeline II=1
    #pragma HLS array_partition variable=a complete
    #pragma HLS array_partition variable=b complete
    #pragma HLS array_partition variable=c complete

    acc_s sum = 0;
    for (unsigned int i = 0; i < NDATA; ++i) {
        sum += (a[i] * b[i]) + c[i];
        assert(sum >= 0);
    }

    return ret_s(sum);
}

template<unsigned int N>
acc_b redux_tree_int(const acc_b in[N]) {
    #pragma HLS inline
    static constexpr int halfWidth = N / 2;
    static constexpr int reducedSize = halfWidth + N % 2;
    acc_b reduced[reducedSize];
    #pragma HLS ARRAY_PARTITION variable=reduced complete
    for (int i = 0; i < halfWidth; ++i) {
      #pragma HLS unroll
      reduced[i] = in[2*i] + in[2*i+1];
    }
    if (halfWidth != reducedSize) {
      reduced[reducedSize - 1] = in[N - 1];
    }
    return redux_tree_int<reducedSize>(reduced);
}

template<>
acc_b redux_tree_int<1>(const acc_b in[1]) {
    #pragma HLS inline
    return in[0];
}
template<>
acc_b redux_tree_int<2>(const acc_b in[2]) {
    #pragma HLS inline
    return in[0]+in[1];
}


template<unsigned int N>
acc_f redux_tree_fix(const acc_f in[N]) {
    #pragma HLS inline
    static constexpr int halfWidth = N / 2;
    static constexpr int reducedSize = halfWidth + N % 2;
    acc_f reduced[reducedSize];
    #pragma HLS ARRAY_PARTITION variable=reduced complete
    for (int i = 0; i < halfWidth; ++i) {
      #pragma HLS unroll
      reduced[i] = in[2*i] + in[2*i+1];
    }
    if (halfWidth != reducedSize) {
      reduced[reducedSize - 1] = in[N - 1];
    }
    return redux_tree_fix<reducedSize>(reduced);
}

template<>
acc_f redux_tree_fix<1>(const acc_f in[1]) {
    #pragma HLS inline
    return in[0];
}
template<>
acc_f redux_tree_fix<2>(const acc_f in[2]) {
    #pragma HLS inline
    return in[0]+in[1];
}

template<unsigned int N>
acc_s redux_tree_sat(const acc_s in[N]) {
    #pragma HLS inline
    static constexpr int halfWidth = N / 2;
    static constexpr int reducedSize = halfWidth + N % 2;
    acc_s reduced[reducedSize];
    #pragma HLS ARRAY_PARTITION variable=reduced complete
    for (int i = 0; i < halfWidth; ++i) {
      #pragma HLS unroll
      reduced[i] = in[2*i] + in[2*i+1];
    }
    if (halfWidth != reducedSize) {
      reduced[reducedSize - 1] = in[N - 1];
    }
    return redux_tree_sat<reducedSize>(reduced);
}

template<>
acc_s redux_tree_sat<1>(const acc_s in[1]) {
    #pragma HLS inline
    return in[0];
}
template<>
acc_s redux_tree_sat<2>(const acc_s in[2]) {
    #pragma HLS inline
    return in[0]+in[1];
}

ret_b op_int_redux(const aterm_b a[NDATA], const bterm_b b[NDATA], const cterm_b c[NDATA]) {
    #pragma HLS pipeline II=1
    #pragma HLS array_partition variable=a complete
    #pragma HLS array_partition variable=b complete
    #pragma HLS array_partition variable=c complete

    acc_b term[NDATA];
    #pragma HLS array_partition variable=term complete
    for (unsigned int i = 0; i < NDATA; ++i) {
        ap_int<30> prod = (a[i] * b[i]) >> (a_dec + b_dec - c_dec);
        term[i] = prod + c[i];
    }

    int ret = redux_tree_int<NDATA>(term) >> (c_dec - r_dec);
    return ret_b(ret >= (1<<(ret_b::width-1)) ? ((1<<(ret_b::width-1))-1) : ret);
}

ret_f op_fix_redux(const aterm_f a[NDATA], const bterm_f b[NDATA], const cterm_f c[NDATA]) {
    #pragma HLS pipeline II=1
    #pragma HLS array_partition variable=a complete
    #pragma HLS array_partition variable=b complete
    #pragma HLS array_partition variable=c complete

    acc_f term[NDATA];
    #pragma HLS array_partition variable=term complete
    for (unsigned int i = 0; i < NDATA; ++i) {
        term[i] = (a[i] * b[i]) + c[i];
    }

    acc_f sum = redux_tree_fix<NDATA>(term);
    const ret_f maxret = (std::pow(2.f, ret_f::width-1)-1) / std::pow(2.f, ret_f::width-ret_f::iwidth);
    return ( sum > maxret ? maxret : ret_f(sum) );
}

ret_s op_sat_redux(const aterm_s a[NDATA], const bterm_s b[NDATA], const cterm_s c[NDATA]) {
    #pragma HLS pipeline II=1
    #pragma HLS array_partition variable=a complete
    #pragma HLS array_partition variable=b complete
    #pragma HLS array_partition variable=c complete

    acc_s term[NDATA];
    #pragma HLS array_partition variable=term complete
    for (unsigned int i = 0; i < NDATA; ++i) {
        term[i] = (a[i] * b[i]) + c[i];
    }

    acc_s sum = redux_tree_sat<NDATA>(term);
    return ret_s(sum);
}



