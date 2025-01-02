#ifndef ARM_ARCH_H
#define ARM_ARCH_H

#include <types.h>
#include <irq.h>

#define PAGE_SIZE 4096
#define PAGE_MASK 0xFFF
#define MAXCPU 256

#define VADDR_USER_BEGIN 0x00100000
#define VADDR_USER_END   0xC0000000
#define VADDR_KERN_BEGIN 0xC0000000
#define VADDR_CODE_BEGIN 0xC0000000
#define VADDR_HEAP_END   0xFFB00000

#define ARCH_STACK_ALIGNMENT       16
#define ARCH_REGISTER_PARAMETERS   4
#define ARCH_STACK_RETURN_ADDR     0
#define ARCH_FRAME_POINTER_OFFSET -4

struct user_fpu
{
	uint64_t d[32];
	uint32_t fpscr;
};

struct user_regs
{
	uint32_t r[16];
	uint32_t cpsr;
};

struct trapframe
{
	union
	{
		struct user_fpu fpu;
		uint8_t fpu_data[260];
	};
	struct user_regs regs;
};

struct arch_copy_zone
{
	uint32_t *l2t_ptr;
	void *ptr;
};

struct arch_cpu
{
	struct irq_handle ipi_handle;
	uintptr_t trap_stack;
	uint32_t mpidr;
	uint32_t gicc_id;
	uint8_t tmp_trap_stack[64];
};

static inline void arch_spin_yield(void)
{
	__asm__ volatile ("yield" : : : "memory");
}

#endif
