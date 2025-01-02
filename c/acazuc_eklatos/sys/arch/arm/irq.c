#include "arch/aarch64/gicv2.h"

#include <arch/csr.h>
#include <arch/asm.h>

#include <sched.h>
#include <proc.h>
#include <cpu.h>
#include <irq.h>
#include <std.h>
#include <mem.h>

#define TRAP_RST  0
#define TRAP_UND  1
#define TRAP_SVC  2
#define TRAP_PABT 3
#define TRAP_DABT 4
#define TRAP_IRQ  5
#define TRAP_FIQ  6

void handle_svc_usr(struct irq_ctx *ctx)
{
	syscall(ctx->trapframe->regs.r[7],
	        ctx->trapframe->regs.r[0],
	        ctx->trapframe->regs.r[1],
	        ctx->trapframe->regs.r[2],
	        ctx->trapframe->regs.r[3],
	        ctx->trapframe->regs.r[4],
	        ctx->trapframe->regs.r[5]);
}

void handle_instruction_abort_usr(struct irq_ctx *ctx)
{
	(void)ctx;
	struct thread *thread = curcpu()->thread;
	if (!thread)
		panic("instruction abort usr without thread\n");
	uint32_t far = get_ifar();
	uint32_t fsr = get_ifsr();
	switch (fsr & 0xF)
	{
		case 0x05: /* translation fault on section */
		case 0x07: /* translation fault on page */
			thread_fault(thread, far, VM_PROT_X);
			break;
		default:
			thread_signal(thread, SIGSEGV);
			break;
	}
}

void handle_data_abort_usr(struct irq_ctx *ctx)
{
	(void)ctx;
	struct thread *thread = curcpu()->thread;
	if (!thread)
		panic("data abort usr without thread\n");
	uint32_t far = get_dfar();
	uint32_t fsr = get_dfsr();
	switch (((fsr >> 6) & 0x10) | (fsr & 0xF))
	{
		case 0x05: /* translation fault on section */
		case 0x07: /* translation fault on page */
			thread_fault(thread, far, (fsr & (1 << 1)) ? VM_PROT_W : VM_PROT_R);
			break;
		default:
			thread_signal(thread, SIGSEGV);
			break;
	}
}

void handle_instruction_abort(struct irq_ctx *ctx)
{
	uint32_t far = get_ifar();
	uint32_t fsr = get_ifsr();
	arch_print_regs(&ctx->trapframe->regs);
	panic("instruction abort 0x%lx @ 0x%lx FSR 0x%lx PSR 0x%lx\n",
	      far, ctx->trapframe->regs.r[15], fsr, ctx->trapframe->regs.cpsr);
}

void handle_data_abort(struct irq_ctx *ctx)
{
	uint32_t far = get_dfar();
	uint32_t fsr = get_dfsr();
	arch_print_regs(&ctx->trapframe->regs);
	panic("data abort 0x%lx @ 0x%lx FSR 0x%lx PSR 0x%lx\n",
	      far, ctx->trapframe->regs.r[15], fsr, ctx->trapframe->regs.cpsr);
}

void handle_illegal_instruction_usr(struct irq_ctx *ctx)
{
	(void)ctx;
	struct thread *thread = curcpu()->thread;
	if (!thread)
		panic("illegal instruction usr without thread\n");
	thread_illegal_instruction(thread);
}

void handle_illegal_instruction(struct irq_ctx *ctx)
{
	arch_print_regs(&ctx->trapframe->regs);
	panic("illegal instruction @ 0x%lx PSR 0x%lx\n",
	      ctx->trapframe->regs.r[15], ctx->trapframe->regs.cpsr);
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
		case TRAP_RST:
			panic("reset\n");
		case TRAP_UND:
			if ((ctx->trapframe->regs.cpsr & 0x1F) == PSR_M_USR)
				handle_illegal_instruction_usr(ctx);
			else
				handle_illegal_instruction(ctx);
			break;
		case TRAP_SVC:
			if ((ctx->trapframe->regs.cpsr & 0x1F) != PSR_M_USR)
				panic("non-user svc\n");
			handle_svc_usr(ctx);
			break;
		case TRAP_PABT:
			if ((ctx->trapframe->regs.cpsr & 0x1F) == PSR_M_USR)
				handle_instruction_abort_usr(ctx);
			else
				handle_instruction_abort(ctx);
			break;
		case TRAP_DABT:
			if ((ctx->trapframe->regs.cpsr & 0x1F) == PSR_M_USR)
				handle_data_abort_usr(ctx);
			else
				handle_data_abort(ctx);
			break;
		case TRAP_IRQ:
			handle_irq(ctx);
			break;
		case TRAP_FIQ:
			panic("fiq\n");
		default:
			panic("unknown trap 0x%zx\n", id);
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
void arm_trap_handle(uint32_t type)
{
	struct irq_ctx ctx;
	ctx.trapframe = curcpu()->trapframe;
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
