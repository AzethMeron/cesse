#ifndef CESSE_STACK_H
#define CESSE_STACK_H

#include "cesse/utils.h"
#include "cesse/functions.h"
#include "cesse/bool.h"

#include <stddef.h>

typedef struct Stack Stack;

Stack* stack_new(error_code_t* error);
void stack_delete(Stack** stack, error_code_t* error, function_free freer, const bool free_as_fallback);
void stack_clear(Stack* stack, error_code_t* error, function_free freer, const bool free_as_fallback);
void stack_push(Stack* stack, void* object, error_code_t* error);
void* stack_pop(Stack* stack, error_code_t* error);
void* stack_top(Stack* stack, error_code_t* error);
size_t stack_size(Stack* stack, error_code_t* error);
size_t stack_max_capacity();

#endif