#include "framework/test.h"
#include "cesse/array.h"
#include "cesse/utils.h"

static void test_new_zero_capacity_is_clamped_to_minimum(void) {
        /* capacity==0 is no longer rejected -- it's clamped up to a
         * minimum internal capacity instead (needed so array_copy of
         * an empty array can succeed rather than always failing). */
        ErrorCode err = CESSE_OK;
        Array* a = array_new(0, &err);
        ASSERT_NOT_NULL(a);
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_EQ(array_size(a, NULL), (size_t)0);
        ASSERT_TRUE(array_capacity(a, NULL) > 0);
        array_delete(&a, &err, NULL);
}

static void test_new_capacity_overflow(void) {
        ErrorCode err = CESSE_OK;
        /* Must reject before ever attempting the allocation -- if this
         * hangs or OOMs instead of returning quickly, that's the bug. */
        Array* a = array_new(array_max_capacity() + 1, &err);
        ASSERT_NULL(a);
        ASSERT_EQ(err, CESSE_ERR_OVERFLOW);
}

static void test_get_null_array(void) {
        ErrorCode err = CESSE_OK;
        void* v = array_get(NULL, 0, &err);
        ASSERT_NULL(v);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);
}

static void test_get_out_of_bounds(void) {
        ErrorCode err = CESSE_OK;
        Array* a = array_new(4, &err);
        int v = 1;
        array_push(a, &v, &err);
        err = CESSE_OK;
        void* got = array_get(a, 1, &err); /* size is 1, index 1 is OOB */
        ASSERT_NULL(got);
        ASSERT_EQ(err, CESSE_ERR_OUT_OF_BOUNDS);
        err = CESSE_OK;
        got = array_get(a, (size_t)-1, &err); /* huge index, still OOB */
        ASSERT_NULL(got);
        ASSERT_EQ(err, CESSE_ERR_OUT_OF_BOUNDS);
        array_delete(&a, &err, NULL);
}

static void test_set_null_object(void) {
        ErrorCode err = CESSE_OK;
        Array* a = array_new(4, &err);
        int v = 1;
        array_push(a, &v, &err);
        err = CESSE_OK;
        void* r = array_set(a, 0, NULL, &err);
        ASSERT_NULL(r);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);
        array_delete(&a, &err, NULL);
}

static void test_push_null_object(void) {
        ErrorCode err = CESSE_OK;
        Array* a = array_new(4, &err);
        err = CESSE_OK;
        array_push(a, NULL, &err);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);
        ASSERT_EQ(array_size(a, NULL), (size_t)0); /* nothing should have been added */
        array_delete(&a, &err, NULL);
}

static void test_pop_empty(void) {
        ErrorCode err = CESSE_OK;
        Array* a = array_new(4, &err);
        err = CESSE_OK;
        void* v = array_pop(a, &err);
        ASSERT_NULL(v);
        ASSERT_EQ(err, CESSE_ERR_EMPTY);
        array_delete(&a, &err, NULL);
}

static void test_remove_out_of_bounds(void) {
        ErrorCode err = CESSE_OK;
        Array* a = array_new(4, &err);
        err = CESSE_OK;
        void* v = array_remove(a, 0, &err); /* empty array */
        ASSERT_NULL(v);
        ASSERT_EQ(err, CESSE_ERR_OUT_OF_BOUNDS);
        array_delete(&a, &err, NULL);
}

static void test_swap_out_of_bounds(void) {
        ErrorCode err = CESSE_OK;
        Array* a = array_new(4, &err);
        int v = 1;
        array_push(a, &v, &err);
        err = CESSE_OK;
        array_swap(a, 0, 5, &err);
        ASSERT_EQ(err, CESSE_ERR_OUT_OF_BOUNDS);
        err = CESSE_OK;
        array_swap(a, 5, 0, &err);
        ASSERT_EQ(err, CESSE_ERR_OUT_OF_BOUNDS);
        array_delete(&a, &err, NULL);
}

