#include "arch/x86/apic.h"
#include "arch/x86/x86.h"
#include "arch/x86/msr.h"
#include "arch/x86/asm.h"
#include "arch/x86/cr.h"

#include <proc.h>
#include <std.h>
#include <cpu.h>
#include <mem.h>

extern uint8_t _kernel_begin;
extern uint8_t _kernel_end;

__attribute__((noreturn))
void context_switch(const struct trapframe *trapframe);
__attribute__((noreturn))
void syscall_leave(const struct trapframe *trapframe);
void syscall_enter(void);

void arch_print_regs(const struct user_regs *regs)
{
	printf("RAX: %016" PRIx64 " RBX: %016" PRIx64 "\n"
	       "RCX: %016" PRIx64 " RDX: %016" PRIx64 "\n"
	       "RSI: %016" PRIx64 " RDI: %016" PRIx64 "\n"
	       "RSP: %016" PRIx64 " RBP: %016" PRIx64 "\n"
	       "R8 : %016" PRIx64 " R9 : %016" PRIx64 "\n"
	       "R10: %016" PRIx64 " R11: %016" PRIx64 "\n"
	       "R12: %016" PRIx64 " R13: %016" PRIx64 "\n"
	       "R14: %016" PRIx64 " R15: %016" PRIx64 "\n"
	       "RIP: %016" PRIx64 " RF : %016" PRIx64 "\n"
	       "CS: %04" PRIx64 " DS: %04" PRIx64 " ES: %04" PRIx64 " "
	       "FS: %04" PRIx64 " GS: %04" PRIx64 " SS: %04" PRIx64 "\n",
	       regs->rax, regs->rbx,
	       regs->rcx, regs->rdx,
	       regs->rsi, regs->rdi,
	       regs->rsp, regs->rbp,
	       regs->r8,  regs->r9 ,
	       regs->r10, regs->r11,
	       regs->r12, regs->r13,
	       regs->r14, regs->r15,
	       regs->rip, regs->rf,
	       regs->cs & 0xFFFF, regs->ds & 0xFFFF, regs->es & 0xFFFF,
	       regs->fs & 0xFFFF, regs->gs & 0xFFFF, regs->ss & 0xFFFF);
}

static void init_trapframe(struct thread *thread)
{
	memcpy(thread->tf_user.fpu_data, &g_default_fpu, sizeof(g_default_fpu));
	memset(&thread->tf_user.regs, 0, sizeof(thread->tf_user.regs));
	thread->tf_user.regs.rsp = (uint64_t)&thread->stack[thread->stack_size];
	thread->tf_user.regs.rbp = (uint64_t)&thread->stack[thread->stack_size];
	thread->tf_user.regs.rip = (uint64_t)thread->proc->entrypoint;
}

void arch_init_trapframe_kern(struct thread *thread)
{
	init_trapframe(thread);
	thread->tf_user.regs.cs = 0x08;
	thread->tf_user.regs.ds = 0x10;
	thread->tf_user.regs.es = 0x10;
	thread->tf_user.regs.fs = 0x10;
	thread->tf_user.regs.gs = 0x10;
	thread->tf_user.regs.ss = 0x10;
	thread->tf_user.regs.rf = EFLAGS_IF;
}

void arch_init_trapframe_user(struct thread *thread)
{
	init_trapframe(thread);
	thread->tf_user.regs.cs = 0x2B;
	thread->tf_user.regs.ds = 0x23;
	thread->tf_user.regs.es = 0x23;
	thread->tf_user.regs.fs = 0x23;
	thread->tf_user.regs.gs = 0x23;
	thread->tf_user.regs.ss = 0x23;
	thread->tf_user.regs.rf = EFLAGS_IF | EFLAGS_IOPL(3);
}

#define ARCH_GET_SET_REG(name, reg) \
void arch_set_##name(struct trapframe *tf, uintptr_t val) \
{ \
	tf->regs.reg = val; \
} \
uintptr_t arch_get_##name(struct trapframe *tf) \
{ \
	return tf->regs.reg; \
}

ARCH_GET_SET_REG(stack_pointer, rsp);
ARCH_GET_SET_REG(instruction_pointer, rip);
ARCH_GET_SET_REG(frame_pointer, rbp);
ARCH_GET_SET_REG(syscall_retval, rax);
ARCH_GET_SET_REG(argument0, rdi);
ARCH_GET_SET_REG(argument1, rsi);
ARCH_GET_SET_REG(argument2, rdx);
ARCH_GET_SET_REG(argument3, rcx);

#undef ARCH_GET_SET_REG

int arch_validate_user_trapframe(struct trapframe *tf)
{
	if ((tf->regs.cs & 0xFFFF) != 0x2B
	 || (tf->regs.ds & 0xFFFF) != 0x23
	 || (tf->regs.es & 0xFFFF) != 0x23
	 || (tf->regs.fs & 0xFFFF) != 0x23
	 || (tf->regs.gs & 0xFFFF) != 0x23
	 || (tf->regs.ss & 0xFFFF) != 0x23
	 || (tf->regs.rf & (EFLAGS_IF | EFLAGS_IOPL(3))) != (EFLAGS_IF | EFLAGS_IOPL(3)))
		return -EPERM;
	return 0;
}

void amd64_setup_syscall(void)
{
	wrmsr(MSR_EFER, rdmsr(MSR_EFER) | EFER_SCE);
	wrmsr(MSR_LSTAR, (uintptr_t)syscall_enter);
	wrmsr(MSR_STAR, (0x8ULL << 32) | (0x1BULL << 48));
	wrmsr(MSR_SF_MASK, EFLAGS_CF | EFLAGS_AF | EFLAGS_ZF | EFLAGS_SF | EFLAGS_TF | EFLAGS_IF | EFLAGS_DF | EFLAGS_OF | EFLAGS_IOPL(3));
}

void arch_set_tls_addr(uintptr_t addr)
{
	wrmsr(MSR_FS_BASE, addr);
	wrmsr(MSR_GS_BASE, (uintptr_t)curcpu()->self);
	wrmsr(MSR_KERNEL_GS_BASE, (uintptr_t)curcpu()->self);
}

void arch_set_singlestep(struct thread *thread)
{
	thread->tf_user.regs.rf |= EFLAGS_TF;
}

void arch_set_trap_stack(void *ptr)
{
	tss_set_ptr(ptr);
	curcpu()->arch.syscall_stack = (uint64_t)ptr;
}

void arch_trap_return(void)
{
	struct cpu *cpu = curcpu();
	struct thread *thread = cpu->thread;
	if (!thread->tf_nest_level && thread->from_syscall)
	{
		thread->from_syscall = 0;
		syscall_leave(cpu->trapframe);
	}
	else
	{
		context_switch(cpu->trapframe);
	}
}
