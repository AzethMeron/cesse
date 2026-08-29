#ifndef CESSE_ARRAY_H
#define CESSE_ARRAY_H

/**
* @file array.h
* @author Jakub Grzana
* @date August 2026
* @brief Dynamic array that can store borrowed objects (as void*)
*/

#include "cesse/bool.h"
#include "cesse/utils.h"
#include "cesse/functions.h"

#include <stddef.h>

/** 
* Dynamic Array type
*
* Array stores pointers to borrowed objects (by default, those are not freed - however user can opt-in for that by providing 
* function_delete compatible function pointer to clear/delete functions). Array reallocs only when capacity breaches power-of-two threshold
* keeping push, pop function at amortized O(1) time complexity.
*/
typedef struct Array Array;

/**
* Create a new Array object on heap and pass ownership to user.
* \param capacity Target capacity - note it's in the end bumped to nearest power-of-two.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
* \return Pointer to created object, or NULL if error occured.
*/
Array* array_new(size_t capacity, ErrorCode* error);

/**
* Delete created array and free its internals.
* Unless function_delete is provided, it does NOT free stored object.
* It's recommanded to first drain the array with array_pop function and free objects on your own, as this gives better error-handling 
* options.
* \param array Pointer-to-pointer of array object. Once freed, pointer is set to NULL (thus double pointer is necessary)
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
* \param function_delete Pointer to function used to free objects still stored in Array. Pass NULL to ignore.
*/
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
