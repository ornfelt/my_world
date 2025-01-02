#include <sys/futex.h>

#include <pthread.h>
#include <limits.h>
#include <errno.h>

#define MUTEX_WAITING (1U << 31)

int pthread_mutexattr_init(pthread_mutexattr_t *attr)
{
	if (!attr)
		return EINVAL;
	attr->type = PTHREAD_MUTEX_DEFAULT;
	return 0;
}

int pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
{
	if (!attr)
		return EINVAL;
	return 0;
}

int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type)
{
	if (!attr)
		return EINVAL;
	switch (type)
	{
		case PTHREAD_MUTEX_NORMAL:
		case PTHREAD_MUTEX_RECURSIVE:
		case PTHREAD_MUTEX_ERRORCHECK:
			attr->type = type;
			return 0;
		default:
			return EINVAL;
	}
}

int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type)
{
	if (!attr)
		return EINVAL;
	if (type)
		*type = attr->type;
	return 0;
}

int pthread_mutex_init(pthread_mutex_t *mutex,
                       const pthread_mutexattr_t *attr)
{
	if (!mutex)
		return EINVAL;
	mutex->type = attr ? attr->type : PTHREAD_MUTEX_DEFAULT;
	mutex->owner = NULL;
	mutex->value = 0;
	return 0;
}

int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
	if (!mutex)
		return EINVAL;
	if (__atomic_load_n(&mutex->value, __ATOMIC_ACQUIRE))
		return EBUSY;
	return 0;
}

static int acquire(pthread_mutex_t *mutex, uint32_t *v)
{
	uint32_t expected = 0;
	if (__atomic_compare_exchange_n(&mutex->value, &expected, 1, 0,
	                                __ATOMIC_ACQUIRE,
	                                __ATOMIC_RELAXED))
		return 1;
	if (v)
		*v = expected;
	return 0;
}

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
	return pthread_mutex_timedlock(mutex, NULL);
}

static int recursive_lock(pthread_mutex_t *mutex)
{
	if (mutex->type != PTHREAD_MUTEX_RECURSIVE)
		return EDEADLK;
	if (__atomic_load_n(&mutex->value, __ATOMIC_ACQUIRE) == UINT32_MAX)
		return EAGAIN;
	__atomic_add_fetch(&mutex->value, 1, __ATOMIC_RELAXED);
	return 0;
}

int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
	if (!mutex)
		return EINVAL;
	if (mutex->owner == pthread_self())
		return recursive_lock(mutex);
	if (!acquire(mutex, NULL))
		return EBUSY;
	mutex->owner = pthread_self();
	return 0;
}

int pthread_mutex_timedlock(pthread_mutex_t *mutex,
                            const struct timespec *abstime)
{
	if (!mutex)
		return EINVAL;
	if (mutex->owner == pthread_self())
		return recursive_lock(mutex);
	uint32_t v;
	while (!acquire(mutex, &v))
	{
		if (!(v & MUTEX_WAITING))
		{
			if (!__atomic_compare_exchange_n(&mutex->value, &v,
			                                 v | MUTEX_WAITING,
			                                 1,
			                                 __ATOMIC_ACQUIRE,
			                                 __ATOMIC_RELAXED))
				continue;
			v |= MUTEX_WAITING;
		}
		if (futex((int*)&mutex->value, FUTEX_WAIT_PRIVATE,
		          v, abstime) != -1)
			continue;
		if (errno == EAGAIN || errno == EINTR)
			continue;
		return errno;
	}
	mutex->owner = pthread_self();
	return 0;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	if (!mutex)
		return EINVAL;
	if (mutex->owner != pthread_self())
		return EPERM;
	uint32_t v = __atomic_load_n(&mutex->value, __ATOMIC_ACQUIRE);
	if ((v & ~MUTEX_WAITING) > 1)
	{
		__atomic_sub_fetch(&mutex->value, 1, __ATOMIC_RELAXED);
		return 0;
	}
	mutex->owner = NULL;
	while (!__atomic_compare_exchange_n(&mutex->value, &v, 0, 1,
	                                    __ATOMIC_RELEASE,
	                                    __ATOMIC_ACQUIRE))
		;
	if (!(v & MUTEX_WAITING))
		return 0;
	if (futex((int*)&mutex->value, FUTEX_WAKE_PRIVATE, INT_MAX, NULL) == -1)
		return errno;
	return 0;
}
