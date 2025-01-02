#include <rwlock.h>
#include <cpu.h>
#include <std.h>

void rwlock_init(struct rwlock *rwlock)
{
	spinlock_init(&rwlock->spinlock);
	waitq_init(&rwlock->rwaitq);
	waitq_init(&rwlock->wwaitq);
	rwlock->wowner = NULL;
	rwlock->rlock_nb = 0;
}

void rwlock_destroy(struct rwlock *rwlock)
{
	spinlock_destroy(&rwlock->spinlock);
	waitq_destroy(&rwlock->rwaitq);
	waitq_destroy(&rwlock->wwaitq);
}

void rwlock_rdlock(struct rwlock *rwlock)
{
	spinlock_lock(&rwlock->spinlock);
	while (1)
	{
		while (rwlock->wowner)
			waitq_wait_tail(&rwlock->rwaitq, &rwlock->spinlock, NULL);
		spinlock_lock(&rwlock->wwaitq.spinlock);
		if (TAILQ_EMPTY(&rwlock->wwaitq.watchers))
		{
			spinlock_unlock(&rwlock->wwaitq.spinlock);
			break;
		}
		spinlock_unlock(&rwlock->wwaitq.spinlock);
		waitq_wait_tail(&rwlock->rwaitq, &rwlock->spinlock, NULL);
	}
	rwlock->rlock_nb++;
	spinlock_unlock(&rwlock->spinlock);
}

int rwlock_tryrdlock(struct rwlock *rwlock)
{
	spinlock_lock(&rwlock->spinlock);
	if (rwlock->wowner)
	{
		spinlock_unlock(&rwlock->spinlock);
		return 1;
	}
	spinlock_lock(&rwlock->wwaitq.spinlock);
	if (!TAILQ_EMPTY(&rwlock->wwaitq.watchers))
	{
		spinlock_unlock(&rwlock->wwaitq.spinlock);
		spinlock_unlock(&rwlock->spinlock);
		return 1;
	}
	spinlock_unlock(&rwlock->wwaitq.spinlock);
	rwlock->rlock_nb++;
	spinlock_unlock(&rwlock->spinlock);
	return 0;
}

void rwlock_wrlock(struct rwlock *rwlock)
{
	spinlock_lock(&rwlock->spinlock);
	while (rwlock->wowner || rwlock->rlock_nb)
		waitq_wait_tail(&rwlock->wwaitq, &rwlock->spinlock, NULL);
	struct thread *thread = curcpu()->thread;
	if (!thread)
		thread = (struct thread*)0x1; /* XXX */
	rwlock->wowner = thread;
	spinlock_unlock(&rwlock->spinlock);
}

int rwlock_tryrwlock(struct rwlock *rwlock)
{
	spinlock_lock(&rwlock->spinlock);
	if (!rwlock->wowner && !rwlock->rlock_nb)
	{
		struct thread *thread = curcpu()->thread;
		if (!thread)
			thread = (struct thread*)0x1; /* XXX */
		rwlock->wowner = thread;
		spinlock_unlock(&rwlock->spinlock);
		return 0;
	}
	spinlock_unlock(&rwlock->spinlock);
	return 1;
}

void rwlock_unlock(struct rwlock *rwlock)
{
	spinlock_lock(&rwlock->spinlock);
	struct thread *thread = curcpu()->thread;
	if (!thread)
		thread = (struct thread*)0x1; /* XXX */
	if (rwlock->wowner && rwlock->wowner == thread)
	{
		rwlock->wowner = NULL;
		if (!waitq_signal(&rwlock->wwaitq, 0))
			waitq_broadcast(&rwlock->rwaitq, 0);
	}
	else
	{
		rwlock->rlock_nb--;
		if (!rwlock->rlock_nb)
			waitq_signal(&rwlock->wwaitq, 0);
	}
	spinlock_unlock(&rwlock->spinlock);
}
