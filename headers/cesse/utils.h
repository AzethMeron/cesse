#ifndef CESSE_UTILS_H
#define CESSE_UTILS_H

/**
* @file utils.h
* @author Jakub Grzana
* @date August 2026
* @brief Shared error-code type and small utility functions used throughout cesse
*/

#include <stdint.h>
#include <stddef.h>

/**
* Error code type used as the ErrorCode* out-parameter across the whole library.
*
* You should ALWAYS create a vessel for the error code like this:
*   ErrorCode error = CESSE_OK;
* then pass its address to functions as necessary. Every ErrorCode* argument
* across cesse can also be safely ignored by passing NULL.
*
* Error codes are set only on error: during normal, successful execution a
* function never touches its error parameter, so initializing your own
* variable to CESSE_OK before the call matters -- otherwise a successful
* call can look like it failed just because of whatever value the
* variable already held. Passing ErrorCode uninitialized or with set error
* will not cause cesse to error out, but it will print warning about it to stderr.
*/
typedef uint16_t ErrorCode;

/**
* The concrete values ErrorCode can hold.
*
* - CESSE_OK: no error; the operation succeeded.
* - CESSE_ERR_ALLOC: a heap allocation (malloc) failed.
* - CESSE_ERR_NULLARG: a required pointer argument was NULL.
* - CESSE_ERR_OUT_OF_BOUNDS: an index argument was outside the valid range.
* - CESSE_ERR_EMPTY: the operation needs at least one element, but the container was empty.
* - CESSE_ERR_BAD_ARG: an argument was structurally invalid (e.g. min > max, or a probability outside [0,1]).
* - CESSE_ERR_OVERFLOW: an internal size/capacity calculation would exceed what's representable.
* - CESSE_ERR_UNDERFLOW: reserved for a size_t computation wrapping below zero; not currently produced anywhere in cesse.
* - CESSE_ERR_KEY_NOT_FOUND: the requested key does not exist in a Map.
*/
typedef enum cesse_error : ErrorCode {
	CESSE_OK = 0,
	CESSE_ERR_ALLOC,
	CESSE_ERR_NULLARG,
	CESSE_ERR_OUT_OF_BOUNDS,
	CESSE_ERR_EMPTY,
	CESSE_ERR_BAD_ARG,
	CESSE_ERR_OVERFLOW,
	CESSE_ERR_UNDERFLOW,
	CESSE_ERR_KEY_NOT_FOUND,
} cesse_error_t;

/**
* Convert an error code into a human-readable, statically-allocated description string.
*
* Time complexity: O(1).
* \param error_code Pointer to the code to describe. Must not be NULL.
* \return A never-NULL, statically-allocated string that must not be
*         free()'d. An unrecognized code also returns a descriptive
*         string rather than crashing.
*/
const char* error_code_to_cstring(const ErrorCode* error_code);

/**
* Round a capacity up to the nearest power of two.
*
* Time complexity: O(1) 
* 
* \param capacity The requested capacity. 0 and 1 both map to 1.
* \return The smallest power of two >= capacity, or 0 if that value would
*         exceed what a size_t can represent. Callers must treat a 0 result as an overflow
*         signal, not a valid capacity of zero.
*/
size_t fit_power_of_two(size_t capacity);

#endif
