#define ENABLE_TRACE

#include <sched.h>
#include <file.h>
#include <proc.h>
#include <std.h>
#include <irq.h>
#include <vfs.h>
#include <uio.h>
#include <cpu.h>

static uint64_t interrupt_count[IRQ_COUNT];
#if __SIZE_WIDTH__ == 32
static struct spinlock interrupt_count_lock = SPINLOCK_INITIALIZER();
#endif

void register_irq(struct irq_handle *handle, enum irq_type type,
                  size_t id, size_t cpuid, irq_fn_t fn, void *userdata)
{
	handle->type = type;
	handle->id = id;
	handle->cpuid = cpuid;
	handle->userdata = userdata;
	handle->fn = fn;
	TAILQ_INSERT_TAIL(&g_cpus[cpuid].irq_handles[id], handle, chain);
}

void unregister_irq(struct irq_handle *handle)
{
	if (!handle->fn)
		return;
	struct cpu *cpu = &g_cpus[handle->cpuid];
	TAILQ_REMOVE(&cpu->irq_handles[handle->id], handle, chain);
	if (!TAILQ_EMPTY(&cpu->irq_handles[handle->id]))
		return;
	switch (handle->type)
	{
		case IRQ_USR:
			break;
		case IRQ_NATIVE:
			arch_disable_native_irq(handle);
			break;
		case IRQ_MSI:
			break;
		case IRQ_MSIX:
			break;
		default:
			panic("unknown interrupt type\n");
	}
	handle->fn = NULL;
}

void trap_handle(size_t id, struct irq_ctx *ctx)
{
	struct cpu *cpu = curcpu();

	cpu_test_panic();

	struct thread *thread = cpu->thread;
	if (thread)
	{
		if (!thread->tf_nest_level)
			kernel_lock();
		thread->tf_nest_level++;
		if (thread->tf_nest_level > 2)
			panic("too much nested interrupt\n");
		cpu->trapframe = &thread->tf_kern;
		proc_add_time_enter();
	}

	arch_trap_handle(id, ctx);
	trap_return();
}

void trap_return(void)
{
	struct cpu *cpu = curcpu();
	struct thread *thread = cpu->thread;

	if (thread && thread->tf_nest_level < 2)
		thread_handle_signals(thread);

	/* XXX that's hacky, but if we're here NOT because of the
	 * cpu sync IRQ (e.g: a fault was pending), then no resched
	 * will be attempted, causing the dead thread continue its
	 * execution, causing death because of death
	 *
	 * even without this, it is always a good thing to make sure
	 * the current thread is runnable when:
	 * - going to userland
	 * - going from wait irq
	 *
	 * NB: we don't want to switch thread when we're inside the kernel
	 * as this would cause very non-linear code flow
	 */
	if (thread)
	{
		if ((thread->state == THREAD_ZOMBIE && thread->tf_nest_level < 2)
		 || thread->state == THREAD_WAITING)
			sched_test();
	}


	thread = cpu->thread;
	if (thread)
	{
		proc_add_time_leave();
		arch_set_trap_stack(&thread->int_stack[thread->int_stack_size]);
		thread->tf_nest_level--;
		cpu->trapframe = thread->tf_nest_level ? &thread->tf_kern : &thread->tf_user;
		if (!thread->tf_nest_level)
		{
			if (thread->ptrace_state == PTRACE_ST_ONESTEP)
				arch_set_singlestep(thread);
		}
#if 0
		if (thread != cpu->idlethread)
		{
			printf("jump back to PID %" PRId32 " TID %" PRId32 " %s PC %p SP %p CPU %" PRIu32 " NEST %zd\n",
			       thread->proc->pid,
			       thread->tid,
			       thread->proc->name,
			       (void*)arch_get_instruction_pointer(cpu->trapframe),
			       (void*)arch_get_stack_pointer(cpu->trapframe),
			       cpu->id,
			       thread->tf_nest_level);
		}
#endif
		if (!thread->tf_nest_level)
		{
			arch_set_tls_addr(thread->tls_addr);
			cpu_sync_leave();
		}
		else
		{
			arch_waitq_leave();
		}
	}
	arch_trap_return();
}

void irq_execute(size_t id)
{
	struct cpu *cpu = curcpu();

	proc_update_loadavg();
	assert(id < IRQ_COUNT, "invalid irq\n");
#if __SIZE_WIDTH__ == 32
	spinlock_lock(&interrupt_count_lock);
	interrupt_count[id]++;
	spinlock_unlock(&interrupt_count_lock);
#else
	__atomic_add_fetch(&interrupt_count[id], 1, __ATOMIC_SEQ_CST);
#endif
	cpu->interrupt_count[id]++;

	if (TAILQ_EMPTY(&cpu->irq_handles[id]))
	{
		TRACE("unhandled interrupt 0x%02" PRIx8 " on cpu %" PRIu32,
		       id, cpu->id);
		return;
	}
	struct irq_handle *handle;
	TAILQ_FOREACH(handle, &cpu->irq_handles[id], chain)
	{
		assert(handle->fn, "no function defined in handle\n");
		handle->fn(handle->userdata);
	}
	cpu_tick();
}

static ssize_t irqinfo_read(struct file *file, struct uio *uio)
{
	(void)file;
	size_t count = uio->count;
	off_t off = uio->off;
	uprintf(uio, "%-4s:", "id");
	for (size_t i = 0; i < g_ncpus; ++i)
	{
		char name[10];
		snprintf(name, sizeof(name), "cpu%zu", i);
		uprintf(uio, "%10s", name);
	}
	uprintf(uio, "%10s", "sum");
	uprintf(uio, "\n");
	for (uint32_t i = 0; i < IRQ_COUNT; ++i)
	{
		if (!interrupt_count[i])
			continue;
		uprintf(uio, "%-#4" PRIx32 ":", i);
		struct cpu *cpu;
		CPU_FOREACH(cpu)
		{
			uprintf(uio, "%10" PRIu64, cpu->interrupt_count[i]);
		}
		uprintf(uio, "%10" PRIu64 "\n", interrupt_count[i]);
	}
	uio->off = off + count - uio->count;
	return count - uio->count;
}

static const struct file_op irqinfo_fop =
{
	.read = irqinfo_read,
};

int irq_register_sysfs(void)
{
	return sysfs_mknode("irqinfo", 0, 0, 0444, &irqinfo_fop, NULL);
}
