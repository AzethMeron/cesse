#include "framework/test.h"
#include "cesse/array.h"
#include "cesse/utils.h"
#include <stdlib.h>

static void test_new_delete(void) {
        ErrorCode err = CESSE_OK;
        Array* a = array_new(4, &err);
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_NOT_NULL(a);
        ASSERT_EQ(array_size(a, NULL), (size_t)0);
        array_delete(&a, &err, NULL);
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_NULL(a);
}

static void test_push_get_order(void) {
        ErrorCode err = CESSE_OK;
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
        array_delete(&a, &err, NULL);
}

static void test_push_grows_capacity(void) {
        ErrorCode err = CESSE_OK;
        Array* a = array_new(1, &err);
        size_t initial_cap = array_capacity(a, NULL);
        int vals[100];
        for (int i = 0; i < 100; i++) { vals[i] = i; array_push(a, &vals[i], &err); }
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_EQ(array_size(a, NULL), (size_t)100);
        ASSERT_TRUE(array_capacity(a, NULL) >= (size_t)100);
        ASSERT_TRUE(array_capacity(a, NULL) >= initial_cap);
        array_delete(&a, &err, NULL);
}

static void test_pop_lifo(void) {
        ErrorCode err = CESSE_OK;
        Array* a = array_new(4, &err);
        int vals[3] = {1, 2, 3};
        for (int i = 0; i < 3; i++) { array_push(a, &vals[i], &err); }
        ASSERT_EQ(*(int*)array_pop(a, &err), 3);
        ASSERT_EQ(*(int*)array_pop(a, &err), 2);
        ASSERT_EQ(*(int*)array_pop(a, &err), 1);
        ASSERT_EQ(array_size(a, NULL), (size_t)0);
        array_delete(&a, &err, NULL);
}

static void test_set_returns_previous(void) {
        ErrorCode err = CESSE_OK;
        Array* a = array_new(4, &err);
        int v1 = 1, v2 = 2;
        array_push(a, &v1, &err);
        void* prev = array_set(a, 0, &v2, &err);
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_EQ(*(int*)prev, 1);
        ASSERT_EQ(*(int*)array_get(a, 0, &err), 2);
        array_delete(&a, &err, NULL);
}

static void test_swap(void) {
        ErrorCode err = CESSE_OK;
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
        array_delete(&a, &err, NULL);
}

static void test_remove_middle(void) {
        ErrorCode err = CESSE_OK;
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
        array_delete(&a, &err, NULL);
}

static void test_remove_last_delegates_to_pop(void) {
        ErrorCode err = CESSE_OK;
        Array* a = array_new(4, &err);
        int vals[3] = {1, 2, 3};
        for (int i = 0; i < 3; i++) { array_push(a, &vals[i], &err); }
        void* removed = array_remove(a, 2, &err);
        ASSERT_EQ(*(int*)removed, 3);
        ASSERT_EQ(array_size(a, NULL), (size_t)2);
        array_delete(&a, &err, NULL);
}

static void test_clear(void) {
        ErrorCode err = CESSE_OK;
        Array* a = array_new(4, &err);
        int vals[5];
        for (int i = 0; i < 5; i++) { vals[i] = i; array_push(a, &vals[i], &err); }
        array_clear(a, &err, NULL);
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_EQ(array_size(a, NULL), (size_t)0);
        array_delete(&a, &err, NULL);
}

static void test_fit_memory(void) {
        ErrorCode err = CESSE_OK;
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
        array_delete(&a, &err, NULL);
}

static int free_call_count = 0;
static ErrorCode counting_freer(void** obj) {
        free_call_count++;
        free(*obj);
        *obj = NULL;
        return CESSE_OK;
}

static void test_delete_calls_custom_freer(void) {
        free_call_count = 0;
        ErrorCode err = CESSE_OK;
        Array* a = array_new(4, &err);
        for (int i = 0; i < 3; i++) {
                int* heap_val = malloc(sizeof(int));
                *heap_val = i;
                array_push(a, heap_val, &err);
        }
        array_delete(&a, &err, counting_freer);
        ASSERT_EQ(free_call_count, 3);
}

static void test_delete_with_default_delete_function(void) {
        ErrorCode err = CESSE_OK;
        Array* a = array_new(4, &err);
        for (int i = 0; i < 3; i++) {
                int* heap_val = malloc(sizeof(int));
                *heap_val = i;
                array_push(a, heap_val, &err);
        }
        /* default_delete_function is the reusable stand-in for "just
         * free() each element" -- passing it directly as the freer is
         * what replaced the old free_as_fallback=true parameter.
         * Correctness here is really "ASan doesn't report a leak". */
        array_delete(&a, &err, default_delete_function);
        ASSERT_EQ(err, CESSE_OK);
}

