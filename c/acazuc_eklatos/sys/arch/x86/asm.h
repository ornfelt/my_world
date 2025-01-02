#ifndef X86_ASM_H
#define X86_ASM_H

#include <types.h>

static inline uint8_t inb(uint16_t port)
{
	uint8_t ret;
	__asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
	return ret;
}

static inline uint16_t inw(uint16_t port)
{
	uint16_t ret;
	__asm__ volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
	return ret;
}

static inline uint32_t inl(uint16_t port)
{
	uint32_t ret;
	__asm__ volatile ("inl %1, %0" : "=a"(ret) : "Nd"(port));
	return ret;
}

static inline void outb(uint16_t port, uint8_t val)
{
	__asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline void outw(uint16_t port, uint16_t val)
{
	__asm__ volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}

static inline void outl(uint16_t port, uint32_t val)
{
	__asm__ volatile ("outl %0, %1" : : "a"(val), "Nd"(port));
}

static inline void io_wait(void)
{
	outb(0x80, 0);
}

static inline void insb(uint16_t port, uint8_t *dst, uint32_t n)
{
	__asm__ volatile ("rep insb" : "+D"(dst), "+c"(n) : "d"(port));
}

static inline void insw(uint16_t port, uint16_t *dst, uint32_t n)
{
	__asm__ volatile ("rep insw" : "+D"(dst), "+c"(n) : "d"(port));
}

static inline void insl(uint16_t port, uint32_t *dst, uint32_t n)
{
	__asm__ volatile ("rep insl" : "+D"(dst), "+c"(n) : "d"(port));
}

static inline void outsb(uint16_t port, const uint8_t *src, uint32_t n)
{
	__asm__ volatile ("rep outsb" : "+S"(src), "+c"(n) : "d"(port));
}

static inline void outsw(uint16_t port, const uint16_t *src, uint32_t n)
{
	__asm__ volatile ("rep outsw" : "+S"(src), "+c"(n) : "d"(port));
}

static inline void outsl(uint16_t port, const uint32_t *src, uint32_t n)
{
	__asm__ volatile ("rep outsl" : "+S"(src), "+c"(n) : "d"(port));
}

static inline void cli(void)
{
	__asm__ volatile ("cli");
}

static inline void sti(void)
{
	__asm__ volatile ("sti");
}

static inline void hlt(void)
{
	__asm__ volatile ("hlt");
}

static inline uint64_t rdmsr(uint32_t msr)
{
	uint32_t lo;
	uint32_t hi;
	__asm__ volatile ("rdmsr" : "=d"(hi), "=a"(lo) : "c"(msr));
	return ((uint64_t)hi << 32) | lo;
}

static inline void wrmsr(uint32_t msr, uint64_t v)
{
	__asm__ volatile ("wrmsr" : : "c"(msr), "d"((uint32_t)(v >> 32)), "a"((uint32_t)v));
}

static inline void invlpg(uintptr_t vaddr)
{
	__asm__ volatile ("invlpg (%0)" : : "a"(vaddr) : "memory");
}

static inline uintptr_t getf(void)
{
	uintptr_t ret;
	__asm__ volatile ("pushf; pop %0" : "=a"(ret));
	return ret;
}

static inline uintptr_t getdr6(void)
{
	uintptr_t ret;
	__asm__ volatile ("mov %%dr6, %0" : "=a"(ret));
	return ret;
}

static inline void setcr4(uintptr_t v)
{
	__asm__ volatile ("mov %0, %%cr4" : : "a"(v) : );
}

static inline uintptr_t getcr4(void)
{
	uintptr_t ret;
	__asm__ volatile ("mov %%cr4, %0" : "=a"(ret));
	return ret;
}

static inline void setcr3(uintptr_t addr)
{
	__asm__ volatile ("mov %0, %%cr3" : : "a"(addr) : "memory");
}

static inline uintptr_t getcr3(void)
{
	uintptr_t ret;
	__asm__ volatile ("mov %%cr3, %0" : "=a"(ret));
	return ret;
}

static inline uintptr_t getcr2(void)
{
	uintptr_t ret;
	__asm__ volatile ("mov %%cr2, %0" : "=a"(ret));
	return ret;
}

static inline void setcr0(uintptr_t v)
{
	__asm__ volatile ("mov %0, %%cr0" : : "a"(v) : );
}

static inline uintptr_t getcr0(void)
{
	uintptr_t ret;
	__asm__ volatile ("mov %%cr0, %0" : "=a"(ret));
	return ret;
}

static inline void pause(void)
{
	__asm__ volatile ("pause" : : : "memory");
}

static inline void lgdt(void *ptr)
{
	__asm__ volatile ("lgdt %0" : : "m"(*(char*)ptr));
}

static inline void ltr(uint16_t v)
{
	__asm__ volatile ("ltr %0" : : "a"(v));
}

static inline void lidt(void *ptr)
{
	__asm__ volatile ("lidt %0" : : "m"(*(char*)ptr));
}

static inline void finit(void)
{
	__asm__ volatile ("finit");
}

static inline void fxsave(void *ptr)
{
	__asm__ volatile ("fxsave %0" : : "m"(*(char*)ptr));
}

static inline void fxrstor(const void *ptr)
{
	__asm__ volatile ("fxrstor %0" : : "m"(*(char*)ptr));
}

static inline void xsave(uint64_t mask, void *ptr)
{
	__asm__ volatile ("xsave %0" : : "m"(*(char*)ptr), "a"((uint32_t)mask), "d"((uint32_t)(mask >> 32)));
}

static inline void xrstor(uint64_t mask, const void *ptr)
{
	__asm__ volatile ("xrstor %0" : : "m"(*(char*)ptr), "a"((uint32_t)mask), "d"((uint32_t)(mask >> 32)));
}

static inline void xsaveopt(uint64_t mask, void *ptr)
{
	__asm__ volatile ("xsaveopt %0" : : "m"(*(char*)ptr), "a"((uint32_t)mask), "d"((uint32_t)(mask >> 32)));
}

static inline uint64_t rdtsc(void)
{
	uint32_t edx;
	uint32_t eax;
	__asm__ volatile ("rdtsc" : "=d"(edx), "=a"(eax));
	return ((uint64_t)edx << 32) | eax;
}

static inline uint64_t xgetbv(uint32_t id)
{
	uint32_t lo;
	uint32_t hi;
	__asm__ volatile ("xgetbv" : "=d"(hi), "=a"(lo) : "c"(id));
	return ((uint64_t)hi << 32) | lo;
}

static inline void xsetbv(uint32_t id, uint64_t v)
{
	__asm__ volatile ("xsetbv" : : "c"(id), "d"((uint32_t)(v >> 32)), "a"((uint32_t)v));
}

static inline void lfence(void)
{
	__asm__ volatile ("lfence" : : : "memory");
}

static inline void sfence(void)
{
	__asm__ volatile ("sfence" : : : "memory");
}

static inline void mfence(void)
{
	__asm__ volatile ("mfence" : : : "memory");
}

static inline void wbinvd(void)
{
	__asm__ volatile ("wbinvd" : : : "memory");
}

#if defined(__x86_64__)
static inline void swapgs(void)
{
	__asm__ inline ("swapgs");
}

static inline void setgs(uint32_t gs)
{
	__asm__ volatile ("mov %0, %%gs" : : "a"(gs));
}
#endif

static inline int rdrand64(uint64_t *ptr)
{
	uint64_t v;
	uint8_t cf;
	__asm__ volatile ("rdrand %0; setc %1" : "=r"(v), "=r"(cf));
	*ptr = v;
	return cf;
}

static inline int rdrand32(uint32_t *ptr)
{
	uint32_t v;
	uint8_t cf;
	__asm__ volatile ("rdrand %0; setc %1" : "=r"(v), "=r"(cf));
	*ptr = v;
	return cf;
}

static inline int rdrand16(uint16_t *ptr)
{
	uint16_t v;
	uint8_t cf;
	__asm__ volatile ("rdrand %0; setc %1" : "=r"(v), "=r"(cf));
	*ptr = v;
	return cf;
}

#define __cpuid(n, eax, ebx, ecx, edx) __asm__ volatile ("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(n))
#define __cpuid_count(n, c, eax, ebx, ecx, edx) __asm__ volatile ("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(n), "c"(c))

#endif
