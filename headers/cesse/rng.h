#ifndef CESSE_RNG_H
#define CESSE_RNG_H

#include "cesse/utils.h"
#include "cesse/bool.h"

#include <stddef.h>
#include <stdint.h>

typedef struct Rng Rng; // xoshiro256**

Rng* rng_new(const uint64_t seed, error_code_t* error);
Rng* rng_new_time(error_code_t* error); // use time(NULL) as seed
void rng_delete(Rng** rng, error_code_t* error);
// Raw generator outputs
uint32_t rng_next_u32(Rng* rng, error_code_t* error);
uint64_t rng_next_u64(Rng* rng, error_code_t* error);
double   rng_next_double(Rng* rng, error_code_t* error); /* uniform [0, 1) */
// Distribution
int64_t  dist_uniform_i64(Rng* rng, const int64_t min, const int64_t max, error_code_t* error); /* inclusive both ends */
double   dist_uniform_double(Rng* rng, const double min, const double max, error_code_t* error);  /* [min, max) */
bool     dist_bernoulli(Rng* rng, const double p, error_code_t* error);                     /* true w/ probability p */
double   dist_normal(Rng* rng, const double mean, const double stddev, error_code_t* error);
double   dist_exponential(Rng* rng, const double lambda, error_code_t* error);
void shuffle(void** anchor, const size_t length, Rng* rng, error_code_t* error);

#endif