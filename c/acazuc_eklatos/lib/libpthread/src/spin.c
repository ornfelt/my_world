#include <pthread.h>
#include <errno.h>

int
pthread_spin_init(pthread_spinlock_t *lock, int pshared)
{
	(void)pshared; /* XXX */
	if (!lock)
		return EINVAL;
	lock->owner = NULL;
	lock->value = 0;
	return 0;
}

int
pthread_spin_destroy(pthread_spinlock_t *lock)
{
	if (!lock)
		return EINVAL;
	return 0;
}

static int
acquire(pthread_spinlock_t *lock)
{
	int expected = 0;
	return __atomic_compare_exchange_n(&lock->value, &expected, 1, 0,
	                                   __ATOMIC_ACQUIRE,
	                                   __ATOMIC_RELAXED);
}

int
pthread_spin_lock(pthread_spinlock_t *lock)
{
	if (!lock)
		return EINVAL;
	if (lock->owner == pthread_self())
		return EDEADLK;
	while (!acquire(lock))
#if defined(__i386__) || defined(__x86_64__)
		__asm__ volatile ("pause" : : : "memory");
#elif defined(__arm__) || defined(__aarch64__)
		__asm__ volatile ("yield" : : : "memory");
#elif defined(__riscv) || defined(__mips__)
		;
#else
# error "unknown arch"
#endif
	lock->owner = pthread_self();
	return 0;
}

int
pthread_spin_trylock(pthread_spinlock_t *lock)
{
	if (!lock)
		return EINVAL;
	if (!acquire(lock))
		return EBUSY;
	lock->owner = pthread_self();
	return 0;
}

int
pthread_spin_unlock(pthread_spinlock_t *lock)
{
	if (!lock)
		return EINVAL;
	if (lock->owner != pthread_self())
		return EINVAL; /* XXX another errno ? */
	lock->owner = NULL;
	__atomic_store_n(&lock->value, 0, __ATOMIC_RELEASE);
	return 0;
}
