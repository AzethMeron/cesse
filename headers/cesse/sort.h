#ifndef CESSE_SORT_H
#define CESSE_SORT_H

#include "cesse/utils.h"
#include "cesse/functions.h"

#include <stddef.h>

void sort(void** begin, const size_t length, function_lt compare_lt, error_code_t* error);

#endif
