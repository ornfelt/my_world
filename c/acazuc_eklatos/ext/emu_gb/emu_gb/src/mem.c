#include "mem.h"
#include "mbc.h"
#include "apu.h"
#include "cpu.h"
#include "gb.h"

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct mem *mem_new(struct gb *gb, struct mbc *mbc)
{
	struct mem *mem = calloc(1, sizeof(*mem));
	if (!mem)
	{
		fprintf(stderr, "allocation failed\n");
		return NULL;
	}

	mem->cgb = (mbc->data[0x143] & 0x80) != 0 ? CGB_YES : CGB_NO;
	mem->dmatransfer = 0xFF;
	mem->mbc = mbc;
	mem->gb = gb;
	return mem;
}

void mem_del(struct mem *mem)
{
	if (!mem)
		return;
	free(mem);
}

static uint8_t joyp_get(struct mem *mem)
{
	uint8_t joyp = mem_get_reg(mem, MEM_REG_JOYP);
	uint8_t v = 0x3F;
	if (!(joyp & (1 << 4)))
		v &= ~((1 << 4) | ((mem->joyp >> 0) & 0xF));
	if (!(joyp & (1 << 5)))
		v &= ~((1 << 5) | ((mem->joyp >> 4) & 0xF));
	return v | 0xC0;
}

static void joyp_set(struct mem *mem, uint8_t v)
{
	mem_set_reg(mem, MEM_REG_JOYP, v & 0x30);
}

static void sc_set(struct mem *mem, uint8_t v)
{
	//XXX
	(void)mem;
	(void)v;
}

static uint8_t sc_get(struct mem *mem)
{
	return mem_get_reg(mem, MEM_REG_SC) | 0x7E;
}

static uint8_t ly_get(struct mem *mem)
{
	if (!(mem_get_reg(mem, MEM_REG_LCDC) & (1 << 7)))
		return 0;
	return mem_get_reg(mem, MEM_REG_LY);
}

static uint8_t tac_get(struct mem *mem)
{
	return mem_get_reg(mem, MEM_REG_TAC) | 0xF8;
}

static uint8_t if_get(struct mem *mem)
{
	return mem_get_reg(mem, MEM_REG_IF) | 0xE0;
}

static uint8_t stat_get(struct mem *mem)
{
	uint8_t v = mem_get_reg(mem, MEM_REG_STAT) | 0x80;
	if (!(mem_get_reg(mem, MEM_REG_LCDC) & (1 << 7)))
		v &= ~0x3;
	return v;
}

static void stat_set(struct mem *mem, uint8_t v)
{
	mem_set_reg(mem, MEM_REG_STAT, (mem_get_reg(mem, MEM_REG_STAT) & 0x03) | (v & 0x7C));
}

static uint8_t div_get(struct mem *mem)
{
	return mem->timer >> 8;
}

static void div_set(struct mem *mem, uint8_t v)
{
	(void)v;
	mem->timer = 0;
}

static void dma_set(struct mem *mem, uint8_t v)
{
	if (v == 0xFE)
		v = 0xC0;
	else if (v == 0xFF)
		v = 0xDF;
	mem_set_reg(mem, MEM_REG_DMA, v);
	mem->dmatransfer = 0x9F;
}

static uint8_t nr10_get(struct mem *mem)
{
	return mem_get_reg(mem, MEM_REG_NR10) | 0x80;
}

static uint8_t nr11_get(struct mem *mem)
{
	return (mem_get_reg(mem, MEM_REG_NR11) & 0xC0) | 0x3F;
}

static void nr12_set(struct mem *mem, uint8_t v)
{
	if (!(v & 0xF8))
		mem_set_reg(mem, MEM_REG_NR52, mem_get_reg(mem, MEM_REG_NR52) & ~(1 << 0));
	mem_set_reg(mem, MEM_REG_NR12, v);
}

static uint8_t nr14_get(struct mem *mem)
{
	return mem_get_reg(mem, MEM_REG_NR14) & (1 << 6);
}

static void nr14_set(struct mem *mem, uint8_t v)
{
	mem_set_reg(mem, MEM_REG_NR14, v);
	if (v & (1 << 7))
		apu_start_channel1(mem->gb->apu);
}

