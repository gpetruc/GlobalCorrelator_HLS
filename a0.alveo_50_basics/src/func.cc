#include "func.h"

void func(unsigned int N, const ap_int<16> *x, const ap_int<16>* y, ap_int<32> *z){
  #pragma hls interface mode=m_axi port=x offset=slave bundle=gmem
  #pragma hls interface mode=m_axi port=y offset=slave bundle=gmem
  #pragma hls interface mode=m_axi port=z offset=slave bundle=gmem
  for(int i = 0; i < N; i++){
    #pragma hls pipeline
    z[i] = x[i] * y[i];
  }
}