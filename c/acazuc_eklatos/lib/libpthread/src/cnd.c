#include <threads.h>
#include <errno.h>

int
cnd_init(cnd_t *cond)
{
	if (pthread_cond_init(&cond->cond, NULL))
		return thrd_error;
	return thrd_success;
}

void
cnd_destroy(cnd_t *cond)
{
	pthread_cond_destroy(&cond->cond);
}

int
cnd_broadcast(cnd_t *cond)
{
	if (pthread_cond_broadcast(&cond->cond))
		return thrd_error;
	return thrd_success;
}

int
cnd_signal(cnd_t *cond)
{
	if (pthread_cond_signal(&cond->cond))
		return thrd_error;
	return thrd_success;
}

int
cnd_wait(cnd_t *cond, mtx_t *mtx)
{
	if (pthread_cond_wait(&cond->cond, &mtx->mutex))
		return thrd_error;
	return thrd_success;
}

int
cnd_timewait(cnd_t *cond, mtx_t *mtx, const struct timespec *ts)
{
	switch (pthread_cond_timedwait(&cond->cond, &mtx->mutex, ts))
	{
		case ETIMEDOUT:
			return thrd_timedout;
		case 0:
			return thrd_success;
		default:
			return thrd_error;
	}
}
