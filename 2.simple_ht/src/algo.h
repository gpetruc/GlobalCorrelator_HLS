#ifndef ALGO_H
#define ALGO_H

#include "data.h"

// implementation to be synthethised
pt_t algo_main(Particle particles[NPARTICLES]) ;

// alternative implementation to be synthethised
pt_t algo_main_recursive(Particle particles[NPARTICLES]) ;

// reference implementation for validation
pt_t algo_main_ref(Particle particles[NPARTICLES]) ;

#endif
