#ifndef NES_H
#define NES_H

#include <stddef.h>
#include <stdint.h>

typedef struct nes nes_t;

enum nes_button
{
	NES_BUTTON_RIGHT  = (1 << 0),
	NES_BUTTON_LEFT   = (1 << 1),
	NES_BUTTON_UP     = (1 << 2),
	NES_BUTTON_DOWN   = (1 << 3),
	NES_BUTTON_A      = (1 << 4),
	NES_BUTTON_B      = (1 << 5),
	NES_BUTTON_SELECT = (1 << 6),
	NES_BUTTON_START  = (1 << 7),
};

nes_t *nes_new(const void *rom_data, size_t rom_size);
void nes_del(nes_t *nes);

void nes_frame(nes_t *nes, uint8_t *video_buf, uint8_t *audio_buf, uint32_t joypad);

void nes_get_mbc_ram(nes_t *nes, uint8_t **data, size_t *size);
void nes_get_mbc_rtc(nes_t *nes, uint8_t **data, size_t *size);

#endif
