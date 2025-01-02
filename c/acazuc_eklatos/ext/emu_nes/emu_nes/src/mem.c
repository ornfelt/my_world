#include "internal.h"
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>

void mem_init(struct mem *mem, struct nes *nes)
{
	mem->nes = nes;
}

uint8_t mem_get(struct mem *mem, uint16_t addr)
{
#if 0
	uint16_t org = addr;
#endif
	uint8_t v;
	if (addr < 0x2000)
	{
		addr &= 0x7FF;
		v = mem->wram[addr];
		goto end;
	}
	if (addr < 0x4000)
	{
		addr &= 7;
		switch (addr)
		{
			case 0x0:
			case 0x1:
			case 0x3:
			case 0x5:
			case 0x6:
#if 0
				printf("read from RO gpu register 0x200%" PRIx16 "\n", addr);
#endif
				v = 0;
				goto end;
			case 0x2:
#if 0
				printf("reset vram ff read ppu status\n");
#endif
				mem->vram_w = 0;
				v = mem->gpu_regs[addr];
				mem->gpu_regs[addr] &= ~0x80;
				goto end;
			case 0x4:
				v = mem->spram[mem->spram_addr];
				goto end;
			case 0x7:
			{
#if 0
				printf("vram get [%04" PRIx16 "]\n", mem->vram_addr);
#endif
				if ((mem->vram_v & 0x3F00) < 0x3F00)
				{
					v = mem->vram_latch;
					mem->vram_latch = mem_gpu_get(mem, mem->vram_v);
				}
				else
				{
					v = mem_gpu_get(mem, mem->vram_v);
				}
				mem->vram_v += (mem->gpu_regs[0x0] & 0x4) ? 32 : 1;
				mem->vram_v &= 0x7FFF;
				goto end;
			}
			default:
				v = mem->gpu_regs[addr];
				goto end;
		}
	}
	if (addr < 0x4018)
	{
		switch (addr)
		{
			case 0x4016:
				if (mem->joypad_latch)
					mem->joypad_shift = 0;
				if (mem->joypad_shift < 8)
				{
					static const uint8_t masks[] =
					{
						NES_BUTTON_A,
						NES_BUTTON_B,
						NES_BUTTON_SELECT,
						NES_BUTTON_START,
						NES_BUTTON_UP,
						NES_BUTTON_DOWN,
						NES_BUTTON_LEFT,
						NES_BUTTON_RIGHT
					};
					v = (mem->joypad_state & masks[mem->joypad_shift]) ? 1 : 0;
					mem->joypad_shift++;
				}
				else
				{
					v = 1;
				}
				break;
			default:
				/* XXX APU IO */
				v = 0;
				break;
		}
		goto end;
	}
	v = mbc_get(&mem->nes->mbc, addr);
end:
#if 0
	printf("get [0x%04" PRIx16 "] = %02" PRIx8 "\n", org, v);
#endif
	return v;
}

