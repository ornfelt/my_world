#include "mem.h"
#include "mbc.h"
#include "gba.h"
#include "cpu.h"
#include "apu.h"
#include "gpu.h"

#include <inttypes.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

static uint32_t g_dma_len_max[4] = {0x4000, 0x4000, 0x4000, 0x10000};

struct mem *mem_new(struct gba *gba, struct mbc *mbc)
{
	struct mem *mem = calloc(sizeof(*mem), 1);
	if (!mem)
		return NULL;

	mem->gba = gba;
	mem->mbc = mbc;
	mem_set_reg32(mem, MEM_REG_SOUNDBIAS, 0x200);
	return mem;
}

void mem_del(struct mem *mem)
{
	if (!mem)
		return;
	free(mem);
}

void mem_timers(struct mem *mem)
{
	static const uint16_t masks[4] = {0, 0x3F, 0xFF, 0x3FF};
	bool prev_overflowed = false;
	for (size_t i = 0; i < 4; ++i)
	{
		uint8_t cnt_h = mem_get_reg8(mem, MEM_REG_TM0CNT_H + i * 4);
		if (!(cnt_h & (1 << 7)))
			goto next_timer;
		if (i && (cnt_h & (1 << 2)))
		{
			if (!prev_overflowed)
				goto next_timer;
		}
		else
		{
			if ((mem->gba->cycle & masks[cnt_h & 3]))
				goto next_timer;
		}
		mem->timers[i].v++;
		if (!mem->timers[i].v)
		{
			mem->timers[i].v = mem_get_reg16(mem, MEM_REG_TM0CNT_L + i * 4);
			if (cnt_h & (1 << 6))
				mem_set_reg16(mem, MEM_REG_IF, mem_get_reg16(mem, MEM_REG_IF) | (1 << (3 + i)));
			uint16_t sndcnt_h = mem_get_reg16(mem, MEM_REG_SOUNDCNT_H);
			if (i == ((sndcnt_h >> 10) & 1))
			{
				uint8_t fifo_nb = mem->fifo_nb[0];
				mem->gba->apu->fifo1_val = mem->fifo[0][fifo_nb ? fifo_nb - 1 : 0];
				if (fifo_nb <= 0xF)
					mem_fifo(mem, 0);
				if (fifo_nb)
					mem->fifo_nb[0]--;
			}
			if (i == ((sndcnt_h >> 14) & 1))
			{
				uint8_t fifo_nb = mem->fifo_nb[1];
				mem->gba->apu->fifo2_val = mem->fifo[1][fifo_nb ? fifo_nb - 1 : 0];
				if (fifo_nb <= 0xF)
					mem_fifo(mem, 1);
				if (fifo_nb)
					mem->fifo_nb[1]--;
			}
			prev_overflowed = true;
			continue;
		}
next_timer:
		prev_overflowed = false;
	}
}

static void load_dma_length(struct mem *mem, size_t dma)
{
	mem->dma[dma].len = mem_get_reg16(mem, MEM_REG_DMA0CNT_L + 0xC * dma);
	if (mem->dma[dma].len)
	{
		if (mem->dma[dma].len > g_dma_len_max[dma])
			mem->dma[dma].len = g_dma_len_max[dma];
	}
	else
	{
		mem->dma[dma].len = g_dma_len_max[dma];
	}
}

void mem_hblank(struct mem *mem)
{
	for (size_t i = 0; i < 4; ++i)
	{
		if (!mem->dma[i].enabled || mem->dma[i].active)
			continue;
		uint16_t cnt_h = mem_get_reg16(mem, MEM_REG_DMA0CNT_H + 0xC * i);
		if (((cnt_h >> 12) & 0x3) != 2)
			continue;
		if (((cnt_h >> 5) & 0x3) == 0x3)
			mem->dma[i].dst = mem_get_reg32(mem, MEM_REG_DMA0DAD + 0xC * i);
		load_dma_length(mem, i);
		mem->dma[i].cnt = 0;
		mem->dma[i].active = true;
#if 0
		printf("start HDMA %d of %08x words from %08x to %08x\n", i, mem->dma[i].len, mem->dma[i].src, mem->dma[i].dst);
#endif
	}
}

