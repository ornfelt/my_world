#include "gpu.h"
#include "mem.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define TO8(v) (((uint32_t)(v) * 527 + 23) >> 6)

struct gpu *gpu_new(struct mem *mem)
{
	struct gpu *gpu = calloc(sizeof(*gpu), 1);
	if (!gpu)
	{
		fprintf(stderr, "allocation failed\n");
		return NULL;
	}

	gpu->mem = mem;
	return gpu;
}

void gpu_del(struct gpu *gpu)
{
	if (!gpu)
		return;
	free(gpu);
}

static void render_tile_dmg(struct gpu *gpu, uint16_t addr,
                            uint8_t x, uint8_t y,
                            uint8_t bx, uint8_t by)
{
	uint8_t coloridx = (((mem_get_vram0(gpu->mem, addr + by * 2 + 0) >> (7 - bx)) & 1) << 0)
	                 | (((mem_get_vram0(gpu->mem, addr + by * 2 + 1) >> (7 - bx)) & 1) << 1);
	static const uint8_t values[4] = {0xFF, 0xAA, 0x55, 0x00};
	uint8_t color = values[(mem_get_reg(gpu->mem, MEM_REG_BGP) >> (coloridx << 1)) & 0x3];
	memset(&gpu->data[(y * 160 + x) * 4], color, 4);
	gpu->priorities[x] = 0;
	gpu->hasprinted[x] = coloridx != 0;
}

static void render_tile_cgb(struct gpu *gpu, uint16_t addr,
                            uint8_t x, uint8_t y,
                            uint8_t bx, uint8_t by, uint8_t attr)
{
	uint8_t palette = gpu->mem->cgb == CGB_YES ? (attr & 0x7) : 0;
	uint8_t coloridx;
	if (attr & (1 << 5))
		bx = 7 - bx;
	if (attr & (1 << 6))
		by = 7 - by;
	if (attr & (1 << 3))
	{
		coloridx = (((mem_get_vram1(gpu->mem, addr + by * 2 + 0) >> (7 - bx)) & 1) << 0)
	             | (((mem_get_vram1(gpu->mem, addr + by * 2 + 1) >> (7 - bx)) & 1) << 1);
	}
	else
	{
		coloridx = (((mem_get_vram0(gpu->mem, addr + by * 2 + 0) >> (7 - bx)) & 1) << 0)
	             | (((mem_get_vram0(gpu->mem, addr + by * 2 + 1) >> (7 - bx)) & 1) << 1);
	}
	if (gpu->mem->cgb == CGB_FORCE)
		coloridx = (mem_get_reg(gpu->mem, MEM_REG_BGP) >> (coloridx << 1)) & 0x3;
	uint8_t *palptr = &gpu->mem->bgpalette[palette * 8 + coloridx * 2];
	uint8_t color[4] =
	{
		TO8((palptr[1] & 0x7C) >> 2),
		TO8((palptr[0] & 0xE0 >> 5) | ((palptr[1] & 3) << 3)),
		TO8(palptr[0] & 0x1F),
		0xFF,
	};
	memcpy(&gpu->data[(y * 160 + x) * 4], color, 4);
	gpu->priorities[x] = (attr >> 7);
	gpu->hasprinted[x] = coloridx != 0;
}

static void render_background(struct gpu *gpu, uint8_t x, uint8_t y)
{
	uint8_t haddr = mem_get_reg(gpu->mem, MEM_REG_LCDC) & (1 << 4);
	uint16_t baseaddr = (mem_get_reg(gpu->mem, MEM_REG_LCDC) & (1 << 3)) ? 0x9C00 : 0x9800;
	uint16_t charaddr = haddr ? 0x8000 : 0x9000;
	uint8_t scx = mem_get_reg(gpu->mem, MEM_REG_SCX);
	uint8_t scy = mem_get_reg(gpu->mem, MEM_REG_SCY);
	uint8_t ry = scy + y;
	uint8_t by = ry % 8;
	uint8_t rx = scx + x;
	uint8_t bx = rx % 8;
	uint16_t caddr = baseaddr + rx / 8 + ry / 8 * 32;
	uint8_t charcode = mem_get_vram0(gpu->mem, caddr);
	uint16_t addr = charaddr + (haddr ? charcode * 16 : (int8_t)charcode * 16);
	if (gpu->mem->cgb != CGB_NO)
		render_tile_cgb(gpu, addr, x, y, bx, by, mem_get_vram1(gpu->mem, caddr));
	else
		render_tile_dmg(gpu, addr, x, y, bx, by);
}

