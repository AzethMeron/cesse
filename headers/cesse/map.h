#ifndef CESSE_MAP_H
#define CESSE_MAP_H

#include "cesse/bool.h"
#include "cesse/utils.h"
#include "cesse/array.h"
#include "cesse/functions.h"

#include <stddef.h>

// API is here. Implementation will be bit later.
// keys are C-Strings. Keys within map are owned by the map; keys in the Array keylist are copied and owned by the user

typedef struct Map Map;

Map* map_new(ErrorCode* error);
void map_delete(Map** map, ErrorCode* error, function_delete freer);
void map_clear(Map* map, ErrorCode* error, function_delete freer);
void* map_set(Map* map, const char* key,  void* value, ErrorCode* error);
void* map_get(Map* map, const char* key, ErrorCode* error);
bool map_contains(Map* map, const char* key, ErrorCode* error);
size_t map_size(Map* map, ErrorCode* error);
Array* map_keylist(Map* map, ErrorCode* error);
void* map_remove(Map* map, const char* key, ErrorCode* error);
Map* map_copy(Map* map, ErrorCode* error, function_copy copier);

#endif