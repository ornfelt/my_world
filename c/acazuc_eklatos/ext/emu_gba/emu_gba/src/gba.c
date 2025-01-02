#include "gba.h"
#include "mbc.h"
#include "mem.h"
#include "apu.h"
#include "cpu.h"
#include "gpu.h"

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

gba_t *gba_new(const void *rom_data, size_t rom_size)
{
	gba_t *gba = calloc(sizeof(*gba), 1);
	if (!gba)
		return NULL;

	gba->mbc = mbc_new(rom_data, rom_size);
	if (!gba->mbc)
		return NULL;

	gba->mem = mem_new(gba, gba->mbc);
	if (!gba->mem)
		return NULL;

	gba->apu = apu_new(gba->mem);
	if (!gba->apu)
		return NULL;

	gba->cpu = cpu_new(gba->mem);
	if (!gba->cpu)
		return NULL;

	gba->gpu = gpu_new(gba->mem);
	if (!gba->gpu)
		return NULL;

	return gba;
}

void gba_del(gba_t *gba)
{
	if (!gba)
		return;
	mbc_del(gba->mbc);
	mem_del(gba->mem);
	apu_del(gba->apu);
	cpu_del(gba->cpu);
	gpu_del(gba->gpu);
	free(gba);
}

static void gba_cycles(gba_t *gba, size_t cycles)
{
	for (; cycles; cycles--)
	{
		gba->cycle++;
		bool has_dma = mem_dma(gba->mem);
		mem_timers(gba->mem);
		if (!has_dma)
			cpu_cycle(gba->cpu);
		apu_cycle(gba->apu);
	}
}

void gba_frame(gba_t *gba, uint8_t *video_buf, int16_t *audio_buf, uint32_t joypad)
{
	gba->joypad = joypad;
	gba_test_keypad_int(gba);
	for (uint8_t y = 0; y < 160; ++y)
	{
		mem_set_reg16(gba->mem, MEM_REG_DISPSTAT, (mem_get_reg16(gba->mem, MEM_REG_DISPSTAT) & 0xFFFC) | 0x0);
		mem_set_reg16(gba->mem, MEM_REG_VCOUNT, y);

		if ((mem_get_reg16(gba->mem, MEM_REG_DISPSTAT) & (1 << 5)) && y == ((mem_get_reg16(gba->mem, MEM_REG_DISPSTAT) >> 8) & 0xFF))
			mem_set_reg16(gba->mem, MEM_REG_IF, mem_get_reg16(gba->mem, MEM_REG_IF) | (1 << 2));

		/* draw */
		gpu_draw(gba->gpu, y);
		gba_cycles(gba, 960);

		/* hblank */
		mem_set_reg16(gba->mem, MEM_REG_DISPSTAT, (mem_get_reg16(gba->mem, MEM_REG_DISPSTAT) & 0xFFFC) | 0x2);
		if (mem_get_reg16(gba->mem, MEM_REG_DISPSTAT) & (1 << 4))
			mem_set_reg16(gba->mem, MEM_REG_IF, mem_get_reg16(gba->mem, MEM_REG_IF) | (1 << 1));
		mem_hblank(gba->mem);

		gba_cycles(gba, 272);
	}

	gpu_commit_bgpos(gba->gpu);
	if (mem_get_reg16(gba->mem, MEM_REG_DISPSTAT) & (1 << 3))
		mem_set_reg16(gba->mem, MEM_REG_IF, mem_get_reg16(gba->mem, MEM_REG_IF) | (1 << 0));
	mem_vblank(gba->mem);

	for (uint8_t y = 160; y < 228; ++y)
	{
		uint8_t vflag = y != 227;
		mem_set_reg16(gba->mem, MEM_REG_DISPSTAT, (mem_get_reg16(gba->mem, MEM_REG_DISPSTAT) & 0xFFFC) | vflag);
		mem_set_reg16(gba->mem, MEM_REG_VCOUNT, y);

		if ((mem_get_reg16(gba->mem, MEM_REG_DISPSTAT) & (1 << 5)) && y == ((mem_get_reg16(gba->mem, MEM_REG_DISPSTAT) >> 8) & 0xFF))
			mem_set_reg16(gba->mem, MEM_REG_IF, mem_get_reg16(gba->mem, MEM_REG_IF) | (1 << 2));

		/* vblank */
		gba_cycles(gba, 960);

		/* hblank */
		mem_set_reg16(gba->mem, MEM_REG_DISPSTAT, (mem_get_reg16(gba->mem, MEM_REG_DISPSTAT) & 0xFFFC) | 0x2 | vflag);
		if (mem_get_reg16(gba->mem, MEM_REG_DISPSTAT) & (1 << 4))
			mem_set_reg16(gba->mem, MEM_REG_IF, mem_get_reg16(gba->mem, MEM_REG_IF) | (1 << 1));

		gba_cycles(gba, 272);
	}
	memcpy(video_buf, gba->gpu->data, sizeof(gba->gpu->data));
	memcpy(audio_buf, gba->apu->data, sizeof(gba->apu->data));
}

void gba_set_bios(gba_t *gba, const uint8_t *data)
{
	memcpy(gba->mem->bios, data, 0x4000);
}

void gba_get_mbc_ram(gba_t *gba, uint8_t **data, size_t *size)
{
	*data = &gba->mbc->backup[0];
	*size = sizeof(gba->mbc->backup);
}

void gba_get_mbc_rtc(gba_t *gba, uint8_t **data, size_t *size)
{
	if (gba->mbc->gpio_type == MBC_RTC)
	{
		*data = (uint8_t*)&gba->mbc->gpio.rtc.offset;
		*size = sizeof(gba->mbc->gpio.rtc.offset);
		return;
	}
	*data = NULL;
	*size = 0;
}

void gba_test_keypad_int(gba_t *gba)
{
	uint16_t keycnt = mem_get_reg16(gba->mem, MEM_REG_KEYCNT);
	if (!(keycnt & (1 << 14)))
		return;
	uint16_t keys = 0;
	if (gba->joypad & GBA_BUTTON_A)
		keys |= (1 << 0);
	if (gba->joypad & GBA_BUTTON_B)
		keys |= (1 << 1);
	if (gba->joypad & GBA_BUTTON_SELECT)
		keys |= (1 << 2);
	if (gba->joypad & GBA_BUTTON_START)
		keys |= (1 << 3);
	if (gba->joypad & GBA_BUTTON_RIGHT)
		keys |= (1 << 4);
	if (gba->joypad & GBA_BUTTON_LEFT)
		keys |= (1 << 5);
	if (gba->joypad & GBA_BUTTON_UP)
		keys |= (1 << 6);
	if (gba->joypad & GBA_BUTTON_DOWN)
		keys |= (1 << 7);
	if (gba->joypad & GBA_BUTTON_R)
		keys |= (1 << 8);
	if (gba->joypad & GBA_BUTTON_L)
		keys |= (1 << 9);
	bool enabled;
	if (keycnt & (1 << 15))
		enabled = (keys & (keycnt & 0x3FF)) == (keycnt & 0x3FF);
	else
		enabled = keys & keycnt;
	if (enabled)
		mem_set_reg16(gba->mem, MEM_REG_IF, mem_get_reg16(gba->mem, MEM_REG_IF) | (1 << 12));
}
