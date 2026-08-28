
#include "cesse/functions.h"
#include "cesse/utils.h"
#include <stdlib.h>

ErrorCode default_delete_function(void**ptr) {
	if(ptr==NULL) { return CESSE_ERR_NULLARG; }
	free(*ptr);
	*ptr = NULL;
	return CESSE_OK;
}
