#ifndef ARM_ASM_H
#define ARM_ASM_H

#include <types.h>

static inline void cpsid_aif(void)
{
	__asm__ volatile ("cpsid aif");
}

static inline void cpsie_aif(void)
{
	__asm__ volatile ("cpsie aif");
}

static inline void wfi(void)
{
	__asm__ volatile ("wfi" : : : "memory");
}

static inline void dsb(void)
{
	__asm__ volatile ("mcr p15, 0, %0, c7, c10, 4" : : "r"(0));
}

static inline void dmb(void)
{
	__asm__ volatile ("mcr p15, 0, %0, c7, c10, 5" : : "r"(0));
}

static inline void tlbivmaa(uintptr_t addr)
{
	__asm__ volatile ("mcr p15, 0, %0, c8, c7, 3" : : "r"(addr));
}

static inline void tlbiall(void)
{
	__asm__ volatile ("mcr p15, 0, %0, c8, c7, 0" : : "r"(0));
}

static inline uintptr_t get_ttbr0(void)
{
	uintptr_t val;
	__asm__ volatile ("mrc p15, 0, %0, c2, c0, 0" : "=r"(val));
	return val;
}

static inline void set_ttbr0(uintptr_t addr)
{
	__asm__ volatile ("mcr p15, 0, %0, c2, c0, 0" : : "r"(addr));
}

static inline void set_tpidrprw(uintptr_t addr)
{
	__asm__ volatile ("mcr p15, 0, %0, c13, c0, 4" : : "r"(addr));
}

static inline void set_tpidruro(uintptr_t addr)
{
	__asm__ volatile ("mcr p15, 0, %0, c13, c0, 3" : : "r"(addr));
}

static inline void yield(void)
{
	__asm__ volatile ("yield" : : : "memory");
}

static inline uint64_t get_cntvct(void)
{
	uint32_t v[2];
	__asm__ volatile ("mrrc p15, 1, %0, %1, c14" : "=r"(v[0]), "=r"(v[1]));
	return ((uint64_t)v[1] << 32) | v[0];
}

static inline uintptr_t get_cntfrq(void)
{
	uintptr_t val;
	__asm__ volatile ("mrc p15, 0, %0, c14, c0, 0" : "=r"(val));
	return val;
}

static inline void set_cntv_cval(uint64_t val)
{
	__asm__ volatile ("mcrr p15, 3, %0, %1, c14" : : "r"(val), "r"(val >> 32));
}

static inline void set_cntv_ctl(uintptr_t val)
{
	__asm__ volatile ("mcr p15, 0, %0, c14, c3, 1" : : "r"(val));
}

static inline void set_vbar(void *val)
{
	__asm__ volatile ("mcr p15, 0, %0, c12, c0, 0" : : "r"(val));
}

static inline uintptr_t get_dfsr(void)
{
	uintptr_t val;
	__asm__ volatile ("mrc p15, 0, %0, c5, c0, 0" : "=r"(val));
	return val;
}

static inline uintptr_t get_dfar(void)
{
	uintptr_t val;
	__asm__ volatile ("mrc p15, 0, %0, c6, c0, 0" : "=r"(val));
	return val;
}

static inline uintptr_t get_ifsr(void)
{
	uintptr_t val;
	__asm__ volatile ("mrc p15, 0, %0, c5, c0, 1" : "=r"(val));
	return val;
}

static inline uintptr_t get_ifar(void)
{
	uintptr_t val;
	__asm__ volatile ("mrc p15, 0, %0, c6, c0, 2" : "=r"(val));
	return val;
}

static inline uintptr_t get_fpexc(void)
{
	uintptr_t val;
	__asm__ volatile ("vmrs %0, fpexc" : "=r"(val));
	return val;
}

static inline void set_fpexc(uintptr_t val)
{
	__asm__ volatile ("vmsr fpexc, %0" : : "r"(val));
}

static inline uintptr_t get_cpacr(void)
{
	uintptr_t val;
	__asm__ volatile ("mrc p15, 0, %0, c1, c0, 2" : "=r"(val));
	return val;
}

static inline void set_cpacr(uintptr_t val)
{
	__asm__ volatile ("mcr p15, 0, %0, c1, c0, 2" : : "r"(val));
}

static inline uintptr_t get_mpidr(void)
{
	uintptr_t val;
	__asm__ volatile ("mrc p15, 0, %0, c0, c0, 5" : "=r"(val));
	return val;
}

#endif
