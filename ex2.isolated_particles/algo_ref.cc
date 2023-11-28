#include "src/algo.h"

int find_seed_ref(unsigned int npuppi, const Puppi puppi[NPUPPI_MAX], const bool masked[NPUPPI_MAX]) {
    int iseed = -1;
    for (int i = 0; i < npuppi; ++i) {
      if (!masked[i] && (iseed == -1 || puppi[iseed].hwPt < puppi[i].hwPt)) {
        iseed = i;
      }
    }
    return iseed;
}
void compute_isolated_ref(unsigned int npuppi, const Puppi puppi[NPUPPI_MAX],
                          Puppi out_ref[NISO_MAX],
                          Puppi::pt_t out_absiso_ref[NISO_MAX],
                          bool verbose) {
  unsigned niso_ref = 0;
  bool masked[NPUPPI_MAX];
  auto dr2_max = drToHwDr2(0.4), dr2_veto = drToHwDr2(0.1);
  for (int i = 0; i < npuppi; ++i)
    masked[i] = (puppi[i].hwID < 2);
  for (int iter = 0; iter < NISO_MAX; ++iter) {
    int iseed = find_seed_ref(npuppi, puppi, masked);
    if (iseed == -1)
      break;
    out_ref[niso_ref] = puppi[iseed];
    out_absiso_ref[niso_ref] = 0;
    for (int i = 0; i < npuppi; ++i) {
      auto dr2 = deltaR2(puppi[iseed], puppi[i]);
      if (dr2 < dr2_max) {
        masked[i] = true;
        if (dr2 > dr2_veto)
          out_absiso_ref[niso_ref] += puppi[i].hwPt;
      }
    }
    if (verbose) {
      printf("Seed %3u pT %8.3f eta %+6.3f phi %+6.3f pid %1u: abs iso %8.3f\n",
             iseed, puppi[iseed].floatPt(), puppi[iseed].floatEta(),
             puppi[iseed].floatPhi(), puppi[iseed].hwID.to_uint(),
             Puppi::floatPt(out_absiso_ref[niso_ref]));
    }
    if (out_absiso_ref[niso_ref] < puppi[iseed].hwPt) {
      niso_ref++;
    }
  }
  for (int i = niso_ref; i < NISO_MAX; ++i) {
    out_ref[i].clear();
    out_absiso_ref[i] = 0;
  }
}