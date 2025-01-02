#include <mutex.h>
#include <sched.h>
#include <time.h>
#include <proc.h>
#include <std.h>
#include <cpu.h>
#include <mem.h>

struct runq
{
	TAILQ_HEAD(, thread) threads; /* priority-ordered schedulable threads */
	struct timespec last_tick;
	struct mutex mutex;
};

static struct runq g_runq[MAXCPU];

static struct thread *find_better_thread(int ignoreidle);

void sched_init(void)
{
	for (size_t i = 0; i < sizeof(g_runq) / sizeof(*g_runq); ++i)
	{
		struct runq *runq = &g_runq[i];
		TAILQ_INIT(&runq->threads);
		mutex_init(&runq->mutex, 0);
	}
}

static void runq_enqueue(struct runq *runq, struct thread *thread)
{
	struct thread *it;
#if 0
	printf("[cpu %2" PRIu32 "] added thread %p (%s; %#zx) to runq\n",
	       curcpu()->id, thread, thread->proc->name,
	       arch_get_instruction_pointer(&thread->tf_user));
#endif
	mutex_spinlock(&runq->mutex);
	TAILQ_FOREACH(it, &runq->threads, runq_chain)
	{
		if (thread->pri >= it->pri)
			continue;
		thread->runq = runq;
		TAILQ_INSERT_BEFORE(it, thread, runq_chain);
		mutex_unlock(&runq->mutex);
		return;
	}
	thread->runq = runq;
	TAILQ_INSERT_TAIL(&runq->threads, thread, runq_chain);
	mutex_unlock(&runq->mutex);
}

void sched_enqueue(struct thread *thread)
{
	runq_enqueue(&g_runq[curcpu()->id], thread);
}

void sched_dequeue(struct thread *thread)
{
	struct runq *runq = thread->runq;
	if (!runq)
		return;
	mutex_spinlock(&runq->mutex);
	thread->runq = NULL;
	TAILQ_REMOVE(&runq->threads, thread, runq_chain);
	mutex_unlock(&runq->mutex);
}

static void test_better_thread(void)
{
	struct thread *better = find_better_thread(1);
	if (better)
		sched_switch(better);
}

void sched_run(struct thread *thread)
{
	struct cpu *cpu = curcpu();
	struct cpu *it;

	runq_enqueue(&g_runq[thread->wait_cpuid], thread);
	if (cpu->thread == cpu->idlethread)
		test_better_thread();
	CPU_FOREACH(it)
	{
		if (it == cpu)
			continue;
		if (it->thread == it->idlethread)
			arch_cpu_ipi(it);
	}
}

static void switch_thread(struct thread *thread)
{
	struct cpu *cpu = curcpu();
	struct thread *current = cpu->thread;
	if (current->tf_nest_level == 1)
		arch_save_fpu(current->tf_user.fpu_data);
	current->stats.nctxsw++;
	current->proc->stats.nctxsw++;
	current->running_cpuid = -1;
	proc_add_time_leave();
	thread_ref(thread);
	cpu->thread = thread;
	thread_free(current);
	thread->running_cpuid = cpu->id;
	cpu->trapframe = thread->tf_nest_level > 1 ? &thread->tf_kern : &thread->tf_user;
	thread->state = THREAD_RUNNING;
	arch_vm_setspace(thread->proc->vm_space);
	cpu->thread_vm_revision = thread->proc->vm_space->revision;
	if (thread->tf_nest_level == 1)
		arch_load_fpu(thread->tf_user.fpu_data);
}

void sched_switch(struct thread *thread)
{
	struct thread *curthread = curcpu()->thread;
	if (thread == curthread)
		return;
#if 0
	printf("[cpu %2" PRIu32 "] changing thread\n"
	       "\tFROM %p (%s IPU %#zx IPK %#zx NEST %zu PRI %" PRId32 " ST %d)\n"
	       "\tTO   %p (%s IPU %#zx IPK %#zx NEST %zu PRI %" PRId32 " ST %d)\n",
	       curcpu()->id, curthread, curthread ? curthread->proc->name : "",
	       curthread ? arch_get_instruction_pointer(&curthread->tf_user) : 0,
	       curthread ? arch_get_instruction_pointer(&curthread->tf_kern) : 0,
	       curthread ? curthread->tf_nest_level : 0,
	       curthread ? curthread->pri : 0,
	       curthread ? curthread->state : 0,
	       thread,
	       thread->proc->name,
	       arch_get_instruction_pointer(&thread->tf_user),
	       arch_get_instruction_pointer(&thread->tf_kern),
	       thread ? thread->tf_nest_level : 0,
	       thread ? thread->pri : 0,
	       thread ? thread->state : 0);
#endif
	if (curthread && curthread->state == THREAD_RUNNING)
	{
#if 0
		printf("[cpu %2" PRIu32 "] thread %p (%s) now paused\n",
		       curcpu()->id, curthread, curthread->proc->name);
#endif
		curthread->state = THREAD_PAUSED;
		sched_enqueue(curthread);
	}
#if 0
	printf("[cpu %2" PRIu32 "] thread %p (%s) now running sigqueue 0x%lx\n",
	       curcpu()->id, thread, thread->proc->name, thread->sigqueue);
#endif
	switch_thread(thread);
}

