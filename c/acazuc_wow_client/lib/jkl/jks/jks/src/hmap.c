#include "hmap.h"
#include <sys/queue.h>
#include <stdlib.h>
#include <string.h>

#define JKS_HMAP_MALLOC(size) (hmap->memory_fn ? hmap->memory_fn->malloc(size) : malloc(size))
#define JKS_HMAP_REALLOC(ptr, size) (hmap->memory_fn ? hmap->memory_fn->realloc(ptr, size) : realloc(ptr, size))
#define JKS_HMAP_FREE(ptr) (hmap->memory_fn ? hmap->memory_fn->free(ptr) : free(ptr))

#define REHASH_LOW_THRESHOLD 0.25
#define REHASH_LOW_SIZE      0.5

#define REHASH_HIGH_THRESHOLD 0.5
#define REHASH_HIGH_SIZE      2

struct jks_hmap_object
{
	SLIST_ENTRY(jks_hmap_object) chain;
	uint32_t hash;
	jks_hmap_key_t key;
	char val[];
};

SLIST_HEAD(jks_hmap_bucket, jks_hmap_object);

static void bucket_destroy(jks_hmap_t *hmap, jks_hmap_bucket_t *bucket)
{
	jks_hmap_object_t *object, *next_object;

	for (object = SLIST_FIRST(bucket); object && (next_object = SLIST_NEXT(object, chain), 1); object = next_object)
	{
		if (hmap->destructor)
			hmap->destructor(object->key, object->val);
		JKS_HMAP_FREE(object);
	}
}

void jks_hmap_init(jks_hmap_t *hmap, size_t value_size,
                   jks_hmap_destructor_t destructor,
                   jks_hmap_hash_fn_t hash_fn,
                   jks_hmap_cmp_fn_t cmp_fn,
                   const jks_hmap_memory_fn_t *memory_fn)
{
	hmap->buckets = NULL;
	hmap->buckets_count = 0;
	hmap->size = 0;
	hmap->value_size = value_size;
	hmap->destructor = destructor;
	hmap->hash_fn = hash_fn;
	hmap->cmp_fn = cmp_fn;
	hmap->memory_fn = memory_fn;
}

void jks_hmap_destroy(jks_hmap_t *hmap)
{
	for (size_t i = 0; i < hmap->buckets_count; ++i)
		bucket_destroy(hmap, &hmap->buckets[i]);
	JKS_HMAP_FREE(hmap->buckets);
	hmap->size = 0;
	hmap->buckets = NULL;
}

static inline jks_hmap_bucket_t *get_bucket(jks_hmap_bucket_t *buckets,
                                            size_t buckets_count,
                                            uint32_t hash)
{
	return &buckets[hash & (buckets_count - 1)];
}

static size_t npot(size_t v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
#if __SIZE_WIDTH__ == 64
	v |= v >> 32;
#endif
	v++;
	return v;
}

static bool rehash(jks_hmap_t *hmap, size_t size)
{
	size_t new_buckets_count = npot(size);
	jks_hmap_bucket_t *new_buckets = JKS_HMAP_MALLOC(sizeof(*new_buckets) * new_buckets_count);
	if (!new_buckets)
		return false;
	memset(new_buckets, 0, sizeof(*new_buckets) * new_buckets_count);
	for (size_t i = 0; i < hmap->buckets_count; ++i)
	{
		jks_hmap_bucket_t *bucket = &hmap->buckets[i];
		while (!SLIST_EMPTY(bucket))
		{
			jks_hmap_object_t *object = SLIST_FIRST(bucket);
			jks_hmap_bucket_t *new_bucket = get_bucket(new_buckets, new_buckets_count, object->hash);
			SLIST_REMOVE_HEAD(bucket, chain);
			SLIST_INSERT_HEAD(new_bucket, object, chain);
		}
	}
	for (size_t i = 0; i < hmap->buckets_count; ++i)
		bucket_destroy(hmap, &hmap->buckets[i]);
	JKS_HMAP_FREE(hmap->buckets);
	hmap->buckets = new_buckets;
	hmap->buckets_count = new_buckets_count;
	return true;
}

bool jks_hmap_reserve(jks_hmap_t *hmap, size_t capacity)
{
	if (hmap->buckets_count >= capacity)
		return true;
	return rehash(hmap, capacity);
}

void jks_hmap_clear(jks_hmap_t *hmap)
{
	if (hmap->size)
		return;
	for (size_t i = 0; i < hmap->buckets_count; ++i)
		bucket_destroy(hmap, &hmap->buckets[i]);
	hmap->size = 0;
}

