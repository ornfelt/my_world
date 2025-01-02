#ifndef X86_CPUID_H
#define X86_CPUID_H

#include <types.h>

enum cpuid_feat_ecx
{
	CPUID_FEAT_ECX_SSE3         = (1 << 0),
	CPUID_FEAT_ECX_PCLMUL       = (1 << 1),
	CPUID_FEAT_ECX_DTES64       = (1 << 2),
	CPUID_FEAT_ECX_MONITOR      = (1 << 3),
	CPUID_FEAT_ECX_DS_CPL       = (1 << 4),
	CPUID_FEAT_ECX_VMX          = (1 << 5),
	CPUID_FEAT_ECX_SMX          = (1 << 6),
	CPUID_FEAT_ECX_EST          = (1 << 7),
	CPUID_FEAT_ECX_TM2          = (1 << 8),
	CPUID_FEAT_ECX_SSSE3        = (1 << 9),
	CPUID_FEAT_ECX_CID          = (1 << 10),
	CPUID_FEAT_ECX_SDBG         = (1 << 11),
	CPUID_FEAT_ECX_FMA          = (1 << 12),
	CPUID_FEAT_ECX_CX16         = (1 << 13),
	CPUID_FEAT_ECX_XTPR         = (1 << 14),
	CPUID_FEAT_ECX_PDCM         = (1 << 15),
	CPUID_FEAT_ECX_PCID         = (1 << 17),
	CPUID_FEAT_ECX_DCA          = (1 << 18),
	CPUID_FEAT_ECX_SSE4_1       = (1 << 19),
	CPUID_FEAT_ECX_SSE4_2       = (1 << 20),
	CPUID_FEAT_ECX_X2APIC       = (1 << 21),
	CPUID_FEAT_ECX_MOVBE        = (1 << 22),
	CPUID_FEAT_ECX_POPCNT       = (1 << 23),
	CPUID_FEAT_ECX_TSC          = (1 << 24),
	CPUID_FEAT_ECX_AES          = (1 << 25),
	CPUID_FEAT_ECX_XSAVE        = (1 << 26),
	CPUID_FEAT_ECX_OSXSAVE      = (1 << 27),
	CPUID_FEAT_ECX_AVX          = (1 << 28),
	CPUID_FEAT_ECX_F16C         = (1 << 29),
	CPUID_FEAT_ECX_RDRAND       = (1 << 30),
	CPUID_FEAT_ECX_HYPERVISOR   = (1 << 31),
};

enum cpuid_feat_edx
{
	CPUID_FEAT_EDX_FPU          = (1 << 0),
	CPUID_FEAT_EDX_VME          = (1 << 1),
	CPUID_FEAT_EDX_DE           = (1 << 2),
	CPUID_FEAT_EDX_PSE          = (1 << 3),
	CPUID_FEAT_EDX_TSC          = (1 << 4),
	CPUID_FEAT_EDX_MSR          = (1 << 5),
	CPUID_FEAT_EDX_PAE          = (1 << 6),
	CPUID_FEAT_EDX_MCE          = (1 << 7),
	CPUID_FEAT_EDX_CX8          = (1 << 8),
	CPUID_FEAT_EDX_APIC         = (1 << 9),
	CPUID_FEAT_EDX_SEP          = (1 << 11),
	CPUID_FEAT_EDX_MTRR         = (1 << 12),
	CPUID_FEAT_EDX_PGE          = (1 << 13),
	CPUID_FEAT_EDX_MCA          = (1 << 14),
	CPUID_FEAT_EDX_CMOV         = (1 << 15),
	CPUID_FEAT_EDX_PAT          = (1 << 16),
	CPUID_FEAT_EDX_PSE36        = (1 << 17),
	CPUID_FEAT_EDX_PSN          = (1 << 18),
	CPUID_FEAT_EDX_CLFLUSH      = (1 << 19),
	CPUID_FEAT_EDX_DS           = (1 << 21),
	CPUID_FEAT_EDX_ACPI         = (1 << 22),
	CPUID_FEAT_EDX_MMX          = (1 << 23),
	CPUID_FEAT_EDX_FXSR         = (1 << 24),
	CPUID_FEAT_EDX_SSE          = (1 << 25),
	CPUID_FEAT_EDX_SSE2         = (1 << 26),
	CPUID_FEAT_EDX_SS           = (1 << 27),
	CPUID_FEAT_EDX_HTT          = (1 << 28),
	CPUID_FEAT_EDX_TM           = (1 << 29),
	CPUID_FEAT_EDX_IA64         = (1 << 30),
	CPUID_FEAT_EDX_PBE          = (1 << 31),
};

