#define ENABLE_TRACE

#include "arch/aarch64/gicv2.h"

#include <endian.h>
#include <errno.h>
#include <cpu.h>
#include <std.h>
#if WITH_FDT
#include <fdt.h>
#endif
#include <mem.h>

#define GICD_CTLR           0x000 /* distributor control */
#define GICD_TYPER          0x004 /* interrupt controller type */
#define GICD_IIDR           0x008 /* ditributor implement identifier */
#define GICD_IGROUPR(n)    (0x080 + (n) * 0x4) /* interrupt group */
#define GICD_ISENABLER(n)  (0x100 + (n) * 0x4) /* interrupt set-enable */
#define GICD_ICENABLER(n)  (0x180 + (n) * 0x4) /* interrupt clear-enable */
#define GICD_ISPENDR(n)    (0x200 + (n) * 0x4) /* interrupt set-pending */
#define GICD_ICPENDR(n)    (0x280 + (n) * 0x4) /* interrupt clear-pending */
#define GICD_ISACTIVER(n)  (0x300 + (n) * 0x4) /* gicv2 interrupt set-active */
#define GICD_ICACTIVER(n)  (0x380 + (n) * 0x4) /* interrupt clear-active */
#define GICD_IPRIORITYR(n) (0x400 + (n) * 0x4) /* interrupt priority */
#define GICD_ITARGETSR(n)  (0x800 + (n) * 0x4) /* interrupt processor targets */
#define GICD_ICFGR(n)      (0xC00 + (n) * 0x4) /* interrupt configuration */
#define GICD_NSACR(n)      (0xE00 + (n) * 0x4) /* non-secure access control */
#define GICD_SGIR           0xF00 /* software generated interrupt */
#define GICD_CPENDSGIR(n)  (0xF10 + (n) * 0x4) /* SGI clear-pending */
#define GICD_SPENDSGIR(n)  (0xF20 + (n) * 0x4) /* SGI set-pending */

#define GICC_CTLR      0x000 /* CPU interface control */
#define GICC_PMR       0x004 /* interrupt prority mask */
#define GICC_BPR       0x008 /* binary point */
#define GICC_IAR       0x00C /* interrupt acknowledge */
#define GICC_EOIR      0x010 /* end of interrupt */
#define GICC_RPR       0x014 /* running priority */
#define GICC_HPPIR     0x018 /* highest priority pending interrupt */
#define GICC_ABPR      0x01C /* aliased binary point */
#define GICC_AIAR      0x020 /* alias interrupt acknowledge */
#define GICC_AEOIR     0x024 /* aliased end of interrupt */
#define GICC_AHPPIR    0x028 /* aliased highest priority pending interrupt */
#define GICC_APR(n)   (0x0D0 + (n) * 0x4) /* active priorities */
#define GICC_NSAPR(n) (0x0E0 + (n) * 0x4) /* non-secure active priorities */
#define GICC_IIDR      0x0FC /* CPU interface identification */
#define GICC_DIR       0x1000 /* deactivate interrupt */

#define GICM_TYPR      0x008
#define GICM_SETSPI_NS 0x040
#define GICM_IIDR      0xFCC

static void *gicc_data;
static void *gicd_data;
static void *gicm_data;
static uintptr_t gicm_addr;

static inline uint32_t gicc_read(uint32_t reg)
{
	return *(uint32_t volatile*)&((uint8_t*)gicc_data)[reg];
}

static inline void gicc_write(uint32_t reg, uint32_t val)
{
	*(uint32_t volatile*)&((uint8_t*)gicc_data)[reg] = val;
}

static inline uint32_t gicd_read(uint32_t reg)
{
	return *(uint32_t volatile*)&((uint8_t*)gicd_data)[reg];
}

static inline void gicd_write(uint32_t reg, uint32_t val)
{
	*(uint32_t volatile*)&((uint8_t*)gicd_data)[reg] = val;
}

static inline uint32_t gicm_read(uint32_t reg)
{
	return *(uint32_t volatile*)&((uint8_t*)gicm_data)[reg];
}

#if !WITH_ACPI
static
#endif
int gicv2_init_gicc(uintptr_t addr)
{
	struct page page;
	pm_init_page(&page, addr / PAGE_SIZE);
	gicc_data = vm_map(&page, PAGE_SIZE, VM_PROT_RW);
	if (!gicc_data)
	{
		TRACE("gicv2: failed to map gicc");
		return -ENOMEM;
	}
	return 0;
}

#if !WITH_ACPI
static
#endif
int gicv2_init_gicd(uintptr_t addr)
{
	struct page page;
	pm_init_page(&page, addr / PAGE_SIZE);
	gicd_data = vm_map(&page, PAGE_SIZE, VM_PROT_RW);
	if (!gicd_data)
	{
		TRACE("gicv2: failed to map gicd");
		return -ENOMEM;
	}
	return 0;
}

