#ifndef JKS_HMAP_H
#define JKS_HMAP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef union
{
	uint64_t u64;
	int64_t i64;
	uint32_t u32;
	int32_t i32;
	uint16_t u16;
	int16_t i16;
	uint8_t u8;
	int8_t i8;
	void *ptr;
} jks_hmap_key_t;

#define JKS_HMAP_KEY_PTR(v) (jks_hmap_key_t){.ptr = v}
#define JKS_HMAP_KEY_U64(v) (jks_hmap_key_t){.u64 = v}
#define JKS_HMAP_KEY_I64(v) (jks_hmap_key_t){.i64 = v}
#define JKS_HMAP_KEY_U32(v) (jks_hmap_key_t){.u32 = v}
#define JKS_HMAP_KEY_I32(v) (jks_hmap_key_t){.i32 = v}
#define JKS_HMAP_KEY_U16(v) (jks_hmap_key_t){.u16 = v}
#define JKS_HMAP_KEY_I16(v) (jks_hmap_key_t){.i16 = v}
#define JKS_HMAP_KEY_U8(v) (jks_hmap_key_t){.u8 = v}
#define JKS_HMAP_KEY_I8(v) (jks_hmap_key_t){.i8 = v}
#define JKS_HMAP_KEY_STR(v) (jks_hmap_key_t){.ptr = v}

typedef void (*jks_hmap_destructor_t)(jks_hmap_key_t key, void *value);
typedef uint32_t (*jks_hmap_hash_fn_t)(jks_hmap_key_t key);
typedef int (*jks_hmap_cmp_fn_t)(const jks_hmap_key_t k1, const jks_hmap_key_t k2);

typedef struct jks_hmap_bucket jks_hmap_bucket_t;
typedef struct jks_hmap_object jks_hmap_object_t;

typedef struct jks_hmap_memory_fn
{
	void *(*malloc)(size_t size);
	void *(*realloc)(void *ptr, size_t size);
	void (*free)(void *data);
} jks_hmap_memory_fn_t;

typedef struct jks_hmap_iterator
{
	jks_hmap_bucket_t *bucket;
	jks_hmap_object_t *object;
} jks_hmap_iterator_t;

typedef struct jks_hmap
{
	jks_hmap_bucket_t *buckets;
	size_t buckets_count;
	size_t size;
	size_t value_size;
	jks_hmap_destructor_t destructor;
	jks_hmap_hash_fn_t hash_fn;
	jks_hmap_cmp_fn_t cmp_fn;
	const jks_hmap_memory_fn_t *memory_fn;
} jks_hmap_t;

void jks_hmap_init(jks_hmap_t *hmap, size_t value_size,
                   jks_hmap_destructor_t destructor,
                   jks_hmap_hash_fn_t hash_fn,
                   jks_hmap_cmp_fn_t cmp_fn,
                   const jks_hmap_memory_fn_t *memory_fn);
void jks_hmap_destroy(jks_hmap_t *hmap);

bool jks_hmap_reserve(jks_hmap_t *hmap, size_t capacity);
void jks_hmap_clear(jks_hmap_t *hmap);

void *jks_hmap_get(const jks_hmap_t *hmap, jks_hmap_key_t key);
void *jks_hmap_set(jks_hmap_t *hmap, jks_hmap_key_t key, void *value);
bool jks_hmap_erase(jks_hmap_t *hmap, jks_hmap_key_t key);

jks_hmap_iterator_t jks_hmap_iterator_begin(const jks_hmap_t *hmap);
jks_hmap_iterator_t jks_hmap_iterator_end(const jks_hmap_t *hmap);
jks_hmap_iterator_t jks_hmap_iterator_find(const jks_hmap_t *hmap,
                                           jks_hmap_key_t key);

jks_hmap_key_t jks_hmap_iterator_get_key(const jks_hmap_iterator_t *iterator);
void *jks_hmap_iterator_get_value(const jks_hmap_iterator_t *iterator);
void jks_hmap_iterator_erase(jks_hmap_t *hmap,
                             const jks_hmap_iterator_t *iterator);
bool jks_hmap_iterator_is_end(const jks_hmap_t *hmap,
                              const jks_hmap_iterator_t *iterator);
void jks_hmap_iterator_next(const jks_hmap_t *hmap,
                            jks_hmap_iterator_t *iterator);

uint32_t jks_hmap_hash_i8(jks_hmap_key_t key);
int jks_hmap_cmp_i8(jks_hmap_key_t k1, jks_hmap_key_t k2);
uint32_t jks_hmap_hash_u8(jks_hmap_key_t key);
int jks_hmap_cmp_u8(jks_hmap_key_t k1, jks_hmap_key_t k2);
uint32_t jks_hmap_hash_i16(jks_hmap_key_t key);
int jks_hmap_cmp_i16(jks_hmap_key_t k1, jks_hmap_key_t k2);
uint32_t jks_hmap_hash_u16(jks_hmap_key_t key);
int jks_hmap_cmp_u16(jks_hmap_key_t k1, jks_hmap_key_t k2);
uint32_t jks_hmap_hash_i32(jks_hmap_key_t key);
int jks_hmap_cmp_i32(jks_hmap_key_t k1, jks_hmap_key_t k2);
uint32_t jks_hmap_hash_u32(jks_hmap_key_t key);
int jks_hmap_cmp_u32(jks_hmap_key_t k1, jks_hmap_key_t k2);
uint32_t jks_hmap_hash_i64(jks_hmap_key_t key);
int jks_hmap_cmp_i64(jks_hmap_key_t k1, jks_hmap_key_t k2);
uint32_t jks_hmap_hash_u64(jks_hmap_key_t key);
int jks_hmap_cmp_u64(jks_hmap_key_t k1, jks_hmap_key_t k2);
uint32_t jks_hmap_hash_str(jks_hmap_key_t key);
int jks_hmap_cmp_str(jks_hmap_key_t k1, jks_hmap_key_t k2);
uint32_t jks_hmap_hash_ptr(jks_hmap_key_t key);
int jks_hmap_cmp_ptr(jks_hmap_key_t k1, jks_hmap_key_t k2);

#define JKS_HMAP_FOREACH(iterator, hmap) \
	for (jks_hmap_iterator_t iterator = jks_hmap_iterator_begin(hmap); \
	     !jks_hmap_iterator_is_end(hmap, &iterator); \
	     jks_hmap_iterator_next(hmap, &iterator))

#ifdef __cplusplus
}
#endif

#endif