static struct thread *find_runq_thread(size_t cpuid,
                                       int ignoreidle)
{
	struct runq *runq = &g_runq[cpuid];
	struct thread *thread;
	mutex_spinlock(&runq->mutex);
	TAILQ_FOREACH(thread, &runq->threads, runq_chain)
	{
		if (thread->state != THREAD_PAUSED)
			continue;
		/* don't steal from other CPU if we're in kernel
		 * for example, if we're inside a syscall handler
		 * and we just returned from a wait, the cpu pointers
		 * used in the code would be invalid and cause harm
		 */
		if (thread->tf_nest_level > 1 && cpuid != curcpu()->id)
			continue;
		if (ignoreidle && thread == g_cpus[cpuid].idlethread)
			continue;
		if (!CPUMASK_GET(&thread->affinity, curcpu()->id))
			continue;
		thread->runq = NULL;
		TAILQ_REMOVE(&runq->threads, thread, runq_chain);
		break;
	}
#if 0
	if (thread)
		printf("[cpu %2" PRIu32 "] removed thread %p (%s; %#zx) from runq\n",
		       curcpu()->id, thread, thread->proc->name,
		       arch_get_instruction_pointer(&thread->tf_user));
#endif
	mutex_unlock(&runq->mutex);
	return thread;
}

static struct thread *find_thread(int ignoreidle)
{
	struct thread *thread = find_runq_thread(curcpu()->id, ignoreidle);
	if (thread && thread != curcpu()->idlethread)
		return thread;
	for (size_t i = 0; i < g_ncpus; ++i)
	{
		if (i == curcpu()->id)
			continue;
		struct thread *other_thread = find_runq_thread(i, 1);
		if (!other_thread)
			continue;
		sched_enqueue(thread);
		return other_thread;
	}
	return thread;
}

static struct thread *find_better_runq_thread(size_t cpuid,
                                              struct thread *relative,
                                              int ignoreidle)
{
	struct runq *runq = &g_runq[cpuid];
	struct thread *thread;
	mutex_spinlock(&runq->mutex);
	TAILQ_FOREACH(thread, &runq->threads, runq_chain)
	{
		if (thread == relative) /* XXX shouldn't happen */
			continue;
		if (thread->state != THREAD_PAUSED)
			continue;
		/* don't steal from other CPU if we're in kernel
		 * for example, if we're inside a syscall handler
		 * and we just returned from a wait, the cpu pointers
		 * used in the code would be invalid and cause harm
		 */
		if (thread->tf_nest_level > 1 && cpuid != curcpu()->id)
			continue;
		if (ignoreidle && thread == g_cpus[cpuid].idlethread)
			continue;
		if (relative && thread->pri > relative->pri)
		{
			mutex_unlock(&runq->mutex);
			return NULL;
		}
		if (!CPUMASK_GET(&thread->affinity, curcpu()->id))
			continue;
		thread->runq = NULL;
		TAILQ_REMOVE(&runq->threads, thread, runq_chain);
		break;
	}
#if 0
	if (thread)
		printf("[cpu %2" PRIu32 "] removed thread %p (%s; %#zx) from runq\n",
		       curcpu()->id, thread, thread->proc->name,
		       arch_get_instruction_pointer(&thread->tf_user));
#endif
	mutex_unlock(&runq->mutex);
	return thread;
}

static struct thread *find_better_thread(int ignoreidle)
{
	struct cpu *cpu = curcpu();
	struct thread *curthread = cpu->thread;
	struct thread *thread = find_better_runq_thread(cpu->id, curthread, ignoreidle);
	if (thread && thread != cpu->idlethread)
		return thread;
	for (size_t i = 0; i < g_ncpus; ++i)
	{
		if (i == cpu->id)
			continue;
		struct thread *other_thread = find_better_runq_thread(i,
		                                                      curthread,
		                                                      1);
		if (other_thread)
			return other_thread;
	}
	return thread;
}

static int test_paused_thread(void)
{
	struct thread *curthread = curcpu()->thread;
	if (curthread && curthread->state != THREAD_RUNNING)
	{
		struct thread *thread = find_thread(0);
		if (!thread)
			panic("can't find paused thread to run\n");
		sched_switch(thread);
		return 1;
	}
	return 0;
}

void sched_resched(void)
{
	if (test_paused_thread())
		return;
	test_better_thread();
}

void sched_yield(void)
{
	struct thread *thread = find_thread(1);
	if (!thread)
		return;
	struct timespec req;
	req.tv_sec = 0;
	req.tv_nsec = 0;
	thread_sleep(&req);
}

void sched_test(void)
{
	test_paused_thread();
}

void sched_tick(const struct timespec *ts)
{
	struct cpu *cpu = curcpu();
	struct cpu *it;
	struct runq *runq = &g_runq[cpu->id];
	struct timespec diff;

	timespec_sub(&diff, ts, &runq->last_tick);
	if (!diff.tv_sec && diff.tv_nsec < 10000000)
		return;
	runq->last_tick = *ts;
	test_better_thread();
	CPU_FOREACH(it)
	{
		if (it == cpu)
			continue;
		__atomic_store_n(&it->must_resched, 1, __ATOMIC_SEQ_CST);
		arch_cpu_ipi(it);
	}
}
