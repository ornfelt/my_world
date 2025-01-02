#include "arch/x86/x86.h"
#include "arch/x86/msr.h"
#include "arch/x86/asm.h"

#include <file.h>
#include <uio.h>
#include <vfs.h>
#include <std.h>
#include <cpu.h>

static const char *cpuid_feature_ecx_str[32] =
{
	"sse3",
	"pclmul",
	"dtes64",
	"monitor",
	"ds_cpl",
	"vmx",
	"smx",
	"est",
	"tm2",
	"ssse3",
	"cid",
	"sdbg",
	"fma",
	"cx16",
	"xtpr",
	"pdcm",
	"",
	"pcid",
	"dca",
	"sse4_1",
	"sse4_2",
	"x2apic",
	"movbe",
	"popcnt",
	"tsc",
	"aes",
	"xsave",
	"osxsave",
	"avx",
	"f16c",
	"rdrand",
	"hypervisor"
};

static const char *cpuid_feature_edx_str[32] =
{
	"fpu",
	"vme",
	"de",
	"pse",
	"tsc",
	"msr",
	"pae",
	"mce",
	"cx8",
	"apic",
	"",
	"sep",
	"mtrr",
	"pge",
	"mca",
	"cmov",
	"pat",
	"pse36",
	"psn",
	"clflush",
	"",
	"ds",
	"acpi",
	"mmx",
	"fxsr",
	"sse",
	"sse2",
	"ss",
	"htt",
	"tm",
	"ia64",
	"p8e"
};

static void get_flags_str(char *flags, size_t size, struct cpuid *cpuid)
{
	int first = 1;
	for (size_t i = 0; i < 32; ++i)
	{
		if (!(cpuid->feat_edx & (1 << i)))
			continue;
		if (first)
			first = 0;
		else
			strlcat(flags, ", ", size);
		strlcat(flags, cpuid_feature_edx_str[i], size);
	}
	for (size_t i = 0; i < 32; ++i)
	{
		if (!(cpuid->feat_ecx & (1 << i)))
			continue;
		if (first)
			first = 0;
		else
			strlcat(flags, ", ", size);
		strlcat(flags, cpuid_feature_ecx_str[i], size);
	}
}

