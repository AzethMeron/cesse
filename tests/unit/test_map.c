#include "framework/test.h"
#include "cesse/map.h"
#include "cesse/array.h"
#include "cesse/utils.h"
#include <stdlib.h>
#include <string.h>

static void test_new_delete(void) {
        ErrorCode err = CESSE_OK;
        Map* m = map_new(&err);
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_NOT_NULL(m);
        ASSERT_EQ(map_size(m, NULL), (size_t)0);
        map_delete(&m, &err, NULL);
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_NULL(m);
}

static void test_set_new_key_returns_null(void) {
        ErrorCode err = CESSE_OK;
        Map* m = map_new(&err);
        int v = 1;
        void* old = map_set(m, "a", &v, &err);
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_NULL(old);
        ASSERT_EQ(map_size(m, NULL), (size_t)1);
        map_delete(&m, &err, NULL);
}

static void test_set_existing_key_returns_previous(void) {
        ErrorCode err = CESSE_OK;
        Map* m = map_new(&err);
        int v1 = 1, v2 = 2;
        map_set(m, "a", &v1, &err);
        void* old = map_set(m, "a", &v2, &err);
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_TRUE(old == &v1);
        ASSERT_EQ(map_size(m, NULL), (size_t)1); /* still one key, just updated */
        ASSERT_TRUE(map_get(m, "a", NULL) == &v2);
        map_delete(&m, &err, NULL);
}

static void test_get_and_contains(void) {
        ErrorCode err = CESSE_OK;
        Map* m = map_new(&err);
        int v = 42;
        map_set(m, "key", &v, &err);

        ASSERT_TRUE(map_contains(m, "key", &err));
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_FALSE(map_contains(m, "missing", &err));
        ASSERT_EQ(err, CESSE_OK); /* contains() reports absence via false, not an error */

        void* got = map_get(m, "key", &err);
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_TRUE(got == &v);

        map_delete(&m, &err, NULL);
}

static void test_get_missing_key_reports_error(void) {
        ErrorCode err = CESSE_OK;
        Map* m = map_new(&err);
        void* got = map_get(m, "nope", &err);
        ASSERT_NULL(got);
        ASSERT_EQ(err, CESSE_ERR_KEY_NOT_FOUND);
        map_delete(&m, &err, NULL);
}

static void test_remove(void) {
        ErrorCode err = CESSE_OK;
        Map* m = map_new(&err);
        int v = 7;
        map_set(m, "key", &v, &err);
        ASSERT_EQ(map_size(m, NULL), (size_t)1);

        void* removed = map_remove(m, "key", &err);
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_TRUE(removed == &v);
        ASSERT_EQ(map_size(m, NULL), (size_t)0);
        ASSERT_FALSE(map_contains(m, "key", NULL));

        /* removing again -- now genuinely missing */
        err = CESSE_OK;
        void* removed2 = map_remove(m, "key", &err);
        ASSERT_NULL(removed2);
        ASSERT_EQ(err, CESSE_ERR_KEY_NOT_FOUND);

        map_delete(&m, &err, NULL);
}

static void test_key_is_copied_not_borrowed(void) {
        /* mutating (or freeing) the caller's own key buffer after
         * set() must not affect the map -- it must have made its own
         * copy, not stored the pointer. */
        ErrorCode err = CESSE_OK;
        Map* m = map_new(&err);
        char* key = malloc(8);
        strcpy(key, "mutable");
        int v = 1;
        map_set(m, key, &v, &err);
        strcpy(key, "changed");
        free(key);

        ASSERT_TRUE(map_contains(m, "mutable", NULL));
        ASSERT_FALSE(map_contains(m, "changed", NULL));

        map_delete(&m, &err, NULL);
}

static void test_grows_past_initial_bucket_count(void) {
        /* Enough insertions to force at least one resize -- correctness
         * of every key/value pair must survive rehashing. */
        ErrorCode err = CESSE_OK;
        Map* m = map_new(&err);
        int vals[200];
        char keys[200][16];
        for (int i = 0; i < 200; i++) {
                vals[i] = i;
                snprintf(keys[i], sizeof(keys[i]), "key%d", i);
                map_set(m, keys[i], &vals[i], &err);
                ASSERT_EQ(err, CESSE_OK);
        }
        ASSERT_EQ(map_size(m, NULL), (size_t)200);
        bool all_correct = true;
        for (int i = 0; i < 200; i++) {
                int* got = map_get(m, keys[i], NULL);
                if (got == NULL || *got != i) { all_correct = false; }
        }
        ASSERT_TRUE(all_correct);
        map_delete(&m, &err, NULL);
}

