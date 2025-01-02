#ifndef MIPS_ARCH_H
#define MIPS_ARCH_H

#include <types.h>
#include <irq.h>

#define PAGE_SIZE 4096
#define PAGE_MASK 0xFFF
#define MAXCPU    256

#ifdef __mips64
#define VADDR_USER_BEGIN 0x0000000000100000ULL
#define VADDR_USER_END   0x4000000000000000ULL
#define VADDR_KERN_BEGIN 0xC000000000000000ULL /* XXX */
#define VADDR_PMAP_BEGIN 0x8000000000000000ULL
#define VADDR_PMAP_END   0xC000000000000000ULL
#define VADDR_HEAP_BEGIN 0xC000000000000000ULL
#define VADDR_HEAP_END   0xFFFFFFFFC0000000ULL
#define VADDR_CODE_BEGIN 0xFFFFFFFFC0000000ULL
#else
#define VADDR_USER_BEGIN 0x00100000
#define VADDR_USER_END   0x80000000
#define VADDR_KERN_BEGIN 0xC0000000
#define VADDR_HEAP_END   0xFF800000
#endif

#define ARCH_STACK_ALIGNMENT     16
#ifdef __mips64
#define ARCH_REGISTER_PARAMETERS 8
#else
#define ARCH_REGISTER_PARAMETERS 4
#endif
#define ARCH_STACK_RETURN_ADDR   0

struct user_fpu
{
	uint8_t fp[32][8];
	/* XXX */
};

struct user_regs
{
	uintptr_t pc;
	uintptr_t at;
	uintptr_t v0;
	uintptr_t v1;
	uintptr_t a0;
	uintptr_t a1;
	uintptr_t a2;
	uintptr_t a3;
#ifdef __mips64
	uintptr_t a4;
	uintptr_t a5;
	uintptr_t a6;
	uintptr_t a7;
#else
	uintptr_t t0;
	uintptr_t t1;
	uintptr_t t2;
	uintptr_t t3;
#endif
	uintptr_t t4;
	uintptr_t t5;
	uintptr_t t6;
	uintptr_t t7;
	uintptr_t s0;
	uintptr_t s1;
	uintptr_t s2;
	uintptr_t s3;
	uintptr_t s4;
	uintptr_t s5;
	uintptr_t s6;
	uintptr_t s7;
	uintptr_t t8;
	uintptr_t t9;
	uintptr_t k0;
	uintptr_t k1;
	uintptr_t gp;
	uintptr_t sp;
	uintptr_t fp;
	uintptr_t ra;
};

struct trapframe
{
	union
	{
		struct user_fpu fpu;
		uint8_t fpu_data[256]; /* XXX */
	};
	struct user_regs regs;
};

struct arch_copy_zone
{
#if defined(__mips64)
	uintptr_t *dir0_ptr;
#else
	uint32_t *tbl_ptr;
#endif
	void *ptr;
};

struct arch_cpu
{
};

static inline void arch_spin_yield(void)
{
	__asm__ volatile ("addi $0, $0, 0");
}

#endif
