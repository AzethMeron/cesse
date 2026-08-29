#include "framework/test.h"
#include "cesse/map.h"
#include "cesse/array.h"
#include "cesse/functions.h"
#include "cesse/utils.h"
#include <stdlib.h>

static void test_all_functions_reject_null_map(void) {
        ErrorCode err;
        int dummy = 1;

        err = CESSE_OK; map_delete(NULL, &err, NULL);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);

        err = CESSE_OK; map_clear(NULL, &err, NULL);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);

        err = CESSE_OK; map_set(NULL, "k", &dummy, &err);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);

        err = CESSE_OK; ASSERT_NULL(map_get(NULL, "k", &err));
        ASSERT_EQ(err, CESSE_ERR_NULLARG);

        err = CESSE_OK; ASSERT_FALSE(map_contains(NULL, "k", &err));
        ASSERT_EQ(err, CESSE_ERR_NULLARG);

        err = CESSE_OK; ASSERT_EQ(map_size(NULL, &err), (size_t)0);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);

        err = CESSE_OK; ASSERT_NULL(map_keylist(NULL, &err));
        ASSERT_EQ(err, CESSE_ERR_NULLARG);

        err = CESSE_OK; ASSERT_NULL(map_remove(NULL, "k", &err));
        ASSERT_EQ(err, CESSE_ERR_NULLARG);
}

static void test_set_rejects_null_key_and_null_value(void) {
        ErrorCode err = CESSE_OK;
        Map* m = map_new(&err);
        int v = 1;

        err = CESSE_OK;
        map_set(m, NULL, &v, &err);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);

        err = CESSE_OK;
        map_set(m, "key", NULL, &err);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);
        ASSERT_EQ(map_size(m, NULL), (size_t)0); /* neither call should have inserted anything */

        map_delete(&m, &err, NULL);
}

static void test_get_contains_remove_reject_null_key(void) {
        ErrorCode err = CESSE_OK;
        Map* m = map_new(&err);

        err = CESSE_OK; ASSERT_NULL(map_get(m, NULL, &err));
        ASSERT_EQ(err, CESSE_ERR_NULLARG);

        err = CESSE_OK; ASSERT_FALSE(map_contains(m, NULL, &err));
        ASSERT_EQ(err, CESSE_ERR_NULLARG);

        err = CESSE_OK; ASSERT_NULL(map_remove(m, NULL, &err));
        ASSERT_EQ(err, CESSE_ERR_NULLARG);

        map_delete(&m, &err, NULL);
}

static void test_delete_of_already_nulled_pointer(void) {
        ErrorCode err = CESSE_OK;
        Map* m = map_new(&err);
        map_delete(&m, &err, NULL);
        ASSERT_NULL(m);
        /* deliberate no-op, mirrors array_delete/stack_delete */
        err = CESSE_OK;
        map_delete(&m, &err, NULL);
        ASSERT_EQ(err, CESSE_OK);
}

static void test_copy_rejects_null_copier_and_null_freer(void) {
        ErrorCode err = CESSE_OK;
        Map* m = map_new(&err);
        int v = 1;
        map_set(m, "a", &v, &err);

        err = CESSE_OK;
        Map* copy = map_copy(m, &err, NULL, default_delete_function);
        ASSERT_NULL(copy);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);

        err = CESSE_OK;
        copy = map_copy(m, &err, default_shallow_copy, NULL);
        ASSERT_NULL(copy);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);

        map_delete(&m, &err, NULL);
}

static int copier_call_count = 0;
static void* failing_on_third_copier(void* obj, ErrorCode* error) {
        (void)obj;
        copier_call_count++;
        if (copier_call_count == 3) { *error = CESSE_ERR_ALLOC; return NULL; }
        int* c = malloc(sizeof(int));
        *c = *(int*)obj;
        return c;
}

static void test_copy_propagates_mid_copy_failure(void) {
        ErrorCode err = CESSE_OK;
        Map* m = map_new(&err);
        int vals[5] = {1, 2, 3, 4, 5};
        char keys[5][8] = {"a", "b", "c", "d", "e"};
        for (int i = 0; i < 5; i++) { map_set(m, keys[i], &vals[i], &err); }

        copier_call_count = 0;
        err = CESSE_OK;
        Map* copy = map_copy(m, &err, failing_on_third_copier, default_delete_function);
        ASSERT_NULL(copy);
        ASSERT_NE(err, CESSE_OK); /* the real failure must be reported, not silently CESSE_OK */

        map_delete(&m, &err, NULL);
}

static void test_error_ptr_may_be_null_everywhere(void) {
        Map* m = map_new(NULL);
        ASSERT_NOT_NULL(m);
        int v = 1;
        map_set(m, "a", &v, NULL);
        map_get(m, "a", NULL);
        map_get(m, "missing", NULL); /* error path with NULL error */
        map_contains(m, "a", NULL);
        map_size(m, NULL);
        Array* keys = map_keylist(m, NULL);
        array_delete(&keys, NULL, default_delete_function);
        map_remove(m, "a", NULL);
        map_remove(m, "a", NULL); /* error path with NULL error */
        map_delete(&m, NULL, NULL);
}

int main(void) {
        TEST_INIT();
        RUN(test_all_functions_reject_null_map);
        RUN(test_set_rejects_null_key_and_null_value);
        RUN(test_get_contains_remove_reject_null_key);
        RUN(test_delete_of_already_nulled_pointer);
        RUN(test_copy_rejects_null_copier_and_null_freer);
        RUN(test_copy_propagates_mid_copy_failure);
        RUN(test_error_ptr_may_be_null_everywhere);
        return TEST_REPORT();
}