static void test_keylist_contains_every_key_and_is_owned_copies(void) {
        ErrorCode err = CESSE_OK;
        Map* m = map_new(&err);
        int v1 = 1, v2 = 2, v3 = 3;
        map_set(m, "a", &v1, &err);
        map_set(m, "b", &v2, &err);
        map_set(m, "c", &v3, &err);

        Array* keys = map_keylist(m, &err);
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_EQ(array_size(keys, NULL), (size_t)3);

        bool has_a = false, has_b = false, has_c = false;
        for (size_t i = 0; i < 3; i++) {
                char* k = array_get(keys, i, NULL);
                if (strcmp(k, "a") == 0) has_a = true;
                if (strcmp(k, "b") == 0) has_b = true;
                if (strcmp(k, "c") == 0) has_c = true;
        }
        ASSERT_TRUE(has_a && has_b && has_c);

        /* caller owns these strings -- must be safe to free them */
        array_delete(&keys, &err, default_delete_function);
        map_delete(&m, &err, NULL);
}

static void test_clear(void) {
        ErrorCode err = CESSE_OK;
        Map* m = map_new(&err);
        int v = 1;
        map_set(m, "a", &v, &err);
        map_set(m, "b", &v, &err);
        map_clear(m, &err, NULL);
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_EQ(map_size(m, NULL), (size_t)0);
        ASSERT_FALSE(map_contains(m, "a", NULL));
        /* map remains usable after clear */
        map_set(m, "c", &v, &err);
        ASSERT_EQ(map_size(m, NULL), (size_t)1);
        map_delete(&m, &err, NULL);
}

static int free_call_count = 0;
static ErrorCode counting_freer(void** obj) {
        (void)obj;
        free_call_count++;
        return CESSE_OK;
}

static void test_delete_calls_custom_freer_on_values(void) {
        free_call_count = 0;
        ErrorCode err = CESSE_OK;
        Map* m = map_new(&err);
        int v1 = 1, v2 = 2;
        map_set(m, "a", &v1, &err);
        map_set(m, "b", &v2, &err);
        map_delete(&m, &err, counting_freer);
        ASSERT_EQ(free_call_count, 2);
}

static void* int_copy(void* obj, ErrorCode* error) {
        (void)error;
        int* c = malloc(sizeof(int));
        *c = *(int*)obj;
        return c;
}

static void test_copy_is_independent(void) {
        ErrorCode err = CESSE_OK;
        Map* m = map_new(&err);
        int v1 = 10, v2 = 20;
        map_set(m, "a", &v1, &err);
        map_set(m, "b", &v2, &err);

        Map* copy = map_copy(m, &err, int_copy, default_delete_function);
        ASSERT_EQ(err, CESSE_OK);
        ASSERT_EQ(map_size(copy, NULL), (size_t)2);

        int* copy_a = map_get(copy, "a", NULL);
        ASSERT_NOT_NULL(copy_a);
        ASSERT_EQ(*copy_a, 10);
        ASSERT_TRUE(copy_a != &v1); /* genuinely a different object */

        map_delete(&m, &err, NULL);
        map_delete(&copy, &err, default_delete_function);
}

int main(void) {
        TEST_INIT();
        RUN(test_new_delete);
        RUN(test_set_new_key_returns_null);
        RUN(test_set_existing_key_returns_previous);
        RUN(test_get_and_contains);
        RUN(test_get_missing_key_reports_error);
        RUN(test_remove);
        RUN(test_key_is_copied_not_borrowed);
        RUN(test_grows_past_initial_bucket_count);
        RUN(test_keylist_contains_every_key_and_is_owned_copies);
        RUN(test_clear);
        RUN(test_delete_calls_custom_freer_on_values);
        RUN(test_copy_is_independent);
        return TEST_REPORT();
}
