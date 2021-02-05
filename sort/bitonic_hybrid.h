/**************************************************
 * HLS version of Bitonic/Hybrid: author Andrea Carlo Marini (CERN) 28/01/2021
 * Search network for optimal depth from: The art of computer programming, vol. III, E. D. Knuth.
 *
 * ********************************************************
 * *                                                      *
 * *   This work is licensed under CC BY-SA 4.0           *
 * *   http://creativecommons.org/licenses/by-sa/4.0/     *
 * *                                                      *
 * *     Dr. Andrea Carlo Marini, CERN, 2021              *
 * *                                                      *
 * ********************************************************
 */

/*    MYTYPE -> Type template specialization will be constructed against
 *    USE_HYBRID -> use low input template specialization optimized for depth. Impl. n=2,3,4,5,6,7, 12,13 (TODO known<=16)
 */   

#ifndef BITONIC_HYBRID_H
#define BITONIC_HYBRID_H

/*
 * Master switch to use hybrid approach or fully bitonic
 */
#define USE_HYBRID

/*
 * define the type as the sorting type in order to have the specialization correctly done. 
 * C++ does not allow for partial specialization
 */
#ifndef MYTYPE
    #define MYTYPE int
#endif

// the following can be written as constexpr in C++11. The above maybe in C++14
constexpr int PowerOf2LessEqualThan(int n){ //  to be called for n>0: 
    return (n && !(n&(n-1))) ? n : PowerOf2LessEqualThan(n-1);
}

constexpr int PowerOf2LessThan(int n){ //  to be called for n>1
    return PowerOf2LessEqualThan(n-1);
}

template <typename T>
inline T max(T a, T b)  
{  
    #pragma HLS inline
    return (a < b)? b : a;  
}  

template <typename T>
inline T min(T a, T b)  
{  
    #pragma HLS inline
    return (a < b)? a : b;  
}  

template<typename T>
void myswap(T& a, T&b)
{
    #pragma HLS inline
	T c = a;
	a=b;
	b=c;
	return;
}

template <typename T,int dir>
void compAndSwap(T a[], int i, int j) 
{ 
    #pragma HLS inline
    #pragma HLS array_partition variable=a complete
    // 14  34 12 23
    if (dir){ if (a[j]<a[i]) myswap(a[i],a[j]);} // (1)
    else { if (a[i]<a[j]) myswap(a[i],a[j]);} // (2)
} 

/*It recursively sorts a bitonic sequence in ascending order, 
  if dir = 1, and in descending order otherwise (means dir=0). 
  The sequence to be sorted starts at index position low, 
  the parameter N is the number of elements to be sorted.*/
template<typename T,int N,int dir>
void bitonicMerge(T a[], int low) 
{ 
    #pragma HLS inline
    #pragma HLS array_partition variable=a complete
    //static constexpr int k = N/2; 
    static constexpr int k = PowerOf2LessThan(N);
    static constexpr int k2 = N-k;
    if (N>1) 
    { 
        for (int i=low; i<low+k; i++){  
        //#pragma HLS pipeline II=1
        #pragma HLS unroll
            if (i+k<low+N) compAndSwap<T,dir>(a, i, i+k); 
        }

        bitonicMerge<T,k,dir>(a, low); 
        bitonicMerge<T,k2,dir>(a, low+k); 

    } 
} 

template<>
void bitonicMerge<MYTYPE,1,1>(MYTYPE a[], int low) 
{
    #pragma HLS inline
    return;
}

template<>
void bitonicMerge<MYTYPE,1,0>(MYTYPE a[], int low) 
{
    #pragma HLS inline
    return;
}

/* This function first produces a bitonic sequence by recursively 
    sorting its two halves in opposite sorting orders, and then 
    calls bitonicMerge to make them in the same order */
template <typename T,int N,int dir>
void bitonicSort(T a[],int low) 
{ 
    #pragma HLS inline
    #pragma HLS array_partition variable=a complete
    static constexpr int lowerSize= N/2;
    static constexpr int upperSize= N-N/2;
    static constexpr int notDir = not dir;

    if (N>1) 
    { 
        // sort in ascending order since dir here is 1 
        //bitonicSort(a, low, k, 1);  // dir, ?
        bitonicSort<T,lowerSize,notDir>(a, low);

        // sort in descending order since dir here is 0 
        bitonicSort<T,upperSize,dir>(a, low+lowerSize);  
  
        // Will merge wole sequence in ascending order 
        // since dir=1. 
        bitonicMerge<T,N,dir>(a,low); 
    } 
} 

