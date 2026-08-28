#ifndef CESSE_STACK_H
#define CESSE_STACK_H

#include "cesse/utils.h"
#include "cesse/functions.h"
#include "cesse/bool.h"

#include <stddef.h>

typedef struct Stack Stack;

Stack* stack_new(ErrorCode* error);
void stack_delete(Stack** stack, ErrorCode* error, function_delete freer);
void stack_clear(Stack* stack, ErrorCode* error, function_delete freer);
void stack_push(Stack* stack, void* object, ErrorCode* error);
void* stack_pop(Stack* stack, ErrorCode* error);
void* stack_top(Stack* stack, ErrorCode* error);
size_t stack_size(Stack* stack, ErrorCode* error);
size_t stack_max_capacity();

#endif