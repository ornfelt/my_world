#include "arch/aarch64/gicv2.h"
#include "arch/aarch64/psci.h"

#include <arch/csr.h>
#include <arch/asm.h>

#include <endian.h>
#include <proc.h>
#include <acpi.h>
#include <cpu.h>
#include <mem.h>

__attribute__((noreturn))
void context_switch(const struct trapframe *trapframe, uintptr_t el1_sp);
void pl031_init_addr(uintptr_t addr);
void pl011_init_acpi(struct acpi_obj *device);
void timer_init(void);
void setup_interrupt_handlers(void);

uintptr_t kern_paddr;
uintptr_t kern_ttbr1_page;
extern uintptr_t trap_vector;

static inline void early_printf_uart(const char *s, size_t n)
{
	for (size_t i = 0; i < n; ++i)
	{
		if (s[i] == '\n')
			*(char*)0x09000000 = '\r';
		*(char*)0x09000000 = s[i];
	}
}

static void aarch64_device_probe(struct acpi_obj *device, void *userdata)
{
	(void)userdata;
	struct acpi_obj *hid = aml_get_child(&device->device.ns, "_HID");
	if (!hid)
		return;
	if (hid->type != ACPI_OBJ_NAME)
	{
		TRACE("_HID isn't a name");
		return;
	}
	if (!hid->namedef.data)
	{
		TRACE("_HID has no data");
		return;
	}
	struct acpi_data *hid_data = hid->namedef.data;
	if (hid_data->type != ACPI_DATA_STRING)
	{
		TRACE("_HID isn't pointing to string");
		return;
	}
	if (!hid_data->string.data)
	{
		TRACE("_HID has empty string");
		return;
	}
	if (!strcmp(hid_data->string.data, "ARMH0011"))
		pl011_init_acpi(device);
}

void arch_cpu_boot(struct cpu *cpu)
{
	if (!cpu->id)
	{
		g_early_printf = early_printf_uart;
		kern_ttbr1_page = get_ttbr1_el1();
	}
	set_cpacr_el1(get_cpacr_el1() | (3 << 20)); /* fpen */
	set_tpidr_el1(cpu);
	set_vbar(&trap_vector);
	if (cpu->id)
	{
		gicv2_enable_gicc();
		setup_interrupt_handlers();
	}
}

void arch_device_init(void)
{
	acpi_probe_devices(aarch64_device_probe, NULL);
	pl031_init_addr(0x9010000);
	gicv2_enable_gicc();
	gicv2_enable_gicd();
	setup_interrupt_handlers();
	timer_init();
}

void arch_print_regs(const struct user_regs *regs)
{
	printf("X00: %016" PRIx64 " X01: %016" PRIx64 "\n"
	       "X02: %016" PRIx64 " X03: %016" PRIx64 "\n"
	       "X04: %016" PRIx64 " X05: %016" PRIx64 "\n"
	       "X06: %016" PRIx64 " X07: %016" PRIx64 "\n"
	       "X08: %016" PRIx64 " X09: %016" PRIx64 "\n"
	       "X10: %016" PRIx64 " X11: %016" PRIx64 "\n"
	       "X12: %016" PRIx64 " X13: %016" PRIx64 "\n"
	       "X14: %016" PRIx64 " X15: %016" PRIx64 "\n"
	       "X16: %016" PRIx64 " X17: %016" PRIx64 "\n"
	       "X18: %016" PRIx64 " X19: %016" PRIx64 "\n"
	       "X20: %016" PRIx64 " X21: %016" PRIx64 "\n"
	       "X22: %016" PRIx64 " X23: %016" PRIx64 "\n"
	       "X24: %016" PRIx64 " X25: %016" PRIx64 "\n"
	       "X26: %016" PRIx64 " X27: %016" PRIx64 "\n"
	       "X28: %016" PRIx64 " X29: %016" PRIx64 "\n"
	       "X30: %016" PRIx64 " X31: %016" PRIx64 "\n"
	       "PC : %016" PRIx64 " PSR: %016" PRIx64 "\n",
	       regs->r[0], regs->r[1],
	       regs->r[2], regs->r[3],
	       regs->r[4], regs->r[5],
	       regs->r[6], regs->r[7],
	       regs->r[8], regs->r[9],
	       regs->r[10], regs->r[11],
	       regs->r[12], regs->r[13],
	       regs->r[14], regs->r[15],
	       regs->r[16], regs->r[17],
	       regs->r[18], regs->r[19],
	       regs->r[20], regs->r[21],
	       regs->r[22], regs->r[23],
	       regs->r[24], regs->r[25],
	       regs->r[26], regs->r[27],
	       regs->r[28], regs->r[29],
	       regs->r[30], regs->r[31],
	       regs->pc, regs->psr);
}

