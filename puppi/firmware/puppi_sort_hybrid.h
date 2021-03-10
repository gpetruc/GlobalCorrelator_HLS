#ifndef puppi_sort_hybrid_h
#define puppi_sort_hybrid_h

#include "../../dataformats/puppi.h"

#include "../../dataformats/layer1_multiplicities.h"

typedef ap_uint<64> PackedPuppiObj;

void sort_puppi_cands_hybrid(PackedPuppiObj presort[NTRACK+NALLNEUTRALS],PackedPuppiObj sorted[NPUPPIFINALSORTED]) ;
void sort_puppi_cands_bitonic(PackedPuppiObj presort[NTRACK+NALLNEUTRALS],PackedPuppiObj sorted[NPUPPIFINALSORTED]) ;

#endif

