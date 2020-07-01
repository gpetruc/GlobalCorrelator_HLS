#include "src/multitap_sr.h"
#include <cstdio>
#include <cstdlib>

int main() {
    srand(125);
    const int NDATA = NTAPS+10;
    ap_uint<16> a[NDATA], ret_tap[NTAPS], ref_tap[NTAPS];
    for (unsigned int itest = 0, ntest = 20; itest <= ntest; ++itest) {
        // create some input data
        for (unsigned int i = 0; i < NDATA; ++i) {
            a[i] = ap_uint<16>(rand() & 0xFFFF);
        }
        if (itest <= 3) {
            printf("input  A[]:  ");
            for (unsigned int i = 0; i < NDATA; ++i) printf(" #%02u = %6d ; ", i, int(a[i]));
            printf("\n");
        }

        for (unsigned int iclock = 0; iclock <  NDATA; ++iclock) {
            bool ok = true, print = false;
            bool ret_flag = multitap_sr_push_simple(iclock == 0, a[iclock], ret_tap);
            if (ret_flag != (iclock >= NTAPS-1)) {
                printf("ERROR: multitap_sr_push_simple: test %2u clock %2u: ret_flag = %d not expected\n", itest, iclock, int(ret_flag)); 
                ok = false;
            }
            if (ret_flag) {
                for (unsigned int icheck = 0; icheck < NTAPS; ++icheck) {
                    if (ret_tap[icheck] != a[iclock+icheck-NTAPS+1]) {
                        printf("ERROR: multitap_sr_push_simple: test %2u clock %2u: ret_tap[%2d] = %6d while %6d expected\n", 
                                    itest, iclock, icheck, int(ret_tap[icheck]), int(a[iclock+icheck-NTAPS+1]));
                        ok = false;
                    }
                }
            }
            if (print || !ok) {
                printf("input  A[]:  ");
                for (unsigned int i = 0; i < NDATA; ++i) printf(" #%02u = %6d ; ", i, int(a[i]));
                printf("\n");
                printf("output taps: ");
                for (unsigned int i = 0; i < NTAPS; ++i) printf(" #%02u = %6d ; ", i, int(ret_tap[i]));
                printf("\n");
                if (!ok) return 1;
            }

            print = (itest <= 3);
            ret_flag = sorting_multitap_sr_push_simple(iclock == 0, a[iclock], ret_tap);
            bool ref_flag = sorting_multitap_sr_push_simple_ref(iclock == 0, a[iclock], ref_tap);
            if (ret_flag != ref_flag) {
                printf("ERROR: sorting_multitap_sr_push_simple: test %2u clock %2u: ret_flag = %d not expected\n", itest, iclock, int(ret_flag)); 
                ok = false;
            }
            if (ret_flag) {
                for (unsigned int icheck = 0; icheck < NTAPS; ++icheck) {
                    if (ret_tap[icheck] != ref_tap[icheck]) {
                        printf("ERROR: sorting multitap_sr_push_simple: test %2u clock %2u: ret_tap[%2d] = %6d while %6d expected\n", 
                                itest, iclock, icheck, int(ret_tap[icheck]), int(ref_tap[icheck]));
                        ok = false;
                    }
                }
            }
            if (print ||!ok) {
                printf("output  @%2u: ", iclock);
                for (unsigned int i = 0; i < NTAPS; ++i) printf(" #%02u = %6d ; ", i, int(ret_tap[i]));
                printf("  flag %d\n", int(ret_flag));
                printf("expect  @%2u: ", iclock);
                for (unsigned int i = 0; i < NTAPS; ++i) printf(" #%02u = %6d ; ", i, int(ref_tap[i]));
                printf("  flag %d\n", int(ref_flag));
                if (!ok) return 1;
            }

        }

        printf("test %u passed\n", itest);
    }
}
