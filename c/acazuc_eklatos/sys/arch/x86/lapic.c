#include "apic.h"
#include "arch/x86/asm.h"
#include "arch/x86/msr.h"

#include <std.h>
#include <mem.h>

/*
 * for software intended to run on Pentium processors, system
 * software should explicitly not map the APIC register space
 * to regular system memory. Doing so can result in an invalid
 * opcode exception (#UD) being generated or unpredictable execution.
 */

#define LAPIC_REG_ID        0x20
#define LAPIC_REG_VERSION   0x30
#define LAPIC_REG_TPR       0x80
#define LAPIC_REG_APR       0x90
#define LAPIC_REG_PPR       0xA0
#define LAPIC_REG_EOI       0xB0
#define LAPIC_REG_RRD       0xC0
#define LAPIC_REG_LOG_DST   0xD0
#define LAPIC_REG_DST_FMT   0xE0
#define LAPIC_REG_SPUR_IV   0xF0
#define LAPIC_REG_ISR       0x100 /* 8 * 0x10 bytes */
#define LAPIC_REG_TMR       0x180 /* 8 * 0x10 bytes */
#define LAPIC_REG_IRR       0x200 /* 8 * 0x10 bytes */
#define LAPIC_REG_ERR_STT   0x280
#define LAPIC_REG_CMCI      0x2F0
#define LAPIC_REG_ICR       0x300 /* 2 * 0x10 bytes */
#define LAPIC_REG_ICR2      0x310
#define LAPIC_REG_LVT_TMR   0x320
#define LAPIC_REG_LVT_TSR   0x330
#define LAPIC_REG_LVT_PMC   0x340
#define LAPIC_REG_LVT_LINT0 0x350
#define LAPIC_REG_LVT_LINT1 0x360
#define LAPIC_REG_LVT_LVTE  0x370
#define LAPIC_REG_INIT_CNT  0x380
#define LAPIC_REG_CUR_CNT   0x390
#define LAPIC_REG_DIV_CONF  0x3E0

uint32_t g_lapics[256];
size_t g_lapics_count;

static struct page g_page;
static uint8_t volatile *g_addr;

static inline void lapic_wr(uint32_t reg, uint32_t v)
{
	*(uint32_t volatile*)&g_addr[reg] = v;
}

static inline uint32_t lapic_rd(uint32_t reg)
{
	return *(uint32_t volatile*)&g_addr[reg];
}

void lapic_init(void)
{
	uint64_t apic_base = rdmsr(IA32_APIC_BASE);
	pm_init_page(&g_page, apic_base / PAGE_SIZE);
	g_addr = vm_map(&g_page, PAGE_SIZE, VM_PROT_RW);
	if (!g_addr)
		panic("lapic: failed to vmap\n");
	lapic_wr(LAPIC_REG_SPUR_IV, 0x1FF);
#if 0
	printf("lapic base: 0x%08" PRIx32 " %" PRIx32 "\n", hi, lo);
	printf("lapic id: 0x%" PRIx32 "\n", lapic_rd(LAPIC_REG_ID));
	printf("lapic version: 0x%" PRIx32 "\n", lapic_rd(LAPIC_REG_VERSION));
#endif
}

void lapic_eoi(void)
{
	lapic_wr(LAPIC_REG_EOI, 0);
}

void lapic_init_smp(void)
{
	lapic_wr(LAPIC_REG_SPUR_IV, 0x1FF);
}

void lapic_sendipi(enum lapic_ipi_dst_type dst_type, uint8_t dst, uint8_t id)
{
	lapic_wr(LAPIC_REG_ERR_STT, 0);
	lapic_wr(LAPIC_REG_ICR2, (lapic_rd(LAPIC_REG_ICR2) & 0x00FFFFFF) | (dst << 24));
	uint8_t shorthand;
	switch (dst_type)
	{
		case LAPIC_IPI_DST:
			shorthand = 0;
			break;
		case LAPIC_IPI_SELF:
			shorthand = 1;
			break;
		case LAPIC_IPI_BROADCAST:
			shorthand = 2;
			break;
		case LAPIC_IPI_OTHER:
			shorthand = 3;
			break;
		default:
			panic("invalid IPI dst type\n");
			return;
	}
	lapic_wr(LAPIC_REG_ICR, (lapic_rd(LAPIC_REG_ICR) & 0xFFF00000) | (shorthand << 18) | 0x04000 | id);
	do
	{
		pause();
	} while (lapic_rd(LAPIC_REG_ICR) & (1 << 12));
}

void lapic_send_init_ipi(uint8_t dst)
{
	lapic_wr(LAPIC_REG_ERR_STT, 0);
	lapic_wr(LAPIC_REG_ICR2, (lapic_rd(LAPIC_REG_ICR2) & 0x00FFFFFF) | (dst << 24));
	lapic_wr(LAPIC_REG_ICR, (lapic_rd(LAPIC_REG_ICR) & 0xFFF00000) | 0x00C500);
	do
	{
		pause();
	} while (lapic_rd(LAPIC_REG_ICR) & (1 << 12));
}

void lapic_send_startup_ipi(uint8_t dst, uintptr_t addr)
{
	assert(!(addr & PAGE_MASK), "non-aligned startup address\n");
	assert(addr < UINT16_MAX, "out of range stratup address\n");
	lapic_wr(LAPIC_REG_ERR_STT, 0);
	lapic_wr(LAPIC_REG_ICR2, (lapic_rd(LAPIC_REG_ICR2) & 0x00FFFFFF) | (dst << 24));
	lapic_wr(LAPIC_REG_ICR, (lapic_rd(LAPIC_REG_ICR) & 0xFFF0F800) | 0x000600 | (addr >> 12));
	do
	{
		pause();
	} while (lapic_rd(LAPIC_REG_ICR) & (1 << 12));
}

void lapic_send_init_deassert(uint8_t dst)
{
	lapic_wr(LAPIC_REG_ICR2, (lapic_rd(LAPIC_REG_ICR2) & 0x00FFFFFF) | (dst << 24));
	lapic_wr(LAPIC_REG_ICR, (lapic_rd(LAPIC_REG_ICR) & 0xFFF00000) | 0x008500);
	do
	{
		pause();
	} while (lapic_rd(LAPIC_REG_ICR) & (1 << 12));
}
