#ifndef EKLAT_LOCK_H
#define EKLAT_LOCK_H

#include <sys/futex.h>

#include <stdbool.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define _EKLAT_LOCK_WAITING (1UL << 31)

static inline bool _eklat_trylock(uint32_t *value)
{
	uint32_t expected = 0;
	return __atomic_compare_exchange_n(value,
	                                   &expected, 1, 0,
	                                   __ATOMIC_ACQUIRE,
	                                   __ATOMIC_RELAXED);
}

static inline void _eklat_lock(uint32_t *value)
{
	while (1)
	{
		uint32_t expected = 0;
		if (__atomic_compare_exchange_n(value,
		                                &expected, 1, 0,
		                                __ATOMIC_ACQUIRE,
		                                __ATOMIC_ACQUIRE))
			return;
		if (!(expected & _EKLAT_LOCK_WAITING))
		{
			if (!__atomic_compare_exchange_n(value,
			                                 &expected,
			                                 expected | _EKLAT_LOCK_WAITING,
			                                 1,
			                                 __ATOMIC_ACQUIRE,
			                                 __ATOMIC_RELAXED))
				continue;
			expected |= _EKLAT_LOCK_WAITING;
		}
		if (futex((int*)value, FUTEX_WAIT_PRIVATE, expected, NULL) == -1)
		{
			if (errno == EAGAIN || errno == EINTR)
				continue;
			fprintf(stderr, "_eklat_lock: futex wait failed: %s\n",
			        strerror(errno));
			abort();
		}
	}
}

static inline void _eklat_unlock(uint32_t *value, void **recursive_mark)
{
	uint32_t cur = __atomic_load_n(value, __ATOMIC_ACQUIRE);
	if ((cur & ~_EKLAT_LOCK_WAITING) > 1)
	{
		__atomic_sub_fetch(value, 1, __ATOMIC_RELAXED);
		return;
	}
	if (recursive_mark)
		*recursive_mark = NULL;
	while (!__atomic_compare_exchange_n(value, &cur, 0, 1,
	                                    __ATOMIC_RELEASE,
	                                    __ATOMIC_ACQUIRE))
		;
	if (!(cur & _EKLAT_LOCK_WAITING))
		return;
	if (futex((int*)value, FUTEX_WAKE_PRIVATE, INT_MAX, NULL) == -1)
	{
		fprintf(stderr, "_eklat_unlock: futex wake failed: %s\n",
		        strerror(errno));
		abort();
	}
}

#ifdef __cplusplus
}
#endif

#endif
