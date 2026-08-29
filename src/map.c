
#include "cesse/bool.h"
#include "cesse/utils.h"
#include "cesse/array.h"
#include "cesse/functions.h"
#include "cesse/map.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

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

Map* map_new(ErrorCode* error) {
	void* ptr = malloc(INITIAL_BUCKET_COUNT * sizeof(map_elem*));
	ERROR_ON_COND(ptr==NULL, error, CESSE_ERR_ALLOC, return NULL;);
	memset(ptr, 0, INITIAL_BUCKET_COUNT * sizeof(map_elem*));
	void* ptrm = malloc(sizeof(Map));
	ERROR_ON_COND(ptrm==NULL, error, CESSE_ERR_ALLOC, {free(ptr); return NULL;});
	Map* map = CAST(ptrm, Map*);
	map->buckets = CAST(ptr, map_elem**);
	map->bucket_count = INITIAL_BUCKET_COUNT;
	map->size = 0;
}

void map_delete(Map** map, ErrorCode* error, function_delete freer);
void map_clear(Map* map, ErrorCode* error, function_delete freer);