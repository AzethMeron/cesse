#include "framework/test.h"
#include "cesse/rng.h"
#include "cesse/utils.h"
#include <stdlib.h>
#include <stdint.h>

/* Randomly generated min/max pairs, drawn from the full int64_t
 * range -- deliberately includes pairs near INT64_MIN/INT64_MAX and
 * very narrow ranges, which is exactly where an off-by-one in the
 * modulo-bias-avoidance logic (bounded_u64) would show up as an
 * out-of-range result. A separate Rng drives the min/max generation
 * so it isn't correlated with the values being tested. */
static void test_uniform_i64_fuzz_bounds(void) {
        error_code_t err = CESSE_OK;
        Rng* rng = rng_new(77711ULL, &err);
        Rng* param_rng = rng_new(77822ULL, &err);

        long trials = 200000;
        for (long t = 0; t < trials; t++) {
                int64_t a = dist_uniform_i64(param_rng, INT64_MIN, INT64_MAX, &err);
                int64_t b = dist_uniform_i64(param_rng, INT64_MIN, INT64_MAX, &err);
                int64_t lo = a < b ? a : b;
                int64_t hi = a < b ? b : a;

                int64_t v = dist_uniform_i64(rng, lo, hi, &err);
                ASSERT_EQ(err, CESSE_OK);
                ASSERT_TRUE(v >= lo && v <= hi);
        }

        rng_delete(&rng, &err);
        rng_delete(&param_rng, &err);
}

/* Narrow, deliberately adversarial ranges: sizes that do NOT evenly
 * divide 2^64 (most don't), tiny ranges (2-3 values), and repeated
 * draws from the same tiny range checking every value stays in
 * bounds across many draws -- a bias or off-by-one would likely
 * surface as an out-of-range value somewhere in a large sample. */
static void test_uniform_i64_fuzz_small_ranges(void) {
        error_code_t err = CESSE_OK;
        Rng* rng = rng_new(4004ULL, &err);
        Rng* param_rng = rng_new(4005ULL, &err);

        long trials = 5000;
        for (long t = 0; t < trials; t++) {
                int64_t lo = dist_uniform_i64(param_rng, -100, 100, &err);
                int64_t width = dist_uniform_i64(param_rng, 0, 20, &err);
                int64_t hi = lo + width;

                for (int draw = 0; draw < 50; draw++) {
                        int64_t v = dist_uniform_i64(rng, lo, hi, &err);
                        ASSERT_EQ(err, CESSE_OK);
                        ASSERT_TRUE(v >= lo && v <= hi);
                }
        }

        rng_delete(&rng, &err);
        rng_delete(&param_rng, &err);
}

/* Randomized array sizes through shuffle, each checked for being a
 * genuine permutation of its input (every original value present
 * exactly once) -- not just "didn't crash". */
static void test_shuffle_fuzz_always_valid_permutation(void) {
        error_code_t err = CESSE_OK;
        Rng* rng = rng_new(32133ULL, &err);
        Rng* size_rng = rng_new(32144ULL, &err);

        long trials = 3000;
        for (long t = 0; t < trials; t++) {
                size_t n = (size_t)dist_uniform_i64(size_rng, 0, 300, &err);
                int* vals = malloc((n ? n : 1) * sizeof(int));
                void** ptrs = malloc((n ? n : 1) * sizeof(void*));
                for (size_t i = 0; i < n; i++) { vals[i] = (int)i; ptrs[i] = &vals[i]; }

                shuffle(ptrs, n, rng, &err);
                ASSERT_EQ(err, CESSE_OK);

                bool* seen = calloc(n ? n : 1, sizeof(bool));
                for (size_t i = 0; i < n; i++) {
                        int v = *(int*)ptrs[i];
                        ASSERT_TRUE(v >= 0 && (size_t)v < (n ? n : 1));
                        ASSERT_FALSE(seen[v]);
                        seen[v] = true;
                }

                free(vals);
                free(ptrs);
                free(seen);
        }

        rng_delete(&rng, &err);
        rng_delete(&size_rng, &err);
}

int main(void) {
        TEST_INIT();
        RUN(test_uniform_i64_fuzz_bounds);
        RUN(test_uniform_i64_fuzz_small_ranges);
        RUN(test_shuffle_fuzz_always_valid_permutation);
        return TEST_REPORT();
}
