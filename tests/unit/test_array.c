#include "framework/test.h"
#include "cesse/array.h"
#include "cesse/utils.h"
#include <stdlib.h>

static void test_new_delete(void) {
        error_code_t err = CESSE_OK;
        Array* a = array_new(4, &err);
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_NOT_NULL(a);
        ASSERT_EQ(array_size(a, NULL), (size_t)0);
        array_delete(&a, &err, NULL, false);
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_NULL(a);
}

static void test_push_get_order(void) {
        error_code_t err = CESSE_OK;
        Array* a = array_new(2, &err);
        int vals[5] = {10, 20, 30, 40, 50};
        for (int i = 0; i < 5; i++) { array_push(a, &vals[i], &err); }
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_EQ(array_size(a, NULL), (size_t)5);
        for (int i = 0; i < 5; i++) {
                int* got = array_get(a, (size_t)i, &err);
                ASSERT_EQ(err, CESSE_OK);
                ASSERT_EQ(*got, vals[i]);
        }
        array_delete(&a, &err, NULL, false);
}

static void test_push_grows_capacity(void) {
        error_code_t err = CESSE_OK;
        Array* a = array_new(1, &err);
        size_t initial_cap = array_capacity(a, NULL);
        int vals[100];
        for (int i = 0; i < 100; i++) { vals[i] = i; array_push(a, &vals[i], &err); }
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_EQ(array_size(a, NULL), (size_t)100);
        ASSERT_TRUE(array_capacity(a, NULL) >= (size_t)100);
        ASSERT_TRUE(array_capacity(a, NULL) >= initial_cap);
        array_delete(&a, &err, NULL, false);
}

static void test_pop_lifo(void) {
        error_code_t err = CESSE_OK;
        Array* a = array_new(4, &err);
        int vals[3] = {1, 2, 3};
        for (int i = 0; i < 3; i++) { array_push(a, &vals[i], &err); }
        ASSERT_EQ(*(int*)array_pop(a, &err), 3);
        ASSERT_EQ(*(int*)array_pop(a, &err), 2);
        ASSERT_EQ(*(int*)array_pop(a, &err), 1);
        ASSERT_EQ(array_size(a, NULL), (size_t)0);
        array_delete(&a, &err, NULL, false);
}

static void test_set_returns_previous(void) {
        error_code_t err = CESSE_OK;
        Array* a = array_new(4, &err);
        int v1 = 1, v2 = 2;
        array_push(a, &v1, &err);
        void* prev = array_set(a, 0, &v2, &err);
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_EQ(*(int*)prev, 1);
        ASSERT_EQ(*(int*)array_get(a, 0, &err), 2);
        array_delete(&a, &err, NULL, false);
}

static void test_swap(void) {
        error_code_t err = CESSE_OK;
        Array* a = array_new(4, &err);
        int v1 = 1, v2 = 2;
        array_push(a, &v1, &err);
        array_push(a, &v2, &err);
        array_swap(a, 0, 1, &err);
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_EQ(*(int*)array_get(a, 0, &err), 2);
        ASSERT_EQ(*(int*)array_get(a, 1, &err), 1);
        /* swapping an index with itself: no-op, no error */
        array_swap(a, 0, 0, &err);
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_EQ(*(int*)array_get(a, 0, &err), 2);
        array_delete(&a, &err, NULL, false);
}

static void test_remove_middle(void) {
        error_code_t err = CESSE_OK;
        Array* a = array_new(4, &err);
        int vals[4] = {1, 2, 3, 4};
        for (int i = 0; i < 4; i++) { array_push(a, &vals[i], &err); }
        void* removed = array_remove(a, 1, &err); /* remove the "2" */
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_EQ(*(int*)removed, 2);
        ASSERT_EQ(array_size(a, NULL), (size_t)3);
        ASSERT_EQ(*(int*)array_get(a, 0, &err), 1);
        ASSERT_EQ(*(int*)array_get(a, 1, &err), 3);
        ASSERT_EQ(*(int*)array_get(a, 2, &err), 4);
        array_delete(&a, &err, NULL, false);
}

static void test_remove_last_delegates_to_pop(void) {
        error_code_t err = CESSE_OK;
        Array* a = array_new(4, &err);
        int vals[3] = {1, 2, 3};
        for (int i = 0; i < 3; i++) { array_push(a, &vals[i], &err); }
        void* removed = array_remove(a, 2, &err);
        ASSERT_EQ(*(int*)removed, 3);
        ASSERT_EQ(array_size(a, NULL), (size_t)2);
        array_delete(&a, &err, NULL, false);
}