static const char *cache_entries[256] = 
{
	[0x01] = "ITLB: 4 KByte pages, 4-way set associative, 32 entries",
	[0x02] = "ITLB: 4 MByte pages, fully associative, 2 entries",
	[0x03] = "DTLB: 4 KByte pages, 4-way set associative, 64 entries",
	[0x04] = "DTLB: 4 MByte pages, 4-way set associative, 8 entries",
	[0x05] = "DTLB: 4 MByte pages, 4-way set associative, 32 entries",
	[0x06] = "L1I: 8 KBytes, 4-way set associative, 32 byte line size",
	[0x08] = "L1I: 16 KBytes, 4-way set associative, 32 byte line size",
	[0x09] = "L1I: 32 KBytes, 4-way set associative, 64 byte line size",
	[0x0A] = "L1D: 8 KBytes, 2-way set associative, 32 byte line size",
	[0x0B] = "ITLB: 4 MByte pages, 4-way set associative, 4 entries",
	[0x0C] = "L1D: 16 KBytes, 4-way set associative, 32 byte line size",
	[0x0D] = "L1D: 16 KBytes, 4-way set associative, 64 byte line size",
	[0x0E] = "L1D: 24 KBytes, 6-way set associative, 64 byte line size",
	[0x1D] = "L2: 128 KBytes, 2-way set associative, 64 byte line size",
	[0x21] = "L2: 256 KBytes, 8-way set associative, 64 byte line size",
	[0x22] = "L3: 512 KBytes, 4-way set associative, 64 byte line size, 2 lines per sector",
	[0x23] = "L3: 1 MBytes, 8-way set associative, 64 byte line size, 2 lines per sector",
	[0x24] = "L2: 1 MBytes, 16-way set associative, 64 byte line size",
	[0x25] = "L3: 2 MBytes, 8-way set associative, 64 byte line size, 2 lines per sector",
	[0x29] = "L3: 4 MBytes, 8-way set associative, 64 byte line size, 2 lines per sector",
	[0x2C] = "L1D: 32 KBytes, 8-way set associative, 64 byte line size",
	[0x30] = "L1I: 32 KBytes, 8-way set associative, 64 byte line size",
	[0x40] = "No 2nd-level cache or, if processor contains a valid 2nd-level cache, no 3rd-level cache",
	[0x41] = "L2: 128 KBytes, 4-way set associative, 32 byte line size",
	[0x42] = "L2: 256 KBytes, 4-way set associative, 32 byte line size",
	[0x43] = "L2: 512 KBytes, 4-way set associative, 32 byte line size",
	[0x44] = "L2: 1 MByte, 4-way set associative, 32 byte line size",
	[0x45] = "L2: 2 MByte, 4-way set associative, 32 byte line size",
	[0x46] = "L3: 4 MByte, 4-way set associative, 64 byte line size",
	[0x47] = "L3: 8 MByte, 8-way set associative, 64 byte line size",
	[0x48] = "L2: 3 MByte, 12-way set associative, 64 byte line size",
	[0x49] = "L3: 4 MB, 16-way set associative, 64 byte line size (Intel Xeon processor MP, Family 0FH, Model 06H); 2nd-level cache: 4 MByte, 16-way set associative, 64 byte line size",
	[0x4A] = "L3: 6 MByte, 12-way set associative, 64 byte line size",
	[0x4B] = "L3: 8 MByte, 16-way set associative, 64 byte line size",
	[0x4C] = "L3: 12 MByte, 12-way set associative, 64 byte line size",
	[0x4D] = "L3: 16 MByte, 16-way set associative, 64 byte line size",
	[0x4E] = "L2: 6 MByte, 24-way set associative, 64 byte line size",
	[0x4F] = "ITLB: 4 KByte pages, 32 entries",
	[0x50] = "ITLB: 4 KByte and 2 MByte or 4 MByte pages, 64 entries",
	[0x51] = "ITLB: 4 KByte and 2 MByte or 4 MByte pages, 128 entries",
	[0x52] = "ITLB: 4 KByte and 2 MByte or 4 MByte pages, 256 entries",
	[0x55] = "ITLB: 2 MByte or 4 MByte pages, fully associative, 7 entries",
	[0x56] = "DTLB: 4 MByte pages, 4-way set associative, 16 entries",
	[0x57] = "DTLB: 4 KByte pages, 4-way associative, 16 entries",
	[0x59] = "DTLB: 4 KByte pages, fully associative, 16 entries",
	[0x5A] = "DTLB: 2 MByte or 4 MByte pages, 4-way set associative, 32 entries",
	[0x5B] = "DTLB: 4 KByte and 4 MByte pages, 64 entries",
	[0x5C] = "DTLB: 4 KByte and 4 MByte pages, 128 entries",
	[0x5D] = "DTLB: 4 KByte and 4 MByte pages, 256 entries",
	[0x60] = "L1D: 16 KByte, 8-way set associative, 64 byte line size",
	[0x61] = "ITLB: 4 KByte pages, fully associative, 48 entries",
	[0x63] = "DTLB: 2 MByte or 4 MByte pages, 4-way set associative, 32 entries and a separate array with 1 GByte pages, 4-way set associative, 4 entries",
	[0x64] = "DTLB: 4 KByte pages, 4-way set associative, 512 entries",
	[0x66] = "L1D: 8 KByte, 4-way set associative, 64 byte line size",
	[0x67] = "L1D: 16 KByte, 4-way set associative, 64 byte line size",
	[0x68] = "L1D: 32 KByte, 4-way set associative, 64 byte line size",
	[0x6A] = "DTLB: 4 KByte pages, 8-way set associative, 64 entries",
	[0x6B] = "DTLB: 4 KByte pages, 8-way set associative, 256 entries",
	[0x6C] = "DTLB: 2M/4M pages, 8-way set associative, 128 entries",
	[0x6D] = "DTLB: 1 GByte pages, fully associative, 16 entries",
	[0x70] = "Trace cache: 12 K-μop, 8-way set associative",
	[0x71] = "Trace cache: 16 K-μop, 8-way set associative",
	[0x72] = "Trace cache: 32 K-μop, 8-way set associative",
	[0x76] = "ITLB: 2M/4M pages, fully associative, 8 entries",
	[0x78] = "L2: 1 MByte, 4-way set associative, 64 byte line size",
	[0x79] = "L2: 128 KByte, 8-way set associative, 64 byte line size, 2 lines per sector",
	[0x7A] = "L2: 256 KByte, 8-way set associative, 64 byte line size, 2 lines per sector",
	[0x7B] = "L2: 512 KByte, 8-way set associative, 64 byte line size, 2 lines per sector",
	[0x7C] = "L2: 1 MByte, 8-way set associative, 64 byte line size, 2 lines per sector",
	[0x7D] = "L2: 2 MByte, 8-way set associative, 64 byte line size",
	[0x7F] = "L2: 512 KByte, 2-way set associative, 64-byte line size",
	[0x80] = "L2: 512 KByte, 8-way set associative, 64-byte line size",
	[0x82] = "L2: 256 KByte, 8-way set associative, 32 byte line size",
	[0x83] = "L2: 512 KByte, 8-way set associative, 32 byte line size",
	[0x84] = "L2: 1 MByte, 8-way set associative, 32 byte line size",
	[0x85] = "L2: 2 MByte, 8-way set associative, 32 byte line size",
	[0x86] = "L2: 512 KByte, 4-way set associative, 64 byte line size",
	[0x87] = "L2: 1 MByte, 8-way set associative, 64 byte line size",
	[0xA0] = "DTLB: 4k pages, fully associative, 32 entries",
	[0xB0] = "ITLB: 4 KByte pages, 4-way set associative, 128 entries",
	[0xB1] = "ITLB: 2M pages, 4-way, 8 entries or 4M pages, 4-way, 4 entries",
	[0xB2] = "ITLB: 4KByte pages, 4-way set associative, 64 entries",
	[0xB3] = "DTLB: 4 KByte pages, 4-way set associative, 128 entries",
	[0xB4] = "DTLB: 4 KByte pages, 4-way associative, 256 entries",
	[0xB5] = "ITLB: 4KByte pages, 8-way set associative, 64 entries",
	[0xB6] = "ITLB: 4KByte pages, 8-way set associative, 128 entries",
	[0xBA] = "DTLB: 4 KByte pages, 4-way associative, 64 entries",
	[0xC0] = "DTLB: 4 KByte and 4 MByte pages, 4-way associative, 8 entries",
	[0xC1] = "L2TLB: 4 KByte / 2MByte pages, 8-way associative, 1024 entries",
	[0xC2] = "DTLB: 4 KByte / 2 MByte pages, 4-way associative, 16 entries",
	[0xC3] = "L2TLB: 4 KByte / 2 MByte pages, 6-way associative, 1536 entries. Also 1GBbyte pages, 4-way, 16 entries.",
	[0xC4] = "DTLB: 2M/4M Byte pages, 4-way associative, 32 entries",
	[0xCA] = "L2TLB: 4 KByte pages, 4-way associative, 512 entries",
	[0xD0] = "L3: 512 KByte, 4-way set associative, 64 byte line size",
	[0xD1] = "L3: 1 MByte, 4-way set associative, 64 byte line size",
	[0xD2] = "L3: 2 MByte, 4-way set associative, 64 byte line size",
	[0xD6] = "L3: 1 MByte, 8-way set associative, 64 byte line size",
	[0xD7] = "L3: 2 MByte, 8-way set associative, 64 byte line size",
	[0xD8] = "L3: 4 MByte, 8-way set associative, 64 byte line size",
	[0xDC] = "L3: 1.5 MByte, 12-way set associative, 64 byte line size",
	[0xDD] = "L3: 3 MByte, 12-way set associative, 64 byte line size",
	[0xDE] = "L3: 6 MByte, 12-way set associative, 64 byte line size",
	[0xE2] = "L3: 2 MByte, 16-way set associative, 64 byte line size",
	[0xE3] = "L3: 4 MByte, 16-way set associative, 64 byte line size",
	[0xE4] = "L3: 8 MByte, 16-way set associative, 64 byte line size",
	[0xEA] = "L3: 12 MByte, 24-way set associative, 64 byte line size",
	[0xEB] = "L3: 18 MByte, 24-way set associative, 64 byte line size",
	[0xEC] = "L3: 24 MByte, 24-way set associative, 64 byte line size",
	[0xF0] = "64-Byte prefetching",
	[0xF1] = "128-Byte prefetching",
	[0xFE] = "CPUID leaf 2 does not report TLB descriptor information; use CPUID leaf 18H to query TLB and other address translation parameters.",
	[0xFF] = "CPUID leaf 2 does not report cache descriptor information, use CPUID leaf 4 to query cache parameters",
};

