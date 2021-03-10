#ifndef BITONIC_HYBRID_REF_H
#define BITONIC_HYBRID_REF_H

#include<algorithm>

namespace hybridBitonicSortUtils {
    unsigned int PowerOf2LessThan(unsigned int n){
        unsigned int i=1;
        unsigned int prev=1;
        while (i<n){
            i<<=1;
            if (i<n) {
                prev =i;
            } else {
                return prev;
            }
        }
    }

    template<typename T>
    void compAndSwap(T a[], int i, int j, bool dir) 
    { 
        if (dir){ if (a[j]<a[i]) std::swap(a[i],a[j]); } 
        else { if (a[i]<a[j]) std::swap(a[i],a[j]); }
    } 
}

template<typename T>
void hybridBitonicMergeRef(T a[], int N, int low, bool dir)
{
    int k = hybridBitonicSortUtils::PowerOf2LessThan(N);
    int k2 = N-k;
    if (N>1) { 
        for (int i=low; i<low+k; i++){  
            if (i+k<low+N) hybridBitonicSortUtils::compAndSwap(a, i, i+k, dir); 
        }

        hybridBitonicMergeRef(a, k, low, dir); 
        hybridBitonicMergeRef(a, k2, low+k, dir); 
    }
} 

template<typename T>
void hybridBitonicSortRef(T a[], int N, int low, bool dir, bool hybrid )
{
    if (hybrid) { // sorting networks defined by hand for a few cases
        switch(N) {
            case 2: 
                hybridBitonicSortUtils::compAndSwap(a, low+0, low+1, dir);
                return;
            case 3:
                hybridBitonicSortUtils::compAndSwap(a, low+0, low+1, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+1, low+2, dir); 
                //---
                hybridBitonicSortUtils::compAndSwap(a, low+0, low+1, dir); 
                return;
            case 4:
                hybridBitonicSortUtils::compAndSwap(a, low+0, low+1, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+2, low+3, dir); 
                //---
                hybridBitonicSortUtils::compAndSwap(a, low+0, low+2, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+1, low+3, dir); 
                //---
                hybridBitonicSortUtils::compAndSwap(a, low+1, low+2, dir); 
                return;
            case 5:
                hybridBitonicSortUtils::compAndSwap(a, low+0, low+1, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+1, low+2, dir); 
                //---
                hybridBitonicSortUtils::compAndSwap(a, low+1, low+3, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+2, low+4, dir); 
                //---
                hybridBitonicSortUtils::compAndSwap(a, low+0, low+2, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+1, low+4, dir); 
                //--
                hybridBitonicSortUtils::compAndSwap(a, low+1, low+2, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+3, low+4, dir); 
                //--
                hybridBitonicSortUtils::compAndSwap(a, low+2, low+3, dir); 
                return;
            case 6:
                //---
                hybridBitonicSortUtils::compAndSwap(a, low+0, low+3, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+1, low+2, dir); 
                //---
                hybridBitonicSortUtils::compAndSwap(a, low+0, low+1, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+2, low+3, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+4, low+5, dir); 
                //---
                hybridBitonicSortUtils::compAndSwap(a, low+0, low+3, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+1, low+4, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+2, low+5, dir); 
                //---
                hybridBitonicSortUtils::compAndSwap(a, low+0, low+1, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+2, low+4, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+3, low+5, dir); 
                //---
                hybridBitonicSortUtils::compAndSwap(a, low+1, low+2, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+3, low+4, dir); 
                return;
            case 12:
                for(int i=0;i<12;i+=2){
                    hybridBitonicSortUtils::compAndSwap(a, low+i, low+i+1, dir); 
                }
                //---
                for(int i=0;i<12;i+=4){
                    hybridBitonicSortUtils::compAndSwap(a, low+i+0, low+i+2, dir); 
                    hybridBitonicSortUtils::compAndSwap(a, low+i+1, low+i+3, dir); 
                }
                //---
                hybridBitonicSortUtils::compAndSwap(a, low+0, low+4, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+1, low+5, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+2, low+6, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+7, low+11, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+9, low+10, dir); 
                //---
                hybridBitonicSortUtils::compAndSwap(a, low+1, low+2, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+6, low+10, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+5, low+9, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+4, low+8, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+3, low+7, dir); 
                //---
                hybridBitonicSortUtils::compAndSwap(a, low+2, low+6, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+1, low+5, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+0, low+4, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+9, low+10, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+7, low+11, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+3, low+8, dir); 
                //---
                hybridBitonicSortUtils::compAndSwap(a, low+1, low+4, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+7, low+10, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+2, low+3, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+5, low+6, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+8, low+9, dir); 
                //---
                hybridBitonicSortUtils::compAndSwap(a, low+2, low+4, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+3, low+5, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+6, low+8, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+7, low+9, dir); 
                //---
                hybridBitonicSortUtils::compAndSwap(a, low+3, low+4, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+5, low+6, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+7, low+8, dir); 
                return;
            case 13:
                for(int i=0;i+1<13;i+=2){
                    #pragma HLS unroll
                    hybridBitonicSortUtils::compAndSwap(a, low+i, low+i+1, dir); 
                }
                //---
                for(int i=0;i+3<13;i+=4){
                    #pragma HLS unroll
                    hybridBitonicSortUtils::compAndSwap(a, low+i+0, low+i+2, dir); 
                    hybridBitonicSortUtils::compAndSwap(a, low+i+1, low+i+3, dir); 
                }
                //---
                hybridBitonicSortUtils::compAndSwap(a, low+0, low+4, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+1, low+5, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+2, low+6, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+3, low+7, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+8, low+12, dir); 
                //---
                hybridBitonicSortUtils::compAndSwap(a, low+0, low+8, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+1, low+9, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+2, low+10, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+3, low+11, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+4, low+12, dir); 
                //---
                hybridBitonicSortUtils::compAndSwap(a, low+1, low+2, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+3, low+12, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+7, low+11, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+4, low+8, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+5, low+10, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+6, low+9, dir); 
                //---
                hybridBitonicSortUtils::compAndSwap(a, low+1, low+4, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+2, low+8, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+6, low+12, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+3, low+10, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+5, low+9, dir); 
                //---
                hybridBitonicSortUtils::compAndSwap(a, low+2, low+4, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+3, low+5, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+6, low+8, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+7, low+9, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+10, low+12, dir); 
                //---
                hybridBitonicSortUtils::compAndSwap(a, low+3, low+6, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+5, low+8, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+7, low+10, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+9, low+12, dir); 
                //---
                hybridBitonicSortUtils::compAndSwap(a, low+3, low+4, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+5, low+6, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+7, low+8, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+9, low+10, dir); 
                hybridBitonicSortUtils::compAndSwap(a, low+11, low+12, dir); 
                return;
        }
    }

    // general case
    if (N > 1) {
        int lowerSize= N/2;
        int upperSize= N-N/2;
        bool notDir = not dir;
        hybridBitonicSortRef(a, lowerSize, low,           notDir, hybrid);
        hybridBitonicSortRef(a, upperSize, low+lowerSize, dir,    hybrid);  
        hybridBitonicMergeRef(a, N, low, dir); 
    }
}


template<typename T>
void hybrid_bitonic_sort_and_crop_ref(unsigned int nIn, unsigned int nOut, T in[], T out[], bool hybrid=true) 
{  // just an interface
    T work[nIn];
    for (unsigned int i = 0; i < nIn; ++i) {
        work[i] = in[i];
    }
    hybridBitonicSortRef(work, nIn, 0, false, hybrid);
    for (unsigned int i = 0; i < nOut; ++i) {
        out[i] = work[i];
    }
}

#endif