enum cpuid_extf_0_ebx
{
	CPUID_EXTF_0_EBX_FSGSBASE    = (1 << 0),
	CPUID_EXTF_0_EBX_TSCADJMSR   = (1 << 1),
	CPUID_EXTF_0_EBX_SGX         = (1 << 2),
	CPUID_EXTF_0_EBX_BMI1        = (1 << 3),
	CPUID_EXTF_0_EBX_HLE         = (1 << 4),
	CPUID_EXTF_0_EBX_AVX2        = (1 << 5),
	CPUID_EXTF_0_EBX_FDP_EONLY   = (1 << 6),
	CPUID_EXTF_0_EBX_SMEP        = (1 << 7),
	CPUID_EXTF_0_EBX_BMI2        = (1 << 8),
	CPUID_EXTF_0_EBX_ERMS        = (1 << 9),
	CPUID_EXTF_0_EBX_INVPCID     = (1 << 10),
	CPUID_EXTF_0_EBX_RTM         = (1 << 11),
	CPUID_EXTF_0_EBX_RTDM_PQM    = (1 << 12),
	CPUID_EXTF_0_EBX_X86_CSDS    = (1 << 13),
	CPUID_EXTF_0_EBX_MPX         = (1 << 14),
	CPUID_EXTF_0_EBX_RDTA_PQE    = (1 << 15),
	CPUID_EXTF_0_EBX_AVX512_F    = (1 << 16),
	CPUID_EXTF_0_EBX_AVX512_DQ   = (1 << 17),
	CPUID_EXTF_0_EBX_RDSEED      = (1 << 18),
	CPUID_EXTF_0_EBX_ADX         = (1 << 19),
	CPUID_EXTF_0_EBX_SMAP        = (1 << 20),
	CPUID_EXTF_0_EBX_AVX512_IFMA = (1 << 21),
	CPUID_EXTF_0_EBX_PCOMMMIT    = (1 << 22),
	CPUID_EXTF_0_EBX_CLFLUSHOPT  = (1 << 23),
	CPUID_EXTF_0_EBX_CLWB        = (1 << 24),
	CPUID_EXTF_0_EBX_PT          = (1 << 25),
	CPUID_EXTF_0_EBX_AVX512_PF   = (1 << 26),
	CPUID_EXTF_0_EBX_AVX512_ER   = (1 << 27),
	CPUID_EXTF_0_EBX_AVX512_CD   = (1 << 28),
	CPUID_EXTF_0_EBX_SHA         = (1 << 29),
	CPUID_EXTF_0_EBX_AVX512_BW   = (1 << 30),
	CPUID_EXTF_0_EBX_AVX512_VL   = (1 << 31),
};

enum cpuid_extf_0_ecx
{
	CPUID_EXTF_0_ECX_PREFETCHWT1      = (1 << 0),
	CPUID_EXTF_0_ECX_AVX512_VBMI      = (1 << 1),
	CPUID_EXTF_0_ECX_UMIP             = (1 << 2),
	CPUID_EXTF_0_ECX_PKU              = (1 << 3),
	CPUID_EXTF_0_ECX_OSPKE            = (1 << 4),
	CPUID_EXTF_0_ECX_WAITPKG          = (1 << 5),
	CPUID_EXTF_0_ECX_AVX512_VBMI2     = (1 << 6),
	CPUID_EXTF_0_ECX_CETSS_SHSTK      = (1 << 7),
	CPUID_EXTF_0_ECX_GFNI             = (1 << 8),
	CPUID_EXTF_0_ECX_VAES             = (1 << 9),
	CPUID_EXTF_0_ECX_VPCLMULQDQ       = (1 << 10),
	CPUID_EXTF_0_ECX_AVX512_VNNI      = (1 << 11),
	CPUID_EXTF_0_ECX_AVX512_BITALG    = (1 << 12),
	CPUID_EXTF_0_ECX_TME              = (1 << 13),
	CPUID_EXTF_0_ECX_AVX512_VPOPCNTDQ = (1 << 14),
	CPUID_EXTF_0_ECX_FZM              = (1 << 15),
	CPUID_EXTF_0_ECX_LA57             = (1 << 16),
	CPUID_EXTF_0_ECX_RDPID            = (1 << 22),
	CPUID_EXTF_0_ECX_KL               = (1 << 23),
	CPUID_EXTF_0_ECX_BUS_LOCK_DETECT  = (1 << 24),
	CPUID_EXTF_0_ECX_CLDEMOTE         = (1 << 25),
	CPUID_EXTF_0_ECX_MPRR             = (1 << 26),
	CPUID_EXTF_0_ECX_MOVDIRI          = (1 << 27),
	CPUID_EXTF_0_ECX_MOVDIR64B        = (1 << 28),
	CPUID_EXTF_0_ECX_ENQCMD           = (1 << 29),
	CPUID_EXTF_0_ECX_SGX_LC           = (1 << 30),
	CPUID_EXTF_0_ECX_PKS              = (1 << 31),
};

