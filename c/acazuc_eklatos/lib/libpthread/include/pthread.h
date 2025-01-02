#ifndef PTHREAD_H
#define PTHREAD_H

#include <sys/types.h>

#include <signal.h>
#include <sched.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PTHREAD_BARRIER_INITIALIZER {0}
#define PTHREAD_COND_INITIALIZER    {NULL, 0, 0}
#define PTHREAD_MUTEX_INITIALIZER   {NULL, 0, 0}
#define PTHREAD_ONCE_INIT           {0}
#define PTHREAD_RWLOCK_INITIALIZER  {NULL, 0}
#define PTHREAD_SPIN_INITIALIZER    {NULL, 0}

#define PTHREAD_PROCESS_PRIVATE 0
#define PTHREAD_PROCESS_SHARED  1

#define PTHREAD_BARRIER_SERIAL_THREAD -1

#define PTHREAD_STACK_MIN (16 * 1024)

#define PTHREAD_CREATE_JOINABLE 0
#define PTHREAD_CREATE_DETACHED 1

#define PTHREAD_KEYS_MAX 256

#define PTHREAD_MUTEX_DEFAULT    PTHREAD_MUTEX_NORMAL
#define PTHREAD_MUTEX_NORMAL     0
#define PTHREAD_MUTEX_RECURSIVE  1
#define PTHREAD_MUTEX_ERRORCHECK 2

typedef struct pthread *pthread_t;
typedef struct pthread_attr pthread_attr_t;
typedef struct pthread_barrier pthread_barrier_t;
typedef struct pthread_barrierattr pthread_barrierattr_t;
typedef struct pthread_cond pthread_cond_t;
typedef struct pthread_condattr pthread_condattr_t;
typedef size_t pthread_key_t;
typedef struct pthread_mutex pthread_mutex_t;
typedef struct pthread_mutexattr pthread_mutexattr_t;
typedef struct pthread_once pthread_once_t;
typedef struct pthread_rwlock pthread_rwlock_t;
typedef struct pthread_rwlockattr pthread_rwlockattr_t;
typedef struct pthread_spinlock pthread_spinlock_t;

struct pthread_barrier
{
	unsigned count;
	unsigned value;
	unsigned revision;
};

struct pthread_barrierattr
{
};

struct pthread_cond
{
	pthread_mutex_t *mutex;
	uint32_t waiters;
	uint32_t value;
};

struct pthread_condattr
{
};

struct pthread_mutex
{
	pthread_t owner;
	uint32_t value;
	int type;
};

struct pthread_mutexattr
{
	int type;
};

struct pthread_once
{
	unsigned value;
};

struct pthread_rwlock
{
	pthread_t owner;
	uint32_t value;
};

struct pthread_rwlockattr
{
};

struct pthread_spinlock
{
	pthread_t owner;
	uint32_t value;
};

struct pthread
{
	/* keep _thread_fork infos at the top */
	void *stack;
	size_t stack_size;

	void *dl_tls;
	void *(*start)(void*);
	void *arg;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	void *ret;
	pid_t tid;
	int flags;
	void *keys[PTHREAD_KEYS_MAX];
};

struct pthread_attr
{
	size_t stack_size;
	void *stack;
	int detachstate;
};

struct timespec;

int pthread_attr_init(pthread_attr_t *attr);
int pthread_attr_destroy(pthread_attr_t *attr);
int pthread_attr_setstacksize(pthread_attr_t *attr, size_t size);
int pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *size);
int pthread_attr_setstack(pthread_attr_t *attr, void *stack, size_t size);
int pthread_attr_getstack(const pthread_attr_t *attr, void **stack, size_t *size);
int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate);
int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate);
int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                   void *(*start)(void*), void *arg);
int pthread_detach(pthread_t thread);
int pthread_join(pthread_t thread, void **ret);
void pthread_exit(void *ret) __attribute__((noreturn));
int pthread_kill(pthread_t thread, int sig);
int pthread_cancel(pthread_t thread);
pthread_t pthread_self(void);
int pthread_compare(pthread_t t1, pthread_t t2);
int pthread_equal(pthread_t t1, pthread_t t2);
int pthread_atfork(void (*prepare)(void), void (*parent)(void),
                   void (*child)(void));
int pthread_setschedprio(pthread_t thread, int prio);

int pthread_barrierattr_init(pthread_barrierattr_t *attr);
int pthread_barrierattr_destroy(pthread_barrierattr_t *attr);
int pthread_barrier_init(pthread_barrier_t *barrier,
                         const pthread_barrierattr_t *attr,
                         unsigned count);
int pthread_barrier_destroy(pthread_barrier_t *barrier);
int pthread_barrier_wait(pthread_barrier_t *barrier);

void pthread_cleanup_push(void (*routine)(void*), void *arg);
void pthread_cleanup_pop(int execute);

int pthread_condattr_init(pthread_condattr_t *attr);
int pthread_condattr_destroy(pthread_condattr_t *attr);
int pthread_cond_init(pthread_cond_t *cond,
                      const pthread_condattr_t *attr);
int pthread_cond_destroy(pthread_cond_t *cond);
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex,
                           const struct timespec *timeout);
int pthread_cond_signal(pthread_cond_t *cond);
int pthread_cond_broadcast(pthread_cond_t *cond);

int pthread_key_create(pthread_key_t *key, void (*destructor)(void*));
int pthread_key_delete(pthread_key_t key);
void *pthread_getspecific(pthread_key_t key);
int pthread_setspecific(pthread_key_t key, const void *val);
int _pthread_key_cleanup(pthread_t thread);

int pthread_mutexattr_init(pthread_mutexattr_t *attr);
int pthread_mutexattr_destroy(pthread_mutexattr_t *attr);
int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type);
int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type);
int pthread_mutex_init(pthread_mutex_t *mutex,
                       const pthread_mutexattr_t *attr);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_trylock(pthread_mutex_t *mutex);
int pthread_mutex_timedlock(pthread_mutex_t *mutex,
                            const struct timespec *abstime);
int pthread_mutex_unlock(pthread_mutex_t *mutex);

int pthread_once(pthread_once_t *once, void (*init)(void));

int pthread_rwlockattr_init(pthread_rwlockattr_t *attr);
int pthread_rwlockattr_destroy(pthread_rwlockattr_t *attr);
int pthread_rwlock_init(pthread_rwlock_t *rwlock,
                        const pthread_rwlockattr_t *attr);
int pthread_rwlock_destroy(pthread_rwlock_t *rwlock);
int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock);
int pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock);
int pthread_rwlock_timedrdlock(pthread_rwlock_t *rwlock,
                               const struct timespec *abstime);
int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock);
int pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock);
int pthread_rwlock_timedwrlock(pthread_rwlock_t *rwlock,
                               const struct timespec *abstime);
int pthread_rwlock_unlock(pthread_rwlock_t *rwlock);

int pthread_spin_init(pthread_spinlock_t *lock, int pshared);
int pthread_spin_destroy(pthread_spinlock_t *lock);
int pthread_spin_lock(pthread_spinlock_t *lock);
int pthread_spin_trylock(pthread_spinlock_t *lock);
int pthread_spin_unlock(pthread_spinlock_t *lock);

#ifdef __cplusplus
}
#endif

#endif
