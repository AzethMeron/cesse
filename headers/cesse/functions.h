#ifndef CESSE_FUNCTIONS_H
#define CESSE_FUNCTIONS_H

#include "cesse/bool.h"
#include "cesse/utils.h"

typedef ErrorCode (*function_delete)(void**); // implement: free(*ptr); *ptr = NULL; Return 0 on success or smt else on error
typedef bool (*function_compare_lt)(void*, void*); // implement: left < right
typedef void* (*function_copy)(void*, ErrorCode*);
//typedef size_t (*function_hash)(void*);

ErrorCode default_delete_function(void**ptr); // essentially if(ptr != NULL) { free(*ptr); *ptr = NULL; return CESSE_OK; } else { return CESSE_ERR_NULLARG; }
void* default_shallow_copy(void* ptr, ErrorCode* error); // just return ptr; 

#endif
