#define ENABLE_TRACE

#include "arch/aarch64/gicv2.h"
#include "arch/aarch64/psci.h"

#include <arch/arch.h>
#include <arch/asm.h>

#include <spinlock.h>
#include <types.h>
#include <proc.h>
#include <std.h>
#if WITH_FDT
#include <fdt.h>
#endif
#include <cpu.h>
#include <mem.h>

#if defined(__aarch64__)
#define PSCI_64BIT 0x40000000
#else
#define PSCI_64BIT 0
#endif

#define CMD_PSCI_VERSION            (0x84000000)
#define CMD_CPU_SUSPEND             (0x84000001 | PSCI_64BIT)
#define CMD_CPU_OFF                 (0x84000002)
#define CMD_CPU_ON                  (0x84000003 | PSCI_64BIT)
#define CMD_AFFINITY_INFO           (0x84000004 | PSCI_64BIT)
#define CMD_MIGRATE                 (0x84000005 | PSCI_64BIT)
#define CMD_MIGRATE_INFO_TYPE       (0x84000006)
#define CMD_MIGRATE_INFO_UP_CPU     (0x84000007 | PSCI_64BIT)
#define CMD_SYSTEM_OFF              (0x84000008)
#define CMD_SYSTEM_OFF2             (0x84000015 | PSCI_64BIT)
#define CMD_SYSTEM_RESET            (0x84000009)
#define CMD_SYSTEM_RESET2           (0x84000012 | PSCI_64BIT)
#define CMD_MEM_PROTECT             (0x84000013)
#define CMD_MEM_PROTECT_CHECK_RANGE (0x84000014 | PSCI_64BIT)
#define CMD_PSCI_FEATURES           (0x8400000A)
#define CMD_CPU_FREEZE              (0x8400000B)
#define CMD_CPU_DEFAULT_SUSPEND     (0x8400000C | PSCI_64BIT)
#define CMD_NODE_HW_STATE           (0x8400000D | PSCI_64BIT)
#define CMD_SYSTEM_SUSPEND          (0x8400000E | PSCI_64BIT)
#define CMD_PSCI_SET_SUSPEND_MODE   (0x8400000F)
#define CMD_PSCI_STAT_RESIDENCY     (0x84000010 | PSCI_64BIT)
#define CMD_PSCI_STAT_COUNT         (0x84000011 | PSCI_64BIT)

#define PSCI_SUCCESS            0
#define PSCI_NOT_SUPPORTED      ((size_t)-1)
#define PSCI_INVALID_PARAMETERS ((size_t)-2)
#define PSCI_DENIED             ((size_t)-3)
#define PSCI_ALREADY_ON         ((size_t)-4)
#define PSCI_ON_PENDING         ((size_t)-5)
#define PCSI_INTERNAL_FAILURE   ((size_t)-6)
#define PSCI_NOT_PRESENT        ((size_t)-7)
#define PSCI_DISABLED           ((size_t)-8)
#define PSCI_INVALID_ADDRESS    ((size_t)-9)

#if defined(__aarch64__)
#define get_mpidr() get_mpidr_el1()
#define MPIDR_MASK 0xFF00FFFFFF
#else
#define MPIDR_MASK 0xFFFFFF
#endif

void smp_trampoline(void);

enum
{
	PSCI_NONE,
	PSCI_SMC,
	PSCI_HVC,
} psci_mode = PSCI_NONE;

extern uintptr_t kern_paddr;

static uintptr_t psci_mpidr[MAXCPU];
static uint32_t psci_giccid[MAXCPU];
static size_t psci_cpu_count;

static inline size_t psci_call0(uint32_t id)
{
	register long r_id __asm__ ("r0") = id;
	register long r_ret __asm__ ("r0");
	__asm__ volatile ("hvc #0":
	                  "=r"(r_ret):
	                  "r"(r_id));
	return r_ret;
}

static inline size_t psci_call1(uint32_t id, uintptr_t arg1)
{
	register long r_id __asm__ ("r0") = id;
	register long r_ret __asm__ ("r0");
	register long r_arg1 __asm__ ("r1") = arg1;
	__asm__ volatile ("hvc #0":
	                  "=r"(r_ret):
	                  "r"(r_id),
	                  "r"(r_arg1));
	return r_ret;
}