/********************************************
 *                N=1                       *
 ********************************************/
template <>
void bitonicSort<MYTYPE,1,1>(MYTYPE a[],int low) 
{ 
    #pragma HLS inline
    return ;
} 

template <>
void bitonicSort<MYTYPE,1,0>(MYTYPE a[],int low) 
{ 
    #pragma HLS inline
    return;
} 

#ifdef USE_HYBRID
//hybrid specializations to speed up low N sorts
/********************************************
 *                N=2                       *
 ********************************************/
template<>
void bitonicSort<MYTYPE,2,0>(MYTYPE a[], int low){
    #pragma HLS inline
    #pragma HLS array_partition variable=a complete
    //---
    compAndSwap<MYTYPE,0>(a, low+0, low+1); 
}
template<>
void bitonicSort<MYTYPE,2,1>(MYTYPE a[], int low){
    #pragma HLS inline
    #pragma HLS array_partition variable=a complete
    //---
    compAndSwap<MYTYPE,1>(a, low+0, low+1); 
}
/********************************************
 *                N=3                       *
 ********************************************/
template<>
void bitonicSort<MYTYPE,3,0>(MYTYPE a[], int low){
    #pragma HLS inline
    #pragma HLS array_partition variable=a complete
    //---
    compAndSwap<MYTYPE,0>(a, low+0, low+1); 
    compAndSwap<MYTYPE,0>(a, low+1, low+2); 
    //---
    compAndSwap<MYTYPE,0>(a, low+0, low+1); 
}
template<>
void bitonicSort<MYTYPE,3,1>(MYTYPE a[], int low){
    #pragma HLS inline
    #pragma HLS array_partition variable=a complete
    //---
    compAndSwap<MYTYPE,1>(a, low+0, low+1); 
    compAndSwap<MYTYPE,1>(a, low+1, low+2); 
    //---
    compAndSwap<MYTYPE,1>(a, low+0, low+1); 
}
/********************************************
 *                N=4                       *
 ********************************************/
template<>
void bitonicSort<MYTYPE,4,0>(MYTYPE a[], int low){
    #pragma HLS inline
    #pragma HLS array_partition variable=a complete
    //---
    compAndSwap<MYTYPE,0>(a, low+0, low+1); 
    compAndSwap<MYTYPE,0>(a, low+2, low+3); 
    //---
    compAndSwap<MYTYPE,0>(a, low+0, low+2); 
    compAndSwap<MYTYPE,0>(a, low+1, low+3); 
    //---
    compAndSwap<MYTYPE,0>(a, low+1, low+2); 
}
template<>
void bitonicSort<MYTYPE,4,1>(MYTYPE a[], int low){
    #pragma HLS inline
    #pragma HLS array_partition variable=a complete
    //---
    compAndSwap<MYTYPE,1>(a, low+0, low+1); 
    compAndSwap<MYTYPE,1>(a, low+2, low+3); 
    //---
    compAndSwap<MYTYPE,1>(a, low+0, low+2); 
    compAndSwap<MYTYPE,1>(a, low+1, low+3); 
    //---
    compAndSwap<MYTYPE,1>(a, low+1, low+2); 
}

/********************************************
 *                N=5                       *
 ********************************************/
