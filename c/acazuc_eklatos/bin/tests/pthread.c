#include "tests.h"

#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

struct pthread_test
{
	int test_arg;
	pthread_t leader;
};

static void *thread_main(void *param)
{
	struct pthread_test *st = param;
	ASSERT_EQ(st->test_arg, 5);
	ASSERT_NE(pthread_self(), st->leader);
	return (void*)6;
}

static void *thread_detached(void *param)
{
	while (!__atomic_load_n((int*)param, __ATOMIC_SEQ_CST));
	return NULL;
}

struct pthread_print
{
	pthread_spinlock_t spin;
	pthread_cond_t cond;
	pthread_mutex_t mutex;
	pthread_barrier_t barrier;
	pthread_rwlock_t rwlock;
	pthread_key_t key;
	int count;
	int barrier_count;
};

static pthread_once_t once = PTHREAD_ONCE_INIT;
static int once_value;
static size_t key_val;

static void once_test(void)
{
	ASSERT_EQ(__atomic_fetch_add(&once_value, 1, __ATOMIC_SEQ_CST), 0);
}

static void thread_key_dtr(void *val)
{
	ASSERT_EQ(val, (void*)(uintptr_t)gettid());
	__atomic_fetch_add(&key_val, 1, __ATOMIC_SEQ_CST);
}

__thread int test_tls_init = 65;

static void *thread_print(void *param)
{
	struct pthread_print *st = param;
	int thr = gettid() - getpid();
	printf("%d... ", thr);
	ASSERT_EQ(pthread_once(&once, once_test), 0);
	ASSERT_EQ(pthread_getspecific(st->key), NULL);
	ASSERT_EQ(pthread_setspecific(st->key, (void*)(uintptr_t)gettid()), 0);
	__atomic_add_fetch(&st->count, 1, __ATOMIC_SEQ_CST);
	ASSERT_EQ(pthread_rwlock_rdlock(&st->rwlock), 0);
	__atomic_add_fetch(&st->count, 1, __ATOMIC_SEQ_CST);
	ASSERT_EQ(pthread_spin_lock(&st->spin), 0);
	int expected = 65;
	ASSERT_EQ(__atomic_compare_exchange_n(&test_tls_init, &expected, 1, 0,
	                                      __ATOMIC_SEQ_CST,
	                                      __ATOMIC_SEQ_CST), 1);
	ASSERT_EQ(pthread_spin_unlock(&st->spin), 0);
	ASSERT_EQ(pthread_mutex_lock(&st->mutex), 0);
	__atomic_add_fetch(&st->count, 1, __ATOMIC_SEQ_CST);
	ASSERT_EQ(pthread_cond_wait(&st->cond, &st->mutex), 0);
	ASSERT_EQ(pthread_mutex_unlock(&st->mutex), 0);
	if (pthread_barrier_wait(&st->barrier))
		__atomic_add_fetch(&st->barrier_count, 1, __ATOMIC_SEQ_CST);
	ASSERT_EQ(pthread_rwlock_unlock(&st->rwlock), 0);
	return NULL;
}

