#ifndef RISCV_ASM_H
#define RISCV_ASM_H

#include <types.h>

static inline void csrc(uint16_t csr, uintptr_t v)
{
	__asm__ volatile ("csrc %0, %1" : : "i"(csr), "r"(v));
}

static inline void csrs(uint16_t csr, uintptr_t v)
{
	__asm__ volatile ("csrs %0, %1" : : "i"(csr), "r"(v));
}

static inline uintptr_t csrr(uint16_t csr)
{
	uintptr_t val;
	__asm__ volatile ("csrr %0, %1" : "=r"(val) : "i"(csr));
	return val;
}

static inline void csrw(uint16_t csr, uintptr_t val)
{
	__asm__ volatile ("csrw %0, %1" : : "i"(csr), "r"(val));
}

static inline void wfi(void)
{
	__asm__ volatile ("wfi" : : : "memory");
}

static inline void sfence_vma(uintptr_t addr, uint16_t asid)
{
	__asm__ volatile ("sfence.vma" : : "r"(addr), "r"(asid));
}

static inline void set_tp(uintptr_t val)
{
	__asm__ volatile ("mv tp, %0" : : "r"(val));
}

#endif
