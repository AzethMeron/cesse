
#include "cesse/bool.h"
#include "cesse/utils.h"
#include "cesse/array.h"
#include "cesse/functions.h"
#include "cesse/macros.h"
#include "cesse/map.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static const uint64_t FNV_OFFSET_BASIS = UINT64_C(14695981039346656037);
static const uint64_t FNV_PRIME = UINT64_C(1099511628211);

#define INITIAL_BUCKET_COUNT (16)
#define MAX_LOAD_FACTOR (0.75)

typedef struct map_elem {
	char* key; // owned!
	void* object; // not owned!
	struct map_elem* next;
} map_elem;

typedef struct Map {
	map_elem** buckets;
	size_t bucket_count; // capacity
	size_t size;
} Map;

#define MAX_BUCKET_ARRAY_LENGTH (SIZE_MAX / sizeof(map_elem *))

static uint64_t hash_cstring(const char* cstring) {
	// fnv-1a
	uint64_t hash = FNV_OFFSET_BASIS;
	for(const unsigned char* p = CAST(cstring, const unsigned char*); *p != '\0'; ++p) {
		hash ^= *p;
		hash *= FNV_PRIME;
	}
	return hash;
}

static size_t bucket_index(const char* key, size_t bucket_count) {
	return hash_cstring(key) & (bucket_count - 1);
}

typedef struct find_result {
	map_elem* entry;
	map_elem* prev;
	size_t index;
} find_result;

#define ENTRY_FIND_RESULT(Entry, Prev, Index) ((find_result){.entry=Entry, .prev=Prev, .index=Index})

// Note: sets CESSE_ERR_KEY_NOT_FOUND on a miss unconditionally -- that's
// exactly what map_get/map_remove want to report, but map_contains (where
// "not found" is a normal `false`, not an error) must call this with a
// throwaway local ErrorCode, never the caller's own error pointer directly.
static find_result find_entry(Map* map, const char* key, ErrorCode* error) {
	ERROR_ON_COND(map==NULL, error, CESSE_ERR_NULLARG, return ENTRY_FIND_RESULT(NULL, NULL, 0););
	ERROR_ON_COND(key==NULL, error, CESSE_ERR_NULLARG, return ENTRY_FIND_RESULT(NULL, NULL, 0););
	size_t idx = bucket_index(key, map->bucket_count);
	map_elem* entry = map->buckets[idx];
	map_elem* prev = NULL;
	while(entry != NULL) {
		if(strcmp(entry->key, key) == 0) { return ENTRY_FIND_RESULT(entry, prev, idx); }
		prev = entry;
		entry = entry->next;
	}
	ERROR_ON_COND(true, error, CESSE_ERR_KEY_NOT_FOUND, return ENTRY_FIND_RESULT(NULL, NULL, idx););
}

static void resize_map(Map* map, size_t new_bucket_count, ErrorCode* error) {
	ERROR_ON_COND(map==NULL, error, CESSE_ERR_NULLARG, return;);
	new_bucket_count = fit_power_of_two(new_bucket_count);
	ERROR_ON_COND(new_bucket_count>MAX_BUCKET_ARRAY_LENGTH, error, CESSE_ERR_OVERFLOW, return;);
	void* ptr = malloc(new_bucket_count * sizeof(map_elem*));
	ERROR_ON_COND(ptr==NULL, error, CESSE_ERR_ALLOC, return;);
	memset(ptr, 0, new_bucket_count * sizeof(map_elem*));
	map_elem** new_buckets = CAST(ptr, map_elem**);
	for(size_t i = 0; i < map->bucket_count; ++i) {
		map_elem* entry = map->buckets[i];
		while(entry != NULL) {
			map_elem* next = entry->next;
			size_t idx = bucket_index(entry->key, new_bucket_count);
			entry->next = new_buckets[idx];
			new_buckets[idx] = entry;
			entry = next;
		}
	}
	free(map->buckets);
	map->buckets = new_buckets;
	map->bucket_count = new_bucket_count;
}

// Map owns its keys -- always stores its own copy, never the caller's pointer.
static char* copy_key(const char* key, ErrorCode* error) {
	size_t len = strlen(key) + 1;
	void* ptr = malloc(len);
	ERROR_ON_COND(ptr==NULL, error, CESSE_ERR_ALLOC, return NULL;);
	memcpy(ptr, key, len);
	return CAST(ptr, char*);
}

