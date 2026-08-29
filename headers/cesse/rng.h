#ifndef CESSE_RNG_H
#define CESSE_RNG_H

/**
* @file rng.h
* @author Jakub Grzana
* @date August 2026
* @brief pseudorandom number generator + common distributions
*/

#include "cesse/utils.h"
#include "cesse/bool.h"

#include <stddef.h>
#include <stdint.h>

/**
* Xoshiro256** generator type.
*/
typedef struct Rng Rng; // xoshiro256**

/**
* Create a new Rng seeded deterministically from seed (via splitmix64 to
* fill the generator's internal state). The same seed always reproduces
* the same sequence of outputs.
*
* Time complexity: O(1).
* \param seed The seed value. Any uint64_t value is accepted.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_ALLOC.
* \return Pointer to the created Rng, or NULL if an error occurred.
*/
Rng* rng_new(const uint64_t seed, ErrorCode* error);

/**
* Create a new Rng seeded from the current time (time(NULL)), for a
* non-reproducible sequence. Equivalent to rng_new((uint64_t)time(NULL), error).
*
* Time complexity: O(1).
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_ALLOC.
* \return Pointer to the created Rng, or NULL if an error occurred.
*/
Rng* rng_new_time(ErrorCode* error); // use time(NULL) as seed

/**
* Delete an Rng and free it.
*
* Time complexity: O(1).
* \param rng Pointer-to-pointer of the Rng. Once freed, the pointer is
*        set to NULL (hence the double pointer). Must NOT be NULL. *rng==NULL is a no-op.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG.
*/
void rng_delete(Rng** rng, ErrorCode* error);

// Raw generator outputs

/**
* Draw a raw, uniformly random 32-bit value (the top 32 bits of one
* generator step).
*
* Time complexity: O(1).
* \param rng The generator to draw from. Must not be NULL.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG.
* \return A value uniformly distributed over the full uint32_t range, or 0 if an error occurred.
*/
uint32_t rng_next_u32(Rng* rng, ErrorCode* error);

/**
* Draw a raw, uniformly random 64-bit value (one full generator step).
*
* Time complexity: O(1).
* \param rng The generator to draw from. Must not be NULL.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG.
* \return A value uniformly distributed over the full uint64_t range, or 0 if an error occurred.
*/
uint64_t rng_next_u64(Rng* rng, ErrorCode* error);

/**
* Draw a uniformly random double in [0, 1).
*
* Time complexity: O(1).
* \param rng The generator to draw from. Must not be NULL.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG.
* \return A value in [0, 1), or 0.0 if an error occurred (0.0 may also be legitimate result - check error to distinguish).
*/
double   rng_next_double(Rng* rng, ErrorCode* error); /* uniform [0, 1) */

// Distribution

/**
* Draw a uniformly random int64_t in [min, max] (both ends inclusive),
* with no modulo bias, via rejection sampling.
*
* Time complexity: Unbounded, O(1) expected. Rejection sampling can in principle
* loop more than once, but the rejection probability is always well
* under 50% for any range, so more than a couple of iterations is
* extremely unlikely in practice.
*
* \param rng The generator to draw from. Must not be NULL.
* \param min Lower bound, inclusive.
* \param max Upper bound, inclusive. Must be >= min.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG, CESSE_ERR_BAD_ARG.
* \return A value in [min, max], or 0 if an error occurred (0 may also
*         be a legitimate, non-error result if it falls within [min, max] - check error to distinguish).
*/
int64_t  dist_uniform_i64(Rng* rng, const int64_t min, const int64_t max, ErrorCode* error); /* inclusive both ends */

/**
* Draw a uniformly random double in [min, max).
*
* Time complexity: O(1).
*
* \param rng The generator to draw from. Must not be NULL.
* \param min Lower bound, inclusive.
* \param max Upper bound, exclusive. Must be > min.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG, CESSE_ERR_BAD_ARG.
* \return A value in [min, max), or 0.0 if an error occurred.
*/
double   dist_uniform_double(Rng* rng, const double min, const double max, ErrorCode* error);  /* [min, max) */

/**
* Draw true with probability p, false otherwise.
*
* Time complexity: O(1).
*
* \param rng The generator to draw from. Must not be NULL.
* \param p Probability of returning true. Must be within [0.0, 1.0].
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG, CESSE_ERR_BAD_ARG.
* \return true or false, or false if an error occurred (false is also a
*         legitimate, non-error outcome - check error to distinguish).
*/
bool     dist_bernoulli(Rng* rng, const double p, ErrorCode* error);                     /* true w/ probability p */

/**
* Draw a sample from a normal (Gaussian) distribution via Box-Muller.
*
* Time complexity: Unbounded, but astonomically unlikely to be worse than O(1)
*
* \param rng The generator to draw from. Must not be NULL.
* \param mean The distribution's mean.
* \param stddev The distribution's standard deviation. Must be >= 0.0.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG, CESSE_ERR_BAD_ARG.
* \return The sampled value, or 0.0 if an error occurred.
*/
double   dist_normal(Rng* rng, const double mean, const double stddev, ErrorCode* error);

/**
* Draw a sample from an exponential distribution via inverse-CDF sampling.
*
* Time complexity: Unbounded, but astonomically unlikely to be worse than O(1)
*
* \param rng The generator to draw from. Must not be NULL.
* \param lambda The distribution's rate parameter. Must be > 0.0.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG, CESSE_ERR_BAD_ARG.
* \return The sampled value, or 0.0 if an error occurred.
*/
double   dist_exponential(Rng* rng, const double lambda, ErrorCode* error);

/**
* Shuffle anchor[0..length) in place via Fisher-Yates, so every
* permutation is equally likely.
*
* Time complexity: O(n), where n is length.
* \param anchor The array of pointers to shuffle. Must not be NULL.
* \param length Number of elements at anchor. 0 or 1 is a no-op.
* \param rng The generator supplying randomness. Must not be NULL.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG
*/
void shuffle(void** anchor, const size_t length, Rng* rng, ErrorCode* error);

#endif
