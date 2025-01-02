#ifndef AMD64_ARCH_H
#define AMD64_ARCH_H

#include "arch/x86/cpuid.h"
#include "arch/x86/irq.h"
#include "arch/x86/x86.h"

#include <types.h>

#define VADDR_USER_BEGIN 0x0000000000100000ULL
#define VADDR_USER_END   0x0000800000000000ULL
#define VADDR_KERN_BEGIN 0xFFFF000000000000ULL
#define VADDR_PMAP_BEGIN 0xFFFF800000000000ULL
#define VADDR_PMAP_END   0xFFFFC00000000000ULL
#define VADDR_HEAP_BEGIN 0xFFFFC00000000000ULL
#define VADDR_HEAP_END   0xFFFFFFFF80000000ULL
#define VADDR_CODE_BEGIN 0xFFFFFFFF80000000ULL

#define ARCH_STACK_ALIGNMENT      16
#define ARCH_REGISTER_PARAMETERS  6
#define ARCH_STACK_RETURN_ADDR    1
#define ARCH_FRAME_POINTER_OFFSET 0

struct tss_entry
{
	uint32_t reserved_00;
	uint64_t rsp0;
	uint64_t rsp1;
	uint64_t rsp2;
	uint64_t reserved_1C;
	uint64_t ist1;
	uint64_t ist2;
	uint64_t ist3;
	uint64_t ist4;
	uint64_t ist5;
	uint64_t ist6;
	uint64_t ist7;
	uint64_t reserved_5C;
	uint16_t reserved_64;
	uint16_t iopb;
} __attribute__((packed));

struct user_regs
{
	uint64_t rax;
	uint64_t rbx;
	uint64_t rcx;
	uint64_t rdx;
	uint64_t rsi;
	uint64_t rdi;
	uint64_t rsp;
	uint64_t rbp;
	uint64_t r8;
	uint64_t r9;
	uint64_t r10;
	uint64_t r11;
	uint64_t r12;
	uint64_t r13;
	uint64_t r14;
	uint64_t r15;
	uint64_t rip;
	uint64_t cs;
	uint64_t ds;
	uint64_t es;
	uint64_t fs;
	uint64_t gs;
	uint64_t ss;
	uint64_t rf;
};

struct arch_copy_zone
{
	uint64_t *dir0_ptr;
	void *ptr;
};

struct arch_cpu
{
	uint64_t syscall_scratch;
	uint64_t syscall_stack;
	uint8_t gdt[8 * 9];
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
