#ifndef NDS_H
#define NDS_H

#include <stddef.h>
#include <stdint.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef ENABLE_MULTITHREAD
# include <pthread.h>
#endif

struct mbc;
struct mem;
struct apu;
struct cpu;
struct gpu;

enum nds_button
{
	NDS_BUTTON_RIGHT  = (1 << 0),
	NDS_BUTTON_LEFT   = (1 << 1),
	NDS_BUTTON_UP     = (1 << 2),
	NDS_BUTTON_DOWN   = (1 << 3),
	NDS_BUTTON_A      = (1 << 4),
	NDS_BUTTON_B      = (1 << 5),
	NDS_BUTTON_X      = (1 << 6),
	NDS_BUTTON_Y      = (1 << 7),
	NDS_BUTTON_L      = (1 << 8),
	NDS_BUTTON_R      = (1 << 9),
	NDS_BUTTON_SELECT = (1 << 10),
	NDS_BUTTON_START  = (1 << 11),
};

typedef struct nds
{
	struct mbc *mbc;
	struct mem *mem;
	struct apu *apu;
	struct cpu *arm7;
	struct cpu *arm9;
	struct gpu *gpu;
	uint32_t joypad;
	uint64_t cycle;
	uint8_t touch_x;
	uint8_t touch_y;
	uint8_t touch;
#ifdef ENABLE_MULTITHREAD
	pthread_t gpu_thread;
	pthread_cond_t gpu_cond;
	pthread_mutex_t gpu_mutex;
	int gpu_y;
	int nds_y;
	int gpu_g3d;
	int nds_g3d;
#endif
} nds_t;

nds_t *nds_new(const void *rom_data, size_t rom_size);
void nds_del(nds_t *nds);

void nds_frame(struct nds *nds, uint8_t *video_top_buf, uint32_t video_top_pitch,
               uint8_t *video_bot_buf, uint32_t video_bot_pitch, int16_t *audio_buf,
               uint32_t joypad, uint8_t touch_x, uint8_t touch_y, uint8_t touch);

void nds_set_arm7_bios(nds_t *nds, const uint8_t *data);
void nds_set_arm9_bios(nds_t *nds, const uint8_t *data);
void nds_set_firmware(nds_t *nds, const uint8_t *data);

void nds_get_mbc_ram(nds_t *nds, uint8_t **data, size_t *size);
void nds_get_mbc_rtc(nds_t *nds, uint8_t **data, size_t *size);

void nds_test_keypad_int(nds_t *nds);

#endif
