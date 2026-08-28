#include "framework/test.h"
#include "cesse/array.h"
#include "cesse/rng.h"
#include "cesse/utils.h"
#include <stdlib.h>
#include <stdint.h>

/* Drives a long randomized sequence of push/pop/get/set/swap/remove
 * against a real Array, mirroring every operation on a plain "shadow"
 * array, and checking full agreement after each step. A fixed seed
 * makes this fully reproducible -- a failure here is a deterministic
 * repro, not a one-off flake. */
static void test_array_fuzz_against_shadow(void) {
        ErrorCode err = CESSE_OK;
        Rng* rng = rng_new(20240827ULL, &err);
        Array* a = array_new(4, &err);

        size_t pool_size = 150000;
        int* pool = malloc(pool_size * sizeof(int));
        size_t pool_next = 0;

        size_t max_elements = 1000;
        int* shadow = malloc(max_elements * sizeof(int));
        size_t shadow_size = 0;

        long iterations = 100000;
        for (long iter = 0; iter < iterations; iter++) {
                int op = (int)dist_uniform_i64(rng, 0, 5, &err);

                if (op == 0 && shadow_size < max_elements && pool_next < pool_size) {
                        int v = (int)dist_uniform_i64(rng, -1000000, 1000000, &err);
                        pool[pool_next] = v;
                        array_push(a, &pool[pool_next], &err);
                        ASSERT_EQ(err, CESSE_OK);
                        pool_next++;
                        shadow[shadow_size++] = v;
                } else if (op == 1) {
                        ErrorCode pop_err = CESSE_OK;
                        void* popped = array_pop(a, &pop_err);
                        if (shadow_size == 0) {
                                ASSERT_EQ(pop_err, CESSE_ERR_EMPTY);
                                ASSERT_NULL(popped);
                        } else {
                                ASSERT_EQ(pop_err, CESSE_OK);
                                ASSERT_EQ(*(int*)popped, shadow[shadow_size - 1]);
                                shadow_size--;
                        }
                } else if (op == 2 && shadow_size > 0) {
                        size_t idx = (size_t)dist_uniform_i64(rng, 0, (int64_t)shadow_size - 1, &err);
                        int* got = array_get(a, idx, &err);
                        ASSERT_EQ(err, CESSE_OK);
                        ASSERT_EQ(*got, shadow[idx]);
                } else if (op == 3 && shadow_size > 0 && pool_next < pool_size) {
                        size_t idx = (size_t)dist_uniform_i64(rng, 0, (int64_t)shadow_size - 1, &err);
                        int v = (int)dist_uniform_i64(rng, -1000000, 1000000, &err);
                        pool[pool_next] = v;
                        array_set(a, idx, &pool[pool_next], &err);
                        ASSERT_EQ(err, CESSE_OK);
                        pool_next++;
                        shadow[idx] = v;
                } else if (op == 4 && shadow_size >= 2) {
                        size_t i1 = (size_t)dist_uniform_i64(rng, 0, (int64_t)shadow_size - 1, &err);
                        size_t i2 = (size_t)dist_uniform_i64(rng, 0, (int64_t)shadow_size - 1, &err);
                        array_swap(a, i1, i2, &err);
                        ASSERT_EQ(err, CESSE_OK);
                        int tmp = shadow[i1]; shadow[i1] = shadow[i2]; shadow[i2] = tmp;
                } else if (op == 5 && shadow_size > 0) {
                        size_t idx = (size_t)dist_uniform_i64(rng, 0, (int64_t)shadow_size - 1, &err);
                        void* removed = array_remove(a, idx, &err);
                        ASSERT_EQ(err, CESSE_OK);
                        ASSERT_EQ(*(int*)removed, shadow[idx]);
                        for (size_t k = idx; k + 1 < shadow_size; k++) { shadow[k] = shadow[k + 1]; }
                        shadow_size--;
                }

                ASSERT_EQ(array_size(a, NULL), shadow_size);
        }

        for (size_t i = 0; i < shadow_size; i++) {
                ASSERT_EQ(*(int*)array_get(a, i, NULL), shadow[i]);
        }

        array_delete(&a, &err, NULL, false);
        rng_delete(&rng, &err);
        free(pool);
        free(shadow);
}

int main(void) {
        TEST_INIT();
        RUN(test_array_fuzz_against_shadow);
        return TEST_REPORT();
}
