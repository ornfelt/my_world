#include <threads.h>

int tss_create(tss_t *key, tss_dtor_t dtor)
{
	if (pthread_key_create(&key->key, dtor))
		return thrd_error;
	return thrd_success;
}

void tss_delete(tss_t key)
{
	pthread_key_delete(key.key);
}

void *tss_get(tss_t key)
{
	return pthread_getspecific(key.key);
}

int tss_set(tss_t key, void *val)
{
	if (pthread_setspecific(key.key, val))
		return thrd_error;
	return thrd_success;
}
