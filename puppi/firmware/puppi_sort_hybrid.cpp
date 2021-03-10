#include "puppi_sort_hybrid.h"
#include "../../common/firmware/bitonic_hybrid.h"

//remove templates 
void sort_puppi_cands_bitonic(PackedPuppiObj presort[NTRACK+NALLNEUTRALS],PackedPuppiObj sorted[NPUPPIFINALSORTED])
{
    #pragma HLS pipeline II=1
    #pragma HLS ARRAY_PARTITION variable=presort complete
    #pragma HLS ARRAY_PARTITION variable=sorted complete
    #pragma HLS interface ap_none port=sorted
    l1ct::PuppiObj unpacked_presort[NTRACK+NALLNEUTRALS];
    #pragma HLS ARRAY_PARTITION variable=unpacked_presort complete
    l1ct::PuppiObj unpacked_sorted[NPUPPIFINALSORTED];
    #pragma HLS ARRAY_PARTITION variable=unpacked_sorted complete
    
    l1pf_pattern_unpack<NTRACK+NALLNEUTRALS>(presort,unpacked_presort);
    hybridBitonicSort::sort<NTRACK+NALLNEUTRALS,NPUPPIFINALSORTED,false>(unpacked_presort,unpacked_sorted);
    l1pf_pattern_pack<NPUPPIFINALSORTED>(unpacked_sorted,sorted);
};

//remove templates 
void sort_puppi_cands_hybrid(PackedPuppiObj presort[NTRACK+NALLNEUTRALS],PackedPuppiObj sorted[NPUPPIFINALSORTED])
{
    #pragma HLS pipeline II=1
    #pragma HLS ARRAY_PARTITION variable=presort complete
    #pragma HLS ARRAY_PARTITION variable=sorted complete
    #pragma HLS interface ap_none port=sorted
    l1ct::PuppiObj unpacked_presort[NTRACK+NALLNEUTRALS];
    #pragma HLS ARRAY_PARTITION variable=unpacked_presort complete
    l1ct::PuppiObj unpacked_sorted[NPUPPIFINALSORTED];
    #pragma HLS ARRAY_PARTITION variable=unpacked_sorted complete
    
    l1pf_pattern_unpack<NTRACK+NALLNEUTRALS>(presort,unpacked_presort);
    hybridBitonicSort::sort<NTRACK+NALLNEUTRALS,NPUPPIFINALSORTED,true>(unpacked_presort,unpacked_sorted);
    l1pf_pattern_pack<NPUPPIFINALSORTED>(unpacked_sorted,sorted);
};

