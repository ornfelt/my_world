#include "array.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

void test_array()
{
	jks_array_t array;
	uint32_t tmp;
	jks_array_init(&array, sizeof(uint32_t), NULL);
	assert(jks_array_resize(&array, 1));
	assert(array.data);
	assert(array.size == 1);
	tmp = 5;
	void *data;
	assert((data = jks_array_get(&array, 0)));
	memcpy(data, &tmp, sizeof(tmp));
	tmp = 6;
	assert(jks_array_push_back(&array, &tmp));
	assert((data = jks_array_get(&array, 1)));
	assert(!memcmp(data, &tmp, sizeof(tmp)));
	tmp = 5;
	assert((data = jks_array_get(&array, 0)));
	assert(!memcmp(data, &tmp, sizeof(tmp)));
	assert(jks_array_reserve(&array, 256));
	assert(array.capacity == 256);
	assert(jks_array_shrink(&array));
	assert(array.capacity == 2);
	jks_array_destroy(&array);
	printf("[OK] test_array\n");
}
