#include "internal.h"
#include <stdlib.h>
#include <string.h>

struct nes *nes_new(const void *rom_data, size_t rom_size)
{
	struct nes *nes = calloc(sizeof(*nes), 1);
	if (!nes)
		return NULL;

	if (!mbc_init(&nes->mbc, nes, rom_data, rom_size))
		return NULL;

	mem_init(&nes->mem, nes);
	apu_init(&nes->apu, nes);
	cpu_init(&nes->cpu, nes);
	gpu_init(&nes->gpu, nes);
	return nes;
}

void nes_del(struct nes *nes)
{
	if (!nes)
		return;
	mbc_destroy(&nes->mbc);
	free(nes);
}

void nes_frame(struct nes *nes, uint8_t *video_buf, uint8_t *audio_buf, uint32_t joypad)
{
	nes->mem.joypad_state = joypad;
	nes->apu.sample_count = 0;
	nes->apu.sample_clock = 0;
	for (size_t i = 0 ; i < 357368; ++i) /* 532034 in PAL */
	{
		cpu_clock(&nes->cpu);
		gpu_clock(&nes->gpu);
		apu_clock(&nes->apu);
	}
	memcpy(video_buf, nes->gpu.data, 256 * 240 * 4);
	memcpy(audio_buf, nes->apu.data, 800);
}

void nes_get_mbc_ram(nes_t *nes, uint8_t **data, size_t *size)
{
	if (!(nes->mbc.ines->flags6 & (1 << 1)))
	{
		*data = NULL;
		*size = 0;
		return;
	}
	*data = nes->mbc.prg_ram_data;
	*size = nes->mbc.prg_ram_size;
}

void nes_get_mbc_rtc(nes_t *nes, uint8_t **data, size_t *size)
{
	(void)nes;
	*data = NULL;
	*size = 0;
}
