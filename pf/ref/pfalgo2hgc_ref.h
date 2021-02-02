#ifndef PFALGO2HGC_REF_H
#define PFALGO2HGC_REF_H

#include "pfalgo_common_ref.h"

namespace l1ct {

void pfalgo2hgc_ref_set_debug(int debug) ;

void pfalgo2hgc_ref(const pfalgo_config &cfg, const PFRegion & region, const HadCaloObj calo[/*cfg.nCALO*/], const TkObj track[/*cfg.nTRACK*/], const MuObj mu[/*cfg.nMU*/], PFChargedObj outch[/*cfg.nTRACK*/], PFNeutralObj outne[/*cfg.nSELCALO*/], PFChargedObj outmu[/*cfg.nMU*/]) ;

} // namespace

#endif