static uint8_t nr21_get(struct mem *mem)
{
	return mem_get_reg(mem, MEM_REG_NR21) & 0xC0;
}

static void nr22_set(struct mem *mem, uint8_t v)
{
	if (!(v & 0xF8))
		mem_set_reg(mem, MEM_REG_NR52, mem_get_reg(mem, MEM_REG_NR52) & ~(1 << 1));
	mem_set_reg(mem, MEM_REG_NR22, v);
}

static uint8_t nr24_get(struct mem *mem)
{
	return mem_get_reg(mem, MEM_REG_NR24) & (1 << 6);
}

static void nr24_set(struct mem *mem, uint8_t v)
{
	mem_set_reg(mem, MEM_REG_NR24, v);
	if (v & (1 << 7))
		apu_start_channel2(mem->gb->apu);
}

static uint8_t nr30_get(struct mem *mem)
{
	return mem_get_reg(mem, MEM_REG_NR30) | 0x7F;
}

static void nr30_set(struct mem *mem, uint8_t v)
{
	mem_set_reg(mem, MEM_REG_NR30, v & 0x80);
}

static uint8_t nr32_get(struct mem *mem)
{
	return mem_get_reg(mem, MEM_REG_NR32) | 0x9F;
}

static uint8_t nr34_get(struct mem *mem)
{
	return mem_get_reg(mem, MEM_REG_NR34) & (1 << 6);
}

static void nr34_set(struct mem *mem, uint8_t v)
{
	mem_set_reg(mem, MEM_REG_NR34, v);
	if (v & (1 << 7))
		apu_start_channel3(mem->gb->apu);
}

static uint8_t nr41_get(struct mem *mem)
{
	return mem_get_reg(mem, MEM_REG_NR41) | 0xC0;
}

static uint8_t nr44_get(struct mem *mem)
{
	return (mem_get_reg(mem, MEM_REG_NR24) & (1 << 6)) | 0x3F;
}

static void nr44_set(struct mem *mem, uint8_t v)
{
	mem_set_reg(mem, MEM_REG_NR24, v);
	if (v & (1 << 7))
		apu_start_channel4(mem->gb->apu);
}

static uint8_t nr52_get(struct mem *mem)
{
	return mem_get_reg(mem, MEM_REG_NR52) | 0x70;
}

static void nr52_set(struct mem *mem, uint8_t v)
{
	if (v & (1 << 7))
		mem_set_reg(mem, MEM_REG_NR52, (mem_get_reg(mem, MEM_REG_NR52) & 0x7F) | (1 << 7));
	else
		mem_set_reg(mem, MEM_REG_NR52, 0);
}

static uint8_t bcpd_get(struct mem *mem)
{
	uint8_t bcps = mem_get_reg(mem, MEM_REG_BCPS);
	return mem->bgpalette[bcps & 0x3F];
}

static void bcpd_set(struct mem *mem, uint8_t v)
{
	uint8_t bcps = mem_get_reg(mem, MEM_REG_BCPS);
	mem->bgpalette[bcps & 0x3F] = v;
	if (bcps & (1 << 7))
		mem_set_reg(mem, MEM_REG_BCPS, 0xC0 | (((bcps & 0x3F) + 1) & 0x3F));
}

static uint8_t ocpd_get(struct mem *mem)
{
	uint8_t ocps = mem_get_reg(mem, MEM_REG_OCPS);
	return mem->objpalette[ocps & 0x3F];
}

static void ocpd_set(struct mem *mem, uint8_t v)
{
	uint8_t ocps = mem_get_reg(mem, MEM_REG_OCPS);
	mem->objpalette[ocps & 0x3F] = v;
	if (ocps & (1 << 7))
		mem_set_reg(mem, MEM_REG_OCPS, (ocps & 0xC0) | (((ocps & 0x3F) + 1) & 0x3F));
}

static void svbk_set(struct mem *mem, uint8_t v)
{
	if (mem->cgb == CGB_NO)
		return;
	mem_set_reg(mem, MEM_REG_SVBK, v);
	v &= 0x7;
	if (v)
		v--;
	mem->svbk = v;
}