template<>
void bitonicSort<MYTYPE,5,0>(MYTYPE a[], int low){
    #pragma HLS inline
    #pragma HLS array_partition variable=a complete
    //---
    compAndSwap<MYTYPE,0>(a, low+0, low+1); 
    compAndSwap<MYTYPE,0>(a, low+1, low+2); 
    //---
    compAndSwap<MYTYPE,0>(a, low+1, low+3); 
    compAndSwap<MYTYPE,0>(a, low+2, low+4); 
    //---
    compAndSwap<MYTYPE,0>(a, low+0, low+2); 
    compAndSwap<MYTYPE,0>(a, low+1, low+4); 
    //--
    compAndSwap<MYTYPE,0>(a, low+1, low+2); 
    compAndSwap<MYTYPE,0>(a, low+3, low+4); 
    //--
    compAndSwap<MYTYPE,0>(a, low+2, low+3); 
}
template<>
void bitonicSort<MYTYPE,5,1>(MYTYPE a[], int low){
    #pragma HLS inline
    #pragma HLS array_partition variable=a complete
    //---
    compAndSwap<MYTYPE,1>(a, low+0, low+1); 
    compAndSwap<MYTYPE,1>(a, low+1, low+2); 
    //---
    compAndSwap<MYTYPE,1>(a, low+1, low+3); 
    compAndSwap<MYTYPE,1>(a, low+2, low+4); 
    //---
    compAndSwap<MYTYPE,1>(a, low+0, low+2); 
    compAndSwap<MYTYPE,1>(a, low+1, low+4); 
    //--
    compAndSwap<MYTYPE,1>(a, low+1, low+2); 
    compAndSwap<MYTYPE,1>(a, low+3, low+4); 
    //--
    compAndSwap<MYTYPE,1>(a, low+2, low+3); 
}

/********************************************
 *                N=6                       *
 ********************************************/
template<>
void bitonicSort<MYTYPE,6,0>(MYTYPE a[], int low){
    #pragma HLS inline
    #pragma HLS array_partition variable=a complete
    //---
    compAndSwap<MYTYPE,0>(a, low+0, low+3); 
    compAndSwap<MYTYPE,0>(a, low+1, low+2); 
    //---
    compAndSwap<MYTYPE,0>(a, low+0, low+1); 
    compAndSwap<MYTYPE,0>(a, low+2, low+3); 
    compAndSwap<MYTYPE,0>(a, low+4, low+5); 
    //---
    compAndSwap<MYTYPE,0>(a, low+0, low+3); 
    compAndSwap<MYTYPE,0>(a, low+1, low+4); 
    compAndSwap<MYTYPE,0>(a, low+2, low+5); 
    //---
    compAndSwap<MYTYPE,0>(a, low+0, low+1); 
    compAndSwap<MYTYPE,0>(a, low+2, low+4); 
    compAndSwap<MYTYPE,0>(a, low+3, low+5); 
    //---
    compAndSwap<MYTYPE,0>(a, low+1, low+2); 
    compAndSwap<MYTYPE,0>(a, low+3, low+4); 
}
template<>
void bitonicSort<MYTYPE,6,1>(MYTYPE a[], int low){
    #pragma HLS inline
    #pragma HLS array_partition variable=a complete
    //---
    compAndSwap<MYTYPE,1>(a, low+0, low+3); 
    compAndSwap<MYTYPE,1>(a, low+1, low+2); 
    //---
    compAndSwap<MYTYPE,1>(a, low+0, low+1); 
    compAndSwap<MYTYPE,1>(a, low+2, low+3); 
    compAndSwap<MYTYPE,1>(a, low+4, low+5); 
    //---
    compAndSwap<MYTYPE,1>(a, low+0, low+3); 
    compAndSwap<MYTYPE,1>(a, low+1, low+4); 
    compAndSwap<MYTYPE,1>(a, low+2, low+5); 
    //---
    compAndSwap<MYTYPE,1>(a, low+0, low+1); 
    compAndSwap<MYTYPE,1>(a, low+2, low+4); 
    compAndSwap<MYTYPE,1>(a, low+3, low+5); 
    //---
    compAndSwap<MYTYPE,1>(a, low+1, low+2); 
    compAndSwap<MYTYPE,1>(a, low+3, low+4); 
}

/********************************************
 *                N=12                       *
 ********************************************/
