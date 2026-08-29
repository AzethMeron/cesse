#ifndef CESSE_SORT_H
#define CESSE_SORT_H

/**
* @file sort.h
* @author Jakub Grzana
* @date August 2026
* @brief Stable sort over a raw array of linked void* objects
*/

#include "cesse/utils.h"
#include "cesse/functions.h"

#include <stddef.h>

/**
* Merge-sort array of linked objects in non-ascending order (largest first),
* using compare_lt to order elements. Note that objects are not moved, only pointers to them.
*
* Stable: elements that compare equal keep their original relative order.
*
* Time complexity: O(n log n) 
*
* \param begin Pointer to the first element of the array to sort, sorted in place.
* \param length Number of elements at begin. 0 or 1 is a no-op.
* \param compare_lt Comparison function implementing "<" operator; see function_compare_lt's contract. Must NOT be NULL.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG, CESSE_ERR_ALLOC.
*/
void sort(void** begin, const size_t length, function_compare_lt compare_lt, ErrorCode* error);

#endif