static void get_cache_str(char *cache, size_t size, struct cpuid *cpuid)
{
	uint32_t entries[4] =
	{
		cpuid->cache_eax,
		cpuid->cache_ebx,
		cpuid->cache_ecx,
		cpuid->cache_edx,
	};
	for (size_t i = 0; i < sizeof(entries); ++i)
	{
		uint8_t v = ((uint8_t*)&entries[0])[i];
		if (!v)
			continue;
		strlcat(cache, "\n              ", size);
		if (cache_entries[v])
			strlcat(cache, cache_entries[v], size);
		else
			strlcat(cache, "unknown", size);
	}
}

static void get_cpuname(struct cpuid *cpuid)
{
	uint32_t *dst = (uint32_t*)&cpuid->name[0];
	if (cpuid->max_cpuid < 0x80000004)
	{
		*dst = '\0';
		return;
	}
	__cpuid(0x80000002, dst[0x0], dst[0x1], dst[0x2], dst[0x3]);
	__cpuid(0x80000003, dst[0x4], dst[0x5], dst[0x6], dst[0x7]);
	__cpuid(0x80000004, dst[0x8], dst[0x9], dst[0xA], dst[0xB]);
	cpuid->name[48] = '\0';
}

static void get_vendor(struct cpuid *cpuid)
{
	uint32_t eax, ebx, ecx, edx;
	__cpuid(0, eax, ebx, ecx, edx);
	cpuid->vendor[0x0] = ebx >> 0;
	cpuid->vendor[0x1] = ebx >> 8;
	cpuid->vendor[0x2] = ebx >> 16;
	cpuid->vendor[0x3] = ebx >> 24;
	cpuid->vendor[0x4] = edx >> 0;
	cpuid->vendor[0x5] = edx >> 8;
	cpuid->vendor[0x6] = edx >> 16;
	cpuid->vendor[0x7] = edx >> 24;
	cpuid->vendor[0x8] = ecx >> 0;
	cpuid->vendor[0x9] = ecx >> 8;
	cpuid->vendor[0xA] = ecx >> 16;
	cpuid->vendor[0xB] = ecx >> 24;
	cpuid->vendor[0xC] = 0;
}

