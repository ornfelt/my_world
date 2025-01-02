#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "libretro.h"
#include "../gba.h"

#define VIDEO_WIDTH 240
#define VIDEO_HEIGHT 160
#define VIDEO_PIXELS VIDEO_WIDTH * VIDEO_HEIGHT

#define VIDEO_FPS (59.72750056960583276373)
#define AUDIO_FPS (48000)

#define AUDIO_FRAME (804) /* ceil(AUDIO_FPS / VIDEO_FPS) */

static struct retro_log_callback logging;
static retro_log_printf_t log_cb;
static char system_dir[256] = ".";

static gba_t *g_gba = NULL;

static void fallback_log(enum retro_log_level level, const char *fmt, ...)
{
	(void)level;
	va_list va;
	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	va_end(va);
}

static retro_environment_t environ_cb;

void retro_init(void)
{
	const char *dir;
	if (environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &dir) && dir)
		snprintf(system_dir, sizeof(system_dir), "%s", dir);
}

void retro_deinit(void)
{
}

unsigned retro_api_version(void)
{
	return RETRO_API_VERSION;
}

void retro_set_controller_port_device(unsigned port, unsigned device)
{
	(void)port;
	(void)device;
}

void retro_get_system_info(struct retro_system_info *info)
{
	memset(info, 0, sizeof(*info));
	info->library_name     = "emu_gba";
	info->library_version  = "0.1";
	info->need_fullpath    = false;
	info->valid_extensions = "gba";
}

static retro_video_refresh_t video_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;

void retro_get_system_av_info(struct retro_system_av_info *info)
{
	memset(info, 0, sizeof(*info));
	info->timing.fps            = VIDEO_FPS;
	info->timing.sample_rate    = AUDIO_FPS;
	info->geometry.base_width   = VIDEO_WIDTH;
	info->geometry.base_height  = VIDEO_HEIGHT;
	info->geometry.max_width    = VIDEO_WIDTH;
	info->geometry.max_height   = VIDEO_HEIGHT;
	info->geometry.aspect_ratio = VIDEO_WIDTH / (float)VIDEO_HEIGHT;
}

void retro_set_environment(retro_environment_t cb)
{
	environ_cb = cb;

	if (cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logging))
		log_cb = logging.log;
	else
		log_cb = fallback_log;

	static const struct retro_controller_description controllers[] =
	{
		{"Controller", RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_JOYPAD, 0)},
	};

	static const struct retro_controller_info ports[] =
	{
		{controllers, 1},
		{NULL, 0},
	};

	cb(RETRO_ENVIRONMENT_SET_CONTROLLER_INFO, (void*)ports);
}

void retro_set_audio_sample(retro_audio_sample_t cb)
{
	audio_cb = cb;
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
	audio_batch_cb = cb;
}

void retro_set_input_poll(retro_input_poll_t cb)
{
	input_poll_cb = cb;
}

void retro_set_input_state(retro_input_state_t cb)
{
	input_state_cb = cb;
}

void retro_set_video_refresh(retro_video_refresh_t cb)
{
	video_cb = cb;
}

void retro_reset(void)
{
}

static uint8_t video_buf[VIDEO_WIDTH * VIDEO_HEIGHT * 4];
static int16_t audio_buf[AUDIO_FRAME * 2];

void retro_run(void)
{
	int16_t tmp_audio[804];
	uint32_t joypad = 0;

	input_poll_cb();

	joypad |= GBA_BUTTON_LEFT   * (!!input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT));
	joypad |= GBA_BUTTON_RIGHT  * (!!input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT));
	joypad |= GBA_BUTTON_UP     * (!!input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP));
	joypad |= GBA_BUTTON_DOWN   * (!!input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN));
	joypad |= GBA_BUTTON_A      * (!!input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A));
	joypad |= GBA_BUTTON_B      * (!!input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B));
	joypad |= GBA_BUTTON_L      * (!!input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L));
	joypad |= GBA_BUTTON_R      * (!!input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R));
	joypad |= GBA_BUTTON_START  * (!!input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START));
	joypad |= GBA_BUTTON_SELECT * (!!input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT));

	gba_frame(g_gba, video_buf, tmp_audio, joypad);

	video_cb(video_buf, VIDEO_WIDTH, VIDEO_HEIGHT, VIDEO_WIDTH * 4);

	for (size_t i = 0; i < AUDIO_FRAME; ++i)
	{
		uint16_t dst = i * 804 / AUDIO_FRAME;
		uint16_t sample = tmp_audio[dst];
		uint8_t l = sample >> 8;
		uint8_t r = sample >> 0;
		audio_buf[i * 2 + 0] = (l >> 1) | (l << 7);
		audio_buf[i * 2 + 1] = (r >> 1) | (r << 7);
	}

	audio_batch_cb(audio_buf, AUDIO_FRAME);
}

