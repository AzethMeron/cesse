#include "framework/test.h"
#include "cesse/stack.h"
#include "cesse/utils.h"
#include <stdlib.h>

static void test_new_delete(void) {
        error_code_t err = CESSE_OK;
        Stack* s = stack_new(&err);
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_NOT_NULL(s);
        ASSERT_EQ(stack_size(s, NULL), (size_t)0);
        stack_delete(&s, &err, NULL, false);
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_NULL(s);
}

static void test_push_pop_lifo(void) {
        error_code_t err = CESSE_OK;
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
        stack_delete(&s, &err, NULL, false);
}

static void test_top_does_not_remove(void) {
        error_code_t err = CESSE_OK;
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
        stack_delete(&s, &err, NULL, false);
}

static void test_size_tracks_pushes_and_pops(void) {
        error_code_t err = CESSE_OK;
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
        stack_delete(&s, &err, NULL, false);
}

static void test_clear(void) {
        error_code_t err = CESSE_OK;
        Stack* s = stack_new(&err);
        int vals[10];
        for (int i = 0; i < 10; i++) { vals[i] = i; stack_push(s, &vals[i], &err); }
        stack_clear(s, &err, NULL, false);
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_EQ(stack_size(s, NULL), (size_t)0);
        /* pushing after a clear should behave like a fresh stack */
        int v = 99;
        stack_push(s, &v, &err);
        ASSERT_EQ(*(int*)stack_top(s, &err), 99);
        stack_delete(&s, &err, NULL, false);
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
        Stack* s = stack_new(&err);
        for (int i = 0; i < 4; i++) {
                int* heap_val = malloc(sizeof(int));
                *heap_val = i;
                stack_push(s, heap_val, &err);
        }
        stack_delete(&s, &err, counting_freer, false);
        ASSERT_EQ(free_call_count, 4);
}

static void test_delete_free_as_fallback(void) {
        error_code_t err = CESSE_OK;
        Stack* s = stack_new(&err);
        for (int i = 0; i < 4; i++) {
                int* heap_val = malloc(sizeof(int));
                *heap_val = i;
                stack_push(s, heap_val, &err);
        }
        stack_delete(&s, &err, NULL, true); /* ASan catches leaks if this is wrong */
        ASSERT_EQ(err, CESSE_OK);
}

static void test_max_capacity_is_positive(void) {
        ASSERT_TRUE(stack_max_capacity() > 0);
}

int main(void) {
        TEST_INIT();
        RUN(test_new_delete);
        RUN(test_push_pop_lifo);
        RUN(test_top_does_not_remove);
        RUN(test_size_tracks_pushes_and_pops);
        RUN(test_clear);
        RUN(test_delete_calls_custom_freer);
        RUN(test_delete_free_as_fallback);
        RUN(test_max_capacity_is_positive);
        return TEST_REPORT();
}