Map* map_new(ErrorCode* error) {
	ASSURE_ERROR_OK(error);
	void* ptr = malloc(INITIAL_BUCKET_COUNT * sizeof(map_elem*));
	ERROR_ON_COND(ptr==NULL, error, CESSE_ERR_ALLOC, return NULL;);
	memset(ptr, 0, INITIAL_BUCKET_COUNT * sizeof(map_elem*));
	void* ptrm = malloc(sizeof(Map));
	ERROR_ON_COND(ptrm==NULL, error, CESSE_ERR_ALLOC, {free(ptr); return NULL;});
	Map* map = CAST(ptrm, Map*);
	map->buckets = CAST(ptr, map_elem**);
	map->bucket_count = INITIAL_BUCKET_COUNT;
	map->size = 0;
	return map;
}

void map_clear(Map* map, ErrorCode* error, function_delete freer) {
	ASSURE_ERROR_OK(error);
	ERROR_ON_COND(map==NULL, error, CESSE_ERR_NULLARG, return;);
	for(size_t i = 0; i < map->bucket_count; ++i) {
		map_elem* entry = map->buckets[i];
		while(entry != NULL) {
			map_elem* next = entry->next;
			if(freer) {
				ErrorCode local_err = freer(&entry->object);
				if(local_err) {
					fprintf(stderr, "Error occured in map_clear with using custom free-er.\nError code %d -> %s. Continuing either way.\n", local_err, error_code_to_cstring(&local_err));
				}
			}
			free(entry->key);
			free(entry);
			entry = next;
		}
		map->buckets[i] = NULL;
	}
	map->size = 0;
}

void map_delete(Map** map, ErrorCode* error, function_delete freer) {
	ASSURE_ERROR_OK(error);
	ERROR_ON_COND(map==NULL, error, CESSE_ERR_NULLARG, return;);
	if((*map) == NULL) { return; } // deliberate no-op, mirrors array_delete/stack_delete on an already-null pointer
	map_clear(*map, error, freer);
	free((*map)->buckets);
	free(*map);
	*map = NULL;
}

void* map_set(Map* map, const char* key, void* value, ErrorCode* error) {
	ASSURE_ERROR_OK(error);
	ERROR_ON_COND(map==NULL, error, CESSE_ERR_NULLARG, return NULL;);
	ERROR_ON_COND(key==NULL, error, CESSE_ERR_NULLARG, return NULL;);
	ERROR_ON_COND(value==NULL, error, CESSE_ERR_NULLARG, return NULL;);

	if((double)(map->size + 1) / (double)map->bucket_count > MAX_LOAD_FACTOR) {
		ErrorCode resize_err = CESSE_OK;
		resize_map(map, map->bucket_count * 2, &resize_err);
		ERROR_ON_COND(resize_err!=CESSE_OK, error, resize_err, return NULL;);
	}

	ErrorCode local_err = CESSE_OK;
	find_result found = find_entry(map, key, &local_err); // local_err may become KEY_NOT_FOUND here -- expected, not a real failure, never propagated
	if(found.entry != NULL) {
		void* old_value = found.entry->object;
		found.entry->object = value;
		return old_value;
	}

	local_err = CESSE_OK;
	char* key_copy = copy_key(key, &local_err);
	ERROR_ON_COND(key_copy==NULL, error, local_err, return NULL;);

	void* ptr = malloc(sizeof(map_elem));
	ERROR_ON_COND(ptr==NULL, error, CESSE_ERR_ALLOC, {free(key_copy); return NULL;});
	map_elem* new_entry = CAST(ptr, map_elem*);
	new_entry->key = key_copy;
	new_entry->object = value;
	new_entry->next = map->buckets[found.index];
	map->buckets[found.index] = new_entry;
	map->size += 1;
	return NULL; // no previous value -- this was a new key
}

void* map_get(Map* map, const char* key, ErrorCode* error) {
	ASSURE_ERROR_OK(error);
	ERROR_ON_COND(map==NULL, error, CESSE_ERR_NULLARG, return NULL;);
	ERROR_ON_COND(key==NULL, error, CESSE_ERR_NULLARG, return NULL;);
	find_result found = find_entry(map, key, error); // KEY_NOT_FOUND here is exactly what map_get wants to report
	if(found.entry == NULL) { return NULL; }
	return found.entry->object;
}

