#include "mem.h"
#include "cpu.h"
#include "nds.h"
#include "mbc.h"
#include "apu.h"
#include "gpu.h"

#include <inttypes.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

static const uint16_t timer_increments[4] = {1 << 10, 1 << 4, 1 << 2, 1 << 0};

static const uint32_t dma_len_max[4] = {0x4000, 0x4000, 0x4000, 0x10000};

static const uint8_t arm7_mram_cycles_32[] = {0, 2, 10, 2,  9};
static const uint8_t arm7_mram_cycles_16[] = {0, 1,  9, 1,  8};
static const uint8_t arm7_mram_cycles_8[]  = {0, 1,  9, 1,  8};

static const uint8_t arm7_wram_cycles_32[] = {0, 1,  1, 1,  1};
static const uint8_t arm7_wram_cycles_16[] = {0, 1,  1, 1,  1};
static const uint8_t arm7_wram_cycles_8[]  = {0, 1,  1, 1,  1};

static const uint8_t arm7_vram_cycles_32[] = {0, 2,  1, 2,  2};
static const uint8_t arm7_vram_cycles_16[] = {0, 1,  1, 1,  1};
static const uint8_t arm7_vram_cycles_8[]  = {0, 1,  1, 1,  1};

/* XXX until cache is implemented, use this */
#if 0
static const uint8_t arm9_mram_cycles_32[] = {0, 4, 20, 18, 18};
static const uint8_t arm9_mram_cycles_16[] = {0, 2, 18,  9,  9};
static const uint8_t arm9_mram_cycles_8[]  = {0, 2, 18,  9,  9};
#else
static const uint8_t arm9_mram_cycles_32[] = {0, 1, 1, 1, 1};
static const uint8_t arm9_mram_cycles_16[] = {0, 1, 1, 1, 1};
static const uint8_t arm9_mram_cycles_8[]  = {0, 1, 1, 1, 1};
#endif

static const uint8_t arm9_wram_cycles_32[] = {0, 2,  8,  8,  8};
static const uint8_t arm9_wram_cycles_16[] = {0, 2,  8,  4,  4};
static const uint8_t arm9_wram_cycles_8[]  = {0, 2,  8,  4,  4};

static const uint8_t arm9_vram_cycles_32[] = {0, 4, 10, 10, 10};
static const uint8_t arm9_vram_cycles_16[] = {0, 2,  8,  5,  5};
static const uint8_t arm9_vram_cycles_8[]  = {0, 2,  8,  5,  5};

static const uint8_t arm9_tcm_cycles_32[]  = {0, 1,  1, 1,  1};
static const uint8_t arm9_tcm_cycles_16[]  = {0, 1,  1, 1,  1};
static const uint8_t arm9_tcm_cycles_8[]   = {0, 1,  1, 1,  1};

