#include "arch/x86/cpuid.h"
#include "arch/x86/x86.h"
#include "arch/x86/asm.h"
#include "arch/x86/cr.h"

#include <cpu.h>

static int sse_init(void)
{
	if (!(curcpu()->arch.cpuid.feat_edx & CPUID_FEAT_EDX_SSE))
		return -EINVAL;
	setcr0((getcr0() & ~CR0_EM) | CR0_MP);
	setcr4(getcr4() | CR4_OSFXSR | CR4_OSXMMEXCPT);
	return 0;
}

static int avx_init(void)
{
	if (!(curcpu()->arch.cpuid.feat_ecx & CPUID_FEAT_ECX_AVX))
		return -EINVAL;
	uint32_t eax, ebx, ecx, edx;
	__cpuid_count(0xD, 0, eax, ebx, ecx, edx);
	if ((eax & XCR0_AVX) != XCR0_AVX)
		return -EINVAL;
	xsetbv(0, xgetbv(0) | XCR0_AVX);
	return 0;
}

static int avx512_init(void)
{
	if (!(curcpu()->arch.cpuid.extf_0_ebx & CPUID_EXTF_0_EBX_AVX512_F))
		return -EINVAL;
	uint32_t eax, ebx, ecx, edx;
	__cpuid_count(0xD, 0, eax, ebx, ecx, edx);
	if ((eax & (XCR0_OPMASK | XCR0_ZMM_HI256 | XCR0_HI16_ZMM)) != (XCR0_OPMASK | XCR0_ZMM_HI256 | XCR0_HI16_ZMM))
		return -EINVAL;
	xsetbv(0, xgetbv(0) | XCR0_OPMASK | XCR0_ZMM_HI256 | XCR0_HI16_ZMM);
	return 0;
}

void fpu_init(void)
{
	finit();
	sse_init();
	if (curcpu()->arch.cpuid.feat_ecx & CPUID_FEAT_ECX_XSAVE)
	{
		setcr4(getcr4() | CR4_OSXSAVE);
		xsetbv(0, xgetbv(0) | XCR0_X87);
		if (!sse_init())
		{
			xsetbv(0, xgetbv(0) | XCR0_SSE);
			if (!avx_init())
				avx512_init();
		}
	}
	else
	{
		sse_init();
	}
}

void arch_save_fpu(void *dst)
{
	struct cpu *cpu = curcpu();
	if (cpu->arch.cpuid.xsave_feat & CPUID_XSAVE_FEAT_XSAVEOPT)
	{
		if (cpu->arch.cpuid.extf_0_ebx & CPUID_EXTF_0_EBX_AVX512_F)
			xsaveopt(0xF, dst);
		else if (cpu->arch.cpuid.feat_ecx & CPUID_FEAT_ECX_AVX)
			xsaveopt(0x7, dst);
		else if (cpu->arch.cpuid.feat_edx & CPUID_FEAT_EDX_SSE)
			xsaveopt(0x3, dst);
		else
			xsaveopt(0x1, dst);
	}
	else if (cpu->arch.cpuid.feat_ecx & CPUID_FEAT_ECX_XSAVE)
	{
		if (cpu->arch.cpuid.extf_0_ebx & CPUID_EXTF_0_EBX_AVX512_F)
			xsave(0xF, dst);
		else if (cpu->arch.cpuid.feat_ecx & CPUID_FEAT_ECX_AVX)
			xsave(0x7, dst);
		else if (cpu->arch.cpuid.feat_edx & CPUID_FEAT_EDX_SSE)
			xsave(0x3, dst);
		else
			xsave(0x1, dst);
	}
	else
	{
		fxsave(dst);
	}
}

void arch_load_fpu(const void *src)
{
	struct cpu *cpu = curcpu();
	if (cpu->arch.cpuid.feat_ecx & CPUID_FEAT_ECX_XSAVE)
	{
		if (cpu->arch.cpuid.extf_0_ebx & CPUID_EXTF_0_EBX_AVX512_F)
			xrstor(0xF, src);
		else if (cpu->arch.cpuid.feat_ecx & CPUID_FEAT_ECX_AVX)
			xrstor(0x7, src);
		else if (cpu->arch.cpuid.feat_edx & CPUID_FEAT_EDX_SSE)
			xrstor(0x3, src);
		else
			xrstor(0x1, src);
	}
	else
	{
		fxrstor(src);
	}
}
