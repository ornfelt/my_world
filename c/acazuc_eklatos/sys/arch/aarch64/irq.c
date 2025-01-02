#include "arch/aarch64/gicv2.h"

#include <arch/csr.h>
#include <arch/asm.h>

#include <sched.h>
#include <proc.h>
#include <std.h>
#include <irq.h>
#include <std.h>
#include <cpu.h>
#include <mem.h>

#define TRAP_EXCEPTION_EL1 1
#define TRAP_IRQ_EL1       2
#define TRAP_FIQ_EL1       3
#define TRAP_SERROR_EL1    4
#define TRAP_EXCEPTION_EL0 5
#define TRAP_IRQ_EL0       6
#define TRAP_FIQ_EL0       7
#define TRAP_SERROR_EL0    8

void handle_illegal_instruction_el1(struct irq_ctx *ctx)
{
	arch_print_regs(&ctx->trapframe->regs);
	panic("illegal instruction @ 0x%lx\n", ctx->trapframe->regs.pc);
}

void handle_data_abort_el1(struct irq_ctx *ctx)
{
	uint64_t far_el1 = get_far_el1();
	arch_print_regs(&ctx->trapframe->regs);
	panic("data abort 0x%lx @ 0x%lx ESR 0x%lx\n",
	      far_el1, ctx->trapframe->regs.pc, ctx->esr);
}

void handle_instruction_abort_el1(struct irq_ctx *ctx)
{
	uint64_t far_el1 = get_far_el1();
	arch_print_regs(&ctx->trapframe->regs);
	panic("instruction abort 0x%lx @ 0x%lx ESR 0x%lx\n",
	      far_el1, ctx->trapframe->regs.pc, ctx->esr);
}

void handle_data_abort_el0(struct irq_ctx *ctx)
{
	struct thread *thread = curcpu()->thread;
	if (!thread)
		panic("data abort el0 without thread\n");
	uint64_t far_el1 = get_far_el1();
	switch (ctx->esr & 0x3F)
	{
		case 0x4: /* translation fault level 0 */
		case 0x5: /* translation fault level 1 */
		case 0x6: /* translation fault level 2 */
		case 0x7: /* translation fault level 3 */
			thread_fault(thread, far_el1,
			             (ctx->esr & (1 << 6)) ? VM_PROT_W : VM_PROT_R);
			break;
		default:
			thread_signal(thread, SIGSEGV);
			break;
	}
}

void handle_instruction_abort_el0(struct irq_ctx *ctx)
{
	struct thread *thread = curcpu()->thread;
	if (!thread)
		panic("instruction abort el0 without thread\n");
	uint64_t far_el1 = get_far_el1();
	switch (ctx->esr & 0x1F)
	{
		case 0x4: /* translation fault level 0 */
		case 0x5: /* translation fault level 1 */
		case 0x6: /* translation fault level 2 */
		case 0x7: /* translation fault level 3 */
			thread_fault(thread, far_el1, VM_PROT_X);
			break;
		default:
			thread_signal(thread, SIGSEGV);
			break;
	}
}

void handle_svc_el0(struct irq_ctx *ctx)
{
	syscall(ctx->trapframe->regs.r[8],
	        ctx->trapframe->regs.r[0],
	        ctx->trapframe->regs.r[1],
	        ctx->trapframe->regs.r[2],
	        ctx->trapframe->regs.r[3],
	        ctx->trapframe->regs.r[4],
	        ctx->trapframe->regs.r[5]);
}

void handle_illegal_instruction_el0(struct irq_ctx *ctx)
{
	(void)ctx;
	struct thread *thread = curcpu()->thread;
	if (!thread)
		panic("illegal instruction el0 without thread\n");
	thread_illegal_instruction(thread);
}

void handle_ss_el0(struct irq_ctx *ctx)
{
	(void)ctx;
	struct thread *thread = curcpu()->thread;
	if (thread->ptrace_state != PTRACE_ST_ONESTEP)
		panic("debug single step without onestep ptrace\n");
	thread_ptrace_stop(thread, SIGTRAP);
	thread->tf_user.regs.psr &= ~PSR_SS;
}

void handle_irq(struct irq_ctx *ctx)
{
	(void)ctx;
	size_t irq;
	int ret = gicv2_get_active_interrupt(&irq);
	if (ret)
		panic("no interrupt active\n");
	gicv2_clear_interrupt(irq);
	gicv2_eoi(irq);
	irq_execute(irq);
}

