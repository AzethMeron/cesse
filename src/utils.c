
#include "cesse/utils.h"
#include "cesse/macros.h"

#include <stddef.h>

const char* error_code_to_cstring(const error_code_t* error) {
	if(error == NULL) {
		return "error_code_to_cstring: NULL error_code pointer passed to decode. Not library bug, but user's incorrect usage.";
	}
	switch(CAST(*error, cesse_error_t)) {
		default: {
			return "error_code_to_cstring: error code not recognized";
		} break;
		case CESSE_OK: {
			return "No error occured, all is OK!";
		} break;
		case CESSE_ERR_ALLOC: {
			return "Memory allocation error.";
		} break;
		case CESSE_ERR_NULLARG: {
			return "NULL argument passed where real address was necessary.";
		} break;
		case CESSE_ERR_OUT_OF_BOUNDS: {
			return "Out-of-bounds error (accessing non-existing element, below 0 or above size)";
		} break;
		case CESSE_ERR_EMPTY: {
			return "Container is empty and thus, this action is invalid (probably pop or sort)";
		} break;
		case CESSE_ERR_BAD_ARG: {
			return "Bad non-pointer argument passed in.";
		} break;
		case CESSE_ERR_OVERFLOW: {
			return "Data (probably size_t) overflow detected & prevented.";
		} break;
		case CESSE_ERR_UNDERFLOW: {
			return "Data (probably size_t) underflow detected & prevented.";
		} break;
	}
}
