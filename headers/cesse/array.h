#ifndef CESSE_ARRAY_H
#define CESSE_ARRAY_H

#include "cesse/bool.h"
#include "cesse/utils.h"
#include "cesse/functions.h"

#include <stddef.h>

typedef struct Array Array;

Array* array_new(size_t capacity, error_code_t* error);
void array_delete(Array** array, error_code_t* error, function_free freer, const bool free_as_fallback);
void array_clear(Array* array, error_code_t* error, function_free freer, const bool free_as_fallback);
void* array_get(Array* array, const size_t idx, error_code_t* error);
void* array_set(Array* array, const size_t idx, void* object, error_code_t* error);
void array_push(Array* array, void* object, error_code_t* error);
void* array_pop(Array* array, error_code_t* error);
size_t array_size(Array* array, error_code_t* error);
size_t array_capacity(Array* array, error_code_t* error);
void array_swap(Array* array, const size_t first, const size_t second, error_code_t* error);
void* array_remove(Array* array, const size_t idx, error_code_t* error);
void array_sort(Array* array, function_lt compare_lt, error_code_t* error);
void array_fit_memory(Array* array, error_code_t* error);
size_t array_max_capacity();

#endif