void *jks_hmap_get(const jks_hmap_t *hmap, const jks_hmap_key_t key)
{
	if (!hmap->size)
		return NULL;
	uint32_t hash = hmap->hash_fn(key);
	jks_hmap_bucket_t *bucket = get_bucket(hmap->buckets, hmap->buckets_count, hash);
	jks_hmap_object_t *object;
	SLIST_FOREACH(object, bucket, chain)
	{
		if (object->hash == hash && !hmap->cmp_fn(object->key, key))
			return object->val;
	}
	return NULL;
}

void *jks_hmap_set(jks_hmap_t *hmap, jks_hmap_key_t key, void *value)
{
	jks_hmap_bucket_t *bucket;
	uint32_t hash = hmap->hash_fn(key);
	if (hmap->buckets_count)
	{
		bucket = get_bucket(hmap->buckets, hmap->buckets_count, hash);
		jks_hmap_object_t *object;
		SLIST_FOREACH(object, bucket, chain)
		{
			if (hash != object->hash || hmap->cmp_fn(object->key, key))
				continue;
			if (hmap->destructor)
				hmap->destructor(object->key, object->val);
			object->key = key;
			if (value)
				memcpy(object->val, value, hmap->value_size);
			return object->val;
		}
	}
	else
	{
		bucket = NULL;
	}
	if (bucket == NULL || hmap->size + 1 > hmap->buckets_count * REHASH_HIGH_THRESHOLD)
	{
		size_t rehash_size = hmap->buckets_count * REHASH_HIGH_SIZE;
		if (rehash_size == 0)
			rehash_size = 32;
		if (!rehash(hmap, rehash_size))
			return NULL;
		bucket = get_bucket(hmap->buckets, hmap->buckets_count, hash);
	}
	jks_hmap_object_t *object = JKS_HMAP_MALLOC(sizeof(*object) + hmap->value_size);
	if (!object)
		return NULL;
	object->hash = hash;
	object->key = key;
	if (value)
		memcpy(object->val, value, hmap->value_size);
	SLIST_INSERT_HEAD(bucket, object, chain);
	hmap->size++;
	return object->val;
}

bool jks_hmap_erase(jks_hmap_t *hmap, const jks_hmap_key_t key)
{
	if (!hmap->size)
		return true;
	uint32_t hash = hmap->hash_fn(key);
	jks_hmap_bucket_t *bucket = get_bucket(hmap->buckets, hmap->buckets_count, hash);
	jks_hmap_object_t *object;
	SLIST_FOREACH(object, bucket, chain)
	{
		if (hash != object->hash || hmap->cmp_fn(key, object->key))
			continue;
		if (hmap->destructor)
			hmap->destructor(object->key, object->val);
		SLIST_REMOVE(bucket, object, jks_hmap_object, chain);
		JKS_HMAP_FREE(object);
		hmap->size--;
		break;
	}
	if (hmap->size < hmap->buckets_count * REHASH_LOW_THRESHOLD)
		return rehash(hmap, hmap->buckets_count * REHASH_LOW_SIZE);
	return true;
}

jks_hmap_iterator_t jks_hmap_iterator_begin(const jks_hmap_t *hmap)
{
	jks_hmap_iterator_t iterator;
	for (size_t i = 0; i < hmap->buckets_count; ++i)
	{
		jks_hmap_bucket_t *bucket = &hmap->buckets[i];
		if (SLIST_EMPTY(bucket))
			continue;
		iterator.bucket = bucket;
		iterator.object = SLIST_FIRST(bucket);
		return iterator;
	}
	return jks_hmap_iterator_end(hmap);
}
jks_hmap_iterator_t jks_hmap_iterator_end(const jks_hmap_t *hmap)
{
	jks_hmap_iterator_t iterator;
	iterator.bucket = &hmap->buckets[hmap->buckets_count];
	return iterator;
}

jks_hmap_iterator_t jks_hmap_iterator_find(const jks_hmap_t *hmap,
                                           const jks_hmap_key_t key)
{
	if (!hmap->size)
		return jks_hmap_iterator_end(hmap);
	uint32_t hash = hmap->hash_fn(key);
	jks_hmap_bucket_t *bucket = get_bucket(hmap->buckets, hmap->buckets_count, hash);
	jks_hmap_object_t *object;
	SLIST_FOREACH(object, bucket, chain)
	{
		if (object->hash != hash || hmap->cmp_fn(object->key, key))
			continue;
		jks_hmap_iterator_t iterator;
		iterator.bucket = bucket;
		iterator.object = object;
		return iterator;
	}
	return jks_hmap_iterator_end(hmap);
}

jks_hmap_key_t jks_hmap_iterator_get_key(const jks_hmap_iterator_t *iterator)
{
	return iterator->object->key;
}

