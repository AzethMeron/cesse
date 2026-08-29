#ifndef CESSE_SORT_H
#define CESSE_SORT_H

/**
* @file sort.h
* @author Jakub Grzana
* @date August 2026
* @brief Standalone stable sort over a raw array of borrowed void* objects
*/

#include "cesse/utils.h"
#include "cesse/functions.h"

#include <stddef.h>

/**
* Sort begin[0..length) in descending order (largest first), stably, using
* compare_lt to order elements. This is the same sort array_sort uses
* internally, exposed here to sort a plain C array without needing an
* Array to hold it.
*
* Stable: elements that compare equal keep their original relative order.
*
* Time complexity: O(n log n), unconditionally -- every split is exactly
* balanced, so unlike quicksort there's no input that degrades this, and
* recursion depth is bounded at O(log n) for any input.
* Space complexity: O(n) auxiliary (one scratch buffer, sized to length,
* allocated once and reused for every merge step).
* \param begin Pointer to the first element of the array to sort, sorted in place.
* \param length Number of elements at begin. 0 or 1 is a no-op.
* \param compare_lt Comparison function; see function_compare_lt's contract. Must not be NULL.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG (begin or compare_lt is NULL),
*        CESSE_ERR_ALLOC (the scratch buffer allocation failed).
*/
void sort(void** begin, const size_t length, function_compare_lt compare_lt, ErrorCode* error);

#endif