void test_pthread(void)
{
	pthread_t thr;

	int detached_mark = 0;
	ASSERT_EQ(pthread_create(&thr, NULL, thread_detached, &detached_mark), 0);
	ASSERT_EQ(pthread_detach(thr), 0);
	ASSERT_EQ(pthread_detach(thr), EINVAL);
	ASSERT_EQ(pthread_join(thr, NULL), EINVAL);
	__atomic_store_n(&detached_mark, 1, __ATOMIC_SEQ_CST);

	struct pthread_test st;
	st.test_arg = 5;
	st.leader = pthread_self();
	ASSERT_EQ(pthread_create(&thr, NULL, thread_main, &st), 0);
	void *ret;
	ASSERT_EQ(pthread_join(thr, &ret), 0);
	ASSERT_EQ(ret, (void*)6);

	usleep(100000); /* wait for the detached to end */

	int expected = 65;
	ASSERT_EQ(__atomic_compare_exchange_n(&test_tls_init, &expected, 1, 0,
	                                      __ATOMIC_SEQ_CST,
	                                      __ATOMIC_SEQ_CST), 1);

	pthread_key_t keys[PTHREAD_KEYS_MAX];
	for (size_t i = 0; i < PTHREAD_KEYS_MAX; ++i)
		ASSERT_EQ(pthread_key_create(&keys[i], NULL), 0);
	pthread_key_t tmp;
	ASSERT_EQ(pthread_key_create(&tmp, NULL), EAGAIN);
	for (size_t i = 0; i < PTHREAD_KEYS_MAX; ++i)
		ASSERT_EQ(pthread_key_delete(keys[i]), 0);

	struct pthread_print print;
	__atomic_store_n(&print.count, 0, __ATOMIC_SEQ_CST);
	__atomic_store_n(&print.barrier_count, 0, __ATOMIC_SEQ_CST);
	pthread_t threads[25];
	ASSERT_EQ(pthread_rwlock_init(&print.rwlock, NULL), 0);
	ASSERT_EQ(pthread_cond_init(&print.cond, NULL), 0);
	ASSERT_EQ(pthread_mutex_init(&print.mutex, NULL), 0);
	ASSERT_EQ(pthread_spin_init(&print.spin, 0), 0);
	ASSERT_EQ(pthread_barrier_init(&print.barrier, NULL, sizeof(threads) / sizeof(*threads)), 0);
	ASSERT_EQ(pthread_key_create(&print.key, thread_key_dtr), 0);
	ASSERT_EQ(pthread_spin_lock(&print.spin), 0);
	ASSERT_EQ(pthread_spin_lock(&print.spin), EDEADLK);
	ASSERT_EQ(pthread_rwlock_wrlock(&print.rwlock), 0);
	ASSERT_EQ(pthread_rwlock_wrlock(&print.rwlock), EDEADLK);
	for (size_t i = 0; i < sizeof(threads) / sizeof(*threads); ++i)
		ASSERT_EQ(pthread_create(&threads[i], NULL, thread_print, &print), 0);
	while (__atomic_load_n(&print.count, __ATOMIC_SEQ_CST) != sizeof(threads) / sizeof(*threads));
	__atomic_store_n(&print.count, 0, __ATOMIC_SEQ_CST);
	ASSERT_EQ(pthread_rwlock_unlock(&print.rwlock), 0);
	while (__atomic_load_n(&print.count, __ATOMIC_SEQ_CST) != sizeof(threads) / sizeof(*threads));
	__atomic_store_n(&print.count, 0, __ATOMIC_SEQ_CST);
	ASSERT_EQ(pthread_spin_unlock(&print.spin), 0);
	while (__atomic_load_n(&print.count, __ATOMIC_SEQ_CST) != sizeof(threads) / sizeof(*threads));
	__atomic_store_n(&print.count, 0, __ATOMIC_SEQ_CST);
	ASSERT_EQ(pthread_rwlock_trywrlock(&print.rwlock), EBUSY);
	ASSERT_EQ(pthread_mutex_lock(&print.mutex), 0);
	ASSERT_EQ(pthread_mutex_lock(&print.mutex), EDEADLK);
	ASSERT_EQ(pthread_cond_broadcast(&print.cond), 0);
	ASSERT_EQ(pthread_mutex_unlock(&print.mutex), 0);
	for (size_t i = 0; i < sizeof(threads) / sizeof(*threads); ++i)
		ASSERT_EQ(pthread_join(threads[i], NULL), 0);
	printf("\n");
	ASSERT_EQ(__atomic_load_n(&once_value, __ATOMIC_SEQ_CST), 1);
	ASSERT_EQ(__atomic_load_n(&print.barrier_count, __ATOMIC_SEQ_CST), 1);
	ASSERT_EQ(key_val, sizeof(threads) / sizeof(*threads));
	ASSERT_EQ(pthread_rwlock_trywrlock(&print.rwlock), 0);
	ASSERT_EQ(pthread_rwlock_unlock(&print.rwlock), 0);
	ASSERT_EQ(pthread_rwlock_destroy(&print.rwlock), 0);
	ASSERT_EQ(pthread_barrier_destroy(&print.barrier), 0);
	ASSERT_EQ(pthread_mutex_destroy(&print.mutex), 0);
	ASSERT_EQ(pthread_spin_destroy(&print.spin), 0);
	ASSERT_EQ(pthread_cond_destroy(&print.cond), 0);
	ASSERT_EQ(pthread_key_delete(print.key), 0);
}
