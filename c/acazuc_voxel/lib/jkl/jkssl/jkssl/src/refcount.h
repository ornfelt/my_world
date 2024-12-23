#ifndef JKSSL_REFCOUNT_H
#define JKSSL_REFCOUNT_H

#include <stdint.h>

#define REFCOUNT_INITIALIZER(value) ((refcount_t){.count = value})

typedef struct refcount
{
	uint32_t count;
} refcount_t;

static inline void refcount_init(refcount_t *refcount, uint32_t count)
{
	__atomic_store_n(&refcount->count, count, __ATOMIC_SEQ_CST);
}

static inline uint32_t refcount_inc(refcount_t *refcount)
{
	return __atomic_add_fetch(&refcount->count, 1, __ATOMIC_SEQ_CST);
}

static inline uint32_t refcount_dec(refcount_t *refcount)
{
	return __atomic_sub_fetch(&refcount->count, 1, __ATOMIC_SEQ_CST);
}

static inline uint32_t refcount_get(refcount_t *refcount)
{
	return __atomic_load_n(&refcount->count, __ATOMIC_SEQ_CST);
}


#endif
