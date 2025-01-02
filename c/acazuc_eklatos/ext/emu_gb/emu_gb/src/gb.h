#ifndef GB_H
#define GB_H

#include <stddef.h>
#include <stdint.h>

struct mbc;
struct mem;
struct cpu;
struct gpu;
struct apu;

enum gb_button
{
	GB_BUTTON_RIGHT  = (1 << 0),
	GB_BUTTON_LEFT   = (1 << 1),
	GB_BUTTON_UP     = (1 << 2),
	GB_BUTTON_DOWN   = (1 << 3),
	GB_BUTTON_A      = (1 << 4),
	GB_BUTTON_B      = (1 << 5),
	GB_BUTTON_SELECT = (1 << 6),
	GB_BUTTON_START  = (1 << 7),
};

typedef struct gb
{
	struct mbc *mbc;
	struct mem *mem;
	struct cpu *cpu;
	struct gpu *gpu;
	struct apu *apu;
	uint16_t lasttimer;
	uint8_t timerint;
	uint32_t frame;
} gb_t;

gb_t *gb_new(const void *rom_data, size_t rom_size);
void gb_del(gb_t *gb);

void gb_frame(gb_t *gb, uint8_t *video_buf, int16_t *audio_buf, uint32_t joypad);

void gb_set_dmg_bios(gb_t *gb, const uint8_t *data); /* 0x100 bytes */
void gb_set_cgb_bios(gb_t *gb, const uint8_t *data); /* 0x900 bytes */

void gb_get_mbc_ram(gb_t *gb, uint8_t **data, size_t *size);
void gb_get_mbc_rtc(gb_t *gb, uint8_t **data, size_t *size);

#endif
