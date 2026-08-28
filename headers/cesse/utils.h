#ifndef CESSE_UTILS_H
#define CESSE_UTILS_H

#include <stdint.h>

// Type and values for error codes
// You should ALWAYS create vessel for error code like this:
//   ErrorCode error = CESSE_OK;
// then pass address of that local variable to functions as necessary
// Every ErrorCode* argument can also be ignored by using NULL
// Error codes are set only on error (during normal execution, functions never set it to anything - thus initialization is important)
typedef uint16_t ErrorCode;
typedef enum cesse_error : ErrorCode {
	CESSE_OK = 0,
	CESSE_ERR_ALLOC,
	CESSE_ERR_NULLARG,
	CESSE_ERR_OUT_OF_BOUNDS,
	CESSE_ERR_EMPTY,
	CESSE_ERR_BAD_ARG,
	CESSE_ERR_OVERFLOW,
	CESSE_ERR_UNDERFLOW,
} cesse_error_t;

const char* ErrorCodeo_cstring(const ErrorCode* error_code);

#endif
