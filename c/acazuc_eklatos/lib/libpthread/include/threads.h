#ifndef THREADS_H
#define THREADS_H

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	pthread_cond_t cond;
} cnd_t;

typedef struct
{
	pthread_mutex_t mutex;
} mtx_t;

typedef struct
{
	pthread_once_t once;
} once_flag;

typedef struct
{
	pthread_key_t key;
} tss_t;

typedef pthread_t thrd_t;

typedef int (*thrd_start_t)(void*);
typedef void (*tss_dtor_t)(void*);

#define mtx_plain     (1 << 0)
#define mtx_timed     (1 << 1)
#define mtx_recursive (1 << 2)

#define thrd_success  0
#define thrd_error    1
#define thrd_timedout 2

#define ONCE_FLAG_INIT ((once_flag){PTHREAD_ONCE_INIT})

int cnd_init(cnd_t *cond);
void cnd_destroy(cnd_t *cond);
int cnd_broadcast(cnd_t *cond);
int cnd_signal(cnd_t *cond);
int cnd_wait(cnd_t *cond, mtx_t *mtx);
int cnd_timewait(cnd_t *cond, mtx_t *mtx, const struct timespec *ts);

void call_once(once_flag *flag, void (*func)(void));

int mtx_init(mtx_t *mtx, int type);
void mtx_destroy(mtx_t *mtx);
int mtx_lock(mtx_t *mtx);
int mtx_timedlock(mtx_t *mtx, const struct timespec *ts);
int mtx_trylock(mtx_t *mtx);
int mtx_unlock(mtx_t *mtx);

int thrd_create(thrd_t *thr, thrd_start_t func, void *arg);
thrd_t thrd_current(void);
int thrd_detach(thrd_t thr);
int thrd_equal(thrd_t t1, thrd_t t2);
void thrd_exit(int res);
int thrd_join(thrd_t thr, int *res);
int thrd_sleep(const struct timespec *duration, struct timespec *remaining);
void thrd_yield(void);

int tss_create(tss_t *key, tss_dtor_t dtor);
void tss_delete(tss_t key);
void *tss_get(tss_t key);
int tss_set(tss_t key, void *val);

#ifdef __cplusplus
}
#endif

#endif
