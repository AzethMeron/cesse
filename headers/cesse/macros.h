#ifndef CESSE_MACROS_H
#define CESSE_MACROS_H

/**
* @file macros.h
* @author Jakub Grzana
* @date August 2026
* @brief Small preprocessor helpers used throughout cesse's own implementation
*
* These are internal plumbing, not part of the public data-structure API,
* but they're exposed here because function_delete/function_copy
* implementations a user writes will often want the same conventions
* (e.g. SET_ERROR) for consistency with the rest of cesse.
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
* ignored by passing NULL -- SET_ERROR is always used to write to it,
* never a direct assignment.
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
* Called at the top of every public cesse function that takes an
* ErrorCode* parameter (except error_code_to_cstring, whose ErrorCode*
* is an input to decode rather than an output channel, so an
* already-non-OK value there is normal, not a mistake).
*
* Note: if one public cesse function calls another with the same
* error_ptr (e.g. array_delete calling array_clear), a single stale
* value will trigger this warning once per function in that chain, not
* just once -- each layer is checking the same root cause independently.
*
* Time complexity: O(1).
*/
#define ASSURE_ERROR_OK(error_ptr) do { if(error_ptr!=NULL) { if(*error_ptr) { fprintf(stderr, "Warning: non-ok error object passed into a function. This makes impossible to pinpoint error-happening point.\n"); } } } while(0)

#endif
