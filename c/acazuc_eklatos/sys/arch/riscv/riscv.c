#define ENABLE_TRACE

#include "arch/riscv/syscon.h"
#include "arch/riscv/plic.h"
#include "arch/riscv/sbi.h"

#include <arch/csr.h>
#include <arch/asm.h>

#include <endian.h>
#include <proc.h>
#include <cpu.h>
#include <fdt.h>
#include <mem.h>

__attribute__((noreturn))
void context_switch(const struct trapframe *trapframe);
int timer_init(void);
int rtc_init_fdt(struct fdt_node *node);
int uart_init_fdt(struct fdt_node *node);
int clint_init_fdt(struct fdt_node *node);
int syscon_init_poweroff_fdt(const struct fdt_node *node);
int syscon_init_reboot_fdt(const struct fdt_node *node);
void smp_trampoline(void);

uintptr_t kern_satp_page;
extern uintptr_t trap_vector;

static uint32_t cpus_harts[MAXCPU];
static size_t cpus_harts_count;

static inline void early_printf_uart(const char *s, size_t n)
{
	for (size_t i = 0; i < n; ++i)
		*(char*)0x10000000 = s[i];
}

static void probe_cpus(const struct fdt_node *node)
{
	struct fdt_node *child;
	TAILQ_FOREACH(child, &node->children, chain)
	{
		if (strncmp(child->name, "cpu@", 4))
			continue;
		const struct fdt_prop *reg = fdt_get_prop(child, "reg");
		if (!reg || reg->len != 4)
			continue;
		uint32_t hart = be32toh(*(uint32_t*)reg->data);
		if (cpus_harts_count >= MAXCPU)
			return;
		cpus_harts[cpus_harts_count++] = hart;
	}
}

static void probe_node_devices(struct fdt_node *node, int int_devices)
{
	struct fdt_node *child;
	TAILQ_FOREACH(child, &node->children, chain)
	{
		if (!fdt_check_compatible(child, "simple-bus"))
			probe_node_devices(child, int_devices);
		if (int_devices)
		{
			/* XXX RISCV_EFI_BOOT_PROTOCOL */
			if (!strcmp(child->name, "chosen"))
			{
				struct fdt_prop *prop = fdt_get_prop(child, "boot-hartid");
				if (prop)
					curcpu()->arch.hartid = be32toh(*(uint32_t*)prop->data);
			}
			if (!fdt_check_compatible(child, "riscv,clint0"))
				clint_init_fdt(child);
			if (!fdt_check_compatible(child, "riscv,plic0")
			 || !fdt_check_compatible(child, "riscv,aplic"))
				plic_init_fdt(child);
		}
		else
		{
			if (!fdt_check_compatible(child, "google,goldfish-rtc"))
				rtc_init_fdt(child);
			if (!fdt_check_compatible(child, "ns16550a"))
				uart_init_fdt(child);
			if (!fdt_check_compatible(child, "syscon-poweroff"))
				syscon_init_poweroff_fdt(child);
			if (!fdt_check_compatible(child, "syscon-reboot"))
				syscon_init_reboot_fdt(child);
			if (!strcmp(child->name, "cpus"))
				probe_cpus(child);
		}
	}
}

static void riscv_device_probe(void)
{
	struct fdt_node *node;
	TAILQ_FOREACH(node, &fdt_nodes, chain)
		probe_node_devices(node, 1);
	plic_setup_cpu(curcpu());
	TAILQ_FOREACH(node, &fdt_nodes, chain)
		probe_node_devices(node, 0);
}

