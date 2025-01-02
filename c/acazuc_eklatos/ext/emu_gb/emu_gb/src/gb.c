#include "gb.h"
#include "mbc.h"
#include "mem.h"
#include "cpu.h"
#include "gpu.h"
#include "apu.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct gb *gb_new(const void *mbc_data, size_t mbc_size)
{
	struct gb *gb = calloc(sizeof(*gb), 1);
	if (gb == NULL)
	{
		fprintf(stderr, "allocation failed\n");
		return NULL;
	}

	gb->mbc = mbc_new(mbc_data, mbc_size);
	if (!gb->mbc)
	{
		fprintf(stderr, "can't create mbc\n");
		return NULL;
	}

	gb->mem = mem_new(gb, gb->mbc);
	if (!gb->mem)
	{
		fprintf(stderr, "can't create memory\n");
		return NULL;
	}

	gb->cpu = cpu_new(gb->mem);
	if (!gb->cpu)
	{
		fprintf(stderr, "can't create cpu\n");
		return NULL;
	}

	gb->gpu = gpu_new(gb->mem);
	if (!gb->gpu)
	{
		fprintf(stderr, "can't create gpu\n");
		return NULL;
	}

	gb->apu = apu_new(gb->mem);
	if (!gb->apu)
	{
		fprintf(stderr, "can't create apu\n");
		return NULL;
	}

	return gb;
}

void gb_del(struct gb *gb)
{
	if (!gb)
		return;

	apu_del(gb->apu);
	gpu_del(gb->gpu);
	cpu_del(gb->cpu);
	mem_del(gb->mem);
	mbc_del(gb->mbc);
	free(gb);
}

static void timer_clock(struct gb *gb)
{
	if (gb->timerint && !--gb->timerint)
	{
		mem_set_reg(gb->mem, MEM_REG_TIMA, mem_get_reg(gb->mem, MEM_REG_TMA));
		mem_set_reg(gb->mem, MEM_REG_IF, mem_get_reg(gb->mem, MEM_REG_IF) | (1 << 2));
		gb->timerint = false;
	}

	if (gb->cpu->state != CPU_STOP)
		gb->mem->timer++;
	uint8_t tac = mem_get_reg(gb->mem, MEM_REG_TAC);
	uint16_t tmp;
	if (tac & (1 << 2))
	{
		static const uint16_t masks[] = {1 << 9, 1 << 3, 1 << 5, 1 << 7};
		tmp = gb->mem->timer & masks[tac & 0x3];
	}
	else
	{
		tmp = 0;
	}

	if (!tmp && gb->lasttimer)
	{
		uint8_t tima = mem_get_reg(gb->mem, MEM_REG_TIMA);
		if (tima == 0xFF)
			gb->timerint = 0x4;
		mem_set_reg(gb->mem, MEM_REG_TIMA, tima + 1);
	}

	gb->lasttimer = tmp;
}

void gb_frame(struct gb *gb, uint8_t *video_buf, int16_t *audio_buf, uint32_t joypad)
{
	gb->frame++;

	if (gb->mem->joyp != joypad)
	{
		gb->mem->joyp = joypad;
		mem_set_reg(gb->mem, MEM_REG_IF, mem_get_reg(gb->mem, MEM_REG_IF) | (1 << 4));
	}

	for (size_t i = 0; i < 154 * 456; ++i)
	{
		if (gb->mem->doublespeed)
			timer_clock(gb);
		timer_clock(gb);

		if (gb->mem->doublespeed)
			cpu_clock(gb->cpu);
		cpu_clock(gb->cpu);

		apu_clock(gb->apu);
		gpu_clock(gb->gpu);
	}

	memcpy(video_buf, gb->gpu->data, 160 * 144 * 4);
	memcpy(audio_buf, gb->apu->data, sizeof(gb->apu->data));
}

void gb_set_dmg_bios(struct gb *gb, const uint8_t *data)
{
	memcpy(gb->mem->dmg_bios, data, 0x100);
}

void gb_set_cgb_bios(struct gb *gb, const uint8_t *data)
{
	memcpy(gb->mem->cgb_bios, data, 0x900);
}

void gb_get_mbc_ram(struct gb *gb, uint8_t **data, size_t *size)
{
	if (gb->mbc->rambanksnb)
	{
		*data = gb->mbc->rambanks;
		*size = 0x2000 * gb->mbc->rambanksnb;
		return;
	}

	*data = NULL;
	*size = 0;
}

void gb_get_mbc_rtc(struct gb *gb, uint8_t **data, size_t *size)
{
	if (gb->mbc->options & MBC_OPT_BATTERY)
	{
		*data = (uint8_t*)&gb->mbc->rtc;
		*size = sizeof(gb->mbc->rtc);
		return;
	}
	*data = NULL;
	*size = 0;
}
