
#include "cesse/bool.h"
#include "cesse/utils.h"
#include "cesse/array.h"
#include "cesse/functions.h"
#include "cesse/macros.h"
#include "cesse/sort.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

typedef struct Array {
	size_t capacity;
	size_t size;
	void** data;
} Array;

#define MIN_CAPACITY (4) 
#define MAX_CAPACITY (SIZE_MAX/sizeof(void*))

static bool internal_expand_if_necessary(Array* array, size_t target_size, ErrorCode* error) {
    if(target_size > array->capacity) {
		// manage size
		ERROR_ON_COND(target_size > MAX_CAPACITY, error, CESSE_ERR_OVERFLOW, return true;);
		size_t new_capacity = target_size;
		if(array->capacity <= MAX_CAPACITY / 2) { // doubling is okay
			new_capacity = array->capacity * 2; 
			if(new_capacity < target_size) { new_capacity = target_size; }
		}
		else { // doubling would overflow
			new_capacity = MAX_CAPACITY; // logical next step, absolute limit.
		}
		// here goes meat
		void* ptr = malloc(new_capacity * sizeof(void*));
		ERROR_ON_COND(ptr==NULL, error, CESSE_ERR_ALLOC, return true;);
		memset(ptr, 0, new_capacity * sizeof(void*));
		memcpy(ptr, array->data, array->capacity * sizeof(void*));
		free(array->data);
		array->data = CAST(ptr, void**);
		array->capacity = new_capacity;
		return false;
    }
	return false;
}

static bool internal_shrink_if_appropriate(Array* array, size_t target_size, ErrorCode* error) {
	if(array->capacity > MIN_CAPACITY) { // reallocation really isn't worth the effort for small arrays 
		if(target_size < array->capacity / 2) {
			size_t new_capacity = array->capacity / 2;
			void* ptr = malloc(new_capacity * sizeof(void*));
			ERROR_ON_COND(ptr==NULL, error, CESSE_ERR_ALLOC, return true;);
			memset(ptr, 0, new_capacity * sizeof(void*));
			memcpy(ptr, array->data, new_capacity * sizeof(void*));
			free(array->data);
			array->data = CAST(ptr, void**);
			array->capacity = new_capacity;
			return false;
		}
	}
	return false;
}

Array* array_new(size_t capacity, ErrorCode* error) {
	ERROR_ON_COND(capacity==0, error, CESSE_ERR_BAD_ARG, return NULL;); // This guarantees we're making array of cap ATLEAST 1
	ERROR_ON_COND(capacity>MAX_CAPACITY, error, CESSE_ERR_OVERFLOW, return NULL);
	Array* vessel = NULL;
	void* ptr = malloc(sizeof(Array));
	ERROR_ON_COND(ptr==NULL, error, CESSE_ERR_ALLOC, return NULL;);
	memset(ptr, 0, sizeof(Array));
	vessel = CAST(ptr, Array*);
	size_t true_capacity = capacity;
	if(true_capacity < MIN_CAPACITY) { true_capacity = MIN_CAPACITY; }
	ptr = malloc(true_capacity * sizeof(void*));
	ERROR_ON_COND(ptr==NULL, error, CESSE_ERR_ALLOC, {free(vessel); return NULL;});
	memset(ptr, 0, true_capacity * sizeof(void*));
	vessel->data = CAST(ptr, void**);
	vessel->capacity = true_capacity;
	vessel->size = 0;
	return vessel;
}

// Frees array structure
// It does its best to cleanup the contents of array, if there's any present
// but error handling in custom freer is non-existant (only printed to stderr)
// It's recommanded to pop all contents first, and do the cleanup from outside on your own
// Note: objects are void*
//       but freer gets void**, pivoted to the address of THE object.
//       You get pointer to the pointer. No arrays involved.
//       You shall free(*ptr) or equivalent, then *ptr = NULL;
void array_delete(Array** array, ErrorCode* error, function_delete freer) {
	ERROR_ON_COND(array==NULL, error, CESSE_ERR_NULLARG, return;);
	if( (*array) == NULL ) { return; } //no-op
	array_clear(*array, error, freer);
	free((*array)->data);
	free(*array);
	*array = NULL;
}

void array_clear(Array* array, ErrorCode* error, function_delete freer) {
	ERROR_ON_COND(array==NULL, error, CESSE_ERR_NULLARG, return;);
	while(array->size) { 
		ErrorCode local_err = CESSE_OK;
		void* object = array_pop(array, &local_err);
		if(local_err) { // Should be impossible
			SET_ERROR(error, local_err);
			fprintf(stderr, "array_clear: internal error during cleanup. Continuing either way, but memory leaks are likely.\nError code %d -> %s", local_err, ErrorCodeo_cstring(&local_err)); 
			return; 
		} 
		if(freer) {
			local_err = freer(&object);
			if(local_err) {
				fprintf(stderr, "Error occured in array_clear while freeing contents.\nError code %d -> %s. Continuing either way.", local_err, ErrorCodeo_cstring(&local_err));
			}
		}
	}
}