void arch_cpu_boot(struct cpu *cpu)
{
	if (!cpu->id)
	{
		g_early_printf = early_printf_uart;
#if __riscv_xlen == 64
		kern_satp_page = (csrr(CSR_SATP) & 0x00000FFFFFFFFFFF) << 12;
#else
		kern_satp_page = (csrr(CSR_SATP) & 0x003FFFFF) << 12;
#endif
	}
	set_tp((uintptr_t)cpu);
	csrw(CSR_STVEC, (uintptr_t)&trap_vector);
	csrs(CSR_SIE, CSR_SIE_SSIE | CSR_SIE_STIE | CSR_SIE_SEIE);
	csrw(CSR_SSCRATCH, (uintptr_t)cpu);
	/* XXX in the best of the world, we shouldn't access user memory
	 * from kernel
	 */
	csrs(CSR_SSTATUS, CSR_SSTATUS_SUM);
	if (cpu->id)
		plic_setup_cpu(curcpu());
}

void arch_device_init(void)
{
	riscv_device_probe();
	timer_init();
}

int arch_start_smp_cpu(struct cpu *cpu, size_t smp_id)
{
	uint32_t hartid = cpus_harts[smp_id];
	if (hartid == curcpu()->arch.hartid)
		return -EAGAIN;
	cpu->arch.hartid = hartid;
	uintptr_t smp_addr = (uintptr_t)smp_trampoline;
#if __riscv_xlen == 64
	smp_addr -= 0xFFFFFFFF40000000;
#else
	smp_addr -= 0x40000000;
#endif
	int ret = sbi_hart_start(hartid, smp_addr, kern_satp_page);
	if (ret)
	{
		TRACE("failed to start hart");
		return ret;
	}
	return 0;
}

void arch_start_smp(void)
{
	uintptr_t has_ext_hsm;
	if (sbi_probe_extension(SBI_EXT_HSM, &has_ext_hsm))
	{
		TRACE("failed to probe SBI HSM extension");
		return;
	}
	if (!has_ext_hsm)
	{
		TRACE("SBI HSM extension not supported");
		return;
	}
	cpu_start_smp(cpus_harts_count);
}

void arch_print_regs(const struct user_regs *regs)
{
	printf("PC : %0*zx RA : %0*zx\n"
	       "SP : %0*zx GP : %0*zx\n"
	       "TP : %0*zx T0 : %0*zx\n"
	       "T1 : %0*zx T2 : %0*zx\n"
	       "FP : %0*zx S1 : %0*zx\n"
	       "A0 : %0*zx A1 : %0*zx\n"
	       "A2 : %0*zx A3 : %0*zx\n"
	       "A4 : %0*zx A5 : %0*zx\n"
	       "A6 : %0*zx A7 : %0*zx\n"
	       "S2 : %0*zx S3 : %0*zx\n"
	       "S4 : %0*zx S5 : %0*zx\n"
	       "S6 : %0*zx S7 : %0*zx\n"
	       "S8 : %0*zx S9 : %0*zx\n"
	       "S10: %0*zx S11: %0*zx\n"
	       "T3 : %0*zx T4 : %0*zx\n"
	       "T5 : %0*zx T6 : %0*zx\n",
	       (int)(sizeof(uintptr_t) * 2), regs->pc,
	       (int)(sizeof(uintptr_t) * 2), regs->ra,
	       (int)(sizeof(uintptr_t) * 2), regs->sp,
	       (int)(sizeof(uintptr_t) * 2), regs->gp,
	       (int)(sizeof(uintptr_t) * 2), regs->tp,
	       (int)(sizeof(uintptr_t) * 2), regs->t0,
	       (int)(sizeof(uintptr_t) * 2), regs->t1,
	       (int)(sizeof(uintptr_t) * 2), regs->t2,
	       (int)(sizeof(uintptr_t) * 2), regs->fp,
	       (int)(sizeof(uintptr_t) * 2), regs->s1,
	       (int)(sizeof(uintptr_t) * 2), regs->a0,
	       (int)(sizeof(uintptr_t) * 2), regs->a1,
	       (int)(sizeof(uintptr_t) * 2), regs->a2,
	       (int)(sizeof(uintptr_t) * 2), regs->a3,
	       (int)(sizeof(uintptr_t) * 2), regs->a4,
	       (int)(sizeof(uintptr_t) * 2), regs->a5,
	       (int)(sizeof(uintptr_t) * 2), regs->a6,
	       (int)(sizeof(uintptr_t) * 2), regs->a7,
	       (int)(sizeof(uintptr_t) * 2), regs->s2,
	       (int)(sizeof(uintptr_t) * 2), regs->s3,
	       (int)(sizeof(uintptr_t) * 2), regs->s4,
	       (int)(sizeof(uintptr_t) * 2), regs->s5,
	       (int)(sizeof(uintptr_t) * 2), regs->s6,
	       (int)(sizeof(uintptr_t) * 2), regs->s7,
	       (int)(sizeof(uintptr_t) * 2), regs->s8,
	       (int)(sizeof(uintptr_t) * 2), regs->s9,
	       (int)(sizeof(uintptr_t) * 2), regs->s10,
	       (int)(sizeof(uintptr_t) * 2), regs->s11,
	       (int)(sizeof(uintptr_t) * 2), regs->t3,
	       (int)(sizeof(uintptr_t) * 2), regs->t4,
	       (int)(sizeof(uintptr_t) * 2), regs->t5,
	       (int)(sizeof(uintptr_t) * 2), regs->t6);
}

