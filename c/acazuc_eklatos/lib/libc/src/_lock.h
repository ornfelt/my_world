#ifndef _LIBC_LOCK_H
#define _LIBC_LOCK_H

#include <eklat/lock.h>

struct _libc_lock
{
	uint32_t value;
	void *owner;
};

static inline void _libc_lock_init(struct _libc_lock *lock)
{
	lock->value = 0;
	lock->owner = NULL;
}

bool _libc_trylock(struct _libc_lock *lock);
void _libc_lock(struct _libc_lock *lock);
void _libc_unlock(struct _libc_lock *lock);

#endif
