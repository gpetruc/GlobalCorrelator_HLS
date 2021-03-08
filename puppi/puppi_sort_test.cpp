#include <cstdlib>

#include "firmware/puppi_sort_hybrid.h"
#include "../common/bitonic_hybrid_sort_ref.h"
#include "../utils/test_utils.h"
#include "../utils/pattern_serializer.h"

#define SORT_validate
#define NTEST 200
int main(int argc, char **argv) {

    l1ct::PuppiObj presort[NTRACK+NALLNEUTRALS];
    l1ct::PuppiObj sorted[NTRACK+NALLNEUTRALS];
    l1ct::PuppiObj sorted2[NTRACK+NALLNEUTRALS];
    PackedPuppiObj outpresort[NTRACK+NALLNEUTRALS];
    PackedPuppiObj outsorted[NPUPPIFINALSORTED];
    srand(123456);

    for (int itest = 0; itest < NTEST; ++itest) {
        for(int i=0;i<NTRACK+NALLNEUTRALS;++i){
            l1ct::PuppiObj x;
            if (itest % 2 == 0) {
                x.hwPt=rand()%2000; //14bit
            } else {
                // intentionally use narrow range to increase chances of ties
                x.hwPt=(rand()%6+1)*(rand()%6); 
            }
            x.hwEta=rand()%1000; //12bit
            x.hwPhi=rand()%1000; //11bit
            x.hwId=l1ct::ParticleID::PID(rand()%8); //3bit
            x.hwData=rand()%1000; // 12bit

            presort[i] = x;
            outpresort[i] = x.pack();
        }

        #ifdef SORT_hybrid
        sort_puppi_cands_hybrid(outpresort, outsorted);
        hybrid_bitonic_sort_and_crop_ref(NTRACK+NALLNEUTRALS, NTRACK+NALLNEUTRALS, presort, sorted);
        #else
        sort_puppi_cands_bitonic(outpresort, outsorted);
        hybrid_bitonic_sort_and_crop_ref(NTRACK+NALLNEUTRALS, NTRACK+NALLNEUTRALS, presort, sorted, false);
        #endif

        int nties = 0;
        for (int i=1;i<NTRACK+NALLNEUTRALS;++i) if (sorted[i+1].hwPt == sorted[i].hwPt) nties++;

        l1ct::PuppiObj prev = l1ct::PuppiObj::unpack(outsorted[0]);
        bool match = true;
        #ifdef SORT_validate
        match = puppi_equals(sorted[0], prev, "cand", 0);
        #endif
        for(int i=1;i<NPUPPIFINALSORTED;++i) {
            l1ct::PuppiObj next = l1ct::PuppiObj::unpack(outsorted[i]);
            if (next > prev) {
                printf("Error on test %i, pt[%d] = %.2f > pt[%d] = %.2f\n", itest, i, next.floatPt(), i-1, prev.floatPt());
                return 1;
            }
            #ifdef SORT_validate
            if (!puppi_equals(sorted[i], next, "cand", i)) match = false;
            #endif
            next = prev;
        }


        printf("Passed test %d (%d ties), matches with ref? %1d\n", itest, nties, int(match));
        #ifdef SORT_validate
        if (!match) {
            l1ct::PuppiObj hls_sorted[NPUPPIFINALSORTED];
            for(int i=0;i<NPUPPIFINALSORTED;++i) hls_sorted[i] = l1ct::PuppiObj::unpack(outsorted[i]);
            HumanReadablePatternSerializer dumper("-");
            dumper.dump_puppi(NPUPPIFINALSORTED, "ref", sorted);
            dumper.dump_puppi(NPUPPIFINALSORTED, "hls", hls_sorted);
            break;
        }
        #endif
    }

    return 0;
}