void mem_vblank(struct mem *mem)
{
	for (size_t i = 0; i < 4; ++i)
	{
		if (!mem->dma[i].enabled || mem->dma[i].active)
			continue;
		uint16_t cnt_h = mem_get_reg16(mem, MEM_REG_DMA0CNT_H + 0xC * i);
		if (((cnt_h >> 12) & 0x3) != 1)
			continue;
		if (((cnt_h >> 5) & 0x3) == 0x3)
			mem->dma[i].dst = mem_get_reg32(mem, MEM_REG_DMA0DAD + 0xC * i);
		load_dma_length(mem, i);
		mem->dma[i].cnt = 0;
		mem->dma[i].active = true;
#if 0
		printf("start VDMA %d of %08x words from %08x to %08x\n", i, mem->dma[i].len, mem->dma[i].src, mem->dma[i].dst);
#endif
	}
}

void mem_fifo(struct mem *mem, uint8_t fifo)
{
	for (size_t i = 1; i < 3; ++i)
	{
		if (!mem->dma[i].enabled)
			continue;
		uint16_t cnt_h = mem_get_reg16(mem, MEM_REG_DMA0CNT_H + 0xC * i);
		if (((cnt_h >> 12) & 0x3) != 3)
			continue;
		if (mem->dma[i].dst != 0x40000a0u + 0x4u * fifo)
			continue;
		mem->dma[i].active = true;
	}
}

bool mem_dma(struct mem *mem)
{
	for (size_t i = 0; i < 4; ++i)
	{
		if (!mem->dma[i].enabled || !mem->dma[i].active)
			continue;
		uint16_t cnt_h = mem_get_reg16(mem, MEM_REG_DMA0CNT_H + 0xC * i);
		if ((i == 1 || i == 2) && (((cnt_h >> 12) & 0x3) == 3))
		{
			uint8_t fifo_id = (mem->dma[i].dst - 0x40000a0u) / 4;
			assert(mem->fifo_nb[i] <= 16);
			memmove(&mem->fifo[fifo_id][16], &mem->fifo[fifo_id][0], mem->fifo_nb[fifo_id]);
			for (size_t j = 0; j < 16; ++j)
				mem->fifo[fifo_id][15 - j] = mem_get8(mem, mem->dma[i].src + j);
			mem->dma[i].src += 16;
			mem->fifo_nb[fifo_id] += 16;
			mem->dma[i].active = false;
			return true;
		}
		uint32_t step;
		if (cnt_h & (1 << 10))
		{
			mem_set32(mem, mem->dma[i].dst, mem_get32(mem, mem->dma[i].src));
			step = 4;
		}
		else
		{
#if 0
			printf("DMA 16 from 0x%" PRIx32 " to 0x%" PRIx32 "\n", mem->dma[i].src, mem->dma[i].dst);
#endif
			mem_set16(mem, mem->dma[i].dst, mem_get16(mem, mem->dma[i].src));
			step = 2;
		}
		switch ((cnt_h >> 5) & 3)
		{
			case 0:
				mem->dma[i].dst += step;
				break;
			case 1:
				mem->dma[i].dst -= step;
				break;
			case 2:
				break;
			case 3:
				mem->dma[i].dst += step;
				break;
		}
		switch ((cnt_h >> 7) & 3)
		{
			case 0:
				mem->dma[i].src += step;
				break;
			case 1:
				mem->dma[i].src -= step;
				break;
			case 2:
				break;
			case 3:
				break;
		}
		mem->dma[i].cnt++;
		if (mem->dma[i].cnt == mem->dma[i].len)
		{
			mem->dma[i].active = false;
			if (!(cnt_h & (1 << 9)))
				mem->dma[i].enabled = false;
			mem_set_reg16(mem, MEM_REG_DMA0CNT_H + 0xC * i, mem_get_reg16(mem, MEM_REG_DMA0CNT_H + 0xC * i) & ~(1 << 15));
			if (cnt_h & (1 << 14))
				mem_set_reg16(mem, MEM_REG_IF, mem_get_reg16(mem, MEM_REG_IF) | (1 << (8 + i)));
		}
		return true;
	}
	return false;
}

