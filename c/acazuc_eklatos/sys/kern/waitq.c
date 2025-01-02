#include <errno.h>
#include <sched.h>
#include <waitq.h>
#include <proc.h>
#include <cpu.h>
#include <std.h>

/* queue of threads waiting and having a timeout, ordered by timeout asc */
static struct spinlock timeout_lock = SPINLOCK_INITIALIZER();
static struct thread_head timeout_threads = TAILQ_HEAD_INITIALIZER(timeout_threads);

void waitq_init(struct waitq *waitq)
{
	spinlock_init(&waitq->spinlock);
	TAILQ_INIT(&waitq->watchers);
}

void waitq_destroy(struct waitq *waitq)
{
	spinlock_lock(&waitq->spinlock);
	assert(TAILQ_EMPTY(&waitq->watchers), "destroying non-empty waitq\n");
	spinlock_destroy(&waitq->spinlock);
}

static void wakeup_thread(struct waitq *waitq, struct thread *thread, int reason)
{
	TAILQ_REMOVE(&waitq->watchers, thread, waitq_chain);
	assert(thread->state == THREAD_WAITING, "waking up non-sleeping thread\n");
	thread->state = thread->proc->state == PROC_STOPPED ? THREAD_STOPPED : THREAD_PAUSED;
	thread->waitq = NULL;
	thread->waitq_ret = reason;
	if (thread->state == THREAD_PAUSED)
		sched_run(thread);
}

void waitq_wakeup_thread(struct waitq *waitq, struct thread *thread, int reason)
{
	if (thread->wait_timeout.tv_sec || thread->wait_timeout.tv_nsec)
	{
		spinlock_lock(&timeout_lock);
		TAILQ_REMOVE(&timeout_threads, thread, waitq_timeout_chain);
		spinlock_unlock(&timeout_lock);
	}
	wakeup_thread(waitq, thread, reason);
}

void waitq_check_timeout(const struct timespec *ts)
{
	struct thread *thread;
	while (1)
	{
		spinlock_lock(&timeout_lock);
		thread = TAILQ_FIRST(&timeout_threads);
		if (!thread)
			break;
		if (timespec_cmp(ts, &thread->wait_timeout) > 0)
		{
			TAILQ_REMOVE(&timeout_threads, thread, waitq_timeout_chain);
			spinlock_unlock(&timeout_lock);
			wakeup_thread(thread->waitq, thread, -EWOULDBLOCK);
			continue;
		}
		break;
	}
	spinlock_unlock(&timeout_lock);
}

static void add_waiting_thread(struct thread *thread)
{
	struct thread *it;
	spinlock_lock(&timeout_lock);
	TAILQ_FOREACH(it, &timeout_threads, waitq_timeout_chain)
	{
		if (timespec_cmp(&thread->wait_timeout, &it->wait_timeout) < 0)
		{
			TAILQ_INSERT_BEFORE(it, thread, waitq_timeout_chain);
			spinlock_unlock(&timeout_lock);
			return;
		}
	}
	TAILQ_INSERT_TAIL(&timeout_threads, thread, waitq_timeout_chain);
	spinlock_unlock(&timeout_lock);
}

static void prepare_sleep(struct thread *thread, struct waitq *waitq,
                          const struct timespec *timeout)
{
	assert(thread->state == THREAD_RUNNING, "waiting on non-running thread\n");
	thread->state = THREAD_WAITING;
	thread->waitq = waitq;
	thread->wait_cpuid = curcpu()->id;
	if (!timeout)
	{
		thread->wait_timeout.tv_sec = 0;
		thread->wait_timeout.tv_nsec = 0;
	}
	else
	{
		clock_gettime(CLOCK_MONOTONIC, &thread->wait_timeout);
		timespec_add(&thread->wait_timeout, &thread->wait_timeout,
		             timeout);
		add_waiting_thread(thread);
	}
}

