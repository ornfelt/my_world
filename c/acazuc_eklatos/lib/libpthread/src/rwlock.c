#include <sys/futex.h>

#include <pthread.h>
#include <limits.h>
#include <errno.h>

#define RWLOCK_WAITING 0x80000000
#define RWLOCK_MASK    0x7FFFFFFF
#define RWLOCK_WRITING 0x7FFFFFFF
#define RWLOCK_MAX     0x7FFFFFFE

int pthread_rwlockattr_init(pthread_rwlockattr_t *attr)
{
	if (!attr)
		return EINVAL;
	return 0;
}

int pthread_rwlockattr_destroy(pthread_rwlockattr_t *attr)
{
	if (!attr)
		return EINVAL;
	return 0;
}

int pthread_rwlock_init(pthread_rwlock_t *rwlock,
                        const pthread_rwlockattr_t *attr)
{
	(void)attr;
	if (!rwlock)
		return EINVAL;
	rwlock->owner = NULL;
	rwlock->value = 0;
	return 0;
}

int pthread_rwlock_destroy(pthread_rwlock_t *rwlock)
{
	if (!rwlock)
		return EINVAL;
	if (__atomic_load_n(&rwlock->value, __ATOMIC_ACQUIRE))
		return EBUSY;
	return 0;
}

int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock)
{
	return pthread_rwlock_timedrdlock(rwlock, NULL);
}

static int rdacquire(pthread_rwlock_t *rwlock, uint32_t *val)
{
	uint32_t cur = __atomic_load_n(&rwlock->value, __ATOMIC_ACQUIRE);
	while (1)
	{
		if ((cur & RWLOCK_MASK) == RWLOCK_WRITING)
		{
			if (val)
				*val = cur;
			return EBUSY;
		}
		if ((cur & RWLOCK_MASK) == RWLOCK_MAX)
			return EAGAIN;
		if (__atomic_compare_exchange_n(&rwlock->value, &cur, cur + 1, 0,
		                                __ATOMIC_ACQUIRE,
		                                __ATOMIC_RELAXED))
			return 0;
	}
}

int pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock)
{
	if (!rwlock)
		return EINVAL;
	return rdacquire(rwlock, NULL);
}

int pthread_rwlock_timedrdlock(pthread_rwlock_t *rwlock,
                               const struct timespec *abstime)
{
	if (!rwlock)
		return EINVAL;
	while (1)
	{
		if (rwlock->owner == pthread_self())
			return EDEADLK;
		uint32_t cur;
		int ret = rdacquire(rwlock, &cur);
		if (ret != EBUSY)
			return ret;
		if (!__atomic_compare_exchange_n(&rwlock->value, &cur,
		                                 cur | RWLOCK_WAITING, 0,
		                                 __ATOMIC_ACQUIRE,
		                                 __ATOMIC_RELAXED))
			continue;
		cur |= RWLOCK_WAITING;
		if (futex((int*)&rwlock->value,
		          FUTEX_WAIT_PRIVATE | FUTEX_CLOCK_REALTIME,
		          cur, abstime) == -1)
		{
			if (errno == EAGAIN || errno == EINTR)
				continue;
			return errno;
		}
	}
}

int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock)
{
	return pthread_rwlock_timedwrlock(rwlock, NULL);
}

static int wracquire(pthread_rwlock_t *rwlock, uint32_t *cur)
{
	uint32_t expected = 0;
	if (__atomic_compare_exchange_n(&rwlock->value, &expected,
	                                RWLOCK_WRITING, 0,
	                                __ATOMIC_ACQUIRE,
	                                __ATOMIC_RELAXED))
	{
		rwlock->owner = pthread_self();
		return 0;
	}
	if (cur)
		*cur = expected;
	return EBUSY;
}

int pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock)
{
	if (!rwlock)
		return EINVAL;
	return wracquire(rwlock, NULL);
}

int pthread_rwlock_timedwrlock(pthread_rwlock_t *rwlock,
                               const struct timespec *abstime)
{
	if (!rwlock)
		return EINVAL;
	if (rwlock->owner == pthread_self())
		return EDEADLK;
	while (1)
	{
		uint32_t cur;
		if (!wracquire(rwlock, &cur))
			return 0;
		if (!__atomic_compare_exchange_n(&rwlock->value, &cur,
		                                 cur | RWLOCK_WAITING, 0,
		                                 __ATOMIC_ACQUIRE,
		                                 __ATOMIC_RELAXED))
			continue;
		cur |= RWLOCK_WAITING;
		if (futex((int*)&rwlock->value,
		          FUTEX_WAIT_PRIVATE | FUTEX_CLOCK_REALTIME,
		          cur, abstime) == -1)
		{
			if (errno == EAGAIN || errno == EINTR)
				continue;
			return errno;
		}
	}
	return 0;
}

int pthread_rwlock_unlock(pthread_rwlock_t *rwlock)
{
	if (!rwlock)
		return EINVAL;
	uint32_t cur = __atomic_load_n(&rwlock->value, __ATOMIC_ACQUIRE);
	if ((cur & RWLOCK_MASK) == RWLOCK_WRITING)
		rwlock->owner = NULL;
	while (1)
	{
		uint32_t new;
		if ((cur & RWLOCK_MASK) == RWLOCK_WRITING)
			new = 0;
		else
			new = cur - 1;
		if (__atomic_compare_exchange_n(&rwlock->value, &cur, new, 0,
		                                __ATOMIC_RELEASE,
		                                __ATOMIC_RELAXED))
			break;
	}
	if (cur & RWLOCK_WAITING)
		futex((int*)&rwlock->value, FUTEX_WAKE_PRIVATE, INT_MAX, NULL);
	return 0;
}
