#ifndef REFCOUNT_H
#define REFCOUNT_H

#include <types.h>
#include <std.h>

#define REFCOUNT_INITIALIZER(value) ((refcount_t){.count = value})

typedef struct refcount
{
	uint32_t count;
} refcount_t;

static void refcount_init(refcount_t *refcount, uint32_t count)
{
	__atomic_store_n(&refcount->count, count, __ATOMIC_SEQ_CST);
}

static uint32_t refcount_inc(refcount_t *refcount)
{
	return __atomic_add_fetch(&refcount->count, 1, __ATOMIC_SEQ_CST);
}

static uint32_t refcount_dec(refcount_t *refcount)
{
	uint32_t ret = __atomic_sub_fetch(&refcount->count, 1, __ATOMIC_SEQ_CST);
	if (ret == (uint32_t)-1)
		panic("refcount -1\n");
	return ret;
}

static uint32_t refcount_get(refcount_t *refcount)
{
	return __atomic_load_n(&refcount->count, __ATOMIC_SEQ_CST);
}

#endif