static void sleep_enter(struct thread *thread)
{
	if (arch_waitq_sleep())
	{
		if (thread->proc->vfork_rel
		 && thread->proc->vfork_rel->parent == thread->proc)
			panic("wakeup vfork parent thread\n");
	}
	else
	{
		curcpu()->thread->tf_nest_level++;
		proc_add_time_enter();
		trap_return();
	}
}

static int do_sleep(struct thread *thread, struct waitq *waitq,
                    struct spinlock *spinlock,
                    const struct timespec *timeout)
{
	struct cpu *cpu = curcpu();

	if (!cpu->thread)
		panic("sleep without thread\n");
	if (cpu->thread == cpu->idlethread)
		panic("idlethread sleep\n");
	prepare_sleep(thread, waitq, timeout);
	spinlock_unlock(&waitq->spinlock);
	spinlock_unlock(spinlock);
	sleep_enter(thread);
	spinlock_lock(spinlock);
	return thread->waitq_ret;
}

static int do_sleep_mutex(struct thread *thread, struct waitq *waitq,
                          struct mutex *mutex,
                          const struct timespec *timeout)
{
	struct cpu *cpu = curcpu();

	if (!cpu->thread)
		panic("sleep without thread\n");
	if (cpu->thread == cpu->idlethread)
		panic("idlethread sleep\n");
	prepare_sleep(thread, waitq, timeout);
	spinlock_unlock(&waitq->spinlock);
	mutex_unlock(mutex);
	sleep_enter(thread);
	mutex_spinlock(mutex);
	return thread->waitq_ret;
}

int waitq_wait_tail(struct waitq *waitq, struct spinlock *spinlock,
                    const struct timespec *timeout)
{
	spinlock_lock(&waitq->spinlock);
	struct thread *thread = curcpu()->thread;
	TAILQ_INSERT_TAIL(&waitq->watchers, thread, waitq_chain);
	return do_sleep(thread, waitq, spinlock, timeout);
}

int waitq_wait_head(struct waitq *waitq, struct spinlock *spinlock,
                    const struct timespec *timeout)
{
	spinlock_lock(&waitq->spinlock);
	struct thread *thread = curcpu()->thread;
	TAILQ_INSERT_HEAD(&waitq->watchers, thread, waitq_chain);
	return do_sleep(thread, waitq, spinlock, timeout);
}

int waitq_wait_tail_mutex(struct waitq *waitq, struct mutex *mutex,
                          const struct timespec *timeout)
{
	spinlock_lock(&waitq->spinlock);
	struct thread *thread = curcpu()->thread;
	TAILQ_INSERT_TAIL(&waitq->watchers, thread, waitq_chain);
	return do_sleep_mutex(thread, waitq, mutex, timeout);
}

int waitq_wait_head_mutex(struct waitq *waitq, struct mutex *mutex,
                          const struct timespec *timeout)
{
	spinlock_lock(&waitq->spinlock);
	struct thread *thread = curcpu()->thread;
	TAILQ_INSERT_HEAD(&waitq->watchers, thread, waitq_chain);
	return do_sleep_mutex(thread, waitq, mutex, timeout);
}

int waitq_signal(struct waitq *waitq, int reason)
{
	spinlock_lock(&waitq->spinlock);
	int res = 0;
	struct thread *thread;
	TAILQ_FOREACH(thread, &waitq->watchers, waitq_chain)
	{
		if (thread->state != THREAD_WAITING
		 || thread->waitq != waitq)
			continue;
		waitq_wakeup_thread(waitq, thread, reason);
		res = 1;
		break;
	}
	spinlock_unlock(&waitq->spinlock);
	return res;
}

int waitq_broadcast(struct waitq *waitq, int reason)
{
	spinlock_lock(&waitq->spinlock);
	int res = 0;
	struct thread *thread;
	struct thread *next;
	TAILQ_FOREACH_SAFE(thread, &waitq->watchers, waitq_chain, next)
	{
		if (thread->state != THREAD_WAITING
		 || thread->waitq != waitq)
			continue;
		waitq_wakeup_thread(waitq, thread, reason);
		res++;
	}
	spinlock_unlock(&waitq->spinlock);
	return res;
}
