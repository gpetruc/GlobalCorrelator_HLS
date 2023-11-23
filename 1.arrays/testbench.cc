#include "src/func.h"
#include <cstdio>
#include <cstdlib>

int main() {
    srand(125);
    ap_int<16> a[NDATA], b[NDATA]; 
    ap_int<24> c_ref, c_fw;
    for (unsigned int itest = 0, ntest = 20; itest <= ntest; ++itest) {
        // create some input data
        c_ref = 0;
        for (unsigned int i = 0; i < NDATA; ++i) {
            a[i] = ap_int<16>(rand() & 0xFFFF);
            b[i] = ap_int<16>(rand() & 0xFFFF);
            ap_int<24> sum = (a[i] * b[i]) >> 8;
            c_ref += sum;
        }

        // call the function
        c_fw = mul_add_basic(a,b);
        printf("Simple test: %d (%d expected)\n", c_fw.to_int(), c_ref.to_int());
        if (c_fw != c_ref) return 1;

        c_fw = mul_add_pipelined(a,b);
        printf("            : %d (%d expected)\n", c_fw.to_int(), c_ref.to_int());
        if (c_fw != c_ref) return 1;

        c_fw = mul_add_partitioned(a,b);
        printf("            : %d (%d expected)\n", c_fw.to_int(), c_ref.to_int());
        if (c_fw != c_ref) return 1;

        printf("\n");
    }
    return 0;
}
