#include "framework/test.h"
#include "cesse/sort.h"
#include "cesse/rng.h"
#include "cesse/utils.h"
#include <stdlib.h>

typedef struct { int key; long pos; } item_t;
static bool item_lt(void* a, void* b) { return ((item_t*)a)->key < ((item_t*)b)->key; }

static int qsort_int_cmp(const void* a, const void* b) {
        int ia = *(const int*)a, ib = *(const int*)b;
        return (ia > ib) - (ia < ib);
}

/* Many random trials, varying both size and key-range (a small key
 * range forces heavy duplication, which is exactly the condition that
 * makes stability bugs visible). Two independent checks per trial:
 *   1. sortedness + stability, checked directly
 *   2. multiset preservation, checked against qsort as an oracle --
 *      sort our array descending, sort the same keys ascending via
 *      the C library's own qsort, and the two must be exact reverses
 *      of each other. This catches "lost" or "duplicated" elements
 *      that a pure sortedness check alone wouldn't reveal. */
static void test_sort_fuzz_random_trials(void) {
        error_code_t err = CESSE_OK;
        Rng* rng = rng_new(555222ULL, &err);

        long trials = 3000;
        for (long t = 0; t < trials; t++) {
                size_t n = (size_t)dist_uniform_i64(rng, 0, 500, &err);
                int key_range = (int)dist_uniform_i64(rng, 1, 50, &err);

                item_t* items = malloc((n ? n : 1) * sizeof(item_t));
                void** ptrs = malloc((n ? n : 1) * sizeof(void*));
                int* keys_only = malloc((n ? n : 1) * sizeof(int));

                for (size_t i = 0; i < n; i++) {
                        int k = (int)dist_uniform_i64(rng, 0, key_range - 1, &err);
                        items[i].key = k;
                        items[i].pos = (long)i;
                        ptrs[i] = &items[i];
                        keys_only[i] = k;
                }

                error_code_t sort_err = CESSE_OK;
                sort(ptrs, n, item_lt, &sort_err);
                ASSERT_EQ(sort_err, CESSE_OK);

                for (size_t i = 1; i < n; i++) {
                        item_t* prev = ptrs[i - 1];
                        item_t* cur = ptrs[i];
                        ASSERT_TRUE(prev->key >= cur->key);
                        if (prev->key == cur->key) {
                                ASSERT_TRUE(prev->pos < cur->pos);
                        }
                }

                qsort(keys_only, n, sizeof(int), qsort_int_cmp);
                for (size_t i = 0; i < n; i++) {
                        int expected = keys_only[n - 1 - i];
                        ASSERT_EQ(((item_t*)ptrs[i])->key, expected);
                }

                free(items);
                free(ptrs);
                free(keys_only);
        }

        rng_delete(&rng, &err);
}

/* Separate, smaller-n, wide-key-range pass: mostly-distinct keys
 * (large range relative to n), the complementary case to the
 * heavy-duplication sweep above. */
static void test_sort_fuzz_mostly_distinct_keys(void) {
        error_code_t err = CESSE_OK;
        Rng* rng = rng_new(556223ULL, &err);

        long trials = 2000;
        for (long t = 0; t < trials; t++) {
                size_t n = (size_t)dist_uniform_i64(rng, 0, 200, &err);
                item_t* items = malloc((n ? n : 1) * sizeof(item_t));
                void** ptrs = malloc((n ? n : 1) * sizeof(void*));

                for (size_t i = 0; i < n; i++) {
                        items[i].key = (int)dist_uniform_i64(rng, -1000000, 1000000, &err);
                        items[i].pos = (long)i;
                        ptrs[i] = &items[i];
                }

                error_code_t sort_err = CESSE_OK;
                sort(ptrs, n, item_lt, &sort_err);
                ASSERT_EQ(sort_err, CESSE_OK);

                for (size_t i = 1; i < n; i++) {
                        ASSERT_TRUE(((item_t*)ptrs[i - 1])->key >= ((item_t*)ptrs[i])->key);
                }

                free(items);
                free(ptrs);
        }

        rng_delete(&rng, &err);
}

int main(void) {
        TEST_INIT();
        RUN(test_sort_fuzz_random_trials);
        RUN(test_sort_fuzz_mostly_distinct_keys);
        return TEST_REPORT();
}
