#ifndef CESSE_MACROS_H
#define CESSE_MACROS_H

/**
* @file macros.h
* @author Jakub Grzana
* @date August 2026
* @brief Preprocessor helpers.
*/

#include <stdio.h>

/**
* Cast obj to type. A thin, named wrapper around a C-style cast, used
* throughout cesse instead of a bare (type)obj for readability at malloc
* call sites (CAST(ptr, Array*) instead of (Array*)ptr).
*
* Time complexity: O(1).
*/
#define CAST(obj,type) ((type) (obj))

/**
* Write val into *ptr, unless ptr is NULL (in which case do nothing).
* This is how every ErrorCode* out-parameter in cesse can safely be
* ignored by passing NULL.
*
* Time complexity: O(1).
*/
#define SET_ERROR(ptr, val) do { if(ptr!=NULL) {*ptr = val;} } while(0)

/**
* If cond is true, report code via SET_ERROR(error_ptr, code) and then
* run cleanup (typically a return statement, optionally preceded by a
* brace-enclosed block doing any necessary teardown first). This is the
* standard argument-validation pattern used at the top of nearly every
* function in cesse.
*
* Time complexity: O(1) plus whatever cleanup itself costs.
*/
#define ERROR_ON_COND(cond, error_ptr, code, cleanup) do { if((cond)) { SET_ERROR(error_ptr, code); cleanup; } } while(0)

/**
* Diagnostic check: if error_ptr is non-NULL and already holds a
* non-CESSE_OK value at the moment a function is entered, print a
* warning to stderr. This catches the common caller mistake of reusing
* an ErrorCode variable across several calls without resetting it to
* CESSE_OK first -- doing so makes a later successful call look like it
* silently inherited an earlier, unrelated failure.
*
* Time complexity: O(1).
*/
#define ASSURE_ERROR_OK(error_ptr) do { if(error_ptr!=NULL) { if(*error_ptr) { fprintf(stderr, "Warning: non-ok error object passed into a function. This makes impossible to pinpoint error-happening point.\n"); } } } while(0)

#endif