static void render_window(struct gpu *gpu, uint8_t x, uint8_t y)
{
	uint8_t wx = mem_get_reg(gpu->mem, MEM_REG_WX);
	uint8_t wy = mem_get_reg(gpu->mem, MEM_REG_WY);
	if (wy > y || wx > x + 7)
		return;
	uint8_t haddr = mem_get_reg(gpu->mem, MEM_REG_LCDC) & (1 << 4);
	uint16_t baseaddr = (mem_get_reg(gpu->mem, MEM_REG_LCDC) & (1 << 6)) ? 0x9C00 : 0x9800;
	uint16_t charaddr = haddr ? 0x8000 : 0x9000;
	uint8_t rx = x - wx + 7;
	uint8_t ry = y - wy;
	uint8_t bx = rx % 8;
	uint8_t by = ry % 8;
	uint16_t caddr = baseaddr + ((rx - bx) + (gpu->windowlines - gpu->windowlines % 8) * 32) / 8;
	uint16_t charcode = mem_get_vram0(gpu->mem, caddr);
	uint16_t addr = charaddr + (haddr ? charcode * 16 : (int8_t)charcode * 16);
	if (gpu->mem->cgb != CGB_NO)
		render_tile_cgb(gpu, addr, x, y, bx, by, mem_get_vram1(gpu->mem, caddr));
	else
		render_tile_dmg(gpu, addr, x, y, bx, by);
}

static void render_object_dmg(struct gpu *gpu, uint8_t x,
                              uint8_t bx, uint8_t by, uint8_t charcode,
                              uint8_t attr, bool height16,
                              int8_t *lowestx, uint8_t *color)
{
	int8_t orgbx = bx;
	bool palette = (attr >> 4) & 1;
	bool prio = (attr >> 7) & 1;
	if (attr & (1 << 5))
		bx = 7 - bx;
	if (attr & (1 << 6))
		by = (height16 ? 15 : 7) - by;
	if (height16)
		charcode &= ~1;
	uint16_t charaddr = 0x8000 + charcode * 16;
	if (orgbx <= *lowestx)
		return;
	*lowestx = orgbx;
	uint8_t pixel = (((mem_get_vram0(gpu->mem, charaddr + by * 2 + 0) >> (7 - bx)) & 1) << 0)
	              | (((mem_get_vram0(gpu->mem, charaddr + by * 2 + 1) >> (7 - bx)) & 1) << 1);
	if (!pixel)
	{
		*color = 0xFF;
		return;
	}
	if ((gpu->priorities[x] || prio) && gpu->hasprinted[x])
	{
		*color = 0xFF;
		return;
	}
	*color = (mem_get_reg(gpu->mem, palette ? MEM_REG_OBP1 : MEM_REG_OBP0) >> (pixel << 1)) & 3;
}

static bool render_object_cgb(struct gpu *gpu, uint8_t x, uint8_t y,
                              uint8_t bx, uint8_t by, uint8_t charcode,
                              uint8_t attr, bool height16, int8_t *lowestx)
{
	int8_t orgbx = bx;
	uint8_t palette = (gpu->mem->cgb == CGB_YES) ? (attr & 0x7) : ((attr >> 4) & 1);
	bool prio = (attr >> 7) & 1;
	if (attr & (1 << 5))
		bx = 7 - bx;
	if (attr & (1 << 6))
		by = (height16 ? 15 : 7) - by;
	if (height16)
		charcode &= ~1;
	uint16_t charaddr = 0x8000 + charcode * 16;
	if ((mem_get_reg(gpu->mem, MEM_REG_OPRI) & (1 << 0)))
	{
		if (orgbx <= *lowestx)
			return false;
		*lowestx = orgbx;
	}
	uint8_t coloridx;
	if (attr & (1 << 3))
	{
		coloridx = (((mem_get_vram1(gpu->mem, charaddr + by * 2 + 0) >> (7 - bx)) & 1) << 0)
		         | (((mem_get_vram1(gpu->mem, charaddr + by * 2 + 1) >> (7 - bx)) & 1) << 1);
	}
	else
	{
		coloridx = (((mem_get_vram0(gpu->mem, charaddr + by * 2 + 0) >> (7 - bx)) & 1) << 0)
		         | (((mem_get_vram0(gpu->mem, charaddr + by * 2 + 1) >> (7 - bx)) & 1) << 1);
	}
	if (!coloridx)
		return false;
	if (!(mem_get_reg(gpu->mem, MEM_REG_OPRI) & (1 << 0)))
	{
		if ((mem_get_reg(gpu->mem, MEM_REG_LCDC) & (1 << 0)) && (gpu->priorities[x] || prio) && gpu->hasprinted[x])
			return false;
	}
	else
	{
		if ((gpu->priorities[x] || prio) && gpu->hasprinted[x])
			return false;
	}
	if (gpu->mem->cgb == CGB_FORCE)
		coloridx = (mem_get_reg(gpu->mem, palette ? MEM_REG_OBP1 : MEM_REG_OBP0) >> (coloridx << 1)) & 3;
	uint8_t *palptr = &gpu->mem->objpalette[palette * 8 + coloridx * 2];
	uint8_t color[4] =
	{
		TO8((palptr[1] & 0x7C) >> 2),
		TO8((palptr[0] & 0xE0 >> 5) | ((palptr[1] & 3) << 3)),
		TO8(palptr[0] & 0x1F),
		0xFF,
	};
	memcpy(&gpu->data[(y * 160 + x) * 4], color, 4);
	return true;
}

