#ifndef CESSE_MACROS_H
#define CESSE_MACROS_H

#define CAST(obj,type) ((type) (obj))
#define SET_ERROR(ptr, val) do { if(ptr!=NULL) {*ptr = val;} } while(0)
#define ERROR_ON_COND(cond, error_ptr, code, cleanup) do { if((cond)) { SET_ERROR(error_ptr, code); cleanup; } } while(0)

#endif
