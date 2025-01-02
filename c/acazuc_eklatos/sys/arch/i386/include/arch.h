#ifndef I386_ARCH_H
#define I386_ARCH_H

#include "arch/x86/cpuid.h"
#include "arch/x86/irq.h"
#include "arch/x86/x86.h"

#include <types.h>

#define VADDR_USER_BEGIN 0x00100000
#define VADDR_USER_END   0xC0000000
#define VADDR_KERN_BEGIN 0xC0000000
#define VADDR_HEAP_END   0xFFC00000

#define ARCH_STACK_ALIGNMENT      1
#define ARCH_REGISTER_PARAMETERS  0
#define ARCH_STACK_RETURN_ADDR    1
#define ARCH_FRAME_POINTER_OFFSET 0

struct tss_entry
{
	uint32_t prev_tss;
	uint32_t esp0;
	uint32_t ss0;
	uint32_t esp1;
	uint32_t ss1;
	uint32_t esp2;
	uint32_t ss2;
	uint32_t cr3;
	uint32_t eip;
	uint32_t eflags;
	uint32_t eax;
	uint32_t ecx;
	uint32_t edx;
	uint32_t ebx;
	uint32_t esp;
	uint32_t ebp;
	uint32_t esi;
	uint32_t edi;
	uint32_t es;
	uint32_t cs;
	uint32_t ss;
	uint32_t ds;
	uint32_t fs;
	uint32_t gs;
	uint32_t ldt;
	uint16_t trap;
	uint16_t iomap_base;
} __attribute__((packed));

struct user_regs
{
	uint32_t eax;
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;
	uint32_t esi;
	uint32_t edi;
	uint32_t esp;
	uint32_t ebp;
	uint32_t eip;
	uint32_t cs;
	uint32_t ds;
	uint32_t es;
	uint32_t fs;
	uint32_t gs;
	uint32_t ss;
	uint32_t ef;
};

struct arch_copy_zone
{
	uint32_t *tbl_ptr;
	void *ptr;
};

struct arch_cpu
{
	uint8_t gdt[8 * 10];
	struct tss_entry tss;
	uint8_t lapic_id;
	struct cpuid cpuid;
	uint32_t kvm_eoi;
	struct irq_handle syscall_handle;
	struct irq_handle ipi_handle;
	struct irq_handle spurious_handle;
};

struct trapframe
{
	union
	{
		struct user_fpu fpu;
		uint8_t fpu_data[3072];
	};
	struct user_regs regs;
};

static inline void arch_spin_yield(void)
{
	__asm__ volatile ("pause" : : : "memory");
}

#endif
