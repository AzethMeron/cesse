#include "framework/test.h"
#include "cesse/rng.h"
#include "cesse/utils.h"
#include <math.h>
#include <stdint.h>

static void test_all_functions_reject_null_rng(void) {
        error_code_t err;

        err = CESSE_OK; rng_delete(NULL, &err);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);

        err = CESSE_OK; ASSERT_EQ(rng_next_u32(NULL, &err), (uint32_t)0);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);

        err = CESSE_OK; ASSERT_EQ(rng_next_u64(NULL, &err), (uint64_t)0);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);

        err = CESSE_OK; ASSERT_EQ(rng_next_double(NULL, &err), 0.0);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);

        err = CESSE_OK; dist_uniform_i64(NULL, 0, 1, &err);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);

        err = CESSE_OK; dist_uniform_double(NULL, 0.0, 1.0, &err);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);

        err = CESSE_OK; dist_bernoulli(NULL, 0.5, &err);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);

        err = CESSE_OK; dist_normal(NULL, 0.0, 1.0, &err);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);

        err = CESSE_OK; dist_exponential(NULL, 1.0, &err);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);

        int dummy;
        void* p[1] = { &dummy };
        err = CESSE_OK; shuffle(p, 1, NULL, &err);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);
}

static void test_shuffle_rejects_null_anchor(void) {
        error_code_t err = CESSE_OK;
        Rng* r = rng_new(1, &err);
        err = CESSE_OK;
        shuffle(NULL, 5, r, &err);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);
        rng_delete(&r, &err);
}

static void test_delete_of_already_nulled_pointer(void) {
        error_code_t err = CESSE_OK;
        Rng* r = rng_new(1, &err);
        rng_delete(&r, &err);
        ASSERT_NULL(r);
        err = CESSE_OK;
        rng_delete(&r, &err);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);
}

static void test_uniform_i64_min_greater_than_max(void) {
        error_code_t err = CESSE_OK;
        Rng* r = rng_new(1, &err);
        err = CESSE_OK;
        dist_uniform_i64(r, 10, 5, &err);
        ASSERT_EQ(err, CESSE_ERR_BAD_ARG);
        rng_delete(&r, &err);
}

static void test_uniform_double_bad_range(void) {
        error_code_t err = CESSE_OK;
        Rng* r = rng_new(1, &err);

        err = CESSE_OK; dist_uniform_double(r, 5.0, 5.0, &err); /* min==max: empty [min,max) */
        ASSERT_EQ(err, CESSE_ERR_BAD_ARG);

        err = CESSE_OK; dist_uniform_double(r, 5.0, 1.0, &err);
        ASSERT_EQ(err, CESSE_ERR_BAD_ARG);

        /* NaN in either position must be rejected, not silently produce NaN output */
        err = CESSE_OK; dist_uniform_double(r, NAN, 5.0, &err);
        ASSERT_EQ(err, CESSE_ERR_BAD_ARG);

        err = CESSE_OK; dist_uniform_double(r, 0.0, NAN, &err);
        ASSERT_EQ(err, CESSE_ERR_BAD_ARG);

        rng_delete(&r, &err);
}

static void test_bernoulli_bad_probability(void) {
        error_code_t err = CESSE_OK;
        Rng* r = rng_new(1, &err);

        err = CESSE_OK; dist_bernoulli(r, -0.1, &err);
        ASSERT_EQ(err, CESSE_ERR_BAD_ARG);

        err = CESSE_OK; dist_bernoulli(r, 1.1, &err);
        ASSERT_EQ(err, CESSE_ERR_BAD_ARG);

        err = CESSE_OK; dist_bernoulli(r, NAN, &err);
        ASSERT_EQ(err, CESSE_ERR_BAD_ARG);

        /* boundary values are valid, not errors */
        err = CESSE_OK; dist_bernoulli(r, 0.0, &err);
        ASSERT_EQ(err, CESSE_OK);
        err = CESSE_OK; dist_bernoulli(r, 1.0, &err);
        ASSERT_EQ(err, CESSE_OK);

        rng_delete(&r, &err);
}

static void test_normal_bad_stddev(void) {
        error_code_t err = CESSE_OK;
        Rng* r = rng_new(1, &err);

        err = CESSE_OK; dist_normal(r, 0.0, -1.0, &err);
        ASSERT_EQ(err, CESSE_ERR_BAD_ARG);

        err = CESSE_OK; dist_normal(r, 0.0, NAN, &err);
        ASSERT_EQ(err, CESSE_ERR_BAD_ARG);

        /* stddev==0 is a valid (degenerate) request: always returns mean */
        err = CESSE_OK;
        double v = dist_normal(r, 42.0, 0.0, &err);
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_EQ(v, 42.0);

        rng_delete(&r, &err);
}

static void test_exponential_bad_lambda(void) {
        error_code_t err = CESSE_OK;
        Rng* r = rng_new(1, &err);

        err = CESSE_OK; dist_exponential(r, 0.0, &err);
        ASSERT_EQ(err, CESSE_ERR_BAD_ARG);

        err = CESSE_OK; dist_exponential(r, -1.0, &err);
        ASSERT_EQ(err, CESSE_ERR_BAD_ARG);

        err = CESSE_OK; dist_exponential(r, NAN, &err);
        ASSERT_EQ(err, CESSE_ERR_BAD_ARG);

        rng_delete(&r, &err);
}

static void test_shuffle_length_zero_and_one_are_noops(void) {
        error_code_t err = CESSE_OK;
        Rng* r = rng_new(1, &err);
        int v = 42;
        void* p[1] = { &v };
        shuffle(p, 0, r, &err);
        ASSERT_EQ(err, CESSE_OK);
        shuffle(p, 1, r, &err);
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_EQ(*(int*)p[0], 42);
        rng_delete(&r, &err);
}

static void test_error_ptr_may_be_null_everywhere(void) {
        Rng* r = rng_new(1, NULL);
        ASSERT_NOT_NULL(r);
        rng_next_u32(r, NULL);
        rng_next_u64(r, NULL);
        rng_next_double(r, NULL);
        dist_uniform_i64(r, 0, 10, NULL);
        dist_uniform_i64(r, 10, 0, NULL); /* bad-arg path with NULL error */
        dist_uniform_double(r, 0.0, 1.0, NULL);
        dist_bernoulli(r, 0.5, NULL);
        dist_normal(r, 0.0, 1.0, NULL);
        dist_exponential(r, 1.0, NULL);
        int v = 1; void* p[1] = { &v };
        shuffle(p, 1, r, NULL);
        rng_delete(&r, NULL);
}

int main(void) {
        TEST_INIT();
        RUN(test_all_functions_reject_null_rng);
        RUN(test_shuffle_rejects_null_anchor);
        RUN(test_delete_of_already_nulled_pointer);
        RUN(test_uniform_i64_min_greater_than_max);
        RUN(test_uniform_double_bad_range);
        RUN(test_bernoulli_bad_probability);
        RUN(test_normal_bad_stddev);
        RUN(test_exponential_bad_lambda);
        RUN(test_shuffle_length_zero_and_one_are_noops);
        RUN(test_error_ptr_may_be_null_everywhere);
        return TEST_REPORT();
}
