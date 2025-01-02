#include <sys/futex.h>

#include <pthread.h>
#include <threads.h>
#include <limits.h>
#include <errno.h>

int pthread_once(pthread_once_t *once, void (*init)(void))
{
	if (!once)
		return EINVAL;
	if (!init)
		return EINVAL;
	unsigned expected = 0;
	if (__atomic_compare_exchange_n(&once->value, &expected, 1, 0,
	                                __ATOMIC_ACQUIRE,
	                                __ATOMIC_ACQUIRE))
	{
		init();
		__atomic_store_n(&once->value, 2, __ATOMIC_RELEASE);
		if (futex((int*)&once->value, FUTEX_WAKE_PRIVATE,
		          INT_MAX, NULL) == -1)
			return errno;
		return 0;
	}
	if (expected == 2)
		return 0;
	do
	{
		if (futex((int*)&once->value, FUTEX_WAIT_PRIVATE,
		          1, NULL) != -1)
			continue;
		if (errno == EAGAIN)
			break;
		if (errno == EINTR)
			continue;
		return errno;
	} while (__atomic_load_n(&once->value, __ATOMIC_RELAXED) != 2);
	return 0;
}

void call_once(once_flag *flag, void (*func)(void))
{
	pthread_once(&flag->once, func);
}