void arch_trap_handle(size_t id, struct irq_ctx *ctx)
{
	switch (id)
	{
		case TRAP_EXCEPTION_EL1:
			//printf("exception el1\n");
			switch ((ctx->esr >> 26) & 0x3F)
			{
				case 0xE:
					handle_illegal_instruction_el1(ctx);
					break;
				case 0x21:
					handle_instruction_abort_el1(ctx);
					break;
				case 0x25:
					handle_data_abort_el1(ctx);
					break;
				default:
					panic("unhandled exception 0x%lx\n", (ctx->esr >> 26) & 0x3F);
			}
			break;
		case TRAP_IRQ_EL1:
			//printf("irq el1 %zu (%zu)\n", irq, get_cntvct_el0());
			handle_irq(ctx);
			break;
		case TRAP_FIQ_EL1:
			panic("fiq el1\n");
			break;
		case TRAP_SERROR_EL1:
			panic("serror el1\n");
			break;
		case TRAP_EXCEPTION_EL0:
			//printf("exception el0\n");
			switch ((ctx->esr >> 26) & 0x3F)
			{
				case 0xE:
					handle_illegal_instruction_el0(ctx);
					break;
				case 0x15:
					handle_svc_el0(ctx);
					break;
				case 0x20:
					handle_instruction_abort_el0(ctx);
					break;
				case 0x24:
					handle_data_abort_el0(ctx);
					break;
				case 0x32:
					handle_ss_el0(ctx);
					break;
				default:
					panic("unhandled exception 0x%lx\n", (ctx->esr >> 26) & 0x3F);
			}
			break;
		case TRAP_IRQ_EL0:
			//printf("irq el0 %zu (%zu)\n", irq, get_cntvct_el0());
			handle_irq(ctx);
			break;
		case TRAP_FIQ_EL0:
			panic("fiq el0\n");
			break;
		case TRAP_SERROR_EL0:
			panic("serror el0\n");
			break;
		default:
			panic("unknown interrupt %zd\n", id);
			break;
	}
}

int arch_register_native_irq(size_t irq, irq_fn_t fn, void *userdata,
                             struct irq_handle *handle)
{
	register_irq(handle, IRQ_NATIVE, irq, 0, fn, userdata);
	handle->native.line = irq;
	gicv2_enable_interrupt(irq);
	return 0;
}

void arch_disable_native_irq(struct irq_handle *handle)
{
	gicv2_disable_interrupt(handle->native.line);
}

int arch_get_msi_irq(size_t *cpuid, size_t *irq)
{
	size_t min = gicv2_get_msi_min_irq();
	size_t max = gicv2_get_msi_max_irq();
	for (size_t i = min; i < max; ++i)
	{
		if (!TAILQ_EMPTY(&g_cpus[0].irq_handles[i]))
			continue;
		*cpuid = 0; /* XXX */
		*irq = i;
		return 0;
	}
	return -ENOENT;
}

void arch_register_msi_irq(struct irq_handle *handle)
{
	gicv2_set_irq_cpu(handle->id, handle->cpuid);
	gicv2_set_edge_trigger(handle->id);
	gicv2_enable_interrupt(handle->id);
	gicv2_clear_interrupt(handle->id);
}

__attribute__ ((noreturn))
void aarch64_trap_handle(uint64_t type, uint64_t esr)
{
	struct irq_ctx ctx;
	ctx.trapframe = curcpu()->trapframe;
	ctx.esr = esr;
	trap_handle(type, &ctx);
}

void handle_ipi(void *userdata)
{
	(void)userdata;
	if (__atomic_exchange_n(&curcpu()->must_resched, 0, __ATOMIC_SEQ_CST))
		sched_resched();
}

static int register_sgi(struct irq_handle *handle, size_t irq, size_t cpuid,
                        irq_fn_t fn, void *userdata)
{
	register_irq(handle, IRQ_NATIVE, irq, cpuid, fn, userdata);
	handle->native.line = irq;
	gicv2_set_edge_trigger(irq);
	gicv2_enable_interrupt(irq);
	gicv2_clear_interrupt(irq);
	return 0;
}

void setup_interrupt_handlers(void)
{
	struct cpu *cpu = curcpu();
	register_sgi(&cpu->arch.ipi_handle, IRQ_ID_IPI, cpu->arch.gicc_id, handle_ipi, NULL);
}
