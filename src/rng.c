#include "cesse/rng.h"
#include "cesse/utils.h"
#include "cesse/macros.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stddef.h>
#include <math.h>

#define TWO_PI 6.283185307179586476925286766559

typedef struct Rng {
	uint64_t state[4];
} Rng;

static inline uint64_t rotl(const uint64_t x, const uint64_t k) {
	return (x << k) | (x >> (64 - k));
}

static uint64_t next(Rng* rng) { // internal, NULLs don't get in here.
	uint64_t result = rotl(rng->state[1] * 5, 7) * 9;
	uint64_t t = rng->state[1] << 17;
	rng->state[2] ^= rng->state[0];
	rng->state[3] ^= rng->state[1];
	rng->state[1] ^= rng->state[2];
	rng->state[0] ^= rng->state[3];
	rng->state[2] ^= t;
	rng->state[3] = rotl(rng->state[3], 45);
	return result;
}

static inline uint64_t splitmix64_mixer(const uint64_t seed) {
	uint64_t z = seed;
	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EB;
	z = (z ^ (z >> 31));
	return z;
}

static void seed_generator(Rng* rng, const uint64_t seed) { // internal, NULLs don't get in here.
	uint64_t x = seed;
	for(size_t i = 0; i < 4; ++i) {
		x += 0x9E3779B97F4A7C15;
		rng->state[i] = splitmix64_mixer(x);
	}
}

/* Uniformly random value in [0, range) with no modulo bias.
 *
 * range==0 is a special case meaning "the full 64-bit space" -- used
 * by dist_uniform_i64 when min..max spans every int64_t value, since
 * the true count of values, 2^64, doesn't fit in a uint64_t.
 *
 * A plain `next(rng) % range` is biased whenever range doesn't evenly
 * divide 2^64: the low `2^64 mod range` outputs would each land one
 * raw value more often than the rest. This rejects exactly that many
 * raw values from the top of the range first, so every remaining
 * value's chance of landing in each of the `range` buckets is
 * identical. internal, NULLs don't get in here.
 */
static uint64_t bounded_u64(Rng* rng, uint64_t range) {
	if (range == 0) {
		return next(rng);
	}
	if (range == 1) {
		return 0;
	}
	uint64_t leftover = (UINT64_MAX % range) + 1;
	uint64_t x;
	if (leftover == range) {
		x = next(rng); /* range evenly divides 2^64: nothing to reject */
	} else {
		uint64_t threshold = UINT64_MAX - leftover + 1;
		do {
			x = next(rng);
		} while (x >= threshold);
	}
	return x % range;
}

Rng* rng_new(const uint64_t seed, ErrorCode* error) {
	ASSURE_ERROR_OK(error);
	void* ptr = malloc(sizeof(Rng));
	ERROR_ON_COND(ptr==NULL, error, CESSE_ERR_ALLOC, return NULL);
	Rng* rng = CAST(ptr, Rng*);
	seed_generator(rng, seed);
	return rng;
}

Rng* rng_new_time(ErrorCode* error) {
	ASSURE_ERROR_OK(error);
	return rng_new(CAST(time(NULL), uint64_t), error);
}

void rng_delete(Rng** rng, ErrorCode* error) {
	ASSURE_ERROR_OK(error);
	ERROR_ON_COND(rng==NULL, error, CESSE_ERR_NULLARG, return;);
	ERROR_ON_COND((*rng)==NULL, error, CESSE_ERR_NULLARG, return;);
	free(*rng);
	*rng = NULL;
}