static void vbk_set(struct mem *mem, uint8_t v)
{
	if (mem->cgb == CGB_NO)
		return;
	mem_set_reg(mem, MEM_REG_VBK, v);
	mem->vbk = v & 1;
}

static void hdm1_set(struct mem *mem, uint8_t v)
{
	mem->hdma_src = ((v << 8) | (mem->hdma_src & 0xFF)) & 0xFFF0;
}

static void hdm2_set(struct mem *mem, uint8_t v)
{
	mem->hdma_src = ((mem->hdma_src & 0xFF00) | v) & 0xFFF0;
}

static void hdm3_set(struct mem *mem, uint8_t v)
{
	mem->hdma_dst = (((v << 8) | (mem->hdma_dst & 0xFF)) & 0x1FF0) | 0x8000;
}

static void hdm4_set(struct mem *mem, uint8_t v)
{
	mem->hdma_dst = (((mem->hdma_dst & 0xFF00) | v) & 0x1FF0) | 0x8000;
}

static uint8_t hdm5_get(struct mem *mem)
{
	if (mem->cgb == CGB_NO)
		return 0xFF;
	return mem_get_reg(mem, MEM_REG_HDM5);
}

static void hdm5_set(struct mem *mem, uint8_t v)
{
	if (mem->cgb == CGB_NO)
		return;
	uint16_t len = ((v & 0x7F) + 1) << 4;
	if (v & (1 << 7))
	{
		mem->hdma_len = len;
		mem_set_reg(mem, MEM_REG_HDM5, v & 0x7F);
		return;
	}
	if (mem->hdma_len)
	{
		mem->hdma_len = 0;
		mem_set_reg(mem, MEM_REG_HDM5, mem_get_reg(mem, MEM_REG_HDM5) | 0x80);
		return;
	}
	mem->hdma_len = 0;
	for (uint16_t i = 0; i < len; ++i)
	{
		uint8_t d = mem_get(mem, mem->hdma_src++);
		mem_set(mem, mem->hdma_dst++, d);
	}
	mem->gb->cpu->hdma_wait = (mem->doublespeed ? 16 : 8) * (len >> 4);
	mem_set_reg(mem, MEM_REG_HDM5, 0xFF);
}

static uint8_t key1_get(struct mem *mem)
{
	if (mem->cgb == CGB_NO)
		return 0;
	uint8_t v = mem_get_reg(mem, MEM_REG_KEY1);
	if (mem->doublespeed)
		v |= 0x80;
	return v;
}

static void key1_set(struct mem *mem, uint8_t v)
{
	if (mem->cgb == CGB_NO)
		return;
	v &= 1;
	mem_set_reg(mem, MEM_REG_KEY1, (mem_get_reg(mem, MEM_REG_KEY1) & 0xFE) | v);
}