static const struct gx_cmd_def gx_cmd_defs[256] =
{
#define GX_CMD_DEF(name, params) \
	[GX_CMD_##name] = {#name, params}

	GX_CMD_DEF(MTX_MODE,       1),
	GX_CMD_DEF(MTX_PUSH,       0),
	GX_CMD_DEF(MTX_POP,        1),
	GX_CMD_DEF(MTX_STORE,      1),
	GX_CMD_DEF(MTX_RESTORE,    1),
	GX_CMD_DEF(MTX_IDENTITY,   0),
	GX_CMD_DEF(MTX_LOAD_4X4,   16),
	GX_CMD_DEF(MTX_LOAD_4X3,   12),
	GX_CMD_DEF(MTX_MULT_4X4,   16),
	GX_CMD_DEF(MTX_MULT_4X3,   12),
	GX_CMD_DEF(MTX_MULT_3X3,   9),
	GX_CMD_DEF(MTX_SCALE,      3),
	GX_CMD_DEF(MTX_TRANS,      3),
	GX_CMD_DEF(COLOR,          1),
	GX_CMD_DEF(NORMAL,         1),
	GX_CMD_DEF(TEXCOORD,       1),
	GX_CMD_DEF(VTX_16,         2),
	GX_CMD_DEF(VTX_10,         1),
	GX_CMD_DEF(VTX_XY,         1),
	GX_CMD_DEF(VTX_XZ,         1),
	GX_CMD_DEF(VTX_YZ,         1),
	GX_CMD_DEF(VTX_DIFF,       1),
	GX_CMD_DEF(POLYGON_ATTR,   1),
	GX_CMD_DEF(TEXIMAGE_PARAM, 1),
	GX_CMD_DEF(PLTT_BASE,      1),
	GX_CMD_DEF(DIF_AMB,        1),
	GX_CMD_DEF(SPE_EMI,        1),
	GX_CMD_DEF(LIGHT_VECTOR,   1),
	GX_CMD_DEF(LIGHT_COLOR,    1),
	GX_CMD_DEF(SHININESS,      32),
	GX_CMD_DEF(BEGIN_VTXS,     1),
	GX_CMD_DEF(END_VTXS,       0),
	GX_CMD_DEF(SWAP_BUFFERS,   1),
	GX_CMD_DEF(VIEWPORT,       1),
	GX_CMD_DEF(BOX_TEST,       3),
	GX_CMD_DEF(POS_TEST,       2),
	GX_CMD_DEF(VEC_TEST,       1),

#undef GX_CMD_DEF
};

static void update_vram_maps(struct mem *mem);
static void update_gxfifo_irq(struct mem *mem);

struct mem *mem_new(struct nds *nds, struct mbc *mbc)
{
	struct mem *mem = calloc(sizeof(*mem), 1);
	if (!mem)
		return NULL;

	mem->nds = nds;
	mem->mbc = mbc;
	mem->arm7_wram_base = 0;
	mem->arm7_wram_mask = 0;
	mem->arm9_wram_base = 0;
	mem->arm9_wram_base = 0x7FFF;
	mem_arm9_set_reg32(mem, MEM_ARM7_REG_ROMCTRL, 1 << 23);
	mem_arm7_set_reg32(mem, MEM_ARM7_REG_SOUNDBIAS, 0x200);
	mem_arm7_set_reg32(mem, MEM_ARM7_REG_POWCNT2, 1);
	mem_arm9_set_reg32(mem, MEM_ARM9_REG_GXSTAT, (1 << 25));
	mem->spi_powerman.regs[0x0] = 0x0C; /* enable backlight */
	mem->spi_powerman.regs[0x4] = 0x42; /* high brightness */
	update_vram_maps(mem);
	mem->sram_size = 0x40000 + mbc->backup_size;
	mem->sram = calloc(mem->sram_size, 1);
	if (!mem->sram)
	{
		free(mem);
		return NULL;
	}
	mbc->backup = &mem->sram[0x40000];
	memset(mbc->backup, 0xFF, mbc->backup_size);
	mem_set_wifi_reg8(mem, MEM_ARM7_REG_W_ID, 0x40);
	mem_set_wifi_reg8(mem, MEM_ARM7_REG_W_ID + 1, 0xC3); /* NDS-lite; 0x14 for NDS */
	mem_set_wifi_reg8(mem, MEM_ARM7_REG_W_RF_STATUS, 9); /* idle */
	mem_set_wifi_reg8(mem, MEM_ARM7_REG_W_POWERSTATE + 1, 2); /* disabled */
	return mem;
}

void mem_del(struct mem *mem)
{
	if (!mem)
		return;
	free(mem->sram);
	free(mem);
}

#define ARM_TIMERS(armv) \
static void arm##armv##_timers(struct mem *mem, uint32_t cycles) \
{ \
	uint16_t prev_overflow = 0; \
	for (unsigned i = 0; i < 4; ++i) \
	{ \
		uint8_t cnt_h = mem_arm##armv##_get_reg8(mem, MEM_ARM##armv##_REG_TM0CNT_H + i * 4); \
		if (!(cnt_h & (1 << 7))) \
		{ \
			prev_overflow = 0; \
			continue; \
		} \
		if (cnt_h & (1 << 2)) \
			mem->arm##armv##_timers[i].v += cycles * (prev_overflow << 10); \
		else \
			mem->arm##armv##_timers[i].v += cycles * timer_increments[cnt_h & 3]; \
		prev_overflow = 0; \
		if (mem->arm##armv##_timers[i].v < (0x10000 << 10)) \
			continue; \
		do \
		{ \
			/* printf("[ARM" #armv "] timer %u overflow (cnt_h: %02" PRIx8 ")\n", i, cnt_h); */ \
			mem->arm##armv##_timers[i].v -= (0x10000 << 10); \
			mem->arm##armv##_timers[i].v += mem_arm##armv##_get_reg16(mem, MEM_ARM##armv##_REG_TM0CNT_L + i * 4) << 10; \
			if (cnt_h & (1 << 6)) \
				mem_arm##armv##_irq(mem, 1 << (3 + i)); \
			prev_overflow++; \
		} \
		while (mem->arm##armv##_timers[i].v >= (0x10000 << 10)); \
	} \
} \
static void arm##armv##_timer_control(struct mem *mem, uint8_t timer, uint8_t v) \
{ \
	uint8_t prev = mem_arm##armv##_get_reg8(mem, MEM_ARM##armv##_REG_TM0CNT_H + timer * 4); \
	mem_arm##armv##_set_reg8(mem, MEM_ARM##armv##_REG_TM0CNT_H + timer * 4, v); \
	if ((v & (1 << 7)) && !(prev & (1 << 7))) \
		mem->arm##armv##_timers[timer].v = mem_arm##armv##_get_reg16(mem, MEM_ARM##armv##_REG_TM0CNT_L + timer * 4) << 10; \
}

ARM_TIMERS(7);
ARM_TIMERS(9);

void mem_timers(struct mem *mem, uint32_t cycles)
{
	arm7_timers(mem, cycles);
	arm9_timers(mem, cycles);
}

static void arm7_dma_start(struct mem *mem, uint8_t cond);
static void arm9_dma_start(struct mem *mem, uint8_t cond);

#define ARM_DMA(armv) \
static void arm##armv##_dma(struct mem *mem, uint8_t id, uint32_t cycles) \
{ \
	struct dma *dma = &mem->arm##armv##_dma[id]; \
	if (dma->status != (MEM_DMA_ACTIVE | MEM_DMA_ENABLE) ) \
		return; \
	while (cycles--) \
	{ \
		uint16_t cnt_h = mem_arm##armv##_get_reg16(mem, MEM_ARM##armv##_REG_DMA0CNT_H + 0xC * id); \
		uint32_t step; \
		if (cnt_h & (1 << 10)) \
		{ \
			/* printf("[ARM" #armv "] DMA %" PRIu8 " 32 bits from 0x%" PRIx32 " to 0x%" PRIx32 "\n", id, dma->src, dma->dst); */ \
			mem_arm##armv##_set32(mem, dma->dst, \
			                      mem_arm##armv##_get32(mem, dma->src, MEM_DIRECT), \
			                      MEM_DIRECT); \
			step = 4; \
		} \
		else \
		{ \
			/* printf("[ARM" #armv "] DMA %" PRIu8 " 16 bits from 0x%" PRIx32 " to 0x%" PRIx32 "\n", id, dma->src, dma->dst); */ \
			mem_arm##armv##_set16(mem, dma->dst, \
			                      mem_arm##armv##_get16(mem, dma->src, MEM_DIRECT), \
			                      MEM_DIRECT); \
			step = 2; \
		} \
		switch ((cnt_h >> 5) & 3) \
		{ \
			case 0: \
				dma->dst += step; \
				break; \
			case 1: \
				dma->dst -= step; \
				break; \
			case 2: \
				break; \
			case 3: \
				dma->dst += step; \
				break; \
		} \
		switch ((cnt_h >> 7) & 3) \
		{ \
			case 0: \
				dma->src += step; \
				break; \
			case 1: \
				dma->src -= step; \
				break; \
			case 2: \
				break; \
			case 3: \
				break; \
		} \
		dma->cnt++; \
		if (dma->cnt != dma->len) \
			continue; \
		/* printf("[ARM" #armv "] DMA %" PRIu8 " end\n", id); */ \
		if ((cnt_h & (1 << 9))) \
		{ \
			if ((armv == 7 && ((cnt_h >> 12) & 0x3) == 0x2) \
			 || (armv == 9 && ((cnt_h >> 11) & 0x7) == 0x5)) \
			{ \
				if (!(mem_arm9_get_reg32(mem, MEM_ARM9_REG_ROMCTRL) & (1 << 31))) \
				{ \
					dma->status &= ~(MEM_DMA_ACTIVE | MEM_DMA_ENABLE); \
					mem->dscard_dma_count--; \
				} \
			} \
			else \
			{ \
				dma->status &= ~MEM_DMA_ACTIVE; \
				if (dma->dst == (0x4000000 | MEM_ARM9_REG_GXFIFO)) \
				{ \
					mem->gxfifo_dma_count--; \
					update_gxfifo_irq(mem); \
					if (!mem->gxfifo_dma_count) \
						arm9_dma_start(mem, 7); \
				} \
			} \
		} \
		else \
		{ \
			dma->status = 0; \
			if (dma->dst == (0x4000000 | MEM_ARM9_REG_GXFIFO)) \
			{ \
				mem->gxfifo_dma_count--; \
				update_gxfifo_irq(mem); \
				if (!mem->gxfifo_dma_count) \
					arm9_dma_start(mem, 7); \
			} \
		} \
		dma->cnt = 0; \
		if (cnt_h & (1 << 14)) \
			mem_arm##armv##_irq(mem, (1 << (8 + id))); \
		if (!(dma->status & MEM_DMA_ACTIVE)) \
		{ \
			mem_arm##armv##_set_reg16(mem, MEM_ARM##armv##_REG_DMA0CNT_H + 0xC * id, \
			                          mem_arm##armv##_get_reg16(mem, MEM_ARM##armv##_REG_DMA0CNT_H + 0xC * id) & ~(1 << 15)); \
			break; \
		} \
	} \
} \
static void arm##armv##_load_dma_length(struct mem *mem, size_t id) \
{ \
	struct dma *dma = &mem->arm##armv##_dma[id]; \
	dma->len = mem_arm##armv##_get_reg16(mem, MEM_ARM##armv##_REG_DMA0CNT_L + 0xC * id); \
	if (armv == 7) \
	{ \
		if (dma->len) \
		{ \
			if (dma->len > dma_len_max[id]) \
				dma->len = dma_len_max[id]; \
		} \
		else \
		{ \
			dma->len = dma_len_max[id]; \
		} \
	} \
	else \
	{ \
		if (dma->len) \
		{ \
			dma->len &= 0x1FFFFF; \
		} \
		else \
		{ \
			dma->len = 0x200000; \
		} \
	} \
} \
static void arm##armv##_dma_control(struct mem *mem, uint8_t id) \
{ \
	struct dma *dma = &mem->arm##armv##_dma[id]; \
	dma->src = mem_arm##armv##_get_reg32(mem, MEM_ARM##armv##_REG_DMA0SAD + 0xC * id); \
	dma->dst = mem_arm##armv##_get_reg32(mem, MEM_ARM##armv##_REG_DMA0DAD + 0xC * id); \
	if (armv == 7) \
	{ \
		dma->src &= 0x07FFFFFE; \
		dma->dst &= 0x07FFFFFE; \
	} \
	else \
	{ \
		dma->src &= 0x0FFFFFFE; \
		dma->dst &= 0x0FFFFFFE; \
	} \
	dma->cnt = 0; \
	arm##armv##_load_dma_length(mem, id); \
	uint16_t cnt_h = mem_arm##armv##_get_reg16(mem, MEM_ARM##armv##_REG_DMA0CNT_H + 0xC * id); \
	uint8_t prev_status = dma->status; \
	(void)prev_status; \
	dma->status = 0; \
	if (cnt_h & (1 << 15)) \
		dma->status |= MEM_DMA_ENABLE; \
	if (armv == 7) \
	{ \
		if (!(cnt_h & (3 << 12))) \
			dma->status |= MEM_DMA_ACTIVE; \
	} \
	else \
	{ \
		if (!(cnt_h & (7 << 11)) \
		 || ((cnt_h & (7 << 11)) == (7 << 11) && !mem->gxfifo_dma_count)) \
		{ \
			dma->status |= MEM_DMA_ACTIVE; \
			if (dma->dst == (0x4000000 | MEM_ARM9_REG_GXFIFO)) \
			{ \
				mem->gxfifo_dma_count++; \
				update_gxfifo_irq(mem); \
			} \
		} \
	} \
	if (0 && (dma->status & MEM_DMA_ENABLE)) \
		printf("[ARM" #armv "] enable DMA %" PRIu8 " type %" PRId32 " of %08" PRIx32 " words from %08" PRIx32 " to %08" PRIx32 " CNT_H=%04" PRIx16 " PREV=%02" PRIx8 "\n",  \
		       id, armv == 7 ? ((cnt_h >> 12) & 3) : ((cnt_h >> 11) & 7), \
		       dma->len, dma->src, dma->dst, cnt_h, prev_status); \
} \
static void arm##armv##_dma_start(struct mem *mem, uint8_t cond) \
{ \
	for (uint8_t i = 0; i < 4; ++i) \
	{ \
		struct dma *dma = &mem->arm##armv##_dma[i]; \
		if (!(dma->status & MEM_DMA_ENABLE) \
		 || (dma->status & MEM_DMA_ACTIVE)) \
			continue; \
		uint16_t cnt_h = mem_arm##armv##_get_reg16(mem, MEM_ARM##armv##_REG_DMA0CNT_H + 0xC * i); \
		if (armv == 7) \
		{ \
			if (((cnt_h >> 12) & 0x3) != cond) \
				continue; \
		} \
		else \
		{ \
			if (((cnt_h >> 11) & 0x7) != cond) \
				continue; \
		} \
		if (((cnt_h >> 5) & 0x3) == 0x3) \
			dma->dst = mem_arm##armv##_get_reg32(mem, MEM_ARM##armv##_REG_DMA0DAD + 0xC * i); \
		arm##armv##_load_dma_length(mem, i); \
		dma->cnt = 0; \
		dma->status |= MEM_DMA_ACTIVE; \
		if (armv == 7) \
		{ \
			if (cond == 2) \
				mem->dscard_dma_count++; \
		} \
		else \
		{ \
			if (cond == 5) \
				mem->dscard_dma_count++; \
			if (dma->dst == (0x4000000 | MEM_ARM9_REG_GXFIFO)) \
			{ \
				mem->gxfifo_dma_count++; \
				update_gxfifo_irq(mem); \
			} \
		} \
		/* printf("[ARM" #armv "] start DMA %" PRIu8 " of %08" PRIx32 " words from %08" PRIx32 " to %08" PRIx32 "\n", i, dma->len, dma->src, dma->dst); */ \
	} \
}

ARM_DMA(7);
ARM_DMA(9);

void mem_dma(struct mem *mem, uint32_t cycles)
{
	arm7_dma(mem, 0, cycles);
	arm7_dma(mem, 1, cycles);
	arm7_dma(mem, 2, cycles);
	arm7_dma(mem, 3, cycles);
	arm9_dma(mem, 0, cycles);
	arm9_dma(mem, 1, cycles);
	arm9_dma(mem, 2, cycles);
	arm9_dma(mem, 3, cycles);
}

void mem_vblank(struct mem *mem)
{
	arm7_dma_start(mem, 1);
	arm9_dma_start(mem, 1);
}

void mem_hblank(struct mem *mem)
{
	arm9_dma_start(mem, 2);
}

void mem_dscard(struct mem *mem)
{
	arm7_dma_start(mem, 2);
	arm9_dma_start(mem, 5);
}

static void update_gxfifo_irq(struct mem *mem)
{
	/* nasty hack v2: fake non-available DMA if irq is running */
	if (mem->gxfifo_dma_count)
	{
		mem_arm9_set_reg32(mem, MEM_ARM9_REG_IF, mem_arm9_get_reg32(mem, MEM_ARM9_REG_IF) & ~(1 << 21));
		cpu_update_irq_state(mem->nds->arm9);
		return;
	}
	switch ((mem_arm9_get_reg32(mem, MEM_ARM9_REG_GXSTAT) >> 30) & 0x3)
	{
		case 0:
		case 3:
			mem_arm9_set_reg32(mem, MEM_ARM9_REG_IF, mem_arm9_get_reg32(mem, MEM_ARM9_REG_IF) & ~(1 << 21));
			break;
		case 1:
		case 2:
			mem_arm9_set_reg32(mem, MEM_ARM9_REG_IF, mem_arm9_get_reg32(mem, MEM_ARM9_REG_IF) | (1 << 21));
			break;
	}
	cpu_update_irq_state(mem->nds->arm9);
}

void mem_arm9_irq(struct mem *mem, uint32_t f)
{
	mem_arm9_set_reg32(mem, MEM_ARM9_REG_IF, mem_arm9_get_reg32(mem, MEM_ARM9_REG_IF) | f);
	cpu_update_irq_state(mem->nds->arm9);
}

void mem_arm7_irq(struct mem *mem, uint32_t f)
{
	mem_arm7_set_reg32(mem, MEM_ARM7_REG_IF, mem_arm7_get_reg32(mem, MEM_ARM7_REG_IF) | f);
	cpu_update_irq_state(mem->nds->arm7);
}

static uint8_t powerman_read(struct mem *mem)
{
#if 0
	printf("[%08" PRIx32 "] SPI powerman read 0x%02" PRIx8 "\n",
	       cpu_get_reg(mem->nds->arm7, CPU_REG_PC),
	       mem->spi_powerman.read_latch);
#endif
	return mem->spi_powerman.read_latch;
}

static uint8_t firmware_read(struct mem *mem)
{
#if 0
	printf("[%08" PRIx32 "] SPI firmware read 0x%02" PRIx8 "\n",
	       cpu_get_reg(mem->nds->arm7, CPU_REG_PC),
	       mem->spi_firmware.read_latch);
#endif
	return mem->spi_firmware.read_latch;
}

static uint8_t touchscreen_read(struct mem *mem)
{
	uint8_t v;
	if (!mem->spi_touchscreen.read_pos)
	{
		v = mem->spi_touchscreen.read_latch >> 5;
		mem->spi_touchscreen.read_pos = 1;
	}
	else
	{
		v = mem->spi_touchscreen.read_latch << 3;
	}
#if 0
	printf("[%08" PRIx32 "] SPI touchscreen read 0x%02" PRIx8 "\n",
	       cpu_get_reg(mem->nds->arm7, CPU_REG_PC), v);
#endif
	return v;
}

static void powerman_write(struct mem *mem, uint8_t v)
{
#if 0
	printf("[%08" PRIx32 "] SPI powerman write 0x%02" PRIx8 "\n",
	       cpu_get_reg(mem->nds->arm7, CPU_REG_PC), v);
#endif
	if (mem->spi_powerman.has_cmd)
	{
		static const uint8_t regs[8] = {0, 1, 2, 3, 4, 4, 4, 4};
		uint8_t reg = regs[mem->spi_powerman.cmd & 0x7];
		if (mem->spi_powerman.cmd & (1 << 7))
		{
			uint8_t val = mem->spi_powerman.regs[reg];
#if 0
			printf("SPI powerman read reg[0x%" PRIx8 "] = 0x%02" PRIx8 "\n",
			       reg, val);
#endif
			mem->spi_powerman.read_latch = val;
		}
		else
		{
			static const uint8_t write_masks[5] = {0x7F, 0x00, 0x01, 0x03, 0x07};
			mem->spi_powerman.regs[reg] = (v & write_masks[reg])
			                            | (mem->spi_powerman.regs[reg] & ~write_masks[reg]);
#if 0
			printf("SPI powerman write reg[0x%" PRIx8 "] = 0x%02" PRIx8 "\n",
			       reg, v);
#endif
		}
		return;
	}
	if (mem_arm7_get_reg16(mem, MEM_ARM7_REG_SPICNT) & (1 << 11))
	{
		mem->spi_powerman.has_cmd = 1;
		mem->spi_powerman.cmd = v;
		return;
	}
}

static void firmware_write(struct mem *mem, uint8_t v)
{
#if 0
	printf("[%08" PRIx32 "] SPI firmware write 0x%02" PRIx8 "\n",
	       cpu_get_reg(mem->nds->arm7, CPU_REG_PC), v);
#endif
	switch (mem->spi_firmware.cmd)
	{
		case SPI_FIRMWARE_CMD_NONE:
			mem->spi_firmware.cmd = v;
			switch (v)
			{
				case SPI_FIRMWARE_CMD_READ:
					mem->spi_firmware.posb = 0;
					mem->spi_firmware.addr = 0;
					break;
				case SPI_FIRMWARE_CMD_RDSR:
					break;
				case SPI_FIRMWARE_CMD_WREN:
					mem->spi_firmware.write = 1;
					break;
				case SPI_FIRMWARE_CMD_WRDI:
					mem->spi_firmware.write = 0;
					break;
				case SPI_FIRMWARE_CMD_PW:
					mem->spi_firmware.posb = 0;
					mem->spi_firmware.addr = 0;
					break;
				default:
					printf("unknown SPI firmware cmd: 0x%02" PRIx8 "\n", v);
					return;
			}
			return;
		case SPI_FIRMWARE_CMD_READ:
			if (mem->spi_firmware.posb < 3)
			{
				mem->spi_firmware.addr <<= 8;
				mem->spi_firmware.addr |= v;
				mem->spi_firmware.posb++;
				return;
			}
			mem->spi_firmware.read_latch = mem->sram[mem->spi_firmware.addr & 0x3FFFF];
#if 0
			printf("[%08" PRIx32 "] firmware read: [%05" PRIx32 "] = %02" PRIx8 "\n",
			       cpu_get_reg(mem->nds->arm7, CPU_REG_PC),
			       mem->spi_firmware.addr,
			       mem->spi_firmware.read_latch);
#endif
			mem->spi_firmware.addr++;
			return;
		case SPI_FIRMWARE_CMD_RDSR:
			mem->spi_firmware.read_latch = (mem->spi_firmware.write & 1) << 1;
			return;
		case SPI_FIRMWARE_CMD_PW:
		{
			if (!mem->spi_firmware.write)
			{
				printf("SPI firmware write page without WREN\n");
				return;
			}
			if (mem->spi_firmware.posb < 3)
			{
				mem->spi_firmware.addr <<= 8;
				mem->spi_firmware.addr |= v;
				mem->spi_firmware.posb++;
				return;
			}
			uint32_t addr = mem->spi_firmware.addr & 0x3FFFF;
#if 0
			printf("[%08" PRIx32 "] firmware write: [%05" PRIx32 "] = %02" PRIx8 "\n",
			       cpu_get_reg(mem->nds->arm7, CPU_REG_PC),
			       addr, v);
#endif
			if (addr < 0x200 || addr >= 0x3FA00)
				mem->sram[mem->spi_firmware.addr & 0x3FFFF] = v;
			mem->spi_firmware.addr++;
			break;
		}
	}
}

static void touchscreen_write(struct mem *mem, uint8_t v)
{
#if 0
	printf("[%08" PRIx32 "] SPI touchscreen write 0x%02" PRIx8 "\n",
	       cpu_get_reg(mem->nds->arm7, CPU_REG_PC), v);
#endif
	if (v & (1 << 7))
	{
		mem->spi_touchscreen.channel = (v >> 4) & 0x7;
		mem->spi_touchscreen.has_channel = 1;
		return;
	}
	/* use values matching initial firmware calibration:
	 * p1: 0x000 / 0x000 -> 0x00 / 0x00
	 * p2: 0xFF0 / 0xBF0 -> 0xFF / 0xBF
	 */
	switch (mem->spi_touchscreen.channel)
	{
		case 0x1:
			if (mem->nds->touch)
				mem->spi_touchscreen.read_latch = mem->nds->touch_y * 0x10;
			else
				mem->spi_touchscreen.read_latch = 0xFFF;
			mem->spi_touchscreen.read_pos = 0;
			break;
		case 0x5:
			if (mem->nds->touch)
				mem->spi_touchscreen.read_latch = mem->nds->touch_x * 0x10;
			else
				mem->spi_touchscreen.read_latch = 0x000;
			mem->spi_touchscreen.read_pos = 0;
			break;
		default:
#if 0
			printf("unknown touchscreen channel: %x\n",
			       mem->spi_touchscreen.channel);
#endif
			mem->spi_touchscreen.read_latch = 0x000;
			mem->spi_touchscreen.read_pos = 0;
			break;
	}
}

static void powerman_reset(struct mem *mem)
{
#if 0
	printf("[%08" PRIx32 "] SPI powerman reset\n",
	       cpu_get_reg(mem->nds->arm7, CPU_REG_PC));
#endif
	mem->spi_powerman.has_cmd = 0;
}

static void firmware_reset(struct mem *mem)
{
#if 0
	printf("[%08" PRIx32 "] SPI firmware reset\n",
	       cpu_get_reg(mem->nds->arm7, CPU_REG_PC));
#endif
	mem->spi_firmware.cmd = 0;
}

static void touchscreen_reset(struct mem *mem)
{
#if 0
	printf("[%08" PRIx32 "] SPI touchscreen reset\n",
	       cpu_get_reg(mem->nds->arm7, CPU_REG_PC));
#endif
	mem->spi_touchscreen.has_channel = 0;
}

static uint8_t spi_read(struct mem *mem)
{
#if 0
	printf("[%08" PRIx32 "] SPI read\n",
	       cpu_get_reg(mem->nds->arm7, CPU_REG_PC));
#endif
	switch ((mem_arm7_get_reg16(mem, MEM_ARM7_REG_SPICNT) >> 8) & 0x3)
	{
		case 0:
			return powerman_read(mem);
		case 1:
			return firmware_read(mem);
		case 2:
			return touchscreen_read(mem);
		case 3:
			assert(!"invalid SPI device");
			return 0;
	}
	return 0;
}

static void spi_write(struct mem *mem, uint8_t v)
{
#if 0
	printf("[%08" PRIx32 "] SPI write %02" PRIx8 "\n",
	       cpu_get_reg(mem->nds->arm7, CPU_REG_PC), v);
#endif
	switch ((mem_arm7_get_reg16(mem, MEM_ARM7_REG_SPICNT) >> 8) & 0x3)
	{
		case 0:
			powerman_write(mem, v);
			break;
		case 1:
			firmware_write(mem, v);
			break;
		case 2:
			touchscreen_write(mem, v);
			break;
		case 3:
			assert(!"invalid SPI device");
			return;
	}
	if (!(mem_arm7_get_reg16(mem, MEM_ARM7_REG_SPICNT) & (1 << 11)))
	{
		switch ((mem_arm7_get_reg16(mem, MEM_ARM7_REG_SPICNT) >> 8) & 0x3)
		{
			case 0:
				powerman_reset(mem);
				break;
			case 1:
				firmware_reset(mem);
				break;
			case 2:
				touchscreen_reset(mem);
				break;
		}
	}
	if (mem_arm7_get_reg16(mem, MEM_ARM7_REG_SPICNT) & (1 << 14))
		mem_arm7_irq(mem, 1 << 23);
}

static uint8_t auxspi_read(struct mem *mem)
{
	return mbc_spi_read(mem->mbc);
}

static void auxspi_write(struct mem *mem, uint8_t v)
{
	mbc_spi_write(mem->mbc, v);
	if (!(mem_arm9_get_reg16(mem, MEM_ARM9_REG_AUXSPICNT) & (1 << 6)))
		mbc_spi_reset(mem->mbc);
}

/* the fact that every single RTC on earth uses BCD scares me */
#define BCD(n) (((n) % 10) + (((n) / 10) * 16))
#define DAA(n) (((n) % 16) + (((n) / 16) * 10))

static void rtc_write(struct mem *mem, uint8_t v)
{
#if 0
	printf("rtc write %02" PRIx8 "\n", v);
#endif
	if (v & (1 << 4))
	{
		if (!(v & (1 << 2)))
		{
#if 0
			printf("mem rtc buf reset\n");
#endif
			mem->rtc.inbuf = 0;
			mem->rtc.inlen = 0;
			mem->rtc.cmd_flip = 1;
			mem->rtc.cmd = 0xFF;
			mem->rtc.wpos = 0;
			return;
		}
		if (mem->rtc.cmd_flip)
		{
			mem->rtc.cmd_flip = 0;
			return;
		}
		if (!(v & (1 << 1)))
			return;
		mem->rtc.inbuf |= (v & 1) << (mem->rtc.inlen % 8);
		mem->rtc.inlen++;
		if (mem->rtc.inlen != 8)
			return;
		mem->rtc.inlen = 0;
		if (mem->rtc.cmd == 0xFF)
		{
			mem->rtc.cmd = mem->rtc.inbuf;
#if 0
			printf("rtc cmd %02" PRIx8 "\n", mem->rtc.cmd);
#endif
			if (mem->rtc.cmd & (1 << 7))
			{
				switch (mem->rtc.cmd)
				{
					case 0x86:
						mem->rtc.outbuf[0] = mem->rtc.sr1;
						mem->rtc.sr1 &= ~0xF0;
						mem->rtc.outpos = 0;
						mem->rtc.outlen = 8;
						break;
					case 0xC6:
						mem->rtc.outbuf[0] = mem->rtc.sr2;
						mem->rtc.outpos = 0;
						mem->rtc.outlen = 8;
						break;
					case 0xA6:
					{
						time_t t = time(NULL) + mem->rtc.offset;
						struct tm *tm = localtime(&t);
						mem->rtc.outbuf[0] = BCD(tm->tm_year - 100);
						mem->rtc.outbuf[1] = BCD(tm->tm_mon + 1);
						mem->rtc.outbuf[2] = BCD(tm->tm_mday);
						mem->rtc.outbuf[3] = BCD(tm->tm_wday);
						mem->rtc.outbuf[4] = BCD(tm->tm_hour);
						mem->rtc.outbuf[5] = BCD(tm->tm_min);
						mem->rtc.outbuf[6] = BCD(tm->tm_sec);
#if 0
						printf("RTC read %x %x %x %x %x %x %x\n",
						       mem->rtc.outbuf[0],
						       mem->rtc.outbuf[1],
						       mem->rtc.outbuf[2],
						       mem->rtc.outbuf[3],
						       mem->rtc.outbuf[4],
						       mem->rtc.outbuf[5],
						       mem->rtc.outbuf[6]);
#endif
						mem->rtc.outpos = 0;
						mem->rtc.outlen = 8 * 7;
						break;
					}
					case 0xE6:
					{
						time_t t = time(NULL) + mem->rtc.offset;
						struct tm *tm = localtime(&t);
						mem->rtc.outbuf[0] = BCD(tm->tm_hour);
						mem->rtc.outbuf[1] = BCD(tm->tm_min);
						mem->rtc.outbuf[2] = BCD(tm->tm_sec);
						mem->rtc.outpos = 0;
						mem->rtc.outlen = 8 * 3;
						break;
					}
					case 0x96:
						switch (mem->rtc.sr2 & 0xF)
						{
							case 0x1:
							case 0x5:
								mem->rtc.outbuf[0] = mem->rtc.int1_steady_freq;
								mem->rtc.outpos = 0;
								mem->rtc.outlen = 8;
								break;
							case 0x4:
								mem->rtc.outbuf[0] = mem->rtc.alarm1[0];
								mem->rtc.outbuf[1] = mem->rtc.alarm1[1];
								mem->rtc.outbuf[2] = mem->rtc.alarm1[2];
								mem->rtc.outpos = 0;
								mem->rtc.outlen = 8 * 3;
								break;
							default:
								printf("unknown rtc read sr2 pos: 0x%01" PRIx8 "\n",
								       mem->rtc.sr2 & 0xF);
								mem->rtc.outpos = 0;
								mem->rtc.outlen = 0;
								break;
						}
						break;
					case 0xD6:
						mem->rtc.outbuf[0] = mem->rtc.alarm2[0];
						mem->rtc.outbuf[1] = mem->rtc.alarm2[1];
						mem->rtc.outbuf[2] = mem->rtc.alarm2[2];
						mem->rtc.outpos = 0;
						mem->rtc.outlen = 8 * 3;
						break;
					case 0xB6:
						mem->rtc.outbuf[0] = mem->rtc.car;
						mem->rtc.outpos = 0;
						mem->rtc.outlen = 8;
						break;
					case 0xF6:
						mem->rtc.outbuf[0] = mem->rtc.fr;
						mem->rtc.outpos = 0;
						mem->rtc.outlen = 8;
						break;
					default:
						printf("unknown rtc read cmd: %02" PRIx8 "\n",
						       mem->rtc.cmd);
						break;
				}
			}
			return;
		}
		if (mem->rtc.cmd & (1 << 7))
		{
			printf("rtc write data on read cmd!\n");
			mem->rtc.inbuf = 0;
			return;
		}
#if 0
		printf("rtc byte %02" PRIx8 " for cmd %02" PRIx8 "\n",
		       mem->rtc.inbuf, mem->rtc.cmd);
#endif
		switch (mem->rtc.cmd)
		{
			case 0x06:
				mem->rtc.sr1 = mem->rtc.inbuf & 0x0E;
				break;
			case 0x46:
				mem->rtc.sr2 = mem->rtc.inbuf;
				break;
			case 0x26:
				switch (mem->rtc.wpos)
				{
					case 0:
						/* XXX for unknown reasons, firmware sends something not ok for year (not bcd?)*/
						mem->rtc.tm.tm_year = DAA(mem->rtc.inbuf) + 100;
						break;
					case 1:
						mem->rtc.tm.tm_mon = DAA(mem->rtc.inbuf) - 1;
						break;
					case 2:
						mem->rtc.tm.tm_mday = DAA(mem->rtc.inbuf);
						break;
					case 3:
						mem->rtc.tm.tm_wday = DAA(mem->rtc.inbuf);
						break;
					case 4:
						mem->rtc.tm.tm_hour = DAA(mem->rtc.inbuf);
						break;
					case 5:
						mem->rtc.tm.tm_min = DAA(mem->rtc.inbuf);
						break;
					case 6:
					{
						mem->rtc.tm.tm_sec = DAA(mem->rtc.inbuf);
						time_t cur = time(NULL);
						time_t t = mktime(&mem->rtc.tm);
						mem->rtc.offset = t - cur;
						break;
					}
				}
				mem->rtc.wpos++;
				break;
			case 0x66:
				switch (mem->rtc.wpos)
				{
					case 0:
						mem->rtc.tm.tm_hour = DAA(mem->rtc.inbuf);
						break;
					case 1:
						mem->rtc.tm.tm_min = DAA(mem->rtc.inbuf);
						break;
					case 2:
					{
						mem->rtc.tm.tm_sec = DAA(mem->rtc.inbuf);
						time_t cur = time(NULL);
						time_t rtc_cur = cur + mem->rtc.offset;
						struct tm *cur_tm = localtime(&rtc_cur);
						mem->rtc.tm.tm_year = cur_tm->tm_year;
						mem->rtc.tm.tm_mon = cur_tm->tm_mon;
						mem->rtc.tm.tm_mday = cur_tm->tm_mday;
						mem->rtc.tm.tm_wday = cur_tm->tm_wday; /* not required */
						time_t t = mktime(&mem->rtc.tm);
						mem->rtc.offset = t - cur;
						break;
					}
				}
				break;
			case 0x16:
				switch (mem->rtc.sr2 & 0xF)
				{
					case 0x1:
					case 0x5:
						mem->rtc.int1_steady_freq = mem->rtc.inbuf;
						break;
					case 0x4:
						mem->rtc.alarm1[mem->rtc.wpos++] = mem->rtc.inbuf;
						break;
					default:
						printf("unknown rtc write sr2 pos: 0x%01" PRIx8 " = %02" PRIx8 "\n",
						       mem->rtc.sr2 & 0xF, mem->rtc.inbuf);
						break;
				}
				break;
			case 0x56:
				mem->rtc.alarm2[mem->rtc.wpos++] = mem->rtc.inbuf;
				break;
			case 0x36:
				mem->rtc.car = mem->rtc.inbuf;
				break;
			case 0x76:
				mem->rtc.fr = mem->rtc.inbuf;
				break;
			default:
				printf("unknown rtc write cmd: %02" PRIx8 "\n",
				       mem->rtc.cmd);
				break;
		}
		mem->rtc.inbuf = 0;
		return;
	}
	else
	{
		uint8_t b = 0;
#if 0
		printf("rtc read %u / %u\n", mem->rtc.outpos, mem->rtc.outlen);
#endif
		if (mem->rtc.outpos < mem->rtc.outlen)
		{
			b = mem->rtc.outbuf[mem->rtc.outpos / 8];
			b >>= mem->rtc.outpos % 8;
			b &= 1;
			if (v & (1 << 1))
				mem->rtc.outpos++;
		}
		mem->rtc.outbyte = 0x66 | b;
	}
}

static uint8_t rtc_read(struct mem *mem)
{
#if 0
	printf("[%08" PRIx32 "] rtc read %02" PRIx8 "\n",
	       cpu_get_reg(mem->nds->arm7, CPU_REG_PC), mem->rtc.outbyte);
#endif
	return mem->rtc.outbyte;
}

static void set_arm7_reg8(struct mem *mem, uint32_t addr, uint8_t v)
{
#if 0
	printf("[ARM7] register [%08" PRIx32 "] = %02" PRIx8 "\n", addr, v);
#endif
	switch (addr)
	{
		case MEM_ARM7_REG_IPCSYNC:
			return;
		case MEM_ARM7_REG_IPCSYNC + 1:
			mem->arm7_regs[addr] = v & 0x47;
			if ((v & (1 << 5))
			 && (mem->arm9_regs[MEM_ARM9_REG_IPCSYNC + 1] & (1 << 6)))
				mem_arm9_irq(mem, 1 << 16);
#if 0
			printf("[ARM7] IPCSYNC write 0x%02" PRIx8 "\n", v);
#endif
			return;
		case MEM_ARM7_REG_IPCSYNC + 2:
		case MEM_ARM7_REG_IPCSYNC + 3:
			return;
		case MEM_ARM7_REG_IE:
		case MEM_ARM7_REG_IE + 1:
		case MEM_ARM7_REG_IE + 2:
		case MEM_ARM7_REG_IE + 3:
			mem->arm7_regs[addr] = v;
#if 0
			printf("[ARM7] IE 0x%08" PRIx32 "\n", mem_arm7_get_reg32(mem, MEM_ARM7_REG_IE));
#endif
			cpu_update_irq_state(mem->nds->arm7);
			return;
		case MEM_ARM7_REG_IME:
		case MEM_ARM7_REG_IME + 1:
		case MEM_ARM7_REG_IME + 2:
		case MEM_ARM7_REG_IME + 3:
		case MEM_ARM7_REG_POSTFLG:
		case MEM_ARM7_REG_TM0CNT_L:
		case MEM_ARM7_REG_TM0CNT_L + 1:
		case MEM_ARM7_REG_TM0CNT_H + 1:
		case MEM_ARM7_REG_TM1CNT_L:
		case MEM_ARM7_REG_TM1CNT_L + 1:
		case MEM_ARM7_REG_TM1CNT_H + 1:
		case MEM_ARM7_REG_TM2CNT_L:
		case MEM_ARM7_REG_TM2CNT_L + 1:
		case MEM_ARM7_REG_TM2CNT_H + 1:
		case MEM_ARM7_REG_TM3CNT_L:
		case MEM_ARM7_REG_TM3CNT_L + 1:
		case MEM_ARM7_REG_TM3CNT_H + 1:
		case MEM_ARM7_REG_SOUNDBIAS:
		case MEM_ARM7_REG_SOUNDBIAS + 1:
		case MEM_ARM7_REG_SOUNDBIAS + 2:
		case MEM_ARM7_REG_SOUNDBIAS + 3:
		case MEM_ARM7_REG_DMA0SAD:
		case MEM_ARM7_REG_DMA0SAD + 1:
		case MEM_ARM7_REG_DMA0SAD + 2:
		case MEM_ARM7_REG_DMA0SAD + 3:
		case MEM_ARM7_REG_DMA0DAD:
		case MEM_ARM7_REG_DMA0DAD + 1:
		case MEM_ARM7_REG_DMA0DAD + 2:
		case MEM_ARM7_REG_DMA0DAD + 3:
		case MEM_ARM7_REG_DMA0CNT_L:
		case MEM_ARM7_REG_DMA0CNT_L + 1:
		case MEM_ARM7_REG_DMA0CNT_H:
		case MEM_ARM7_REG_DMA1SAD:
		case MEM_ARM7_REG_DMA1SAD + 1:
		case MEM_ARM7_REG_DMA1SAD + 2:
		case MEM_ARM7_REG_DMA1SAD + 3:
		case MEM_ARM7_REG_DMA1DAD:
		case MEM_ARM7_REG_DMA1DAD + 1:
		case MEM_ARM7_REG_DMA1DAD + 2:
		case MEM_ARM7_REG_DMA1DAD + 3:
		case MEM_ARM7_REG_DMA1CNT_L:
		case MEM_ARM7_REG_DMA1CNT_L + 1:
		case MEM_ARM7_REG_DMA1CNT_H:
		case MEM_ARM7_REG_DMA2SAD:
		case MEM_ARM7_REG_DMA2SAD + 1:
		case MEM_ARM7_REG_DMA2SAD + 2:
		case MEM_ARM7_REG_DMA2SAD + 3:
		case MEM_ARM7_REG_DMA2DAD:
		case MEM_ARM7_REG_DMA2DAD + 1:
		case MEM_ARM7_REG_DMA2DAD + 2:
		case MEM_ARM7_REG_DMA2DAD + 3:
		case MEM_ARM7_REG_DMA2CNT_L:
		case MEM_ARM7_REG_DMA2CNT_L + 1:
		case MEM_ARM7_REG_DMA2CNT_H:
		case MEM_ARM7_REG_DMA3SAD:
		case MEM_ARM7_REG_DMA3SAD + 1:
		case MEM_ARM7_REG_DMA3SAD + 2:
		case MEM_ARM7_REG_DMA3SAD + 3:
		case MEM_ARM7_REG_DMA3DAD:
		case MEM_ARM7_REG_DMA3DAD + 1:
		case MEM_ARM7_REG_DMA3DAD + 2:
		case MEM_ARM7_REG_DMA3DAD + 3:
		case MEM_ARM7_REG_DMA3CNT_L:
		case MEM_ARM7_REG_DMA3CNT_L  +1:
		case MEM_ARM7_REG_DMA3CNT_H:
		case MEM_ARM7_REG_POWCNT2:
		case MEM_ARM7_REG_POWCNT2 + 1:
		case MEM_ARM7_REG_POWCNT2 + 2:
		case MEM_ARM7_REG_POWCNT2 + 3:
		case MEM_ARM7_REG_RCNT:
		case MEM_ARM7_REG_RCNT + 1:
		case MEM_ARM7_REG_SOUNDCNT:
		case MEM_ARM7_REG_SOUNDCNT + 1:
		case MEM_ARM7_REG_SOUNDCNT + 2:
		case MEM_ARM7_REG_SOUNDCNT + 3:
		case MEM_ARM7_REG_WIFIWAITCNT:
		case MEM_ARM7_REG_WIFIWAITCNT + 1:
		case MEM_ARM7_REG_SNDCAP0CNT:
		case MEM_ARM7_REG_SNDCAP0DAD:
		case MEM_ARM7_REG_SNDCAP0DAD + 1:
		case MEM_ARM7_REG_SNDCAP0DAD + 2:
		case MEM_ARM7_REG_SNDCAP0DAD + 3:
		case MEM_ARM7_REG_SNDCAP0LEN:
		case MEM_ARM7_REG_SNDCAP0LEN + 1:
		case MEM_ARM7_REG_SNDCAP0LEN + 2:
		case MEM_ARM7_REG_SNDCAP0LEN + 3:
		case MEM_ARM7_REG_SNDCAP1CNT:
		case MEM_ARM7_REG_SNDCAP1DAD:
		case MEM_ARM7_REG_SNDCAP1DAD + 1:
		case MEM_ARM7_REG_SNDCAP1DAD + 2:
		case MEM_ARM7_REG_SNDCAP1DAD + 3:
		case MEM_ARM7_REG_SNDCAP1LEN:
		case MEM_ARM7_REG_SNDCAP1LEN + 1:
		case MEM_ARM7_REG_SNDCAP1LEN + 2:
		case MEM_ARM7_REG_SNDCAP1LEN + 3:
			mem->arm7_regs[addr] = v;
			return;
		case MEM_ARM7_REG_SOUNDXCNT(0):
		case MEM_ARM7_REG_SOUNDXCNT(0) + 1:
		case MEM_ARM7_REG_SOUNDXCNT(0) + 2:
		case MEM_ARM7_REG_SOUNDXCNT(1):
		case MEM_ARM7_REG_SOUNDXCNT(1) + 1:
		case MEM_ARM7_REG_SOUNDXCNT(1) + 2:
		case MEM_ARM7_REG_SOUNDXCNT(2):
		case MEM_ARM7_REG_SOUNDXCNT(2) + 1:
		case MEM_ARM7_REG_SOUNDXCNT(2) + 2:
		case MEM_ARM7_REG_SOUNDXCNT(3):
		case MEM_ARM7_REG_SOUNDXCNT(3) + 1:
		case MEM_ARM7_REG_SOUNDXCNT(3) + 2:
		case MEM_ARM7_REG_SOUNDXCNT(4):
		case MEM_ARM7_REG_SOUNDXCNT(4) + 1:
		case MEM_ARM7_REG_SOUNDXCNT(4) + 2:
		case MEM_ARM7_REG_SOUNDXCNT(5):
		case MEM_ARM7_REG_SOUNDXCNT(5) + 1:
		case MEM_ARM7_REG_SOUNDXCNT(5) + 2:
		case MEM_ARM7_REG_SOUNDXCNT(6):
		case MEM_ARM7_REG_SOUNDXCNT(6) + 1:
		case MEM_ARM7_REG_SOUNDXCNT(6) + 2:
		case MEM_ARM7_REG_SOUNDXCNT(7):
		case MEM_ARM7_REG_SOUNDXCNT(7) + 1:
		case MEM_ARM7_REG_SOUNDXCNT(7) + 2:
		case MEM_ARM7_REG_SOUNDXCNT(8):
		case MEM_ARM7_REG_SOUNDXCNT(8) + 1:
		case MEM_ARM7_REG_SOUNDXCNT(8) + 2:
		case MEM_ARM7_REG_SOUNDXCNT(9):
		case MEM_ARM7_REG_SOUNDXCNT(9) + 1:
		case MEM_ARM7_REG_SOUNDXCNT(9) + 2:
		case MEM_ARM7_REG_SOUNDXCNT(10):
		case MEM_ARM7_REG_SOUNDXCNT(10) + 1:
		case MEM_ARM7_REG_SOUNDXCNT(10) + 2:
		case MEM_ARM7_REG_SOUNDXCNT(11):
		case MEM_ARM7_REG_SOUNDXCNT(11) + 1:
		case MEM_ARM7_REG_SOUNDXCNT(11) + 2:
		case MEM_ARM7_REG_SOUNDXCNT(12):
		case MEM_ARM7_REG_SOUNDXCNT(12) + 1:
		case MEM_ARM7_REG_SOUNDXCNT(12) + 2:
		case MEM_ARM7_REG_SOUNDXCNT(13):
		case MEM_ARM7_REG_SOUNDXCNT(13) + 1:
		case MEM_ARM7_REG_SOUNDXCNT(13) + 2:
		case MEM_ARM7_REG_SOUNDXCNT(14):
		case MEM_ARM7_REG_SOUNDXCNT(14) + 1:
		case MEM_ARM7_REG_SOUNDXCNT(14) + 2:
		case MEM_ARM7_REG_SOUNDXCNT(15):
		case MEM_ARM7_REG_SOUNDXCNT(15) + 1:
		case MEM_ARM7_REG_SOUNDXCNT(15) + 2:
		case MEM_ARM7_REG_SOUNDXSAD(0):
		case MEM_ARM7_REG_SOUNDXSAD(0) + 1:
		case MEM_ARM7_REG_SOUNDXSAD(0) + 2:
		case MEM_ARM7_REG_SOUNDXSAD(0) + 3:
		case MEM_ARM7_REG_SOUNDXSAD(1):
		case MEM_ARM7_REG_SOUNDXSAD(1) + 1:
		case MEM_ARM7_REG_SOUNDXSAD(1) + 2:
		case MEM_ARM7_REG_SOUNDXSAD(1) + 3:
		case MEM_ARM7_REG_SOUNDXSAD(2):
		case MEM_ARM7_REG_SOUNDXSAD(2) + 1:
		case MEM_ARM7_REG_SOUNDXSAD(2) + 2:
		case MEM_ARM7_REG_SOUNDXSAD(2) + 3:
		case MEM_ARM7_REG_SOUNDXSAD(3):
		case MEM_ARM7_REG_SOUNDXSAD(3) + 1:
		case MEM_ARM7_REG_SOUNDXSAD(3) + 2:
		case MEM_ARM7_REG_SOUNDXSAD(3) + 3:
		case MEM_ARM7_REG_SOUNDXSAD(4):
		case MEM_ARM7_REG_SOUNDXSAD(4) + 1:
		case MEM_ARM7_REG_SOUNDXSAD(4) + 2:
		case MEM_ARM7_REG_SOUNDXSAD(4) + 3:
		case MEM_ARM7_REG_SOUNDXSAD(5):
		case MEM_ARM7_REG_SOUNDXSAD(5) + 1:
		case MEM_ARM7_REG_SOUNDXSAD(5) + 2:
		case MEM_ARM7_REG_SOUNDXSAD(5) + 3:
		case MEM_ARM7_REG_SOUNDXSAD(6):
		case MEM_ARM7_REG_SOUNDXSAD(6) + 1:
		case MEM_ARM7_REG_SOUNDXSAD(6) + 2:
		case MEM_ARM7_REG_SOUNDXSAD(6) + 3:
		case MEM_ARM7_REG_SOUNDXSAD(7):
		case MEM_ARM7_REG_SOUNDXSAD(7) + 1:
		case MEM_ARM7_REG_SOUNDXSAD(7) + 2:
		case MEM_ARM7_REG_SOUNDXSAD(7) + 3:
		case MEM_ARM7_REG_SOUNDXSAD(8):
		case MEM_ARM7_REG_SOUNDXSAD(8) + 1:
		case MEM_ARM7_REG_SOUNDXSAD(8) + 2:
		case MEM_ARM7_REG_SOUNDXSAD(8) + 3:
		case MEM_ARM7_REG_SOUNDXSAD(9):
		case MEM_ARM7_REG_SOUNDXSAD(9) + 1:
		case MEM_ARM7_REG_SOUNDXSAD(9) + 2:
		case MEM_ARM7_REG_SOUNDXSAD(9) + 3:
		case MEM_ARM7_REG_SOUNDXSAD(10):
		case MEM_ARM7_REG_SOUNDXSAD(10) + 1:
		case MEM_ARM7_REG_SOUNDXSAD(10) + 2:
		case MEM_ARM7_REG_SOUNDXSAD(10) + 3:
		case MEM_ARM7_REG_SOUNDXSAD(11):
		case MEM_ARM7_REG_SOUNDXSAD(11) + 1:
		case MEM_ARM7_REG_SOUNDXSAD(11) + 2:
		case MEM_ARM7_REG_SOUNDXSAD(11) + 3:
		case MEM_ARM7_REG_SOUNDXSAD(12):
		case MEM_ARM7_REG_SOUNDXSAD(12) + 1:
		case MEM_ARM7_REG_SOUNDXSAD(12) + 2:
		case MEM_ARM7_REG_SOUNDXSAD(12) + 3:
		case MEM_ARM7_REG_SOUNDXSAD(13):
		case MEM_ARM7_REG_SOUNDXSAD(13) + 1:
		case MEM_ARM7_REG_SOUNDXSAD(13) + 2:
		case MEM_ARM7_REG_SOUNDXSAD(13) + 3:
		case MEM_ARM7_REG_SOUNDXSAD(14):
		case MEM_ARM7_REG_SOUNDXSAD(14) + 1:
		case MEM_ARM7_REG_SOUNDXSAD(14) + 2:
		case MEM_ARM7_REG_SOUNDXSAD(14) + 3:
		case MEM_ARM7_REG_SOUNDXSAD(15):
		case MEM_ARM7_REG_SOUNDXSAD(15) + 1:
		case MEM_ARM7_REG_SOUNDXSAD(15) + 2:
		case MEM_ARM7_REG_SOUNDXSAD(15) + 3:
		case MEM_ARM7_REG_SOUNDXTMR(0):
		case MEM_ARM7_REG_SOUNDXTMR(0) + 1:
		case MEM_ARM7_REG_SOUNDXTMR(1):
		case MEM_ARM7_REG_SOUNDXTMR(1) + 1:
		case MEM_ARM7_REG_SOUNDXTMR(2):
		case MEM_ARM7_REG_SOUNDXTMR(2) + 1:
		case MEM_ARM7_REG_SOUNDXTMR(3):
		case MEM_ARM7_REG_SOUNDXTMR(3) + 1:
		case MEM_ARM7_REG_SOUNDXTMR(4):
		case MEM_ARM7_REG_SOUNDXTMR(4) + 1:
		case MEM_ARM7_REG_SOUNDXTMR(5):
		case MEM_ARM7_REG_SOUNDXTMR(5) + 1:
		case MEM_ARM7_REG_SOUNDXTMR(6):
		case MEM_ARM7_REG_SOUNDXTMR(6) + 1:
		case MEM_ARM7_REG_SOUNDXTMR(7):
		case MEM_ARM7_REG_SOUNDXTMR(7) + 1:
		case MEM_ARM7_REG_SOUNDXTMR(8):
		case MEM_ARM7_REG_SOUNDXTMR(8) + 1:
		case MEM_ARM7_REG_SOUNDXTMR(9):
		case MEM_ARM7_REG_SOUNDXTMR(9) + 1:
		case MEM_ARM7_REG_SOUNDXTMR(10):
		case MEM_ARM7_REG_SOUNDXTMR(10) + 1:
		case MEM_ARM7_REG_SOUNDXTMR(11):
		case MEM_ARM7_REG_SOUNDXTMR(11) + 1:
		case MEM_ARM7_REG_SOUNDXTMR(12):
		case MEM_ARM7_REG_SOUNDXTMR(12) + 1:
		case MEM_ARM7_REG_SOUNDXTMR(13):
		case MEM_ARM7_REG_SOUNDXTMR(13) + 1:
		case MEM_ARM7_REG_SOUNDXTMR(14):
		case MEM_ARM7_REG_SOUNDXTMR(14) + 1:
		case MEM_ARM7_REG_SOUNDXTMR(15):
		case MEM_ARM7_REG_SOUNDXTMR(15) + 1:
		case MEM_ARM7_REG_SOUNDXPNT(0):
		case MEM_ARM7_REG_SOUNDXPNT(0) + 1:
		case MEM_ARM7_REG_SOUNDXPNT(1):
		case MEM_ARM7_REG_SOUNDXPNT(1) + 1:
		case MEM_ARM7_REG_SOUNDXPNT(2):
		case MEM_ARM7_REG_SOUNDXPNT(2) + 1:
		case MEM_ARM7_REG_SOUNDXPNT(3):
		case MEM_ARM7_REG_SOUNDXPNT(3) + 1:
		case MEM_ARM7_REG_SOUNDXPNT(4):
		case MEM_ARM7_REG_SOUNDXPNT(4) + 1:
		case MEM_ARM7_REG_SOUNDXPNT(5):
		case MEM_ARM7_REG_SOUNDXPNT(5) + 1:
		case MEM_ARM7_REG_SOUNDXPNT(6):
		case MEM_ARM7_REG_SOUNDXPNT(6) + 1:
		case MEM_ARM7_REG_SOUNDXPNT(7):
		case MEM_ARM7_REG_SOUNDXPNT(7) + 1:
		case MEM_ARM7_REG_SOUNDXPNT(8):
		case MEM_ARM7_REG_SOUNDXPNT(8) + 1:
		case MEM_ARM7_REG_SOUNDXPNT(9):
		case MEM_ARM7_REG_SOUNDXPNT(9) + 1:
		case MEM_ARM7_REG_SOUNDXPNT(10):
		case MEM_ARM7_REG_SOUNDXPNT(10) + 1:
		case MEM_ARM7_REG_SOUNDXPNT(11):
		case MEM_ARM7_REG_SOUNDXPNT(11) + 1:
		case MEM_ARM7_REG_SOUNDXPNT(12):
		case MEM_ARM7_REG_SOUNDXPNT(12) + 1:
		case MEM_ARM7_REG_SOUNDXPNT(13):
		case MEM_ARM7_REG_SOUNDXPNT(13) + 1:
		case MEM_ARM7_REG_SOUNDXPNT(14):
		case MEM_ARM7_REG_SOUNDXPNT(14) + 1:
		case MEM_ARM7_REG_SOUNDXPNT(15):
		case MEM_ARM7_REG_SOUNDXPNT(15) + 1:
		case MEM_ARM7_REG_SOUNDXLEN(0):
		case MEM_ARM7_REG_SOUNDXLEN(0) + 1:
		case MEM_ARM7_REG_SOUNDXLEN(0) + 2:
		case MEM_ARM7_REG_SOUNDXLEN(0) + 3:
		case MEM_ARM7_REG_SOUNDXLEN(1):
		case MEM_ARM7_REG_SOUNDXLEN(1) + 1:
		case MEM_ARM7_REG_SOUNDXLEN(1) + 2:
		case MEM_ARM7_REG_SOUNDXLEN(1) + 3:
		case MEM_ARM7_REG_SOUNDXLEN(2):
		case MEM_ARM7_REG_SOUNDXLEN(2) + 1:
		case MEM_ARM7_REG_SOUNDXLEN(2) + 2:
		case MEM_ARM7_REG_SOUNDXLEN(2) + 3:
		case MEM_ARM7_REG_SOUNDXLEN(3):
		case MEM_ARM7_REG_SOUNDXLEN(3) + 1:
		case MEM_ARM7_REG_SOUNDXLEN(3) + 2:
		case MEM_ARM7_REG_SOUNDXLEN(3) + 3:
		case MEM_ARM7_REG_SOUNDXLEN(4):
		case MEM_ARM7_REG_SOUNDXLEN(4) + 1:
		case MEM_ARM7_REG_SOUNDXLEN(4) + 2:
		case MEM_ARM7_REG_SOUNDXLEN(4) + 3:
		case MEM_ARM7_REG_SOUNDXLEN(5):
		case MEM_ARM7_REG_SOUNDXLEN(5) + 1:
		case MEM_ARM7_REG_SOUNDXLEN(5) + 2:
		case MEM_ARM7_REG_SOUNDXLEN(5) + 3:
		case MEM_ARM7_REG_SOUNDXLEN(6):
		case MEM_ARM7_REG_SOUNDXLEN(6) + 1:
		case MEM_ARM7_REG_SOUNDXLEN(6) + 2:
		case MEM_ARM7_REG_SOUNDXLEN(6) + 3:
		case MEM_ARM7_REG_SOUNDXLEN(7):
		case MEM_ARM7_REG_SOUNDXLEN(7) + 1:
		case MEM_ARM7_REG_SOUNDXLEN(7) + 2:
		case MEM_ARM7_REG_SOUNDXLEN(7) + 3:
		case MEM_ARM7_REG_SOUNDXLEN(8):
		case MEM_ARM7_REG_SOUNDXLEN(8) + 1:
		case MEM_ARM7_REG_SOUNDXLEN(8) + 2:
		case MEM_ARM7_REG_SOUNDXLEN(8) + 3:
		case MEM_ARM7_REG_SOUNDXLEN(9):
		case MEM_ARM7_REG_SOUNDXLEN(9) + 1:
		case MEM_ARM7_REG_SOUNDXLEN(9) + 2:
		case MEM_ARM7_REG_SOUNDXLEN(9) + 3:
		case MEM_ARM7_REG_SOUNDXLEN(10):
		case MEM_ARM7_REG_SOUNDXLEN(10) + 1:
		case MEM_ARM7_REG_SOUNDXLEN(10) + 2:
		case MEM_ARM7_REG_SOUNDXLEN(10) + 3:
		case MEM_ARM7_REG_SOUNDXLEN(11):
		case MEM_ARM7_REG_SOUNDXLEN(11) + 1:
		case MEM_ARM7_REG_SOUNDXLEN(11) + 2:
		case MEM_ARM7_REG_SOUNDXLEN(11) + 3:
		case MEM_ARM7_REG_SOUNDXLEN(12):
		case MEM_ARM7_REG_SOUNDXLEN(12) + 1:
		case MEM_ARM7_REG_SOUNDXLEN(12) + 2:
		case MEM_ARM7_REG_SOUNDXLEN(12) + 3:
		case MEM_ARM7_REG_SOUNDXLEN(13):
		case MEM_ARM7_REG_SOUNDXLEN(13) + 1:
		case MEM_ARM7_REG_SOUNDXLEN(13) + 2:
		case MEM_ARM7_REG_SOUNDXLEN(13) + 3:
		case MEM_ARM7_REG_SOUNDXLEN(14):
		case MEM_ARM7_REG_SOUNDXLEN(14) + 1:
		case MEM_ARM7_REG_SOUNDXLEN(14) + 2:
		case MEM_ARM7_REG_SOUNDXLEN(14) + 3:
		case MEM_ARM7_REG_SOUNDXLEN(15):
		case MEM_ARM7_REG_SOUNDXLEN(15) + 1:
		case MEM_ARM7_REG_SOUNDXLEN(15) + 2:
		case MEM_ARM7_REG_SOUNDXLEN(15) + 3:
#if 0
			printf("[ARM7] SND[%08" PRIx32 "] = %02" PRIx8 "\n", addr, v);
#endif
			mem->arm7_regs[addr] = v;
			return;
		case MEM_ARM7_REG_SOUNDXCNT(0) + 3:
		case MEM_ARM7_REG_SOUNDXCNT(1) + 3:
		case MEM_ARM7_REG_SOUNDXCNT(2) + 3:
		case MEM_ARM7_REG_SOUNDXCNT(3) + 3:
		case MEM_ARM7_REG_SOUNDXCNT(4) + 3:
		case MEM_ARM7_REG_SOUNDXCNT(5) + 3:
		case MEM_ARM7_REG_SOUNDXCNT(6) + 3:
		case MEM_ARM7_REG_SOUNDXCNT(7) + 3:
		case MEM_ARM7_REG_SOUNDXCNT(8) + 3:
		case MEM_ARM7_REG_SOUNDXCNT(9) + 3:
		case MEM_ARM7_REG_SOUNDXCNT(10) + 3:
		case MEM_ARM7_REG_SOUNDXCNT(11) + 3:
		case MEM_ARM7_REG_SOUNDXCNT(12) + 3:
		case MEM_ARM7_REG_SOUNDXCNT(13) + 3:
		case MEM_ARM7_REG_SOUNDXCNT(14) + 3:
		case MEM_ARM7_REG_SOUNDXCNT(15) + 3:
		{
#if 0
			printf("[ARM7] SND[%08" PRIx32 "] = %02" PRIx8 "\n", addr, v);
#endif
			bool start = ((v & (1 << 7)) != (mem->arm7_regs[addr] & (1 << 7)));
			mem->arm7_regs[addr] = v;
			if (start)
				apu_start_channel(mem->nds->apu, (addr - (MEM_ARM7_REG_SOUNDXCNT(0) + 3)) / 0x10);
			return;
		}
		case MEM_ARM7_REG_ROMCTRL:
		case MEM_ARM7_REG_ROMCTRL + 1:
		case MEM_ARM7_REG_ROMCMD:
		case MEM_ARM7_REG_ROMCMD + 1:
		case MEM_ARM7_REG_ROMCMD + 2:
		case MEM_ARM7_REG_ROMCMD + 3:
		case MEM_ARM7_REG_ROMCMD + 4:
		case MEM_ARM7_REG_ROMCMD + 5:
		case MEM_ARM7_REG_ROMCMD + 6:
		case MEM_ARM7_REG_ROMCMD + 7:
			mem->arm9_regs[addr] = v;
			return;
		case MEM_ARM7_REG_DISPSTAT:
#if 0
			printf("[ARM7] DISPSTAT[%08" PRIx32 "] = %02" PRIx8 "\n",
			       addr, v);
#endif
			mem->arm7_regs[addr] = v & 0xB8;
			return;
		case MEM_ARM7_REG_DISPSTAT + 1:
#if 0
			printf("[ARM7] DISPSTAT[%08" PRIx32 "] = %02" PRIx8 "\n",
			       addr, v);
			mem->arm7_regs[addr] = v;
			return;
#endif
		case MEM_ARM7_REG_AUXSPICNT:
#if 0
			printf("[ARM7] AUXSPICNT[%08" PRIx32 "] = %02" PRIx8 "\n",
			       addr, v);
#endif
			mem->arm9_regs[addr] = v & ~(1 << 7);
			return;
		case MEM_ARM7_REG_AUXSPICNT + 1:
#if 0
			printf("[ARM7] AUXSPICNT[%08" PRIx32 "] = %02" PRIx8 "\n",
			       addr, v);
#endif
			mem->arm9_regs[addr] = v;
			return;
		case MEM_ARM7_REG_AUXSPIDATA:
			auxspi_write(mem, v);
			return;
		case MEM_ARM7_REG_ROMDATA:
		case MEM_ARM7_REG_ROMDATA + 1:
		case MEM_ARM7_REG_ROMDATA + 2:
		case MEM_ARM7_REG_ROMDATA + 3:
#if 0
			printf("[ARM7] MBC write %02" PRIx8 "\n", v);
#endif
			mbc_write(mem->mbc, v);
			return;
		case MEM_ARM7_REG_ROMCTRL + 2:
			mem->arm9_regs[addr] = (mem->arm9_regs[addr] & (1 << 7))
			                     | (v & ~(1 << 7));
			return;
		case MEM_ARM7_REG_ROMCTRL + 3:
			mem->arm9_regs[addr] = v;
			if (v & 0x80)
			{
#if 0
				printf("[ARM7] MBC cmd %02" PRIx8 "\n", v);
#endif
				mbc_cmd(mem->mbc);
			}
			return;
		case MEM_ARM7_REG_IF:
		case MEM_ARM7_REG_IF + 1:
		case MEM_ARM7_REG_IF + 3:
			mem->arm7_regs[addr] &= ~v;
			if (v)
				cpu_update_irq_state(mem->nds->arm7);
			return;
		case MEM_ARM7_REG_IF + 2:
			v &= ~(1 << 5);
			mem->arm7_regs[addr] &= ~v;
			if (v)
				cpu_update_irq_state(mem->nds->arm7);
			return;
		case MEM_ARM7_REG_TM0CNT_H:
			arm7_timer_control(mem, 0, v);
			return;
		case MEM_ARM7_REG_TM1CNT_H:
			arm7_timer_control(mem, 1, v);
			return;
		case MEM_ARM7_REG_TM2CNT_H:
			arm7_timer_control(mem, 2, v);
			return;
		case MEM_ARM7_REG_TM3CNT_H:
			arm7_timer_control(mem, 3, v);
			return;
		case MEM_ARM7_REG_SPICNT:
#if 0
			printf("[ARM7] [%08" PRIx32 "] SPICNT[%08" PRIx32 "] = %02" PRIx8 "\n",
			       cpu_get_reg(mem->nds->arm7, CPU_REG_PC), addr, v);
#endif
			mem->arm7_regs[addr] = v & ~(1 << 7);
			return;
		case MEM_ARM7_REG_SPICNT + 1:
#if 0
			printf("[ARM7] [%08" PRIx32 "] SPICNT[%08" PRIx32 "] = %02" PRIx8 "\n",
			       cpu_get_reg(mem->nds->arm7, CPU_REG_PC), addr, v);
#endif
			mem->arm7_regs[addr] = v;
			return;
		case MEM_ARM7_REG_SPIDATA:
			spi_write(mem, v);
			return;
		case MEM_ARM7_REG_HALTCNT:
			switch ((v >> 6) & 0x3)
			{
				case 0:
					return;
				case 1:
					assert(!"GBA mode not supported");
					return;
				case 2:
					mem->nds->arm7->state = CPU_STATE_HALT;
					cpu_update_irq_state(mem->nds->arm7);
					return;
				case 3:
					mem->nds->arm7->state = CPU_STATE_STOP;
					cpu_update_irq_state(mem->nds->arm7);
					return;
			}
			return;
		case MEM_ARM7_REG_BIOSPROT:
		case MEM_ARM7_REG_BIOSPROT + 1:
		case MEM_ARM7_REG_BIOSPROT + 2:
			if (!mem->biosprot)
				mem->arm7_regs[addr] = v;
			return;
		case MEM_ARM7_REG_BIOSPROT + 3:
			if (!mem->biosprot)
				mem->arm7_regs[addr] = v;
			mem->biosprot = 1;
			return;
		case MEM_ARM7_REG_RTC:
			rtc_write(mem, v);
			return;
		case MEM_ARM7_REG_DMA0CNT_H + 1:
			mem->arm7_regs[addr] = v;
			arm7_dma_control(mem, 0);
			return;
		case MEM_ARM7_REG_DMA1CNT_H + 1:
			mem->arm7_regs[addr] = v;
			arm7_dma_control(mem, 1);
			return;
		case MEM_ARM7_REG_DMA2CNT_H + 1:
			mem->arm7_regs[addr] = v;
			arm7_dma_control(mem, 2);
			return;
		case MEM_ARM7_REG_DMA3CNT_H + 1:
			mem->arm7_regs[addr] = v;
			arm7_dma_control(mem, 3);
			return;
		case MEM_ARM7_REG_SPIDATA + 1:
		case MEM_ARM7_REG_RTC + 1:
		case MEM_ARM7_REG_RTC + 2:
		case MEM_ARM7_REG_RTC + 3:
		case MEM_ARM7_REG_WRAMSTAT:
		case MEM_ARM7_REG_SIODATA32:
		case MEM_ARM7_REG_SIODATA32 + 1:
		case MEM_ARM7_REG_SIODATA32 + 2:
		case MEM_ARM7_REG_SIODATA32 + 3:
		case MEM_ARM7_REG_SIOCNT:
		case MEM_ARM7_REG_SIOCNT + 1:
		case MEM_ARM7_REG_SIOCNT + 2:
		case MEM_ARM7_REG_SIOCNT + 3:
		case MEM_ARM7_REG_EXMEMSTAT:
		case MEM_ARM7_REG_EXMEMSTAT + 1:
		case MEM_ARM7_REG_IPCFIFOCNT + 2:
		case MEM_ARM7_REG_IPCFIFOCNT + 3:
		case MEM_ARM7_REG_AUXSPIDATA + 1:
			return;
		case MEM_ARM7_REG_KEYCNT:
		case MEM_ARM7_REG_KEYCNT + 1:
			mem->arm7_regs[addr] = v;
			nds_test_keypad_int(mem->nds);
			return;
		case MEM_ARM7_REG_IPCFIFOCNT:
#if 0
			printf("[ARM7] FIFOCNT[0] write %02" PRIx8 "\n", v);
#endif
			if (v & (1 << 3))
			{
#if 0
				printf("[ARM9] FIFO clean\n");
#endif
				mem->arm9_fifo.len = 0;
				mem->arm9_fifo.latch[0] = 0;
				mem->arm9_fifo.latch[1] = 0;
				mem->arm9_fifo.latch[2] = 0;
				mem->arm9_fifo.latch[3] = 0;
			}
			mem->arm7_regs[addr] &= ~(1 << 2);
			mem->arm7_regs[addr] |= v & (1 << 2);
			return;
		case MEM_ARM7_REG_IPCFIFOCNT + 1:
#if 0
			printf("[ARM7] FIFOCNT[1] write %02" PRIx8 "\n", v);
#endif
			mem->arm7_regs[addr] &= ~0x84;
			mem->arm7_regs[addr] |= v & 0x84;
			if (v & (1 << 6))
				mem->arm7_regs[addr] &= ~(1 << 6);
			return;
		case MEM_ARM7_REG_IPCFIFOSEND:
		case MEM_ARM7_REG_IPCFIFOSEND + 1:
		case MEM_ARM7_REG_IPCFIFOSEND + 2:
		case MEM_ARM7_REG_IPCFIFOSEND + 3:
			if (!(mem->arm9_regs[MEM_ARM9_REG_IPCFIFOCNT + 1] & (1 << 7)))
				return;
			if (mem->arm9_fifo.len == 64)
			{
#if 0
				printf("[ARM9] FIFO full\n");
#endif
				mem->arm7_regs[MEM_ARM7_REG_IPCFIFOCNT + 1] |= (1 << 6);
				return;
			}
			mem->arm9_fifo.data[(mem->arm9_fifo.pos + mem->arm9_fifo.len) % 64] = v;
			mem->arm9_fifo.len++;
			if (mem->arm9_fifo.len == 4
			 && (mem->arm9_regs[MEM_ARM9_REG_IPCFIFOCNT + 1] & (1 << 2)))
				mem_arm9_irq(mem, 1 << 18);
#if 0
			printf("[ARM7] IPCFIFO write (now %" PRIu8 ")\n", mem->arm9_fifo.len);
#endif
			return;
		case MEM_ARM7_REG_ROMSEED0_L:
		case MEM_ARM7_REG_ROMSEED0_L + 1:
		case MEM_ARM7_REG_ROMSEED0_L + 2:
		case MEM_ARM7_REG_ROMSEED0_L + 3:
		case MEM_ARM7_REG_ROMSEED1_L:
		case MEM_ARM7_REG_ROMSEED1_L + 1:
		case MEM_ARM7_REG_ROMSEED1_L + 2:
		case MEM_ARM7_REG_ROMSEED1_L + 3:
		case MEM_ARM7_REG_ROMSEED0_H:
		case MEM_ARM7_REG_ROMSEED0_H + 1:
		case MEM_ARM7_REG_ROMSEED1_H:
		case MEM_ARM7_REG_ROMSEED1_H + 1:
			return;
		case MEM_ARM7_REG_W_BB_CNT:
		case MEM_ARM7_REG_W_BB_WRITE:
		case MEM_ARM7_REG_W_BB_WRITE + 1:
		case MEM_ARM7_REG_W_BB_MODE:
		case MEM_ARM7_REG_W_BB_MODE + 1:
		case MEM_ARM7_REG_W_BB_POWER:
		case MEM_ARM7_REG_W_BB_POWER + 1:
		case MEM_ARM7_REG_W_MACADDR_0:
		case MEM_ARM7_REG_W_MACADDR_0 + 1:
		case MEM_ARM7_REG_W_MACADDR_1:
		case MEM_ARM7_REG_W_MACADDR_1 + 1:
		case MEM_ARM7_REG_W_MACADDR_2:
		case MEM_ARM7_REG_W_MACADDR_2 + 1:
		case MEM_ARM7_REG_W_BSSID_0:
		case MEM_ARM7_REG_W_BSSID_0 + 1:
		case MEM_ARM7_REG_W_BSSID_1:
		case MEM_ARM7_REG_W_BSSID_1 + 1:
		case MEM_ARM7_REG_W_BSSID_2:
		case MEM_ARM7_REG_W_BSSID_2 + 1:
		case MEM_ARM7_REG_W_TX_RETRYLIMIT:
		case MEM_ARM7_REG_W_TX_RETRYLIMIT + 1:
		case MEM_ARM7_REG_W_CONFIG_140:
		case MEM_ARM7_REG_W_CONFIG_140 + 1:
		case MEM_ARM7_REG_W_CONFIG_142:
		case MEM_ARM7_REG_W_CONFIG_142 + 1:
		case MEM_ARM7_REG_W_CONFIG_144:
		case MEM_ARM7_REG_W_CONFIG_146:
		case MEM_ARM7_REG_W_CONFIG_148:
		case MEM_ARM7_REG_W_CONFIG_14A:
		case MEM_ARM7_REG_W_CONFIG_14C:
		case MEM_ARM7_REG_W_CONFIG_14C + 1:
		case MEM_ARM7_REG_W_CONFIG_150 + 1:
		case MEM_ARM7_REG_W_BEACONINT:
		case MEM_ARM7_REG_W_RF_DATA1:
		case MEM_ARM7_REG_W_RF_DATA1 + 1:
		case MEM_ARM7_REG_W_RF_DATA2:
		case MEM_ARM7_REG_W_RF_DATA2 + 1:
		case MEM_ARM7_REG_W_AID_FULL:
		case MEM_ARM7_REG_W_RXBUF_BEGIN:
		case MEM_ARM7_REG_W_RXBUF_BEGIN + 1:
		case MEM_ARM7_REG_W_RXBUF_END:
		case MEM_ARM7_REG_W_RXBUF_END + 1:
		case MEM_ARM7_REG_W_RXBUF_WR_ADDR:
		case MEM_ARM7_REG_W_RXBUF_READCSR:
		case MEM_ARM7_REG_W_RXBUF_COUNT:
		case MEM_ARM7_REG_W_CONFIG_120:
		case MEM_ARM7_REG_W_CONFIG_122:
		case MEM_ARM7_REG_W_CONFIG_122 + 1:
		case MEM_ARM7_REG_W_CONFIG_124:
		case MEM_ARM7_REG_W_CONFIG_124 + 1:
		case MEM_ARM7_REG_W_CONFIG_128:
		case MEM_ARM7_REG_W_CONFIG_128 + 1:
		case MEM_ARM7_REG_W_CONFIG_130:
		case MEM_ARM7_REG_W_CONFIG_132:
		case MEM_ARM7_REG_W_POST_BEACON:
		case MEM_ARM7_REG_W_POST_BEACON + 1:
		case MEM_ARM7_REG_W_RXBUF_GAPDISP:
		case MEM_ARM7_REG_W_TXBUF_COUNT:
		case MEM_ARM7_REG_W_MODE_RST:
		case MEM_ARM7_REG_W_TXSTATCNT:
		case MEM_ARM7_REG_W_TXSTATCNT + 1:
		case MEM_ARM7_REG_W_IE:
		case MEM_ARM7_REG_W_IE + 1:
		case MEM_ARM7_REG_W_X_00A:
		case MEM_ARM7_REG_W_X_00A + 1:
			mem_set_wifi_reg8(mem, addr, v);
			return;
		case MEM_ARM7_REG_W_MODE_WEP:
			mem_set_wifi_reg8(mem, addr, v & 0x7F);
			return;
		case MEM_ARM7_REG_W_MODE_WEP + 1:
			return;
		case MEM_ARM7_REG_W_BB_CNT + 1:
			switch (v & 0xF0)
			{
				case 0x50:
#if 0
					printf("WIFI BB write [%02" PRIx8 "] = 0x%02" PRIx8 "\n",
					       mem_get_wifi_reg8(mem, MEM_ARM7_REG_W_BB_CNT),
					       mem_get_wifi_reg8(mem, MEM_ARM7_REG_W_BB_WRITE));
#endif
					mem->wifi.bb_regs[mem_get_wifi_reg8(mem, MEM_ARM7_REG_W_BB_CNT)] = mem_get_wifi_reg8(mem, MEM_ARM7_REG_W_BB_WRITE);
					break;
				case 0x60:
					mem_set_wifi_reg8(mem, MEM_ARM7_REG_W_BB_READ, mem->wifi.bb_regs[mem_get_wifi_reg8(mem, MEM_ARM7_REG_W_BB_CNT)]);
#if 0
					printf("WIFI BB read [%02" PRIx8 "] = 0x%02" PRIx8 "\n",
					       mem_get_wifi_reg8(mem, MEM_ARM7_REG_W_BB_CNT),
					       mem_get_wifi_reg8(mem, MEM_ARM7_REG_W_BB_READ));
#endif
					break;
				default:
					printf("WIFI unknown BB dir: 0x%02" PRIx8 "\n", v);
					break;
			}
			return;
		case MEM_ARM7_REG_W_CONFIG_144 + 1:
		case MEM_ARM7_REG_W_CONFIG_146 + 1:
		case MEM_ARM7_REG_W_CONFIG_148 + 1:
		case MEM_ARM7_REG_W_CONFIG_14A + 1:
		case MEM_ARM7_REG_W_POWER_US + 1:
		case MEM_ARM7_REG_W_TXREQ_SET + 1:
		case MEM_ARM7_REG_W_POWER_TX + 1:
		case MEM_ARM7_REG_W_PREAMBLE + 1:
			return;
		case MEM_ARM7_REG_W_CONFIG_150:
			mem_set_wifi_reg8(mem, addr, v & 0x3F);
			return;
		case MEM_ARM7_REG_W_CONFIG_154:
			mem_set_wifi_reg8(mem, addr, v & 0x7F);
			return;
		case MEM_ARM7_REG_W_CONFIG_154 + 1:
			mem_set_wifi_reg8(mem, addr, v & 0x7A);
			return;
		case MEM_ARM7_REG_W_POWER_US:
			mem_set_wifi_reg8(mem, addr, v & 0x3);
			return;
		case MEM_ARM7_REG_W_RF_CNT:
			mem_set_wifi_reg8(mem, addr, v & 0x3F);
			return;
		case MEM_ARM7_REG_W_RF_CNT + 1:
			mem_set_wifi_reg8(mem, addr, v & 0x41);
			return;
		case MEM_ARM7_REG_W_BEACONINT + 1:
			mem_set_wifi_reg8(mem, addr, v & 0x3);
			return;
		case MEM_ARM7_REG_W_AID_FULL + 1:
			mem_set_wifi_reg8(mem, addr, v & 0x7);
			return;
		case MEM_ARM7_REG_W_RXBUF_WR_ADDR + 1:
			mem_set_wifi_reg8(mem, addr, v & 0xF);
			return;
		case MEM_ARM7_REG_W_RXBUF_RD_ADDR:
			mem_set_wifi_reg8(mem, addr, v & 0xFE);
			return;
		case MEM_ARM7_REG_W_RXBUF_RD_ADDR + 1:
			mem_set_wifi_reg8(mem, addr, v & 0x1F);
			return;
		case MEM_ARM7_REG_W_RXBUF_READCSR + 1:
			mem_set_wifi_reg8(mem, addr, v & 0xF);
			return;
		case MEM_ARM7_REG_W_RXBUF_COUNT + 1:
			mem_set_wifi_reg8(mem, addr, v & 0xF);
			return;
		case MEM_ARM7_REG_W_RXBUF_GAP:
			mem_set_wifi_reg8(mem, addr, v & 0xFE);
			return;
		case MEM_ARM7_REG_W_RXBUF_GAP + 1:
			mem_set_wifi_reg8(mem, addr, v & 0x1F);
			return;
		case MEM_ARM7_REG_W_CONFIG_120 + 1:
			mem_set_wifi_reg8(mem, addr, v & 0x81);
			return;
		case MEM_ARM7_REG_W_CONFIG_130 + 1:
			mem_set_wifi_reg8(mem, addr, v & 0xF);
			return;
		case MEM_ARM7_REG_W_CONFIG_132 + 1:
			mem_set_wifi_reg8(mem, addr, v & 0x8F);
			return;
		case MEM_ARM7_REG_W_RXBUF_GAPDISP + 1:
			mem_set_wifi_reg8(mem, addr, v & 0xF);
			return;
		case MEM_ARM7_REG_W_TXBUF_WR_ADDR:
			mem_set_wifi_reg8(mem, addr, v & 0xFE);
			return;
		case MEM_ARM7_REG_W_TXBUF_WR_ADDR + 1:
			mem_set_wifi_reg8(mem, addr, v & 0x1F);
			return;
		case MEM_ARM7_REG_W_TXBUF_COUNT + 1:
			mem_set_wifi_reg8(mem, addr, v & 0xF);
			return;
		case MEM_ARM7_REG_W_TXBUF_GAP:
			mem_set_wifi_reg8(mem, addr, v & 0xFE);
			return;
		case MEM_ARM7_REG_W_TXBUF_GAP + 1:
			mem_set_wifi_reg8(mem, addr, v & 0x1F);
			return;
		case MEM_ARM7_REG_W_TXREQ_SET:
			mem_set_wifi_reg8(mem, MEM_ARM7_REG_W_TXREQ_READ, mem_get_wifi_reg8(mem, MEM_ARM7_REG_W_TXREQ_READ) | (v & 0xF));
			return;
		case MEM_ARM7_REG_W_POWER_TX:
			mem_set_wifi_reg8(mem, addr, v & 0x7);
			return;
		case MEM_ARM7_REG_W_MODE_RST + 1:
			mem_set_wifi_reg8(mem, addr, v & 0x9F);
			return;
		case MEM_ARM7_REG_W_IF:
			mem_set_wifi_reg8(mem, addr, mem_get_wifi_reg8(mem, addr) & ~v);
			return;
		case MEM_ARM7_REG_W_IF + 1:
			mem_set_wifi_reg8(mem, addr, mem_get_wifi_reg8(mem, addr) & ~v);
			return;
		case MEM_ARM7_REG_W_PREAMBLE:
			mem_set_wifi_reg8(mem, addr, v & 0x3);
			return;
		case MEM_ARM7_REG_W_POWERFORCE:
			mem_set_wifi_reg8(mem, addr, v & 0x1);
			return;
		case MEM_ARM7_REG_W_POWERFORCE + 1:
			mem_set_wifi_reg8(mem, addr, v & 0x80);
			return;
		case 0xE0:  /* silent these. they are memset(0) */
		case 0xE1:
		case 0xE2:
		case 0xE3:
		case 0xE4:
		case 0xE5:
		case 0xE6:
		case 0xE7:
		case 0xE8:
		case 0xE9:
		case 0xEA:
		case 0xEB:
		case 0xEC:
		case 0xED:
		case 0xEE:
		case 0xEF:
		case 0xF0:
		case 0xF1:
		case 0xF2:
		case 0xF3:
		case 0xF4:
		case 0xF5:
		case 0xF6:
		case 0xF7:
		case 0xF8:
		case 0xF9:
		case 0xFA:
		case 0xFB:
		case 0xFC:
		case 0xFD:
		case 0xFE:
		case 0xFF:
		case 0x110:
		case 0x111:
		case 0x112:
		case 0x113:
		case 0x114:
		case 0x115:
		case 0x116:
		case 0x117:
		case 0x118:
		case 0x119:
		case 0x11A:
		case 0x11B:
		case 0x11C:
		case 0x11D:
		case 0x11E:
		case 0x11F:
		case 0x124:
		case 0x125:
		case 0x126:
		case 0x127:
		case 0x12C:
		case 0x12D:
		case 0x12E:
		case 0x12F:
		case 0x50A:
		case 0x50B:
		case 0x50C:
		case 0x50D:
		case 0x50E:
		case 0x50F:
			return;
		default:
			if (addr >= 0x804000 && addr < 0x806000)
			{
				mem->wifi.xbuf[addr - 0x804000] = v;
				return;
			}
			printf("[ARM7] [%08" PRIx32 "] unknown set register %08" PRIx32 " = %02" PRIx8 "\n",
			       cpu_get_reg(mem->nds->arm7, CPU_REG_PC), addr, v);
			break;
	}
}

static void set_arm7_reg16(struct mem *mem, uint32_t addr, uint16_t v)
{
	set_arm7_reg8(mem, addr + 0, v >> 0);
	set_arm7_reg8(mem, addr + 1, v >> 8);
}

static void set_arm7_reg32(struct mem *mem, uint32_t addr, uint32_t v)
{
	set_arm7_reg8(mem, addr + 0, v >> 0);
	set_arm7_reg8(mem, addr + 1, v >> 8);
	set_arm7_reg8(mem, addr + 2, v >> 16);
	set_arm7_reg8(mem, addr + 3, v >> 24);
}

static uint8_t get_arm7_reg8(struct mem *mem, uint32_t addr)
{
	switch (addr)
	{
		case MEM_ARM7_REG_IPCSYNC:
			return mem->arm9_regs[MEM_ARM9_REG_IPCSYNC + 1] & 0x7;
		case MEM_ARM7_REG_IPCSYNC + 1:
		case MEM_ARM7_REG_IPCSYNC + 2:
		case MEM_ARM7_REG_IPCSYNC + 3:
		case MEM_ARM7_REG_IE:
		case MEM_ARM7_REG_IE + 1:
		case MEM_ARM7_REG_IE + 2:
		case MEM_ARM7_REG_IE + 3:
		case MEM_ARM7_REG_IF:
		case MEM_ARM7_REG_IF + 1:
		case MEM_ARM7_REG_IF + 2:
		case MEM_ARM7_REG_IF + 3:
		case MEM_ARM7_REG_IME:
		case MEM_ARM7_REG_IME + 1:
		case MEM_ARM7_REG_IME + 2:
		case MEM_ARM7_REG_IME + 3:
		case MEM_ARM7_REG_POSTFLG:
		case MEM_ARM7_REG_HALTCNT:
		case MEM_ARM7_REG_BIOSPROT:
		case MEM_ARM7_REG_BIOSPROT + 1:
		case MEM_ARM7_REG_BIOSPROT + 2:
		case MEM_ARM7_REG_BIOSPROT + 3:
		case MEM_ARM7_REG_SOUNDBIAS:
		case MEM_ARM7_REG_SOUNDBIAS + 1:
		case MEM_ARM7_REG_SOUNDBIAS + 2:
		case MEM_ARM7_REG_SOUNDBIAS + 3:
		case MEM_ARM7_REG_DMA0SAD:
		case MEM_ARM7_REG_DMA0SAD + 1:
		case MEM_ARM7_REG_DMA0SAD + 2:
		case MEM_ARM7_REG_DMA0SAD + 3:
		case MEM_ARM7_REG_DMA0DAD:
		case MEM_ARM7_REG_DMA0DAD + 1:
		case MEM_ARM7_REG_DMA0DAD + 2:
		case MEM_ARM7_REG_DMA0DAD + 3:
		case MEM_ARM7_REG_DMA0CNT_L:
		case MEM_ARM7_REG_DMA0CNT_L + 1:
		case MEM_ARM7_REG_DMA0CNT_H:
		case MEM_ARM7_REG_DMA0CNT_H + 1:
		case MEM_ARM7_REG_DMA1SAD:
		case MEM_ARM7_REG_DMA1SAD + 1:
		case MEM_ARM7_REG_DMA1SAD + 2:
		case MEM_ARM7_REG_DMA1SAD + 3:
		case MEM_ARM7_REG_DMA1DAD:
		case MEM_ARM7_REG_DMA1DAD + 1:
		case MEM_ARM7_REG_DMA1DAD + 2:
		case MEM_ARM7_REG_DMA1DAD + 3:
		case MEM_ARM7_REG_DMA1CNT_L:
		case MEM_ARM7_REG_DMA1CNT_L + 1:
		case MEM_ARM7_REG_DMA1CNT_H:
		case MEM_ARM7_REG_DMA1CNT_H + 1:
		case MEM_ARM7_REG_DMA2SAD:
		case MEM_ARM7_REG_DMA2SAD + 1:
		case MEM_ARM7_REG_DMA2SAD + 2:
		case MEM_ARM7_REG_DMA2SAD + 3:
		case MEM_ARM7_REG_DMA2DAD:
		case MEM_ARM7_REG_DMA2DAD + 1:
		case MEM_ARM7_REG_DMA2DAD + 2:
		case MEM_ARM7_REG_DMA2DAD + 3:
		case MEM_ARM7_REG_DMA2CNT_L:
		case MEM_ARM7_REG_DMA2CNT_L + 1:
		case MEM_ARM7_REG_DMA2CNT_H:
		case MEM_ARM7_REG_DMA2CNT_H + 1:
		case MEM_ARM7_REG_DMA3SAD:
		case MEM_ARM7_REG_DMA3SAD + 1:
		case MEM_ARM7_REG_DMA3SAD + 2:
		case MEM_ARM7_REG_DMA3SAD + 3:
		case MEM_ARM7_REG_DMA3DAD:
		case MEM_ARM7_REG_DMA3DAD + 1:
		case MEM_ARM7_REG_DMA3DAD + 2:
		case MEM_ARM7_REG_DMA3DAD + 3:
		case MEM_ARM7_REG_DMA3CNT_L:
		case MEM_ARM7_REG_DMA3CNT_L  +1:
		case MEM_ARM7_REG_DMA3CNT_H:
		case MEM_ARM7_REG_DMA3CNT_H + 1:
		case MEM_ARM7_REG_POWCNT2:
		case MEM_ARM7_REG_POWCNT2 + 1:
		case MEM_ARM7_REG_POWCNT2 + 2:
		case MEM_ARM7_REG_POWCNT2 + 3:
		case MEM_ARM7_REG_RCNT:
		case MEM_ARM7_REG_RCNT + 1:
		case MEM_ARM7_REG_SOUNDCNT:
		case MEM_ARM7_REG_SOUNDCNT + 1:
		case MEM_ARM7_REG_SOUNDCNT + 2:
		case MEM_ARM7_REG_SOUNDCNT + 3:
		case MEM_ARM7_REG_WIFIWAITCNT:
		case MEM_ARM7_REG_WIFIWAITCNT + 1:
		case MEM_ARM7_REG_SNDCAP0CNT:
		case MEM_ARM7_REG_SNDCAP0DAD:
		case MEM_ARM7_REG_SNDCAP0DAD + 1:
		case MEM_ARM7_REG_SNDCAP0DAD + 2:
		case MEM_ARM7_REG_SNDCAP0DAD + 3:
		case MEM_ARM7_REG_SNDCAP1CNT:
		case MEM_ARM7_REG_SNDCAP1DAD:
		case MEM_ARM7_REG_SNDCAP1DAD + 1:
		case MEM_ARM7_REG_SNDCAP1DAD + 2:
		case MEM_ARM7_REG_SNDCAP1DAD + 3:
		case MEM_ARM7_REG_SOUNDXCNT(0):
		case MEM_ARM7_REG_SOUNDXCNT(0) + 1:
		case MEM_ARM7_REG_SOUNDXCNT(0) + 2:
		case MEM_ARM7_REG_SOUNDXCNT(0) + 3:
		case MEM_ARM7_REG_SOUNDXCNT(1):
		case MEM_ARM7_REG_SOUNDXCNT(1) + 1:
		case MEM_ARM7_REG_SOUNDXCNT(1) + 2:
		case MEM_ARM7_REG_SOUNDXCNT(1) + 3:
		case MEM_ARM7_REG_SOUNDXCNT(2):
		case MEM_ARM7_REG_SOUNDXCNT(2) + 1:
		case MEM_ARM7_REG_SOUNDXCNT(2) + 2:
		case MEM_ARM7_REG_SOUNDXCNT(2) + 3:
		case MEM_ARM7_REG_SOUNDXCNT(3):
		case MEM_ARM7_REG_SOUNDXCNT(3) + 1:
		case MEM_ARM7_REG_SOUNDXCNT(3) + 2:
		case MEM_ARM7_REG_SOUNDXCNT(3) + 3:
		case MEM_ARM7_REG_SOUNDXCNT(4):
		case MEM_ARM7_REG_SOUNDXCNT(4) + 1:
		case MEM_ARM7_REG_SOUNDXCNT(4) + 2:
		case MEM_ARM7_REG_SOUNDXCNT(4) + 3:
		case MEM_ARM7_REG_SOUNDXCNT(5):
		case MEM_ARM7_REG_SOUNDXCNT(5) + 1:
		case MEM_ARM7_REG_SOUNDXCNT(5) + 2:
		case MEM_ARM7_REG_SOUNDXCNT(5) + 3:
		case MEM_ARM7_REG_SOUNDXCNT(6):
		case MEM_ARM7_REG_SOUNDXCNT(6) + 1:
		case MEM_ARM7_REG_SOUNDXCNT(6) + 2:
		case MEM_ARM7_REG_SOUNDXCNT(6) + 3:
		case MEM_ARM7_REG_SOUNDXCNT(7):
		case MEM_ARM7_REG_SOUNDXCNT(7) + 1:
		case MEM_ARM7_REG_SOUNDXCNT(7) + 2:
		case MEM_ARM7_REG_SOUNDXCNT(7) + 3:
		case MEM_ARM7_REG_SOUNDXCNT(8):
		case MEM_ARM7_REG_SOUNDXCNT(8) + 1:
		case MEM_ARM7_REG_SOUNDXCNT(8) + 2:
		case MEM_ARM7_REG_SOUNDXCNT(8) + 3:
		case MEM_ARM7_REG_SOUNDXCNT(9):
		case MEM_ARM7_REG_SOUNDXCNT(9) + 1:
		case MEM_ARM7_REG_SOUNDXCNT(9) + 2:
		case MEM_ARM7_REG_SOUNDXCNT(9) + 3:
		case MEM_ARM7_REG_SOUNDXCNT(10):
		case MEM_ARM7_REG_SOUNDXCNT(10) + 1:
		case MEM_ARM7_REG_SOUNDXCNT(10) + 2:
		case MEM_ARM7_REG_SOUNDXCNT(10) + 3:
		case MEM_ARM7_REG_SOUNDXCNT(11):
		case MEM_ARM7_REG_SOUNDXCNT(11) + 1:
		case MEM_ARM7_REG_SOUNDXCNT(11) + 2:
		case MEM_ARM7_REG_SOUNDXCNT(11) + 3:
		case MEM_ARM7_REG_SOUNDXCNT(12):
		case MEM_ARM7_REG_SOUNDXCNT(12) + 1:
		case MEM_ARM7_REG_SOUNDXCNT(12) + 2:
		case MEM_ARM7_REG_SOUNDXCNT(12) + 3:
		case MEM_ARM7_REG_SOUNDXCNT(13):
		case MEM_ARM7_REG_SOUNDXCNT(13) + 1:
		case MEM_ARM7_REG_SOUNDXCNT(13) + 2:
		case MEM_ARM7_REG_SOUNDXCNT(13) + 3:
		case MEM_ARM7_REG_SOUNDXCNT(14):
		case MEM_ARM7_REG_SOUNDXCNT(14) + 1:
		case MEM_ARM7_REG_SOUNDXCNT(14) + 2:
		case MEM_ARM7_REG_SOUNDXCNT(14) + 3:
		case MEM_ARM7_REG_SOUNDXCNT(15):
		case MEM_ARM7_REG_SOUNDXCNT(15) + 1:
		case MEM_ARM7_REG_SOUNDXCNT(15) + 2:
		case MEM_ARM7_REG_SOUNDXCNT(15) + 3:
		case MEM_ARM7_REG_DISPSTAT:
		case MEM_ARM7_REG_DISPSTAT + 1:
		case MEM_ARM7_REG_TM0CNT_H:
		case MEM_ARM7_REG_TM0CNT_H + 1:
		case MEM_ARM7_REG_TM1CNT_H:
		case MEM_ARM7_REG_TM1CNT_H + 1:
		case MEM_ARM7_REG_TM2CNT_H:
		case MEM_ARM7_REG_TM2CNT_H + 1:
		case MEM_ARM7_REG_TM3CNT_H:
		case MEM_ARM7_REG_TM3CNT_H + 1:
			return mem->arm7_regs[addr];
		case MEM_ARM7_REG_SPICNT:
		case MEM_ARM7_REG_SPICNT + 1:
#if 0
			printf("[ARM7] [%08" PRIx32 "] SPICNT[%08" PRIx32 "] read 0x%02" PRIx8 "\n",
			       cpu_get_reg(mem->nds->arm7, CPU_REG_PC), addr, mem->arm7_regs[addr]);
#endif
			return mem->arm7_regs[addr];
		case MEM_ARM7_REG_ROMCTRL:
		case MEM_ARM7_REG_ROMCTRL + 1:
		case MEM_ARM7_REG_ROMCTRL + 3:
		case MEM_ARM7_REG_ROMCMD:
		case MEM_ARM7_REG_ROMCMD + 1:
		case MEM_ARM7_REG_ROMCMD + 2:
		case MEM_ARM7_REG_ROMCMD + 3:
		case MEM_ARM7_REG_ROMCMD + 4:
		case MEM_ARM7_REG_ROMCMD + 5:
		case MEM_ARM7_REG_ROMCMD + 6:
		case MEM_ARM7_REG_ROMCMD + 7:
		case MEM_ARM7_REG_EXMEMSTAT:
		case MEM_ARM7_REG_EXMEMSTAT + 1:
		case MEM_ARM7_REG_KEYCNT:
		case MEM_ARM7_REG_KEYCNT + 1:
		case MEM_ARM7_REG_VCOUNT:
		case MEM_ARM7_REG_VCOUNT + 1:
			return mem->arm9_regs[addr];
		case MEM_ARM7_REG_AUXSPICNT:
		case MEM_ARM7_REG_AUXSPICNT + 1:
#if 0
			printf("[ARM7] [%08" PRIx32 "] AUXSPICNT[%08" PRIx32 "] read 0x%02" PRIx8 "\n",
			       cpu_get_reg(mem->nds->arm7, CPU_REG_PC), addr, mem->arm9_regs[addr]);
#endif
			return mem->arm9_regs[addr];
		case MEM_ARM7_REG_AUXSPIDATA:
			return auxspi_read(mem);
		case MEM_ARM7_REG_ROMDATA:
		case MEM_ARM7_REG_ROMDATA + 1:
		case MEM_ARM7_REG_ROMDATA + 2:
		case MEM_ARM7_REG_ROMDATA + 3:
			return mbc_read(mem->mbc);
		case MEM_ARM7_REG_TM0CNT_L:
			return mem->arm7_timers[0].v >> 10;
		case MEM_ARM7_REG_TM0CNT_L + 1:
			return mem->arm7_timers[0].v >> 18;
		case MEM_ARM7_REG_TM1CNT_L:
			return mem->arm7_timers[1].v >> 10;
		case MEM_ARM7_REG_TM1CNT_L + 1:
			return mem->arm7_timers[1].v >> 18;
		case MEM_ARM7_REG_TM2CNT_L:
			return mem->arm7_timers[2].v >> 10;
		case MEM_ARM7_REG_TM2CNT_L + 1:
			return mem->arm7_timers[2].v >> 18;
		case MEM_ARM7_REG_TM3CNT_L:
			return mem->arm7_timers[3].v >> 10;
		case MEM_ARM7_REG_TM3CNT_L + 1:
			return mem->arm7_timers[3].v >> 18;
		case MEM_ARM7_REG_SPIDATA:
			return spi_read(mem);
		case MEM_ARM7_REG_SPIDATA + 1:
		case MEM_ARM7_REG_RTC + 1:
		case MEM_ARM7_REG_RTC + 2:
		case MEM_ARM7_REG_RTC + 3:
		case MEM_ARM7_REG_SIODATA32:
		case MEM_ARM7_REG_SIODATA32 + 1:
		case MEM_ARM7_REG_SIODATA32 + 2:
		case MEM_ARM7_REG_SIODATA32 + 3:
		case MEM_ARM7_REG_SIOCNT:
		case MEM_ARM7_REG_SIOCNT + 1:
		case MEM_ARM7_REG_SIOCNT + 2:
		case MEM_ARM7_REG_SIOCNT + 3:
		case MEM_ARM7_REG_EXTKEYIN + 1:
		case MEM_ARM7_REG_AUXSPIDATA + 1:
			return 0;
		case MEM_ARM7_REG_WRAMSTAT:
			return mem->arm9_regs[MEM_ARM9_REG_WRAMCNT];
		case MEM_ARM7_REG_RTC:
			return rtc_read(mem);
		case MEM_ARM7_REG_KEYINPUT:
		{
			uint8_t v = 0;
			if (!(mem->nds->joypad & NDS_BUTTON_A))
				v |= (1 << 0);
			if (!(mem->nds->joypad & NDS_BUTTON_B))
				v |= (1 << 1);
			if (!(mem->nds->joypad & NDS_BUTTON_SELECT))
				v |= (1 << 2);
			if (!(mem->nds->joypad & NDS_BUTTON_START))
				v |= (1 << 3);
			if (!(mem->nds->joypad & NDS_BUTTON_RIGHT))
				v |= (1 << 4);
			if (!(mem->nds->joypad & NDS_BUTTON_LEFT))
				v |= (1 << 5);
			if (!(mem->nds->joypad & NDS_BUTTON_UP))
				v |= (1 << 6);
			if (!(mem->nds->joypad & NDS_BUTTON_DOWN))
				v |= (1 << 7);
			return v;
		}
		case MEM_ARM7_REG_KEYINPUT + 1:
		{
			uint8_t v = 0;
			if (!(mem->nds->joypad & NDS_BUTTON_R))
				v |= (1 << 0);
			if (!(mem->nds->joypad & NDS_BUTTON_L))
				v |= (1 << 1);
			return v;
		}
		case MEM_ARM7_REG_EXTKEYIN:
		{
			uint8_t v = 0;
			if (!(mem->nds->joypad & NDS_BUTTON_X))
				v |= (1 << 0);
			if (!(mem->nds->joypad & NDS_BUTTON_Y))
				v |= (1 << 1);
			v |= (1 << 2);
			v |= (1 << 3);
			v |= (1 << 4);
			v |= (1 << 5);
			if (!mem->nds->touch)
				v |= (1 << 6);
			return v;
		}
		case MEM_ARM7_REG_IPCFIFOCNT:
		{
			uint8_t v = mem->arm7_regs[addr] & (1 << 2);
			if (mem->arm9_fifo.len < 4)
				v |= (1 << 0);
			if (mem->arm9_fifo.len == 64)
				v |= (1 << 1);
#if 0
			printf("[ARM7] FIFOCNT[0] read %02" PRIx8 "\n", v);
#endif
			return v;
		}
		case MEM_ARM7_REG_IPCFIFOCNT + 1:
		{
			uint8_t v = mem->arm7_regs[addr] & 0xC4;
			if (mem->arm7_fifo.len < 4)
				v |= (1 << 0);
			if (mem->arm7_fifo.len == 64)
				v |= (1 << 1);
#if 0
			printf("[ARM7] FIFOCNT[1] read %02" PRIx8 "\n", v);
#endif
			return v;
		}
		case MEM_ARM7_REG_IPCFIFORECV:
		case MEM_ARM7_REG_IPCFIFORECV + 1:
		case MEM_ARM7_REG_IPCFIFORECV + 2:
		case MEM_ARM7_REG_IPCFIFORECV + 3:
		{
#if 0
			printf("[ARM7] IPCFIFO read\n");
#endif
			if (!(mem->arm7_regs[MEM_ARM7_REG_IPCFIFOCNT + 1] & (1 << 7)))
				return mem->arm7_fifo.latch[addr - MEM_ARM7_REG_IPCFIFORECV];
			if (!mem->arm7_fifo.len)
			{
				mem->arm7_regs[MEM_ARM7_REG_IPCFIFOCNT + 1] |= (1 << 6);
				return mem->arm7_fifo.latch[addr - MEM_ARM7_REG_IPCFIFORECV];
			}
			uint8_t v = mem->arm7_fifo.data[mem->arm7_fifo.pos];
			mem->arm7_fifo.pos = (mem->arm7_fifo.pos + 1) % 64;
			mem->arm7_fifo.latch[addr - MEM_ARM7_REG_IPCFIFORECV] = v;
			mem->arm7_fifo.len--;
			if (!mem->arm7_fifo.len
			 && (mem->arm9_regs[MEM_ARM9_REG_IPCFIFOCNT] & (1 << 2)))
				mem_arm9_irq(mem, 1 << 17);
			return v;
		}
		case MEM_ARM7_REG_VRAMSTAT:
		{
			uint8_t v = 0;
			if ((mem_arm9_get_reg8(mem, MEM_ARM9_REG_VRAMCNT_C) & 0x87) == 0x82)
				v |= (1 << 0);
			if ((mem_arm9_get_reg8(mem, MEM_ARM9_REG_VRAMCNT_D) & 0x87) == 0x82)
				v |= (1 << 1);
			return v;
		}
		case MEM_ARM7_REG_ROMCTRL + 2:
			if (mem->dscard_dma_count)  /* nasty hack: fake non-availibility if dma is running */
				return mem->arm9_regs[addr] & ~(1 << 7);
			return mem->arm9_regs[addr];
		case MEM_ARM7_REG_W_ID:
		case MEM_ARM7_REG_W_ID + 1:
		case MEM_ARM7_REG_W_BB_READ:
		case MEM_ARM7_REG_W_BB_READ + 1:
		case MEM_ARM7_REG_W_BB_BUSY:
		case MEM_ARM7_REG_W_BB_BUSY + 1:
		case MEM_ARM7_REG_W_BB_MODE:
		case MEM_ARM7_REG_W_BB_MODE + 1:
		case MEM_ARM7_REG_W_BB_POWER:
		case MEM_ARM7_REG_W_BB_POWER + 1:
		case MEM_ARM7_REG_W_MODE_WEP:
		case MEM_ARM7_REG_W_MODE_WEP + 1:
		case MEM_ARM7_REG_W_MACADDR_0:
		case MEM_ARM7_REG_W_MACADDR_0 + 1:
		case MEM_ARM7_REG_W_MACADDR_1:
		case MEM_ARM7_REG_W_MACADDR_1 + 1:
		case MEM_ARM7_REG_W_MACADDR_2:
		case MEM_ARM7_REG_W_MACADDR_2 + 1:
		case MEM_ARM7_REG_W_BSSID_0:
		case MEM_ARM7_REG_W_BSSID_0 + 1:
		case MEM_ARM7_REG_W_BSSID_1:
		case MEM_ARM7_REG_W_BSSID_1 + 1:
		case MEM_ARM7_REG_W_BSSID_2:
		case MEM_ARM7_REG_W_BSSID_2 + 1:
		case MEM_ARM7_REG_W_TX_RETRYLIMIT:
		case MEM_ARM7_REG_W_TX_RETRYLIMIT + 1:
		case MEM_ARM7_REG_W_CONFIG_140:
		case MEM_ARM7_REG_W_CONFIG_140 + 1:
		case MEM_ARM7_REG_W_CONFIG_142:
		case MEM_ARM7_REG_W_CONFIG_142 + 1:
		case MEM_ARM7_REG_W_CONFIG_144:
		case MEM_ARM7_REG_W_CONFIG_144 + 1:
		case MEM_ARM7_REG_W_CONFIG_146:
		case MEM_ARM7_REG_W_CONFIG_146 + 1:
		case MEM_ARM7_REG_W_CONFIG_148:
		case MEM_ARM7_REG_W_CONFIG_148 + 1:
		case MEM_ARM7_REG_W_CONFIG_14A:
		case MEM_ARM7_REG_W_CONFIG_14A + 1:
		case MEM_ARM7_REG_W_CONFIG_14C:
		case MEM_ARM7_REG_W_CONFIG_14C + 1:
		case MEM_ARM7_REG_W_CONFIG_150:
		case MEM_ARM7_REG_W_CONFIG_150 + 1:
		case MEM_ARM7_REG_W_CONFIG_154:
		case MEM_ARM7_REG_W_CONFIG_154 + 1:
		case MEM_ARM7_REG_W_POWER_US:
		case MEM_ARM7_REG_W_POWER_US + 1:
		case MEM_ARM7_REG_W_RF_CNT:
		case MEM_ARM7_REG_W_RF_CNT + 1:
		case MEM_ARM7_REG_W_BEACONINT:
		case MEM_ARM7_REG_W_BEACONINT + 1:
		case MEM_ARM7_REG_W_RF_DATA1:
		case MEM_ARM7_REG_W_RF_DATA1 + 1:
		case MEM_ARM7_REG_W_RF_DATA2:
		case MEM_ARM7_REG_W_RF_DATA2 + 1:
		case MEM_ARM7_REG_W_AID_FULL:
		case MEM_ARM7_REG_W_AID_FULL + 1:
		case MEM_ARM7_REG_W_RXBUF_BEGIN:
		case MEM_ARM7_REG_W_RXBUF_BEGIN + 1:
		case MEM_ARM7_REG_W_RXBUF_END:
		case MEM_ARM7_REG_W_RXBUF_END + 1:
		case MEM_ARM7_REG_W_RXBUF_WR_ADDR:
		case MEM_ARM7_REG_W_RXBUF_WR_ADDR + 1:
		case MEM_ARM7_REG_W_RXBUF_RD_ADDR:
		case MEM_ARM7_REG_W_RXBUF_RD_ADDR + 1:
		case MEM_ARM7_REG_W_RXBUF_READCSR:
		case MEM_ARM7_REG_W_RXBUF_READCSR + 1:
		case MEM_ARM7_REG_W_RXBUF_COUNT:
		case MEM_ARM7_REG_W_RXBUF_COUNT + 1:
		case MEM_ARM7_REG_W_RXBUF_GAP:
		case MEM_ARM7_REG_W_RXBUF_GAP + 1:
		case MEM_ARM7_REG_W_CONFIG_120:
		case MEM_ARM7_REG_W_CONFIG_120 + 1:
		case MEM_ARM7_REG_W_CONFIG_122:
		case MEM_ARM7_REG_W_CONFIG_122 + 1:
		case MEM_ARM7_REG_W_CONFIG_124:
		case MEM_ARM7_REG_W_CONFIG_124 + 1:
		case MEM_ARM7_REG_W_CONFIG_128:
		case MEM_ARM7_REG_W_CONFIG_128 + 1:
		case MEM_ARM7_REG_W_CONFIG_130:
		case MEM_ARM7_REG_W_CONFIG_130 + 1:
		case MEM_ARM7_REG_W_CONFIG_132:
		case MEM_ARM7_REG_W_CONFIG_132 + 1:
		case MEM_ARM7_REG_W_POST_BEACON:
		case MEM_ARM7_REG_W_POST_BEACON + 1:
		case MEM_ARM7_REG_W_RXBUF_GAPDISP:
		case MEM_ARM7_REG_W_RXBUF_GAPDISP + 1:
		case MEM_ARM7_REG_W_TXBUF_WR_ADDR:
		case MEM_ARM7_REG_W_TXBUF_WR_ADDR + 1:
		case MEM_ARM7_REG_W_TXBUF_COUNT:
		case MEM_ARM7_REG_W_TXBUF_COUNT + 1:
		case MEM_ARM7_REG_W_TXBUF_GAP:
		case MEM_ARM7_REG_W_TXBUF_GAP + 1:
		case MEM_ARM7_REG_W_POWER_TX:
		case MEM_ARM7_REG_W_POWER_TX + 1:
		case MEM_ARM7_REG_W_TXSTATCNT:
		case MEM_ARM7_REG_W_TXSTATCNT + 1:
		case MEM_ARM7_REG_W_IE:
		case MEM_ARM7_REG_W_IE + 1:
		case MEM_ARM7_REG_W_IF:
		case MEM_ARM7_REG_W_IF + 1:
		case MEM_ARM7_REG_W_X_00A:
		case MEM_ARM7_REG_W_X_00A + 1:
		case MEM_ARM7_REG_W_POWERSTATE:
		case MEM_ARM7_REG_W_POWERSTATE + 1:
		case MEM_ARM7_REG_W_PREAMBLE:
		case MEM_ARM7_REG_W_PREAMBLE + 1:
		case MEM_ARM7_REG_W_POWERFORCE:
		case MEM_ARM7_REG_W_POWERFORCE + 1:
		case MEM_ARM7_REG_W_RF_STATUS:
		case MEM_ARM7_REG_W_RF_STATUS + 1:
			return mem_get_wifi_reg8(mem, addr);
		case MEM_ARM7_REG_W_RF_BUSY:
		case MEM_ARM7_REG_W_RF_BUSY + 1:
			return 0; /* XXX */
		default:
			if (addr >= 0x804000 && addr < 0x806000)
				return mem->wifi.xbuf[addr - 0x804000];
			printf("[ARM7] [%08" PRIx32 "] unknown get register %08" PRIx32 "\n",
			       cpu_get_reg(mem->nds->arm7, CPU_REG_PC), addr);
			break;
	}
	return 0;
}

static uint16_t get_arm7_reg16(struct mem *mem, uint32_t addr)
{
	return (get_arm7_reg8(mem, addr + 0) << 0)
	     | (get_arm7_reg8(mem, addr + 1) << 8);
}

static uint32_t get_arm7_reg32(struct mem *mem, uint32_t addr)
{
	return (get_arm7_reg8(mem, addr + 0) << 0)
	     | (get_arm7_reg8(mem, addr + 1) << 8)
	     | (get_arm7_reg8(mem, addr + 2) << 16)
	     | (get_arm7_reg8(mem, addr + 3) << 24);
}

static void arm7_instr_delay(struct mem *mem, const uint8_t *table, enum mem_type type)
{
	mem->nds->arm7->instr_delay += table[type];
}

static void *get_arm7_vram_ptr(struct mem *mem, uint32_t addr)
{
	uint32_t base = mem->vram_arm7_bases[(addr / 0x20000) & 0x1];
	if (base == 0xFFFFFFFF)
		return NULL;
	return &mem->vram[base + (addr & 0x1FFFF)];
}

#define MEM_ARM7_GET(size) \
uint##size##_t mem_arm7_get##size(struct mem *mem, uint32_t addr, enum mem_type type) \
{ \
	if (size == 16) \
		addr &= ~1; \
	if (size == 32) \
		addr &= ~3; \
	switch ((addr >> 24) & 0xFF) \
	{ \
		case 0x0: /* ARM7 bios */ \
			if (addr >= sizeof(mem->arm7_bios)) \
				break; \
			uint32_t biosprot = mem_arm7_get_reg32(mem, MEM_ARM7_REG_BIOSPROT); \
			if (addr < biosprot && cpu_get_reg(mem->nds->arm7, CPU_REG_PC) >= biosprot) \
				return (uint##size##_t)0xFFFFFFFF; \
			arm7_instr_delay(mem, arm7_wram_cycles_##size, type); \
			return *(uint##size##_t*)&mem->arm7_bios[addr]; \
		case 0x2: /* main memory */ \
			arm7_instr_delay(mem, arm7_mram_cycles_##size, type); \
			return *(uint##size##_t*)&mem->mram[addr & 0x3FFFFF]; \
		case 0x3: /* wram */ \
			arm7_instr_delay(mem, arm7_wram_cycles_##size, type); \
			if (!mem->arm7_wram_mask || addr >= 0x3800000) \
				return *(uint##size##_t*)&mem->arm7_wram[addr & 0xFFFF]; \
			return *(uint##size##_t*)&mem->wram[mem->arm7_wram_base \
			     + (addr & mem->arm7_wram_mask)]; \
		case 0x4: /* io ports */ \
			arm7_instr_delay(mem, arm7_wram_cycles_##size, type); \
			return get_arm7_reg##size(mem, addr - 0x4000000); \
		case 0x6: /* vram */ \
		{ \
			void *ptr = get_arm7_vram_ptr(mem, addr & 0x3FFFF); \
			if (!ptr) \
				break; \
			arm7_instr_delay(mem, arm7_vram_cycles_##size, type); \
			return *(uint##size##_t*)ptr; \
		} \
		case 0x7: /* oam */ \
			break; \
		case 0x8: /* GBA rom */ \
		case 0x9: \
			if (!(mem->arm9_regs[MEM_ARM9_REG_EXMEMCNT] & (1 << 7))) \
				return 0; \
			if (size == 8 || size == 16) \
				return addr >> 1; \
			return (addr >> 1) | (((addr + 1) >> 1) << 16); \
		case 0xA: /* GBA ram */ \
			if (!(mem->arm9_regs[MEM_ARM9_REG_EXMEMCNT] & (1 << 7))) \
				return 0; \
			return 0xFF; \
	} \
	printf("[ARM7] [%08" PRIx32 "] unknown get" #size " [%08" PRIx32 "]\n", \
	       cpu_get_reg(mem->nds->arm7, CPU_REG_PC), addr); \
	return 0; \
}

MEM_ARM7_GET(8);
MEM_ARM7_GET(16);
MEM_ARM7_GET(32);

#define MEM_ARM7_SET(size) \
void mem_arm7_set##size(struct mem *mem, uint32_t addr, uint##size##_t v, enum mem_type type) \
{ \
	if (size == 16) \
		addr &= ~1; \
	if (size == 32) \
		addr &= ~3; \
	switch ((addr >> 24) & 0xFF) \
	{ \
		case 0x0: /* ARM7 bios */ \
			arm7_instr_delay(mem, arm7_wram_cycles_##size, type); \
			break; \
		case 0x2: /* main memory */ \
			*(uint##size##_t*)&mem->mram[addr & 0x3FFFFF] = v; \
			arm7_instr_delay(mem, arm7_mram_cycles_##size, type); \
			return; \
		case 0x3: /* wram */ \
			if (!mem->arm7_wram_mask || addr >= 0x3800000) \
				*(uint##size##_t*)&mem->arm7_wram[addr & 0xFFFF] = v; \
			else \
				*(uint##size##_t*)&mem->wram[mem->arm7_wram_base \
				                          + (addr & mem->arm7_wram_mask)] = v; \
			arm7_instr_delay(mem, arm7_wram_cycles_##size, type); \
			return; \
		case 0x4: /* io ports */ \
			set_arm7_reg##size(mem, addr - 0x4000000, v); \
			arm7_instr_delay(mem, arm7_wram_cycles_##size, type); \
			return; \
		case 0x6: /* vram */ \
		{ \
			/* printf("[ARM7] vram write [%08" PRIx32 "] = %x\n", addr, v); */ \
			void *ptr = get_arm7_vram_ptr(mem, addr & 0x3FFFF); \
			if (!ptr) \
				break; \
			arm7_instr_delay(mem, arm7_vram_cycles_##size, type); \
			*(uint##size##_t*)ptr = v; \
			return; \
		} \
		case 0x8: /* GBA */ \
		case 0x9: \
		case 0xA: \
			return; \
	} \
	printf("[ARM7] [%08" PRIx32 "] unknown set" #size " [%08" PRIx32 "] = %x\n", \
	       cpu_get_reg(mem->nds->arm7, CPU_REG_PC), addr, v); \
}

MEM_ARM7_SET(8);
MEM_ARM7_SET(16);
MEM_ARM7_SET(32);

static void run_div(struct mem *mem)
{
	mem->arm9_regs[MEM_ARM9_REG_DIVCNT + 1] &= ~(1 << 7);
	if (!mem_arm9_get_reg32(mem, MEM_ARM9_REG_DIV_DENOM + 0)
	 && !mem_arm9_get_reg32(mem, MEM_ARM9_REG_DIV_DENOM + 4))
	{
		mem->arm9_regs[MEM_ARM9_REG_DIVCNT + 1] |= (1 << 6);
		return;
	}
	mem->arm9_regs[MEM_ARM9_REG_DIVCNT + 1] &= ~(1 << 6);
	switch (mem->arm9_regs[MEM_ARM9_REG_DIVCNT] & 0x3)
	{
		case 0x0:
		{
			int32_t num = mem_arm9_get_reg32(mem, MEM_ARM9_REG_DIV_NUMER);
			int32_t den = mem_arm9_get_reg32(mem, MEM_ARM9_REG_DIV_DENOM);
			int32_t div;
			int32_t rem;
			if (den)
			{
				div = num / den;
				rem = num % den;
			}
			else if (num == INT32_MIN && den == -1)
			{
				div = INT32_MIN;
				rem = 0;
			}
			else
			{
				div = num > 0 ? -1 : 1;
				rem = num;
			}
#if 0
			printf("DIV0: %" PRId32 " / %" PRId32 " = %" PRId32 " / %" PRId32 "\n",
			       num, den, div, rem);
#endif
#if 0
			printf("DIV0: %+6.4f (%08x) / %+6.4f (%08x) = %+6.4f (%08x) / %+6.4f (%08x)\n",
			       num / (float)(1 << 12), (uint32_t)num,
			       den / (float)(1 << 12), (uint32_t)den,
			       div / (float)(1 << 12), (uint32_t)div,
			       rem / (float)(1 << 12), (uint32_t)rem);
#endif
			mem_arm9_set_reg32(mem, MEM_ARM9_REG_DIV_RESULT, div);
			mem_arm9_set_reg32(mem, MEM_ARM9_REG_DIVREM_RESULT, rem);
			break;
		}
		case 0x1:
		case 0x3:
		{
			int64_t num = mem_arm9_get_reg64(mem, MEM_ARM9_REG_DIV_NUMER);
			int32_t den = mem_arm9_get_reg32(mem, MEM_ARM9_REG_DIV_DENOM);
			int64_t div;
			int32_t rem;
			if (den)
			{
				div = num / den;
				rem = num % den;
			}
			else if (num == INT64_MIN && den == -1)
			{
				div = INT64_MIN;
				rem = 0;
			}
			else
			{
				div = num > 0 ? -1 : 1;
				rem = num;
			}
#if 0
			printf("DIV1: %" PRId64 " / %" PRId32 " = %" PRId64 " / %" PRId32 "\n",
			       num, den, div, rem);
#endif
#if 0
			printf("DIV1: %+6.4f (%016lx) / %+6.4f (%08x) = %+6.4f (%016lx) / %+6.4f (%08x)\n",
			       num / (float)(1 << 12), (uint64_t)num,
			       den / (float)(1 << 12), (uint32_t)den,
			       div / (float)(1 << 12), (uint64_t)div,
			       rem / (float)(1 << 12), (uint32_t)rem);
#endif
			mem_arm9_set_reg64(mem, MEM_ARM9_REG_DIV_RESULT, div);
			mem_arm9_set_reg32(mem, MEM_ARM9_REG_DIVREM_RESULT, rem);
			break;
		}
		case 0x2:
		{
			int64_t num = mem_arm9_get_reg64(mem, MEM_ARM9_REG_DIV_NUMER);
			int64_t den = mem_arm9_get_reg64(mem, MEM_ARM9_REG_DIV_DENOM);
			int64_t div;
			int64_t rem;
			if (den)
			{
				div = num / den;
				rem = num % den;
			}
			else if (num == INT64_MIN && den == -1)
			{
				div = INT64_MIN;
				rem = 0;
			}
			else
			{
				div = num > 0 ? -1 : 1;
				rem = num;
			}
#if 0
			printf("DIV2: %" PRId64 " / %" PRId64 " = %" PRId64 " / %" PRId64 "\n",
			       num, den, div, rem);
#endif
#if 0
			printf("DIV2: %+6.4f / %+6.4f = %+6.4f / %+6.4f\n",
			       num / (float)(1 << 12), den / (float)(1 << 12),
			       div / (float)(1 << 12), rem / (float)(1 << 12));
#endif
			mem_arm9_set_reg64(mem, MEM_ARM9_REG_DIV_RESULT, div);
			mem_arm9_set_reg64(mem, MEM_ARM9_REG_DIVREM_RESULT, rem);
			break;
		}
	}
}

static void run_sqrt(struct mem *mem)
{
	uint64_t param;
	if (mem_arm9_get_reg32(mem, MEM_ARM9_REG_SQRTCNT) & (1 << 0))
		param = mem_arm9_get_reg64(mem, MEM_ARM9_REG_SQRT_PARAM);
	else
		param = mem_arm9_get_reg32(mem, MEM_ARM9_REG_SQRT_PARAM);
	uint64_t l = 0;
	uint64_t m;
	uint64_t r = param + 1;
	while (l != r - 1)
	{
		m = (l + r) / 2;
		if (m * m <= param)
			l = m;
		else
			r = m;
	}
#if 0
	printf("SQRT(%" PRIu64 ") = %" PRIu64 "\n", param, l);
#endif
	mem_arm9_set_reg32(mem, MEM_ARM9_REG_SQRT_RESULT, l);
}

static void update_vram_maps(struct mem *mem)
{
	for (size_t i = 0; i < 32; ++i)
		mem->vram_bga_bases[i] = 0xFFFFFFFF;
	for (size_t i = 0; i < 8; ++i)
		mem->vram_bgb_bases[i] = 0xFFFFFFFF;
	for (size_t i = 0; i < 16; ++i)
		mem->vram_obja_bases[i] = 0xFFFFFFFF;
	for (size_t i = 0; i < 8; ++i)
		mem->vram_objb_bases[i] = 0xFFFFFFFF;
	for (size_t i = 0; i < 2; ++i)
		mem->vram_arm7_bases[i] = 0xFFFFFFFF;
	for (size_t i = 0; i < 2; ++i)
		mem->vram_bgepa_bases[i] = 0xFFFFFFFF;
	mem->vram_bgepb_base = 0xFFFFFFFF;
	mem->vram_objepa_base = 0xFFFFFFFF;
	mem->vram_objepb_base = 0xFFFFFFFF;
	for (size_t i = 0; i < 4; ++i)
		mem->vram_trpi_bases[i] = 0xFFFFFFFF;
	for (size_t i = 0; i < 8; ++i)
		mem->vram_texp_bases[i] = 0xFFFFFFFF;
	uint8_t cnt_a = mem_arm9_get_reg8(mem, MEM_ARM9_REG_VRAMCNT_A);
#if 0
	printf("VRAMCNT_A = %02" PRIx8 "\n", cnt_a);
#endif
	if (cnt_a & (1 << 7))
	{
		uint8_t ofs_a = (cnt_a >> 3) & 0x3;
		switch (cnt_a & 0x3)
		{
			case 0:
				break;
			case 1:
				for (size_t i = 0; i < 8; ++i)
					mem->vram_bga_bases[ofs_a * 8 + i] = MEM_VRAM_A_BASE + i * 0x4000;
				break;
			case 2:
				ofs_a &= 1;
				for (size_t i = 0; i < 8; ++i)
					mem->vram_obja_bases[ofs_a * 8 + i] = MEM_VRAM_A_BASE + i * 0x4000;
				break;
			case 3:
				mem->vram_trpi_bases[ofs_a] = MEM_VRAM_A_BASE;
				break;
		}
	}
	uint8_t cnt_b = mem_arm9_get_reg8(mem, MEM_ARM9_REG_VRAMCNT_B);
#if 0
	printf("VRAMCNT_B = %02" PRIx8 "\n", cnt_b);
#endif
	if (cnt_b & (1 << 7))
	{
		uint8_t ofs_b = (cnt_b >> 3) & 0x3;
		switch (cnt_b & 0x3)
		{
			case 0:
				break;
			case 1:
				for (size_t i = 0; i < 8; ++i)
					mem->vram_bga_bases[ofs_b * 8 + i] = MEM_VRAM_B_BASE + i * 0x4000;
				break;
			case 2:
				ofs_b &= 1;
				for (size_t i = 0; i < 8; ++i)
					mem->vram_obja_bases[ofs_b * 8 + i] = MEM_VRAM_B_BASE + i * 0x4000;
				break;
			case 3:
				mem->vram_trpi_bases[ofs_b] = MEM_VRAM_B_BASE;
				break;
		}
	}
	uint8_t cnt_c = mem_arm9_get_reg8(mem, MEM_ARM9_REG_VRAMCNT_C);
#if 0
	printf("VRAMCNT_C = %02" PRIx8 "\n", cnt_c);
#endif
	if (cnt_c & (1 << 7))
	{
		uint8_t ofs_c = (cnt_c >> 3) & 0x3;
		switch (cnt_c & 0x7)
		{
			case 0:
				break;
			case 1:
				for (size_t i = 0; i < 8; ++i)
					mem->vram_bga_bases[ofs_c * 8 + i] = MEM_VRAM_C_BASE + i * 0x4000;
				break;
			case 2:
				mem->vram_arm7_bases[ofs_c & 1] = MEM_VRAM_C_BASE + (ofs_c & 1) * 0x20000;
				break;
			case 3:
				mem->vram_trpi_bases[ofs_c] = MEM_VRAM_C_BASE;
				break;
			case 4:
				for (size_t i = 0; i < 8; ++i)
					mem->vram_bgb_bases[i] = MEM_VRAM_C_BASE + i * 0x4000;
				break;
			case 5:
			case 6:
			case 7:
				break;
		}
	}
	uint8_t cnt_d = mem_arm9_get_reg8(mem, MEM_ARM9_REG_VRAMCNT_D);
#if 0
	printf("VRAMCNT_D = %02" PRIx8 "\n", cnt_d);
#endif
	if (cnt_d & (1 << 7))
	{
		uint8_t ofs_d = (cnt_d >> 3) & 0x3;
		switch (cnt_d & 0x7)
		{
			case 0:
				break;
			case 1:
				for (size_t i = 0; i < 8; ++i)
					mem->vram_bga_bases[ofs_d * 8 + i] = MEM_VRAM_D_BASE + i * 0x4000;
				break;
			case 2:
				mem->vram_arm7_bases[ofs_d & 1] = MEM_VRAM_D_BASE + (ofs_d & 1) * 0x20000;
				break;
			case 3:
				mem->vram_trpi_bases[ofs_d] = MEM_VRAM_D_BASE;
				break;
			case 4:
				for (size_t i = 0; i < 8; ++i)
					mem->vram_objb_bases[i] = MEM_VRAM_D_BASE + i * 0x4000;
				break;
			case 5:
			case 6:
			case 7:
				break;
		}
	}
	uint8_t cnt_e = mem_arm9_get_reg8(mem, MEM_ARM9_REG_VRAMCNT_E);
#if 0
	printf("VRAMCNT_E = %02" PRIx8 "\n", cnt_e);
#endif
	if (cnt_e & (1 << 7))
	{
		switch (cnt_e & 0x7)
		{
			case 0:
				break;
			case 1:
				for (size_t i = 0; i < 4; ++i)
					mem->vram_bga_bases[i] = MEM_VRAM_E_BASE + i * 0x4000;
				break;
			case 2:
				for (size_t i = 0; i < 4; ++i)
					mem->vram_obja_bases[i] = MEM_VRAM_E_BASE + i * 0x4000;
				break;
			case 3:
				for (size_t i = 0; i < 4; ++i)
					mem->vram_texp_bases[i] = MEM_VRAM_E_BASE + i * 0x4000;
				break;
			case 4:
				for (size_t i = 0; i < 2; ++i)
					mem->vram_bgepa_bases[i] = MEM_VRAM_E_BASE + i * 0x4000;
				break;
			case 5:
			case 6:
			case 7:
				break;
		}
	}
	uint8_t cnt_f = mem_arm9_get_reg8(mem, MEM_ARM9_REG_VRAMCNT_F);
#if 0
	printf("VRAMCNT_F = %02" PRIx8 "\n", cnt_f);
#endif
	if (cnt_f & (1 << 7))
	{
		uint8_t ofs_f = (cnt_f >> 3) & 0x3;
		switch (cnt_f & 0x7)
		{
			case 0:
				break;
			case 1:
				mem->vram_bga_bases[(ofs_f & 1) | ((ofs_f & 2) << 1)] = MEM_VRAM_F_BASE;
				break;
			case 2:
				mem->vram_obja_bases[(ofs_f & 1) | ((ofs_f & 2) << 1)] = MEM_VRAM_F_BASE;
				break;
			case 3:
				mem->vram_texp_bases[(ofs_f & 1) | ((ofs_f & 2) << 1)] = MEM_VRAM_F_BASE;
				break;
			case 4:
				mem->vram_bgepa_bases[ofs_f & 1] = MEM_VRAM_F_BASE;
				break;
			case 5:
				mem->vram_objepa_base = MEM_VRAM_F_BASE;
				break;
			case 6:
			case 7:
				break;
		}
	}
	uint8_t cnt_g = mem_arm9_get_reg8(mem, MEM_ARM9_REG_VRAMCNT_G);
#if 0
	printf("VRAMCNT_G = %02" PRIx8 "\n", cnt_g);
#endif
	if (cnt_g & (1 << 7))
	{
		uint8_t ofs_g = (cnt_g >> 3) & 0x3;
		switch (cnt_g & 0x7)
		{
			case 0:
				break;
			case 1:
				mem->vram_bga_bases[(ofs_g & 1) | ((ofs_g & 2) << 1)] = MEM_VRAM_G_BASE;
				break;
			case 2:
				mem->vram_obja_bases[(ofs_g & 1) | ((ofs_g & 2) << 1)] = MEM_VRAM_G_BASE;
				break;
			case 3:
				mem->vram_texp_bases[(ofs_g & 1) | ((ofs_g & 2) << 1)] = MEM_VRAM_G_BASE;
				break;
			case 4:
				mem->vram_bgepa_bases[ofs_g & 1] = MEM_VRAM_G_BASE;
				break;
			case 5:
				mem->vram_objepa_base = MEM_VRAM_G_BASE;
				break;
			case 6:
			case 7:
				break;
		}
	}
	uint8_t cnt_h = mem_arm9_get_reg8(mem, MEM_ARM9_REG_VRAMCNT_H);
#if 0
	printf("VRAMCNT_H = %02" PRIx8 "\n", cnt_h);
#endif
	if (cnt_h & (1 << 7))
	{
		switch (cnt_h & 0x3)
		{
			case 0:
				break;
			case 1:
				mem->vram_bgb_bases[0] = MEM_VRAM_H_BASE;
				mem->vram_bgb_bases[1] = MEM_VRAM_H_BASE + 0x4000;
				break;
			case 2:
				mem->vram_bgepb_base = MEM_VRAM_H_BASE;
				break;
			case 3:
				break;
		}
	}
	uint8_t cnt_i = mem_arm9_get_reg8(mem, MEM_ARM9_REG_VRAMCNT_I);
#if 0
	printf("VRAMCNT_I = %02" PRIx8 "\n", cnt_i);
#endif
	if (cnt_i & (1 << 7))
	{
		switch (cnt_i & 0x3)
		{
			case 0:
				break;
			case 1:
				mem->vram_bgb_bases[2] = MEM_VRAM_I_BASE;
				break;
			case 2:
				mem->vram_objb_bases[0] = MEM_VRAM_I_BASE;
				break;
			case 3:
				mem->vram_objepb_base = MEM_VRAM_I_BASE;
				break;
		}
	}
}

static void commit_gx_cmd(struct mem *mem)
{
	struct gx_cmd *cmd = &mem->gx_cmd[mem->gx_cmd_nb - 1];
	mem->gx_cmd_nb--;
	if (cmd->params_nb != cmd->def->params)
	{
#if 1
		printf("[GX] %s doesn't have all the required params (%" PRIu8 " / %" PRIu8 ")\n",
		       cmd->def->name, cmd->params_nb, cmd->params_nb);
#endif
		return;
	}
#if 0
	printf("[GX] execute %s with %u params\n", cmd->def->name, cmd->params_nb);
#endif
	gpu_gx_cmd(mem->nds->gpu, cmd->def - &gx_cmd_defs[0], cmd->params);
}

static void start_gx_cmd(struct mem *mem, const struct gx_cmd_def *def)
{
	struct gx_cmd *cmd = &mem->gx_cmd[mem->gx_cmd_nb++];
	cmd->def = def;
	cmd->params_nb = 0;
#if 0
	printf("[GX] start %s\n", def->name);
#endif
	if (!def->params)
		commit_gx_cmd(mem);
}

static void add_gx_cmd_param(struct mem *mem, uint32_t param)
{
	struct gx_cmd *cmd = &mem->gx_cmd[mem->gx_cmd_nb - 1];
#if 0
	printf("[GX] add param [%" PRIu8 "] = 0x%08" PRIx32 " to %s\n",
	       cmd->params_nb, param, cmd->def->name);
#endif
	cmd->params[cmd->params_nb++] = param;
	if (cmd->params_nb == cmd->def->params)
		commit_gx_cmd(mem);
}

static void set_arm9_reg8(struct mem *mem, uint32_t addr, uint8_t v)
{
	switch (addr)
	{
		case MEM_ARM9_REG_IPCSYNC:
			return;
		case MEM_ARM9_REG_IPCSYNC + 1:
			mem->arm9_regs[addr] = v & 0x47;
			if ((v & (1 << 5))
			 && (mem->arm7_regs[MEM_ARM7_REG_IPCSYNC + 1] & (1 << 6)))
				mem_arm7_irq(mem, 1 << 16);
#if 0
			printf("[ARM9] IPCSYNC write 0x%02" PRIx8 "\n", v);
#endif
			return;
		case MEM_ARM9_REG_IPCSYNC + 2:
		case MEM_ARM9_REG_IPCSYNC + 3:
			return;
		case MEM_ARM9_REG_IE:
		case MEM_ARM9_REG_IE + 1:
		case MEM_ARM9_REG_IE + 2:
		case MEM_ARM9_REG_IE + 3:
			mem->arm9_regs[addr] = v;
#if 0
			printf("[ARM9] IE 0x%08" PRIx32 "\n", mem_arm9_get_reg32(mem, MEM_ARM9_REG_IE));
#endif
			cpu_update_irq_state(mem->nds->arm9);
			return;
		case MEM_ARM9_REG_IME:
		case MEM_ARM9_REG_IME + 1:
		case MEM_ARM9_REG_IME + 2:
		case MEM_ARM9_REG_IME + 3:
		case MEM_ARM9_REG_POSTFLG:
		case MEM_ARM9_REG_POSTFLG + 1:
		case MEM_ARM9_REG_POSTFLG + 2:
		case MEM_ARM9_REG_POSTFLG + 3:
		case MEM_ARM9_REG_ROMCTRL:
		case MEM_ARM9_REG_ROMCTRL + 1:
		case MEM_ARM9_REG_ROMCMD:
		case MEM_ARM9_REG_ROMCMD + 1:
		case MEM_ARM9_REG_ROMCMD + 2:
		case MEM_ARM9_REG_ROMCMD + 3:
		case MEM_ARM9_REG_ROMCMD + 4:
		case MEM_ARM9_REG_ROMCMD + 5:
		case MEM_ARM9_REG_ROMCMD + 6:
		case MEM_ARM9_REG_ROMCMD + 7:
		case MEM_ARM9_REG_TM0CNT_L:
		case MEM_ARM9_REG_TM0CNT_L + 1:
		case MEM_ARM9_REG_TM0CNT_H + 1:
		case MEM_ARM9_REG_TM1CNT_L:
		case MEM_ARM9_REG_TM1CNT_L + 1:
		case MEM_ARM9_REG_TM1CNT_H + 1:
		case MEM_ARM9_REG_TM2CNT_L:
		case MEM_ARM9_REG_TM2CNT_L + 1:
		case MEM_ARM9_REG_TM2CNT_H + 1:
		case MEM_ARM9_REG_TM3CNT_L:
		case MEM_ARM9_REG_TM3CNT_L + 1:
		case MEM_ARM9_REG_TM3CNT_H + 1:
		case MEM_ARM9_REG_EXMEMCNT:
		case MEM_ARM9_REG_EXMEMCNT + 1:
		case MEM_ARM9_REG_DMA0SAD:
		case MEM_ARM9_REG_DMA0SAD + 1:
		case MEM_ARM9_REG_DMA0SAD + 2:
		case MEM_ARM9_REG_DMA0SAD + 3:
		case MEM_ARM9_REG_DMA0DAD:
		case MEM_ARM9_REG_DMA0DAD + 1:
		case MEM_ARM9_REG_DMA0DAD + 2:
		case MEM_ARM9_REG_DMA0DAD + 3:
		case MEM_ARM9_REG_DMA0CNT_L:
		case MEM_ARM9_REG_DMA0CNT_L + 1:
		case MEM_ARM9_REG_DMA0CNT_H:
		case MEM_ARM9_REG_DMA1SAD:
		case MEM_ARM9_REG_DMA1SAD + 1:
		case MEM_ARM9_REG_DMA1SAD + 2:
		case MEM_ARM9_REG_DMA1SAD + 3:
		case MEM_ARM9_REG_DMA1DAD:
		case MEM_ARM9_REG_DMA1DAD + 1:
		case MEM_ARM9_REG_DMA1DAD + 2:
		case MEM_ARM9_REG_DMA1DAD + 3:
		case MEM_ARM9_REG_DMA1CNT_L:
		case MEM_ARM9_REG_DMA1CNT_L + 1:
		case MEM_ARM9_REG_DMA1CNT_H:
		case MEM_ARM9_REG_DMA2SAD:
		case MEM_ARM9_REG_DMA2SAD + 1:
		case MEM_ARM9_REG_DMA2SAD + 2:
		case MEM_ARM9_REG_DMA2SAD + 3:
		case MEM_ARM9_REG_DMA2DAD:
		case MEM_ARM9_REG_DMA2DAD + 1:
		case MEM_ARM9_REG_DMA2DAD + 2:
		case MEM_ARM9_REG_DMA2DAD + 3:
		case MEM_ARM9_REG_DMA2CNT_L:
		case MEM_ARM9_REG_DMA2CNT_L + 1:
		case MEM_ARM9_REG_DMA2CNT_H:
		case MEM_ARM9_REG_DMA3CNT_L:
		case MEM_ARM9_REG_DMA3CNT_L  +1:
		case MEM_ARM9_REG_DMA3CNT_H:
		case MEM_ARM9_REG_DMA0FILL:
		case MEM_ARM9_REG_DMA0FILL + 1:
		case MEM_ARM9_REG_DMA0FILL + 2:
		case MEM_ARM9_REG_DMA0FILL + 3:
		case MEM_ARM9_REG_DMA1FILL:
		case MEM_ARM9_REG_DMA1FILL + 1:
		case MEM_ARM9_REG_DMA1FILL + 2:
		case MEM_ARM9_REG_DMA1FILL + 3:
		case MEM_ARM9_REG_DMA2FILL:
		case MEM_ARM9_REG_DMA2FILL + 1:
		case MEM_ARM9_REG_DMA2FILL + 2:
		case MEM_ARM9_REG_DMA2FILL + 3:
		case MEM_ARM9_REG_DMA3FILL:
		case MEM_ARM9_REG_DMA3FILL + 1:
		case MEM_ARM9_REG_DMA3FILL + 2:
		case MEM_ARM9_REG_DMA3FILL + 3:
		case MEM_ARM9_REG_POWCNT1:
		case MEM_ARM9_REG_POWCNT1 + 1:
		case MEM_ARM9_REG_POWCNT1 + 2:
		case MEM_ARM9_REG_POWCNT1 + 3:
		case MEM_ARM9_REG_DISPCNT:
		case MEM_ARM9_REG_DISPCNT + 1:
		case MEM_ARM9_REG_DISPCNT + 2:
		case MEM_ARM9_REG_DISPCNT + 3:
		case MEM_ARM9_REG_DISPCNT + 0x1000:
		case MEM_ARM9_REG_DISPCNT + 0x1000 + 1:
		case MEM_ARM9_REG_DISPCNT + 0x1000 + 2:
		case MEM_ARM9_REG_DISPCNT + 0x1000 + 3:
		case MEM_ARM9_REG_MASTER_BRIGHT:
		case MEM_ARM9_REG_MASTER_BRIGHT + 1:
		case MEM_ARM9_REG_MASTER_BRIGHT + 2:
		case MEM_ARM9_REG_MASTER_BRIGHT + 3:
		case MEM_ARM9_REG_MASTER_BRIGHT + 0x1000:
		case MEM_ARM9_REG_MASTER_BRIGHT + 0x1000 + 1:
		case MEM_ARM9_REG_MASTER_BRIGHT + 0x1000 + 2:
		case MEM_ARM9_REG_MASTER_BRIGHT + 0x1000 + 3:
		case MEM_ARM9_REG_BG0CNT:
		case MEM_ARM9_REG_BG0CNT + 1:
		case MEM_ARM9_REG_BG0CNT + 0x1000:
		case MEM_ARM9_REG_BG0CNT + 0x1000 + 1:
		case MEM_ARM9_REG_BG1CNT:
		case MEM_ARM9_REG_BG1CNT + 1:
		case MEM_ARM9_REG_BG1CNT + 0x1000:
		case MEM_ARM9_REG_BG1CNT + 0x1000 + 1:
		case MEM_ARM9_REG_BG2CNT:
		case MEM_ARM9_REG_BG2CNT + 1:
		case MEM_ARM9_REG_BG2CNT + 0x1000:
		case MEM_ARM9_REG_BG2CNT + 0x1000 + 1:
		case MEM_ARM9_REG_BG3CNT:
		case MEM_ARM9_REG_BG3CNT + 1:
		case MEM_ARM9_REG_BG3CNT + 0x1000:
		case MEM_ARM9_REG_BG3CNT + 0x1000 + 1:
		case MEM_ARM9_REG_BG0HOFS:
		case MEM_ARM9_REG_BG0HOFS + 1:
		case MEM_ARM9_REG_BG0HOFS + 0x1000:
		case MEM_ARM9_REG_BG0HOFS + 0x1000 + 1:
		case MEM_ARM9_REG_BG0VOFS:
		case MEM_ARM9_REG_BG0VOFS + 1:
		case MEM_ARM9_REG_BG0VOFS + 0x1000:
		case MEM_ARM9_REG_BG0VOFS + 0x1000 + 1:
		case MEM_ARM9_REG_BG1HOFS:
		case MEM_ARM9_REG_BG1HOFS + 1:
		case MEM_ARM9_REG_BG1HOFS + 0x1000:
		case MEM_ARM9_REG_BG1HOFS + 0x1000 + 1:
		case MEM_ARM9_REG_BG1VOFS:
		case MEM_ARM9_REG_BG1VOFS + 1:
		case MEM_ARM9_REG_BG1VOFS + 0x1000:
		case MEM_ARM9_REG_BG1VOFS + 0x1000 + 1:
		case MEM_ARM9_REG_BG2HOFS:
		case MEM_ARM9_REG_BG2HOFS + 1:
		case MEM_ARM9_REG_BG2HOFS + 0x1000:
		case MEM_ARM9_REG_BG2HOFS + 0x1000 + 1:
		case MEM_ARM9_REG_BG2VOFS:
		case MEM_ARM9_REG_BG2VOFS + 1:
		case MEM_ARM9_REG_BG2VOFS + 0x1000:
		case MEM_ARM9_REG_BG2VOFS + 0x1000 + 1:
		case MEM_ARM9_REG_BG3HOFS:
		case MEM_ARM9_REG_BG3HOFS + 1:
		case MEM_ARM9_REG_BG3HOFS + 0x1000:
		case MEM_ARM9_REG_BG3HOFS + 0x1000 + 1:
		case MEM_ARM9_REG_BG3VOFS:
		case MEM_ARM9_REG_BG3VOFS + 1:
		case MEM_ARM9_REG_BG3VOFS + 0x1000:
		case MEM_ARM9_REG_BG3VOFS + 0x1000 + 1:
		case MEM_ARM9_REG_BG2PA:
		case MEM_ARM9_REG_BG2PA + 1:
		case MEM_ARM9_REG_BG2PA + 0x1000:
		case MEM_ARM9_REG_BG2PA + 0x1000 + 1:
		case MEM_ARM9_REG_BG2PB:
		case MEM_ARM9_REG_BG2PB + 1:
		case MEM_ARM9_REG_BG2PB + 0x1000:
		case MEM_ARM9_REG_BG2PB + 0x1000 + 1:
		case MEM_ARM9_REG_BG2PC:
		case MEM_ARM9_REG_BG2PC + 1:
		case MEM_ARM9_REG_BG2PC + 0x1000:
		case MEM_ARM9_REG_BG2PC + 0x1000 + 1:
		case MEM_ARM9_REG_BG2PD:
		case MEM_ARM9_REG_BG2PD + 1:
		case MEM_ARM9_REG_BG2PD + 0x1000:
		case MEM_ARM9_REG_BG2PD + 0x1000 + 1:
		case MEM_ARM9_REG_BG2X:
		case MEM_ARM9_REG_BG2X + 1:
		case MEM_ARM9_REG_BG2X + 2:
		case MEM_ARM9_REG_BG2X + 3:
		case MEM_ARM9_REG_BG2X + 0x1000:
		case MEM_ARM9_REG_BG2X + 0x1000 + 1:
		case MEM_ARM9_REG_BG2X + 0x1000 + 2:
		case MEM_ARM9_REG_BG2X + 0x1000 + 3:
		case MEM_ARM9_REG_BG2Y:
		case MEM_ARM9_REG_BG2Y + 1:
		case MEM_ARM9_REG_BG2Y + 2:
		case MEM_ARM9_REG_BG2Y + 3:
		case MEM_ARM9_REG_BG2Y + 0x1000:
		case MEM_ARM9_REG_BG2Y + 0x1000 + 1:
		case MEM_ARM9_REG_BG2Y + 0x1000 + 2:
		case MEM_ARM9_REG_BG2Y + 0x1000 + 3:
		case MEM_ARM9_REG_BG3PA:
		case MEM_ARM9_REG_BG3PA + 1:
		case MEM_ARM9_REG_BG3PA + 0x1000:
		case MEM_ARM9_REG_BG3PA + 0x1000 + 1:
		case MEM_ARM9_REG_BG3PB:
		case MEM_ARM9_REG_BG3PB + 1:
		case MEM_ARM9_REG_BG3PB + 0x1000:
		case MEM_ARM9_REG_BG3PB + 0x1000 + 1:
		case MEM_ARM9_REG_BG3PC:
		case MEM_ARM9_REG_BG3PC + 1:
		case MEM_ARM9_REG_BG3PC + 0x1000:
		case MEM_ARM9_REG_BG3PC + 0x1000 + 1:
		case MEM_ARM9_REG_BG3PD:
		case MEM_ARM9_REG_BG3PD + 1:
		case MEM_ARM9_REG_BG3PD + 0x1000:
		case MEM_ARM9_REG_BG3PD + 0x1000 + 1:
		case MEM_ARM9_REG_BG3X:
		case MEM_ARM9_REG_BG3X + 1:
		case MEM_ARM9_REG_BG3X + 2:
		case MEM_ARM9_REG_BG3X + 3:
		case MEM_ARM9_REG_BG3X + 0x1000:
		case MEM_ARM9_REG_BG3X + 0x1000 + 1:
		case MEM_ARM9_REG_BG3X + 0x1000 + 2:
		case MEM_ARM9_REG_BG3X + 0x1000 + 3:
		case MEM_ARM9_REG_BG3Y:
		case MEM_ARM9_REG_BG3Y + 1:
		case MEM_ARM9_REG_BG3Y + 2:
		case MEM_ARM9_REG_BG3Y + 3:
		case MEM_ARM9_REG_BG3Y + 0x1000:
		case MEM_ARM9_REG_BG3Y + 0x1000 + 1:
		case MEM_ARM9_REG_BG3Y + 0x1000 + 2:
		case MEM_ARM9_REG_BG3Y + 0x1000 + 3:
		case MEM_ARM9_REG_WIN0H:
		case MEM_ARM9_REG_WIN0H + 1:
		case MEM_ARM9_REG_WIN0H + 0x1000:
		case MEM_ARM9_REG_WIN0H + 0x1000 + 1:
		case MEM_ARM9_REG_WIN1H:
		case MEM_ARM9_REG_WIN1H + 1:
		case MEM_ARM9_REG_WIN1H + 0x1000:
		case MEM_ARM9_REG_WIN1H + 0x1000 + 1:
		case MEM_ARM9_REG_WIN0V:
		case MEM_ARM9_REG_WIN0V + 1:
		case MEM_ARM9_REG_WIN0V + 0x1000:
		case MEM_ARM9_REG_WIN0V + 0x1000 + 1:
		case MEM_ARM9_REG_WIN1V:
		case MEM_ARM9_REG_WIN1V + 1:
		case MEM_ARM9_REG_WIN1V + 0x1000:
		case MEM_ARM9_REG_WIN1V + 0x1000 + 1:
		case MEM_ARM9_REG_WININ:
		case MEM_ARM9_REG_WININ + 1:
		case MEM_ARM9_REG_WININ + 0x1000:
		case MEM_ARM9_REG_WININ + 0x1000 + 1:
		case MEM_ARM9_REG_WINOUT:
		case MEM_ARM9_REG_WINOUT + 1:
		case MEM_ARM9_REG_WINOUT + 0x1000:
		case MEM_ARM9_REG_WINOUT + 0x1000 + 1:
		case MEM_ARM9_REG_MOSAIC:
		case MEM_ARM9_REG_MOSAIC + 1:
		case MEM_ARM9_REG_MOSAIC + 2:
		case MEM_ARM9_REG_MOSAIC + 3:
		case MEM_ARM9_REG_MOSAIC + 0x1000:
		case MEM_ARM9_REG_MOSAIC + 0x1000 + 1:
		case MEM_ARM9_REG_MOSAIC + 0x1000 + 2:
		case MEM_ARM9_REG_MOSAIC + 0x1000 + 3:
		case MEM_ARM9_REG_BLDCNT:
		case MEM_ARM9_REG_BLDCNT + 1:
		case MEM_ARM9_REG_BLDCNT + 0x1000:
		case MEM_ARM9_REG_BLDCNT + 0x1000 + 1:
		case MEM_ARM9_REG_BLDALPHA:
		case MEM_ARM9_REG_BLDALPHA + 1:
		case MEM_ARM9_REG_BLDALPHA + 0x1000:
		case MEM_ARM9_REG_BLDALPHA + 0x1000 + 1:
		case MEM_ARM9_REG_BLDY:
		case MEM_ARM9_REG_BLDY + 1:
		case MEM_ARM9_REG_BLDY + 2:
		case MEM_ARM9_REG_BLDY + 3:
		case MEM_ARM9_REG_BLDY + 0x1000:
		case MEM_ARM9_REG_BLDY + 0x1000 + 1:
		case MEM_ARM9_REG_BLDY + 0x1000 + 2:
		case MEM_ARM9_REG_BLDY + 0x1000 + 3:
		case MEM_ARM9_REG_DISP3DCNT:
		case MEM_ARM9_REG_DISP3DCNT + 1:
		case MEM_ARM9_REG_DISP3DCNT + 2:
		case MEM_ARM9_REG_DISP3DCNT + 3:
		case MEM_ARM9_REG_DISPCAPCNT:
		case MEM_ARM9_REG_DISPCAPCNT + 1:
		case MEM_ARM9_REG_DISPCAPCNT + 2:
		case MEM_ARM9_REG_DISPCAPCNT + 3:
		case MEM_ARM9_REG_ALPHA_TEST_REF:
		case MEM_ARM9_REG_ALPHA_TEST_REF + 1:
		case MEM_ARM9_REG_ALPHA_TEST_REF + 2:
		case MEM_ARM9_REG_ALPHA_TEST_REF + 3:
		case MEM_ARM9_REG_CLEAR_COLOR:
		case MEM_ARM9_REG_CLEAR_COLOR + 1:
		case MEM_ARM9_REG_CLEAR_COLOR + 2:
		case MEM_ARM9_REG_CLEAR_COLOR + 3:
		case MEM_ARM9_REG_CLEAR_DEPTH:
		case MEM_ARM9_REG_CLEAR_DEPTH + 1:
		case MEM_ARM9_REG_CLEAR_DEPTH + 2:
		case MEM_ARM9_REG_CLEAR_DEPTH + 3:
		case MEM_ARM9_REG_FOG_COLOR:
		case MEM_ARM9_REG_FOG_COLOR + 1:
		case MEM_ARM9_REG_FOG_COLOR + 2:
		case MEM_ARM9_REG_FOG_COLOR + 3:
		case MEM_ARM9_REG_FOG_OFFSET:
		case MEM_ARM9_REG_FOG_OFFSET + 1:
		case MEM_ARM9_REG_FOG_OFFSET + 2:
		case MEM_ARM9_REG_FOG_OFFSET + 3:
			mem->arm9_regs[addr] = v;
			return;
		case MEM_ARM9_REG_DISPSTAT:
#if 0
			printf("[ARM9] DISPSTAT[%08" PRIx32 "] = %02" PRIx8 "\n",
			       addr, v);
#endif
			mem->arm9_regs[addr] = v & 0xB8;
			return;
		case MEM_ARM9_REG_DISPSTAT + 1:
#if 0
			printf("[ARM9] DISPSTAT[%08" PRIx32 "] = %02" PRIx8 "\n",
			       addr, v);
#endif
			mem->arm9_regs[addr] = v;
			return;
		case MEM_ARM9_REG_AUXSPICNT:
#if 0
			printf("[ARM9] AUXSPICNT[%08" PRIx32 "] = %02" PRIx8 "\n",
			       addr, v);
#endif
			mem->arm9_regs[addr] = v & ~(1 << 7);
			return;
		case MEM_ARM9_REG_AUXSPICNT + 1:
#if 0
			printf("[ARM9] AUXSPICNT[%08" PRIx32 "] = %02" PRIx8 "\n",
			       addr, v);
#endif
			mem->arm9_regs[addr] = v;
			return;
		case MEM_ARM9_REG_DMA3SAD:
		case MEM_ARM9_REG_DMA3SAD + 1:
		case MEM_ARM9_REG_DMA3SAD + 2:
		case MEM_ARM9_REG_DMA3SAD + 3:
#if 0
			printf("[ARM9] [%08" PRIx32 "] DMA3SAD[%08" PRIx32 "] = %02" PRIx8 "\n",
			       cpu_get_reg(mem->nds->arm9, CPU_REG_PC), addr, v);
#endif
			mem->arm9_regs[addr] = v;
			return;
		case MEM_ARM9_REG_DMA3DAD:
		case MEM_ARM9_REG_DMA3DAD + 1:
		case MEM_ARM9_REG_DMA3DAD + 2:
		case MEM_ARM9_REG_DMA3DAD + 3:
#if 0
			printf("[ARM9] [%08" PRIx32 "] DMA3DAD[%08" PRIx32 "] = %02" PRIx8 "\n",
			       cpu_get_reg(mem->nds->arm9, CPU_REG_PC), addr, v);
#endif
			mem->arm9_regs[addr] = v;
			return;
		case MEM_ARM7_REG_AUXSPIDATA:
			auxspi_write(mem, v);
			return;
		case MEM_ARM9_REG_ROMCTRL + 2:
			mem->arm9_regs[addr] = (mem->arm9_regs[addr] & (1 << 7))
			                     | (v & ~(1 << 7));
			return;
		case MEM_ARM9_REG_ROMCTRL + 3:
			mem->arm9_regs[addr] = v;
			if (v & 0x80)
			{
#if 0
				printf("[ARM9] MBC cmd %02" PRIx8 "\n", v);
#endif
				mbc_cmd(mem->mbc);
			}
			return;
		case MEM_ARM9_REG_ROMDATA:
		case MEM_ARM9_REG_ROMDATA + 1:
		case MEM_ARM9_REG_ROMDATA + 2:
		case MEM_ARM9_REG_ROMDATA + 3:
#if 0
			printf("[ARM9] MBC write %02" PRIx8 "\n", v);
#endif
			mbc_write(mem->mbc, v);
			return;
		case MEM_ARM9_REG_IF:
		case MEM_ARM9_REG_IF + 1:
		case MEM_ARM9_REG_IF + 3:
			mem->arm9_regs[addr] &= ~v;
			if (v)
				cpu_update_irq_state(mem->nds->arm9);
			return;
		case MEM_ARM9_REG_IF + 2:
			v &= ~(1 << 5);
			mem->arm9_regs[addr] &= ~v;
			if (v)
				cpu_update_irq_state(mem->nds->arm9);
			return;
		case MEM_ARM9_REG_TM0CNT_H:
			arm9_timer_control(mem, 0, v);
			return;
		case MEM_ARM9_REG_TM1CNT_H:
			arm9_timer_control(mem, 1, v);
			return;
		case MEM_ARM9_REG_TM2CNT_H:
			arm9_timer_control(mem, 2, v);
			return;
		case MEM_ARM9_REG_TM3CNT_H:
			arm9_timer_control(mem, 3, v);
			return;
		case MEM_ARM9_REG_WRAMCNT:
#if 0
			printf("[ARM9] WRAMCNT = %02" PRIx8 "\n", v);
#endif
			v &= 3;
			switch (v)
			{
				case 0:
					mem->arm7_wram_base = 0;
					mem->arm7_wram_mask = 0;
					mem->arm9_wram_base = 0;
					mem->arm9_wram_mask = 0x7FFF;
					break;
				case 1:
					mem->arm7_wram_base = 0x4000;
					mem->arm7_wram_mask = 0x3FFF;
					mem->arm9_wram_base = 0;
					mem->arm9_wram_mask = 0x3FFF;
					break;
				case 2:
					mem->arm7_wram_base = 0;
					mem->arm7_wram_mask = 0x3FFF;
					mem->arm9_wram_base = 0x4000;
					mem->arm9_wram_mask = 0x3FFF;
					break;
				case 3:
					mem->arm7_wram_base = 0;
					mem->arm7_wram_mask = 0x7FFF;
					mem->arm9_wram_base = 0;
					mem->arm9_wram_mask = 0;
					break;
			}
			mem->arm9_regs[addr] = v;
			return;
		case MEM_ARM9_REG_KEYCNT:
		case MEM_ARM9_REG_KEYCNT + 1:
			mem->arm9_regs[addr] = v;
			nds_test_keypad_int(mem->nds);
			return;
		case MEM_ARM9_REG_IPCFIFOCNT:
#if 0
			printf("[ARM9] FIFOCNT[0] write %02" PRIx8 "\n", v);
#endif
			if (v & (1 << 3))
			{
#if 0
				printf("[ARM7] FIFO clean\n");
#endif
				mem->arm7_fifo.len = 0;
				mem->arm7_fifo.latch[0] = 0;
				mem->arm7_fifo.latch[1] = 0;
				mem->arm7_fifo.latch[2] = 0;
				mem->arm7_fifo.latch[3] = 0;
			}
			mem->arm9_regs[addr] &= ~(1 << 2);
			mem->arm9_regs[addr] |= v & (1 << 2);
			return;
		case MEM_ARM9_REG_IPCFIFOCNT + 1:
#if 0
			printf("[ARM9] FIFOCNT[1] write %02" PRIx8 "\n", v);
#endif
			mem->arm9_regs[addr] &= ~0x84;
			mem->arm9_regs[addr] |= v & 0x84;
			if (v & (1 << 6))
				mem->arm9_regs[addr] &= ~(1 << 6);
			return;
		case MEM_ARM9_REG_IPCFIFOSEND:
		case MEM_ARM9_REG_IPCFIFOSEND + 1:
		case MEM_ARM9_REG_IPCFIFOSEND + 2:
		case MEM_ARM9_REG_IPCFIFOSEND + 3:
			if (!(mem->arm7_regs[MEM_ARM7_REG_IPCFIFOCNT + 1] & (1 << 7)))
				return;
			if (mem->arm7_fifo.len == 64)
			{
#if 0
				printf("[ARM7] FIFO full\n");
#endif
				mem->arm9_regs[MEM_ARM9_REG_IPCFIFOCNT + 1] |= (1 << 6);
				return;
			}
			mem->arm7_fifo.data[(mem->arm7_fifo.pos + mem->arm7_fifo.len) % 64] = v;
			mem->arm7_fifo.len++;
			if (mem->arm7_fifo.len == 4
			 && (mem->arm7_regs[MEM_ARM7_REG_IPCFIFOCNT + 1] & (1 << 2)))
				mem_arm7_irq(mem, 1 << 18);
#if 0
			printf("[ARM9] IPCFIFO write (now %" PRIu8 ")\n", mem->arm7_fifo.len);
#endif
			return;
		case MEM_ARM9_REG_DMA0CNT_H + 1:
			mem->arm9_regs[addr] = v;
			arm9_dma_control(mem, 0);
			return;
		case MEM_ARM9_REG_DMA1CNT_H + 1:
			mem->arm9_regs[addr] = v;
			arm9_dma_control(mem, 1);
			return;
		case MEM_ARM9_REG_DMA2CNT_H + 1:
			mem->arm9_regs[addr] = v;
			arm9_dma_control(mem, 2);
			return;
		case MEM_ARM9_REG_DMA3CNT_H + 1:
			mem->arm9_regs[addr] = v;
			arm9_dma_control(mem, 3);
			return;
		case MEM_ARM9_REG_DIVCNT:
		case MEM_ARM9_REG_DIVCNT + 1:
		case MEM_ARM9_REG_DIVCNT + 2:
		case MEM_ARM9_REG_DIVCNT + 3:
		case MEM_ARM9_REG_DIV_NUMER:
		case MEM_ARM9_REG_DIV_NUMER + 1:
		case MEM_ARM9_REG_DIV_NUMER + 2:
		case MEM_ARM9_REG_DIV_NUMER + 3:
		case MEM_ARM9_REG_DIV_NUMER + 4:
		case MEM_ARM9_REG_DIV_NUMER + 5:
		case MEM_ARM9_REG_DIV_NUMER + 6:
		case MEM_ARM9_REG_DIV_NUMER + 7:
		case MEM_ARM9_REG_DIV_DENOM:
		case MEM_ARM9_REG_DIV_DENOM + 1:
		case MEM_ARM9_REG_DIV_DENOM + 2:
		case MEM_ARM9_REG_DIV_DENOM + 3:
		case MEM_ARM9_REG_DIV_DENOM + 4:
		case MEM_ARM9_REG_DIV_DENOM + 5:
		case MEM_ARM9_REG_DIV_DENOM + 6:
		case MEM_ARM9_REG_DIV_DENOM + 7:
#if 0
			printf("[ARM9] DIV[%08" PRIx32 "] = %02" PRIx8 "\n", addr, v);
#endif
			mem->arm9_regs[addr] = v;
			run_div(mem);
			return;
		case MEM_ARM9_REG_VRAMCNT_A:
		case MEM_ARM9_REG_VRAMCNT_B:
		case MEM_ARM9_REG_VRAMCNT_C:
		case MEM_ARM9_REG_VRAMCNT_D:
		case MEM_ARM9_REG_VRAMCNT_E:
		case MEM_ARM9_REG_VRAMCNT_F:
		case MEM_ARM9_REG_VRAMCNT_G:
		case MEM_ARM9_REG_VRAMCNT_H:
		case MEM_ARM9_REG_VRAMCNT_I:
			mem->arm9_regs[addr] = v;
			update_vram_maps(mem);
			return;
		case MEM_ARM9_REG_SQRTCNT + 1:
			v &= ~(1 << 7);
			/* FALLTHROUGH */
		case MEM_ARM9_REG_SQRTCNT:
		case MEM_ARM9_REG_SQRTCNT + 2:
		case MEM_ARM9_REG_SQRTCNT + 3:
		case MEM_ARM9_REG_SQRT_PARAM:
		case MEM_ARM9_REG_SQRT_PARAM + 1:
		case MEM_ARM9_REG_SQRT_PARAM + 2:
		case MEM_ARM9_REG_SQRT_PARAM + 3:
		case MEM_ARM9_REG_SQRT_PARAM + 4:
		case MEM_ARM9_REG_SQRT_PARAM + 5:
		case MEM_ARM9_REG_SQRT_PARAM + 6:
		case MEM_ARM9_REG_SQRT_PARAM + 7:
#if 0
			printf("[ARM9] SQRT[%08" PRIx32 "] = %02" PRIx8 "\n", addr, v);
#endif
			mem->arm9_regs[addr] = v;
			run_sqrt(mem);
			return;
		case MEM_ARM9_REG_KEYINPUT:
		case MEM_ARM9_REG_KEYINPUT + 1:
		case MEM_ARM9_REG_DIV_RESULT:
		case MEM_ARM9_REG_DIV_RESULT + 1:
		case MEM_ARM9_REG_DIV_RESULT + 2:
		case MEM_ARM9_REG_DIV_RESULT + 3:
		case MEM_ARM9_REG_DIV_RESULT + 4:
		case MEM_ARM9_REG_DIV_RESULT + 5:
		case MEM_ARM9_REG_DIV_RESULT + 6:
		case MEM_ARM9_REG_DIV_RESULT + 7:
		case MEM_ARM9_REG_DIVREM_RESULT:
		case MEM_ARM9_REG_DIVREM_RESULT + 1:
		case MEM_ARM9_REG_DIVREM_RESULT + 2:
		case MEM_ARM9_REG_DIVREM_RESULT + 3:
		case MEM_ARM9_REG_DIVREM_RESULT + 4:
		case MEM_ARM9_REG_DIVREM_RESULT + 5:
		case MEM_ARM9_REG_DIVREM_RESULT + 6:
		case MEM_ARM9_REG_DIVREM_RESULT + 7:
		case MEM_ARM9_REG_SQRT_RESULT:
		case MEM_ARM9_REG_SQRT_RESULT + 1:
		case MEM_ARM9_REG_SQRT_RESULT + 2:
		case MEM_ARM9_REG_SQRT_RESULT + 3:
		case MEM_ARM9_REG_GXSTAT:
		case MEM_ARM9_REG_GXSTAT + 2:
			return;
		case MEM_ARM9_REG_GXSTAT + 1:
#if 0
			printf("[ARM9] GXSTAT[%08" PRIx32 "] set %02" PRIx8 "\n", addr, v);
#endif
			if (v & (1 << 7))
				mem->arm9_regs[addr] &= ~((1 << 7) | (1 << 5) | 0x1F);
			return;
		case MEM_ARM9_REG_GXSTAT + 3:
#if 0
			printf("[ARM9] GXSTAT[%08" PRIx32 "] set %02" PRIx8 "\n", addr, v);
#endif
			mem->arm9_regs[addr] &= 0x3F;
			mem->arm9_regs[addr] |= v & 0xC0;
			update_gxfifo_irq(mem);
			return;
		case MEM_ARM9_REG_TOON_TABLE:
		case MEM_ARM9_REG_TOON_TABLE + 1:
		case MEM_ARM9_REG_TOON_TABLE + 2:
		case MEM_ARM9_REG_TOON_TABLE + 3:
		case MEM_ARM9_REG_TOON_TABLE + 4:
		case MEM_ARM9_REG_TOON_TABLE + 5:
		case MEM_ARM9_REG_TOON_TABLE + 6:
		case MEM_ARM9_REG_TOON_TABLE + 7:
		case MEM_ARM9_REG_TOON_TABLE + 8:
		case MEM_ARM9_REG_TOON_TABLE + 9:
		case MEM_ARM9_REG_TOON_TABLE + 10:
		case MEM_ARM9_REG_TOON_TABLE + 11:
		case MEM_ARM9_REG_TOON_TABLE + 12:
		case MEM_ARM9_REG_TOON_TABLE + 13:
		case MEM_ARM9_REG_TOON_TABLE + 14:
		case MEM_ARM9_REG_TOON_TABLE + 15:
		case MEM_ARM9_REG_TOON_TABLE + 16:
		case MEM_ARM9_REG_TOON_TABLE + 17:
		case MEM_ARM9_REG_TOON_TABLE + 18:
		case MEM_ARM9_REG_TOON_TABLE + 19:
		case MEM_ARM9_REG_TOON_TABLE + 20:
		case MEM_ARM9_REG_TOON_TABLE + 21:
		case MEM_ARM9_REG_TOON_TABLE + 22:
		case MEM_ARM9_REG_TOON_TABLE + 23:
		case MEM_ARM9_REG_TOON_TABLE + 24:
		case MEM_ARM9_REG_TOON_TABLE + 25:
		case MEM_ARM9_REG_TOON_TABLE + 26:
		case MEM_ARM9_REG_TOON_TABLE + 27:
		case MEM_ARM9_REG_TOON_TABLE + 28:
		case MEM_ARM9_REG_TOON_TABLE + 29:
		case MEM_ARM9_REG_TOON_TABLE + 30:
		case MEM_ARM9_REG_TOON_TABLE + 31:
		case MEM_ARM9_REG_TOON_TABLE + 32:
		case MEM_ARM9_REG_TOON_TABLE + 33:
		case MEM_ARM9_REG_TOON_TABLE + 34:
		case MEM_ARM9_REG_TOON_TABLE + 35:
		case MEM_ARM9_REG_TOON_TABLE + 36:
		case MEM_ARM9_REG_TOON_TABLE + 37:
		case MEM_ARM9_REG_TOON_TABLE + 38:
		case MEM_ARM9_REG_TOON_TABLE + 39:
		case MEM_ARM9_REG_TOON_TABLE + 40:
		case MEM_ARM9_REG_TOON_TABLE + 41:
		case MEM_ARM9_REG_TOON_TABLE + 42:
		case MEM_ARM9_REG_TOON_TABLE + 43:
		case MEM_ARM9_REG_TOON_TABLE + 44:
		case MEM_ARM9_REG_TOON_TABLE + 45:
		case MEM_ARM9_REG_TOON_TABLE + 46:
		case MEM_ARM9_REG_TOON_TABLE + 47:
		case MEM_ARM9_REG_TOON_TABLE + 48:
		case MEM_ARM9_REG_TOON_TABLE + 49:
		case MEM_ARM9_REG_TOON_TABLE + 50:
		case MEM_ARM9_REG_TOON_TABLE + 51:
		case MEM_ARM9_REG_TOON_TABLE + 52:
		case MEM_ARM9_REG_TOON_TABLE + 53:
		case MEM_ARM9_REG_TOON_TABLE + 54:
		case MEM_ARM9_REG_TOON_TABLE + 55:
		case MEM_ARM9_REG_TOON_TABLE + 56:
		case MEM_ARM9_REG_TOON_TABLE + 57:
		case MEM_ARM9_REG_TOON_TABLE + 58:
		case MEM_ARM9_REG_TOON_TABLE + 59:
		case MEM_ARM9_REG_TOON_TABLE + 60:
		case MEM_ARM9_REG_TOON_TABLE + 61:
		case MEM_ARM9_REG_TOON_TABLE + 62:
		case MEM_ARM9_REG_TOON_TABLE + 63:
			((uint8_t*)&mem->nds->gpu->g3d.toon)[addr - MEM_ARM9_REG_TOON_TABLE] = v;
			return;
		case MEM_ARM9_REG_FOG_TABLE:
		case MEM_ARM9_REG_FOG_TABLE + 1:
		case MEM_ARM9_REG_FOG_TABLE + 2:
		case MEM_ARM9_REG_FOG_TABLE + 3:
		case MEM_ARM9_REG_FOG_TABLE + 4:
		case MEM_ARM9_REG_FOG_TABLE + 5:
		case MEM_ARM9_REG_FOG_TABLE + 6:
		case MEM_ARM9_REG_FOG_TABLE + 7:
		case MEM_ARM9_REG_FOG_TABLE + 8:
		case MEM_ARM9_REG_FOG_TABLE + 9:
		case MEM_ARM9_REG_FOG_TABLE + 10:
		case MEM_ARM9_REG_FOG_TABLE + 11:
		case MEM_ARM9_REG_FOG_TABLE + 12:
		case MEM_ARM9_REG_FOG_TABLE + 13:
		case MEM_ARM9_REG_FOG_TABLE + 14:
		case MEM_ARM9_REG_FOG_TABLE + 15:
		case MEM_ARM9_REG_FOG_TABLE + 16:
		case MEM_ARM9_REG_FOG_TABLE + 17:
		case MEM_ARM9_REG_FOG_TABLE + 18:
		case MEM_ARM9_REG_FOG_TABLE + 19:
		case MEM_ARM9_REG_FOG_TABLE + 20:
		case MEM_ARM9_REG_FOG_TABLE + 21:
		case MEM_ARM9_REG_FOG_TABLE + 22:
		case MEM_ARM9_REG_FOG_TABLE + 23:
		case MEM_ARM9_REG_FOG_TABLE + 24:
		case MEM_ARM9_REG_FOG_TABLE + 25:
		case MEM_ARM9_REG_FOG_TABLE + 26:
		case MEM_ARM9_REG_FOG_TABLE + 27:
		case MEM_ARM9_REG_FOG_TABLE + 28:
		case MEM_ARM9_REG_FOG_TABLE + 29:
		case MEM_ARM9_REG_FOG_TABLE + 30:
		case MEM_ARM9_REG_FOG_TABLE + 31:
			mem->nds->gpu->g3d.fog[addr - MEM_ARM9_REG_FOG_TABLE] = v;
			return;
		case 0x58: /* silent these. they are memset(0) */
		case 0x59:
		case 0x5A:
		case 0x5B:
		case 0x5C:
		case 0x5D:
		case 0x5E:
		case 0x5F:
		case 0x70:
		case 0x71:
		case 0x72:
		case 0x73:
		case 0x74:
		case 0x75:
		case 0x76:
		case 0x77:
		case 0x78:
		case 0x79:
		case 0x7A:
		case 0x7B:
		case 0x7C:
		case 0x7D:
		case 0x7E:
		case 0x7F:
		case 0x80:
		case 0x81:
		case 0x82:
		case 0x83:
		case 0x84:
		case 0x85:
		case 0x86:
		case 0x87:
		case 0x88:
		case 0x89:
		case 0x8A:
		case 0x8B:
		case 0x8C:
		case 0x8D:
		case 0x8E:
		case 0x8F:
		case 0x90:
		case 0x91:
		case 0x92:
		case 0x93:
		case 0x94:
		case 0x95:
		case 0x96:
		case 0x97:
		case 0x98:
		case 0x99:
		case 0x9A:
		case 0x9B:
		case 0x9C:
		case 0x9D:
		case 0x9E:
		case 0x9F:
		case 0xA0:
		case 0xA1:
		case 0xA2:
		case 0xA3:
		case 0xA4:
		case 0xA5:
		case 0xA6:
		case 0xA7:
		case 0xA8:
		case 0xA9:
		case 0xAA:
		case 0xAB:
		case 0xAC:
		case 0xAD:
		case 0xAE:
		case 0xAF:
		case 0xF0:
		case 0xF1:
		case 0xF2:
		case 0xF3:
		case 0xF4:
		case 0xF5:
		case 0xF6:
		case 0xF7:
		case 0xF8:
		case 0xF9:
		case 0xFA:
		case 0xFB:
		case 0xFC:
		case 0xFD:
		case 0xFE:
		case 0xFF:
		case 0x110:
		case 0x111:
		case 0x112:
		case 0x113:
		case 0x114:
		case 0x115:
		case 0x116:
		case 0x117:
		case 0x118:
		case 0x119:
		case 0x11A:
		case 0x11B:
		case 0x11C:
		case 0x11D:
		case 0x11E:
		case 0x11F:
		case 0x120:
		case 0x121:
		case 0x122:
		case 0x123:
		case 0x124:
		case 0x125:
		case 0x126:
		case 0x127:
		case 0x128:
		case 0x129:
		case 0x12A:
		case 0x12B:
		case 0x12C:
		case 0x12D:
		case 0x12E:
		case 0x12F:
		case 0x284:
		case 0x285:
		case 0x286:
		case 0x287:
		case 0x288:
		case 0x289:
		case 0x28A:
		case 0x28B:
		case 0x28C:
		case 0x28D:
		case 0x28E:
		case 0x28F:
		case 0x1004:
		case 0x1005:
		case 0x1006:
		case 0x1007:
		case 0x1058:
		case 0x1059:
		case 0x105A:
		case 0x105B:
		case 0x105C:
		case 0x105D:
		case 0x105E:
		case 0x105F:
		case 0x1060:
		case 0x1061:
		case 0x1062:
		case 0x1063:
		case 0x1064:
		case 0x1065:
		case 0x1066:
		case 0x1067:
		case 0x1068:
		case 0x1069:
		case 0x106A:
		case 0x106B:
			return;
		default:
			printf("[ARM9] [%08" PRIx32 "] unknown set register %08" PRIx32 " = %02" PRIx8 "\n",
			       cpu_get_reg(mem->nds->arm9, CPU_REG_PC), addr, v);
			break;
	}
}

static void set_arm9_reg16(struct mem *mem, uint32_t addr, uint16_t v)
{
	set_arm9_reg8(mem, addr + 0, v >> 0);
	set_arm9_reg8(mem, addr + 1, v >> 8);
}

static void set_arm9_reg32(struct mem *mem, uint32_t addr, uint32_t v)
{
	switch (addr)
	{
		case MEM_ARM9_REG_GXFIFO + 0x00:
		case MEM_ARM9_REG_GXFIFO + 0x04:
		case MEM_ARM9_REG_GXFIFO + 0x08:
		case MEM_ARM9_REG_GXFIFO + 0x0C:
		case MEM_ARM9_REG_GXFIFO + 0x10:
		case MEM_ARM9_REG_GXFIFO + 0x14:
		case MEM_ARM9_REG_GXFIFO + 0x18:
		case MEM_ARM9_REG_GXFIFO + 0x1C:
		case MEM_ARM9_REG_GXFIFO + 0x20:
		case MEM_ARM9_REG_GXFIFO + 0x24:
		case MEM_ARM9_REG_GXFIFO + 0x28:
		case MEM_ARM9_REG_GXFIFO + 0x2C:
		case MEM_ARM9_REG_GXFIFO + 0x30:
		case MEM_ARM9_REG_GXFIFO + 0x34:
		case MEM_ARM9_REG_GXFIFO + 0x38:
		case MEM_ARM9_REG_GXFIFO + 0x3C:
#if 0
			printf("[GX] FIFO write 0x%08" PRIx32 "\n", v);
#endif
			if (!mem->gx_cmd_nb)
			{
				const struct gx_cmd_def *cmd_def;
				cmd_def = &gx_cmd_defs[(v >> 24) & 0xFF];
				if (cmd_def->name)
					start_gx_cmd(mem, cmd_def);
				cmd_def = &gx_cmd_defs[(v >> 16) & 0xFF];
				if (cmd_def->name)
					start_gx_cmd(mem, cmd_def);
				cmd_def = &gx_cmd_defs[(v >> 8) & 0xFF];
				if (cmd_def->name)
					start_gx_cmd(mem, cmd_def);
				cmd_def = &gx_cmd_defs[(v >> 0) & 0xFF];
				if (cmd_def->name)
					start_gx_cmd(mem, cmd_def);
				break;
			}
			add_gx_cmd_param(mem, v);
			break;
		case MEM_ARM9_REG_MTX_MODE:
		case MEM_ARM9_REG_MTX_PUSH:
		case MEM_ARM9_REG_MTX_POP:
		case MEM_ARM9_REG_MTX_STORE:
		case MEM_ARM9_REG_MTX_RESTORE:
		case MEM_ARM9_REG_MTX_IDENTITY:
		case MEM_ARM9_REG_MTX_LOAD_4X4:
		case MEM_ARM9_REG_MTX_LOAD_4X3:
		case MEM_ARM9_REG_MTX_MULT_4X4:
		case MEM_ARM9_REG_MTX_MULT_4X3:
		case MEM_ARM9_REG_MTX_MULT_3X3:
		case MEM_ARM9_REG_MTX_SCALE:
		case MEM_ARM9_REG_MTX_TRANS:
		case MEM_ARM9_REG_COLOR:
		case MEM_ARM9_REG_NORMAL:
		case MEM_ARM9_REG_TEXCOORD:
		case MEM_ARM9_REG_VTX_16:
		case MEM_ARM9_REG_VTX_10:
		case MEM_ARM9_REG_VTX_XY:
		case MEM_ARM9_REG_VTX_XZ:
		case MEM_ARM9_REG_VTX_YZ:
		case MEM_ARM9_REG_VTX_DIFF:
		case MEM_ARM9_REG_POLYGON_ATTR:
		case MEM_ARM9_REG_TEXIMAGE_PARAM:
		case MEM_ARM9_REG_PLTT_BASE:
		case MEM_ARM9_REG_DIF_AMB:
		case MEM_ARM9_REG_SPE_EMI:
		case MEM_ARM9_REG_LIGHT_VECTOR:
		case MEM_ARM9_REG_LIGHT_COLOR:
		case MEM_ARM9_REG_SHININESS:
		case MEM_ARM9_REG_BEGIN_VTXS:
		case MEM_ARM9_REG_END_VTXS:
		case MEM_ARM9_REG_SWAP_BUFFERS:
		case MEM_ARM9_REG_VIEWPORT:
		case MEM_ARM9_REG_BOX_TEST:
		case MEM_ARM9_REG_POS_TEST:
		case MEM_ARM9_REG_VEC_TEST:
		{
			uint8_t cmd_id = (addr - MEM_ARM9_REG_GXFIFO) / 4;
#if 0
			printf("[GX] MMIO [0x%02" PRIx8 "] = 0x%08" PRIx32 "\n",
			       cmd_id, v);
#endif
			const struct gx_cmd_def *def = &gx_cmd_defs[cmd_id];
			if (!def->name)
			{
				printf("[GX] unknown cmd 0x%02" PRIx8 "\n", cmd_id);
				break;
			}
			if (!mem->gx_cmd_nb)
			{
				start_gx_cmd(mem, def);
				if (def->params)
					add_gx_cmd_param(mem, v);
				break;
			}
			struct gx_cmd *cmd = &mem->gx_cmd[mem->gx_cmd_nb - 1];
			if (cmd->def != def)
			{
				commit_gx_cmd(mem);
				start_gx_cmd(mem, def);
				if (def->params)
					add_gx_cmd_param(mem, v);
				break;
			}
			add_gx_cmd_param(mem, v);
			break;
		}
		default:
			set_arm9_reg8(mem, addr + 0, v >> 0);
			set_arm9_reg8(mem, addr + 1, v >> 8);
			set_arm9_reg8(mem, addr + 2, v >> 16);
			set_arm9_reg8(mem, addr + 3, v >> 24);
			break;
	}
}

static uint8_t get_arm9_reg8(struct mem *mem, uint32_t addr)
{
	switch (addr)
	{
		case MEM_ARM9_REG_IPCSYNC:
			return mem->arm7_regs[MEM_ARM7_REG_IPCSYNC + 1] & 0x7;
		case MEM_ARM9_REG_IPCSYNC + 1:
		case MEM_ARM9_REG_IPCSYNC + 2:
		case MEM_ARM9_REG_IPCSYNC + 3:
		case MEM_ARM9_REG_IE:
		case MEM_ARM9_REG_IE + 1:
		case MEM_ARM9_REG_IE + 2:
		case MEM_ARM9_REG_IE + 3:
		case MEM_ARM9_REG_IF:
		case MEM_ARM9_REG_IF + 1:
		case MEM_ARM9_REG_IF + 2:
		case MEM_ARM9_REG_IF + 3:
		case MEM_ARM9_REG_IME:
		case MEM_ARM9_REG_IME + 1:
		case MEM_ARM9_REG_IME + 2:
		case MEM_ARM9_REG_IME + 3:
		case MEM_ARM9_REG_POSTFLG:
		case MEM_ARM9_REG_POSTFLG + 1:
		case MEM_ARM9_REG_POSTFLG + 2:
		case MEM_ARM9_REG_POSTFLG + 3:
		case MEM_ARM9_REG_ROMCTRL:
		case MEM_ARM9_REG_ROMCTRL + 1:
		case MEM_ARM9_REG_ROMCTRL + 3:
		case MEM_ARM9_REG_ROMCMD:
		case MEM_ARM9_REG_ROMCMD + 1:
		case MEM_ARM9_REG_ROMCMD + 2:
		case MEM_ARM9_REG_ROMCMD + 3:
		case MEM_ARM9_REG_ROMCMD + 4:
		case MEM_ARM9_REG_ROMCMD + 5:
		case MEM_ARM9_REG_ROMCMD + 6:
		case MEM_ARM9_REG_ROMCMD + 7:
		case MEM_ARM9_REG_WRAMCNT:
		case MEM_ARM9_REG_EXMEMCNT:
		case MEM_ARM9_REG_EXMEMCNT + 1:
		case MEM_ARM9_REG_KEYCNT:
		case MEM_ARM9_REG_KEYCNT + 1:
		case MEM_ARM9_REG_DISPSTAT:
		case MEM_ARM9_REG_DISPSTAT + 1:
		case MEM_ARM9_REG_VCOUNT:
		case MEM_ARM9_REG_VCOUNT + 1:
		case MEM_ARM9_REG_DMA0SAD:
		case MEM_ARM9_REG_DMA0SAD + 1:
		case MEM_ARM9_REG_DMA0SAD + 2:
		case MEM_ARM9_REG_DMA0SAD + 3:
		case MEM_ARM9_REG_DMA0DAD:
		case MEM_ARM9_REG_DMA0DAD + 1:
		case MEM_ARM9_REG_DMA0DAD + 2:
		case MEM_ARM9_REG_DMA0DAD + 3:
		case MEM_ARM9_REG_DMA0CNT_L:
		case MEM_ARM9_REG_DMA0CNT_L + 1:
		case MEM_ARM9_REG_DMA0CNT_H:
		case MEM_ARM9_REG_DMA0CNT_H + 1:
		case MEM_ARM9_REG_DMA1SAD:
		case MEM_ARM9_REG_DMA1SAD + 1:
		case MEM_ARM9_REG_DMA1SAD + 2:
		case MEM_ARM9_REG_DMA1SAD + 3:
		case MEM_ARM9_REG_DMA1DAD:
		case MEM_ARM9_REG_DMA1DAD + 1:
		case MEM_ARM9_REG_DMA1DAD + 2:
		case MEM_ARM9_REG_DMA1DAD + 3:
		case MEM_ARM9_REG_DMA1CNT_L:
		case MEM_ARM9_REG_DMA1CNT_L + 1:
		case MEM_ARM9_REG_DMA1CNT_H:
		case MEM_ARM9_REG_DMA1CNT_H + 1:
		case MEM_ARM9_REG_DMA2SAD:
		case MEM_ARM9_REG_DMA2SAD + 1:
		case MEM_ARM9_REG_DMA2SAD + 2:
		case MEM_ARM9_REG_DMA2SAD + 3:
		case MEM_ARM9_REG_DMA2DAD:
		case MEM_ARM9_REG_DMA2DAD + 1:
		case MEM_ARM9_REG_DMA2DAD + 2:
		case MEM_ARM9_REG_DMA2DAD + 3:
		case MEM_ARM9_REG_DMA2CNT_L:
		case MEM_ARM9_REG_DMA2CNT_L + 1:
		case MEM_ARM9_REG_DMA2CNT_H:
		case MEM_ARM9_REG_DMA2CNT_H + 1:
		case MEM_ARM9_REG_DMA3SAD:
		case MEM_ARM9_REG_DMA3SAD + 1:
		case MEM_ARM9_REG_DMA3SAD + 2:
		case MEM_ARM9_REG_DMA3SAD + 3:
		case MEM_ARM9_REG_DMA3DAD:
		case MEM_ARM9_REG_DMA3DAD + 1:
		case MEM_ARM9_REG_DMA3DAD + 2:
		case MEM_ARM9_REG_DMA3DAD + 3:
		case MEM_ARM9_REG_DMA3CNT_L:
		case MEM_ARM9_REG_DMA3CNT_L  +1:
		case MEM_ARM9_REG_DMA3CNT_H:
		case MEM_ARM9_REG_DMA3CNT_H + 1:
		case MEM_ARM9_REG_DMA0FILL:
		case MEM_ARM9_REG_DMA0FILL + 1:
		case MEM_ARM9_REG_DMA0FILL + 2:
		case MEM_ARM9_REG_DMA0FILL + 3:
		case MEM_ARM9_REG_DMA1FILL:
		case MEM_ARM9_REG_DMA1FILL + 1:
		case MEM_ARM9_REG_DMA1FILL + 2:
		case MEM_ARM9_REG_DMA1FILL + 3:
		case MEM_ARM9_REG_DMA2FILL:
		case MEM_ARM9_REG_DMA2FILL + 1:
		case MEM_ARM9_REG_DMA2FILL + 2:
		case MEM_ARM9_REG_DMA2FILL + 3:
		case MEM_ARM9_REG_DMA3FILL:
		case MEM_ARM9_REG_DMA3FILL + 1:
		case MEM_ARM9_REG_DMA3FILL + 2:
		case MEM_ARM9_REG_DMA3FILL + 3:
		case MEM_ARM9_REG_POWCNT1:
		case MEM_ARM9_REG_POWCNT1 + 1:
		case MEM_ARM9_REG_POWCNT1 + 2:
		case MEM_ARM9_REG_POWCNT1 + 3:
		case MEM_ARM9_REG_DIVCNT:
		case MEM_ARM9_REG_DIVCNT + 1:
		case MEM_ARM9_REG_DIVCNT + 2:
		case MEM_ARM9_REG_DIVCNT + 3:
		case MEM_ARM9_REG_DIV_NUMER:
		case MEM_ARM9_REG_DIV_NUMER + 1:
		case MEM_ARM9_REG_DIV_NUMER + 2:
		case MEM_ARM9_REG_DIV_NUMER + 3:
		case MEM_ARM9_REG_DIV_NUMER + 4:
		case MEM_ARM9_REG_DIV_NUMER + 5:
		case MEM_ARM9_REG_DIV_NUMER + 6:
		case MEM_ARM9_REG_DIV_NUMER + 7:
		case MEM_ARM9_REG_DIV_DENOM:
		case MEM_ARM9_REG_DIV_DENOM + 1:
		case MEM_ARM9_REG_DIV_DENOM + 2:
		case MEM_ARM9_REG_DIV_DENOM + 3:
		case MEM_ARM9_REG_DIV_DENOM + 4:
		case MEM_ARM9_REG_DIV_DENOM + 5:
		case MEM_ARM9_REG_DIV_DENOM + 6:
		case MEM_ARM9_REG_DIV_DENOM + 7:
		case MEM_ARM9_REG_DISPCNT:
		case MEM_ARM9_REG_DISPCNT + 1:
		case MEM_ARM9_REG_DISPCNT + 2:
		case MEM_ARM9_REG_DISPCNT + 3:
		case MEM_ARM9_REG_DISPCNT + 0x1000:
		case MEM_ARM9_REG_DISPCNT + 0x1000 + 1:
		case MEM_ARM9_REG_DISPCNT + 0x1000 + 2:
		case MEM_ARM9_REG_DISPCNT + 0x1000 + 3:
		case MEM_ARM9_REG_MASTER_BRIGHT:
		case MEM_ARM9_REG_MASTER_BRIGHT + 1:
		case MEM_ARM9_REG_MASTER_BRIGHT + 2:
		case MEM_ARM9_REG_MASTER_BRIGHT + 3:
		case MEM_ARM9_REG_MASTER_BRIGHT + 0x1000:
		case MEM_ARM9_REG_MASTER_BRIGHT + 0x1000 + 1:
		case MEM_ARM9_REG_MASTER_BRIGHT + 0x1000 + 2:
		case MEM_ARM9_REG_MASTER_BRIGHT + 0x1000 + 3:
		case MEM_ARM9_REG_BG0CNT:
		case MEM_ARM9_REG_BG0CNT + 1:
		case MEM_ARM9_REG_BG1CNT:
		case MEM_ARM9_REG_BG1CNT + 1:
		case MEM_ARM9_REG_BG2CNT:
		case MEM_ARM9_REG_BG2CNT + 1:
		case MEM_ARM9_REG_BG3CNT:
		case MEM_ARM9_REG_BG3CNT + 1:
		case MEM_ARM9_REG_WININ:
		case MEM_ARM9_REG_WININ + 1:
		case MEM_ARM9_REG_WINOUT:
		case MEM_ARM9_REG_WINOUT + 1:
		case MEM_ARM9_REG_BLDCNT:
		case MEM_ARM9_REG_BLDCNT + 1:
		case MEM_ARM9_REG_BLDALPHA:
		case MEM_ARM9_REG_BLDALPHA + 1:
		case MEM_ARM9_REG_BG0CNT + 0x1000:
		case MEM_ARM9_REG_BG0CNT + 0x1000 + 1:
		case MEM_ARM9_REG_BG1CNT + 0x1000:
		case MEM_ARM9_REG_BG1CNT + 0x1000 + 1:
		case MEM_ARM9_REG_BG2CNT + 0x1000:
		case MEM_ARM9_REG_BG2CNT + 0x1000 + 1:
		case MEM_ARM9_REG_BG3CNT + 0x1000:
		case MEM_ARM9_REG_BG3CNT + 0x1000 + 1:
		case MEM_ARM9_REG_WININ + 0x1000:
		case MEM_ARM9_REG_WININ + 0x1000 + 1:
		case MEM_ARM9_REG_WINOUT + 0x1000:
		case MEM_ARM9_REG_WINOUT + 0x1000 + 1:
		case MEM_ARM9_REG_BLDCNT + 0x1000:
		case MEM_ARM9_REG_BLDCNT + 0x1000 + 1:
		case MEM_ARM9_REG_BLDALPHA + 0x1000:
		case MEM_ARM9_REG_BLDALPHA + 0x1000 + 1:
		case MEM_ARM9_REG_SQRTCNT:
		case MEM_ARM9_REG_SQRTCNT + 1:
		case MEM_ARM9_REG_SQRT_RESULT:
		case MEM_ARM9_REG_SQRT_RESULT + 1:
		case MEM_ARM9_REG_SQRT_RESULT + 2:
		case MEM_ARM9_REG_SQRT_RESULT + 3:
		case MEM_ARM9_REG_SQRT_PARAM:
		case MEM_ARM9_REG_SQRT_PARAM + 1:
		case MEM_ARM9_REG_SQRT_PARAM + 2:
		case MEM_ARM9_REG_SQRT_PARAM + 3:
		case MEM_ARM9_REG_SQRT_PARAM + 4:
		case MEM_ARM9_REG_SQRT_PARAM + 5:
		case MEM_ARM9_REG_SQRT_PARAM + 6:
		case MEM_ARM9_REG_SQRT_PARAM + 7:
		case MEM_ARM9_REG_TM0CNT_H:
		case MEM_ARM9_REG_TM0CNT_H + 1:
		case MEM_ARM9_REG_TM1CNT_H:
		case MEM_ARM9_REG_TM1CNT_H + 1:
		case MEM_ARM9_REG_TM2CNT_H:
		case MEM_ARM9_REG_TM2CNT_H + 1:
		case MEM_ARM9_REG_TM3CNT_H:
		case MEM_ARM9_REG_TM3CNT_H + 1:
		case MEM_ARM9_REG_DISP3DCNT:
		case MEM_ARM9_REG_DISP3DCNT + 1:
		case MEM_ARM9_REG_DISP3DCNT + 2:
		case MEM_ARM9_REG_DISP3DCNT + 3:
		case MEM_ARM9_REG_DISPCAPCNT:
		case MEM_ARM9_REG_DISPCAPCNT + 1:
		case MEM_ARM9_REG_DISPCAPCNT + 2:
		case MEM_ARM9_REG_DISPCAPCNT + 3:
		case MEM_ARM9_REG_VEC_RESULT:
		case MEM_ARM9_REG_VEC_RESULT + 1:
		case MEM_ARM9_REG_VEC_RESULT + 2:
		case MEM_ARM9_REG_VEC_RESULT + 3:
		case MEM_ARM9_REG_VEC_RESULT + 4:
		case MEM_ARM9_REG_VEC_RESULT + 5:
		case MEM_ARM9_REG_POS_RESULT:
		case MEM_ARM9_REG_POS_RESULT + 1:
		case MEM_ARM9_REG_POS_RESULT + 2:
		case MEM_ARM9_REG_POS_RESULT + 3:
		case MEM_ARM9_REG_POS_RESULT + 4:
		case MEM_ARM9_REG_POS_RESULT + 5:
		case MEM_ARM9_REG_POS_RESULT + 6:
		case MEM_ARM9_REG_POS_RESULT + 7:
		case MEM_ARM9_REG_POS_RESULT + 8:
		case MEM_ARM9_REG_POS_RESULT + 9:
		case MEM_ARM9_REG_POS_RESULT + 10:
		case MEM_ARM9_REG_POS_RESULT + 11:
		case MEM_ARM9_REG_POS_RESULT + 12:
		case MEM_ARM9_REG_POS_RESULT + 13:
		case MEM_ARM9_REG_POS_RESULT + 14:
		case MEM_ARM9_REG_POS_RESULT + 15:
			return mem->arm9_regs[addr];
		case MEM_ARM9_REG_DIV_RESULT:
		case MEM_ARM9_REG_DIV_RESULT + 1:
		case MEM_ARM9_REG_DIV_RESULT + 2:
		case MEM_ARM9_REG_DIV_RESULT + 3:
		case MEM_ARM9_REG_DIV_RESULT + 4:
		case MEM_ARM9_REG_DIV_RESULT + 5:
		case MEM_ARM9_REG_DIV_RESULT + 6:
		case MEM_ARM9_REG_DIV_RESULT + 7:
#if 0
			printf("[ARM9] [%08" PRIx32 "] DIV_RESULT[%08" PRIx32 "] read 0x%02" PRIx8 "\n",
			       cpu_get_reg(mem->nds->arm9, CPU_REG_PC), addr, mem->arm9_regs[addr]);
#endif
			return mem->arm9_regs[addr];
		case MEM_ARM9_REG_DIVREM_RESULT:
		case MEM_ARM9_REG_DIVREM_RESULT + 1:
		case MEM_ARM9_REG_DIVREM_RESULT + 2:
		case MEM_ARM9_REG_DIVREM_RESULT + 3:
		case MEM_ARM9_REG_DIVREM_RESULT + 4:
		case MEM_ARM9_REG_DIVREM_RESULT + 5:
		case MEM_ARM9_REG_DIVREM_RESULT + 6:
		case MEM_ARM9_REG_DIVREM_RESULT + 7:
#if 0
			printf("[ARM9] [%08" PRIx32 "] DIVREM_RESULT[%08" PRIx32 "] read 0x%02" PRIx8 "\n",
			       cpu_get_reg(mem->nds->arm9, CPU_REG_PC), addr, mem->arm9_regs[addr]);
#endif
			return mem->arm9_regs[addr];
		case MEM_ARM9_REG_GXSTAT:
		case MEM_ARM9_REG_GXSTAT + 1:
		case MEM_ARM9_REG_GXSTAT + 2:
#if 0
			printf("[ARM9] [%08" PRIx32 "] GXSTAT[%08" PRIx32 "] read 0x%02" PRIx8 "\n",
			       cpu_get_reg(mem->nds->arm9, CPU_REG_PC), addr, mem->arm9_regs[addr]);
#endif
			return mem->arm9_regs[addr];
		case MEM_ARM9_REG_GXSTAT + 3:
		{
#if 0
			printf("[ARM9] [%08" PRIx32 "] GXSTAT[%08" PRIx32 "] read 0x%02" PRIx8 "\n",
			       cpu_get_reg(mem->nds->arm9, CPU_REG_PC), addr, mem->arm9_regs[addr]);
#endif
			uint8_t v = mem->arm9_regs[addr];
			if (!mem->gxfifo_dma_count)
				v |= (1 << 2);
			return v;
		}
		case MEM_ARM9_REG_AUXSPICNT:
		case MEM_ARM9_REG_AUXSPICNT + 1:
#if 0
			printf("[ARM9] [%08" PRIx32 "] AUXSPICNT[%08" PRIx32 "] read 0x%02" PRIx8 "\n",
			       cpu_get_reg(mem->nds->arm9, CPU_REG_PC), addr, mem->arm9_regs[addr]);
#endif
			return mem->arm9_regs[addr];
		case MEM_ARM9_REG_AUXSPIDATA:
			return auxspi_read(mem);
		case MEM_ARM9_REG_ROMDATA:
		case MEM_ARM9_REG_ROMDATA + 1:
		case MEM_ARM9_REG_ROMDATA + 2:
		case MEM_ARM9_REG_ROMDATA + 3:
			return mbc_read(mem->mbc);
		case MEM_ARM9_REG_TM0CNT_L:
			return mem->arm9_timers[0].v >> 10;
		case MEM_ARM9_REG_TM0CNT_L + 1:
			return mem->arm9_timers[0].v >> 18;
		case MEM_ARM9_REG_TM1CNT_L:
			return mem->arm9_timers[1].v >> 10;
		case MEM_ARM9_REG_TM1CNT_L + 1:
			return mem->arm9_timers[1].v >> 18;
		case MEM_ARM9_REG_TM2CNT_L:
			return mem->arm9_timers[2].v >> 10;
		case MEM_ARM9_REG_TM2CNT_L + 1:
			return mem->arm9_timers[2].v >> 18;
		case MEM_ARM9_REG_TM3CNT_L:
			return mem->arm9_timers[3].v >> 10;
		case MEM_ARM9_REG_TM3CNT_L + 1:
			return mem->arm9_timers[3].v >> 18;
		case MEM_ARM9_REG_KEYINPUT:
		{
			uint8_t v = 0;
			if (!(mem->nds->joypad & NDS_BUTTON_A))
				v |= (1 << 0);
			if (!(mem->nds->joypad & NDS_BUTTON_B))
				v |= (1 << 1);
			if (!(mem->nds->joypad & NDS_BUTTON_SELECT))
				v |= (1 << 2);
			if (!(mem->nds->joypad & NDS_BUTTON_START))
				v |= (1 << 3);
			if (!(mem->nds->joypad & NDS_BUTTON_RIGHT))
				v |= (1 << 4);
			if (!(mem->nds->joypad & NDS_BUTTON_LEFT))
				v |= (1 << 5);
			if (!(mem->nds->joypad & NDS_BUTTON_UP))
				v |= (1 << 6);
			if (!(mem->nds->joypad & NDS_BUTTON_DOWN))
				v |= (1 << 7);
			return v;
		}
		case MEM_ARM9_REG_KEYINPUT + 1:
		{
			uint8_t v = 0;
			if (!(mem->nds->joypad & NDS_BUTTON_R))
				v |= (1 << 0);
			if (!(mem->nds->joypad & NDS_BUTTON_L))
				v |= (1 << 1);
			return v;
		}
		case MEM_ARM9_REG_IPCFIFOCNT:
		{
			uint8_t v = mem->arm9_regs[addr] & (1 << 2);
			if (mem->arm7_fifo.len < 4)
				v |= (1 << 0);
			if (mem->arm7_fifo.len == 64)
				v |= (1 << 1);
#if 0
			printf("ARM9 FIFOCNT[0] read %02" PRIx8 "\n", v);
#endif
			return v;
		}
		case MEM_ARM9_REG_IPCFIFOCNT + 1:
		{
			uint8_t v = mem->arm9_regs[addr] & 0xC4;
			if (mem->arm9_fifo.len < 4)
				v |= (1 << 0);
			if (mem->arm9_fifo.len == 64)
				v |= (1 << 1);
#if 0
			printf("ARM9 FIFOCNT[1] read %02" PRIx8 "\n", v);
#endif
			return v;
		}
		case MEM_ARM9_REG_IPCFIFORECV:
		case MEM_ARM9_REG_IPCFIFORECV + 1:
		case MEM_ARM9_REG_IPCFIFORECV + 2:
		case MEM_ARM9_REG_IPCFIFORECV + 3:
		{
#if 0
			printf("ARM9 IPCFIFO read\n");
#endif
			if (!(mem->arm9_regs[MEM_ARM9_REG_IPCFIFOCNT + 1] & (1 << 7)))
				return mem->arm9_fifo.latch[addr - MEM_ARM9_REG_IPCFIFORECV];
			if (!mem->arm9_fifo.len)
			{
				mem->arm9_regs[MEM_ARM9_REG_IPCFIFOCNT + 1] |= (1 << 6);
				return mem->arm9_fifo.latch[addr - MEM_ARM9_REG_IPCFIFORECV];
			}
			uint8_t v = mem->arm9_fifo.data[mem->arm9_fifo.pos];
			mem->arm9_fifo.pos = (mem->arm9_fifo.pos + 1) % 64;
			mem->arm9_fifo.latch[addr - MEM_ARM9_REG_IPCFIFORECV] = v;
			mem->arm9_fifo.len--;
			if (!mem->arm9_fifo.len
			 && (mem->arm7_regs[MEM_ARM7_REG_IPCFIFOCNT] & (1 << 2)))
				mem_arm7_irq(mem, 1 << 17);
			return v;
		}
		case MEM_ARM9_REG_ROMCTRL + 2:
			if (mem->dscard_dma_count) /* nasty hack: fake non-availibility if dma is running */
				return mem->arm9_regs[addr] & ~(1 << 7);
			return mem->arm9_regs[addr];
		case MEM_ARM9_REG_RAM_COUNT:
			return mem->nds->gpu->g3d.back->polygons_nb;
		case MEM_ARM9_REG_RAM_COUNT + 1:
			return mem->nds->gpu->g3d.back->polygons_nb >> 8;
		case MEM_ARM9_REG_RAM_COUNT + 2:
			return mem->nds->gpu->g3d.back->vertexes_nb;
		case MEM_ARM9_REG_RAM_COUNT + 3:
			return mem->nds->gpu->g3d.back->vertexes_nb >> 8;
		default:
			printf("[ARM9] [%08" PRIx32 "] unknown get register %08" PRIx32 "\n",
			       cpu_get_reg(mem->nds->arm9, CPU_REG_PC), addr);
			break;
	}
	return 0;
}

static uint16_t get_arm9_reg16(struct mem *mem, uint32_t addr)
{
	return (get_arm9_reg8(mem, addr + 0) << 0)
	     | (get_arm9_reg8(mem, addr + 1) << 8);
}

static uint32_t get_arm9_reg32(struct mem *mem, uint32_t addr)
{
	switch (addr)
	{
		case MEM_ARM9_REG_CLIPMTX_RESULT:
			return mem->nds->gpu->g3d.clip_matrix.x.x;
		case MEM_ARM9_REG_CLIPMTX_RESULT + 0x04:
			return mem->nds->gpu->g3d.clip_matrix.x.y;
		case MEM_ARM9_REG_CLIPMTX_RESULT + 0x08:
			return mem->nds->gpu->g3d.clip_matrix.x.z;
		case MEM_ARM9_REG_CLIPMTX_RESULT + 0x0C:
			return mem->nds->gpu->g3d.clip_matrix.x.w;
		case MEM_ARM9_REG_CLIPMTX_RESULT + 0x10:
			return mem->nds->gpu->g3d.clip_matrix.y.x;
		case MEM_ARM9_REG_CLIPMTX_RESULT + 0x14:
			return mem->nds->gpu->g3d.clip_matrix.y.y;
		case MEM_ARM9_REG_CLIPMTX_RESULT + 0x18:
			return mem->nds->gpu->g3d.clip_matrix.y.z;
		case MEM_ARM9_REG_CLIPMTX_RESULT + 0x1C:
			return mem->nds->gpu->g3d.clip_matrix.y.w;
		case MEM_ARM9_REG_CLIPMTX_RESULT + 0x20:
			return mem->nds->gpu->g3d.clip_matrix.z.x;
		case MEM_ARM9_REG_CLIPMTX_RESULT + 0x24:
			return mem->nds->gpu->g3d.clip_matrix.z.y;
		case MEM_ARM9_REG_CLIPMTX_RESULT + 0x28:
			return mem->nds->gpu->g3d.clip_matrix.z.z;
		case MEM_ARM9_REG_CLIPMTX_RESULT + 0x2C:
			return mem->nds->gpu->g3d.clip_matrix.z.w;
		case MEM_ARM9_REG_CLIPMTX_RESULT + 0x30:
			return mem->nds->gpu->g3d.clip_matrix.w.x;
		case MEM_ARM9_REG_CLIPMTX_RESULT + 0x34:
			return mem->nds->gpu->g3d.clip_matrix.w.y;
		case MEM_ARM9_REG_CLIPMTX_RESULT + 0x38:
			return mem->nds->gpu->g3d.clip_matrix.w.z;
		case MEM_ARM9_REG_CLIPMTX_RESULT + 0x3C:
			return mem->nds->gpu->g3d.clip_matrix.w.w;
		case MEM_ARM9_REG_VECMTX_RESULT:
			return mem->nds->gpu->g3d.dir_matrix.x.x;
		case MEM_ARM9_REG_VECMTX_RESULT + 0x04:
			return mem->nds->gpu->g3d.dir_matrix.x.y;
		case MEM_ARM9_REG_VECMTX_RESULT + 0x08:
			return mem->nds->gpu->g3d.dir_matrix.x.z;
		case MEM_ARM9_REG_VECMTX_RESULT + 0x0C:
			return mem->nds->gpu->g3d.dir_matrix.y.x;
		case MEM_ARM9_REG_VECMTX_RESULT + 0x10:
			return mem->nds->gpu->g3d.dir_matrix.y.y;
		case MEM_ARM9_REG_VECMTX_RESULT + 0x14:
			return mem->nds->gpu->g3d.dir_matrix.y.z;
		case MEM_ARM9_REG_VECMTX_RESULT + 0x18:
			return mem->nds->gpu->g3d.dir_matrix.z.x;
		case MEM_ARM9_REG_VECMTX_RESULT + 0x1C:
			return mem->nds->gpu->g3d.dir_matrix.z.y;
		case MEM_ARM9_REG_VECMTX_RESULT + 0x20:
			return mem->nds->gpu->g3d.dir_matrix.z.z;
		default:
			return (get_arm9_reg8(mem, addr + 0) << 0)
			     | (get_arm9_reg8(mem, addr + 1) << 8)
			     | (get_arm9_reg8(mem, addr + 2) << 16)
			     | (get_arm9_reg8(mem, addr + 3) << 24);
	}
}

static void arm9_instr_delay(struct mem *mem, const uint8_t *table, enum mem_type type)
{
	mem->nds->arm9->instr_delay += table[type];
}

static void *get_vram_bga_ptr(struct mem *mem, uint32_t addr)
{
	uint32_t base = mem->vram_bga_bases[(addr / 0x4000) & 0x1F];
	if (base == 0xFFFFFFFF)
		return NULL;
	return &mem->vram[base + (addr & 0x3FFF)];
}

static void *get_vram_bgb_ptr(struct mem *mem, uint32_t addr)
{
	uint32_t base = mem->vram_bgb_bases[(addr / 0x4000) & 0x7];
	if (base == 0xFFFFFFFF)
		return NULL;
	return &mem->vram[base + (addr & 0x3FFF)];
}

static void *get_vram_obja_ptr(struct mem *mem, uint32_t addr)
{
	uint32_t base = mem->vram_obja_bases[(addr / 0x4000) & 0xF];
	if (base == 0xFFFFFFFF)
		return NULL;
	return &mem->vram[base + (addr & 0x3FFF)];
}

static void *get_vram_objb_ptr(struct mem *mem, uint32_t addr)
{
	uint32_t base = mem->vram_objb_bases[(addr / 0x4000) & 0x7];
	if (base == 0xFFFFFFFF)
		return NULL;
	return &mem->vram[base + (addr & 0x3FFF)];
}

static void *get_vram_bgepa_ptr(struct mem *mem, uint32_t addr)
{
	uint32_t base = mem->vram_bgepa_bases[(addr / 0x4000) & 0x1];
	if (base == 0xFFFFFFFF)
		return NULL;
	return &mem->vram[base + (addr & 0x3FFF)];
}

static void *get_vram_bgepb_ptr(struct mem *mem, uint32_t addr)
{
	uint32_t base = mem->vram_bgepb_base;
	if (base == 0xFFFFFFFF)
		return NULL;
	return &mem->vram[base + (addr & 0x7FFF)];
}

static void *get_vram_objepa_ptr(struct mem *mem, uint32_t addr)
{
	uint32_t base = mem->vram_objepa_base;
	if (base == 0xFFFFFFFF)
		return NULL;
	return &mem->vram[base + (addr & 0x1FFF)];
}

static void *get_vram_objepb_ptr(struct mem *mem, uint32_t addr)
{
	uint32_t base = mem->vram_objepb_base;
	if (base == 0xFFFFFFFF)
		return NULL;
	return &mem->vram[base + (addr & 0x1FFF)];
}

static void *get_vram_trpi_ptr(struct mem *mem, uint32_t addr)
{
	uint32_t base = mem->vram_trpi_bases[(addr / 0x20000) & 0x3];
	if (base == 0xFFFFFFFF)
		return NULL;
	return &mem->vram[base + (addr & 0x1FFFF)];
}

static void *get_vram_texp_ptr(struct mem *mem, uint32_t addr)
{
	uint32_t base = mem->vram_texp_bases[(addr / 0x4000) & 0x7];
	if (base == 0xFFFFFFFF)
		return NULL;
	return &mem->vram[base + (addr & 0x3FFF)];
}

void *get_arm9_vram_ptr(struct mem *mem, uint32_t addr)
{
	switch ((addr >> 20) & 0xF)
	{
		case 0x0:
			return get_vram_bga_ptr(mem, addr);
		case 0x2:
			return get_vram_bgb_ptr(mem, addr);
		case 0x4:
			return get_vram_obja_ptr(mem, addr);
		case 0x6:
			return get_vram_objb_ptr(mem, addr);
		case 0x8:
			switch ((addr >> 16) & 0xF)
			{
				case 0x0:
				case 0x1:
					if ((mem_arm9_get_reg8(mem, MEM_ARM9_REG_VRAMCNT_A) & 0x83) != 0x80)
						return NULL;
					return &mem->vram[MEM_VRAM_A_BASE + (addr & MEM_VRAM_A_MASK)];
				case 0x2:
				case 0x3:
					if ((mem_arm9_get_reg8(mem, MEM_ARM9_REG_VRAMCNT_B) & 0x83) != 0x80)
						return NULL;
					return &mem->vram[MEM_VRAM_B_BASE + (addr & MEM_VRAM_B_MASK)];
				case 0x4:
				case 0x5:
					if ((mem_arm9_get_reg8(mem, MEM_ARM9_REG_VRAMCNT_C) & 0x87) != 0x80)
						return NULL;
					return &mem->vram[MEM_VRAM_C_BASE + (addr & MEM_VRAM_C_MASK)];
				case 0x6:
				case 0x7:
					if ((mem_arm9_get_reg8(mem, MEM_ARM9_REG_VRAMCNT_D) & 0x87) != 0x80)
						return NULL;
					return &mem->vram[MEM_VRAM_D_BASE + (addr & MEM_VRAM_D_MASK)];
				case 0x8:
					if ((mem_arm9_get_reg8(mem, MEM_ARM9_REG_VRAMCNT_E) & 0x87) != 0x80)
						return NULL;
					return &mem->vram[MEM_VRAM_E_BASE + (addr & MEM_VRAM_E_MASK)];
				case 0x9:
					switch ((addr >> 14) & 0x3)
					{
						case 0x0:
							if ((mem_arm9_get_reg8(mem, MEM_ARM9_REG_VRAMCNT_F) & 0x87) != 0x80)
								return NULL;
							return &mem->vram[MEM_VRAM_F_BASE + (addr & MEM_VRAM_F_MASK)];
						case 0x1:
							if ((mem_arm9_get_reg8(mem, MEM_ARM9_REG_VRAMCNT_G) & 0x87) != 0x80)
								return NULL;
							return &mem->vram[MEM_VRAM_G_BASE + (addr & MEM_VRAM_G_MASK)];
						case 0x2:
						case 0x3:
							if ((mem_arm9_get_reg8(mem, MEM_ARM9_REG_VRAMCNT_H) & 0x83) != 0x80)
							{
								printf("failed to get vram h %" PRIx32 "\n", addr);
								return NULL;
							}
							return &mem->vram[MEM_VRAM_H_BASE + (addr & MEM_VRAM_H_MASK)];
					}
					break;
				case 0xA:
					if ((mem_arm9_get_reg8(mem, MEM_ARM9_REG_VRAMCNT_I) & 0x83) != 0x80)
						return NULL;
					return &mem->vram[MEM_VRAM_I_BASE + (addr & MEM_VRAM_I_MASK)];
			}
			break;
	}
	return NULL;
}

#define MEM_ARM9_GET(size) \
uint##size##_t mem_arm9_get##size(struct mem *mem, uint32_t addr, enum mem_type type) \
{ \
	if (addr < 0x10) \
		printf("[%08" PRIx32 "] ARM9 get" #size " addr: %08" PRIx32 "\n", cpu_get_reg(mem->nds->arm9, CPU_REG_PC), addr); \
	if (size == 16) \
		addr &= ~1; \
	if (size == 32) \
		addr &= ~3; \
	if (addr != MEM_DIRECT) \
	{ \
		if ((addr & ~mem->itcm_mask) == mem->itcm_base) \
		{ \
			arm9_instr_delay(mem, arm9_tcm_cycles_##size, type); \
			return *(uint##size##_t*)&mem->itcm[addr & 0x7FFF]; \
		} \
		if ((addr & ~mem->dtcm_mask) == mem->dtcm_base) \
		{ \
			arm9_instr_delay(mem, arm9_tcm_cycles_##size, type); \
			return *(uint##size##_t*)&mem->dtcm[addr & 0x3FFF]; \
		} \
	} \
	if (addr >= 0xFFFF0000) \
	{ \
		uint32_t a = addr - 0xFFFF0000; \
		a &= 0xFFF; \
		arm9_instr_delay(mem, arm9_wram_cycles_##size, type); \
		return *(uint##size##_t*)&mem->arm9_bios[a]; \
	} \
	switch ((addr >> 24) & 0xFF) \
	{ \
		case 0x2: /* main memory */ \
			arm9_instr_delay(mem, arm9_mram_cycles_##size, type); \
			return *(uint##size##_t*)&mem->mram[addr & 0x3FFFFF]; \
		case 0x3: /* shared wram */ \
			arm9_instr_delay(mem, arm9_wram_cycles_##size, type); \
			if (!mem->arm9_wram_mask) \
				return 0; \
			return *(uint##size##_t*)&mem->wram[mem->arm9_wram_base \
			                                  + (addr & mem->arm9_wram_mask)]; \
		case 0x4: /* io ports */ \
			arm9_instr_delay(mem, arm9_wram_cycles_##size, type); \
			return get_arm9_reg##size(mem, addr - 0x4000000); \
		case 0x5: /* palette */ \
			arm9_instr_delay(mem, arm9_vram_cycles_##size, type); \
			return *(uint##size##_t*)&mem->palette[addr & 0x7FF]; \
		case 0x6: /* vram */ \
		{ \
			void *ptr = get_arm9_vram_ptr(mem, addr & 0xFFFFFF); \
			if (!ptr) \
				break; \
			arm9_instr_delay(mem, arm9_vram_cycles_##size, type); \
			return *(uint##size##_t*)ptr; \
		} \
		case 0x7: /* oam */ \
			arm9_instr_delay(mem, arm9_wram_cycles_##size, type); \
			return *(uint##size##_t*)&mem->oam[addr & 0x7FF]; \
		case 0x8: /* GBA rom */ \
		case 0x9: \
			if (mem->arm9_regs[MEM_ARM9_REG_EXMEMCNT] & (1 << 7)) \
				return 0; \
			if (size == 8 || size == 16) \
				return addr >> 1; \
			return (addr >> 1) | (((addr + 1) >> 1) << 16); \
		case 0xA: /* GBA ram */ \
			if (mem->arm9_regs[MEM_ARM9_REG_EXMEMCNT] & (1 << 7)) \
				return 0; \
			return 0xFF; \
	} \
	printf("[ARM9] [%08" PRIx32 "] unknown get" #size " [%08" PRIx32 "]\n", \
	       cpu_get_reg(mem->nds->arm9, CPU_REG_PC), addr); \
	return 0; \
} \
uint##size##_t mem_vram_bga_get##size(struct mem *mem, uint32_t addr) \
{ \
	void *ptr = get_vram_bga_ptr(mem, addr); \
	if (!ptr) \
		return 0; \
	return *(uint##size##_t*)ptr; \
} \
uint##size##_t mem_vram_bgb_get##size(struct mem *mem, uint32_t addr) \
{ \
	void *ptr = get_vram_bgb_ptr(mem, addr); \
	if (!ptr) \
		return 0; \
	return *(uint##size##_t*)ptr; \
} \
uint##size##_t mem_vram_obja_get##size(struct mem *mem, uint32_t addr) \
{ \
	void *ptr = get_vram_obja_ptr(mem, addr); \
	if (!ptr) \
		return 0; \
	return *(uint##size##_t*)ptr; \
} \
uint##size##_t mem_vram_objb_get##size(struct mem *mem, uint32_t addr) \
{ \
	void *ptr = get_vram_objb_ptr(mem, addr); \
	if (!ptr) \
		return 0; \
	return *(uint##size##_t*)ptr; \
} \
uint##size##_t mem_vram_bgepa_get##size(struct mem *mem, uint32_t addr) \
{ \
	void *ptr = get_vram_bgepa_ptr(mem, addr); \
	if (!ptr) \
		return 0; \
	return *(uint##size##_t*)ptr; \
} \
uint##size##_t mem_vram_bgepb_get##size(struct mem *mem, uint32_t addr) \
{ \
	void *ptr = get_vram_bgepb_ptr(mem, addr); \
	if (!ptr) \
		return 0; \
	return *(uint##size##_t*)ptr; \
} \
uint##size##_t mem_vram_objepa_get##size(struct mem *mem, uint32_t addr) \
{ \
	void *ptr = get_vram_objepa_ptr(mem, addr); \
	if (!ptr) \
		return 0; \
	return *(uint##size##_t*)ptr; \
} \
uint##size##_t mem_vram_objepb_get##size(struct mem *mem, uint32_t addr) \
{ \
	void *ptr = get_vram_objepb_ptr(mem, addr); \
	if (!ptr) \
		return 0; \
	return *(uint##size##_t*)ptr; \
} \
uint##size##_t mem_vram_trpi_get##size(struct mem *mem, uint32_t addr) \
{ \
	void *ptr = get_vram_trpi_ptr(mem, addr); \
	if (!ptr) \
		return 0; \
	return *(uint##size##_t*)ptr; \
} \
uint##size##_t mem_vram_texp_get##size(struct mem *mem, uint32_t addr) \
{ \
	void *ptr = get_vram_texp_ptr(mem, addr); \
	if (!ptr) \
		return 0; \
	return *(uint##size##_t*)ptr; \
}

MEM_ARM9_GET(8);
MEM_ARM9_GET(16);
MEM_ARM9_GET(32);

#define MEM_ARM9_SET(size) \
void mem_arm9_set##size(struct mem *mem, uint32_t addr, uint##size##_t v, enum mem_type type) \
{ \
	if (addr < 0x10) \
		printf("[%08" PRIx32 "] ARM9 set" #size " addr: %08" PRIx32 "\n", cpu_get_reg(mem->nds->arm9, CPU_REG_PC), addr); \
	if (size == 16) \
		addr &= ~1; \
	if (size == 32) \
		addr &= ~3; \
	if (addr != MEM_DIRECT) \
	{ \
		if ((addr & ~mem->itcm_mask) == mem->itcm_base) \
		{ \
			*(uint##size##_t*)&mem->itcm[addr & 0x7FFF] = v; \
			arm9_instr_delay(mem, arm9_tcm_cycles_##size, type); \
			return; \
		} \
		if ((addr & ~mem->dtcm_mask) == mem->dtcm_base) \
		{ \
			*(uint##size##_t*)&mem->dtcm[addr & 0x3FFF] = v; \
			arm9_instr_delay(mem, arm9_tcm_cycles_##size, type); \
			return; \
		} \
	} \
	switch ((addr >> 24) & 0xFF) \
	{ \
		case 0x2: /* main memory */ \
			*(uint##size##_t*)&mem->mram[addr & 0x3FFFFF] = v; \
			arm9_instr_delay(mem, arm9_mram_cycles_##size, type); \
			return; \
		case 0x3: /* shared wram */ \
			if (!mem->arm9_wram_mask) \
				return; \
			*(uint##size##_t*)&mem->wram[mem->arm9_wram_base \
			                           + (addr & mem->arm9_wram_mask)] = v; \
			arm9_instr_delay(mem, arm9_wram_cycles_##size, type); \
			return; \
		case 0x4: /* io ports */ \
			set_arm9_reg##size(mem, addr - 0x4000000, v); \
			arm9_instr_delay(mem, arm9_wram_cycles_##size, type); \
			return; \
		case 0x5: /* palette */ \
			/* printf("palette write [%08" PRIx32 "] = %x\n", addr, v); */ \
			*(uint##size##_t*)&mem->palette[addr & 0x7FF] = v; \
			arm9_instr_delay(mem, arm9_vram_cycles_##size, type); \
			return; \
		case 0x6: /* vram */ \
		{ \
			/* printf("[ARM9] vram write [%08" PRIx32 "] = %x\n", addr, v); */ \
			void *ptr = get_arm9_vram_ptr(mem, addr & 0xFFFFFF); \
			if (!ptr) \
				break; \
			arm9_instr_delay(mem, arm9_vram_cycles_##size, type); \
			*(uint##size##_t*)ptr = v; \
			return; \
		} \
		case 0x7: /* oam */ \
			/* printf("oam write [%08" PRIx32 "] = %x\n", addr, v); */ \
			*(uint##size##_t*)&mem->oam[addr & 0x7FF] = v; \
			arm9_instr_delay(mem, arm9_wram_cycles_##size, type); \
			return; \
		case 0x8: /* GBA */ \
		case 0x9: \
		case 0xA: \
			return; \
	} \
	printf("[ARM9] [%08" PRIx32 "] unknown set" #size " [%08" PRIx32 "] = %x\n", \
	       cpu_get_reg(mem->nds->arm9, CPU_REG_PC), addr, v); \
}

MEM_ARM9_SET(8);
MEM_ARM9_SET(16);
MEM_ARM9_SET(32);
