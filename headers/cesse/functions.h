#ifndef CESSE_FUNCTIONS_H
#define CESSE_FUNCTIONS_H

#include "cesse/bool.h"
#include "cesse/utils.h"

typedef error_code_t (*function_free)(void**); // implement: free(*ptr); *ptr = NULL; Return 0 on success or smt else on error
typedef bool (*function_lt)(void*, void*); // implement: left < right

#endif
