#ifndef CESSE_ARRAY_H
#define CESSE_ARRAY_H

/**
* @file array.h
* @author Jakub Grzana
* @date August 2026
* @brief Dynamic array that stores borrowed objects (as void*)
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
*
* Time complexity: O(n), where n is the resulting capacity (the buffer
* is zero-initialized).
* \param capacity Target capacity, though it's clamped to the next power of two or to internal minimum of 8, whichever is higher.
         So it's more of suggestion.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_OVERFLOW, CESSE_ERR_ALLOC.
* \return Pointer to created object, or NULL if error occured.
*/
Array* array_new(size_t capacity, ErrorCode* error);

/**
* Delete created array and free its internals.
* Unless function_delete is provided, it does NOT free stored object.
* It's recommanded to first drain the array and free objects on your own, as this gives better error-handling options.
*
* Time complexity: O(n), where n is the number of elements still stored.
* \param array Pointer-to-pointer of array object. Once freed, pointer is set to NULL (thus double pointer is necessary).
*        Passing a pointer-to-NULL is a safe no-op.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG.
* \param freer Function used to free the objects being removed. Pass NULL to ignore (meaning memory leak if there're objects in container)
*        A failure reported by freer itself is printed to stderr but does not abort the clear (that also will lead to memory leaks).
*/
void array_delete(Array** array, ErrorCode* error, function_delete freer);

/**
* Remove every element from an Array without deleting the Array itself,
* leaving it empty and reusable. Capacity may shrink as elements are
* removed (see array_fit_memory for forcing this explicitly).
*
* Time complexity: O(n), where n is the number of elements.
* \param array The array to clear. Must not be NULL.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG.
* \param freer Function used to free the objects being removed. Pass NULL to ignore (meaning memory leak if there're objects in container)
*        A failure reported by freer itself is printed to stderr but does not abort the clear (that also will lead to memory leaks).
*/
void array_clear(Array* array, ErrorCode* error, function_delete freer);

/**
* Return the object stored at idx.
*
* Time complexity: O(1).
* \param array The array to read from. Must not be NULL.
* \param idx Index to read. Must be < array_size(array).
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG, CESSE_ERR_OUT_OF_BOUNDS.
* \return The stored object, or NULL if an error occurred.
*/
void* array_get(Array* array, const size_t idx, ErrorCode* error);

/**
* Overwrite the object at idx, returning whatever was previously there.
*
* Time complexity: O(1).
* \param array The array to modify. Must not be NULL.
* \param idx Index to overwrite. Must be < array_size(array).
* \param object The new object to store (borrowed). Must not be NULL.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG, CESSE_ERR_OUT_OF_BOUNDS.
* \return The object that was previously at idx or NULL if an error occurred.
*/
void* array_set(Array* array, const size_t idx, void* object, ErrorCode* error);

/**
* Append object to the end of the array, growing capacity if necessary.
*
* Time complexity: amortized O(1); O(n) if resize needed
* \param array The array to push onto. Must not be NULL.
* \param object The object to store (borrowed). Must not be NULL.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG, CESSE_ERR_OVERFLOW, CESSE_ERR_ALLOC
*/
void array_push(Array* array, void* object, ErrorCode* error); // object is borrowed

/**
* Remove and return the last element. This is the default, cheap way to
* drain an array one element at a time (compare array_remove, which
* additionally supports removing from the middle at higher cost).
*
* Time complexity: amortized O(1); O(n) if resize occurs
*
* \param array The array to pop from. Must not be NULL.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG, CESSE_ERR_EMPTY, CESSE_ERR_ALLOC 
* \return The removed object or NULL if an error occurred.
*/
void* array_pop(Array* array, ErrorCode* error); // doesn't free memory! Also this is the default way of emptying array

/**
* Return the number of elements currently stored.
*
* Time complexity: O(1).
* \param array The array to query. Must not be NULL.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG.
* \return The element count, or 0 if an error occurred (0 is otherwise a
*         perfectly valid, non-error result too, for a genuinely empty array).
*/
size_t array_size(Array* array, ErrorCode* error);

/**
* Return the array's current storage capacity (>= array_size(array)).
*
* Time complexity: O(1).
* \param array The array to query. Must not be NULL.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG.
* \return The current capacity, or 0 if an error occurred.
*/
size_t array_capacity(Array* array, ErrorCode* error);

/**
* Swap the objects at two indices.
*
* Time complexity: O(1).
* \param array The array to modify. Must not be NULL.
* \param first First index. Must be < array_size(array).
* \param second Second index. Must be < array_size(array). May equal first (a no-op in that case).
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG, CESSE_ERR_OUT_OF_BOUNDS (either index).
*/
void array_swap(Array* array, const size_t first, const size_t second, ErrorCode* error);

/**
* Remove and return the object at idx, shifting subsequent elements down
* to close the gap. Removing the last index delegates to array_pop, at
* array_pop's cheaper cost.
*
* Time complexity: O(n) where n is the number of elements after idx that must shift.
*
* \param array The array to modify. Must not be NULL.
* \param idx Index to remove. Must be < array_size(array).
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG, CESSE_ERR_OUT_OF_BOUNDS, CESSE_ERR_ALLOC
* \return The removed object, now owned by the caller, or NULL if an error occurred.
*/
void* array_remove(Array* array, const size_t idx, ErrorCode* error);

/**
* Sort the array's elements in place, in descending order (largest
* first), stably, using compare_lt to order elements.
* Stable-sort the array's elements in place in non-ascending order (largest first), using compare_lt to order elements.
*
* Time complexity: O(n log n).
* Space complexity: O(n) auxiliary.
*
* \param array The array to sort. Must not be NULL.
* \param compare_lt Comparison function; see function_compare_lt's contract. Must not be NULL.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG, CESSE_ERR_ALLOC
*/
void array_sort(Array* array, function_compare_lt compare_lt, ErrorCode* error);

/**
* Shrink the array's capacity down to fit its current size (or the
* library's internal minimum, whichever is larger), reclaiming unused
* memory. A no-op if there's nothing worth reclaiming.
*
* Time complexity: O(n), where n is the resulting capacity
*
* \param array The array to shrink. Must not be NULL.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG, CESSE_ERR_ALLOC.
*/
void array_fit_memory(Array* array, ErrorCode* error);

/**
* Return the largest capacity an Array can theoretically reach.
*
* Time complexity: O(1).
* \return SIZE_MAX / sizeof(void*) -- the largest capacity whose backing
*         buffer size (capacity * sizeof(void*)) doesn't itself overflow size_t.
*/
size_t array_max_capacity();

/**
* Produce a new Array holding independent copies of every element, in
* the same order, using copier to duplicate each one. The original
* array is left untouched.
*
* On a failure partway through, everything already copied into the new
* array is cleaned up via freer before returning NULL -- freer is
* therefore required (not optional), since the objects being cleaned up
* were just created by copier, not borrowed from anywhere else that
* might already own them. Recovery is best-effor but no-guarantee.
*
* Time complexity: O(n), where n is the number of elements.
* \param array The array to copy. Must not be NULL.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG, CESSE_ERR_OVERFLOW, CESSE_ERR_ALLOC, whatever code copier itself reports on failure.
* \param copier Function used to duplicate each stored object. Must not be NULL.
* \param freer Function used to clean up already-copied objects if the
*        copy fails partway through. Must not be NULL.
* \return The new, independent Array, or NULL if an error occurred.
*/
Array* array_copy(Array* array, ErrorCode* error, function_copy copier, function_delete freer);

#endif
