#include <threads.h>
#include <errno.h>

int
mtx_init(mtx_t *mtx, int type)
{
	pthread_mutexattr_t attr;
	int ret = thrd_error;

	pthread_mutexattr_init(&attr);
	switch (type)
	{
		case mtx_plain:
		case mtx_timed:
			if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL))
				goto end;
			break;
		case mtx_recursive:
			if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE))
				goto end;
			break;
		default:
			goto end;
	}
	if (pthread_mutex_init(&mtx->mutex, &attr))
		goto end;
	ret = thrd_success;

end:
	pthread_mutexattr_destroy(&attr);
	return ret;
}

void
mtx_destroy(mtx_t *mtx)
{
	pthread_mutex_destroy(&mtx->mutex);
}

int
mtx_lock(mtx_t *mtx)
{
	if (pthread_mutex_lock(&mtx->mutex))
		return thrd_error;
	return thrd_success;
}

int
mtx_timedlock(mtx_t *mtx, const struct timespec *ts)
{
	switch (pthread_mutex_timedlock(&mtx->mutex, ts))
	{
		case ETIMEDOUT:
			return thrd_timedout;
		case 0:
			return thrd_success;
		default:
			return thrd_error;
	}
}

int
mtx_trylock(mtx_t *mtx)
{
	switch (pthread_mutex_trylock(&mtx->mutex))
	{
		case EBUSY:
			return thrd_timedout;
		case 0:
			return thrd_success;
		default:
			return thrd_error;
	}
}

int
mtx_unlock(mtx_t *mtx)
{
	if (pthread_mutex_unlock(&mtx->mutex))
		return thrd_error;
	return thrd_success;
}
