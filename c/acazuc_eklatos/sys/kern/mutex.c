#include <spinlock.h>
#include <mutex.h>
#include <cpu.h>
#include <std.h>

void mutex_init(struct mutex *mutex, int flags)
{
	spinlock_init(&mutex->spinlock);
	waitq_init(&mutex->waitq);
	mutex->owner = NULL;
	mutex->flags = flags;
	mutex->recursive_nb = 0;
}

void mutex_destroy(struct mutex *mutex)
{
	spinlock_lock(&mutex->spinlock);
	assert(!mutex->owner, "destroying locked mutex\n");
	spinlock_destroy(&mutex->spinlock);
	waitq_destroy(&mutex->waitq);
}

static void mutex_acquire(struct mutex *mutex)
{
	mutex->owner = curcpu()->thread;
	if (mutex->flags & MUTEX_RECURSIVE)
		mutex->recursive_nb = 1;
}

void mutex_lock(struct mutex *mutex)
{
	spinlock_lock(&mutex->spinlock);
	if (!mutex->owner)
	{
		mutex_acquire(mutex);
		spinlock_unlock(&mutex->spinlock);
		return;
	}
	struct thread *thread = curcpu()->thread;
	do
	{
		if (mutex->owner == thread)
		{
			if (!(mutex->flags & MUTEX_RECURSIVE))
				panic("recursive lock without recursive flags\n");
			mutex->recursive_nb++;
			spinlock_unlock(&mutex->spinlock);
			return;
		}
		waitq_wait_tail(&mutex->waitq, &mutex->spinlock, NULL);
	} while (mutex->owner);
	mutex->owner = thread;
	if (mutex->flags & MUTEX_RECURSIVE)
		mutex->recursive_nb = 1;
	spinlock_unlock(&mutex->spinlock);
}

void mutex_spinlock(struct mutex *mutex)
{
	spinlock_lock(&mutex->spinlock);
	if (!mutex->owner)
	{
		mutex_acquire(mutex);
		spinlock_unlock(&mutex->spinlock);
		return;
	}
	struct thread *thread = curcpu()->thread;
	do
	{
		if (mutex->owner == thread)
		{
			if (!(mutex->flags & MUTEX_RECURSIVE))
				panic("recursive lock without recursive flags\n");
			mutex->recursive_nb++;
			spinlock_unlock(&mutex->spinlock);
			return;
		}
		spinlock_unlock(&mutex->spinlock);
		arch_spin_yield();
		spinlock_lock(&mutex->spinlock);
	} while (mutex->owner);
	mutex->owner = thread;
	if (mutex->flags & MUTEX_RECURSIVE)
		mutex->recursive_nb = 1;
	spinlock_unlock(&mutex->spinlock);
}

int mutex_trylock(struct mutex *mutex)
{
	int res;
	if (spinlock_trylock(&mutex->spinlock))
		return 1;
	if (!mutex->owner)
	{
		mutex_acquire(mutex);
		res = 0;
	}
	else if (mutex->owner == curcpu()->thread)
	{
		if (!(mutex->flags & MUTEX_RECURSIVE))
			panic("recursive lock without recursive flags\n");
		mutex->recursive_nb++;
		res = 0;
	}
	else
	{
		res = 1;
	}
	spinlock_unlock(&mutex->spinlock);
	return res;
}

void mutex_unlock(struct mutex *mutex)
{
	spinlock_lock(&mutex->spinlock);
	/* check thread to avoid panic in early boot sequence */
	if (!mutex->owner && curcpu()->thread)
		panic("unlocking mutex without owner\n");
	if (mutex->flags & MUTEX_RECURSIVE)
	{
		/* check thread to avoid panic in early boot sequence */
		if (!mutex->recursive_nb && mutex->owner)
			panic("unlocking non-acquired recursive mutex\n");
		mutex->recursive_nb--;
		if (!mutex->recursive_nb)
		{
			mutex->owner = NULL;
			waitq_signal(&mutex->waitq, 0);
		}
	}
	else
	{
		mutex->owner = NULL;
		waitq_signal(&mutex->waitq, 0);
	}
	spinlock_unlock(&mutex->spinlock);
}