void mem_set(struct mem *mem, uint16_t addr, uint8_t v)
{
#if 0
	printf("set [0x%04" PRIx16 "] = %02" PRIx8 "\n", addr, v);
#endif
	if (addr < 0x2000)
	{
		addr &= 0x7FF;
		mem->wram[addr] = v;
		return;
	}
	if (addr < 0x4000)
	{
		addr &= 0x7;
		switch (addr)
		{
			case 0x0:
#if 0
				printf("set gpu reg [%04" PRIx16 "] = %02" PRIx8 "\n", addr, v);
#endif
				mem->gpu_regs[addr] = v;
				mem->vram_t = (mem->vram_t & ~0xC00) | ((v & 3) << 10);
				break;
			case 0x1:
#if 0
				printf("set gpu reg [%04" PRIx16 "] = %02" PRIx8 "\n", addr, v);
#endif
				mem->gpu_regs[addr] = v;
				return;
			case 0x2:
#if 0
				printf("write to RO gpu register 0x200%" PRIx16 "\n", addr);
#endif
				return;
			case 0x3:
				mem->spram_addr = v;
				return;
			case 0x4:
				mem->spram[mem->spram_addr++] = v;
				return;
			case 0x5:
#if 0
				printf("writing VRAM t %02" PRIx8 " (%d)\n", v, mem->vram_w);
#endif
				if (mem->vram_w)
				{
					mem->vram_t = (mem->vram_t & ~0x73E0) | ((v & 0xF8) << 2) | ((v & 0x7) << 12);
				}
				else
				{
					mem->vram_t = (mem->vram_t & ~0x1F) | (v >> 3);
					mem->vram_x = v & 7;
				}
				mem->vram_w = !mem->vram_w;
				return;
			case 0x6:
#if 0
				printf("writing VRAM v %02" PRIx8 " (%d)\n", v, mem->vram_w);
#endif
				if (mem->vram_w)
				{
					mem->vram_t = (mem->vram_t & ~0xFF) | v;
					mem->vram_v = mem->vram_t;
				}
				else
				{
					mem->vram_t = (mem->vram_t & ~0x7F00) | ((v & 0x3F) << 8);
				}
				mem->vram_w = !mem->vram_w;
				return;
			case 0x7:
#if 0
				printf("writing VRAM data [%04" PRIx16 "] = %02" PRIx8 "\n", mem->vram_v, v);
#endif
				mem_gpu_set(mem, mem->vram_v, v);
				mem->vram_v += (mem->gpu_regs[0x0] & 0x4) ? 32 : 1;
				mem->vram_v &= 0x7FFF;
				return;
		}
		return;
	}
	if (addr < 0x4018)
	{
		static const uint8_t lengths[] =
		{
			0x05, 0x7F, 0x0A, 0x01, 0x14, 0x02, 0x28, 0x03,
			0x50, 0x04, 0x1E, 0x05, 0x07, 0x06, 0x0D, 0x07,
			0x06, 0x89, 0x0C, 0x09, 0x18, 0x0A, 0x30, 0x0B,
			0x60, 0x0C, 0x24, 0x0D, 0x08, 0x0E, 0x10, 0x0F,
		};
		switch (addr)
		{
			case 0x4000:
				mem->nes->apu.wave1.status = v;
				return;
			case 0x4001:
				mem->nes->apu.wave1.sweep = v;
				return;
			case 0x4002:
				mem->nes->apu.wave1.frequency = (mem->nes->apu.wave1.frequency & ~0xFF) | v;
				return;
			case 0x4003:
				mem->nes->apu.wave1.frequency = (mem->nes->apu.wave1.frequency & ~0x700) | ((v & 0x7) << 8);
				mem->nes->apu.wave1.length = lengths[v >> 3] + 1;
				mem->nes->apu.wave1.decay = 0xF;
				return;
			case 0x4004:
				mem->nes->apu.wave2.status = v;
				return;
			case 0x4005:
				mem->nes->apu.wave2.sweep = v;
				return;
			case 0x4006:
				mem->nes->apu.wave2.frequency = (mem->nes->apu.wave2.frequency & ~0xFF) | v;
				return;
			case 0x4007:
				mem->nes->apu.wave2.frequency = (mem->nes->apu.wave2.frequency & ~0x700) | ((v & 0x7) << 8);
				mem->nes->apu.wave2.length = lengths[v >> 3] + 1;
				mem->nes->apu.wave2.decay = 0xF;
				return;
			case 0x4008:
				mem->nes->apu.triangle.status = v;
				return;
			case 0x400A:
				mem->nes->apu.triangle.frequency = (mem->nes->apu.triangle.frequency & ~0xFF) | v;
				return;
			case 0x400B:
				mem->nes->apu.triangle.frequency = (mem->nes->apu.triangle.frequency & ~0x700) | ((v & 0x7) << 8);
				mem->nes->apu.triangle.length = lengths[v >> 3] + 1;
				mem->nes->apu.triangle.reload = 1;
				return;
			case 0x400C:
				mem->nes->apu.noise.status = v;
				return;
			case 0x400E:
				mem->nes->apu.noise.frequency = v;
				return;
			case 0x400F:
				mem->nes->apu.noise.length = lengths[v >> 3] + 1;
				mem->nes->apu.noise.decay = 0xF;
				return;
			case 0x4014:
				for (size_t i = 0; i < 256; ++i)
					mem_set(mem, 0x2004, mem_get(mem, v * 0x100 + i));
				mem->nes->cpu.instr_delay += 512;
				return;
			case 0x4015:
				mem->nes->apu.status = v;
				return;
			case 0x4016:
				mem->joypad_latch = v & 1;
				if (mem->joypad_latch)
					mem->joypad_shift = 0;
				return;
			case 0x4017:
				mem->nes->apu.control = v;
				mem->nes->apu.frame_clock = 0;
				return;
			default:
				/* XXX APU IO */
				return;
		}
	}
	mbc_set(&mem->nes->mbc, addr, v);
}

uint8_t mem_gpu_get(struct mem *mem, uint16_t addr)
{
#if 0
	uint16_t org = addr;
#endif
	addr &= 0x3FFF;
	uint8_t v;
	switch (addr >> 12)
	{
		case 0x0:
		case 0x1:
			v = mbc_gpu_get(&mem->nes->mbc, addr);
			break;
		case 0x2:
			switch (mem->nes->mbc.mapping)
			{
				case 0:
					addr = (addr & ~0xC00) | ((addr >> 1) & 0x400);
					break;
				case 1:
					addr &= ~0x800;
					break;
				case 2:
					break;
			}
			if (addr < 0x2800)
				v = mem->gpu_names[addr - 0x2000];
			else
				v = mbc_gpu_get(&mem->nes->mbc, addr);
			break;
		case 0x3:
			if (addr < 0x3F00)
				v = mem_gpu_get(mem, addr - 0x1000);
			else
				v = mem->gpu_palettes[palette_table[addr & 0x1F]];
			break;
		default:
			printf("get unknown gpu memory %04" PRIx16 "\n", addr);
			v = 0;
			break;
	}
#if 0
	printf("gpu get [0x%04" PRIx16 "] = %02" PRIx8 "\n", org, v);
#endif
	return v;
}

void mem_gpu_set(struct mem *mem, uint16_t addr, uint8_t v)
{
#if 0
	printf("gpu set [0x%04" PRIx16 "] = %02" PRIx8 "\n", addr, v);
#endif
	addr &= 0x3FFF;
	switch (addr >> 12)
	{
		case 0x0:
		case 0x1:
			mbc_gpu_set(&mem->nes->mbc, addr, v);
			break;
		case 0x2:
			switch (mem->nes->mbc.mapping)
			{
				case 0:
					addr = (addr & ~0xC00) | ((addr >> 1) & 0x400);
					break;
				case 1:
					addr &= ~0x800;
					break;
				case 2:
					break;
			}
			if (addr < 0x2800)
				mem->gpu_names[addr - 0x2000] = v;
			else
				mbc_gpu_set(&mem->nes->mbc, addr, v);
			break;
		case 0x3:
			if (addr < 0x3F00)
			{
				mem_gpu_set(mem, addr - 0x1000, v);
				return;
			}
			mem->gpu_palettes[palette_table[addr & 0x1F]] = v;
			break;
		default:
			printf("set unknown gpu memory %04" PRIx16 " = %02" PRIx8 "\n",
			       addr, v);
			return;
	}
}