static void dma_control(struct mem *mem, uint8_t dma)
{
	mem->dma[dma].src = mem_get_reg32(mem, MEM_REG_DMA0SAD + 0xC * dma);
	mem->dma[dma].dst = mem_get_reg32(mem, MEM_REG_DMA0DAD + 0xC * dma);
	mem->dma[dma].cnt = 0;
	load_dma_length(mem, dma);
	uint16_t cnt_h = mem_get_reg16(mem, MEM_REG_DMA0CNT_H + 0xC * dma);
	mem->dma[dma].enabled = (cnt_h >> 15) & 0x1;
	mem->dma[dma].active = (((cnt_h >> 12) & 0x3) == 0);
#if 0
	if (mem->dma[dma].active)
		printf("start DMA %" PRIu8 " of %08" PRIx32 " words from %08" PRIx32 " to %08" PRIx32 ": %04" PRIx16 "\n", dma, mem->dma[dma].len, mem->dma[dma].src, mem->dma[dma].dst, cnt_h);
#endif
}

static void timer_control(struct mem *mem, uint8_t timer, uint8_t v)
{
	uint8_t prev = mem_get_reg8(mem, MEM_REG_TM0CNT_H + timer * 4);
	mem_set_reg8(mem, MEM_REG_TM0CNT_H + timer * 4, v);
	if ((v & (1 << 7)) && !(prev & (1 << 7)))
		mem->timers[timer].v = mem_get_reg16(mem, MEM_REG_TM0CNT_L + timer * 4);
}