enum cpuid_extf_0_edx
{
	CPUID_EXTF_0_EDX_SGX_TERM          = (1 << 0),
	CPUID_EXTF_0_EDX_SGX_KEYS          = (1 << 1),
	CPUID_EXTF_0_EDX_AVX512_4NVVIW     = (1 << 2),
	CPUID_EXTF_0_EDX_AVX512_4FMAPS     = (1 << 3),
	CPUID_EXTF_0_EDX_FSRM              = (1 << 4),
	CPUID_EXTF_0_EDX_UINTR             = (1 << 5),
	CPUID_EXTF_0_EDX_AVX512VP2INT      = (1 << 8),
	CPUID_EXTF_0_EDX_SRDBS_CTRL        = (1 << 9),
	CPUID_EXTF_0_EDX_MC_CLEAR          = (1 << 10),
	CPUID_EXTF_0_EDX_RTM_ALL_ABORT     = (1 << 11),
	CPUID_EXTF_0_EDX_TXS_ABORT_MSR     = (1 << 13),
	CPUID_EXTF_0_EDX_SERIALIZE         = (1 << 14),
	CPUID_EXTF_0_EDX_HYBRID            = (1 << 15),
	CPUID_EXTF_0_EDX_TSXLDTRK          = (1 << 16),
	CPUID_EXTF_0_EDX_PCONFIG           = (1 << 18),
	CPUID_EXTF_0_EDX_LBR               = (1 << 19),
	CPUID_EXTF_0_EDX_CET_IBT           = (1 << 20),
	CPUID_EXTF_0_EDX_AMX_BF16          = (1 << 22),
	CPUID_EXTF_0_EDX_AVX512_FP16       = (1 << 23),
	CPUID_EXTF_0_EDX_AMX_TILE          = (1 << 24),
	CPUID_EXTF_0_EDX_AMX_INT8          = (1 << 25),
	CPUID_EXTF_0_EDX_IBRS              = (1 << 26),
	CPUID_EXTF_0_EDX_STIBP             = (1 << 27),
	CPUID_EXTF_0_EDX_L1D_FLUSH         = (1 << 28),
	CPUID_EXTF_0_EDX_ARCH_CAPABILITIES = (1 << 29),
	CPUID_EXTF_0_EDX_CORE_CAPABILITIES = (1 << 30),
	CPUID_EXTF_0_EDX_SSBD              = (1 << 31),
};

enum cpuid_extf_1_eax
{
	CPUID_EXTF_1_EAX_SHA512         = (1 << 0),
	CPUID_EXTF_1_EAX_SM3            = (1 << 1),
	CPUID_EXTF_1_EAX_SM4            = (1 << 2),
	CPUID_EXTF_1_EAX_RAO_INT        = (1 << 3),
	CPUID_EXTF_1_EAX_AVXVNNI        = (1 << 4),
	CPUID_EXTF_1_EAX_AVX512_BF16    = (1 << 5),
	CPUID_EXTF_1_EAX_LASS           = (1 << 6),
	CPUID_EXTF_1_EAX_CMPCCXADDD     = (1 << 7),
	CPUID_EXTF_1_EAX_ARCHPERFMONEXT = (1 << 8),
	CPUID_EXTF_1_EAX_DEDUP          = (1 << 9),
	CPUID_EXTF_1_EAX_FZRM           = (1 << 10),
	CPUID_EXTF_1_EAX_FSRS           = (1 << 11),
	CPUID_EXTF_1_EAX_RSRCS          = (1 << 12),
	CPUID_EXTF_1_EAX_FRED           = (1 << 17),
	CPUID_EXTF_1_EAX_LKGS           = (1 << 18),
	CPUID_EXTF_1_EAX_WRMSRNS        = (1 << 19),
	CPUID_EXTF_1_EAX_AMXFP_16       = (1 << 21),
	CPUID_EXTF_1_EAX_HRESET         = (1 << 22),
	CPUID_EXTF_1_EAX_AVX_IFMA       = (1 << 23),
	CPUID_EXTF_1_EAX_LAM            = (1 << 26),
	CPUID_EXTF_1_EAX_MSRLIST        = (1 << 27),
};

enum cpuid_extf_1_ebx
{
	CPUID_EXTF_1_EBX_PINN_CTL = (1 << 0),
	CPUID_EXTF_1_EBX_PBNDKB   = (1 << 1),
};

