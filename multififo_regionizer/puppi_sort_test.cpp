#include <cstdlib>

#include "./firmware/../../firmware/data.h"
#include "./firmware/regionizer.h"
//#include "../sort/merge_sort.h"
//#include "../sort/merge_sort_new.h"

//#include "./firmware/../../firmware/l1pf_encoding.h"

#define PACKING_PUPPIOBJ_SIZE 64
typedef ap_uint<PACKING_PUPPIOBJ_SIZE> PackedPuppiObj;


#define MYTYPE PuppiObj
#include "../sort/bitonic_hybrid.h"
//remove templates 
void sort_pfpuppi_cands_hybrid(PackedPuppiObj presort[NTRACK+NALLNEUTRALS],PackedPuppiObj sorted[NPUPPIFINALSORTED])
{
    #pragma HLS pipeline II=1
    #pragma HLS ARRAY_PARTITION variable=presort complete
    #pragma HLS ARRAY_PARTITION variable=sorted complete
    PuppiObj unpacked_presort[NTRACK+NALLNEUTRALS];
    #pragma HLS ARRAY_PARTITION variable=unpacked_presort complete
    
    l1pf_pattern_unpack<NTRACK+NALLNEUTRALS,0>(presort,unpacked_presort);
    bitonicSort<MYTYPE,NTRACK+NALLNEUTRALS,0>(unpacked_presort,0);  // remove one layer of wrapping
    l1pf_pattern_pack<NPUPPIFINALSORTED,0>(unpacked_presort,sorted);
    return ;

};

int main(int argc, char **argv) {

    PackedPuppiObj outpresort[NTRACK+NALLNEUTRALS];
    PackedPuppiObj outsorted[NPUPPIFINALSORTED];
    srand(123456);

    for(int i=0;i<NTRACK+NALLNEUTRALS ;++i){
        PuppiObj x;
        x.hwPt=rand()%1000; //16bit
        x.hwEta=rand()%1000; //10bit
        x.hwPhi=rand()%1000; //10bit
        x.hwId=rand()%8; //3bit
        x.hwData=rand()%1000; // 12bit

        outpresort[i] = l1pf_pattern_pack_one(x);
    }

    sort_pfpuppi_cands_hybrid(outpresort, outsorted) ;

    for(int i=0;i<NPUPPIFINALSORTED;++i)
    {
        PuppiObj x;
        PuppiObj y;
        l1pf_pattern_unpack_one(outsorted[i],x);
        l1pf_pattern_unpack_one(outsorted[i+1],y);
        if (not (x<y)) return 1;
    }

    return 0;
}