template<>
void bitonicSort<MYTYPE,12,0>(MYTYPE a[], int low){
    #pragma HLS inline
    #pragma HLS array_partition variable=a complete
    //---
    for(int i=0;i<12;i+=2){
        #pragma HLS unroll
        compAndSwap<MYTYPE,0>(a, low+i, low+i+1); 
    }
    //---
    for(int i=0;i<12;i+=4){
        #pragma HLS unroll
        compAndSwap<MYTYPE,0>(a, low+i+0, low+i+2); 
        compAndSwap<MYTYPE,0>(a, low+i+1, low+i+3); 
    }
    //---
    compAndSwap<MYTYPE,0>(a, low+0, low+4); 
    compAndSwap<MYTYPE,0>(a, low+1, low+5); 
    compAndSwap<MYTYPE,0>(a, low+2, low+6); 
    compAndSwap<MYTYPE,0>(a, low+7, low+11); 
    compAndSwap<MYTYPE,0>(a, low+9, low+10); 
    //---
    compAndSwap<MYTYPE,0>(a, low+1, low+2); 
    compAndSwap<MYTYPE,0>(a, low+6, low+10); 
    compAndSwap<MYTYPE,0>(a, low+5, low+9); 
    compAndSwap<MYTYPE,0>(a, low+4, low+8); 
    compAndSwap<MYTYPE,0>(a, low+3, low+7); 
    //---
    compAndSwap<MYTYPE,0>(a, low+2, low+6); 
    compAndSwap<MYTYPE,0>(a, low+1, low+5); 
    compAndSwap<MYTYPE,0>(a, low+0, low+4); 
    compAndSwap<MYTYPE,0>(a, low+9, low+10); 
    compAndSwap<MYTYPE,0>(a, low+7, low+11); 
    compAndSwap<MYTYPE,0>(a, low+3, low+8); 
    //---
    compAndSwap<MYTYPE,0>(a, low+1, low+4); 
    compAndSwap<MYTYPE,0>(a, low+7, low+10); 
    compAndSwap<MYTYPE,0>(a, low+2, low+3); 
    compAndSwap<MYTYPE,0>(a, low+5, low+6); 
    compAndSwap<MYTYPE,0>(a, low+8, low+9); 
    //---
    compAndSwap<MYTYPE,0>(a, low+2, low+4); 
    compAndSwap<MYTYPE,0>(a, low+3, low+5); 
    compAndSwap<MYTYPE,0>(a, low+6, low+8); 
    compAndSwap<MYTYPE,0>(a, low+7, low+9); 
    //---
    compAndSwap<MYTYPE,0>(a, low+3, low+4); 
    compAndSwap<MYTYPE,0>(a, low+5, low+6); 
    compAndSwap<MYTYPE,0>(a, low+7, low+8); 
}

template<>
void bitonicSort<MYTYPE,12,1>(MYTYPE a[], int low){
    #pragma HLS inline
    #pragma HLS array_partition variable=a complete
    //---
    for(int i=0;i<12;i+=2){
        #pragma HLS unroll
        compAndSwap<MYTYPE,1>(a, low+i, low+i+1); 
    }
    //---
    for(int i=0;i<12;i+=4){
        #pragma HLS unroll
        compAndSwap<MYTYPE,1>(a, low+i+0, low+i+2); 
        compAndSwap<MYTYPE,1>(a, low+i+1, low+i+3); 
    }
    //---
    compAndSwap<MYTYPE,1>(a, low+0, low+4); 
    compAndSwap<MYTYPE,1>(a, low+1, low+5); 
    compAndSwap<MYTYPE,1>(a, low+2, low+6); 
    compAndSwap<MYTYPE,1>(a, low+7, low+11); 
    compAndSwap<MYTYPE,1>(a, low+9, low+10); 
    //---
    compAndSwap<MYTYPE,1>(a, low+1, low+2); 
    compAndSwap<MYTYPE,1>(a, low+6, low+10); 
    compAndSwap<MYTYPE,1>(a, low+5, low+9); 
    compAndSwap<MYTYPE,1>(a, low+4, low+8); 
    compAndSwap<MYTYPE,1>(a, low+3, low+7); 
    //---
    compAndSwap<MYTYPE,1>(a, low+2, low+6); 
    compAndSwap<MYTYPE,1>(a, low+1, low+5); 
    compAndSwap<MYTYPE,1>(a, low+0, low+4); 
    compAndSwap<MYTYPE,1>(a, low+9, low+10); 
    compAndSwap<MYTYPE,1>(a, low+7, low+11); 
    compAndSwap<MYTYPE,1>(a, low+3, low+8); 
    //---
    compAndSwap<MYTYPE,1>(a, low+1, low+4); 
    compAndSwap<MYTYPE,1>(a, low+7, low+10); 
    compAndSwap<MYTYPE,1>(a, low+2, low+3); 
    compAndSwap<MYTYPE,1>(a, low+5, low+6); 
    compAndSwap<MYTYPE,1>(a, low+8, low+9); 
    //---
    compAndSwap<MYTYPE,1>(a, low+2, low+4); 
    compAndSwap<MYTYPE,1>(a, low+3, low+5); 
    compAndSwap<MYTYPE,1>(a, low+6, low+8); 
    compAndSwap<MYTYPE,1>(a, low+7, low+9); 
    //---
    compAndSwap<MYTYPE,1>(a, low+3, low+4); 
    compAndSwap<MYTYPE,1>(a, low+5, low+6); 
    compAndSwap<MYTYPE,1>(a, low+7, low+8); 

}

