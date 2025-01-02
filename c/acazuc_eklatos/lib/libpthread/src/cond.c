#include <sys/futex.h>

#include <pthread.h>
#include <limits.h>
#include <errno.h>

int pthread_condattr_init(pthread_condattr_t *attr)
{
	if (!attr)
		return EINVAL;
	return 0;
}

int pthread_condattr_destroy(pthread_condattr_t *attr)
{
	if (!attr)
		return EINVAL;
	return 0;
}

int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr)
{
	(void)attr;
	if (!cond)
		return EINVAL;
	cond->mutex = NULL;
	cond->waiters = 0;
	cond->value = 0;
	return 0;
}

int pthread_cond_destroy(pthread_cond_t *cond)
{
	if (!cond)
		return EINVAL;
	if (__atomic_load_n(&cond->waiters, __ATOMIC_ACQUIRE))
		return EBUSY;
	return 0;
}

int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
	return pthread_cond_timedwait(cond, mutex, NULL);
}

int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex,
                           const struct timespec *abstime)
{
	if (!cond || !mutex)
		return EINVAL;
	pthread_mutex_t *expected = NULL;
	if (!__atomic_compare_exchange_n(&cond->mutex, &expected, mutex, 0,
	                                 __ATOMIC_ACQUIRE,
	                                 __ATOMIC_RELAXED))
	{
		if (expected != mutex)
			return EINVAL;
	}
	__atomic_add_fetch(&cond->waiters, 1, __ATOMIC_ACQUIRE);
	int value = __atomic_load_n(&cond->value, __ATOMIC_ACQUIRE);
	pthread_mutex_unlock(mutex);
	int ret = 0;
	while (1)
	{
		if (futex((int*)&cond->value,
		          FUTEX_WAIT_PRIVATE | FUTEX_CLOCK_REALTIME,
		          value, abstime) != -1)
			break;
		if (errno == EAGAIN)
			break;
		if (errno == EINTR)
			continue;
		ret = errno;
		break;
	}
	pthread_mutex_lock(mutex);
	if (!__atomic_sub_fetch(&cond->waiters, 1, __ATOMIC_RELEASE))
		__atomic_store_n(&cond->mutex, NULL, __ATOMIC_RELEASE);
	return ret;
}

int pthread_cond_signal(pthread_cond_t *cond)
{
	if (!cond)
		return EINVAL;
	__atomic_add_fetch(&cond->value, 1, __ATOMIC_RELEASE);
	return futex((int*)&cond->value, FUTEX_WAKE_PRIVATE, 1, NULL);
}

int pthread_cond_broadcast(pthread_cond_t *cond)
{
	if (!cond)
		return EINVAL;
	__atomic_add_fetch(&cond->value, 1, __ATOMIC_RELEASE);
	return futex((int*)&cond->value, FUTEX_WAKE_PRIVATE, INT_MAX, NULL);
}