bool map_contains(Map* map, const char* key, ErrorCode* error) {
	ASSURE_ERROR_OK(error);
	ERROR_ON_COND(map==NULL, error, CESSE_ERR_NULLARG, return false;);
	ERROR_ON_COND(key==NULL, error, CESSE_ERR_NULLARG, return false;);
	ErrorCode local_err = CESSE_OK; // swallow KEY_NOT_FOUND -- not finding it is a normal `false` here, not an error
	find_result found = find_entry(map, key, &local_err);
	return found.entry != NULL;
}

size_t map_size(Map* map, ErrorCode* error) {
	ASSURE_ERROR_OK(error);
	ERROR_ON_COND(map==NULL, error, CESSE_ERR_NULLARG, return 0;);
	return map->size;
}

void* map_remove(Map* map, const char* key, ErrorCode* error) {
	ASSURE_ERROR_OK(error);
	ERROR_ON_COND(map==NULL, error, CESSE_ERR_NULLARG, return NULL;);
	ERROR_ON_COND(key==NULL, error, CESSE_ERR_NULLARG, return NULL;);
	find_result found = find_entry(map, key, error); // KEY_NOT_FOUND here is exactly what map_remove wants to report
	if(found.entry == NULL) { return NULL; }

	if(found.prev == NULL) {
		map->buckets[found.index] = found.entry->next;
	} else {
		found.prev->next = found.entry->next;
	}
	void* value = found.entry->object;
	free(found.entry->key);
	free(found.entry);
	map->size -= 1;
	return value;
}

Array* map_keylist(Map* map, ErrorCode* error) {
	ASSURE_ERROR_OK(error);
	ERROR_ON_COND(map==NULL, error, CESSE_ERR_NULLARG, return NULL;);
	ErrorCode local_err = CESSE_OK;
	Array* result = array_new(map->size > 0 ? map->size : 1, &local_err);
	ERROR_ON_COND(result==NULL, error, local_err, return NULL;);
	for(size_t i = 0; i < map->bucket_count; ++i) {
		map_elem* entry = map->buckets[i];
		while(entry != NULL) {
			char* key_copy = copy_key(entry->key, &local_err);
			if(key_copy == NULL) {
				SET_ERROR(error, local_err);
				array_delete(&result, NULL, default_delete_function);
				return NULL;
			}
			array_push(result, key_copy, &local_err);
			if(local_err) {
				SET_ERROR(error, local_err);
				free(key_copy);
				array_delete(&result, NULL, default_delete_function);
				return NULL;
			}
			entry = entry->next;
		}
	}
	return result;
}

Map* map_copy(Map* map, ErrorCode* error, function_copy copier, function_delete freer) {
	ASSURE_ERROR_OK(error);
	ERROR_ON_COND(map==NULL, error, CESSE_ERR_NULLARG, return NULL;);
	ERROR_ON_COND(copier==NULL, error, CESSE_ERR_NULLARG, return NULL;);
	ERROR_ON_COND(freer==NULL, error, CESSE_ERR_NULLARG, return NULL;);

	ErrorCode local_err = CESSE_OK;
	Map* copy = map_new(&local_err);
	ERROR_ON_COND(copy==NULL, error, local_err, return NULL;);

	for(size_t i = 0; i < map->bucket_count; ++i) {
		map_elem* entry = map->buckets[i];
		while(entry != NULL) {
			void* copied_value = copier(entry->object, &local_err);
			if(!local_err) {
				map_set(copy, entry->key, copied_value, &local_err);
				if(local_err) {
					// map_set failed after copier already succeeded --
					// the copy was never actually stored anywhere, so
					// it's ours to free here, not map_set's: map_set
					// never took ownership of a value it didn't insert.
					freer(&copied_value);
				}
			}
			if(local_err) {
				SET_ERROR(error, local_err);
				ErrorCode fall_err = CESSE_OK;
				map_delete(&copy, &fall_err, freer);
				if(fall_err) {
					fprintf(stderr, "map_copy: error occured during fallback action. Cleanup-after-failed-copy errored out.\nError code %d -> %s\n", fall_err, error_code_to_cstring(&fall_err));
				}
				return NULL;
			}
			entry = entry->next;
		}
	}
	return copy;
}
