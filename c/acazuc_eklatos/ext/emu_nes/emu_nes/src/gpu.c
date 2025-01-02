#include "internal.h"
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>

void gpu_init(struct gpu *gpu, struct nes *nes)
{
	gpu->nes = nes;
}

static int draw_bg(struct gpu *gpu, uint8_t *color)
{
	uint8_t cr1 = mem_get_gpu_reg(&gpu->nes->mem, MEM_REG_GPU_CR1);
	uint16_t vram_addr = gpu->nes->mem.vram_v;
	uint8_t palid = mem_gpu_get(&gpu->nes->mem, 0x23C0 | (vram_addr & 0xC00) | ((vram_addr >> 2) & 0x7) | ((vram_addr >> 4) & 0x38));
	uint8_t pals = (vram_addr & 0x2) | ((vram_addr >> 4) & 0x4);
	palid = (palid >> pals) & 0x3;
	uint16_t addr = gpu->chr * 16;
	if (cr1 & (1 << 4))
		addr += 0x1000;
	uint8_t v = ((gpu->bg_v1 >> 7) & 1)
	          | ((gpu->bg_v2 >> 6) & 2);
	if (v)
	{
		*color = mem_get_palette(&gpu->nes->mem, (palid << 2) | v);
		return 1;
	}
	return 0;
}

static void draw_spr(struct gpu *gpu, uint8_t *color, int bg_drawn, uint8_t *collision)
{
	uint8_t cr1 = mem_get_gpu_reg(&gpu->nes->mem, MEM_REG_GPU_CR1);
	uint8_t h16 = cr1 & (1 << 5);
	uint8_t height = h16 ? 16 : 8;
	for (size_t i = 0; i < 64; ++i)
	{
		uint8_t y = mem_get_spr(&gpu->nes->mem, i * 4 + 0);
		uint8_t chr = mem_get_spr(&gpu->nes->mem, i * 4 + 1);
		uint8_t attr = mem_get_spr(&gpu->nes->mem, i * 4 + 2);
		uint8_t x = mem_get_spr(&gpu->nes->mem, i * 4 + 3);
		if (gpu->y <= y)
			continue;
		if (gpu->x < x)
			continue;
		if (gpu->x >= x + 8)
			continue;
		if (gpu->y - height > y)
			continue;
		uint8_t py = gpu->y - y - 1;
		uint8_t px = gpu->x - x;
		if (attr & (1 << 7))
			py = height - py - 1;
		if (attr & (1 << 6))
			px = 7 - px;
		uint8_t palid = attr & 0x3;
		uint16_t addr;
		if (h16)
		{
			addr = (chr & ~1) * 16;
			if (chr & (1 << 0))
				addr += 0x1000;
			if (py >= 8)
			{
				addr += 16;
				py -= 8;
			}
		}
		else
		{
			addr = chr * 16;
			if (cr1 & (1 << 3))
				addr += 0x1000;
		}
		uint8_t v1 = mem_gpu_get(&gpu->nes->mem, addr + py + 0);
		uint8_t v2 = mem_gpu_get(&gpu->nes->mem, addr + py + 8);
		uint8_t v = (((v1 >> (7 - px)) & 1) << 0)
		          | (((v2 >> (7 - px)) & 1) << 1);
		if (v)
		{
			if (bg_drawn && !i)
				*collision = 1;
			if (!bg_drawn || !(attr & (1 << 5)))
				*color = mem_get_palette(&gpu->nes->mem, 0x10 | (palid << 2) | v);
			return;
		}
	}
}

