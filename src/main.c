
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <math.h>

#include "cesse/bool.h"
#include "cesse/utils.h"
#include "cesse/array.h"
#include "cesse/functions.h"
#include "cesse/macros.h"
#include "cesse/rng.h"

void* alloc_double(const double value);
bool compare(void* left, void* right);

void* alloc_double(const double value) {
	void* ptr = malloc(sizeof(value));
	*CAST(ptr, double*) = value;
	return ptr;
}

bool compare(void* left, void* right) {
	double* l = CAST(left, double*);
	double* r = CAST(right, double*);
	if(isnan(*l)) return false;
	if(isnan(*r)) return true;
	return *l < *r;
}

#define PRINT_ERROR(error) { if(error) { printf("Error %d: %s", error, error_code_to_cstring(&error)); } }

int main(int argc, char* argv[]) {
	if(argc >= 1) { printf("Hello from %s\n", argv[0]); }
	ErrorCode error = CESSE_OK;
	Array* array = array_new(2, &error);
	PRINT_ERROR(error);
	Rng* rng = rng_new_time(NULL);
	for(int i = 0; i < 100; ++i) {
		array_push(array, alloc_double(dist_uniform_double(rng, -100, 100, NULL)), NULL);
	}
	array_sort(array, compare, NULL);
	size_t size = array_size(array, NULL);
	printf("\nMax capacity: %ld\n", array_capacity(array, NULL));
	for(size_t i = 0; i < size; ++i) {
		double* item = array_get(array, i, NULL);
		printf("%.2f ", *item);
	}
	printf("\nMax capacity: %ld\n", array_capacity(array, NULL));
	array_delete(&array, &error, default_delete_function);
	PRINT_ERROR(error);
	return 0;
}
