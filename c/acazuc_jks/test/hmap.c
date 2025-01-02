#include "hmap.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

void test_hmap()
{
	jks_hmap_t hmap;
	jks_hmap_init(&hmap, NULL, jks_hmap_hash_uint32, jks_hmap_cmp_uint32);
	for (int i = 0; i < 500; ++i)
	{
		uint32_t val = 1000 + i;
		assert(jks_hmap_set(&hmap, (void*)(intptr_t)i, (void*)(intptr_t)val));
	}
	assert(hmap.size == 500);
	uint32_t old_size = hmap.buckets_count;
	assert(jks_hmap_reserve(&hmap, 2000));
	assert(hmap.buckets_count != old_size);
	for (int i = 0; i < 500; ++i)
	{
		void *data;
		assert(jks_hmap_get(&hmap, (void*)(intptr_t)i, &data));
		assert((uint32_t)(intptr_t)data == 1000u + i);
	}
	uint32_t i = 0;
	JKS_HMAP_FOREACH(iter, &hmap)
		++i;
	assert(i == 500);
	jks_hmap_destroy(&hmap);
	printf("[OK] test_hmap\n");
}
