#ifndef CESSE_FUNCTIONS_H
#define CESSE_FUNCTIONS_H

/**
* @file functions.h
* @author Jakub Grzana
* @date August 2026
* @brief Callback contracts (function_delete, function_compare_lt, function_copy)
*        shared across Array, Stack, and Map, plus a couple of ready-made
*        implementations of them.
*/

#include "cesse/bool.h"
#include "cesse/utils.h"

/**
* Contract for a caller-supplied destructor, passed to array_delete,
* stack_delete, map_delete, and their *_clear counterparts, to free the
* objects they were storing.
*
* Implementations receive a pointer to the pointer being freed (not the
* object itself): free(*ptr), then set *ptr = NULL. Expected to return
* CESSE_OK on success, or any other ErrorCode to indicate the object
* couldn't be freed as expected -- the caller reports that failure to
* stderr but does not abort the overall clear/delete for it.
*
* See default_delete_function below for the common case of a plain
* heap-allocated object needing an ordinary free().
*/
typedef ErrorCode (*function_delete)(void**);

/**
* Contract for a caller-supplied "less than" comparison, passed to
* array_sort/sort to order two borrowed objects: implementations should
* return true if left < right, matching strcmp-style ordering semantics
* rather than a three-way result.
*/
typedef bool (*function_compare_lt)(void*, void*);

/**
* Contract for a caller-supplied deep-copy function, passed to
* array_copy/stack_copy/map_copy to produce a new, independent copy of
* each stored object. Implementations should return a newly-allocated
* copy of ptr's pointee on success, or NULL with error set to a
* non-CESSE_OK code on failure.
*
* See default_shallow_copy below for the degenerate case of sharing the
* same object across both containers instead of duplicating it.
*/
typedef void* (*function_copy)(void*, ErrorCode*);

//typedef size_t (*function_hash)(void*);

/**
* A ready-made function_delete for plain heap-allocated objects: frees
* *ptr and sets it to NULL.
*
* Time complexity: O(1).
* \param ptr Pointer to the pointer to free. Note this is void** (a
*        pointer to the stored pointer), matching function_delete's
*        contract, not the object itself.
* \return CESSE_OK on success, or CESSE_ERR_NULLARG if ptr is NULL
*         (nothing is freed in that case).
*/
ErrorCode default_delete_function(void**ptr);

/**
* A ready-made function_copy that shares rather than duplicates: it
* simply returns ptr unchanged. Useful when array_copy/stack_copy/
* map_copy is only being used to produce a new container holding the
* same underlying objects, not independent copies of them.
*
* Time complexity: O(1).
* \param ptr The object to "copy".
* \param error Never set -- this implementation cannot fail. Present
*        only to satisfy function_copy's signature.
* \return ptr, unchanged.
*/
void* default_shallow_copy(void* ptr, ErrorCode* error);

#endif