void cpuid_load(void)
{
	struct cpuid *cpuid = &curcpu()->arch.cpuid;
	uint32_t eax, ebx, ecx, edx;
	__cpuid(0x80000000, eax, ebx, ecx, edx);
	cpuid->max_cpuid = eax;
	get_cpuname(cpuid);
	get_vendor(cpuid);
	__cpuid(1, eax, ebx, ecx, edx);
	cpuid->feat_eax = eax;
	cpuid->feat_ebx = ebx;
	cpuid->feat_ecx = ecx;
	cpuid->feat_edx = edx;
	__cpuid_count(7, 0, eax, ebx, ecx, edx);
	uint32_t extf_max = eax;
	cpuid->extf_0_ebx = ebx;
	cpuid->extf_0_ecx = ecx;
	cpuid->extf_0_edx = edx;
	if (extf_max > 0)
	{
		__cpuid_count(7, 1, eax, ebx, ecx, edx);
		cpuid->extf_1_eax = eax;
		cpuid->extf_1_ebx = ebx;
		cpuid->extf_1_edx = edx;
		if (extf_max > 1)
		{
			__cpuid_count(7, 2, eax, ebx, ecx, edx);
			cpuid->extf_2_edx = edx;
		}
	}
	__cpuid(2, eax, ebx, ecx, edx);
	cpuid->cache_eax = eax;
	cpuid->cache_ebx = ebx;
	cpuid->cache_ecx = ecx;
	cpuid->cache_edx = edx;
	__cpuid(6, eax, ebx, ecx, edx);
	cpuid->th_pm_eax = eax;
	cpuid->th_pm_ebx = ebx;
	cpuid->th_pm_ecx = ecx;
	cpuid->th_pm_edx = edx;
	if (cpuid->feat_ecx & CPUID_FEAT_ECX_XSAVE)
	{
		__cpuid(0xD, eax, ebx, ecx, edx);
		cpuid->xsave_feat = eax;
	}
	if (cpuid->feat_ecx & CPUID_FEAT_ECX_HYPERVISOR)
	{
		for (uint32_t i = 0x40000000; i < 0x50000000; i += 0x100)
		{
			uint32_t sig[3];
			__cpuid(i, eax, sig[0], sig[1], sig[2]);
			if (memcmp(sig, "KVMKVMKVM\0\0\0", 12))
				continue;
			cpuid->kvm_base = i;
			__cpuid(i | 1, eax, ebx, ecx, edx);
			cpuid->kvm_feat = eax;
			break;
		}
	}
}

