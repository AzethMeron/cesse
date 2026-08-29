#include "framework/test.h"
#include "cesse/functions.h"
#include "cesse/utils.h"
#include <stdlib.h>

static void test_frees_and_nulls_out(void) {
        int* p = malloc(sizeof(int));
        *p = 42;
        void* vp = p;
        ErrorCode err = default_delete_function(&vp);
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_NULL(vp);
        /* Correctness of the free() itself is really "ASan doesn't
         * report a leak or a double-free", checked at process level. */
}

static void test_rejects_null_ptr_argument(void) {
        ErrorCode err = default_delete_function(NULL);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);
}

static void test_usable_as_function_delete_callback(void) {
        /* The whole point of this function is to be passable wherever
         * a function_delete is expected (array_delete/array_clear/
         * stack_delete/stack_clear's freer parameter) -- this is a
         * compile-time check as much as a runtime one: if the
         * signature ever drifts from function_delete's shape, this
         * assignment stops compiling. */
        function_delete fn = default_delete_function;
        int* p = malloc(sizeof(int));
        void* vp = p;
        ErrorCode err = fn(&vp);
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_NULL(vp);
}

static void test_shallow_copy_returns_same_pointer(void) {
        int v = 7;
        ErrorCode err = CESSE_OK;
        void* result = default_shallow_copy(&v, &err);
        ASSERT_TRUE(result == &v); /* aliases, doesn't duplicate -- that's the point */
        ASSERT_EQ(err, CESSE_OK);
}

static void test_shallow_copy_usable_as_function_copy_callback(void) {
        function_copy fn = default_shallow_copy;
        int v = 7;
        ErrorCode err = CESSE_OK;
        ASSERT_TRUE(fn(&v, &err) == &v);
}

int main(void) {
        TEST_INIT();
        RUN(test_frees_and_nulls_out);
        RUN(test_rejects_null_ptr_argument);
        RUN(test_usable_as_function_delete_callback);
        RUN(test_shallow_copy_returns_same_pointer);
        RUN(test_shallow_copy_usable_as_function_copy_callback);
        return TEST_REPORT();
}
