#ifndef CESSE_RNG_H
#define CESSE_RNG_H

#include "cesse/utils.h"
#include "cesse/bool.h"

#include <stddef.h>
#include <stdint.h>

typedef struct Rng Rng; // xoshiro256**

Rng* rng_new(const uint64_t seed, ErrorCode* error);
Rng* rng_new_time(ErrorCode* error); // use time(NULL) as seed
void rng_delete(Rng** rng, ErrorCode* error);
// Raw generator outputs
uint32_t rng_next_u32(Rng* rng, ErrorCode* error);
uint64_t rng_next_u64(Rng* rng, ErrorCode* error);
double   rng_next_double(Rng* rng, ErrorCode* error); /* uniform [0, 1) */
// Distribution
int64_t  dist_uniform_i64(Rng* rng, const int64_t min, const int64_t max, ErrorCode* error); /* inclusive both ends */
double   dist_uniform_double(Rng* rng, const double min, const double max, ErrorCode* error);  /* [min, max) */
bool     dist_bernoulli(Rng* rng, const double p, ErrorCode* error);                     /* true w/ probability p */
double   dist_normal(Rng* rng, const double mean, const double stddev, ErrorCode* error);
double   dist_exponential(Rng* rng, const double lambda, ErrorCode* error);
void shuffle(void** anchor, const size_t length, Rng* rng, ErrorCode* error);

#endif