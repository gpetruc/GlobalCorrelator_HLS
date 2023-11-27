#include <ap_int.h>
#include <cstdint>

ap_uint<64>  func_once(ap_uint<16> n) ;
void func_many_seq(uint16_t n, const uint16_t *ins, uint64_t *results) ;
void func_many_streams(uint16_t n, const uint16_t *ins, uint64_t *results) ;