#ifndef CESSE_FUNCTIONS_H
#define CESSE_FUNCTIONS_H

/**
* @file functions.h
* @author Jakub Grzana
* @date August 2026
* @brief Function pointer types and contracts.
*/

#include "cesse/bool.h"
#include "cesse/utils.h"

/**
* Contract for a caller-supplied destructor, used f.e. in cesse container 
* *_delete and *_clear functions to free objects stored in them.
*
* Implementation is expected to check whether ptr!=NULL, then free(*ptr); or equivalent,
* then *ptr = NULL; and return CESSE_OK; Returning anything else signals an error.
* cesse containers report such failures to stderr but does not abort the overall clear/delete for it.
*
* See default_delete_function below for the common case of a plain
* heap-allocated object needing an ordinary free().
*/
typedef ErrorCode (*function_delete)(void**);

/**
* Contract for a caller-supplied "less than" comparison (operator <).
* Implementations should return true if left < right.
* Used mainly for sorting, direction can be flipped to achieve ascending order instead of descending.
*/
typedef bool (*function_compare_lt)(void*, void*);

/**
* Contract for a caller-supplied deep-copy function, passed to *_copy functions of cesse containers 
* to produce a new, independent copy of each stored object. 
* Implementations should return a newly-allocated copy of ptr's pointee on success, 
* or NULL with error set to a non-CESSE_OK code on failure.
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
* \param ptr Pointer to the pointer to be freed. 
* \return CESSE_OK on success, or CESSE_ERR_NULLARG if ptr is NULL
*         (nothing is freed in that case).
*/
ErrorCode default_delete_function(void**ptr);

/**
* A ready-made function_copy that shares rather than duplicates: it
* simply returns ptr unchanged. Useful when container is only being used to produce a new container holding the
* same underlying objects, not independent copies of them. Provided for convenience, but real usage is a bad idea messing up ownership.
*
* Time complexity: O(1).
* \param ptr The object to "copy".
* \param error Never set.
* \return ptr, unchanged.
*/
void* default_shallow_copy(void* ptr, ErrorCode* error);

#endif
