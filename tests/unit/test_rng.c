#include "framework/test.h"
#include "cesse/rng.h"
#include "cesse/utils.h"
#include <math.h>
#include <stdlib.h>
#include <stdint.h>

static void test_new_delete(void) {
        error_code_t err = CESSE_OK;
        Rng* r = rng_new(1234, &err);
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_NOT_NULL(r);
        rng_delete(&r, &err);
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_NULL(r);
}

static void test_determinism_same_seed(void) {
        error_code_t err = CESSE_OK;
        Rng* a = rng_new(42, &err);
        Rng* b = rng_new(42, &err);
        for (int i = 0; i < 1000; i++) {
                ASSERT_EQ(rng_next_u64(a, &err), rng_next_u64(b, &err));
        }
        rng_delete(&a, &err);
        rng_delete(&b, &err);
}

static void test_different_seeds_diverge(void) {
        error_code_t err = CESSE_OK;
        Rng* a = rng_new(1, &err);
        Rng* b = rng_new(2, &err);
        int differences = 0;
        for (int i = 0; i < 100; i++) {
                if (rng_next_u64(a, &err) != rng_next_u64(b, &err)) { differences++; }
        }
        ASSERT_TRUE(differences > 90); /* virtually certain for a sound PRNG */
        rng_delete(&a, &err);
        rng_delete(&b, &err);
}

static void test_next_double_in_range(void) {
        error_code_t err = CESSE_OK;
        Rng* r = rng_new(7, &err);
        for (int i = 0; i < 100000; i++) {
                double d = rng_next_double(r, &err);
                ASSERT_TRUE(d >= 0.0 && d < 1.0);
        }
        rng_delete(&r, &err);
}

static void test_uniform_i64_within_bounds_and_bias(void) {
        error_code_t err = CESSE_OK;
        Rng* r = rng_new(99, &err);
        long n = 600000;
        long counts[3] = {0, 0, 0}; /* range=3 does NOT evenly divide 2^64 */
        for (long i = 0; i < n; i++) {
                int64_t v = dist_uniform_i64(r, 0, 2, &err);
                ASSERT_TRUE(v >= 0 && v <= 2);
                counts[v]++;
        }
        for (int i = 0; i < 3; i++) {
                double frac = (double)counts[i] / (double)n;
                ASSERT_NEAR(frac, 1.0/3.0, 0.01); /* generous: catches real bias, not noise */
        }
        rng_delete(&r, &err);
}

static void test_uniform_i64_min_equals_max(void) {
        error_code_t err = CESSE_OK;
        Rng* r = rng_new(3, &err);
        for (int i = 0; i < 20; i++) {
                ASSERT_EQ(dist_uniform_i64(r, 42, 42, &err), (int64_t)42);
        }
        rng_delete(&r, &err);
}

static void test_uniform_i64_full_range(void) {
        error_code_t err = CESSE_OK;
        Rng* r = rng_new(5, &err);
        bool saw_negative = false, saw_positive = false;
        for (int i = 0; i < 2000; i++) {
                int64_t v = dist_uniform_i64(r, INT64_MIN, INT64_MAX, &err);
                if (v < 0) { saw_negative = true; }
                if (v > 0) { saw_positive = true; }
        }
        ASSERT_TRUE(saw_negative);
        ASSERT_TRUE(saw_positive);
        rng_delete(&r, &err);
}

static void test_uniform_double_in_range(void) {
        error_code_t err = CESSE_OK;
        Rng* r = rng_new(11, &err);
        for (int i = 0; i < 100000; i++) {
                double d = dist_uniform_double(r, -5.0, 5.0, &err);
                ASSERT_TRUE(d >= -5.0 && d < 5.0);
        }
        rng_delete(&r, &err);
}

static void test_bernoulli_matches_probability(void) {
        error_code_t err = CESSE_OK;
        Rng* r = rng_new(13, &err);
        long n = 500000, trues = 0;
        for (long i = 0; i < n; i++) { if (dist_bernoulli(r, 0.3, &err)) { trues++; } }
        ASSERT_NEAR((double)trues / (double)n, 0.3, 0.01);
        rng_delete(&r, &err);
}

static void test_normal_mean_and_stddev(void) {
        error_code_t err = CESSE_OK;
        Rng* r = rng_new(17, &err);
        long n = 500000;
        double sum = 0.0, sumsq = 0.0;
        for (long i = 0; i < n; i++) {
                double x = dist_normal(r, 50.0, 10.0, &err);
                ASSERT_TRUE(isfinite(x));
                sum += x;
                sumsq += x * x;
        }
        double mean = sum / (double)n;
        double var = sumsq / (double)n - mean * mean;
        ASSERT_NEAR(mean, 50.0, 0.5);
        ASSERT_NEAR(sqrt(var), 10.0, 0.5);
        rng_delete(&r, &err);
}

static void test_exponential_mean_and_positivity(void) {
        error_code_t err = CESSE_OK;
        Rng* r = rng_new(23, &err);
        long n = 500000;
        double sum = 0.0;
        for (long i = 0; i < n; i++) {
                double x = dist_exponential(r, 2.0, &err);
                ASSERT_TRUE(x >= 0.0);
                sum += x;
        }
        ASSERT_NEAR(sum / (double)n, 0.5, 0.02); /* 1/lambda */
        rng_delete(&r, &err);
}

static void test_shuffle_is_a_permutation(void) {
        error_code_t err = CESSE_OK;
        Rng* r = rng_new(29, &err);
        int vals[8] = {0, 1, 2, 3, 4, 5, 6, 7};
        void* ptrs[8];
        for (int i = 0; i < 8; i++) { ptrs[i] = &vals[i]; }
        shuffle(ptrs, 8, r, &err);
        ASSERT_EQ(err, CESSE_OK);
        bool seen[8] = {false};
        for (int i = 0; i < 8; i++) {
                int v = *(int*)ptrs[i];
                ASSERT_TRUE(v >= 0 && v < 8);
                ASSERT_FALSE(seen[v]);
                seen[v] = true;
        }
        rng_delete(&r, &err);
}

int main(void) {
        TEST_INIT();
        RUN(test_new_delete);
        RUN(test_determinism_same_seed);
        RUN(test_different_seeds_diverge);
        RUN(test_next_double_in_range);
        RUN(test_uniform_i64_within_bounds_and_bias);
        RUN(test_uniform_i64_min_equals_max);
        RUN(test_uniform_i64_full_range);
        RUN(test_uniform_double_in_range);
        RUN(test_bernoulli_matches_probability);
        RUN(test_normal_mean_and_stddev);
        RUN(test_exponential_mean_and_positivity);
        RUN(test_shuffle_is_a_permutation);
        return TEST_REPORT();
}
