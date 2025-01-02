#include "_lock.h"

/* NB: we don't want to use pthread_self as this would require
 * libc to be linked against libpthread
 * we here use the address of a __thread variable which is
 * totally fine since each thread got its own unique address
 * for a given __thread var
 */
static __thread int self;

bool _libc_trylock(struct _libc_lock *lock)
{
	if (lock->owner == &self)
	{
		__atomic_add_fetch(&lock->value, 1, __ATOMIC_RELAXED);
		return true;
	}
	if (!_eklat_trylock(&lock->value))
		return false;
	lock->owner = &self;
	return true;
}

void _libc_lock(struct _libc_lock *lock)
{
	if (lock->owner == &self)
	{
		__atomic_add_fetch(&lock->value, 1, __ATOMIC_RELAXED);
		return;
	}
	_eklat_lock(&lock->value);
	lock->owner = &self;
}

void _libc_unlock(struct _libc_lock *lock)
{
	_eklat_unlock(&lock->value, &lock->owner);
}
