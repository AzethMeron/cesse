#ifndef CESSE_MAP_H
#define CESSE_MAP_H

#include "cesse/bool.h"
#include "cesse/utils.h"
#include "cesse/array.h"

typedef struct Map Map;

Map* map_new(ErrorCode* error);
void map_delete(Map** map, ErrorCode* error, function_free freer, bool free_as_fallback);
void map_clear(Map* map, ErrorCode* error, function_free freer, bool free_as_fallback);
void* map_set(Map* map, const char* key,  void* value, ErrorCode* error);
void* map_get(Map* map, const char* key, ErrorCode* error);
bool map_contains(Map* map, const char* key, ErrorCode* error);
size_t map_size(Map* map, ErrorCode* error);
Array* map_keylist(Map* map, ErrorCode* error);
void* map_remove(Map* map, const char* key, ErrorCode* error);

#endif