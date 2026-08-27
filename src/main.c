
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <math.h>

#include "cesse/bool.h"
#include "cesse/utils.h"
#include "cesse/array.h"
#include "cesse/functions.h"
#include "cesse/macros.h"

void* alloc_double(const double value);
error_code_t freer(void** addr);
bool compare(void* left, void* right);

void* alloc_double(const double value) {
	void* ptr = malloc(sizeof(value));
	*CAST(ptr, double*) = value;
	return ptr;
}

error_code_t freer(void** addr) {
	free(*addr);
	*addr = NULL;
	return CESSE_OK;
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
	error_code_t error = CESSE_OK;
	Array* array = array_new(2, &error);
	PRINT_ERROR(error);
	array_push(array, alloc_double(-53.442), NULL);
	array_push(array, alloc_double(49.75), NULL);
	array_push(array, alloc_double(-94.041), NULL);
	array_push(array, alloc_double(-54.42), NULL);
	array_push(array, alloc_double(-50.674), NULL);
	array_push(array, alloc_double(36.923), NULL);
	array_push(array, alloc_double(78.436), NULL);
	array_push(array, alloc_double(0.0), NULL);
	array_push(array, alloc_double(40.914), NULL);
	array_push(array, alloc_double(91.443), NULL);
	array_push(array, alloc_double(-54.19), NULL);
	array_push(array, alloc_double(20.745), NULL);
	array_push(array, alloc_double(69.499), NULL);
	array_push(array, alloc_double(28.0), NULL);
	array_push(array, alloc_double(8.46), NULL);
	array_push(array, alloc_double(-81.451), NULL);
	array_push(array, alloc_double(-37.064), NULL);
	array_push(array, alloc_double(17.853), NULL);
	array_push(array, alloc_double(61.886), NULL);
	array_push(array, alloc_double(-46.452), NULL);
	array_push(array, alloc_double(15.47), NULL);
	array_push(array, alloc_double(-44.994), NULL);
	array_push(array, alloc_double(-8.23), NULL);
	array_push(array, alloc_double(97.905), NULL);
	array_push(array, alloc_double(-46.604), NULL);
	array_push(array, alloc_double(-25.964), NULL);
	array_push(array, alloc_double(-23.73), NULL);
	array_push(array, alloc_double(0.0), NULL);
	array_push(array, alloc_double(NAN), NULL);
	array_push(array, alloc_double(1.071), NULL);
	array_push(array, alloc_double(55.2), NULL);
	array_push(array, alloc_double(39.628), NULL);
	array_push(array, alloc_double(14.3), NULL);
	array_push(array, alloc_double(-39.04), NULL);
	array_push(array, alloc_double(-94.693), NULL);
	array_push(array, alloc_double(-84.042), NULL);
	array_push(array, alloc_double(88.582), NULL);
	array_push(array, alloc_double(-80.657), NULL);
	array_push(array, alloc_double(-20.874), NULL);
	array_push(array, alloc_double(-55.912), NULL);
	array_push(array, alloc_double(-56.272), NULL);
	array_push(array, alloc_double(10.408), NULL);
	array_push(array, alloc_double(61.426), NULL);
	array_push(array, alloc_double(14.3), NULL);
	array_push(array, alloc_double(-40.91), NULL);
	array_push(array, alloc_double(14.3), NULL);
	array_push(array, alloc_double(47.294), NULL);
	array_push(array, alloc_double(11.39), NULL);
	array_push(array, alloc_double(-79.8), NULL);
	array_push(array, alloc_double(0.95), NULL);
	array_push(array, alloc_double(35.34), NULL);
	array_push(array, alloc_double(65.881), NULL);
	array_push(array, alloc_double(45.825), NULL);
	array_push(array, alloc_double(-55.358), NULL);
	array_push(array, alloc_double(8.988), NULL);
	array_push(array, alloc_double(39.78), NULL);
	array_push(array, alloc_double(-58.099), NULL);
	array_push(array, alloc_double(87.331), NULL);
	array_push(array, alloc_double(-65.772), NULL);
	array_push(array, alloc_double(-68.904), NULL);
	array_push(array, alloc_double(-10.06), NULL);
	array_push(array, alloc_double(-82.612), NULL);
	array_push(array, alloc_double(6.14), NULL);
	array_push(array, alloc_double(45.946), NULL);
	array_push(array, alloc_double(-90.835), NULL);
	array_push(array, alloc_double(-28.07), NULL);
	array_push(array, alloc_double(-47.024), NULL);
	array_push(array, alloc_double(-44.405), NULL);
	array_push(array, alloc_double(-98.7), NULL);
	array_push(array, alloc_double(-32.681), NULL);
	array_push(array, alloc_double(75.274), NULL);
	array_push(array, alloc_double(72.341), NULL);
	array_push(array, alloc_double(-60.232), NULL);
	array_push(array, alloc_double(31.088), NULL);
	array_push(array, alloc_double(-31.95), NULL);
	array_push(array, alloc_double(68.57), NULL);
	array_push(array, alloc_double(94.623), NULL);
	array_push(array, alloc_double(27.885), NULL);
	array_push(array, alloc_double(-67.32), NULL);
	array_push(array, alloc_double(61.164), NULL);
	array_push(array, alloc_double(21.826), NULL);
	array_push(array, alloc_double(23.704), NULL);
	array_push(array, alloc_double(100.0), NULL);
	array_push(array, alloc_double(-42.122), NULL);
	array_push(array, alloc_double(-0.0), NULL);
	array_push(array, alloc_double(-24.293), NULL);
	array_push(array, alloc_double(-36.909), NULL);
	array_push(array, alloc_double(29.977), NULL);
	array_push(array, alloc_double(12.74), NULL);
	array_push(array, alloc_double(-94.998), NULL);
	array_push(array, alloc_double(-57.803), NULL);
	array_push(array, alloc_double(7.246), NULL);
	array_push(array, alloc_double(-93.58), NULL);
	array_push(array, alloc_double(-24.109), NULL);
	array_push(array, alloc_double(-45.29), NULL);
	array_push(array, alloc_double(27.137), NULL);
	array_push(array, alloc_double(-27.034), NULL);
	array_push(array, alloc_double(-15.616), NULL);
	array_push(array, alloc_double(82.91), NULL);
	array_push(array, alloc_double(29.607), NULL);
	array_push(array, alloc_double(-100.0), NULL);
	array_sort(array, compare, NULL);
	size_t size = array_size(array, NULL);
	printf("\nMax capacity: %ld\n", array_capacity(array, NULL));
	for(size_t i = 0; i < size; ++i) {
		double* item = array_get(array, i, NULL);
		printf("%.2f ", *item);
	}
	printf("\nMax capacity: %ld\n", array_capacity(array, NULL));
	array_delete(&array, &error, freer, false);
	PRINT_ERROR(error);
	return 0;
}
