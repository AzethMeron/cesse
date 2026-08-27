#include "framework/test.h"
#include "cesse/stack.h"
#include "cesse/rng.h"
#include "cesse/utils.h"
#include <stdlib.h>

/* Same shadow-model approach as the array fuzz test, adapted to
 * Stack's narrower (LIFO-only) interface: push, pop, top. Fixed seed
 * -> fully reproducible failure if one ever occurs. */
static void test_stack_fuzz_against_shadow(void) {
        error_code_t err = CESSE_OK;
        Rng* rng = rng_new(998877ULL, &err);
        Stack* s = stack_new(&err);

        size_t pool_size = 150000;
        int* pool = malloc(pool_size * sizeof(int));
        size_t pool_next = 0;

        size_t max_elements = 2000;
        int* shadow = malloc(max_elements * sizeof(int));
        size_t shadow_size = 0;

        long iterations = 150000;
        for (long iter = 0; iter < iterations; iter++) {
                int op = (int)dist_uniform_i64(rng, 0, 2, &err);

                if (op == 0 && shadow_size < max_elements && pool_next < pool_size) {
                        int v = (int)dist_uniform_i64(rng, -1000000, 1000000, &err);
                        pool[pool_next] = v;
                        stack_push(s, &pool[pool_next], &err);
                        ASSERT_EQ(err, CESSE_OK);
                        pool_next++;
                        shadow[shadow_size++] = v;
                } else if (op == 1) {
                        error_code_t pop_err = CESSE_OK;
                        void* popped = stack_pop(s, &pop_err);
                        if (shadow_size == 0) {
                                ASSERT_EQ(pop_err, CESSE_ERR_EMPTY);
                                ASSERT_NULL(popped);
                        } else {
                                ASSERT_EQ(pop_err, CESSE_OK);
                                ASSERT_EQ(*(int*)popped, shadow[shadow_size - 1]);
                                shadow_size--;
                        }
                } else if (op == 2) {
                        error_code_t top_err = CESSE_OK;
                        void* top = stack_top(s, &top_err);
                        if (shadow_size == 0) {
                                ASSERT_EQ(top_err, CESSE_ERR_EMPTY);
                                ASSERT_NULL(top);
                        } else {
                                ASSERT_EQ(top_err, CESSE_OK);
                                ASSERT_EQ(*(int*)top, shadow[shadow_size - 1]);
                        }
                }

                ASSERT_EQ(stack_size(s, NULL), shadow_size);
        }

        stack_delete(&s, &err, NULL, false);
        rng_delete(&rng, &err);
        free(pool);
        free(shadow);
}

int main(void) {
        TEST_INIT();
        RUN(test_stack_fuzz_against_shadow);
        return TEST_REPORT();
}
