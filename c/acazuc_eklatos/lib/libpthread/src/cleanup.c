#include <pthread.h>

void pthread_cleanup_push(void (*routine)(void*), void *arg)
{
	/* XXX */
	(void)routine;
	(void)arg;
}

void pthread_cleanup_pop(int execute)
{
	/* XXX */
	(void)execute;
}
