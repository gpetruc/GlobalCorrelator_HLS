#ifndef BITONIC_HYBRID_H
#define BITONIC_HYBRID_H

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

/*    T -> Type template specialization will be constructed against
 *    USE_HYBRID -> use low input template specialization optimized for depth. Impl. n=2,3,4,5,6,7, 12,13 (TODO known<=16)
 */   


namespace hybridBitonicSort {
        static constexpr int PowerOf2LessEqualThan(int n){ //  to be called for n>0: 
            return (n && !(n&(n-1))) ? n : PowerOf2LessEqualThan(n-1);
        }

        static constexpr int PowerOf2LessThan(int n){ //  to be called for n>1
            return PowerOf2LessEqualThan(n-1);
        }

        template<typename T>
        static inline T max(T a, T b)  
        {  
            #pragma HLS inline
            return (a < b)? b : a;  
        }  

        template<typename T>
        static inline T min(T a, T b)  
        {  
            #pragma HLS inline
            return (a < b)? a : b;  
        }  

        template<typename T>
        static void myswap(T& a, T&b)
        {
            #pragma HLS inline
                T c = a;
                a=b;
                b=c;
                return;
        }

        template<typename T, int dir>
        static void compAndSwap(T a[], int i, int j) 
        { 
            #pragma HLS inline
            #pragma HLS array_partition variable=a complete
            if (dir){ if (a[j]<a[i]) myswap(a[i],a[j]);} // (1)
            else { if (a[i]<a[j]) myswap(a[i],a[j]);} // (2)
        } 

        /*It recursively sorts a bitonic sequence in ascending order, 
          if dir = 1, and in descending order otherwise (means dir=0). 
          The sequence to be sorted starts at index position low, 
          the parameter N is the number of elements to be sorted.*/
        template<typename T, int N, int dir>
        struct bitonicMerger {
            inline static void run(T a[], int low) { 
                #pragma HLS inline
                #pragma HLS array_partition variable=a complete
                static constexpr int k = PowerOf2LessThan(N);
                static constexpr int k2 = N-k;
                if (N>1) 
                { 
                    for (int i=low; i<low+k; i++){  
                        #pragma HLS unroll
                        if (i+k<low+N) compAndSwap<T,dir>(a, i, i+k); 
                    }

                    bitonicMerger<T,k,dir>::run(a, low); 
                    bitonicMerger<T,k2,dir>::run(a, low+k); 
                } 
            }
        };


        template<typename T, int dir>
        struct bitonicMerger<T,1,dir> {
            inline static void run(T a[], int low) { 
                #pragma HLS inline
                return;
            }
        };

        /* This function first produces a bitonic sequence by recursively 
            sorting its two halves in opposite sorting orders, and then 
            calls bitonicMerge to make them in the same order */
        template <typename T, int N, int dir, bool hybrid=false>
        struct bitonicSorter {
            inline static void run(T a[],int low) { 
                #pragma HLS inline
                #pragma HLS array_partition variable=a complete
                static constexpr int lowerSize= N/2;
                static constexpr int upperSize= N-N/2;
                static constexpr int notDir = not dir;

                if (N>1) 
                { 
                    // sort in ascending order since dir here is 1 
                    //bitonicSort(a, low, k, 1);  // dir, ?
                    bitonicSorter<T,lowerSize,notDir,hybrid>::run(a, low);

                    // sort in descending order since dir here is 0 
                    bitonicSorter<T,upperSize,dir,hybrid>::run(a, low+lowerSize);  
              
                    // Will merge wole sequence in ascending order 
                    // since dir=1. 
                    bitonicMerger<T,N,dir>::run(a,low); 
                } 
            } 
        };

        /********************************************
         *                N=1                       *
         ********************************************/
        template<typename T, int dir, bool hybrid>
        struct bitonicSorter<T, 1, dir, hybrid> {
            inline static void run(T a[],int low) { 
                #pragma HLS inline
                return ;
            } 
        };

        template<typename T, int dir, bool hybrid>
        struct bitonicSorter<T, 2, dir, hybrid> {
            inline static void run(T a[], int low){
                #pragma HLS inline
                #pragma HLS array_partition variable=a complete
                compAndSwap<T,dir>(a, low+0, low+1); 
            }
        };

