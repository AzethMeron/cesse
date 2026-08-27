#include "framework/test.h"
#include "cesse/stack.h"
#include "cesse/utils.h"

static void test_top_on_empty_returns_error_not_crash(void) {
        /* This is exactly the bug that used to crash (dereferencing
         * stack->front->object with front==NULL). Confirms the fix. */
        error_code_t err = CESSE_OK;
        Stack* s = stack_new(&err);
        err = CESSE_OK;
        void* v = stack_top(s, &err);
        ASSERT_NULL(v);
        ASSERT_EQ(err, CESSE_ERR_EMPTY);
        stack_delete(&s, &err, NULL, false);
}

static void test_pop_on_empty_returns_error_not_crash(void) {
        error_code_t err = CESSE_OK;
        Stack* s = stack_new(&err);
        err = CESSE_OK;
        void* v = stack_pop(s, &err);
        ASSERT_NULL(v);
        ASSERT_EQ(err, CESSE_ERR_EMPTY);
        stack_delete(&s, &err, NULL, false);
}

static void test_top_and_pop_on_empty_after_draining(void) {
        /* Fill it, drain it completely via pop, then hit the same
         * empty-state checks again -- catches any bug where "empty
         * from birth" and "emptied via pop" are treated differently. */
        error_code_t err = CESSE_OK;
        Stack* s = stack_new(&err);
        int v = 1;
        stack_push(s, &v, &err);
        stack_pop(s, &err);
        err = CESSE_OK;
        ASSERT_NULL(stack_top(s, &err));
        ASSERT_EQ(err, CESSE_ERR_EMPTY);
        err = CESSE_OK;
        ASSERT_NULL(stack_pop(s, &err));
        ASSERT_EQ(err, CESSE_ERR_EMPTY);
        stack_delete(&s, &err, NULL, false);
}

static void test_push_null_object(void) {
        error_code_t err = CESSE_OK;
        Stack* s = stack_new(&err);
        err = CESSE_OK;
        stack_push(s, NULL, &err);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);
        ASSERT_EQ(stack_size(s, NULL), (size_t)0);
        stack_delete(&s, &err, NULL, false);
}

static void test_all_functions_reject_null_stack(void) {
        error_code_t err;
        int dummy = 1;

        err = CESSE_OK; stack_delete(NULL, &err, NULL, false);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);

        err = CESSE_OK; stack_clear(NULL, &err, NULL, false);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);

        err = CESSE_OK; stack_push(NULL, &dummy, &err);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);

        err = CESSE_OK; ASSERT_NULL(stack_pop(NULL, &err));
        ASSERT_EQ(err, CESSE_ERR_NULLARG);

        err = CESSE_OK; ASSERT_NULL(stack_top(NULL, &err));
        ASSERT_EQ(err, CESSE_ERR_NULLARG);

        err = CESSE_OK; ASSERT_EQ(stack_size(NULL, &err), (size_t)0);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);
}

static void test_delete_of_already_nulled_pointer(void) {
        error_code_t err = CESSE_OK;
        Stack* s = stack_new(&err);
        stack_delete(&s, &err, NULL, false);
        ASSERT_NULL(s);
        err = CESSE_OK;
        stack_delete(&s, &err, NULL, false);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);
}

static void test_error_ptr_may_be_null_everywhere(void) {
        Stack* s = stack_new(NULL);
        ASSERT_NOT_NULL(s);
        int v = 1;
        stack_push(s, &v, NULL);
        stack_top(s, NULL);
        stack_size(s, NULL);
        stack_pop(s, NULL);
        stack_pop(s, NULL); /* empty now -- error path with NULL error */
        stack_top(s, NULL); /* empty -- error path with NULL error */
        stack_delete(&s, NULL, NULL, false);
}

static void test_deep_push_pop_cycle(void) {
        /* Stresses malloc/free churn in the linked-list backing --
         * adversarial in the sense of hammering the allocator, not
         * any particular API misuse. */
        error_code_t err = CESSE_OK;
        Stack* s = stack_new(&err);
        int vals[10000];
        for (int round = 0; round < 5; round++) {
                for (int i = 0; i < 10000; i++) {
                        vals[i] = i;
                        stack_push(s, &vals[i], &err);
                }
                ASSERT_EQ(stack_size(s, NULL), (size_t)10000);
                for (int i = 9999; i >= 0; i--) {
                        int* popped = stack_pop(s, &err);
                        ASSERT_EQ(*popped, i);
                }
                ASSERT_EQ(stack_size(s, NULL), (size_t)0);
        }
        stack_delete(&s, &err, NULL, false);
}

int main(void) {
        TEST_INIT();
        RUN(test_top_on_empty_returns_error_not_crash);
        RUN(test_pop_on_empty_returns_error_not_crash);
        RUN(test_top_and_pop_on_empty_after_draining);
        RUN(test_push_null_object);
        RUN(test_all_functions_reject_null_stack);
        RUN(test_delete_of_already_nulled_pointer);
        RUN(test_error_ptr_may_be_null_everywhere);
        RUN(test_deep_push_pop_cycle);
        return TEST_REPORT();
}
