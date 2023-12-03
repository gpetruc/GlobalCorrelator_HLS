#include "func.h"
#include <hls_stream.h>
#include <cassert>

ap_uint<64>  func_once(ap_uint<16> n) {
  ap_uint<64> ret = 0;
  for (ap_uint<16> i = 0; i <= n; ++i) {
    ret += (i*i*i);
  }
  return ret;
}

void func_many_seq(uint16_t n, const uint16_t *ins, uint64_t *results) {
  for (int i = 0; i < n; ++i) {
    ap_uint<64> ret = func_once(ins[i]);
    results[i] = ret.to_uint64();
  }
}


void func_stream(hls::stream<uint16_t> & in, hls::stream<uint64_t> & out) {
  ap_uint<16> n = in.read();
  ap_uint<64> ret = func_once(n);
  out.write(ret.to_uint64());
}

void func_queue(unsigned int nitems, hls::stream<uint16_t> & in, hls::stream<uint64_t> & out) {
  for (unsigned int i = 0; i < nitems; ++i) {
      ap_uint<16> n = in.read();
      ap_uint<64> ret = func_once(n);
      out.write(ret.to_uint64());
  }
}


#define NSTREAMS 64
void func_many_streams(uint16_t n, const uint16_t *ins, uint64_t *results) {
  #pragma hls interface mode=m_axi port=ins offset=slave bundle=gmem depth=1024
  #pragma hls interface mode=m_axi port=results offset=slave bundle=gmem depth=1024
  hls::stream<uint16_t> stream_in[NSTREAMS]; 
  hls::stream<uint64_t> stream_out[NSTREAMS];
  #pragma hls array_partition variable=stream_in complete
  #pragma hls array_partition variable=stream_out complete
  assert(n % NSTREAMS == 0);
  for (uint16_t i = 0, is = 0; i < n; i += NSTREAMS) {
    for (unsigned int j = 0; j < NSTREAMS; ++j) {
      #pragma hls unroll
      stream_in[j].write(ins[i+j]);
      func_stream(stream_in[j], stream_out[j]);
    }
    for (unsigned int j = 0; j < NSTREAMS; ++j) {
      #pragma hls unroll
      results[i+j] = stream_out[j].read();
    }
  }
}

void func_many_queues(uint16_t n, const uint16_t *ins, uint64_t *results) {
  #pragma hls interface mode=m_axi port=ins offset=slave bundle=gmem depth=1024
  #pragma hls interface mode=m_axi port=results offset=slave bundle=gmem depth=1024
  hls::stream<uint16_t> stream_in[NSTREAMS]; 
  hls::stream<uint64_t> stream_out[NSTREAMS];
  #pragma hls array_partition variable=stream_in complete
  #pragma hls array_partition variable=stream_out complete
  #pragma HLS STREAM variable=stream_in depth=16
  #pragma HLS STREAM variable=stream_out depth=16
  assert(n % NSTREAMS == 0);
  for (uint16_t i = 0, is = 0; i < n; i += NSTREAMS) {
    for (unsigned int j = 0; j < NSTREAMS; ++j) {
      #pragma hls unroll
      stream_in[j].write(ins[i+j]);
    }
  }
  for (unsigned int j = 0; j < NSTREAMS; ++j) {
    #pragma hls unroll
    func_queue(n/NSTREAMS, stream_in[j], stream_out[j]);
  }
  for (uint16_t i = 0, is = 0; i < n; i += NSTREAMS) {
    for (unsigned int j = 0; j < NSTREAMS; ++j) {
      #pragma hls unroll
      results[i+j] = stream_out[j].read();
    }
  }
}