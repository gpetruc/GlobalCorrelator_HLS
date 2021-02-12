#ifndef FIRMWARE_PFALGO_COMMON_H
#define FIRMWARE_PFALGO_COMMON_H

#include "../../dataformats/layer1_objs.h"
#include "../../dataformats/layer1_multiplicities.h"
#include "../../dataformats/pf.h"
#include "pfalgo_types.h"

namespace l1ct {

pt_t ptErr(pt_t pt, eta_t eta, glbeta_t eta0) ;

} // namespace


#ifndef CMSSW_GIT_HASH

#define PFALGO_DR2MAX_TK_MU 2101
#define PFALGO_DR2MAX_TK_CALO 525

#if defined(REG_Barrel)
#define   PTERR_BINS 3
#define   PTERR_EDGES { 0.700,  1.200,  1.600 }
#define   PTERR_OFFS  { 2.909,  2.864,  0.294 }
#define   PTERR_SCALE { 0.118,  0.130,  0.442 }  
#elif defined(REG_HGCal)
#define   PTERR_BINS 5
#define   PTERR_EDGES { 1.700,  1.900,  2.200,  2.500,  2.800 }
#define   PTERR_OFFS  { 1.793,  1.827,  2.363,  2.538,  2.812 }
#define   PTERR_SCALE { 0.138,  0.137,  0.124,  0.115,  0.106 }
#else // this is dummy
#define   PTERR_BINS 1
#define   PTERR_EDGES { 6.0 }
#define   PTERR_OFFS  { 7.0 }
#define   PTERR_SCALE { 0.3125 }
#endif // regions for PTERR

#endif // CMSSW_GIT_HASH

#endif