#if !WITH_ACPI
static
#endif
int gicv2_init_gicm(uintptr_t addr)
{
	gicm_addr = addr;
	struct page page;
	pm_init_page(&page, addr / PAGE_SIZE);
	gicm_data = vm_map(&page, PAGE_SIZE, VM_PROT_RW);
	if (!gicm_data)
	{
		TRACE("gicv2: failed to map gicm");
		return -ENOMEM;
	}
	return 0;
}

#if WITH_FDT
int gicv2_init_fdt(struct fdt_node *node)
{
	int ret;
	struct fdt_prop *reg = fdt_get_prop(node, "reg");
	if (!reg)
	{
		TRACE("gicv2: no 'reg' property");
		return -EINVAL;
	}
	uintptr_t gicd_base;
	size_t gicd_size;
	ret = fdt_get_base_size_reg(reg, 0, &gicd_base, &gicd_size);
	if (ret)
	{
		TRACE("gicv2: invalid gicd reg");
		return ret;
	}
	uintptr_t gicc_base;
	size_t gicc_size;
	ret = fdt_get_base_size_reg(reg, 1, &gicc_base, &gicc_size);
	if (ret)
	{
		TRACE("gicv2: invalid gicc reg");
		return ret;
	}
	struct fdt_node *gicm;
	TAILQ_FOREACH(gicm, &node->children, chain)
	{
		if (!fdt_check_compatible(gicm, "arm,gic-v2m-frame"))
			break;
	}
	if (!gicm)
	{
		TRACE("gicv2: no gicm");
		return -EINVAL;
	}
	struct fdt_prop *gicm_reg = fdt_get_prop(gicm, "reg");
	if (!gicm_reg)
	{
		TRACE("gicv2: no gicm 'reg' property");
		return -EINVAL;
	}
	uintptr_t gicm_base;
	size_t gicm_size;
	ret = fdt_get_base_size_reg(gicm_reg, 0, &gicm_base, &gicm_size);
	if (ret)
	{
		TRACE("gicv2: invalid gicm reg");
		return ret;
	}
	ret = gicv2_init_gicd(gicd_base);
	if (ret)
		return ret;
	ret = gicv2_init_gicc(gicc_base);
	if (ret)
		return ret;
	ret = gicv2_init_gicm(gicm_base);
	if (ret)
		return ret;
	return 0;
}
#endif

void gicv2_enable_gicc(void)
{
	gicc_write(GICC_CTLR, 0x1);
	gicc_write(GICC_PMR, 0xFF);
}

void gicv2_enable_gicd(void)
{
	gicd_write(GICD_CTLR, 0x1);
}

int gicv2_get_active_interrupt(size_t *irq)
{
	*irq = gicc_read(GICC_IAR) & 0x3FF;
	if (*irq == 1023)
		return -ENOENT;
	return 0;
}

void gicv2_clear_interrupt(size_t id)
{
	gicd_write(GICD_ICPENDR(id / 32), 1 << (id % 32));
}

void gicv2_enable_interrupt(size_t id)
{
	gicd_write(GICD_ISENABLER(id / 32), 1 << (id % 32));
}

void gicv2_disable_interrupt(size_t id)
{
	gicd_write(GICD_ICENABLER(id / 32), 1 << (id % 32));
}

void gicv2_set_edge_trigger(size_t id)
{
	gicd_write(GICD_ICFGR(id / 16), (gicd_read(GICD_ICFGR(id / 16)) & ~(3 << ((id % 16) * 2))) | (2 << ((id % 16) * 2)));
}

void gicv2_eoi(size_t id)
{
	gicc_write(GICC_EOIR, id);
}

uint64_t gicv2_get_msi_addr(void)
{
	return gicm_addr + GICM_SETSPI_NS;
}

uint32_t gicv2_get_msi_data(size_t irq)
{
	return irq;
}

void gicv2_set_irq_cpu(size_t irq, size_t cpu)
{
	(void)irq;
	(void)cpu;
	/* XXX */
}

size_t gicv2_get_msi_min_irq(void)
{
	return gicm_read(GICM_TYPR) >> 16;
}

size_t gicv2_get_msi_max_irq(void)
{
	return gicv2_get_msi_min_irq() + (gicm_read(GICM_TYPR) & 0xFFFF);
}

void gicv2_sgi(struct cpu *cpu, uint8_t irq)
{
	assert(irq < 0x10, "invalid irq id\n");
	gicd_write(GICD_SGIR, (0x10000 << cpu->arch.gicc_id) | irq);
}
