#ifndef CESSE_STACK_H
#define CESSE_STACK_H

/**
* @file stack.h
* @author Jakub Grzana
* @date August 2026
* @brief Linked-list LIFO stack that stores borrowed objects (as void*)
*/

#include "cesse/utils.h"
#include "cesse/functions.h"
#include "cesse/bool.h"

#include <stddef.h>

/**
* Stack type (singly-linked list, LIFO).
* 
* Stack stores pointer to objects borrowed from user. By default, nothing
* is automatically freed (user can opt-in by providing function_delete to _delete
* and/or _clear, but it's not recommanded) It's implemented as a one-directional 
* linked list, meaning it's simple and efficient for the task. Gives access to the top 
* element only (which is the most recently pushed one - LIFO)
*/
typedef struct Stack Stack;

/**
* Create a new, empty Stack on the heap and pass ownership to the caller.
*
* Time complexity: O(1).
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_ALLOC.
* \return Pointer to the created Stack, or NULL if an error occurred.
*/
Stack* stack_new(ErrorCode* error);

/**
* Delete a Stack and free its internal nodes.
* Unless function_delete is provided, it does NOT free stored object.
* It's recommanded to first drain the stack and free objects on your own, as this gives better error-handling options.
*
* Time complexity: O(n), where n is the number of elements still stored.
* \param stack Pointer-to-pointer of the stack. Once freed, the pointer
*        is set to NULL (hence the double pointer). Passing a
*        pointer-to-NULL is a safe no-op.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG.
* \param freer Function used to free the objects being removed. Pass NULL to ignore (meaning memory leak if there're objects in container)
*        A failure reported by freer itself is printed to stderr but does not abort the clear (that also will lead to memory leaks).
*/
void stack_delete(Stack** stack, ErrorCode* error, function_delete freer);

/**
* Remove every element from a Stack without deleting the Stack itself,
* leaving it empty and reusable.
*
* Time complexity: O(n), where n is the number of elements.
* \param stack The stack to clear. Must not be NULL.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG.
* \param freer Function used to free the objects being removed. Pass NULL to ignore (meaning memory leak if there're objects in container)
*        A failure reported by freer itself is printed to stderr but does not abort the clear (that also will lead to memory leaks).
*/
void stack_clear(Stack* stack, ErrorCode* error, function_delete freer);

/**
* Push object onto the top of the stack.
*
* Time complexity: O(1).
* \param stack The stack to push onto. Must not be NULL.
* \param object The object to store (borrowed -- ownership stays with the caller unless a freer is used later). Must not be NULL.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG, CESSE_ERR_OVERFLOW, CESSE_ERR_ALLOC
*/
void stack_push(Stack* stack, void* object, ErrorCode* error);

/**
* Remove and return the top object.
*
* Time complexity: O(1).
* \param stack The stack to pop from. Must not be NULL.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG, CESSE_ERR_EMPTY
* \return The object that was on top, now owned by the caller, or NULL if an error occurred.
*/
void* stack_pop(Stack* stack, ErrorCode* error);

/**
* Return the top object without removing it.
*
* Time complexity: O(1).
* \param stack The stack to peek at. Must not be NULL.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG, CESSE_ERR_EMPTY.
* \return The top object or NULL if an error occurred.
*/
void* stack_top(Stack* stack, ErrorCode* error);

/**
* Return the number of elements currently stored.
*
* Time complexity: O(1).
* \param stack The stack to query. Must not be NULL.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG.
* \return The element count, or 0 if an error occurred (0 is valid for empty stack, so use error to distinguish).
*/
size_t stack_size(Stack* stack, ErrorCode* error);

/**
* Return the largest size a Stack can theoretically reach.
*
* Time complexity: O(1).
* \return SIZE_MAX
*/
size_t stack_max_capacity();

/**
* Produce a new Stack holding independent copies of every element,
* using copier to duplicate each one, in the same top-to-bottom order
* as the original. The original stack is left untouched.
*
* On a failure partway through, everything already copied into the new
* stack is cleaned up via freer before returning NULL -- freer is
* therefore required (not optional), since the objects being cleaned up
* were just created by copier, not borrowed from anywhere else that
* might already own them.
*
* Time complexity: O(n), where n is the number of elements.
* 
* \param stack The stack to copy. Must not be NULL.
* \param error Pointer to ErrorCode object, to be populated with error if one occurs. Pass NULL to ignore.
*        Possible codes: CESSE_ERR_NULLARG, CESSE_ERR_ALLOC or whatever
*        code copier itself reports on failure.
* \param copier Function used to duplicate each stored object. Must not be NULL.
* \param freer Function used to clean up already-copied objects if the
*        copy fails partway through. Must not be NULL.
* \return The new, independent Stack, or NULL if an error occurred.
*/
Stack* stack_copy(Stack* stack, ErrorCode* error, function_copy copier, function_delete freer);

#endif
