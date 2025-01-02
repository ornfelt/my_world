#ifndef RISCV_ARCH_H
#define RISCV_ARCH_H

#include <types.h>
#include <irq.h>

#define PAGE_SIZE 4096
#define PAGE_MASK 0xFFF
#define MAXCPU 256

#if __riscv_xlen == 64
#define VADDR_USER_BEGIN 0x0000000000100000ULL
#define VADDR_USER_END   0x0000800000000000ULL
#define VADDR_KERN_BEGIN 0xFFFF000000000000ULL
#define VADDR_PMAP_BEGIN 0xFFFF800000000000ULL
#define VADDR_PMAP_END   0xFFFFC00000000000ULL
#define VADDR_HEAP_BEGIN 0xFFFFC00000000000ULL
#define VADDR_HEAP_END   0xFFFFFFFFC0000000ULL
#define VADDR_CODE_BEGIN 0xFFFFFFFFC0000000ULL
#define ARCH_FRAME_POINTER_OFFSET -16
#else
#define VADDR_USER_BEGIN 0x00100000
#define VADDR_USER_END   0xC0000000
#define VADDR_KERN_BEGIN 0xC0000000
#define VADDR_HEAP_END   0xFF800000
#define ARCH_FRAME_POINTER_OFFSET -8
#endif

#define ARCH_STACK_ALIGNMENT     16
#define ARCH_REGISTER_PARAMETERS 8
#define ARCH_STACK_RETURN_ADDR   0

struct user_fpu
{
	uint8_t f[32][16];
	uint32_t fcsr;
};

struct user_regs
{
	uintptr_t pc;
	uintptr_t ra;
	uintptr_t sp;
	uintptr_t gp;
	uintptr_t tp;
	uintptr_t t0;
	uintptr_t t1;
	uintptr_t t2;
	uintptr_t fp;
	uintptr_t s1;
	uintptr_t a0;
	uintptr_t a1;
	uintptr_t a2;
	uintptr_t a3;
	uintptr_t a4;
	uintptr_t a5;
	uintptr_t a6;
	uintptr_t a7;
	uintptr_t s2;
	uintptr_t s3;
	uintptr_t s4;
	uintptr_t s5;
	uintptr_t s6;
	uintptr_t s7;
	uintptr_t s8;
	uintptr_t s9;
	uintptr_t s10;
	uintptr_t s11;
	uintptr_t t3;
	uintptr_t t4;
	uintptr_t t5;
	uintptr_t t6;
};

struct trapframe
{
	union
	{
		struct user_fpu fpu;
		uint8_t fpu_data[520];
	};
	struct user_regs regs;
};

struct arch_copy_zone
{
#if __riscv_xlen == 64
	uint64_t *dir0_ptr;
#else
	uint32_t *tbl_ptr;
#endif
	void *ptr;
};

struct arch_cpu
{
	uintptr_t scratch[2];
	uintptr_t trap_stack;
	uint32_t hartid;
	uint16_t plic_ctx_id;
	void *plic_enable_data;
	void *plic_control_data;
};

static inline void arch_spin_yield(void)
{
	/* zihintpause required */
	//__builtin_riscv_pause();
	__asm__ volatile ("addi x0, x0, 1");
}

#endif