void *jks_hmap_iterator_get_value(const jks_hmap_iterator_t *iterator)
{
	return iterator->object->val;
}

void jks_hmap_iterator_erase(jks_hmap_t *hmap,
                             const jks_hmap_iterator_t *iterator)
{
	if (hmap->destructor)
		hmap->destructor(iterator->object->key, iterator->object->val);
	SLIST_REMOVE(iterator->bucket, iterator->object, jks_hmap_object, chain);
	JKS_HMAP_FREE(iterator->object);
	hmap->size--;
}

bool jks_hmap_iterator_is_end(const jks_hmap_t *hmap,
                              const jks_hmap_iterator_t *iterator)
{
	return iterator->bucket == &hmap->buckets[hmap->buckets_count];
}

void jks_hmap_iterator_next(const jks_hmap_t *hmap,
                            jks_hmap_iterator_t *iterator)
{
	jks_hmap_object_t *next = SLIST_NEXT(iterator->object, chain);
	if (next != NULL)
	{
		iterator->object = next;
		return;
	}
	jks_hmap_bucket_t *bucket = iterator->bucket + 1;
	while (bucket != &hmap->buckets[hmap->buckets_count])
	{
		if (!SLIST_EMPTY(bucket))
		{
			iterator->object = SLIST_FIRST(bucket);
			break;
		}
		bucket++;
	}
	iterator->bucket = bucket;
}

uint32_t jks_hmap_hash_i8(jks_hmap_key_t key)
{
	return key.i8;
}

int jks_hmap_cmp_i8(jks_hmap_key_t k1, jks_hmap_key_t k2)
{
	return k1.i8 != k2.i8;
}

uint32_t jks_hmap_hash_u8(jks_hmap_key_t key)
{
	return key.u8;
}

int jks_hmap_cmp_u8(jks_hmap_key_t k1, jks_hmap_key_t k2)
{
	return k1.u8 != k2.u8;
}

uint32_t jks_hmap_hash_i16(jks_hmap_key_t key)
{
	return key.i16;
}

int jks_hmap_cmp_i16(jks_hmap_key_t k1, jks_hmap_key_t k2)
{
	return k1.i16 != k2.i16;
}

uint32_t jks_hmap_hash_u16(jks_hmap_key_t key)
{
	return key.u16;
}

int jks_hmap_cmp_u16(jks_hmap_key_t k1, jks_hmap_key_t k2)
{
	return k1.u16 != k2.u16;
}

uint32_t jks_hmap_hash_i32(jks_hmap_key_t key)
{
	return key.i32;
}

int jks_hmap_cmp_i32(jks_hmap_key_t k1, jks_hmap_key_t k2)
{
	return k1.i32 != k2.i32;
}

uint32_t jks_hmap_hash_u32(jks_hmap_key_t key)
{
	return key.u32;
}

int jks_hmap_cmp_u32(jks_hmap_key_t k1, jks_hmap_key_t k2)
{
	return k1.u32 != k2.u32;
}

uint32_t jks_hmap_hash_i64(jks_hmap_key_t key)
{
	return jks_hmap_hash_u64(key);
}

int jks_hmap_cmp_i64(jks_hmap_key_t k1, jks_hmap_key_t k2)
{
	return jks_hmap_cmp_u64(k1, k2);
}

uint32_t jks_hmap_hash_u64(jks_hmap_key_t key)
{
	return key.u64 * 5381 + (key.u64 >> 32);
}

int jks_hmap_cmp_u64(jks_hmap_key_t k1, jks_hmap_key_t k2)
{
	return k1.u64 != k2.u64;
}

uint32_t jks_hmap_hash_str(jks_hmap_key_t key)
{
	const char *str = key.ptr;
	uint32_t hash = 5381;
	for (int c; (c = *str); str++)
		hash = ((hash << 5) + hash) + c;
	return hash;
}

int jks_hmap_cmp_str(jks_hmap_key_t k1, jks_hmap_key_t k2)
{
	return strcmp(k1.ptr, k2.ptr) != 0;
}

uint32_t jks_hmap_hash_ptr(jks_hmap_key_t key)
{
	if (sizeof(key.ptr) == 4)
		return key.u32;
	uint32_t hash = 5381;
	hash = ((hash << 5) + hash) + (key.u64 & 0xFFFFFFFF);
	hash = ((hash << 5) + hash) + (key.u64 >> 32);
	return hash;
}

int jks_hmap_cmp_ptr(jks_hmap_key_t k1, jks_hmap_key_t k2)
{
	return (uint8_t*)k1.ptr != (uint8_t*)k2.ptr;
}