static int print_cpuinfo(struct uio *uio)
{
	struct cpu *cpu;
	CPU_FOREACH(cpu)
	{
		struct cpuid *cpuid = &cpu->arch.cpuid;
		char flags[1024] = "";
		char cache[1024] = "";
		uint8_t stepping, model, family, type, ext_model, ext_family;
		uint8_t brand, clflush_size, addressable_ids, apic_id;
		(void)brand;
		(void)addressable_ids;
		(void)ext_family;
		(void)ext_model;
		(void)type;
		stepping =   (cpuid->feat_eax >> 0x00) & 0xF;
		model =      (cpuid->feat_eax >> 0x04) & 0xF;
		family =     (cpuid->feat_eax >> 0x08) & 0xF;
		type =       (cpuid->feat_eax >> 0x0C) & 0x3;
		ext_model =  (cpuid->feat_eax >> 0x10) & 0xF;
		ext_family = (cpuid->feat_eax >> 0x14) & 0xFF;
		brand =           (cpuid->feat_ebx >>  0) & 0xFF;
		clflush_size =    (cpuid->feat_ebx >>  8) & 0xFF;
		addressable_ids = (cpuid->feat_ebx >> 16) & 0xFF;
		apic_id =         (cpuid->feat_ebx >> 24) & 0xFF;
		get_flags_str(flags, sizeof(flags), cpuid);
		get_cache_str(cache, sizeof(cache), cpuid);
		if (cpu->id)
			uprintf(uio, "\n");
		uprintf(uio, "processor   : %" PRIu32 "\n"
		             "vendor_id   : %s\n"
		             "cpu_family  : %" PRIu8 "\n"
		             "model       : %" PRIu8 "\n"
		             "model_name  : %s\n"
		             "stepping    : %" PRIu8 "\n"
		             "apicid      : %" PRIu8 "\n"
		             "flags       : %s (%" PRIx32 ", %" PRIx32 ")\n"
		             "clflush size: %" PRIu8 "\n"
		             "cache       : %08" PRIx32 " %08" PRIx32 " %08" PRIx32 " %08" PRIx32 "%s\n"
		             "thermal/pwr : %08" PRIx32 " %08" PRIx32 " %08" PRIx32 " %08" PRIx32 "\n"
		             "misc enable : %016" PRIx64 "\n",
		             cpu->id,
		             cpu->arch.cpuid.vendor,
		             family,
		             model,
		             cpu->arch.cpuid.name,
		             stepping,
		             apic_id,
		             flags,
		             cpuid->feat_ecx,
		             cpuid->feat_edx,
		             clflush_size,
		             cpuid->cache_eax,
		             cpuid->cache_ebx,
		             cpuid->cache_ecx,
		             cpuid->cache_edx,
		             cache,
		             cpuid->th_pm_eax,
		             cpuid->th_pm_ebx,
		             cpuid->th_pm_ecx,
		             cpuid->th_pm_edx,
		             rdmsr(IA32_MISC_ENABLE));
	}
	return 0;
}

static ssize_t cpuinfo_read(struct file *file, struct uio *uio)
{
	(void)file;
	size_t count = uio->count;
	off_t off = uio->off;
	int ret = print_cpuinfo(uio);
	if (ret < 0)
		return ret;
	uio->off = off + count - uio->count;
	return count - uio->count;
}

static const struct file_op cpuinfo_fop =
{
	.read = cpuinfo_read,
};

int cpuinfo_register_sysfs(void)
{
	return sysfs_mknode("cpuinfo", 0, 0, 0444, &cpuinfo_fop, NULL);
}