static void render_objects(struct gpu *gpu, uint8_t x, uint8_t y)
{
	uint8_t color = 0xFF;
	int8_t lowestx = -1;
	bool height16 = mem_get_reg(gpu->mem, MEM_REG_LCDC) & (1 << 2);
	for (uint8_t i = 0; i < gpu->sprites_count; ++i)
	{
		uint16_t addr = 0xFE00 + 4 * gpu->sprites[i];
		uint8_t cx = mem_get_oam(gpu->mem, addr + 1);
		if (!cx || cx >= 168 || cx > x + 8 || cx <= x)
			continue;
		uint8_t cy = mem_get_oam(gpu->mem, addr + 0);
		uint8_t charcode = mem_get_oam(gpu->mem, addr + 2);
		uint8_t cattr = mem_get_oam(gpu->mem, addr + 3);
		if (gpu->mem->cgb != CGB_NO)
		{
			if (render_object_cgb(gpu, x, y, x - cx + 8, y - cy + 16, charcode, cattr, height16, &lowestx))
				return;
		}
		else
		{
			render_object_dmg(gpu, x, x - cx + 8, y - cy + 16, charcode, cattr, height16, &lowestx, &color);
		}
	}
	if (gpu->mem->cgb == CGB_NO)
	{
		if (color != 0xFF)
		{
			static const uint8_t values[4] = {0xFF, 0xAA, 0x55, 0x00};
			memset(&gpu->data[(y * 160 + x) * 4], values[color], 4);
		}
	}
}

static void render_pixel(struct gpu *gpu, uint8_t x, uint8_t y)
{
	if (!(mem_get_reg(gpu->mem, MEM_REG_LCDC) & (1 << 7)))
	{
		memset(&gpu->data[(160 * y + x) * 4], 0xFF, 4);
		return;
	}

	if (gpu->mem->cgb == CGB_YES || mem_get_reg(gpu->mem, MEM_REG_LCDC) & (1 << 0))
	{
		render_background(gpu, x, y);
		if (mem_get_reg(gpu->mem, MEM_REG_LCDC) & (1 << 5))
			render_window(gpu, x, y);
	}

	if (gpu->mem->dmatransfer == 0xFF && mem_get_reg(gpu->mem, MEM_REG_LCDC) & (1 << 1))
		render_objects(gpu, x, y);
}

static void collect_sprites(struct gpu *gpu, uint8_t y)
{
	gpu->sprites_count = 0;
	bool height16 = mem_get_reg(gpu->mem, MEM_REG_LCDC) & (1 << 2);
	for (uint8_t i = 0; i < 40; ++i)
	{
		uint16_t addr = 0xFE00 + 4 * i;
		uint8_t cy = mem_get_oam(gpu->mem, addr + 0);
		if (!cy || cy >= 160 || cy > y + 16 || cy + (height16 ? 16 : 8) <= y + 16)
			continue;
		gpu->sprites[gpu->sprites_count++] = i;
		if (gpu->sprites_count == 10)
			return;
	}
}

