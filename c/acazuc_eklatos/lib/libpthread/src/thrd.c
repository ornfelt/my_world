#include <threads.h>
#include <sched.h>
#include <errno.h>
#include <time.h>

int
thrd_create(thrd_t *thr, thrd_start_t func, void *arg)
{
	if (pthread_create(thr, NULL, (void*)func, arg))
		return thrd_error;
	return thrd_success;
}

thrd_t
thrd_current(void)
{
	return pthread_self();
}

int
thrd_detach(thrd_t thr)
{
	if (pthread_detach(thr))
		return thrd_error;
	return thrd_success;
}

int
thrd_equal(thrd_t t1, thrd_t t2)
{
	return pthread_compare(t1, t2);
}

void
thrd_exit(int res)
{
	pthread_exit((void*)(intptr_t)res);
}

int
thrd_join(thrd_t thr, int *res)
{
	void *ret;
	if (pthread_join(thr, &ret))
		return thrd_error;
	*res = (int)(intptr_t)ret;
	return thrd_success;
}

int
thrd_sleep(const struct timespec *duration, struct timespec *remaining)
{
	if (nanosleep(duration, remaining) == -1)
	{
		if (errno == EINTR)
			return -1;
		return 2;
	}
	return 0;
}

void
thrd_yield(void)
{
	sched_yield();
}