static void gpu_cycle(struct gpu *gpu)
{
	uint8_t cr1 = mem_get_gpu_reg(&gpu->nes->mem, MEM_REG_GPU_CR1);
	uint8_t cr2 = mem_get_gpu_reg(&gpu->nes->mem, MEM_REG_GPU_CR2);
	uint8_t collision_flag = 0;
	if (gpu->x < 256 && gpu->y < 240)
	{
		if (cr2 & (0x3 << 3))
		{
			uint8_t color = mem_get_palette(&gpu->nes->mem, 0);
			int bg_drawn;
			if ((cr2 & (1 << 3)) && (gpu->x >= 8 || (cr2 & (1 << 1))))
				bg_drawn = draw_bg(gpu, &color);
			else
				bg_drawn = 0;
			if ((cr2 & (1 << 4)) && (gpu->x >= 8 || (cr2 & (1 << 2))))
				draw_spr(gpu, &color, bg_drawn, &collision_flag);
			static const uint8_t colors[3 * 0x40] =
			{
				124, 124, 124,
				  0,   0, 252,
				  0,   0, 188,
				 68,  40, 188,
				148,   0, 132,
				168,   0,  32,
				168,  16,   0,
				136,  20,   0,
				 80,  48,   0,
				  0, 120,   0,
				  0, 104,   0,
				  0,  88,   0,
				  0,  64,  88,
				  0,   0,   0,
				  0,   0,   0,
				  0,   0,   0,
				188, 188, 188,
				  0, 120, 248,
				  0,  88, 248,
				104,  68, 252,
				216,   0, 204,
				228,   0,  88,
				248,  56,   0,
				228,  92,  16,
				172, 124,   0,
				  0, 184,   0,
				  0, 168,   0,
				  0, 168,  68,
				  0, 136, 136,
				  0,   0,   0,
				  0,   0,   0,
				  0,   0,   0,
				248, 248, 248,
				 60, 188, 252,
				104, 136, 252,
				152, 120, 248,
				248, 120, 248,
				248,  88, 152,
				248, 120,  88,
				252, 160,  68,
				248, 184,   0,
				184, 248,  24,
				 88, 216,  84,
				 88, 248, 152,
				  0, 232, 216,
				120, 120, 120,
				  0,   0,   0,
				  0,   0,   0,
				252, 252, 252,
				164, 228, 252,
				184, 184, 248,
				216, 184, 248,
				248, 184, 248,
				248, 164, 192,
				240, 208, 176,
				252, 224, 168,
				248, 216, 120,
				216, 248, 120,
				184, 248, 184,
				184, 248, 216,
				  0, 252, 252,
				248, 216, 248,
				  0,   0,   0,
				  0,   0,   0,
			};
			uint32_t idx = (gpu->x + gpu->y * 256) * 4;
			uint8_t col = 3 * (color & 0x3F);
			gpu->data[idx + 0] = colors[col + 2];
			gpu->data[idx + 1] = colors[col + 1];
			gpu->data[idx + 2] = colors[col + 0];
			gpu->data[idx + 3] = 0xff;
		}
		else
		{
			uint32_t idx = (gpu->x + gpu->y * 256) * 4;
			gpu->data[idx + 0] = 0;
			gpu->data[idx + 1] = 0;
			gpu->data[idx + 2] = 0;
			gpu->data[idx + 3] = 0xff;
		}
	}
	if (cr2 & (0x3 << 3))
	{
		if (gpu->x == 255 && gpu->y < 240)
		{
			if (!gpu->nes->mbc.irq_value || gpu->nes->mbc.irq_latch)
			{
				gpu->nes->mbc.irq_value = gpu->nes->mbc.irq_reload;
				gpu->nes->mbc.irq_latch = 0;
			}
			else
			{
				gpu->nes->mbc.irq_value--;
			}
			if (!CPU_GET_FLAG_I(&gpu->nes->cpu) && gpu->nes->mbc.irq_state && !gpu->nes->mbc.irq_value)
				gpu->nes->cpu.irq = 1;
		}
		if (gpu->y == 261 && gpu->x >= 279 && gpu->x < 304)
		{
			gpu->nes->mem.vram_v &= ~0x7BE0;
			gpu->nes->mem.vram_v |= gpu->nes->mem.vram_t & 0x7BE0;
#if 0
			printf("vblank reset to %x\n", gpu->nes->mem.vram_v);
#endif
		}
		if (gpu->y < 240 || gpu->y == 261)
		{
			if (gpu->x < 256)
			{
				if (((gpu->x + gpu->nes->mem.vram_x) & 0x7) == 0x7)
				{
					if ((gpu->nes->mem.vram_v & 0x1F) == 0x1F)
					{
						gpu->nes->mem.vram_v ^= 0x400;
						gpu->nes->mem.vram_v &= ~0x1F;
					}
					else
					{
						gpu->nes->mem.vram_v++;
					}
					gpu->chr = mem_gpu_get(&gpu->nes->mem, 0x2000 | (gpu->nes->mem.vram_v & 0xFFF));
					uint16_t chr_addr = gpu->chr * 16;
					if (cr1 & (1 << 4))
						chr_addr += 0x1000;
					chr_addr += gpu->nes->mem.vram_v >> 12;
					gpu->bg_v1 = mem_gpu_get(&gpu->nes->mem, chr_addr + 0);
					gpu->bg_v2 = mem_gpu_get(&gpu->nes->mem, chr_addr + 8);
				}
				else
				{
					gpu->bg_v1 <<= 1;
					gpu->bg_v2 <<= 1;
				}
			}
			if (gpu->x == 255)
			{
				if ((gpu->nes->mem.vram_v & 0x7000) == 0x7000)
				{
					gpu->nes->mem.vram_v &= ~0x7000;
					if ((gpu->nes->mem.vram_v & 0x3E0) == 0x3A0)
					{
						gpu->nes->mem.vram_v ^= 0x800;
						gpu->nes->mem.vram_v &= ~0x3E0;
					}
					else if ((gpu->nes->mem.vram_v & 0x3E0) == 0x3E0)
					{
						gpu->nes->mem.vram_v &= ~0x3E0;
					}
					else
					{
						gpu->nes->mem.vram_v += 0x20;
					}
				}
				else
				{
					gpu->nes->mem.vram_v += 0x1000;
				}
			}
			if (gpu->x == 256)
			{
				gpu->nes->mem.vram_v &= ~0x41F;
				gpu->nes->mem.vram_v |= gpu->nes->mem.vram_t & 0x41F;
				gpu->chr = mem_gpu_get(&gpu->nes->mem, 0x2000 | (gpu->nes->mem.vram_v & 0xFFF));
				uint16_t chr_addr = gpu->chr * 16;
				if (cr1 & (1 << 4))
					chr_addr += 0x1000;
				chr_addr += gpu->nes->mem.vram_v >> 12;
				gpu->bg_v1 = mem_gpu_get(&gpu->nes->mem, chr_addr + 0);
				gpu->bg_v2 = mem_gpu_get(&gpu->nes->mem, chr_addr + 8);
				gpu->bg_v1 <<= gpu->nes->mem.vram_x;
				gpu->bg_v2 <<= gpu->nes->mem.vram_x;
	#if 0
				printf("hblank %u reset to %04x (%04x)\n", gpu->y, gpu->nes->mem.vram_v, gpu->nes->mem.vram_t);
	#endif
			}
		}
	}
	if (gpu->y < 240)
	{
		mem_set_gpu_reg(&gpu->nes->mem, MEM_REG_GPU_STATUS,
		                mem_get_gpu_reg(&gpu->nes->mem, MEM_REG_GPU_STATUS)
		              | (collision_flag << 6));
	}
	if (gpu->y == 240 && gpu->x == 1)
	{
		mem_set_gpu_reg(&gpu->nes->mem, MEM_REG_GPU_STATUS,
		                mem_get_gpu_reg(&gpu->nes->mem, MEM_REG_GPU_STATUS)
		              | 0x80);
		if (mem_get_gpu_reg(&gpu->nes->mem, MEM_REG_GPU_CR1) & 0x80)
			gpu->nes->cpu.nmi = 1;
	}
	gpu->x++;
	if (gpu->x == 341)
	{
		gpu->x = 0;
		gpu->y++;
		if (gpu->y == 262)
		{
			gpu->y = 0;
			mem_set_gpu_reg(&gpu->nes->mem, MEM_REG_GPU_STATUS,
			                mem_get_gpu_reg(&gpu->nes->mem, MEM_REG_GPU_STATUS)
			              & ~0x60);
		}
	}
}

void gpu_clock(struct gpu *gpu)
{
	/* 5 in PAL, 4 in NTSC */
	if (++gpu->clock_count == 4)
	{
		gpu_cycle(gpu);
		gpu->clock_count = 0;
	}
}