enum cpuid_extf_1_edx
{
	CPUID_EXTF_1_EDX_AVX_VNNI_INT8  = (1 << 4),
	CPUID_EXTF_1_EDX_AVX_NE_CONVERT = (1 << 5),
	CPUID_EXTF_1_EDX_AMX_COMPLEX    = (1 << 8),
	CPUID_EXTF_1_EDX_AVX_VNNI_INT16 = (1 << 10),
	CPUID_EXTF_1_EDX_PREFETCHI      = (1 << 14),
	CPUID_EXTF_1_EDX_USER_MSR       = (1 << 15),
	CPUID_EXTF_1_EDX_UIRET_UIF      = (1 << 17),
	CPUID_EXTF_1_EDX_CET_SSS        = (1 << 18),
	CPUID_EXTF_1_EDX_AVX10          = (1 << 19),
	CPUID_EXTF_1_EDX_APX_F          = (1 << 21),
};

enum cpuid_extf_2_edx
{
	CPUID_EXTF_2_EDX_PFSD       = (1 << 0),
	CPUID_EXTF_2_EDX_IPRED_DIS  = (1 << 1),
	CPUID_EXTF_2_EDX_RRSBA_CTRL = (1 << 2),
	CPUID_EXTF_2_EDX_DPPD_U     = (1 << 3),
	CPUID_EXTF_2_EDX_BHI_CTRL   = (1 << 4),
	CPUID_EXTF_2_EDX_MCDT_NO    = (1 << 5),
	CPUID_EXTF_2_EDX_UC_LOCK    = (1 << 6),
};

enum cpuid_xsave_feat
{
	CPUID_XSAVE_FEAT_XSAVEOPT    = (1 << 0),
	CPUID_XSAVE_FEAT_XSAVEC      = (1 << 1),
	CPUID_XSAVE_FEAT_XGETBV_ECX1 = (1 << 2),
	CPUID_XSAVE_FEAT_XSS         = (1 << 3),
	CPUID_XSAVE_FEAT_XFD         = (1 << 4),
};

enum cpuid_kvm_feat
{
	CPUID_KVM_FEAT_CLOCKSOURCE       = (1 << 0),
	CPUID_KVM_FEAT_NOP_IO_DELAY      = (1 << 1),
	CPUID_KVM_FEAT_MMU_OP            = (1 << 2),
	CPUID_KVM_FEAT_CLOCKSOURCE2      = (1 << 3),
	CPUID_KVM_FEAT_ASYNC_PF          = (1 << 4),
	CPUID_KVM_FEAT_STEAL_TIME        = (1 << 5),
	CPUID_KVM_FEAT_PV_EOI            = (1 << 6),
	CPUID_KVM_FEAT_PV_UNHALT         = (1 << 7),
	CPUID_KVM_FEAT_PV_TLB_FLUSH      = (1 << 8),
	CPUID_KVM_FEAT_ASYNC_PF_VMEXIT   = (1 << 9),
	CPUID_KVM_FEAT_PV_SEND_IPI       = (1 << 10),
	CPUID_KVM_FEAT_POLL_CONTROL      = (1 << 11),
	CPUID_KVM_FEAT_PV_SCHED_YIELD    = (1 << 12),
	CPUID_KVM_FEAT_ASYNC_PF_INT      = (1 << 13),
	CPUID_KVM_FEAT_MSI_EXT_DST_ID    = (1 << 14),
	CPUID_KVM_FEAT_HC_MAP_GPA_RANGE  = (1 << 15),
	CPUID_KVM_FEAT_MIGRATION_CONTROL = (1 << 16),
};

struct cpuid
{
	uint32_t max_cpuid;
	char name[49];
	char vendor[13];
	uint32_t feat_eax;
	uint32_t feat_ebx;
	uint32_t feat_ecx;
	uint32_t feat_edx;
	uint32_t extf_0_ebx;
	uint32_t extf_0_ecx;
	uint32_t extf_0_edx;
	uint32_t extf_1_eax;
	uint32_t extf_1_ebx;
	uint32_t extf_1_edx;
	uint32_t extf_2_edx;
	uint32_t cache_eax;
	uint32_t cache_ebx;
	uint32_t cache_ecx;
	uint32_t cache_edx;
	uint32_t th_pm_eax;
	uint32_t th_pm_ebx;
	uint32_t th_pm_ecx;
	uint32_t th_pm_edx;
	uint32_t xsave_feat;
	uint32_t kvm_base;
	uint32_t kvm_feat;
};

int cpuinfo_register_sysfs(void);

#endif
