#include "gui/gui.h"

#include "textures.h"
#include "voxel.h"
#include "log.h"

#include <gfx/window.h>

#include <stdlib.h>

static const struct vec4f text_colors[16] =
{
	{0.000f, 0.000f, 0.000f, 1.0f},
	{0.000f, 0.000f, 0.666f, 1.0f},
	{0.000f, 0.666f, 0.000f, 1.0f},
	{0.000f, 0.666f, 0.666f, 1.0f},
	{0.666f, 0.000f, 0.000f, 1.0f},
	{0.666f, 0.000f, 0.666f, 1.0f},
	{1.000f, 0.666f, 0.000f, 1.0f},
	{0.666f, 0.666f, 0.666f, 1.0f},
	{0.333f, 0.333f, 0.333f, 1.0f},
	{0.333f, 0.333f, 1.000f, 1.0f},
	{0.333f, 1.000f, 0.333f, 1.0f},
	{0.333f, 1.000f, 1.000f, 1.0f},
	{1.000f, 0.333f, 0.333f, 1.0f},
	{1.000f, 0.333f, 1.000f, 1.0f},
	{1.000f, 1.000f, 0.333f, 1.0f},
	{1.000f, 1.000f, 1.000f, 1.0f},
};

static const struct vec4f text_shadow_colors[16] =
{
	{0.000f, 0.000f, 0.000f, 1.0f},
	{0.000f, 0.000f, 0.165f, 1.0f},
	{0.000f, 0.165f, 0.000f, 1.0f},
	{0.000f, 0.165f, 0.165f, 1.0f},
	{0.165f, 0.000f, 0.000f, 1.0f},
	{0.165f, 0.000f, 0.165f, 1.0f},
	{0.165f, 0.165f, 0.000f, 1.0f},
	{0.165f, 0.165f, 0.165f, 1.0f},
	{0.082f, 0.082f, 0.082f, 1.0f},
	{0.082f, 0.082f, 0.247f, 1.0f},
	{0.082f, 0.274f, 0.082f, 1.0f},
	{0.082f, 0.247f, 0.247f, 1.0f},
	{0.247f, 0.082f, 0.082f, 1.0f},
	{0.247f, 0.082f, 0.247f, 1.0f},
	{0.247f, 0.247f, 0.082f, 1.0f},
	{0.247f, 0.247f, 0.247f, 1.0f},

};

static bool has_pixel_alpha(uint32_t x, uint32_t y)
{
	struct texture *ascii = &g_voxel->textures->ascii;
	size_t scale = ascii->height / 128;
	for (size_t yy = 0; yy < scale; ++yy)
	{
		for (size_t xx = 0; xx < scale; ++xx)
		{
			if (ascii->data[((y * scale + yy) * ascii->width + x * scale + xx) * 4 + 3])
				return true;
		}
	}
	return false;
}

struct gui *gui_new(void)
{
	struct gui *gui = calloc(sizeof(*gui), 1);
	if (!gui)
	{
		LOG_ERROR("gui allocation failed");
		return NULL;
	}
	gui->scale = 3;
	for (size_t i = 0; i < 256; ++i)
	{
		if (i == ' ')
		{
			gui->char_size[i] = 5;
			continue;
		}
		uint8_t x = i % 16;
		uint8_t y = i / 16;
		gui->char_size[i] = 8;
		for (size_t xx = 0; xx < 8; ++xx)
		{
			bool found = false;
			for (size_t yy = 0; yy < 8; ++yy)
			{
				if (has_pixel_alpha(x * 8 + xx, y * 8 + yy))
				{
					found = true;
					break;
				}
			}
			if (found)
				break;
			gui->char_size[i]--;
			gui->char_size[i] += (1 << 4);
		}
		for (ssize_t xx = 7; xx > (gui->char_size[i] >> 4); --xx)
		{
			bool found = false;
			for (size_t yy = 0; yy < 8; ++yy)
			{
				if (has_pixel_alpha(x * 8 + xx, y * 8 + yy))
				{
					found = true;
					break;
				}
			}
			if (found)
				break;
			gui->char_size[i]--;
		}
		if (!(gui->char_size[i] & 0xF))
			gui->char_size[i] = 8;
	}
	return gui;
}

void gui_delete(struct gui *gui)
{
	if (!gui)
		return;
	free(gui);
}

void gui_update(struct gui *gui)
{
	gui->width = g_voxel->window->width / gui->scale;
	gui->height = g_voxel->window->height / gui->scale;
	MAT4_ORTHO(float, gui->mat, 0, gui->width, gui->height, 0, -2, 2);
}

struct vec4f gui_get_text_color(char i)
{
	if (i >= '0' && i <= '9')
		return text_colors[i - '0'];
	if (i >= 'a' && i <=  'f')
		return text_colors[10 + i - 'a'];
	if (i >= 'A' && i <= 'F')
		return text_colors[10 + i - 'A'];
	return text_colors[15];
}

struct vec4f gui_get_text_shadow_color(char i)
{
	if (i >= '0' && i <= '9')
		return text_shadow_colors[i - '0'];
	if (i >= 'a' && i <= 'f')
		return text_shadow_colors[10 + i - 'a'];
	if (i >= 'A' && i <= 'F')
		return text_shadow_colors[10 + i - 'A'];
	return text_shadow_colors[15];
}
