#include "src/func.h"
#include <cstdio>
#include <cstdlib>
#include <vector>

int main(int argc, char **argv) {
    srand(125);
    assert(argc > 2);
    unsigned int ntest = atoi(argv[1]);
    unsigned int ndata = atoi(argv[2]);
    printf("Will run %u tests with max %u data\n", ntest, ndata);
    assert(ndata > 5 && ntest > 0);
    std::vector<ap_int<16>> a(ndata), b(ndata);
    std::vector<ap_int<32>> c(ndata), c_ref(ndata);
    for (unsigned int itest = 0; itest < ntest; ++itest) {
        unsigned int n = (rand() % (ndata-5)) + 5;
        assert(n <= ndata);
        printf("Test %u, with %u data\n", itest, n);
        for (unsigned int i = 0; i < n; ++i) {
            a[i] = (rand() & 0xFFF);
            b[i] = (rand() & 0xFFF);
            c_ref[i] = a[i]*b[i];
        }
        func(n, a.data(), b.data(), c.data());
        for (unsigned int i = 0; i < n; ++i) {
            printf("%3u  %7d  %7d  %7d  %7d\n", i, a[i].to_int(), b[i].to_int(), c[i].to_int(), c_ref[i].to_int());
            if (c[i] != c_ref[i]) return 1;
        }
    }
}
