#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <arch/arch.h>

#include <types.h>

struct spinlock
{
	uintptr_t val;
};

#define SPINLOCK_INITIALIZER() {0}

static inline void spinlock_init(struct spinlock *spinlock)
{
	__atomic_store_n(&spinlock->val, 0, __ATOMIC_RELAXED);
}

static inline void spinlock_destroy(struct spinlock *spinlock)
{
	(void)spinlock;
}

static inline int spinlock_trylock(struct spinlock *spinlock)
{
	uintptr_t expected = 0;
	return __atomic_compare_exchange_n(&spinlock->val, &expected, 1, 0,
	                                   __ATOMIC_ACQUIRE, __ATOMIC_RELAXED);
}

static inline void spinlock_lock(struct spinlock *spinlock)
{
	while (!spinlock_trylock(spinlock))
		arch_spin_yield();
}

static inline void spinlock_unlock(struct spinlock *spinlock)
{
	__atomic_store_n(&spinlock->val, 0, __ATOMIC_RELEASE);
}

#endif
