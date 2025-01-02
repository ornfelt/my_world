#include <sys/futex.h>

#include <pthread.h>
#include <limits.h>
#include <errno.h>

int pthread_barrierattr_init(pthread_barrierattr_t *attr)
{
	if (!attr)
		return EINVAL;
	return 0;
}

int pthread_barrierattr_destroy(pthread_barrierattr_t *attr)
{
	if (!attr)
		return EINVAL;
	return 0;
}

int pthread_barrier_init(pthread_barrier_t *barrier,
                                const pthread_barrierattr_t *attr,
                                unsigned count)
{
	(void)attr;
	if (!barrier)
		return EINVAL;
	barrier->count = count;
	barrier->value = 0;
	barrier->revision = 0;
	return 0;
}

int pthread_barrier_destroy(pthread_barrier_t *barrier)
{
	if (!barrier)
		return EINVAL;
	return 0;
}

int pthread_barrier_wait(pthread_barrier_t *barrier)
{
	unsigned revision = __atomic_load_n(&barrier->revision, __ATOMIC_ACQUIRE);
	if (__atomic_add_fetch(&barrier->value, 1, __ATOMIC_ACQUIRE) == barrier->count)
	{
		__atomic_store_n(&barrier->value, 0, __ATOMIC_RELEASE);
		__atomic_add_fetch(&barrier->revision, 1, __ATOMIC_RELEASE);
		futex((int*)&barrier->revision, FUTEX_WAKE_PRIVATE,
		      INT_MAX, NULL);
		return PTHREAD_BARRIER_SERIAL_THREAD;
	}
	do
	{
		if (futex((int*)&barrier->revision,
		          FUTEX_WAIT_PRIVATE,
		          revision, NULL) == -1)
		{
			if (errno == EAGAIN)
				break;
			if (errno == EINTR)
				continue;
			return errno;
		}
	} while (__atomic_load_n(&barrier->revision, __ATOMIC_RELAXED) == revision);
	return 0;
}
