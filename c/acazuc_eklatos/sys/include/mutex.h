#ifndef MUTEX_H
#define MUTEX_H

#include <spinlock.h>
#include <waitq.h>
#include <types.h>

#define MUTEX_RECURSIVE (1 << 0)

struct mutex
{
	struct spinlock spinlock;
	struct waitq waitq;
	struct thread *owner;
	int flags;
	size_t recursive_nb;
};

void mutex_init(struct mutex *mutex, int flags);
void mutex_destroy(struct mutex *mutex);
void mutex_lock(struct mutex *mutex);
void mutex_spinlock(struct mutex *mutex);
int mutex_trylock(struct mutex *mutex);
void mutex_unlock(struct mutex *mutex);

#endif