static uint8_t get(struct mem *mem, uint16_t addr)
{
	switch (addr >> 12)
	{
		case 0x0: /* bios / mbc */
			if (mem->cgb != CGB_NO)
			{
				if (addr < 0x100)
				{
					if (!mem_get_reg(mem, MEM_REG_BOOT))
						return mem->cgb_bios[addr];
				}
				if (addr >= 0x200 && addr < 0x900)
				{
					if (!mem_get_reg(mem, MEM_REG_BOOT))
						return mem->cgb_bios[addr];
				}
			}
			else
			{
				if (addr < 0x100)
				{
					if (!mem_get_reg(mem, MEM_REG_BOOT))
						return mem->dmg_bios[addr];
				}
			}
			/* FALLTHROUGH */
		case 0x1:
		case 0x2:
		case 0x3:
		case 0x4:
		case 0x5:
		case 0x6:
		case 0x7: /* mbc */
			return mbc_get(mem->mbc, addr);
		case 0x8:
		case 0x9: /* vram */
			return mem->vram[addr - 0x8000 + 0x2000 * mem->vbk];
		case 0xA:
		case 0xB: /* external ram */
			return mbc_get(mem->mbc, addr);
		case 0xC: /* work ram bank 0 */
			return mem->workram0[addr - 0xC000];
		case 0xD: /* work ram bank n */
			return mem->workram1[addr - 0xD000 + 0x1000 * mem->svbk];
		case 0xE: /* echo ram C000-D000 */
			return mem->workram0[addr - 0xE000];
		case 0xF:
			if (addr < 0xFE00) /* echo ram D000-DDFF */
				return mem->workram1[addr - 0xF000 + 0x1000 * mem->svbk];

			if (addr < 0xFEA0) /* OAM */
				return mem->oam[addr - 0xFE00];

			if (addr < 0xFF00) /* unmapped */
				return 0;

			if (mem->cgb == CGB_NO && addr >= 0xFF4C && addr <= 0xFF7F)
				return 0xFF;

			switch (addr)
			{
				case 0xFF03:
				case 0xFF08:
				case 0xFF09:
				case 0xFF0A:
				case 0xFF0B:
				case 0xFF0C:
				case 0xFF0D:
				case 0xFF0E:
				case 0xFF15:
				case 0xFF1F:
				case 0xFF27:
				case 0xFF28:
				case 0xFF29:
					return 0xFF;
				case MEM_REG_JOYP:
					return joyp_get(mem);
				case MEM_REG_SC:
					return sc_get(mem);
				case MEM_REG_LY:
					return ly_get(mem);
				case MEM_REG_DIV:
					return div_get(mem);
				case MEM_REG_TAC:
					return tac_get(mem);
				case MEM_REG_IF:
					return if_get(mem);
				case MEM_REG_STAT:
					return stat_get(mem);
				case MEM_REG_NR10:
					return nr10_get(mem);
				case MEM_REG_NR11:
					return nr11_get(mem);
				case MEM_REG_NR13:
					return 0;
				case MEM_REG_NR14:
					return nr14_get(mem);
				case MEM_REG_NR21:
					return nr21_get(mem);
				case MEM_REG_NR23:
					return 0;
				case MEM_REG_NR24:
					return nr24_get(mem);
				case MEM_REG_NR30:
					return nr30_get(mem);
				case MEM_REG_NR32:
					return nr32_get(mem);
				case MEM_REG_NR33:
					return 0;
				case MEM_REG_NR34:
					return nr34_get(mem);
				case MEM_REG_NR41:
					return nr41_get(mem);
				case MEM_REG_NR44:
					return nr44_get(mem);
				case MEM_REG_NR52:
					return nr52_get(mem);
				case MEM_REG_BCPD:
					return bcpd_get(mem);
				case MEM_REG_OCPD:
					return ocpd_get(mem);
				case MEM_REG_HDM5:
					return hdm5_get(mem);
				case MEM_REG_KEY1:
					return key1_get(mem);
			}
			return mem->highram[addr - 0xFF00];
	}
	return 0;
}

