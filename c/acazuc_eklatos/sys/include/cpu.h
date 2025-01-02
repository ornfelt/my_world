#ifndef CPU_H
#define CPU_H

#include <arch/arch.h>
#include <arch/irq.h>

#include <spinlock.h>
#include <types.h>
#include <queue.h>
#include <time.h>

struct cpu
{
	struct cpu *self;
	struct trapframe *trapframe; /* trapframe used for incoming interrupt */
	struct arch_cpu arch;
	uint32_t id;
	uint8_t *stack;
	size_t stack_size;
	size_t must_resched;
	size_t vm_revision;
	size_t thread_vm_revision;
	struct thread *thread;
	struct thread *idlethread;
	struct timespec last_proc_time; /* last time of process duration measurement (jump to userland for user time, interrupt enter / waitq leave for sys) */
	struct timespec user_time;
	struct timespec sys_time;
	struct timespec idle_time;
	struct timespec loadavg_last_idle; /* last user time for idle thread */
	struct timespec loadavg_time; /* last update time */
	uint32_t loadavg[3]; /* fixed point 16.16 */
	struct arch_copy_zone copy_src_page;
	struct arch_copy_zone copy_dst_page;
	uint64_t interrupt_count[IRQ_COUNT];
	TAILQ_HEAD(, irq_handle) irq_handles[IRQ_COUNT];
} __attribute__((aligned(PAGE_SIZE)));

static inline struct cpu *curcpu(void)
{
#if defined(__amd64__)
	return ((__seg_gs struct cpu*)0)->self;
#else
	return __builtin_thread_pointer();
#endif
}

#define CPU_FOREACH(v) for (v = &g_cpus[0]; v < &g_cpus[g_ncpus]; v++)

#define CPUMASK_BPW (sizeof(size_t) * 8)
#define CPUMASK_WORDS ((MAXCPU + CPUMASK_BPW - 1) / CPUMASK_BPW)
#define CPUMASK_BIT(n) ((size_t)1 << ((n) % CPUMASK_BPW))
#define CPUMASK_OFF(n) ((n) / CPUMASK_BPW)

typedef struct cpumask
{
	size_t mask[CPUMASK_WORDS];
} cpumask_t;

#define CPUMASK_FILL(m) \
do \
{ \
	for (size_t i = 0; i < CPUMASK_WORDS; ++i) \
		(m)->mask[i] = (size_t)-1; \
} while (0)
#define CPUMASK_CLEAR(m) \
do \
{ \
	for (size_t i = 0; i < CPUMASK_WORDS; ++i) \
		(m)->mask[i] = 0; \
} while (0)

#define CPUMASK_GET(m, n) ((m)->mask[CPUMASK_OFF(n)] & CPUMASK_BIT(n))
#define CPUMASK_SET(m, n, v) \
do \
{ \
	if (v) \
		(m)->mask[CPUMASK_OFF(n)] |= CPUMASK_BIT(n); \
	else \
		(m)->mask[CPUMASK_OFF(n)] &= ~CPUMASK_BIT(n); \
} while (0)

void arch_cpu_ipi(struct cpu *cpu);
void arch_init_copy_zone(struct arch_copy_zone *zone);
void *arch_set_copy_zone(struct arch_copy_zone *zone, uintptr_t poff);
int arch_register_sysfs(void);
void arch_paging_init(void);
void arch_device_init(void);
void arch_cpu_boot(struct cpu *cpu);
void arch_start_smp(void);
int arch_start_smp_cpu(struct cpu *cpu, size_t smp_id);

void cpu_sync(cpumask_t *mask);
void cpu_test_panic(void);
void cpu_sync_leave(void);
void cpu_ipi_other(struct cpu *from);
void cpu_tick(void);

void cpu_start_smp(size_t ncpu);

void kernel_lock(void);
void kernel_unlock(void);

extern struct cpu g_cpus[MAXCPU];
extern size_t g_ncpus;

#endif
