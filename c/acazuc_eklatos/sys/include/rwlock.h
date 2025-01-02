#ifndef RWLOCK_H
#define RWLOCK_H

#include <spinlock.h>
#include <waitq.h>
#include <types.h>

struct thread;

struct rwlock
{
	struct spinlock spinlock;
	struct waitq rwaitq;
	struct waitq wwaitq;
	struct thread *wowner;
	size_t rlock_nb;
};

void rwlock_init(struct rwlock *rwlock);
void rwlock_destroy(struct rwlock *rwlock);
void rwlock_rdlock(struct rwlock *rwlock);
int rwlock_tryrdlock(struct rwlock *rwlock);
void rwlock_wrlock(struct rwlock *rwlock);
int rwlock_tryrwlock(struct rwlock *rwlock);
void rwlock_unlock(struct rwlock *rwlock);

#endif
