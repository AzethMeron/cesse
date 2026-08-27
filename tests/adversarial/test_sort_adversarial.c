#include "framework/test.h"
#include "cesse/sort.h"
#include "cesse/utils.h"
#include <stdlib.h>
#include <math.h>

static bool int_lt(void* a, void* b) { return *(int*)a < *(int*)b; }

/* NaN-aware comparator: NaN is treated as the maximum value, so it
 * sorts to the front in this descending sort -- see the conversation
 * history for why a naive `*l < *r` on doubles containing NaN breaks
 * the strict-weak-ordering assumption every comparison sort relies on. */
static bool double_lt_nan_safe(void* a, void* b) {
        double l = *(double*)a, r = *(double*)b;
        if (isnan(l)) { return false; }
        if (isnan(r)) { return true; }
        return l < r;
}

static void test_null_begin(void) {
        error_code_t err = CESSE_OK;
        sort(NULL, 5, int_lt, &err);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);
}

static void test_null_comparator(void) {
        error_code_t err = CESSE_OK;
        int v = 1; void* p[1] = { &v };
        sort(p, 1, NULL, &err);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);
}

static void test_nan_sorts_to_front_and_does_not_crash(void) {
        double vals[] = {5.0, -3.0, NAN, 1.0, 0.0, -0.0, 100.0};
        size_t n = sizeof vals / sizeof vals[0];
        void* ptrs[7];
        for (size_t i = 0; i < n; i++) { ptrs[i] = &vals[i]; }
        error_code_t err = CESSE_OK;
        sort(ptrs, n, double_lt_nan_safe, &err);
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_TRUE(isnan(*(double*)ptrs[0])); /* NaN treated as max -> front */
        for (size_t i = 2; i < n; i++) {
                ASSERT_TRUE(*(double*)ptrs[i-1] >= *(double*)ptrs[i]);
        }
}

static void test_already_descending_large_n_stays_fast_and_correct(void) {
        /* The classic worst case for a naive last-element-pivot
         * quicksort. Merge sort should handle it identically to any
         * other input -- this also exercises recursion depth at a
         * realistic scale, confirming the O(log n) depth guarantee. */
        size_t n = 200000;
        int* vals = malloc(n * sizeof(int));
        void** ptrs = malloc(n * sizeof(void*));
        for (size_t i = 0; i < n; i++) { vals[i] = (int)(n - i); ptrs[i] = &vals[i]; }
        error_code_t err = CESSE_OK;
        sort(ptrs, n, int_lt, &err);
        ASSERT_EQ(err, CESSE_OK);
        bool ok = true;
        for (size_t i = 1; i < n; i++) { if (*(int*)ptrs[i-1] < *(int*)ptrs[i]) { ok = false; break; } }
        ASSERT_TRUE(ok);
        free(vals);
        free(ptrs);
}

static void test_all_equal_large_n(void) {
        size_t n = 200000;
        int* vals = malloc(n * sizeof(int));
        void** ptrs = malloc(n * sizeof(void*));
        for (size_t i = 0; i < n; i++) { vals[i] = 42; ptrs[i] = &vals[i]; }
        error_code_t err = CESSE_OK;
        sort(ptrs, n, int_lt, &err);
        ASSERT_EQ(err, CESSE_OK);
        for (size_t i = 0; i < n; i++) { ASSERT_EQ(*(int*)ptrs[i], 42); }
        free(vals);
        free(ptrs);
}

static void test_already_ascending_large_n(void) {
        /* The mirror image of the descending case -- also a classic
         * quicksort worst case, tested for completeness. */
        size_t n = 200000;
        int* vals = malloc(n * sizeof(int));
        void** ptrs = malloc(n * sizeof(void*));
        for (size_t i = 0; i < n; i++) { vals[i] = (int)i; ptrs[i] = &vals[i]; }
        error_code_t err = CESSE_OK;
        sort(ptrs, n, int_lt, &err);
        ASSERT_EQ(err, CESSE_OK);
        bool ok = true;
        for (size_t i = 1; i < n; i++) { if (*(int*)ptrs[i-1] < *(int*)ptrs[i]) { ok = false; break; } }
        ASSERT_TRUE(ok);
        free(vals);
        free(ptrs);
}

static void test_two_distinct_values_only(void) {
        /* Only two distinct keys, heavily repeated -- another shape
         * known to stress partition-based sorts differently than
         * "all equal" or "all distinct". */
        size_t n = 50000;
        int* vals = malloc(n * sizeof(int));
        void** ptrs = malloc(n * sizeof(void*));
        for (size_t i = 0; i < n; i++) { vals[i] = (int)(i % 2); ptrs[i] = &vals[i]; }
        error_code_t err = CESSE_OK;
        sort(ptrs, n, int_lt, &err);
        ASSERT_EQ(err, CESSE_OK);
        size_t i = 0;
        while (i < n && *(int*)ptrs[i] == 1) { i++; }
        while (i < n) { ASSERT_EQ(*(int*)ptrs[i], 0); i++; }
        free(vals);
        free(ptrs);
}

int main(void) {
        TEST_INIT();
        RUN(test_null_begin);
        RUN(test_null_comparator);
        RUN(test_nan_sorts_to_front_and_does_not_crash);
        RUN(test_already_descending_large_n_stays_fast_and_correct);
        RUN(test_all_equal_large_n);
        RUN(test_already_ascending_large_n);
        RUN(test_two_distinct_values_only);
        return TEST_REPORT();
}