/********************************************
 *                N=13                       *
 ********************************************/
template<>
void bitonicSort<MYTYPE,13,0>(MYTYPE a[], int low){
    #pragma HLS inline
    #pragma HLS array_partition variable=a complete
    //---
    for(int i=0;i+1<13;i+=2){
        #pragma HLS unroll
        compAndSwap<MYTYPE,0>(a, low+i, low+i+1); 
    }
    //---
    for(int i=0;i+3<13;i+=4){
        #pragma HLS unroll
        compAndSwap<MYTYPE,0>(a, low+i+0, low+i+2); 
        compAndSwap<MYTYPE,0>(a, low+i+1, low+i+3); 
    }
    //---
    compAndSwap<MYTYPE,0>(a, low+0, low+4); 
    compAndSwap<MYTYPE,0>(a, low+1, low+5); 
    compAndSwap<MYTYPE,0>(a, low+2, low+6); 
    compAndSwap<MYTYPE,0>(a, low+3, low+7); 
    compAndSwap<MYTYPE,0>(a, low+8, low+12); 
    //---
    compAndSwap<MYTYPE,0>(a, low+0, low+8); 
    compAndSwap<MYTYPE,0>(a, low+1, low+9); 
    compAndSwap<MYTYPE,0>(a, low+2, low+10); 
    compAndSwap<MYTYPE,0>(a, low+3, low+11); 
    compAndSwap<MYTYPE,0>(a, low+4, low+12); 
    //---
    compAndSwap<MYTYPE,0>(a, low+1, low+2); 
    compAndSwap<MYTYPE,0>(a, low+3, low+12); 
    compAndSwap<MYTYPE,0>(a, low+7, low+11); 
    compAndSwap<MYTYPE,0>(a, low+4, low+8); 
    compAndSwap<MYTYPE,0>(a, low+5, low+10); 
    compAndSwap<MYTYPE,0>(a, low+6, low+9); 
    //---
    compAndSwap<MYTYPE,0>(a, low+1, low+4); 
    compAndSwap<MYTYPE,0>(a, low+2, low+8); 
    compAndSwap<MYTYPE,0>(a, low+6, low+12); 
    compAndSwap<MYTYPE,0>(a, low+3, low+10); 
    compAndSwap<MYTYPE,0>(a, low+5, low+9); 
    //---
    compAndSwap<MYTYPE,0>(a, low+2, low+4); 
    compAndSwap<MYTYPE,0>(a, low+3, low+5); 
    compAndSwap<MYTYPE,0>(a, low+6, low+8); 
    compAndSwap<MYTYPE,0>(a, low+7, low+9); 
    compAndSwap<MYTYPE,0>(a, low+10, low+12); 
    //---
    compAndSwap<MYTYPE,0>(a, low+3, low+6); 
    compAndSwap<MYTYPE,0>(a, low+5, low+8); 
    compAndSwap<MYTYPE,0>(a, low+7, low+10); 
    compAndSwap<MYTYPE,0>(a, low+9, low+12); 
    //---
    compAndSwap<MYTYPE,0>(a, low+3, low+4); 
    compAndSwap<MYTYPE,0>(a, low+5, low+6); 
    compAndSwap<MYTYPE,0>(a, low+7, low+8); 
    compAndSwap<MYTYPE,0>(a, low+9, low+10); 
    compAndSwap<MYTYPE,0>(a, low+11, low+12); 
}