        template<typename T, int dir>
        struct bitonicSorter<T, 3, dir, true> {
            inline static void run(T a[], int low){
                #pragma HLS inline
                #pragma HLS array_partition variable=a complete
                compAndSwap<T,dir>(a, low+0, low+1); 
                compAndSwap<T,dir>(a, low+1, low+2); 
                //---
                compAndSwap<T,dir>(a, low+0, low+1); 
            }
        };

        template<typename T, int dir>
        struct bitonicSorter<T, 4, dir, true> {
            inline static void run(T a[], int low){
                #pragma HLS inline
                #pragma HLS array_partition variable=a complete
                compAndSwap<T,dir>(a, low+0, low+1); 
                compAndSwap<T,dir>(a, low+2, low+3); 
                //---
                compAndSwap<T,dir>(a, low+0, low+2); 
                compAndSwap<T,dir>(a, low+1, low+3); 
                //---
                compAndSwap<T,dir>(a, low+1, low+2); 
            }
        };

        template<typename T, int dir>
        struct bitonicSorter<T, 5, dir, true> {
            inline static void run(T a[], int low){
                #pragma HLS inline
                #pragma HLS array_partition variable=a complete
                compAndSwap<T,dir>(a, low+0, low+1); 
                compAndSwap<T,dir>(a, low+1, low+2); 
                //---
                compAndSwap<T,dir>(a, low+1, low+3); 
                compAndSwap<T,dir>(a, low+2, low+4); 
                //---
                compAndSwap<T,dir>(a, low+0, low+2); 
                compAndSwap<T,dir>(a, low+1, low+4); 
                //--
                compAndSwap<T,dir>(a, low+1, low+2); 
                compAndSwap<T,dir>(a, low+3, low+4); 
                //--
                compAndSwap<T,dir>(a, low+2, low+3); 
            }
        };

        template<typename T, int dir>
        struct bitonicSorter<T, 6, dir, true> {
            inline static void run(T a[], int low){
                #pragma HLS inline
                #pragma HLS array_partition variable=a complete
                //---
                compAndSwap<T,dir>(a, low+0, low+3); 
                compAndSwap<T,dir>(a, low+1, low+2); 
                //---
                compAndSwap<T,dir>(a, low+0, low+1); 
                compAndSwap<T,dir>(a, low+2, low+3); 
                compAndSwap<T,dir>(a, low+4, low+5); 
                //---
                compAndSwap<T,dir>(a, low+0, low+3); 
                compAndSwap<T,dir>(a, low+1, low+4); 
                compAndSwap<T,dir>(a, low+2, low+5); 
                //---
                compAndSwap<T,dir>(a, low+0, low+1); 
                compAndSwap<T,dir>(a, low+2, low+4); 
                compAndSwap<T,dir>(a, low+3, low+5); 
                //---
                compAndSwap<T,dir>(a, low+1, low+2); 
                compAndSwap<T,dir>(a, low+3, low+4); 
            }
        };

        template<typename T, int dir>
        struct bitonicSorter<T, 12, dir, true> {
            inline static void run(T a[], int low){
                #pragma HLS inline
                #pragma HLS array_partition variable=a complete
                for(int i=0;i<12;i+=2){
                    #pragma HLS unroll
                    compAndSwap<T,dir>(a, low+i, low+i+1); 
                }
                //---
                for(int i=0;i<12;i+=4){
                    #pragma HLS unroll
                    compAndSwap<T,dir>(a, low+i+0, low+i+2); 
                    compAndSwap<T,dir>(a, low+i+1, low+i+3); 
                }
                //---
                compAndSwap<T,dir>(a, low+0, low+4); 
                compAndSwap<T,dir>(a, low+1, low+5); 
                compAndSwap<T,dir>(a, low+2, low+6); 
                compAndSwap<T,dir>(a, low+7, low+11); 
                compAndSwap<T,dir>(a, low+9, low+10); 
                //---
                compAndSwap<T,dir>(a, low+1, low+2); 
                compAndSwap<T,dir>(a, low+6, low+10); 
                compAndSwap<T,dir>(a, low+5, low+9); 
                compAndSwap<T,dir>(a, low+4, low+8); 
                compAndSwap<T,dir>(a, low+3, low+7); 
                //---
                compAndSwap<T,dir>(a, low+2, low+6); 
                compAndSwap<T,dir>(a, low+1, low+5); 
                compAndSwap<T,dir>(a, low+0, low+4); 
                compAndSwap<T,dir>(a, low+9, low+10); 
                compAndSwap<T,dir>(a, low+7, low+11); 
                compAndSwap<T,dir>(a, low+3, low+8); 
                //---
                compAndSwap<T,dir>(a, low+1, low+4); 
                compAndSwap<T,dir>(a, low+7, low+10); 
                compAndSwap<T,dir>(a, low+2, low+3); 
                compAndSwap<T,dir>(a, low+5, low+6); 
                compAndSwap<T,dir>(a, low+8, low+9); 
                //---
                compAndSwap<T,dir>(a, low+2, low+4); 
                compAndSwap<T,dir>(a, low+3, low+5); 
                compAndSwap<T,dir>(a, low+6, low+8); 
                compAndSwap<T,dir>(a, low+7, low+9); 
                //---
                compAndSwap<T,dir>(a, low+3, low+4); 
                compAndSwap<T,dir>(a, low+5, low+6); 
                compAndSwap<T,dir>(a, low+7, low+8); 
            }
        };