static void test_clear(void) {
        error_code_t err = CESSE_OK;
        Array* a = array_new(4, &err);
        int vals[5];
        for (int i = 0; i < 5; i++) { vals[i] = i; array_push(a, &vals[i], &err); }
        array_clear(a, &err, NULL, false);
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_EQ(array_size(a, NULL), (size_t)0);
        array_delete(&a, &err, NULL, false);
}

static void test_fit_memory(void) {
        error_code_t err = CESSE_OK;
        Array* a = array_new(2, &err);
        int vals[50];
        for (int i = 0; i < 50; i++) { vals[i] = i; array_push(a, &vals[i], &err); }
        array_fit_memory(a, &err);
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_EQ(array_capacity(a, NULL), array_size(a, NULL));
        /* contents must survive the reallocation intact */
        for (int i = 0; i < 50; i++) {
                ASSERT_EQ(*(int*)array_get(a, (size_t)i, &err), i);
        }
        array_delete(&a, &err, NULL, false);
}

static int free_call_count = 0;
static error_code_t counting_freer(void** obj) {
        free_call_count++;
        free(*obj);
        *obj = NULL;
        return CESSE_OK;
}

static void test_delete_calls_custom_freer(void) {
        free_call_count = 0;
        error_code_t err = CESSE_OK;
        Array* a = array_new(4, &err);
        for (int i = 0; i < 3; i++) {
                int* heap_val = malloc(sizeof(int));
                *heap_val = i;
                array_push(a, heap_val, &err);
        }
        array_delete(&a, &err, counting_freer, false);
        ASSERT_EQ(free_call_count, 3);
}

static void test_delete_free_as_fallback(void) {
        error_code_t err = CESSE_OK;
        Array* a = array_new(4, &err);
        for (int i = 0; i < 3; i++) {
                int* heap_val = malloc(sizeof(int));
                *heap_val = i;
                array_push(a, heap_val, &err);
        }
        /* no custom freer, but free_as_fallback=true: each element
         * should get free()'d directly. Correctness here is really
         * "ASan doesn't report a leak", checked at the process level. */
        array_delete(&a, &err, NULL, true);
        ASSERT_EQ(err, CESSE_OK);
}

static void test_max_capacity_is_positive(void) {
        ASSERT_TRUE(array_max_capacity() > 0);
}

static bool int_lt(void* a, void* b) { return *(int*)a < *(int*)b; }

static void test_sort_wrapper(void) {
        error_code_t err = CESSE_OK;
        Array* a = array_new(4, &err);
        int vals[5] = {3, 1, 4, 1, 5};
        for (int i = 0; i < 5; i++) { array_push(a, &vals[i], &err); }
        array_sort(a, int_lt, &err);
        ASSERT_EQ(err, CESSE_OK);
        for (size_t i = 1; i < 5; i++) {
                ASSERT_TRUE(*(int*)array_get(a, i-1, NULL) >= *(int*)array_get(a, i, NULL));
        }
        array_delete(&a, &err, NULL, false);
}

static error_code_t failing_freer(void** obj) {
        (void)obj;
        return CESSE_ERR_ALLOC; /* simulates a custom freer reporting its own failure */
}

static void test_delete_reports_freer_failure(void) {
        /* Doesn't assert on stderr output (out of scope for this
         * framework) -- exists to exercise the "freer reported an
         * error" branch under ASan, catching any use-after-free or
         * leak that path might otherwise hide. */
        error_code_t err = CESSE_OK;
        Array* a = array_new(4, &err);
        int* heap_val = malloc(sizeof(int));
        *heap_val = 1;
        array_push(a, heap_val, &err);
        array_delete(&a, &err, failing_freer, false);
        ASSERT_EQ(err, CESSE_OK); /* the freer's own failure doesn't fail delete() itself */
        free(heap_val); /* failing_freer deliberately didn't free it */
}

int main(void) {
        TEST_INIT();
        RUN(test_new_delete);
        RUN(test_push_get_order);
        RUN(test_push_grows_capacity);
        RUN(test_pop_lifo);
        RUN(test_set_returns_previous);
        RUN(test_swap);
        RUN(test_remove_middle);
        RUN(test_remove_last_delegates_to_pop);
        RUN(test_clear);
        RUN(test_fit_memory);
        RUN(test_delete_calls_custom_freer);
        RUN(test_delete_free_as_fallback);
        RUN(test_max_capacity_is_positive);
        RUN(test_sort_wrapper);
        RUN(test_delete_reports_freer_failure);
        return TEST_REPORT();
}
