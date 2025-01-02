#include "arch/x86/apic.h"
#include "arch/x86/x86.h"
#include "arch/x86/cr.h"

#include <proc.h>
#include <cpu.h>
#include <std.h>
#include <mem.h>

__attribute__((noreturn))
void context_switch(const struct trapframe *trapframe);
void gdt_set_user_gs_base(uint32_t base);

void arch_print_regs(const struct user_regs *regs)
{
	printf("EAX: %08" PRIx32 " EBX: %08" PRIx32 " "
	       "ECX: %08" PRIx32 " EDX: %08" PRIx32 "\n"
	       "ESI: %08" PRIx32 " EDI: %08" PRIx32 " "
	       "ESP: %08" PRIx32 " EBP: %08" PRIx32 "\n"
	       "EIP: %08" PRIx32 " EF : %08" PRIx32 "\n"
	       "CS: %04" PRIx32 " DS: %04" PRIx32 " ES: %04" PRIx32 " "
	       "FS: %04" PRIx32 " GS: %04" PRIx32 " SS: %04" PRIx32 "\n",
	       regs->esi, regs->edi, regs->esp, regs->ebp,
	       regs->eax, regs->ebx, regs->ecx, regs->edx,
	       regs->eip, regs->ef,
	       regs->cs & 0xFFFF, regs->ds & 0xFFFF, regs->es & 0xFFFF,
	       regs->fs & 0xFFFF, regs->gs & 0xFFFF, regs->ss & 0xFFFF);
}

static void init_trapframe(struct thread *thread)
{
	memcpy(thread->tf_user.fpu_data, &g_default_fpu, sizeof(g_default_fpu));
	memset(&thread->tf_user.regs, 0, sizeof(thread->tf_user.regs));
	thread->tf_user.regs.esp = (uint32_t)&thread->stack[thread->stack_size];
	thread->tf_user.regs.ebp = (uint32_t)&thread->stack[thread->stack_size];
	thread->tf_user.regs.eip = (uint32_t)thread->proc->entrypoint;
}

void arch_init_trapframe_kern(struct thread *thread)
{
	init_trapframe(thread);
	thread->tf_user.regs.cs = 0x08;
	thread->tf_user.regs.ds = 0x10;
	thread->tf_user.regs.es = 0x10;
	thread->tf_user.regs.fs = 0x28;
	thread->tf_user.regs.gs = 0x30;
	thread->tf_user.regs.ss = 0x10;
	thread->tf_user.regs.ef = EFLAGS_IF;
}

void arch_init_trapframe_user(struct thread *thread)
{
	init_trapframe(thread);
	thread->tf_user.regs.cs = 0x1B;
	thread->tf_user.regs.ds = 0x23;
	thread->tf_user.regs.es = 0x23;
	thread->tf_user.regs.fs = 0x3B;
	thread->tf_user.regs.gs = 0x43;
	thread->tf_user.regs.ss = 0x23;
	thread->tf_user.regs.ef = EFLAGS_IF | EFLAGS_IOPL(3);
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

ARCH_GET_SET_REG(stack_pointer, esp);
ARCH_GET_SET_REG(instruction_pointer, eip);
ARCH_GET_SET_REG(frame_pointer, ebp);
ARCH_GET_SET_REG(syscall_retval, eax);

#undef ARCH_GET_SET_REG

int arch_validate_user_trapframe(struct trapframe *tf)
{
	if ((tf->regs.cs & 0xFFFF) != 0x1B
	 || (tf->regs.ds & 0xFFFF) != 0x23
	 || (tf->regs.es & 0xFFFF) != 0x23
	 || (tf->regs.fs & 0xFFFF) != 0x3B
	 || (tf->regs.gs & 0xFFFF) != 0x43
	 || (tf->regs.ss & 0xFFFF) != 0x23
	 || (tf->regs.ef & (EFLAGS_IF | EFLAGS_IOPL(3))) != (EFLAGS_IF | EFLAGS_IOPL(3)))
		return -EPERM;
	return 0;
}

void arch_set_tls_addr(uintptr_t addr)
{
	gdt_set_user_gs_base(addr);
	gdt_load(curcpu()->id);
}

void arch_set_singlestep(struct thread *thread)
{
	thread->tf_user.regs.ef |= EFLAGS_TF;
}

void arch_set_trap_stack(void *ptr)
{
	tss_set_ptr(ptr);
}

void arch_trap_return(void)
{
	context_switch(curcpu()->trapframe);
}