static bool load_bios(const char *name, uint8_t *data, size_t size)
{
	char path[512];
	snprintf(path, sizeof(path), "%s/%s", system_dir, name);
	FILE *fp = fopen(path, "r");
	if (!fp)
	{
		char error[4096];
		snprintf(error, sizeof(error), "failed to open file '%s'\n", path);
		log_cb(RETRO_LOG_ERROR, error);
		return false;
	}
	if (fread(data, 1, size, fp) != size)
	{
		char error[4096];
		snprintf(error, sizeof(error), "failed to read %u bytes from '%s'\n", (unsigned)size, path);
		log_cb(RETRO_LOG_ERROR, error);
		fclose(fp);
		return false;
	}
	fclose(fp);
	return true;
}

bool retro_load_game(const struct retro_game_info *info)
{
	struct retro_input_descriptor desc[] =
	{
		{0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,   "Left"  },
		{0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,     "Up"    },
		{0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,   "Down"  },
		{0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT,  "Right" },
		{0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,      "A"     },
		{0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,      "B"     },
		{0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L,      "L"     },
		{0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R,      "R"     },
		{0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Select"},
		{0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,  "Start" },
		{0},
	};

	environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc);

	enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;
	if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
	{
		log_cb(RETRO_LOG_ERROR, "XRGB8888 is not supported.\n");
		goto err;
	}

	if (info->data == NULL || info->size == 0)
	{
		log_cb(RETRO_LOG_ERROR, "empty rom\n");
		goto err;
	}

	gba_del(g_gba);
	g_gba = gba_new(info->data, info->size);
	if (!g_gba)
	{
		log_cb(RETRO_LOG_ERROR, "can't create gba\n");
		goto err;
	}

	uint8_t gba_bios[0x4000];
	if (!load_bios("gba_bios.bin", gba_bios, sizeof(gba_bios)))
	{
		log_cb(RETRO_LOG_ERROR, "failed to load gba_bios.bin\n");
		goto err;
	}
	gba_set_bios(g_gba, gba_bios);
	return true;

err:
	gba_del(g_gba);
	g_gba = NULL;
	return false;
}

void retro_unload_game(void)
{
	gba_del(g_gba);
	g_gba = NULL;
}

unsigned retro_get_region(void)
{
	return RETRO_REGION_NTSC;
}

bool retro_load_game_special(unsigned type, const struct retro_game_info *info, size_t num)
{
	(void)type;
	(void)info;
	(void)num;
	return false;
}

size_t retro_serialize_size(void)
{
	return 0;
}

bool retro_serialize(void *data, size_t size)
{
	(void)data;
	(void)size;
	return false;
}

bool retro_unserialize(const void *data, size_t size)
{
	(void)data;
	(void)size;
	return false;
}

void *retro_get_memory_data(unsigned id)
{
	uint8_t *data;
	size_t size;
	switch (id)
	{
		case RETRO_MEMORY_SAVE_RAM:
			gba_get_mbc_ram(g_gba, &data, &size);
			return data;
		case RETRO_MEMORY_RTC:
			gba_get_mbc_rtc(g_gba, &data, &size);
			return data;
	}
	return NULL;
}

size_t retro_get_memory_size(unsigned id)
{
	uint8_t *data;
	size_t size;
	switch (id)
	{
		case RETRO_MEMORY_SAVE_RAM:
			gba_get_mbc_ram(g_gba, &data, &size);
			return size;
		case RETRO_MEMORY_RTC:
			gba_get_mbc_rtc(g_gba, &data, &size);
			return size;
	}
	return 0;
}

void retro_cheat_reset(void)
{
}

void retro_cheat_set(unsigned index, bool enabled, const char *code)
{
	(void)index;
	(void)enabled;
	(void)code;
}