void* array_get(Array* array, const size_t idx, ErrorCode* error) {
	ERROR_ON_COND(array==NULL, error, CESSE_ERR_NULLARG, return NULL);
	ERROR_ON_COND(idx>=(array->size), error, CESSE_ERR_OUT_OF_BOUNDS, return NULL);
	return array->data[idx];
}

void* array_set(Array* array, const size_t idx, void* object, ErrorCode* error) { // overrides pointer at position, return previous pointer
        ERROR_ON_COND(array==NULL, error, CESSE_ERR_NULLARG, return NULL);
        ERROR_ON_COND(idx>=(array->size), error, CESSE_ERR_OUT_OF_BOUNDS, return NULL);
	ERROR_ON_COND(object==NULL, error, CESSE_ERR_NULLARG, return NULL);
	void* ptr = array->data[idx];
	array->data[idx] = object;
	return ptr;
}

void array_push(Array* array, void* object, ErrorCode* error) { // object is borrowed
	ERROR_ON_COND(array==NULL, error, CESSE_ERR_NULLARG, return);
	ERROR_ON_COND(object==NULL, error, CESSE_ERR_NULLARG, return);
	ERROR_ON_COND(array->size>=MAX_CAPACITY, error, CESSE_ERR_OVERFLOW, return);
	if(internal_expand_if_necessary(array, array->size+1, error)) { return; }
	array->data[array->size] = object;
        array->size = array->size + 1;
}

void* array_pop(Array* array, ErrorCode* error) { // doesn't free memory! Also this is the default way of emptying array
	ERROR_ON_COND(array==NULL, error, CESSE_ERR_NULLARG, return NULL);
	ERROR_ON_COND(array->size == 0, error, CESSE_ERR_EMPTY, return NULL);
	void* ptr = array->data[array->size - 1];
	array->data[array->size - 1] = NULL;
	array->size = array->size - 1;
	if(internal_shrink_if_appropriate(array, array->size, error)) { }
	return ptr;
}

size_t array_size(Array* array, ErrorCode* error) {
	ERROR_ON_COND(array==NULL, error, CESSE_ERR_NULLARG, return 0;);
	return array->size;
}

size_t array_capacity(Array* array, ErrorCode* error) {
	ERROR_ON_COND(array==NULL, error, CESSE_ERR_NULLARG, return 0;);
	return array->capacity;
}

void array_swap(Array* array, const size_t first, const size_t second, ErrorCode* error) {
	ERROR_ON_COND(array==NULL, error, CESSE_ERR_NULLARG, return;);
	ERROR_ON_COND(first>=array->size, error, CESSE_ERR_OUT_OF_BOUNDS, return;);
	ERROR_ON_COND(second>=array->size, error, CESSE_ERR_OUT_OF_BOUNDS, return;);
	if(first==second) return; // Acceptable, no error, no change needed
	void* ptr = array->data[first];
	array->data[first] = array->data[second];
	array->data[second] = ptr;
}

void* array_remove(Array* array, const size_t idx, ErrorCode* error) {
	ERROR_ON_COND(array==NULL, error, CESSE_ERR_NULLARG, return NULL;);
	ERROR_ON_COND(idx>=array->size, error, CESSE_ERR_OUT_OF_BOUNDS, return NULL;);
	if(idx == array->size-1) { return array_pop(array, error); }
	void* ptr = array->data[idx];
	for(size_t i = idx; i < array->size-1; ++i) {
		array->data[i] = array->data[i+1];
	}
	array->data[array->size-1] = NULL;
	array->size = array->size - 1;
	if(internal_shrink_if_appropriate(array, array->size, error)) { }
	return ptr;
}

void array_sort(Array* array, function_compare_lt compare_lt, ErrorCode* error) {
    ERROR_ON_COND(array==NULL, error, CESSE_ERR_NULLARG, return;);
	sort(array->data, array->size, compare_lt, error);
}

void array_fit_memory(Array* array, ErrorCode* error) {
	ERROR_ON_COND(array==NULL, error, CESSE_ERR_NULLARG, return;);
	if(array->size == array->capacity) { return; } // Nothing to do
	if(array->capacity <= MIN_CAPACITY) { return; } // Nothing worth to do
	size_t target_size = array->size;
	if(target_size < MIN_CAPACITY) { target_size = MIN_CAPACITY; }
	void* ptr = malloc(target_size*sizeof(void*));
	ERROR_ON_COND(ptr==NULL, error, CESSE_ERR_ALLOC, return;);
	memcpy(ptr, array->data, target_size*sizeof(void*));
	free(array->data);
	array->data = CAST(ptr, void**);
	array->capacity = target_size;
}

size_t array_max_capacity() {
	return MAX_CAPACITY;
}