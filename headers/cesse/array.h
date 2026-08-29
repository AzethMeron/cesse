#ifndef CESSE_ARRAY_H
#define CESSE_ARRAY_H

#include "cesse/bool.h"
#include "cesse/utils.h"
#include "cesse/functions.h"

#include <stddef.h>

typedef struct Array Array;

Array* array_new(size_t capacity, ErrorCode* error);
void array_delete(Array** array, ErrorCode* error, function_delete freer);
void array_clear(Array* array, ErrorCode* error, function_delete freer);
void* array_get(Array* array, const size_t idx, ErrorCode* error);
void* array_set(Array* array, const size_t idx, void* object, ErrorCode* error);
void array_push(Array* array, void* object, ErrorCode* error);
void* array_pop(Array* array, ErrorCode* error);
size_t array_size(Array* array, ErrorCode* error);
size_t array_capacity(Array* array, ErrorCode* error);
void array_swap(Array* array, const size_t first, const size_t second, ErrorCode* error);
void* array_remove(Array* array, const size_t idx, ErrorCode* error);
void array_sort(Array* array, function_compare_lt compare_lt, ErrorCode* error);
void array_fit_memory(Array* array, ErrorCode* error);
size_t array_max_capacity();
Array* array_copy(Array* array, ErrorCode* error, function_copy copier, function_delete freer);

#endif