static void set_reg(struct mem *mem, uint32_t reg, uint8_t v)
{
	switch (reg)
	{
		case MEM_REG_HALTCNT:
			if (v & 0x80)
				mem->gba->cpu->state = CPU_STATE_STOP;
			else
				mem->gba->cpu->state = CPU_STATE_HALT;
			return;
		case MEM_REG_IF:
		case MEM_REG_IF + 1:
			mem->io_regs[reg] &= ~v;
			return;
		case MEM_REG_DMA0CNT_H + 1:
			mem->io_regs[reg] = v;
			dma_control(mem, 0);
			return;
		case MEM_REG_DMA1CNT_H + 1:
			mem->io_regs[reg] = v;
			dma_control(mem, 1);
			return;
		case MEM_REG_DMA2CNT_H + 1:
			mem->io_regs[reg] = v;
			dma_control(mem, 2);
			return;
		case MEM_REG_DMA3CNT_H + 1:
			mem->io_regs[reg] = v;
			dma_control(mem, 3);
			return;
		case MEM_REG_TM0CNT_H:
			timer_control(mem, 0, v);
			return;
		case MEM_REG_TM1CNT_H:
			timer_control(mem, 1, v);
			return;
		case MEM_REG_TM2CNT_H:
			timer_control(mem, 2, v);
			return;
		case MEM_REG_TM3CNT_H:
			timer_control(mem, 3, v);
			return;
		case MEM_REG_KEYINPUT:
		case MEM_REG_KEYINPUT + 1:
			return;
		case MEM_REG_KEYCNT:
		case MEM_REG_KEYCNT + 1:
			mem->io_regs[reg] = v;
			gba_test_keypad_int(mem->gba);
			return;
		case MEM_REG_SOUND1CNT_X + 1:
			mem->io_regs[reg] = v;
			if (v & (1 << 7))
				apu_start_channel1(mem->gba->apu);
			return;
		case MEM_REG_SOUND2CNT_H + 1:
			mem->io_regs[reg] = v;
			if (v & (1 << 7))
				apu_start_channel2(mem->gba->apu);
			return;
		case MEM_REG_SOUND3CNT_X + 1:
			mem->io_regs[reg] = v;
			if (v & (1 << 7))
				apu_start_channel3(mem->gba->apu);
			return;
		case MEM_REG_SOUND4CNT_H + 1:
			mem->io_regs[reg] = v;
			if (v & (1 << 7))
				apu_start_channel3(mem->gba->apu);
			return;
		case MEM_REG_WAVE_RAM0_L:
		case MEM_REG_WAVE_RAM0_L + 1:
		case MEM_REG_WAVE_RAM0_H:
		case MEM_REG_WAVE_RAM0_H + 1:
		case MEM_REG_WAVE_RAM1_L:
		case MEM_REG_WAVE_RAM1_L + 1:
		case MEM_REG_WAVE_RAM1_H:
		case MEM_REG_WAVE_RAM1_H + 1:
		case MEM_REG_WAVE_RAM2_L:
		case MEM_REG_WAVE_RAM2_L + 1:
		case MEM_REG_WAVE_RAM2_H:
		case MEM_REG_WAVE_RAM2_H + 1:
		case MEM_REG_WAVE_RAM3_L:
		case MEM_REG_WAVE_RAM3_L + 1:
		case MEM_REG_WAVE_RAM3_H:
		case MEM_REG_WAVE_RAM3_H + 1:
			if (mem_get_reg16(mem, MEM_REG_SOUND3CNT_L) & (1 << 6))
				mem->wave[reg - MEM_REG_WAVE_RAM0_L] = v;
			else
				mem->wave[reg - MEM_REG_WAVE_RAM0_L + 0x10] = v;
			return;
		case MEM_REG_FIFO_A:
		case MEM_REG_FIFO_A + 1:
		case MEM_REG_FIFO_A + 2:
		case MEM_REG_FIFO_A + 3:
			return;
		case MEM_REG_BG2X:
		case MEM_REG_BG2X + 1:
		case MEM_REG_BG2X + 2:
		case MEM_REG_BG2X + 3:
			mem->io_regs[reg] = v;
			mem->gba->gpu->bg2x = mem_get_reg32(mem, MEM_REG_BG2X) & 0xFFFFFFF;
			TRANSFORM_INT28(mem->gba->gpu->bg2x);
			return;
		case MEM_REG_BG2Y:
		case MEM_REG_BG2Y + 1:
		case MEM_REG_BG2Y + 2:
		case MEM_REG_BG2Y + 3:
			mem->io_regs[reg] = v;
			mem->gba->gpu->bg2y = mem_get_reg32(mem, MEM_REG_BG2Y) & 0xFFFFFFF;
			TRANSFORM_INT28(mem->gba->gpu->bg2y);
			return;
		case MEM_REG_BG3X:
		case MEM_REG_BG3X + 1:
		case MEM_REG_BG3X + 2:
		case MEM_REG_BG3X + 3:
			mem->io_regs[reg] = v;
			mem->gba->gpu->bg3x = mem_get_reg32(mem, MEM_REG_BG3X) & 0xFFFFFFF;
			TRANSFORM_INT28(mem->gba->gpu->bg3x);
			return;
		case MEM_REG_BG3Y:
		case MEM_REG_BG3Y + 1:
		case MEM_REG_BG3Y + 2:
		case MEM_REG_BG3Y + 3:
			mem->io_regs[reg] = v;
			mem->gba->gpu->bg3y = mem_get_reg32(mem, MEM_REG_BG3Y) & 0xFFFFFFF;
			TRANSFORM_INT28(mem->gba->gpu->bg3y);
			return;
		case MEM_REG_DISPSTAT:
			mem->io_regs[reg] = (mem->io_regs[reg] & 0x47) | (v & ~0x47);
			return;
		case MEM_REG_DISPSTAT + 1:
			mem->io_regs[reg] = v;
			return;
		case MEM_REG_VCOUNT:
		case MEM_REG_VCOUNT + 1:
			break;
		case MEM_REG_SOUNDCNT_X:
			mem->io_regs[reg] = (mem->io_regs[reg] & 0xF) | (v & ~0xF);
			return;
		case MEM_REG_SOUNDCNT_H + 1:
			mem->io_regs[reg] = v & 0x77;
			if (v & 0x8)
				mem->fifo_nb[0] = 0;
			if (v & 0x80)
				mem->fifo_nb[1] = 0;
			break;
		case MEM_REG_WIN0H:
		case MEM_REG_WIN0H + 1:
		case MEM_REG_WIN0V:
		case MEM_REG_WIN0V + 1:
		case MEM_REG_WIN1H:
		case MEM_REG_WIN1H + 1:
		case MEM_REG_WIN1V:
		case MEM_REG_WIN1V + 1:
		case MEM_REG_DMA0SAD:
		case MEM_REG_DMA0SAD + 1:
		case MEM_REG_DMA0SAD + 2:
		case MEM_REG_DMA0SAD + 3:
		case MEM_REG_DMA0DAD:
		case MEM_REG_DMA0DAD + 1:
		case MEM_REG_DMA0DAD + 2:
		case MEM_REG_DMA0DAD + 3:
		case MEM_REG_DMA0CNT_L:
		case MEM_REG_DMA0CNT_L + 1:
		case MEM_REG_DMA1SAD:
		case MEM_REG_DMA1SAD + 1:
		case MEM_REG_DMA1SAD + 2:
		case MEM_REG_DMA1SAD + 3:
		case MEM_REG_DMA1DAD:
		case MEM_REG_DMA1DAD + 1:
		case MEM_REG_DMA1DAD + 2:
		case MEM_REG_DMA1DAD + 3:
		case MEM_REG_DMA1CNT_L:
		case MEM_REG_DMA1CNT_L + 1:
		case MEM_REG_DMA2SAD:
		case MEM_REG_DMA2SAD + 1:
		case MEM_REG_DMA2SAD + 2:
		case MEM_REG_DMA2SAD + 3:
		case MEM_REG_DMA2DAD:
		case MEM_REG_DMA2DAD + 1:
		case MEM_REG_DMA2DAD + 2:
		case MEM_REG_DMA2DAD + 3:
		case MEM_REG_DMA2CNT_L:
		case MEM_REG_DMA2CNT_L + 1:
		case MEM_REG_DMA3SAD:
		case MEM_REG_DMA3SAD + 1:
		case MEM_REG_DMA3SAD + 2:
		case MEM_REG_DMA3SAD + 3:
		case MEM_REG_DMA3DAD:
		case MEM_REG_DMA3DAD + 1:
		case MEM_REG_DMA3DAD + 2:
		case MEM_REG_DMA3DAD + 3:
		case MEM_REG_DMA3CNT_L:
		case MEM_REG_DMA3CNT_L  +1:
		case MEM_REG_DISPCNT:
		case MEM_REG_DISPCNT + 1:
		case MEM_REG_BG0CNT:
		case MEM_REG_BG0CNT + 1:
		case MEM_REG_BG1CNT:
		case MEM_REG_BG1CNT + 1:
		case MEM_REG_BG2CNT:
		case MEM_REG_BG2CNT + 1:
		case MEM_REG_BG3CNT:
		case MEM_REG_BG3CNT + 1:
		case MEM_REG_BG0HOFS:
		case MEM_REG_BG0HOFS + 1:
		case MEM_REG_BG0VOFS:
		case MEM_REG_BG0VOFS + 1:
		case MEM_REG_BG1HOFS:
		case MEM_REG_BG1HOFS + 1:
		case MEM_REG_BG1VOFS:
		case MEM_REG_BG1VOFS + 1:
		case MEM_REG_BG2HOFS:
		case MEM_REG_BG2HOFS + 1:
		case MEM_REG_BG2VOFS:
		case MEM_REG_BG2VOFS + 1:
		case MEM_REG_BG3HOFS:
		case MEM_REG_BG3HOFS + 1:
		case MEM_REG_BG3VOFS:
		case MEM_REG_BG3VOFS + 1:
		case MEM_REG_BG2PA:
		case MEM_REG_BG2PA + 1:
		case MEM_REG_BG2PB:
		case MEM_REG_BG2PB + 1:
		case MEM_REG_BG2PC:
		case MEM_REG_BG2PC + 1:
		case MEM_REG_BG2PD:
		case MEM_REG_BG2PD + 1:
		case MEM_REG_BG3PA:
		case MEM_REG_BG3PA + 1:
		case MEM_REG_BG3PB:
		case MEM_REG_BG3PB + 1:
		case MEM_REG_BG3PC:
		case MEM_REG_BG3PC + 1:
		case MEM_REG_BG3PD:
		case MEM_REG_BG3PD + 1:
		case MEM_REG_IME:
		case MEM_REG_IME + 1:
		case MEM_REG_IE:
		case MEM_REG_IE + 1:
		case MEM_REG_TM0CNT_L:
		case MEM_REG_TM0CNT_L + 1:
		case MEM_REG_TM0CNT_H + 1:
		case MEM_REG_TM1CNT_L:
		case MEM_REG_TM1CNT_L + 1:
		case MEM_REG_TM1CNT_H + 1:
		case MEM_REG_TM2CNT_L:
		case MEM_REG_TM2CNT_L + 1:
		case MEM_REG_TM2CNT_H + 1:
		case MEM_REG_TM3CNT_L:
		case MEM_REG_TM3CNT_L + 1:
		case MEM_REG_TM3CNT_H + 1:
			break;
		default:
#if 0
			printf("writing to unknown register [%04x] = %x\n", reg, v);
#endif
			break;
	}
	mem->io_regs[reg] = v;
}