static void init_trapframe(struct thread *thread)
{
	memset(thread->tf_user.fpu_data, 0, sizeof(thread->tf_user.fpu_data));
	memset(&thread->tf_user.regs, 0, sizeof(thread->tf_user.regs));
	thread->tf_user.regs.r[29] = (uint64_t)&thread->stack[thread->stack_size];
	thread->tf_user.regs.r[31] = (uint64_t)&thread->stack[thread->stack_size];
	thread->tf_user.regs.pc = (uint64_t)thread->proc->entrypoint;
}

void arch_init_trapframe_kern(struct thread *thread)
{
	init_trapframe(thread);
	thread->tf_user.regs.psr = PSR_M_EL1H;
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

ARCH_GET_SET_REG(stack_pointer, r[31]);
ARCH_GET_SET_REG(instruction_pointer, pc);
ARCH_GET_SET_REG(frame_pointer, r[29]);
ARCH_GET_SET_REG(syscall_retval, r[0]);
ARCH_GET_SET_REG(argument0, r[0]);
ARCH_GET_SET_REG(argument1, r[1]);
ARCH_GET_SET_REG(argument2, r[2]);
ARCH_GET_SET_REG(argument3, r[3]);
ARCH_GET_SET_REG(return_address, r[30]);

#undef ARCH_GET_SET_REG

int arch_validate_user_trapframe(struct trapframe *tf)
{
	if (tf->regs.psr & 0xF) /* EL0 */
		return -EPERM;
	if (tf->regs.psr & (PSR_D | PSR_A | PSR_I | PSR_F))
		return -EPERM;
	return 0;
}

void arch_cpu_ipi(struct cpu *cpu)
{
	gicv2_sgi(cpu, IRQ_ID_IPI);
}

void arch_disable_interrupts(void)
{
	set_daif(get_daif() | (PSR_D | PSR_A | PSR_I | PSR_F));
}

void arch_enable_interrupts(void)
{
	set_daif(get_daif() & ~(PSR_D | PSR_A | PSR_I | PSR_F));
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
	set_tpidr_el0(addr);
}

void arch_set_singlestep(struct thread *thread)
{
	thread->tf_user.regs.psr |= PSR_SS;
	set_mdscr_el1(get_mdscr_el1() | (1 << 0));
}

void arch_set_trap_stack(void *ptr)
{
	curcpu()->arch.trap_stack = (uintptr_t)ptr;
}

void arch_trap_return(void)
{
	context_switch(curcpu()->trapframe, curcpu()->arch.trap_stack);
}

int arch_shutdown(void)
{
	return psci_shutdown();
}

int arch_reboot(void)
{
	return psci_reboot();
}

int arch_suspend(void)
{
	return psci_suspend();
}

int arch_hibernate(void)
{
	return -EOPNOTSUPP;
}

int arch_get_msi_addr(size_t cpuid, uint64_t *addr)
{
	(void)cpuid;
	*addr = gicv2_get_msi_addr();
	return 0;
}

int arch_get_msi_data(size_t irq, uint32_t *data)
{
	*data = gicv2_get_msi_data(irq);
	return 0;
}