void gpu_clock(struct gpu *gpu)
{
	if (gpu->y < 144)
	{
		if (!gpu->line_cycle)
		{
			collect_sprites(gpu, gpu->y);
			if (mem_get_reg(gpu->mem, MEM_REG_LCDC) & (1 << 7))
			{
				uint8_t lyc = mem_get_reg(gpu->mem, MEM_REG_LYC);
				mem_set_reg(gpu->mem, MEM_REG_LY, gpu->y);
				if ((mem_get_reg(gpu->mem, MEM_REG_STAT) & (1 << 6)) && lyc == gpu->y)
					mem_set_reg(gpu->mem, MEM_REG_IF, mem_get_reg(gpu->mem, MEM_REG_IF) | (1 << 1));

				if (lyc == gpu->y)
					mem_set_reg(gpu->mem, MEM_REG_STAT, mem_get_reg(gpu->mem, MEM_REG_STAT) | (1 << 2));
				else
					mem_set_reg(gpu->mem, MEM_REG_STAT, mem_get_reg(gpu->mem, MEM_REG_STAT) & ~(1 << 2));

				/* mode 2: oam */
				mem_set_reg(gpu->mem, MEM_REG_STAT, (mem_get_reg(gpu->mem, MEM_REG_STAT) & (~0x3)) | 2);
				if (mem_get_reg(gpu->mem, MEM_REG_STAT) & (1 << 5))
					mem_set_reg(gpu->mem, MEM_REG_IF, mem_get_reg(gpu->mem, MEM_REG_IF) | (1 << 1));
			}
		}
		else if (gpu->line_cycle == 80)
		{
			if (mem_get_reg(gpu->mem, MEM_REG_LCDC) & (1 << 7))
			{
				/* mode 3: lcd data */
				mem_set_reg(gpu->mem, MEM_REG_STAT, (mem_get_reg(gpu->mem, MEM_REG_STAT) & (~0x3)) | 3);
			}
		}
		else if (gpu->line_cycle >= 91 && gpu->line_cycle < 252)
		{
			if (gpu->line_cycle == 91)
			{
				gpu->x = 0;
				gpu->x -= mem_get_reg(gpu->mem, MEM_REG_SCX) % 8;
			}
			if (gpu->x < 160)
				render_pixel(gpu, gpu->x, gpu->y);
			gpu->x++;
		}
		else if (gpu->line_cycle == 252)
		{
			if (mem_get_reg(gpu->mem, MEM_REG_WY) <= gpu->y && mem_get_reg(gpu->mem, MEM_REG_WX) < 166)
				gpu->windowlines++;
			if (mem_get_reg(gpu->mem, MEM_REG_LCDC) & (1 << 7))
			{
				/* mode 0: hblank */
				mem_hdmatransfer(gpu->mem);
				mem_set_reg(gpu->mem, MEM_REG_STAT, (mem_get_reg(gpu->mem, MEM_REG_STAT) & (~0x3)) | 0);
				if (mem_get_reg(gpu->mem, MEM_REG_STAT) & (1 << 3))
					mem_set_reg(gpu->mem, MEM_REG_IF, mem_get_reg(gpu->mem, MEM_REG_IF) | (1 << 1));
			}
		}
	}
	else
	{
		if (!gpu->line_cycle)
		{
			if (gpu->y == 144)
			{
				if (mem_get_reg(gpu->mem, MEM_REG_LCDC) & (1 << 7))
				{
					/* mode 1: vblank */
					mem_set_reg(gpu->mem, MEM_REG_STAT, (mem_get_reg(gpu->mem, MEM_REG_STAT) & (~0x3)) | 1);
					if (mem_get_reg(gpu->mem, MEM_REG_STAT) & (3 << 4)) /* OAM and Vblank are triggered */
						mem_set_reg(gpu->mem, MEM_REG_IF, mem_get_reg(gpu->mem, MEM_REG_IF) | (1 << 1));
					mem_set_reg(gpu->mem, MEM_REG_IF, mem_get_reg(gpu->mem, MEM_REG_IF) | (1 << 0));
				}
			}
			if (mem_get_reg(gpu->mem, MEM_REG_LCDC) & (1 << 7))
			{
				uint8_t lyc = mem_get_reg(gpu->mem, MEM_REG_LYC);
				mem_set_reg(gpu->mem, MEM_REG_LY, /*gpu->y != 153 ?*/ gpu->y/* : 0*/);

				if ((mem_get_reg(gpu->mem, MEM_REG_STAT) & (1 << 6)) && lyc == gpu->y)
					mem_set_reg(gpu->mem, MEM_REG_IF, mem_get_reg(gpu->mem, MEM_REG_IF) | (1 << 1));

				if (lyc == gpu->y)
					mem_set_reg(gpu->mem, MEM_REG_STAT, mem_get_reg(gpu->mem, MEM_REG_STAT) | (1 << 2));
				else
					mem_set_reg(gpu->mem, MEM_REG_STAT, mem_get_reg(gpu->mem, MEM_REG_STAT) & ~(1 << 2));
			}
		}
	}

	gpu->line_cycle++;
	if (gpu->line_cycle == 456)
	{
		gpu->y = (gpu->y + 1) % 154;
		gpu->line_cycle = 0;
		if (!gpu->y)
		{
			memset(gpu->priorities, 0, sizeof(gpu->priorities));
			memset(gpu->hasprinted, 0, sizeof(gpu->hasprinted));
			gpu->windowlines = 0;
		}
	}
}
