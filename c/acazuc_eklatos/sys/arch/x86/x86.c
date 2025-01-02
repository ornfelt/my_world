#include "arch/x86/apic.h"
#include "arch/x86/x86.h"
#include "arch/x86/asm.h"
#include "arch/x86/msr.h"
#include "arch/x86/cr.h"

#include <multiboot.h>
#include <endian.h>
#include <random.h>
#include <file.h>
#include <proc.h>
#include <acpi.h>
#include <std.h>
#include <vfs.h>
#include <uio.h>
#include <cpu.h>
#include <mem.h>

uint8_t g_isa_irq[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
int g_has_apic;
extern uint8_t _kernel_end;
struct user_fpu g_default_fpu;
#if defined(__i386__)
uint32_t kern_dir_page;
#else
uint64_t kern_pml_page;
#endif

#if defined(__x86_64__)
void amd64_setup_syscall(void);
#endif

void smp_trampoline(void);

static void setup_framebuffer(void)
{
	const struct multiboot_tag *tag = multiboot_find_tag(MULTIBOOT_TAG_TYPE_FRAMEBUFFER);
	if (!tag)
		return;
	const struct multiboot_tag_framebuffer *mb_fb = (struct multiboot_tag_framebuffer*)tag;
	switch (mb_fb->common.framebuffer_type)
	{
		case MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT:
			vga_init(0,
			         mb_fb->common.framebuffer_addr,
			         mb_fb->common.framebuffer_width,
			         mb_fb->common.framebuffer_height,
			         mb_fb->common.framebuffer_pitch,
			         16);
			break;
		case MULTIBOOT_FRAMEBUFFER_TYPE_RGB:
			vga_init(1,
			         mb_fb->common.framebuffer_addr,
			         mb_fb->common.framebuffer_width,
			         mb_fb->common.framebuffer_height,
			         mb_fb->common.framebuffer_pitch,
			         mb_fb->common.framebuffer_bpp); /* XXX: use mask & fields from mb_info ? */
			break;
		default:
			panic("can't init vga\n");
			return;
	}
}

static ssize_t rdrand_collect(void *buf, size_t size, void *userdata)
{
	(void)userdata;
	size_t total = 0;
#if defined(__x86_64__)
	while (size - total >= 8)
	{
		uint64_t v;
		if (!rdrand64(&v))
			goto end;
		*(uint64_t*)&((uint8_t*)buf)[total] = v;
		total += 8;
	}
#endif
	if (size - total >= 4)
	{
		uint32_t v;
		if (!rdrand32(&v))
			goto end;
		*(uint32_t*)&((uint8_t*)buf)[total] = v;
		total += 4;
	}
	if (size - total >= 2)
	{
		uint16_t v;
		if (!rdrand16(&v))
			goto end;
		*(uint16_t*)&((uint8_t*)buf)[total] = v;
		total += 2;
	}
end:
	return total;
}

static inline uint32_t lapic_id(void)
{
	uint32_t eax, ebx, ecx, edx;
	__cpuid(1, eax, ebx, ecx, edx);
	return ebx >> 24;
}

void arch_cpu_boot(struct cpu *cpu)
{
	if (!cpu->id)
	{
		com_init();
#if defined(__i386__)
		kern_dir_page = getcr3();
#else
		kern_pml_page = getcr3();
#endif
	}
#if defined(__x86_64__)
	wrmsr(MSR_EFER, rdmsr(MSR_EFER) | EFER_NXE);
#endif
	wrmsr(IA32_PAT, 0x0000050100070406ULL);
	gdt_init(cpu->id);
	gdt_load(cpu->id);
	if (!cpu->id)
		cpu->arch.lapic_id = lapic_id();
	cpuid_load();
#if defined(__x86_64__)
	amd64_setup_syscall();
#endif
	fpu_init();
	if (!cpu->id)
		idt_init();
	idt_load();
	if (!cpu->id)
	{
		if (cpu->arch.cpuid.kvm_feat & CPUID_KVM_FEAT_PV_EOI)
		{
			uintptr_t paddr;
			if (vm_paddr(NULL, (uintptr_t)&cpu->arch.kvm_eoi, &paddr))
				panic("invalid kvm eoi paddr\n");
			wrmsr(MSR_KVM_EOI_EN, paddr | 1);
		}
		arch_save_fpu(&g_default_fpu);
		if (cpu->arch.cpuid.feat_ecx & CPUID_FEAT_ECX_RDRAND)
			random_register(rdrand_collect, NULL);
	}
	if (cpu->id)
		lapic_init_smp();
}

void arch_device_init(void)
{
	mptable_init();
	setup_framebuffer();
	pic_init(0x20, 0x28);
	if (curcpu()->arch.cpuid.feat_edx & CPUID_FEAT_EDX_APIC)
		g_has_apic = 1;
	if (g_has_apic)
		lapic_init();
	acpi_hpet_init();
	com_init_tty();
	pit_init();
	rtc_init();
	tsc_init(); /* must be done "late" to get another good clock source for tsc precision */
}

int arch_start_smp_cpu(struct cpu *cpu, size_t smp_id)
{
	static const struct timespec deassert_delay = {0, 10000000};
	static const struct timespec startup_delay = {0, 200000};
	uint8_t lapic_id;

	lapic_id = g_lapics[smp_id];
	if (lapic_id == curcpu()->arch.lapic_id)
		return -EAGAIN;
	cpu->arch.lapic_id = lapic_id;
	lapic_send_init_ipi(lapic_id);
	lapic_send_init_deassert(lapic_id);
	spinsleep(&deassert_delay);
	/* XXX send startup only if not 82489DX */
	for (size_t j = 0; j < 2; ++j)
	{
		lapic_send_startup_ipi(lapic_id, (uintptr_t)&smp_trampoline);
		spinsleep(&startup_delay);
	}
	return 0;
}

void arch_start_smp(void)
{
	if (!g_has_apic)
		return;
	cpu_start_smp(g_lapics_count);
}

int arch_register_sysfs(void)
{
	cpuinfo_register_sysfs();
	return 0;
}

void arch_cpu_ipi(struct cpu *cpu)
{
	lapic_sendipi(LAPIC_IPI_DST, cpu->arch.lapic_id, IRQ_ID_IPI);
}

void arch_disable_interrupts(void)
{
	cli();
}

void arch_enable_interrupts(void)
{
	sti();
}

void arch_wait_for_interrupt(void)
{
	hlt();
}

int arch_shutdown(void)
{
	return acpi_shutdown();
}

int arch_reboot(void)
{
	return acpi_reboot();
}

int arch_suspend(void)
{
	return acpi_suspend();
}

int arch_hibernate(void)
{
	return acpi_hibernate();
}

int arch_get_msi_addr(size_t cpuid, uint64_t *addr)
{
	*addr = 0xFEE00000 | (cpuid << 12);
	return 0;
}

int arch_get_msi_data(size_t irq, uint32_t *data)
{
	*data = irq;
	return 0;
}