        template<typename T, int dir>
        struct bitonicSorter<T, 13, dir, true> {
            inline static void run(T a[], int low){
                #pragma HLS inline
                #pragma HLS array_partition variable=a complete
                for(int i=0;i+1<13;i+=2){
                    #pragma HLS unroll
                    compAndSwap<T,dir>(a, low+i, low+i+1); 
                }
                //---
                for(int i=0;i+3<13;i+=4){
                    #pragma HLS unroll
                    compAndSwap<T,dir>(a, low+i+0, low+i+2); 
                    compAndSwap<T,dir>(a, low+i+1, low+i+3); 
                }
                //---
                compAndSwap<T,dir>(a, low+0, low+4); 
                compAndSwap<T,dir>(a, low+1, low+5); 
                compAndSwap<T,dir>(a, low+2, low+6); 
                compAndSwap<T,dir>(a, low+3, low+7); 
                compAndSwap<T,dir>(a, low+8, low+12); 
                //---
                compAndSwap<T,dir>(a, low+0, low+8); 
                compAndSwap<T,dir>(a, low+1, low+9); 
                compAndSwap<T,dir>(a, low+2, low+10); 
                compAndSwap<T,dir>(a, low+3, low+11); 
                compAndSwap<T,dir>(a, low+4, low+12); 
                //---
                compAndSwap<T,dir>(a, low+1, low+2); 
                compAndSwap<T,dir>(a, low+3, low+12); 
                compAndSwap<T,dir>(a, low+7, low+11); 
                compAndSwap<T,dir>(a, low+4, low+8); 
                compAndSwap<T,dir>(a, low+5, low+10); 
                compAndSwap<T,dir>(a, low+6, low+9); 
                //---
                compAndSwap<T,dir>(a, low+1, low+4); 
                compAndSwap<T,dir>(a, low+2, low+8); 
                compAndSwap<T,dir>(a, low+6, low+12); 
                compAndSwap<T,dir>(a, low+3, low+10); 
                compAndSwap<T,dir>(a, low+5, low+9); 
                //---
                compAndSwap<T,dir>(a, low+2, low+4); 
                compAndSwap<T,dir>(a, low+3, low+5); 
                compAndSwap<T,dir>(a, low+6, low+8); 
                compAndSwap<T,dir>(a, low+7, low+9); 
                compAndSwap<T,dir>(a, low+10, low+12); 
                //---
                compAndSwap<T,dir>(a, low+3, low+6); 
                compAndSwap<T,dir>(a, low+5, low+8); 
                compAndSwap<T,dir>(a, low+7, low+10); 
                compAndSwap<T,dir>(a, low+9, low+12); 
                //---
                compAndSwap<T,dir>(a, low+3, low+4); 
                compAndSwap<T,dir>(a, low+5, low+6); 
                compAndSwap<T,dir>(a, low+7, low+8); 
                compAndSwap<T,dir>(a, low+9, low+10); 
                compAndSwap<T,dir>(a, low+11, low+12); 
            }
    };

    template<int NIN,int NOUT,bool hybrid=false,int dir=0, typename T>
    void sort(T in[], T out[]) 
    {  // just an interface
        #pragma HLS inline
        #pragma HLS array_partition variable=in complete
        #pragma HLS array_partition variable=out complete

        bitonicSorter<T,NIN,dir,hybrid>::run(in,0); 

        for(int i=0;i<NOUT;++i){
            out[i] = in[i];
        }
    }

} // namespace

#endif

