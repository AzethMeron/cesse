#include "framework/test.h"
#include "cesse/sort.h"
#include "cesse/utils.h"
#include <stdlib.h>

static bool int_lt(void* a, void* b) { return *(int*)a < *(int*)b; }

static void test_basic_descending_order(void) {
        int vals[] = {9, 3, 7, 1, 8, 2, 5, 4, 6, 0};
        size_t n = sizeof vals / sizeof vals[0];
        void* ptrs[10];
        for (size_t i = 0; i < n; i++) { ptrs[i] = &vals[i]; }
        ErrorCode err = CESSE_OK;
        sort(ptrs, n, int_lt, &err);
        ASSERT_EQ(err, CESSE_OK);
        for (size_t i = 0; i < n; i++) {
                ASSERT_EQ(*(int*)ptrs[i], (int)(9 - i));
        }
}

static void test_edge_sizes(void) {
        int v0[1]; void* p0[1]; ErrorCode err = CESSE_OK;
        sort(p0, 0, int_lt, &err); /* nothing to check, must not crash */
        ASSERT_EQ(err, CESSE_OK);

        int v1 = 5; void* p1[1] = { &v1 };
        sort(p1, 1, int_lt, &err);
        ASSERT_EQ(*(int*)p1[0], 5);

        int v2a = 1, v2b = 2; void* p2[2] = { &v2a, &v2b };
        sort(p2, 2, int_lt, &err);
        ASSERT_EQ(*(int*)p2[0], 2);
        ASSERT_EQ(*(int*)p2[1], 1);

        (void)v0;
}

typedef struct { int key; int pos; } item_t;
static bool item_lt(void* a, void* b) { return ((item_t*)a)->key < ((item_t*)b)->key; }

static void test_stability(void) {
        int keys[] = {3, 1, 3, 2, 1, 3, 2, 1, 3, 2};
        size_t n = sizeof keys / sizeof keys[0];
        item_t items[10];
        void* ptrs[10];
        for (size_t i = 0; i < n; i++) {
                items[i].key = keys[i];
                items[i].pos = (int)i;
                ptrs[i] = &items[i];
        }
        ErrorCode err = CESSE_OK;
        sort(ptrs, n, item_lt, &err);
        ASSERT_EQ(err, CESSE_OK);
        for (size_t i = 1; i < n; i++) {
                item_t* prev = ptrs[i-1];
                item_t* cur = ptrs[i];
                ASSERT_TRUE(prev->key >= cur->key);
                if (prev->key == cur->key) {
                        ASSERT_TRUE(prev->pos < cur->pos);
                }
        }
}

static void test_all_equal(void) {
        int vals[20];
        void* ptrs[20];
        for (int i = 0; i < 20; i++) { vals[i] = 7; ptrs[i] = &vals[i]; }
        ErrorCode err = CESSE_OK;
        sort(ptrs, 20, int_lt, &err);
        ASSERT_EQ(err, CESSE_OK);
        for (int i = 0; i < 20; i++) { ASSERT_EQ(*(int*)ptrs[i], 7); }
}

static void test_null_args(void) {
        ErrorCode err = CESSE_OK;
        int v = 1; void* p[1] = { &v };
        sort(NULL, 1, int_lt, &err);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);
        err = CESSE_OK;
        sort(p, 1, NULL, &err);
        ASSERT_EQ(err, CESSE_ERR_NULLARG);
}

int main(void) {
        TEST_INIT();
        RUN(test_basic_descending_order);
        RUN(test_edge_sizes);
        RUN(test_stability);
        RUN(test_all_equal);
        RUN(test_null_args);
        return TEST_REPORT();
}
