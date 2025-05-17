#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define BITMAP_BPW (sizeof(size_t) * 8)

static void (*g_destructors[PTHREAD_KEYS_MAX])(void*);
static size_t g_bitmap[PTHREAD_KEYS_MAX / BITMAP_BPW];
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

int
pthread_key_create(pthread_key_t *key, void (*destructor)(void*))
{
	if (!key)
		return EINVAL;
	int ret = pthread_mutex_lock(&g_mutex);
	if (ret)
		return ret;
	for (size_t i = 0; i < sizeof(g_bitmap) / sizeof(*g_bitmap); ++i)
	{
		size_t word = g_bitmap[i];
		if (word == SIZE_MAX)
			continue;
		for (size_t j = 0; j < sizeof(word) * 8; ++j)
		{
			size_t mask = (size_t)1 << j;
			if (word & mask)
				continue;
			g_bitmap[i] |= mask;
			size_t n = i * BITMAP_BPW + j;
			g_destructors[n] = destructor;
			pthread_mutex_unlock(&g_mutex);
			*key = n;
			return 0;
		}
		fprintf(stderr, "*** corrupted pthread key bitmap ***\n");
		abort();
	}
	pthread_mutex_unlock(&g_mutex);
	return EAGAIN;
}

int
pthread_key_delete(pthread_key_t key)
{
	if (key >= PTHREAD_KEYS_MAX)
		return EINVAL;
	int ret = pthread_mutex_lock(&g_mutex);
	if (ret)
		return ret;
	size_t i = key / BITMAP_BPW;
	size_t j = key % BITMAP_BPW;
	size_t mask = (size_t)1 << j;
	if (!(g_bitmap[i] & mask))
	{
		pthread_mutex_unlock(&g_mutex);
		return EINVAL;
	}
	g_bitmap[i] &= ~mask;
	g_destructors[key] = NULL;
	pthread_mutex_unlock(&g_mutex);
	return 0;
}

void *
pthread_getspecific(pthread_key_t key)
{
	return pthread_self()->keys[key];
}

int
pthread_setspecific(pthread_key_t key, const void *val)
{
	pthread_self()->keys[key] = (void*)val;
	return 0;
}

int
_pthread_key_cleanup(pthread_t thread)
{
	int ret = pthread_mutex_lock(&g_mutex);
	if (ret)
		return ret;
	for (size_t i = 0; i < sizeof(g_bitmap) / sizeof(*g_bitmap); ++i)
	{
		size_t word = g_bitmap[i];
		if (!word)
			continue;
		for (size_t j = 0; j < sizeof(word) * 8; ++j)
		{
			size_t mask = (size_t)1 << j;
			if (!(word & mask))
				continue;
			size_t n = i * BITMAP_BPW + j;
			if (!g_destructors[n])
				continue;
			void (*dtr)(void*) = g_destructors[n];
			pthread_mutex_unlock(&g_mutex);
			dtr(thread->keys[n]);
			pthread_mutex_unlock(&g_mutex);
		}
	}
	pthread_mutex_unlock(&g_mutex);
	return 0;
}
