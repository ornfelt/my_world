#define ENABLE_TRACE

#include <proc.h>
#include <cpu.h>
#include <fdt.h>
#include <mem.h>

void arch_cpu_boot(struct cpu *cpu)
{
	/* XXX */
}

void arch_device_init(void)
{
	/* XXX */
}

int arch_start_smp_cpu(struct cpu *cpu, size_t smp_id)
{
	/* XXX */
}

void arch_start_smp(void)
{
	/* XXX */
}

void arch_print_user_stack_trace(struct thread *thread)
{
	/* XXX */
}

void arch_print_stack_trace(void)
{
	/* XXX */
}

void arch_print_regs(const struct user_regs *regs)
{
	/* XXX */
}

void arch_init_trapframe_kern(struct thread *thread)
{
	/* XXX */
}

void arch_init_trapframe_user(struct thread *thread)
{
	/* XXX */
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

ARCH_GET_SET_REG(stack_pointer, sp);
ARCH_GET_SET_REG(instruction_pointer, pc);
ARCH_GET_SET_REG(frame_pointer, fp);
ARCH_GET_SET_REG(syscall_retval, a0);
ARCH_GET_SET_REG(argument0, a0);
ARCH_GET_SET_REG(argument1, a1);
ARCH_GET_SET_REG(argument2, a2);
ARCH_GET_SET_REG(argument3, a3);
ARCH_GET_SET_REG(return_address, ra);

#undef ARCH_GET_SET_REG

int arch_validate_user_trapframe(struct trapframe *tf)
{
	(void)tf;
	return 0;
}

void arch_cpu_ipi(struct cpu *cpu)
{
	/* XXX */
}

void arch_disable_interrupts(void)
{
	/* XXX */
}

void arch_enable_interrupts(void)
{
	/* XXX */
}

void arch_wait_for_interrupt(void)
{
	/* XXX */
}

int arch_register_sysfs(void)
{
	/* XXX */
}

void arch_set_tls_addr(uintptr_t addr)
{
	/* XXX */
}

void arch_set_singlestep(struct thread *thread)
{
	/* XXX */
}

void arch_set_trap_stack(void *ptr)
{
	/* XXX */

}

void arch_trap_return(void)
{
	/* XXX */
}