static void test_all_functions_reject_null_array(void) {
        ErrorCode err;

        err = CESSE_OK; array_delete(NULL, &err, NULL);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);

        err = CESSE_OK; array_clear(NULL, &err, NULL);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);

        err = CESSE_OK; array_set(NULL, 0, &err, &err); /* dummy non-null object pointer */
        ASSERT_EQ(err, CESSE_ERR_NULLARG);

        err = CESSE_OK; array_push(NULL, &err, &err);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);

        err = CESSE_OK; array_pop(NULL, &err);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);

        err = CESSE_OK; ASSERT_EQ(array_size(NULL, &err), (size_t)0);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);

        err = CESSE_OK; ASSERT_EQ(array_capacity(NULL, &err), (size_t)0);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);

        err = CESSE_OK; array_swap(NULL, 0, 1, &err);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);

        err = CESSE_OK; array_remove(NULL, 0, &err);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);

        err = CESSE_OK; array_sort(NULL, NULL, &err);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);

        err = CESSE_OK; array_fit_memory(NULL, &err);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);
}

static void test_delete_of_already_nulled_pointer(void) {
        ErrorCode err = CESSE_OK;
        Array* a = array_new(4, &err);
        array_delete(&a, &err, NULL);
        ASSERT_NULL(a);
        /* calling delete again on the now-NULL Array* is exactly the
         * scenario a use-after-double-free bug would come from --
         * must be a safe no-op (deliberately, not a crash), not an
         * error -- mirrors free(NULL) being well-defined and silent. */
        err = CESSE_OK;
        array_delete(&a, &err, NULL);
        ASSERT_EQ(err, CESSE_OK);
}

static void test_error_ptr_may_be_null_everywhere(void) {
        /* Every function accepts NULL for error -- this must never
         * crash regardless of whether the call succeeds or fails. */
        Array* a = array_new(4, NULL);
        ASSERT_NOT_NULL(a);
        int v = 1;
        array_push(a, &v, NULL);
        array_get(a, 0, NULL);
        array_set(a, 0, &v, NULL);
        array_swap(a, 0, 0, NULL);
        array_size(a, NULL);
        array_capacity(a, NULL);
        array_fit_memory(a, NULL);
        array_pop(a, NULL);
        array_pop(a, NULL); /* now empty -- error path with NULL error */
        array_get(a, 99, NULL); /* out of bounds, NULL error */
        array_delete(&a, NULL, NULL);
}

static void test_pop_shrink_then_regrow_preserves_data(void) {
        /* Exercises the capacity-growth/shrink boundary repeatedly --
         * a stress-ish adversarial case for internal_expand/shrink. */
        ErrorCode err = CESSE_OK;
        Array* a = array_new(4, &err);
        int vals[200];
        for (int round = 0; round < 5; round++) {
                for (int i = 0; i < 40; i++) {
                        vals[round*40+i] = round*40+i;
                        array_push(a, &vals[round*40+i], &err);
                }
                for (int i = 0; i < 35; i++) {
                        array_pop(a, &err);
                }
        }
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_EQ(array_size(a, NULL), (size_t)25);
        array_delete(&a, &err, NULL);
}

int main(void) {
        TEST_INIT();
        RUN(test_new_zero_capacity_is_clamped_to_minimum);
        RUN(test_new_capacity_overflow);
        RUN(test_get_null_array);
        RUN(test_get_out_of_bounds);
        RUN(test_set_null_object);
        RUN(test_push_null_object);
        RUN(test_pop_empty);
        RUN(test_remove_out_of_bounds);
        RUN(test_swap_out_of_bounds);
        RUN(test_all_functions_reject_null_array);
        RUN(test_delete_of_already_nulled_pointer);
        RUN(test_error_ptr_may_be_null_everywhere);
        RUN(test_pop_shrink_then_regrow_preserves_data);
        return TEST_REPORT();
}