// Raw outputs
uint32_t rng_next_u32(Rng* rng, ErrorCode* error) {
	ASSURE_ERROR_OK(error);
	ERROR_ON_COND(rng==NULL, error, CESSE_ERR_NULLARG, return 0;);
	return (uint32_t)(next(rng) >> 32);
}
uint64_t rng_next_u64(Rng* rng, ErrorCode* error) {
	ASSURE_ERROR_OK(error);
	ERROR_ON_COND(rng==NULL, error, CESSE_ERR_NULLARG, return 0;);
	return next(rng);
}
double   rng_next_double(Rng* rng, ErrorCode* error) { 
	ASSURE_ERROR_OK(error);
	ERROR_ON_COND(rng==NULL, error, CESSE_ERR_NULLARG, return 0;);
	return CAST((next(rng) >> 11), double) * 0x1.0p-53;

}
// distribution
int64_t  dist_uniform_i64(Rng* rng, const int64_t min, const int64_t max, ErrorCode* error) {
	ASSURE_ERROR_OK(error);
	ERROR_ON_COND(rng==NULL, error, CESSE_ERR_NULLARG, return 0;);
	ERROR_ON_COND(min > max, error, CESSE_ERR_BAD_ARG, return 0;);
	/* Number of representable values in [min, max]. Computed in
	 * uint64_t so it wraps to exactly 0 -- bounded_u64's "full range"
	 * case -- when min==INT64_MIN and max==INT64_MAX, since 2^64
	 * itself can't be represented as a uint64_t. */
	uint64_t range = CAST(max, uint64_t) - CAST(min, uint64_t) + 1;
	uint64_t offset = bounded_u64(rng, range);
	return CAST(CAST(min, uint64_t) + offset, int64_t);
}
double   dist_uniform_double(Rng* rng, const double min, const double max, ErrorCode* error) {
	ASSURE_ERROR_OK(error);
	ERROR_ON_COND(rng == NULL, error, CESSE_ERR_NULLARG, return 0.0;);
    ERROR_ON_COND(!(min < max), error, CESSE_ERR_BAD_ARG, return 0.0;);
	return min + (max - min) * rng_next_double(rng, error);
}	
bool     dist_bernoulli(Rng* rng, const double p, ErrorCode* error) {
	ASSURE_ERROR_OK(error);
	ERROR_ON_COND(rng == NULL, error, CESSE_ERR_NULLARG, return false;);
    ERROR_ON_COND(!(p >= 0.0 && p <= 1.0), error, CESSE_ERR_BAD_ARG, return false);
    return rng_next_double(rng, error) < p;
}
double   dist_normal(Rng* rng, const double mean, const double stddev, ErrorCode* error) {
	ASSURE_ERROR_OK(error);
	ERROR_ON_COND(rng==NULL, error, CESSE_ERR_NULLARG, return 0.0;);
	ERROR_ON_COND(!(stddev >= 0.0), error, CESSE_ERR_BAD_ARG, return 0.0;);
	/* Box-Muller. u1==0.0 would make log(u1) = -infinity; rng_next_double's
	 * range is [0,1), so 0.0 is reachable (if astronomically unlikely) -- redraw rather
	 * than let a non-finite value escape. */
	double u1;
	do {
		u1 = rng_next_double(rng, error);
	} while (u1 == 0.0);
	double u2 = rng_next_double(rng, error);
	double z0 = sqrt(-2.0 * log(u1)) * cos(TWO_PI * u2);
	return mean + z0 * stddev;
}
double   dist_exponential(Rng* rng, const double lambda, ErrorCode* error) {
	ASSURE_ERROR_OK(error);
	ERROR_ON_COND(rng==NULL, error, CESSE_ERR_NULLARG, return 0.0;);
	ERROR_ON_COND(!(lambda > 0.0), error, CESSE_ERR_BAD_ARG, return 0.0;);
	/* Inverse-CDF sampling: same u==0.0 concern as dist_normal. */
	double u;
	do {
		u = rng_next_double(rng, error);
	} while (u == 0.0);
	return -log(u) / lambda;
}
void shuffle(void** anchor, const size_t length, Rng* rng, ErrorCode* error) {
	ASSURE_ERROR_OK(error);
	ERROR_ON_COND(anchor==NULL, error, CESSE_ERR_NULLARG, return;);
	ERROR_ON_COND(rng==NULL, error, CESSE_ERR_NULLARG, return;);
	if (length < 2) { return; }
	/* Fisher-Yates: each element i, from the end backward, is swapped
	 * with a uniformly random element from [0, i] (itself included).
	 * This produces every permutation with equal probability. */
	for (size_t i = length - 1; i > 0; i--) {
		uint64_t j = bounded_u64(rng, i + 1);
		void* tmp = anchor[i];
		anchor[i] = anchor[j];
		anchor[j] = tmp;
	}
}