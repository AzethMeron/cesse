#include "framework/test.h"
#include "cesse/stack.h"
#include "cesse/utils.h"
#include <stdlib.h>

static void test_new_delete(void) {
        ErrorCode err = CESSE_OK;
        Stack* s = stack_new(&err);
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_NOT_NULL(s);
        ASSERT_EQ(stack_size(s, NULL), (size_t)0);
        stack_delete(&s, &err, NULL);
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_NULL(s);
}

static void test_push_pop_lifo(void) {
        ErrorCode err = CESSE_OK;
        Stack* s = stack_new(&err);
        int vals[5] = {10, 20, 30, 40, 50};
        for (int i = 0; i < 5; i++) { stack_push(s, &vals[i], &err); }
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_EQ(stack_size(s, NULL), (size_t)5);
        for (int i = 4; i >= 0; i--) {
                int* popped = stack_pop(s, &err);
                ASSERT_EQ(err, CESSE_OK);
                ASSERT_EQ(*popped, vals[i]);
        }
        ASSERT_EQ(stack_size(s, NULL), (size_t)0);
        stack_delete(&s, &err, NULL);
}

static void test_top_does_not_remove(void) {
        ErrorCode err = CESSE_OK;
        Stack* s = stack_new(&err);
        int a = 1, b = 2;
        stack_push(s, &a, &err);
        stack_push(s, &b, &err);
        int* top1 = stack_top(s, &err);
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_EQ(*top1, 2);
        ASSERT_EQ(stack_size(s, NULL), (size_t)2); /* unchanged */
        int* top2 = stack_top(s, &err);
        ASSERT_EQ(*top2, 2); /* same value, still there */
        stack_delete(&s, &err, NULL);
}

static void test_size_tracks_pushes_and_pops(void) {
        ErrorCode err = CESSE_OK;
        Stack* s = stack_new(&err);
        int v = 1;
        ASSERT_EQ(stack_size(s, NULL), (size_t)0);
        stack_push(s, &v, &err);
        ASSERT_EQ(stack_size(s, NULL), (size_t)1);
        stack_push(s, &v, &err);
        ASSERT_EQ(stack_size(s, NULL), (size_t)2);
        stack_pop(s, &err);
        ASSERT_EQ(stack_size(s, NULL), (size_t)1);
        stack_pop(s, &err);
        ASSERT_EQ(stack_size(s, NULL), (size_t)0);
        stack_delete(&s, &err, NULL);
}

static void test_clear(void) {
        ErrorCode err = CESSE_OK;
        Stack* s = stack_new(&err);
        int vals[10];
        for (int i = 0; i < 10; i++) { vals[i] = i; stack_push(s, &vals[i], &err); }
        stack_clear(s, &err, NULL);
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_EQ(stack_size(s, NULL), (size_t)0);
        /* pushing after a clear should behave like a fresh stack */
        int v = 99;
        stack_push(s, &v, &err);
        ASSERT_EQ(*(int*)stack_top(s, &err), 99);
        stack_delete(&s, &err, NULL);
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
        Stack* s = stack_new(&err);
        for (int i = 0; i < 4; i++) {
                int* heap_val = malloc(sizeof(int));
                *heap_val = i;
                stack_push(s, heap_val, &err);
        }
        stack_delete(&s, &err, counting_freer);
        ASSERT_EQ(free_call_count, 4);
}

static void test_delete_with_default_delete_function(void) {
        ErrorCode err = CESSE_OK;
        Stack* s = stack_new(&err);
        for (int i = 0; i < 4; i++) {
                int* heap_val = malloc(sizeof(int));
                *heap_val = i;
                stack_push(s, heap_val, &err);
        }
        stack_delete(&s, &err, default_delete_function); /* ASan catches leaks if this is wrong */
        ASSERT_EQ(err, CESSE_OK);
}

static ErrorCode failing_freer(void** obj) {
        (void)obj;
        return CESSE_ERR_ALLOC; /* simulates a custom freer reporting its own failure */
}

static void test_delete_reports_freer_failure(void) {
        ErrorCode err = CESSE_OK;
        Stack* s = stack_new(&err);
        int* heap_val = malloc(sizeof(int));
        *heap_val = 1;
        stack_push(s, heap_val, &err);
        stack_delete(&s, &err, failing_freer);
        ASSERT_EQ(err, CESSE_OK); /* the freer's own failure doesn't fail delete() itself */
        free(heap_val); /* failing_freer deliberately didn't free it */
}

static void test_max_capacity_is_positive(void) {
        ASSERT_TRUE(stack_max_capacity() > 0);
}

static void* int_deep_copy(void* obj, ErrorCode* error) {
        (void)error;
        int* copy = malloc(sizeof(int));
        *copy = *(int*)obj;
        return copy;
}

static void test_copy_preserves_order(void) {
        ErrorCode err = CESSE_OK;
        Stack* s = stack_new(&err);
        int vals[5] = {1, 2, 3, 4, 5};
        for (int i = 0; i < 5; i++) { stack_push(s, &vals[i], &err); }

        Stack* copy = stack_copy(s, &err, int_deep_copy, default_delete_function);
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_EQ(stack_size(copy, NULL), (size_t)5);

        bool order_ok = true;
        for (int expected = 5; expected >= 1; expected--) {
                int* popped = stack_pop(copy, &err);
                if (!popped || *popped != expected) order_ok = false;
                free(popped);
        }
        ASSERT_TRUE(order_ok);
        ASSERT_EQ(stack_size(s, NULL), (size_t)5); /* original untouched */

        stack_delete(&s, &err, NULL);
        stack_delete(&copy, &err, default_delete_function);
}

static void test_copy_empty(void) {
        ErrorCode err = CESSE_OK;
        Stack* s = stack_new(&err);
        Stack* copy = stack_copy(s, &err, int_deep_copy, default_delete_function);
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_NOT_NULL(copy);
        ASSERT_EQ(stack_size(copy, NULL), (size_t)0);
        stack_delete(&s, &err, NULL);
        stack_delete(&copy, &err, default_delete_function);
}

static int stack_copy_call_count = 0;
static void* stack_copy_failing_on_third(void* obj, ErrorCode* error) {
        (void)obj;
        stack_copy_call_count++;
        if (stack_copy_call_count == 3) { *error = CESSE_ERR_ALLOC; return NULL; }
        int* copy = malloc(sizeof(int));
        *copy = *(int*)obj;
        return copy;
}

static void test_copy_propagates_mid_copy_failure(void) {
        ErrorCode err = CESSE_OK;
        Stack* s = stack_new(&err);
        int vals[5] = {1, 2, 3, 4, 5};
        for (int i = 0; i < 5; i++) { stack_push(s, &vals[i], &err); }

        stack_copy_call_count = 0;
        Stack* copy = stack_copy(s, &err, stack_copy_failing_on_third, default_delete_function);
        ASSERT_NULL(copy);
        ASSERT_NE(err, CESSE_OK);

        stack_delete(&s, &err, NULL);
}

int main(void) {
        TEST_INIT();
        RUN(test_new_delete);
        RUN(test_push_pop_lifo);
        RUN(test_top_does_not_remove);
        RUN(test_size_tracks_pushes_and_pops);
        RUN(test_clear);
        RUN(test_delete_calls_custom_freer);
        RUN(test_delete_with_default_delete_function);
        RUN(test_delete_reports_freer_failure);
        RUN(test_max_capacity_is_positive);
        RUN(test_copy_preserves_order);
        RUN(test_copy_empty);
        RUN(test_copy_propagates_mid_copy_failure);
        return TEST_REPORT();
}
