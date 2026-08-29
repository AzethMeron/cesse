#include "framework/test.h"
#include "cesse/map.h"
#include "cesse/rng.h"
#include "cesse/utils.h"
#include <stdlib.h>
#include <string.h>

/* Shadow model: a plain array of (key, value) pairs. Deliberately not
 * itself a hash table, so it can't share a bug with the real Map --
 * lookups here are linear scans over a small, fixed key vocabulary.
 * Slots are reused: `present` tracks which of the fixed VOCAB_SIZE
 * slots currently holds a live key, not how many inserts have ever
 * happened (which would be unbounded over many remove+reinsert
 * cycles on the same key). */
typedef struct { char key[16]; int value; bool present; } shadow_entry_t;

#define VOCAB_SIZE 40
#define MAX_ENTRIES VOCAB_SIZE

static shadow_entry_t* shadow_find(shadow_entry_t* shadow, const char* key) {
        for (int i = 0; i < MAX_ENTRIES; i++) {
                if (shadow[i].present && strcmp(shadow[i].key, key) == 0) { return &shadow[i]; }
        }
        return NULL;
}

static shadow_entry_t* shadow_find_free_slot(shadow_entry_t* shadow) {
        for (int i = 0; i < MAX_ENTRIES; i++) {
                if (!shadow[i].present) { return &shadow[i]; }
        }
        return NULL; /* can't happen: map_size is bounded by VOCAB_SIZE, same as this shadow */
}

static void test_map_fuzz_against_shadow(void) {
        /* This variable is deliberately re-initialized to CESSE_OK
         * immediately before every single call below, never left to
         * carry a stale value from the previous operation forward --
         * cesse's own convention is "error codes are set only on
         * error", so a leftover CESSE_ERR_KEY_NOT_FOUND from a prior
         * get/remove would otherwise look like the next, unrelated
         * call failed. */
        ErrorCode err = CESSE_OK;
        Rng* rng = rng_new(778899ULL, &err);
        Map* map = map_new(&err);

        shadow_entry_t shadow[MAX_ENTRIES];
        memset(shadow, 0, sizeof(shadow));

        int* pool = malloc(500000 * sizeof(int));
        size_t pool_next = 0;

        long iterations = 100000;
        for (long iter = 0; iter < iterations; iter++) {
                err = CESSE_OK;
                int vocab_idx = (int)dist_uniform_i64(rng, 0, VOCAB_SIZE - 1, &err);
                char key[16];
                snprintf(key, sizeof(key), "key%d", vocab_idx);

                err = CESSE_OK;
                int op = (int)dist_uniform_i64(rng, 0, 3, &err);
                shadow_entry_t* existing = shadow_find(shadow, key);

                if (op == 0) {
                        /* set */
                        err = CESSE_OK;
                        int v = (int)dist_uniform_i64(rng, -1000000, 1000000, &err);
                        pool[pool_next] = v;
                        err = CESSE_OK;
                        void* old = map_set(map, key, &pool[pool_next], &err);
                        ASSERT_EQ(err, CESSE_OK);
                        pool_next++;
                        if (existing) {
                                ASSERT_TRUE(old != NULL);
                                ASSERT_EQ(*(int*)old, existing->value);
                                existing->value = v;
                        } else {
                                ASSERT_NULL(old);
                                shadow_entry_t* slot = shadow_find_free_slot(shadow);
                                strncpy(slot->key, key, sizeof(slot->key) - 1);
                                slot->value = v;
                                slot->present = true;
                        }
                } else if (op == 1) {
                        /* get */
                        err = CESSE_OK;
                        int* got = map_get(map, key, &err);
                        if (existing) {
                                ASSERT_EQ(err, CESSE_OK);
                                ASSERT_NOT_NULL(got);
                                ASSERT_EQ(*got, existing->value);
                        } else {
                                ASSERT_EQ(err, CESSE_ERR_KEY_NOT_FOUND);
                                ASSERT_NULL(got);
                        }
                } else if (op == 2) {
                        /* contains */
                        err = CESSE_OK;
                        bool has = map_contains(map, key, &err);
                        ASSERT_EQ(err, CESSE_OK);
                        ASSERT_EQ(has, existing != NULL);
                } else {
                        /* remove */
                        err = CESSE_OK;
                        void* removed = map_remove(map, key, &err);
                        if (existing) {
                                ASSERT_EQ(err, CESSE_OK);
                                ASSERT_NOT_NULL(removed);
                                ASSERT_EQ(*(int*)removed, existing->value);
                                existing->present = false;
                        } else {
                                ASSERT_EQ(err, CESSE_ERR_KEY_NOT_FOUND);
                                ASSERT_NULL(removed);
                        }
                }

                size_t expected_size = 0;
                for (int i = 0; i < MAX_ENTRIES; i++) { if (shadow[i].present) { expected_size++; } }
                ASSERT_EQ(map_size(map, NULL), expected_size);
        }

        map_delete(&map, &err, NULL);
        rng_delete(&rng, &err);
        free(pool);
}

int main(void) {
        TEST_INIT();
        RUN(test_map_fuzz_against_shadow);
        return TEST_REPORT();
}
