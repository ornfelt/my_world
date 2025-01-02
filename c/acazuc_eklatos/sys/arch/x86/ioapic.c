#include "apic.h"

#include <file.h>
#include <std.h>
#include <uio.h>
#include <vfs.h>
#include <mem.h>

/*
 * Intel®
 * 82093AA I/O ADVANCED
 * PROGRAMMABLE INTERRUPT
 * CONTROLLER (IOAPIC)
 */

#define IOAPIC_IOREGSEL 0x0
#define IOAPIC_IOWIN    0x4

#define IOAPIC_REG_ID     0x0
#define IOAPIC_REG_VER    0x1
#define IOAPIC_REG_ARB    0x2
#define IOAPIC_REG_REDTBL 0x10

#define IOAPIC_R_DST_MASK    (0xFFULL << 56)
#define IOAPIC_R_INT_MASK    (0x1     << 16)
#define IOAPIC_R_TRIG_MASK   (0x1     << 15)
#define IOAPIC_R_IRR_MASK    (0x1     << 14)
#define IOAPIC_R_INTPOL_MASK (0x1     << 13)
#define IOAPIC_R_DELIVS_MASK (0x1     << 12)
#define IOAPIC_R_DSTMOD_MASK (0x1     << 11)
#define IOAPIC_R_DELMOD_MASK (0x7     << 8)
#define IOAPIC_R_INTVEC_MASK (0xFF    << 0)

struct ioapic
{
	uint8_t id;
	uint32_t gsib;
	uint32_t volatile *data;
	struct page page;
	struct node *sysfs_node;
};

static struct ioapic ioapics[256];
static uint8_t ioapics_ids[256];
static uint8_t ioapics_set[256];
static int ioapics_count;

static inline uint32_t ioapic_rd(struct ioapic *ioapic, uint32_t reg)
{
	ioapic->data[IOAPIC_IOREGSEL] = reg;
	return ioapic->data[IOAPIC_IOWIN];
}

static inline void ioapic_wr(struct ioapic *ioapic, uint32_t reg, uint32_t v)
{
	ioapic->data[IOAPIC_IOREGSEL] = reg;
	ioapic->data[IOAPIC_IOWIN] = v;
}

static int ioapic_sys_open(struct file *file, struct node *node)
{
	file->userdata = node->userdata;
	return 0;
}

static ssize_t ioapic_sys_read(struct file *file, struct uio *uio)
{
	struct ioapic *ioapic = file->userdata;
	size_t count = uio->count;
	off_t off = uio->off;
	uprintf(uio, "id      : 0x%08" PRIx32 "\n",
	        ioapic_rd(ioapic, IOAPIC_REG_ID));
	uprintf(uio, "version : 0x%08" PRIx32 "\n",
	        ioapic_rd(ioapic, IOAPIC_REG_VER));
	uprintf(uio, "arbid   : 0x%08" PRIx32 "\n",
	        ioapic_rd(ioapic, IOAPIC_REG_ARB));
	for (uint8_t i = 0; i < 24; ++i)
	{
		uprintf(uio, "redtbl%02" PRIx8 ": 0x%08" PRIx32 "%08" PRIx32 "\n",
		        i,
		        ioapic_rd(ioapic, IOAPIC_REG_REDTBL + i * 2 + 0),
		        ioapic_rd(ioapic, IOAPIC_REG_REDTBL + i * 2 + 1));
	}
	uio->off = off + count - uio->count;
	return count - uio->count;
}

static struct file_op ioapic_fop =
{
	.open = ioapic_sys_open,
	.read = ioapic_sys_read,
};

void ioapic_init(uint8_t id, uint32_t addr, uint32_t gsib)
{
	if (ioapics_set[id])
		panic("ioapic: multiple ioapic with same id\n");
	ioapics_set[id] = 1;
	ioapics_ids[id] = ioapics_count;
	struct ioapic *ioapic = &ioapics[ioapics_count++];
	ioapic->id = id;
	ioapic->gsib = gsib;
	pm_init_page(&ioapic->page, addr / PAGE_SIZE);
	ioapic->data = (uint32_t volatile*)vm_map(&ioapic->page, PAGE_SIZE,
	                                          VM_PROT_RW);
	if (!ioapic->data)
		panic("ioapic: failed to vmap\n");
	ioapic->data += ((uintptr_t)addr & PAGE_MASK) / sizeof(uint32_t);
	char path[1024];
	snprintf(path, sizeof(path), "ioapic%" PRIu8, ioapics_ids[id]);
	int ret = sysfs_mknode(path, 0, 0, 0400, &ioapic_fop, &ioapic->sysfs_node);
	if (!ret)
		ioapic->sysfs_node->userdata = ioapic;
	else
		printf("ioapic: failed to create sysfs node\n");
}

void ioapic_enable_irq(uint8_t id, uint8_t irq, int active_low,
                       int level_trigger)
{
	if (!ioapics_set[id])
		panic("ioapic: invalid id: %" PRIx8 "\n", id);
	struct ioapic *ioapic = &ioapics[ioapics_ids[id]];
	uint32_t reg_base = IOAPIC_REG_REDTBL + irq * 2;
	uint32_t value = irq + 32;
	if (level_trigger)
		value |= IOAPIC_R_TRIG_MASK;
	if (active_low)
		value |= IOAPIC_R_INTPOL_MASK;
	ioapic_wr(ioapic, reg_base + 0, value);
	ioapic_wr(ioapic, reg_base + 1, 0);
}

void ioapic_disable_irq(uint8_t id, uint8_t irq)
{
	if (!ioapics_set[id])
		panic("ioapic: invalid id: %" PRIx8 "\n", id);
	struct ioapic *ioapic = &ioapics[ioapics_ids[id]];
	uint32_t reg_base = IOAPIC_REG_REDTBL + irq * 2;
	ioapic_wr(ioapic, reg_base + 0, IOAPIC_R_INT_MASK);
	ioapic_wr(ioapic, reg_base + 1, 0);
}

uint8_t ioapic_default_id(void)
{
	uint8_t id = ioapics[0].id;
	if (!ioapics_set[id])
		panic("ioapic: invalid id: %" PRIx8 "\n", id);
	return id;
}