static void set(struct mem *mem, uint16_t addr, uint8_t v)
{
	switch (addr >> 12)
	{
		case 0x0:
		case 0x1:
		case 0x2:
		case 0x3:
		case 0x4:
		case 0x5:
		case 0x6:
		case 0x7:
			if (addr < 0x100)
			{
				if (mem_get_reg(mem, MEM_REG_BOOT))
					mbc_set(mem->mbc, addr, v);
				return;
			}
			mbc_set(mem->mbc, addr, v);
			break;
		case 0x8:
		case 0x9: /* vram */
			mem->vram[addr - 0x8000 + 0x2000 * mem->vbk] = v;
			break;
		case 0xA:
		case 0xB: /* external ram */
			mbc_set(mem->mbc, addr, v);
			break;
		case 0xC: /* work ram bank 0 */
			mem->workram0[addr - 0xC000] = v;
			break;
		case 0xD: /* work ram bank n */
			mem->workram1[addr - 0xD000 + 0x1000 * mem->svbk] = v;
			break;
		case 0xE: /* echo ram C000-D000 */
			mem->workram0[addr - 0xE000] = v;
			break;
		case 0xF:
			if (addr < 0xFE00) /* echo ram D000-DDFF */
			{
				mem->workram1[addr - 0xF000 + 0x1000 * mem->svbk] = v;
				break;
			}
			if (addr < 0xFEA0) /* OAM */
			{
				mem->oam[addr - 0xFE00] = v;
				break;
			}
			if (addr < 0xFF00) /* unmapped */
				break;
			switch (addr)
			{
				case MEM_REG_JOYP:
					joyp_set(mem, v);
					return;
				case MEM_REG_SC:
					sc_set(mem, v);
					return;
				case MEM_REG_LY:
					return;
				case MEM_REG_STAT:
					stat_set(mem, v);
					return;
				case MEM_REG_DIV:
					div_set(mem, v);
					return;
				case MEM_REG_DMA:
					dma_set(mem, v);
					return;
				case MEM_REG_NR12:
					nr12_set(mem, v);
					return;
				case MEM_REG_NR14:
					nr14_set(mem, v);
					return;
				case MEM_REG_NR22:
					nr22_set(mem, v);
					return;
				case MEM_REG_NR24:
					nr24_set(mem, v);
					return;
				case MEM_REG_NR30:
					nr30_set(mem, v);
					return;
				case MEM_REG_NR34:
					nr34_set(mem, v);
					return;
				case MEM_REG_NR44:
					nr44_set(mem, v);
					return;
				case MEM_REG_NR52:
					nr52_set(mem, v);
					return;
				case MEM_REG_BCPD:
					bcpd_set(mem, v);
					return;
				case MEM_REG_OCPD:
					ocpd_set(mem, v);
					return;
				case MEM_REG_SVBK:
					svbk_set(mem, v);
					return;
				case MEM_REG_VBK:
					vbk_set(mem, v);
					return;
				case MEM_REG_HDM1:
					hdm1_set(mem, v);
					return;
				case MEM_REG_HDM2:
					hdm2_set(mem, v);
					return;
				case MEM_REG_HDM3:
					hdm3_set(mem, v);
					return;
				case MEM_REG_HDM4:
					hdm4_set(mem, v);
					return;
				case MEM_REG_HDM5:
					hdm5_set(mem, v);
					return;
				case MEM_REG_KEY1:
					key1_set(mem, v);
					return;
				case MEM_REG_KEY0:
					break;
			}
			mem->highram[addr - 0xFF00] = v;
			break;
	}
}

void mem_dmatransfer(struct mem *mem)
{
	uint8_t i = 0x9F - mem->dmatransfer;
	mem->oam[i] = get(mem, mem_get_reg(mem, MEM_REG_DMA) * 0x100 + i);
}

void mem_hdmatransfer(struct mem *mem)
{
	if (!mem->hdma_len)
		return;
	mem->gb->cpu->hdma_wait = mem->doublespeed ? 16 : 8;
	for (size_t i = 0; i < 0x10; ++i)
	{
		uint8_t v = mem_get(mem, mem->hdma_src++);
		mem_set(mem, mem->hdma_dst++, v);
	}
	mem->hdma_len -= 0x10;

	if (!mem->hdma_len)
	{
		mem_set_reg(mem, MEM_REG_HDM5, 0xFF);
		return;
	}

	uint8_t hdm5 = mem_get_reg(mem, MEM_REG_HDM5);
	mem_set_reg(mem, MEM_REG_HDM5, (hdm5 & 0x7F) - 1);
}

uint8_t mem_get(struct mem *mem, uint16_t addr)
{
	if (mem->dmatransfer != 0xFF)
	{
		if (addr >= 0xFE00 && addr < 0xFEA0)
			return 0xFF;
		if (addr == MEM_REG_DMA || (addr >= 0xFF80 && addr < 0xFFFF))
			return mem->highram[addr - 0xFF00];
		return 0;
	}

	return get(mem, addr);
}

void mem_set(struct mem *mem, uint16_t addr, uint8_t v)
{
	if (mem->dmatransfer != 0xFF)
	{
		if (addr == MEM_REG_DMA || (addr >= 0xFF80 && addr < 0xFFFF))
			mem->highram[addr - 0xFF00] = v;
		return;
	}
	set(mem, addr, v);
}