static void test_max_capacity_is_positive(void) {
        ASSERT_TRUE(array_max_capacity() > 0);
}

static bool int_lt(void* a, void* b) { return *(int*)a < *(int*)b; }

static void test_sort_wrapper(void) {
        ErrorCode err = CESSE_OK;
        Array* a = array_new(4, &err);
        int vals[5] = {3, 1, 4, 1, 5};
        for (int i = 0; i < 5; i++) { array_push(a, &vals[i], &err); }
        array_sort(a, int_lt, &err);
        ASSERT_EQ(err, CESSE_OK);
        for (size_t i = 1; i < 5; i++) {
                ASSERT_TRUE(*(int*)array_get(a, i-1, NULL) >= *(int*)array_get(a, i, NULL));
        }
        array_delete(&a, &err, NULL);
}

static ErrorCode failing_freer(void** obj) {
        (void)obj;
        return CESSE_ERR_ALLOC; /* simulates a custom freer reporting its own failure */
}

static void test_delete_reports_freer_failure(void) {
        /* Doesn't assert on stderr output (out of scope for this
         * framework) -- exists to exercise the "freer reported an
         * error" branch under ASan, catching any use-after-free or
         * leak that path might otherwise hide. */
        ErrorCode err = CESSE_OK;
        Array* a = array_new(4, &err);
        int* heap_val = malloc(sizeof(int));
        *heap_val = 1;
        array_push(a, heap_val, &err);
        array_delete(&a, &err, failing_freer);
        ASSERT_EQ(err, CESSE_OK); /* the freer's own failure doesn't fail delete() itself */
        free(heap_val); /* failing_freer deliberately didn't free it */
}

static void* int_deep_copy(void* obj, ErrorCode* error) {
        (void)error;
        int* copy = malloc(sizeof(int));
        *copy = *(int*)obj;
        return copy;
}

static void test_copy_deep_and_independent(void) {
        ErrorCode err = CESSE_OK;
        Array* a = array_new(4, &err);
        int vals[5] = {10, 20, 30, 40, 50};
        for (int i = 0; i < 5; i++) { array_push(a, &vals[i], &err); }

        Array* copy = array_copy(a, &err, int_deep_copy, default_delete_function);
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_EQ(array_size(copy, NULL), (size_t)5);

        bool values_match = true, pointers_independent = true;
        for (int i = 0; i < 5; i++) {
                int* orig = array_get(a, (size_t)i, NULL);
                int* cop = array_get(copy, (size_t)i, NULL);
                if (*orig != *cop) values_match = false;
                if (orig == cop) pointers_independent = false;
        }
        ASSERT_TRUE(values_match);
        ASSERT_TRUE(pointers_independent);

        array_delete(&a, &err, NULL);
        array_delete(&copy, &err, default_delete_function);
}

static void test_copy_empty(void) {
        ErrorCode err = CESSE_OK;
        Array* a = array_new(4, &err);
        Array* copy = array_copy(a, &err, int_deep_copy, default_delete_function);
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_NOT_NULL(copy);
        ASSERT_EQ(array_size(copy, NULL), (size_t)0);
        array_delete(&a, &err, NULL);
        array_delete(&copy, &err, default_delete_function);
}

static int array_copy_call_count = 0;
static void* array_copy_failing_on_third(void* obj, ErrorCode* error) {
        (void)obj;
        array_copy_call_count++;
        if (array_copy_call_count == 3) { *error = CESSE_ERR_ALLOC; return NULL; }
        int* copy = malloc(sizeof(int));
        *copy = *(int*)obj;
        return copy;
}

static void test_copy_propagates_mid_copy_failure(void) {
        ErrorCode err = CESSE_OK;
        Array* a = array_new(5, &err);
        int vals[5] = {1, 2, 3, 4, 5};
        for (int i = 0; i < 5; i++) { array_push(a, &vals[i], &err); }

        array_copy_call_count = 0;
        Array* copy = array_copy(a, &err, array_copy_failing_on_third, default_delete_function);
        ASSERT_NULL(copy);
        ASSERT_NE(err, CESSE_OK); /* the real failure must be reported, not silently CESSE_OK */

        array_delete(&a, &err, NULL);
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
        RUN(test_delete_with_default_delete_function);
        RUN(test_max_capacity_is_positive);
        RUN(test_sort_wrapper);
        RUN(test_delete_reports_freer_failure);
        RUN(test_copy_deep_and_independent);
        RUN(test_copy_empty);
        RUN(test_copy_propagates_mid_copy_failure);
        return TEST_REPORT();
}