static inline size_t psci_call3(uint32_t id, uintptr_t arg1,
                                uintptr_t arg2, uintptr_t arg3)
{
	register long r_id __asm__ ("r0") = id;
	register long r_ret __asm__ ("r0");
	register long r_arg1 __asm__ ("r1") = arg1;
	register long r_arg2 __asm__ ("r2") = arg2;
	register long r_arg3 __asm__ ("r3") = arg3;
	__asm__ volatile ("hvc #0":
	                  "=r"(r_ret):
	                  "r"(r_id),
	                  "r"(r_arg1),
	                  "r"(r_arg2),
	                  "r"(r_arg3));
	return r_ret;
}

void psci_init(int use_hvc)
{
	psci_mode = use_hvc ? PSCI_HVC : PSCI_SMC;
}

#if WITH_FDT
int psci_init_fdt(struct fdt_node *node)
{
	struct fdt_prop *method = fdt_get_prop(node, "method");
	if (!method)
	{
		TRACE("psci: no 'method' property");
		return -EINVAL;
	}
	if (!method->len)
	{
		TRACE("psci: empty 'method' property");
		return -EINVAL;
	}
	if (!strcmp((char*)method->data, "hvc"))
	{
		psci_init(1);
		return 0;
	}
	if (!strcmp((char*)method->data, "smc"))
	{
		psci_init(0);
		return 0;
	}
	TRACE("psci: unknown method");
	return -EINVAL;
}
#endif

void psci_add_cpu(uintptr_t mpidr, uint32_t giccid)
{
	psci_mpidr[psci_cpu_count] = mpidr;
	psci_giccid[psci_cpu_count] = giccid;
	psci_cpu_count++;
}

int arch_start_smp_cpu(struct cpu *cpu, size_t smp_id)
{
	uintptr_t smp_addr = (uintptr_t)&smp_trampoline - VADDR_CODE_BEGIN + kern_paddr;
	uintptr_t mpidr = psci_mpidr[smp_id] & MPIDR_MASK;
	if (mpidr == (get_mpidr() & MPIDR_MASK))
		return -EAGAIN;
	cpu->arch.mpidr = psci_mpidr[smp_id];
	cpu->arch.gicc_id = psci_giccid[smp_id];
	size_t ret = psci_call3(CMD_CPU_ON, mpidr, smp_addr, kern_paddr);
	if (ret)
	{
		TRACE("psci: failed to init cpu: 0x%zx", ret);
		return ret;
	}
	return 0;
}

void arch_start_smp(void)
{
	if (psci_mode != PSCI_HVC)
	{
		TRACE("psci: non-hvc not supported");
		return;
	}
	size_t version = psci_call0(CMD_PSCI_VERSION);
	if (version != 0x10001
	 && version != 0x10000
	 && version != 0x00002)
	{
		TRACE("psci: invalid version: 0x%08" PRIx32, version);
		return;
	}
	cpu_start_smp(psci_cpu_count);
}

int psci_shutdown(void)
{
	if (psci_mode != PSCI_HVC)
	{
		TRACE("psci: non-hvc not supported");
		return -EINVAL;
	}
	size_t ret = psci_call0(CMD_SYSTEM_OFF);
	TRACE("psci: shutdown failed: 0x%zx", ret);
	return -EINVAL;
}

int psci_reboot(void)
{
	if (psci_mode != PSCI_HVC)
	{
		TRACE("psci: non-hvc not supported");
		return -EINVAL;
	}
	size_t ret = psci_call0(CMD_SYSTEM_RESET);
	TRACE("psci: reboot failed: 0x%zx", ret);
	return -EINVAL;
}

int psci_suspend(void)
{
	if (psci_mode != PSCI_HVC)
	{
		TRACE("psci: non-hvc not supported");
		return -EINVAL;
	}
	size_t version = psci_call0(CMD_PSCI_VERSION);
	if (version < 0x10000)
	{
		TRACE("psci: version too old for suspend command");
		return -EINVAL;
	}
	size_t features = psci_call1(CMD_PSCI_FEATURES, CMD_SYSTEM_SUSPEND);
	if (features == PSCI_NOT_SUPPORTED)
	{
		TRACE("psci: suspend not supported");
		return -EINVAL;
	}
	size_t ret = psci_call0(CMD_SYSTEM_SUSPEND);
	TRACE("psci: suspend failed: 0x%zx", ret);
	return -EINVAL;
}
