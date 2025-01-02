#include "arch/riscv/plic.h"

#include <arch/csr.h>
#include <arch/asm.h>

#include <sched.h>
#include <proc.h>
#include <irq.h>
#include <cpu.h>
#include <std.h>
#include <mem.h>

void timer_interrupt(const struct irq_ctx *ctx, void *userdata);

void handle_timer(struct irq_ctx *ctx)
{
	timer_interrupt(ctx, NULL);
	cpu_tick();
}

void handle_load_page_fault(struct irq_ctx *ctx)
{
	uintptr_t status = csrr(CSR_SSTATUS);
	uintptr_t stval = csrr(CSR_STVAL);
	if (status & CSR_SSTATUS_SPP)
	{
		arch_print_regs(&ctx->trapframe->regs);
		panic("load page fault 0x%lx @ 0x%lx\n",
		      stval, ctx->trapframe->regs.pc);
	}
	struct thread *thread = curcpu()->thread;
	if (!thread)
		panic("load page fault without thread\n");
	thread_fault(thread, stval, VM_PROT_R);
}

void handle_store_page_fault(struct irq_ctx *ctx)
{
	uintptr_t status = csrr(CSR_SSTATUS);
	uintptr_t stval = csrr(CSR_STVAL);
	if (status & CSR_SSTATUS_SPP)
	{
		arch_print_regs(&ctx->trapframe->regs);
		panic("store page fault 0x%lx @ 0x%lx\n",
		      stval, ctx->trapframe->regs.pc);
	}
	struct thread *thread = curcpu()->thread;
	if (!thread)
		panic("store page fault without thread\n");
	thread_fault(thread, stval, VM_PROT_W);
}

void handle_instruction_page_fault(struct irq_ctx *ctx)
{
	uintptr_t status = csrr(CSR_SSTATUS);
	uintptr_t stval = csrr(CSR_STVAL);
	if (status & CSR_SSTATUS_SPP)
	{
		arch_print_regs(&ctx->trapframe->regs);
		panic("instruction page fault 0x%lx @ 0x%lx\n",
		      stval, ctx->trapframe->regs.pc);
	}
	struct thread *thread = curcpu()->thread;
	if (!thread)
		panic("instruction page fault without thread\n");
	thread_fault(thread, stval, VM_PROT_X);
}

void handle_syscall(struct irq_ctx *ctx)
{
	/* riscv ecall makes sepc point to the ecall instruction
	 * make it arch-independent by making PC point to the next instruction
	 */
	ctx->trapframe->regs.pc += 4;
	syscall(ctx->trapframe->regs.a7,
	        ctx->trapframe->regs.a0,
	        ctx->trapframe->regs.a1,
	        ctx->trapframe->regs.a2,
	        ctx->trapframe->regs.a3,
	        ctx->trapframe->regs.a4,
	        ctx->trapframe->regs.a5);
}

void handle_irq(struct irq_ctx *ctx)
{
	(void)ctx;
	size_t irq;
	int ret = plic_get_active_interrupt(curcpu(), &irq);
	if (ret)
		panic("no interrupt active\n");
	plic_eoi(curcpu(), irq);
	irq_execute(irq);
}

void handle_ipi(struct irq_ctx *ctx)
{
	(void)ctx;
	csrc(CSR_SIP, CSR_SIP_SSIP); /* EOI */
	if (__atomic_exchange_n(&curcpu()->must_resched, 0, __ATOMIC_SEQ_CST))
		sched_resched();
	cpu_tick();
}

void handle_illegal_instruction(struct irq_ctx *ctx)
{
	uintptr_t status = csrr(CSR_SSTATUS);
	uintptr_t stval = csrr(CSR_STVAL);
	if (status & CSR_SSTATUS_SPP)
	{
		arch_print_regs(&ctx->trapframe->regs);
		panic("illegal instruction 0x%lx @ 0x%lx\n",
		      stval, ctx->trapframe->regs.pc);
	}
	struct thread *thread = curcpu()->thread;
	if (!thread)
		panic("illegal instruction without thread\n");
	thread_illegal_instruction(thread);
}

void arch_trap_handle(size_t cause, struct irq_ctx *ctx)
{
	if (cause & ((uintptr_t)1 << (sizeof(uintptr_t) * 8 - 1)))
	{
		switch (cause & ~((uintptr_t)1 << (sizeof(uintptr_t) * 8 - 1)))
		{
			case 0x1:
				handle_ipi(ctx);
				break;
			case 0x5:
				handle_timer(ctx);
				break;
			case 0x9:
				handle_irq(ctx);
				break;
			default:
				panic("unhandled trap 0x%zx\n", cause);
				break;
		}
	}
	else
	{
		switch (cause)
		{
			case 0x2:
				handle_illegal_instruction(ctx);
				break;
			case 0x8:
				handle_syscall(ctx);
				break;
			case 0xC:
				handle_instruction_page_fault(ctx);
				break;
			case 0xD:
				handle_load_page_fault(ctx);
				break;
			case 0xF:
				handle_store_page_fault(ctx);
				break;
			default:
				panic("unhandled trap 0x%zx\n", cause);
				break;
		}
	}
}

int arch_register_native_irq(size_t irq, irq_fn_t fn, void *userdata,
                             struct irq_handle *handle)
{
	register_irq(handle, IRQ_NATIVE, irq, 0, fn, userdata);
	handle->native.line = irq;
	plic_enable_interrupt(irq);
	plic_enable_cpu_int(&g_cpus[0], irq);
	return 0;
}

void arch_disable_native_irq(struct irq_handle *handle)
{
	plic_disable_interrupt(handle->native.line);
}

int arch_get_msi_irq(size_t *cpuid, size_t *irq)
{
	for (size_t i = 0; i < IRQ_COUNT; ++i)
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
	plic_enable_interrupt(handle->id);
	plic_enable_cpu_int(&g_cpus[handle->cpuid], handle->id);
}

__attribute__ ((noreturn))
void riscv_trap_handle(uintptr_t cause)
{
	struct irq_ctx ctx;
	ctx.trapframe = curcpu()->trapframe;
	trap_handle(cause, &ctx);
}
