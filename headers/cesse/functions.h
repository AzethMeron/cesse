#ifndef CESSE_FUNCTIONS_H
#define CESSE_FUNCTIONS_H

#include "cesse/bool.h"
#include "cesse/utils.h"

typedef ErrorCode (*function_delete)(void**); // implement: free(*ptr); *ptr = NULL; Return 0 on success or smt else on error
typedef bool (*function_compare_lt)(void*, void*); // implement: left < right
typedef void* (*function_copy)(void*);
//typedef size_t (*function_hash)(void*);

ErrorCode default_delete_function(void**ptr); 

#endif
