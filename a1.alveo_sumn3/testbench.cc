#include "src/func.h"
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <chrono>

uint64_t byhand(uint64_t n) {
    uint64_t ref = 0;
    for (uint64_t i = 0; i <= n; ++i) {
        ref += i*i*i;
    }
    return ref;
}
uint64_t closed_formula(uint64_t n) {
    uint64_t x = n*(n+1)/2;
    return x*x;
}
int main(int argc, char **argv) {
    srand(125);
    assert(argc > 2);
    unsigned int ntest = atoi(argv[1]);
    unsigned int ndata = atoi(argv[2]);
    if (ntest == 1) {
        auto t0 = std::chrono::steady_clock::now();
        uint64_t ref = byhand(ndata);
        auto t1 = std::chrono::steady_clock::now();
        auto dt = std::chrono::duration<double>(t1 - t0).count();
        uint64_t ref_math = closed_formula(ndata);
        ap_uint<64> out = func_once(ndata);
        printf("for N = %u, ref = %lu after %.1fus (exact: %lu), firmware %lu, %s\n",
            ndata, ref, dt*1e6, ref_math, out.to_uint64(), ref == out.to_uint64() ? "ok" : "FAIL");
        return (ref == out.to_uint64() ? 0 : 1);
    } else if (ntest == 2) {
        std::vector<uint16_t> ins(ndata);
        std::vector<uint64_t> outs(ndata), refs(ndata), maths(ndata);
        for (int i = 0; i < ndata; ++i) {
            ins[i] = rand() & 0x3FFF;
            maths[i] = closed_formula(ins[i]);
        }
        auto t0 = std::chrono::steady_clock::now();
        for (int i = 0; i < ndata; ++i) {
            refs[i] = byhand(ins[i]);
        }
        auto t1 = std::chrono::steady_clock::now();
        auto dt = std::chrono::duration<double>(t1 - t0).count();
        //func_many_seq(ndata, ins.data(), outs.data());
        func_many_streams(ndata, ins.data(), outs.data());
        for (int i = 0; i < ndata; ++i) {
            if (outs[i] != refs[i] || refs[i] != maths[i]) {
                printf("Mismatch %d, n = %u, math = %lu, ref = %lu, fw = %lu\n",
                    i, unsigned(ins[i]), maths[i], refs[i], outs[i]);
                return 1;
            }
        }
        printf("Run %u tests in %.1fus, result is ok\n", ndata, dt*1e6);
    }
    return 0;
}
