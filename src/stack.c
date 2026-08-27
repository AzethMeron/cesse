
#include "cesse/stack.h"
#include "cesse/utils.h"
#include "cesse/bool.h"
#include "cesse/macros.h"
#include "cesse/functions.h"

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CAPACITY (SIZE_MAX)

typedef struct stack_elem {
	struct stack_elem* next;
	void* object;
} stack_elem;

typedef struct Stack {
	size_t size;
	stack_elem* front;
} Stack;

Stack* stack_new(error_code_t* error) {
	void* ptr = malloc(sizeof(Stack));
	ERROR_ON_COND(ptr==NULL, error, CESSE_ERR_ALLOC, return NULL;);
	Stack* stack = CAST(ptr, Stack*);
	stack->size = 0;
	stack->front = NULL;
	return stack;
}

void stack_delete(Stack** stack, error_code_t* error, function_free freer, const bool free_as_fallback) {
	ERROR_ON_COND(stack==NULL, error, CESSE_ERR_NULLARG, return;);
	ERROR_ON_COND((*stack)==NULL, error, CESSE_ERR_NULLARG, return;);
	stack_clear(*stack, error, freer, free_as_fallback);
	free(*stack);
	*stack = NULL;
}

void stack_clear(Stack* stack, error_code_t* error, function_free freer, const bool free_as_fallback) {
	ERROR_ON_COND(stack==NULL, error, CESSE_ERR_NULLARG, return;);
	while(stack->front != NULL) { 
		error_code_t local_err = CESSE_OK;
		void* object = stack_pop(stack, &local_err);
		if(local_err) { // Should be impossible
			SET_ERROR(error, local_err);
			fprintf(stderr, "stack_clear: internal error during cleanup. Continuing either way, but memory leaks are likely.\nError code %d -> %s", local_err, error_code_to_cstring(&local_err)); 
			return; 
		} 
		if(freer) {
			local_err = freer(&object);
			if(local_err) {
				fprintf(stderr, "Error occured in stack_clear with using custom free-er.\nError code %d -> %s. Continuing either way.", local_err, error_code_to_cstring(&local_err));
			}
		} else if (free_as_fallback) {
			free(object);
		}
	}
}

void stack_push(Stack* stack, void* object, error_code_t* error) {
	ERROR_ON_COND(stack==NULL, error, CESSE_ERR_NULLARG, return;);
	ERROR_ON_COND(object==NULL, error, CESSE_ERR_NULLARG, return;);
	ERROR_ON_COND(stack->size == MAX_CAPACITY, error, CESSE_ERR_OVERFLOW, return;);
	void* ptr = malloc(sizeof(stack_elem));
	ERROR_ON_COND(ptr==NULL, error, CESSE_ERR_ALLOC, return;);
	stack_elem* elem = CAST(ptr, stack_elem*);
	elem->object = object;
	elem->next = stack->front;
	stack->front = elem;
	stack->size = stack->size + 1;
}

void* stack_pop(Stack* stack, error_code_t* error) {
	ERROR_ON_COND(stack==NULL, error, CESSE_ERR_NULLARG, return NULL;);
	ERROR_ON_COND(stack->front == NULL, error, CESSE_ERR_EMPTY, return NULL;);
	stack_elem* to_remove = stack->front;
	void* object = to_remove->object;
	stack->front = to_remove->next;
	stack->size = stack->size - 1;
	free(to_remove);
	return object;
}

void* stack_top(Stack* stack, error_code_t* error) {
	ERROR_ON_COND(stack==NULL, error, CESSE_ERR_NULLARG, return NULL;);
	ERROR_ON_COND(stack->front==NULL, error, CESSE_ERR_EMPTY, return NULL;);
	return stack->front->object;
}

size_t stack_size(Stack* stack, error_code_t* error) {
	ERROR_ON_COND(stack==NULL, error, CESSE_ERR_NULLARG, return 0;);
	return stack->size;
}

size_t stack_max_capacity() {
	return MAX_CAPACITY;
}