static void set_reg32(struct mem *mem, uint32_t reg, uint32_t v)
{
	set_reg(mem, reg + 0, v >> 0);
	set_reg(mem, reg + 1, v >> 8);
	set_reg(mem, reg + 2, v >> 16);
	set_reg(mem, reg + 3, v >> 24);
}

static void set_reg16(struct mem *mem, uint32_t reg, uint16_t v)
{
	set_reg(mem, reg + 0, v >> 0);
	set_reg(mem, reg + 1, v >> 8);
}

static void set_reg8(struct mem *mem, uint32_t reg, uint8_t v)
{
	set_reg(mem, reg, v);
}

static uint8_t get_reg(struct mem *mem, uint32_t reg)
{
	switch (reg)
	{
		case MEM_REG_TM0CNT_L:
			return mem->timers[0].v;
		case MEM_REG_TM0CNT_L + 1:
			return mem->timers[0].v >> 8;
		case MEM_REG_TM1CNT_L:
			return mem->timers[1].v;
		case MEM_REG_TM1CNT_L + 1:
			return mem->timers[1].v >> 8;
		case MEM_REG_TM2CNT_L:
			return mem->timers[2].v;
		case MEM_REG_TM2CNT_L + 1:
			return mem->timers[2].v >> 8;
		case MEM_REG_TM3CNT_L:
			return mem->timers[3].v;
		case MEM_REG_TM3CNT_L + 1:
			return mem->timers[3].v >> 8;
		case MEM_REG_KEYINPUT:
		{
			uint8_t v = 0xFF;
			if (mem->gba->joypad & GBA_BUTTON_A)
				v &= ~(1 << 0);
			if (mem->gba->joypad & GBA_BUTTON_B)
				v &= ~(1 << 1);
			if (mem->gba->joypad & GBA_BUTTON_SELECT)
				v &= ~(1 << 2);
			if (mem->gba->joypad & GBA_BUTTON_START)
				v &= ~(1 << 3);
			if (mem->gba->joypad & GBA_BUTTON_RIGHT)
				v &= ~(1 << 4);
			if (mem->gba->joypad & GBA_BUTTON_LEFT)
				v &= ~(1 << 5);
			if (mem->gba->joypad & GBA_BUTTON_UP)
				v &= ~(1 << 6);
			if (mem->gba->joypad & GBA_BUTTON_DOWN)
				v &= ~(1 << 7);
			return v;
		}
		case MEM_REG_KEYINPUT + 1:
		{
			uint8_t v = 0x3;
			if (mem->gba->joypad & GBA_BUTTON_R)
				v &= ~(1 << 0);
			if (mem->gba->joypad & GBA_BUTTON_L)
				v &= ~(1 << 1);
			return v;
		}
		case MEM_REG_WAVE_RAM0_L:
		case MEM_REG_WAVE_RAM0_L + 1:
		case MEM_REG_WAVE_RAM0_H:
		case MEM_REG_WAVE_RAM0_H + 1:
		case MEM_REG_WAVE_RAM1_L:
		case MEM_REG_WAVE_RAM1_L + 1:
		case MEM_REG_WAVE_RAM1_H:
		case MEM_REG_WAVE_RAM1_H + 1:
		case MEM_REG_WAVE_RAM2_L:
		case MEM_REG_WAVE_RAM2_L + 1:
		case MEM_REG_WAVE_RAM2_H:
		case MEM_REG_WAVE_RAM2_H + 1:
		case MEM_REG_WAVE_RAM3_L:
		case MEM_REG_WAVE_RAM3_L + 1:
		case MEM_REG_WAVE_RAM3_H:
		case MEM_REG_WAVE_RAM3_H + 1:
			if (mem_get_reg16(mem, MEM_REG_SOUND3CNT_L) & (1 << 6))
				return mem->wave[reg - MEM_REG_WAVE_RAM0_L];
			return mem->wave[reg - MEM_REG_WAVE_RAM0_L + 0x10];
	}
	return mem->io_regs[reg];
}

