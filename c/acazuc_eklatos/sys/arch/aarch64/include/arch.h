#ifndef AARCH64_ARCH_H
#define AARCH64_ARCH_H

#include <types.h>
#include <irq.h>

#define PAGE_SIZE 4096
#define PAGE_MASK 0xFFF
#define MAXCPU 256

#define VADDR_USER_BEGIN 0x0000000000100000ULL
#define VADDR_USER_END   0x0000800000000000ULL
#define VADDR_KERN_BEGIN 0xFFFF000000000000ULL
#define VADDR_PMAP_BEGIN 0xFFFF800000000000ULL
#define VADDR_PMAP_END   0xFFFFC00000000000ULL
#define VADDR_HEAP_BEGIN 0xFFFFC00000000000ULL
#define VADDR_HEAP_END   0xFFFFFFFFC0000000ULL
#define VADDR_CODE_BEGIN 0xFFFFFFFFC0000000ULL

#define ARCH_STACK_ALIGNMENT      16
#define ARCH_REGISTER_PARAMETERS  8
#define ARCH_STACK_RETURN_ADDR    0
#define ARCH_FRAME_POINTER_OFFSET 0

struct user_fpu
{
	uint8_t q[32][16];
	uint64_t fpcr;
	uint64_t fpsr;
};

struct user_regs
{
	uint64_t r[32];
	uint64_t pc;
	uint64_t psr;
};

struct trapframe
{
	union
	{
		struct user_fpu fpu;
		uint8_t fpu_data[528];
	};
	struct user_regs regs;
};

struct arch_copy_zone
{
	uint64_t *dir0_ptr;
	void *ptr;
};

struct arch_cpu
{
	struct irq_handle ipi_handle;
	uintptr_t trap_stack;
	uint64_t mpidr;
	uint32_t gicc_id;
};

static inline void arch_spin_yield(void)
{
	__asm__ volatile ("yield" : : : "memory");
}

#endif
