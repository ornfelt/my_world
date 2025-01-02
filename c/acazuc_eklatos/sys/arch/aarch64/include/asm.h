#ifndef AARCH64_ASM_H
#define AARCH64_ASM_H

#include <types.h>

static inline void dsb_ishst(void)
{
	__asm__ volatile ("dsb ishst" : : : "memory");
}

static inline void dsb_ish(void)
{
	__asm__ volatile ("dsb ish" : : : "memory");
}

static inline void dsb_st(void)
{
	__asm__ volatile ("dsb st" : : : "memory");
}

static inline void dsb_sy(void)
{
	__asm__ volatile ("dsb sy" : : : "memory");
}

static inline void isb_sy(void)
{
	__asm__ volatile ("isb sy" : : : "memory");
}

static inline void isb(void)
{
	__asm__ volatile ("isb" : : : "memory");
}

static inline void tlbi_vaale1is(uintptr_t addr)
{
	__asm__ volatile ("tlbi vaale1is, %0" : : "r"(addr) : "memory");
}

static inline void tlbi_vmalle1(void)
{
	__asm__ volatile ("tlbi vmalle1" : : : "memory");
}

static inline void tlbi_vmalle1is(void)
{
	__asm__ volatile ("tlbi vmalle1is" : : : "memory");
}

static inline void *get_tpidr_el1(void)
{
	void *ptr;
	__asm__ volatile ("mrs %0, tpidr_el1" : "=r"(ptr));
	return ptr;
}

static inline void set_tpidr_el1(void *val)
{
	__asm__ volatile ("msr tpidr_el1, %0" : : "r"(val));
}

static inline void set_tpidr_el0(uintptr_t val)
{
	__asm__ volatile ("msr tpidr_el0, %0" : : "r"(val));
}

static inline void set_ttbr0_el1(uintptr_t val)
{
	__asm__ volatile ("msr ttbr0_el1, %0" : : "r"(val));
}

static inline uintptr_t get_ttbr1_el1(void)
{
	uintptr_t val;
	__asm__ volatile ("mrs %0, ttbr1_el1" : "=r"(val));
	return val;
}

static inline uintptr_t get_daif(void)
{
	uintptr_t val;
	__asm__ volatile ("mrs %0, daif" : "=r"(val));
	return val;
}

static inline void set_daif(uintptr_t val)
{
	__asm__ volatile ("msr daif, %0" : : "r"(val));
}

static inline void wfe(void)
{
	__asm__ volatile ("wfe");
}

static inline void wfi(void)
{
	__asm__ volatile ("wfi");
}

static inline void set_vbar(void *val)
{
	__asm__ volatile ("msr vbar_el1, %0" : : "r"(val));
}

static inline uintptr_t get_far_el1(void)
{
	uintptr_t val;
	__asm__ volatile ("mrs %0, far_el1" : "=r"(val));
	return val;
}

static inline uintptr_t get_cntfrq_el0(void)
{
	uintptr_t val;
	__asm__ volatile ("mrs %0, cntfrq_el0" : "=r"(val));
	return val;
}

static inline void set_cntv_tval_el0(uintptr_t val)
{
	__asm__ volatile ("msr cntv_tval_el0, %0" : : "r"(val));
}

static inline void set_cntv_cval_el0(uintptr_t val)
{
	__asm__ volatile ("msr cntv_cval_el0, %0" : : "r"(val));
}

static inline void set_cntv_ctl_el0(uintptr_t val)
{
	__asm__ volatile ("msr cntv_ctl_el0, %0" : : "r"(val));
}

static inline uint64_t get_cntvct_el0(void)
{
	uintptr_t val;
	__asm__ volatile ("mrs %0, cntvct_el0" : "=r"(val));
	return val;
}

static inline uint64_t get_mdscr_el1(void)
{
	uintptr_t val;
	__asm__ volatile ("mrs %0, mdscr_el1" : "=r"(val));
	return val;
}

static inline void set_mdscr_el1(uintptr_t val)
{
	__asm__ volatile ("msr mdscr_el1, %0" : : "r"(val));
}

static inline uint64_t get_id_aa64pfr0_el1(void)
{
	uintptr_t val;
	__asm__ volatile ("mrs %0, id_aa64pfr0_el1" : "=r"(val));
	return val;
}

static inline uint64_t get_id_aa64pfr1_el1(void)
{
	uintptr_t val;
	__asm__ volatile ("mrs %0, id_aa64pfr1_el1" : "=r"(val));
	return val;
}

#if 0
static inline uint64_t get_id_aa64pfr2_el1(void)
{
	uintptr_t val;
	__asm__ volatile ("mrs %0, id_aa64pfr2_el1" : "=r"(val));
	return val;
}
#endif

static inline uint64_t get_id_aa64isar0_el1(void)
{
	uintptr_t val;
	__asm__ volatile ("mrs %0, id_aa64isar0_el1" : "=r"(val));
	return val;
}

static inline uint64_t get_id_aa64isar1_el1(void)
{
	uintptr_t val;
	__asm__ volatile ("mrs %0, id_aa64isar1_el1" : "=r"(val));
	return val;
}

static inline uint64_t get_id_aa64isar2_el1(void)
{
	uintptr_t val;
	__asm__ volatile ("mrs %0, id_aa64isar2_el1" : "=r"(val));
	return val;
}

static inline uint64_t get_id_aa64isar3_el1(void)
{
	uintptr_t val;
	__asm__ volatile ("mrs %0, id_aa64isar3_el1" : "=r"(val));
	return val;
}

static inline uint64_t get_id_aa64mmfr0_el1(void)
{
	uintptr_t val;
	__asm__ volatile ("mrs %0, id_aa64mmfr0_el1" : "=r"(val));
	return val;
}

static inline uint64_t get_id_aa64mmfr1_el1(void)
{
	uintptr_t val;
	__asm__ volatile ("mrs %0, id_aa64mmfr1_el1" : "=r"(val));
	return val;
}

static inline uint64_t get_id_aa64mmfr2_el1(void)
{
	uintptr_t val;
	__asm__ volatile ("mrs %0, id_aa64mmfr2_el1" : "=r"(val));
	return val;
}

#if 0
static inline uint64_t get_id_aa64zfr0_el1(void)
{
	uintptr_t val;
	__asm__ volatile ("mrs %0, id_aa64zfr0_el1" : "=r"(val));
	return val;
}
#endif

#if 0
static inline uint64_t get_id_aa64smfr0_el1(void)
{
	uintptr_t val;
	__asm__ volatile ("mrs %0, id_aa64smfr0_el1" : "=r"(val));
	return val;
}
#endif

#if 0
static inline uint64_t get_id_aa64fpfr0_el1(void)
{
	uintptr_t val;
	__asm__ volatile ("mrs %0, id_aa64fpfr0_el1" : "=r"(val));
	return val;
}
#endif

static inline uint64_t get_mpidr_el1(void)
{
	uint64_t val;
	__asm__ volatile ("mrs %0, mpidr_el1" : "=r"(val));
	return val;
}

static inline uintptr_t get_cpacr_el1(void)
{
	uint64_t val;
	__asm__ volatile ("mrs %0, cpacr_el1" : "=r"(val));
	return val;
}

static inline void set_cpacr_el1(uint64_t val)
{
	__asm__ volatile ("msr cpacr_el1, %0" : : "r"(val));
}

#endif