static void init_trapframe(struct thread *thread)
{
	memset(&thread->tf_user.fpu, 0, sizeof(thread->tf_user.fpu));
	memset(&thread->tf_user.regs, 0, sizeof(thread->tf_user.regs));
	thread->tf_user.regs.sp = (uintptr_t)&thread->stack[thread->stack_size];
	thread->tf_user.regs.fp = (uintptr_t)&thread->stack[thread->stack_size];
	thread->tf_user.regs.pc = (uintptr_t)thread->proc->entrypoint;
}

void arch_init_trapframe_kern(struct thread *thread)
{
	init_trapframe(thread);
}

void arch_init_trapframe_user(struct thread *thread)
{
	init_trapframe(thread);
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
	sbi_send_ipi(1, cpu->arch.hartid);
}

void arch_disable_interrupts(void)
{
	csrc(CSR_SSTATUS, CSR_SSTATUS_SIE);
}

void arch_enable_interrupts(void)
{
	csrs(CSR_SSTATUS, CSR_SSTATUS_SIE);
}

void arch_wait_for_interrupt(void)
{
	wfi();
}

int arch_register_sysfs(void)
{
	return 0;
}

void arch_set_tls_addr(uintptr_t addr)
{
	curcpu()->thread->tf_user.regs.tp = addr;
}

void arch_set_singlestep(struct thread *thread)
{
	/* XXX */
	(void)thread;
}

void arch_set_trap_stack(void *ptr)
{
	curcpu()->arch.trap_stack = (uintptr_t)ptr;
}

void arch_trap_return(void)
{
	/* XXX if priviledged, not only idle thread */
	if (curcpu()->thread == curcpu()->idlethread
	 || curcpu()->thread->tf_nest_level)
		csrs(CSR_SSTATUS, CSR_SSTATUS_SPP);
	else
		csrc(CSR_SSTATUS, CSR_SSTATUS_SPP);
	if (curcpu()->thread->tf_nest_level)
		csrc(CSR_SSTATUS, CSR_SSTATUS_SPIE);
	else
		csrs(CSR_SSTATUS, CSR_SSTATUS_SPIE);
	context_switch(curcpu()->trapframe);
}

int arch_shutdown(void)
{
	return syscon_poweroff();
}

int arch_reboot(void)
{
	return syscon_reboot();
}

int arch_suspend(void)
{
	return -EOPNOTSUPP;
}

int arch_hibernate(void)
{
	return -EOPNOTSUPP;
}

int arch_get_msi_addr(size_t cpuid, uint64_t *addr)
{
	/* XXX */
	(void)cpuid;
	(void)addr;
	return -EINVAL;
}

int arch_get_msi_data(size_t irq, uint32_t *data)
{
	/* XXX */
	(void)irq;
	(void)data;
	return -EINVAL;
}
