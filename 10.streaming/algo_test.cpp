#include <cstdio>
#include <cstdlib>
#include <cassert>
#include "src/algo.h"

#define NTEST 50

int main() {
    srand(42);
    num_t save[NITEMS];
    FILE *patterns_in  = fopen("input.txt","w");
    FILE *patterns_out = fopen("expected-output.txt","w");
    for (int test = 0; test < NTEST; ++test) {
        hls::stream<num_t> input_hw, input_ref;
        num_t threshold = abs(rand()) % 1024;
        for (int i = 0; i < NITEMS; ++i) {
            num_t val = rand() % 1024;
            input_hw.write(val);
            input_ref.write(val);
            save[i] = val;
            fprintf(patterns_in, "%d %d %d %d\n", test*NITEMS+i, i, int(threshold), int(val));
        }
        // run the algorithm
        num_t hw  = algo_main(threshold, input_hw);
        num_t ref = algo_main_ref(threshold, input_ref);
        assert(input_ref.empty());
        for (int i = 1; i <= NITEMS; ++i) {
            fprintf(patterns_out, "%5d %d %5d\n", test*NITEMS+i, i == NITEMS, i == NITEMS ? int(ref) : 0);
        }

        // check the output
        if (hw != ref || !input_ref.empty()) {
            printf("Error in test %d\n", test);
            for (int i = 0; i < NITEMS; ++i) {
                printf("   item %2d   value  %5d   threshold %5d\n", i, int(save[i]), int(threshold));
            }
            printf("  Algo result     :  %6d\n", int(hw));
            printf("  Reference result:  %6d\n", int(ref));
            printf("\n");
            break;
        }
    }
    fclose(patterns_in);
    fclose(patterns_out);
    printf("Passed all %d tests\n", NTEST);
    return 0;
}