template<>
void bitonicSort<MYTYPE,13,1>(MYTYPE a[], int low){
    #pragma HLS inline
    #pragma HLS array_partition variable=a complete
    //---
    for(int i=0;i+1<13;i+=2){
        #pragma HLS unroll
        compAndSwap<MYTYPE,1>(a, low+i, low+i+1); 
    }
    //---
    for(int i=0;i+3<13;i+=4){
        #pragma HLS unroll
        compAndSwap<MYTYPE,1>(a, low+i+0, low+i+2); 
        compAndSwap<MYTYPE,1>(a, low+i+1, low+i+3); 
    }
    //---
    compAndSwap<MYTYPE,1>(a, low+0, low+4); 
    compAndSwap<MYTYPE,1>(a, low+1, low+5); 
    compAndSwap<MYTYPE,1>(a, low+2, low+6); 
    compAndSwap<MYTYPE,1>(a, low+3, low+7); 
    compAndSwap<MYTYPE,1>(a, low+8, low+12); 
    //---
    compAndSwap<MYTYPE,1>(a, low+0, low+8); 
    compAndSwap<MYTYPE,1>(a, low+1, low+9); 
    compAndSwap<MYTYPE,1>(a, low+2, low+10); 
    compAndSwap<MYTYPE,1>(a, low+3, low+11); 
    compAndSwap<MYTYPE,1>(a, low+4, low+12); 
    //---
    compAndSwap<MYTYPE,1>(a, low+1, low+2); 
    compAndSwap<MYTYPE,1>(a, low+3, low+12); 
    compAndSwap<MYTYPE,1>(a, low+7, low+11); 
    compAndSwap<MYTYPE,1>(a, low+4, low+8); 
    compAndSwap<MYTYPE,1>(a, low+5, low+10); 
    compAndSwap<MYTYPE,1>(a, low+6, low+9); 
    //---
    compAndSwap<MYTYPE,1>(a, low+1, low+4); 
    compAndSwap<MYTYPE,1>(a, low+2, low+8); 
    compAndSwap<MYTYPE,1>(a, low+6, low+12); 
    compAndSwap<MYTYPE,1>(a, low+3, low+10); 
    compAndSwap<MYTYPE,1>(a, low+5, low+9); 
    //---
    compAndSwap<MYTYPE,1>(a, low+2, low+4); 
    compAndSwap<MYTYPE,1>(a, low+3, low+5); 
    compAndSwap<MYTYPE,1>(a, low+6, low+8); 
    compAndSwap<MYTYPE,1>(a, low+7, low+9); 
    compAndSwap<MYTYPE,1>(a, low+10, low+12); 
    //---
    compAndSwap<MYTYPE,1>(a, low+3, low+6); 
    compAndSwap<MYTYPE,1>(a, low+5, low+8); 
    compAndSwap<MYTYPE,1>(a, low+7, low+10); 
    compAndSwap<MYTYPE,1>(a, low+9, low+12); 
    //---
    compAndSwap<MYTYPE,1>(a, low+3, low+4); 
    compAndSwap<MYTYPE,1>(a, low+5, low+6); 
    compAndSwap<MYTYPE,1>(a, low+7, low+8); 
    compAndSwap<MYTYPE,1>(a, low+9, low+10); 
    compAndSwap<MYTYPE,1>(a, low+11, low+12); 
}


#endif


// or assume that the vector is longth enough?
template<typename T,int NP>
void sort_and_crop_ref(unsigned int nIn, unsigned int nOut, T in[], T out[]) 
{  // just an interface
    #pragma HLS inline
    #pragma HLS array_partition variable=in complete
    #pragma HLS array_partition variable=out complete

    //int n=get_pow2_size(nIn);

    //T a[n];
    // #pragma HLS array_partition variable=a complete
    //for (int i=0;i<n;++i) {
    //     #pragma HLS unroll
    //    if (i<nIn) a[i] = in[i]; else a[i] = zero;
    //}

    //bitonicSort(a,0, n, 1); 
    bitonicSort<T,NP,0>(in,0); 

    for(int i=0;i<NP;++i){
        #pragma HLS unroll
        if (i<nOut) out[i] = in[i];
    }
}

#endif

