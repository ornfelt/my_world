#include "title_screen/title_screen.h"

#include "gui/gui.h"

#include "graphics.h"
#include "textures.h"
#include "voxel.h"
#include "log.h"

#include <gfx/device.h>

#include <stdlib.h>

static void destroy(struct screen *screen)
{
	struct title_screen *title_screen = (struct title_screen*)screen;
	gui_singleplayer_destroy(&title_screen->singleplayer);
	gui_multiplayer_destroy(&title_screen->multiplayer);
	gui_options_destroy(&title_screen->options);
	gui_sprite_destroy(&title_screen->background);
	gui_title_destroy(&title_screen->title);
}

static void draw(struct screen *screen)
{
	struct title_screen *title_screen = (struct title_screen*)screen;
	gfx_bind_pipeline_state(g_voxel->device,
	                        &g_voxel->graphics->gui.pipeline_state);
	gui_sprite_set_size(&title_screen->background, g_voxel->gui->width, g_voxel->gui->height);
	gui_sprite_set_tex_size(&title_screen->background,
	                        g_voxel->gui->width / (float)(16 * 2),
	                        g_voxel->gui->height / (float)(16 * 2));
	gui_sprite_draw(&title_screen->background);
	switch (title_screen->mode)
	{
		case TITLE_SCREEN_TITLE:
			gui_title_draw(&title_screen->title);
			break;
		case TITLE_SCREEN_SINGLEPLAYER:
			gui_singleplayer_draw(&title_screen->singleplayer);
			break;
		case TITLE_SCREEN_MULTIPLAYER:
			gui_multiplayer_draw(&title_screen->multiplayer);
			break;
		case TITLE_SCREEN_OPTIONS:
			gui_options_draw(&title_screen->options);
			break;
		case TITLE_SCREEN_TEXTURE_PACKS:
			break;
	}
}

static void mouse_move(struct screen *screen, struct gfx_pointer_event *event)
{
	struct title_screen *title_screen = (struct title_screen*)screen;
	switch (title_screen->mode)
	{
		case TITLE_SCREEN_TITLE:
			gui_title_mouse_move(&title_screen->title, event);
			break;
		case TITLE_SCREEN_SINGLEPLAYER:
			gui_singleplayer_mouse_move(&title_screen->singleplayer, event);
			break;
		case TITLE_SCREEN_MULTIPLAYER:
			gui_multiplayer_mouse_move(&title_screen->multiplayer, event);
			break;
		case TITLE_SCREEN_OPTIONS:
			gui_options_mouse_move(&title_screen->options, event);
			break;
		case TITLE_SCREEN_TEXTURE_PACKS:
			break;
	}
}

static void mouse_down(struct screen *screen, struct gfx_mouse_event *event)
{
	struct title_screen *title_screen = (struct title_screen*)screen;
	switch (title_screen->mode)
	{
		case TITLE_SCREEN_TITLE:
			gui_title_mouse_down(&title_screen->title, event);
			break;
		case TITLE_SCREEN_SINGLEPLAYER:
			gui_singleplayer_mouse_down(&title_screen->singleplayer, event);
			break;
		case TITLE_SCREEN_MULTIPLAYER:
			gui_multiplayer_mouse_down(&title_screen->multiplayer, event);
			break;
		case TITLE_SCREEN_OPTIONS:
			gui_options_mouse_down(&title_screen->options, event);
			break;
		case TITLE_SCREEN_TEXTURE_PACKS:
			break;
	}
}

static void mouse_up(struct screen *screen, struct gfx_mouse_event *event)
{
	struct title_screen *title_screen = (struct title_screen*)screen;
	switch (title_screen->mode)
	{
		case TITLE_SCREEN_TITLE:
			gui_title_mouse_up(&title_screen->title, event);
			break;
		case TITLE_SCREEN_SINGLEPLAYER:
			gui_singleplayer_mouse_up(&title_screen->singleplayer, event);
			break;
		case TITLE_SCREEN_MULTIPLAYER:
			gui_multiplayer_mouse_up(&title_screen->multiplayer, event);
			break;
		case TITLE_SCREEN_OPTIONS:
			gui_options_mouse_up(&title_screen->options, event);
			break;
		case TITLE_SCREEN_TEXTURE_PACKS:
			break;
	}
}

static void key_down(struct screen *screen, struct gfx_key_event *event)
{
	struct title_screen *title_screen = (struct title_screen*)screen;
	switch (title_screen->mode)
	{
		case TITLE_SCREEN_TITLE:
			break;
		case TITLE_SCREEN_SINGLEPLAYER:
			gui_singleplayer_key_down(&title_screen->singleplayer, event);
			break;
		case TITLE_SCREEN_MULTIPLAYER:
			gui_multiplayer_key_down(&title_screen->multiplayer, event);
			break;
		case TITLE_SCREEN_OPTIONS:
			gui_options_key_down(&title_screen->options, event);
			break;
		case TITLE_SCREEN_TEXTURE_PACKS:
			break;
	}
}

static const struct screen_op screen_op =
{
	.destroy = destroy,
	.draw = draw,
	.mouse_move = mouse_move,
	.mouse_down = mouse_down,
	.mouse_up = mouse_up,
	.key_down = key_down,
};

struct screen *title_screen_new(void)
{
	struct title_screen *title_screen = calloc(sizeof(*title_screen), 1);
	if (!title_screen)
	{
		LOG_ERROR("title screen allocation failed");
		return NULL;
	}
	title_screen->mode = TITLE_SCREEN_TITLE;
	title_screen->screen.op = &screen_op;
	gui_singleplayer_init(&title_screen->singleplayer);
	gui_multiplayer_init(&title_screen->multiplayer);
	gui_options_init(&title_screen->options);
	gui_sprite_init(&title_screen->background);
	gui_sprite_set_texture(&title_screen->background, &g_voxel->textures->bg);
	gui_sprite_set_color(&title_screen->background, 0.25f, 0.25f, 0.25f, 1);
	gui_title_init(&title_screen->title);
	voxel_ungrab_cursor(g_voxel);
	return &title_screen->screen;
}