static uint32_t get_reg32(struct mem *mem, uint32_t reg)
{
	return (get_reg(mem, reg + 0) << 0)
	     | (get_reg(mem, reg + 1) << 8)
	     | (get_reg(mem, reg + 2) << 16)
	     | (get_reg(mem, reg + 3) << 24);
}

static uint16_t get_reg16(struct mem *mem, uint32_t reg)
{
	return (get_reg(mem, reg + 0) << 0)
	     | (get_reg(mem, reg + 1) << 8);
}

static uint8_t get_reg8(struct mem *mem, uint32_t reg)
{
	return get_reg(mem, reg);
}

#define MEM_GET(size) \
uint##size##_t mem_get##size(struct mem *mem, uint32_t addr) \
{ \
	if (size == 16) \
		addr &= ~1; \
	if (size == 32) \
		addr &= ~3; \
	if (addr >= 0x10000000) \
		goto end; \
	switch ((addr >> 24) & 0xF) \
	{ \
		case 0x0: /* bios */ \
			if (addr < 0x4000) \
			{ \
				if (cpu_get_reg(mem->gba->cpu, CPU_REG_PC) < 0x4000) \
					return *(uint##size##_t*)&mem->bios[addr]; \
				return *(uint##size##_t*)&mem->bios[mem->gba->cpu->last_bios_decode]; \
			} \
			break; \
		case 0x1: /* empty */ \
			break; \
		case 0x2: /* board wram */ \
		{ \
			uint32_t a = addr & 0x3FFFF; \
			return *(uint##size##_t*)&mem->board_wram[a]; \
		} \
		case 0x3: /* chip wram */ \
		{ \
			uint32_t a = addr & 0x7FFF; \
			return *(uint##size##_t*)&mem->chip_wram[a]; \
		} \
		case 0x4: /* registers */ \
		{ \
			uint32_t a = addr & 0x3FF; \
			return get_reg##size(mem, a); \
		} \
		case 0x5: /* palette */ \
		{ \
			uint32_t a = addr & 0x3FF; \
			return *(uint##size##_t*)&mem->palette[a]; \
		} \
		case 0x6: /* vram */ \
		{ \
			uint32_t a = addr & 0x1FFFF; \
			if (a >= 0x18000) \
				a &= ~0x8000; \
			return *(uint##size##_t*)&mem->vram[a]; \
		} \
		case 0x7: /* oam */ \
		{ \
			uint32_t a = addr & 0x3FF; \
			return *(uint##size##_t*)&mem->oam[a]; \
		} \
		case 0x8: \
		case 0x9: \
		case 0xA: \
		case 0xB: \
		case 0xC: \
		case 0xD: \
		case 0xE: \
		case 0xF: \
			return mbc_get##size(mem->mbc, addr); \
	} \
end: \
	/*printf("[%08x] unknown get" #size " addr: %08x\n", cpu_get_reg(mem->gba->cpu, CPU_REG_PC), addr);*/ \
	return 0; \
}

MEM_GET(8);
MEM_GET(16);
MEM_GET(32);

#define MEM_SET(size) \
void mem_set##size(struct mem *mem, uint32_t addr, uint##size##_t v) \
{ \
	if (addr >= 0x10000000) \
		goto end; \
	if (((addr >> 24) & 0xF) < 0x8) \
	{ \
		if (size == 16) \
			addr &= ~1; \
		if (size == 32) \
			addr &= ~3; \
	} \
	switch ((addr >> 24) & 0xF) \
	{ \
		case 0x0: /* bios */ \
			break; \
		case 0x1: /* empty */ \
			break; \
		case 0x2: /* board wram */ \
		{ \
			uint32_t a = addr & 0x3FFFF; \
			*(uint##size##_t*)&mem->board_wram[a] = v; \
			return; \
		} \
		case 0x3: /* chip wram */ \
		{ \
			uint32_t a = addr & 0x7FFF; \
			*(uint##size##_t*)&mem->chip_wram[a] = v; \
			return; \
		} \
		case 0x4: /* registers */ \
		{ \
			uint32_t a = addr & 0x3FF; \
			set_reg##size(mem, a, v); \
			return; \
		} \
		case 0x5: /* palette */ \
		{ \
			uint32_t a = addr & 0x3FF; \
			if (size == 8) \
				*(uint##size##_t*)&mem->palette[a + 1] = v; \
			*(uint##size##_t*)&mem->palette[a] = v; \
			return; \
		} \
		case 0x6: /* vram */ \
		{ \
			uint32_t a = addr & 0x1FFFF; \
			if (a >= 0x18000) \
				a &= ~0x8000; \
			*(uint##size##_t*)&mem->vram[a] = v; \
			return; \
		} \
		case 0x7: /* oam */ \
		{ \
			uint32_t a = addr & 0x3FF; \
			*(uint##size##_t*)&mem->oam[a] = v; \
			return; \
		} \
		case 0x8: \
		case 0x9: \
		case 0xA: \
		case 0xB: \
		case 0xC: \
		case 0xD: \
		case 0xE: \
		case 0xF: \
			mbc_set##size(mem->mbc, addr, v); \
			return; \
		default: \
			break; \
	} \
end:; \
	/*printf("[%08x] unknown set" #size " addr: %08x\n", cpu_get_reg(mem->gba->cpu, CPU_REG_PC), addr);*/ \
}

MEM_SET(8);
MEM_SET(16);
MEM_SET(32);
