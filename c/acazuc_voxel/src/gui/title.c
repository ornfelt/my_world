#include "gui/title.h"
#include "gui/gui.h"

#include "title_screen/title_screen.h"

#include "textures.h"
#include "voxel.h"

#include <stdlib.h>

static void on_singleplayer(void *data)
{
	(void)data;
	struct title_screen *title_screen = (struct title_screen*)g_voxel->screen;
	title_screen->mode = TITLE_SCREEN_SINGLEPLAYER;
}

static void on_multiplayer(void *data)
{
	struct title_screen *title_screen = (struct title_screen*)g_voxel->screen;
	title_screen->mode = TITLE_SCREEN_MULTIPLAYER;
}

static void on_texture_packs(void *data)
{
	struct title_screen *title_screen = (struct title_screen*)g_voxel->screen;
	title_screen->mode = TITLE_SCREEN_TEXTURE_PACKS;
}

static void on_options(void *data)
{
	struct title_screen *title_screen = (struct title_screen*)g_voxel->screen;
	title_screen->mode = TITLE_SCREEN_OPTIONS;
}

static void on_quit(void *data)
{
	(void)data;
	exit(EXIT_SUCCESS);
}

void gui_title_init(struct gui_title *title)
{
	gui_sprite_init(&title->logo_left);
	gui_sprite_set_texture(&title->logo_left, &g_voxel->textures->logo);
	gui_sprite_set_size(&title->logo_left, 156, 45);
	gui_sprite_set_tex_size(&title->logo_left, 156 / 256.0f, 45 / 256.0f);
	gui_sprite_init(&title->logo_right);
	gui_sprite_set_texture(&title->logo_right, &g_voxel->textures->logo);
	gui_sprite_set_size(&title->logo_right, 120, 45);
	gui_sprite_set_tex_size(&title->logo_right, 120 / 256.0f, 45 / 256.0f);
	gui_sprite_set_tex_pos(&title->logo_right, 0, 45 / 256.0f);
	gui_button_init(&title->singleplayer);
	gui_button_set_text(&title->singleplayer, "Singleplayer");
	gui_button_set_callback(&title->singleplayer, on_singleplayer, NULL);
	gui_button_init(&title->multiplayer);
	gui_button_set_text(&title->multiplayer, "Multiplayer");
	gui_button_set_callback(&title->multiplayer, on_multiplayer, NULL);
	gui_button_init(&title->texture_packs);
	gui_button_set_text(&title->texture_packs, "Texture Packs");
	gui_button_set_callback(&title->texture_packs, on_texture_packs, NULL);
	gui_button_init(&title->options);
	gui_button_set_text(&title->options, "Options");
	gui_button_set_callback(&title->options, on_options, NULL);
	gui_button_set_size(&title->options, 98, 20);
	gui_button_init(&title->quit);
	gui_button_set_text(&title->quit, "Quit");
	gui_button_set_callback(&title->quit, on_quit, NULL);
	gui_button_set_size(&title->quit, 98, 20);
	gui_label_init(&title->version);
	gui_label_set_text(&title->version, "voxel 1.0");
	gui_label_init(&title->license);
	gui_label_set_text(&title->license, "Copyleft GNU AGPLv3. Do distribute!");
}

void gui_title_destroy(struct gui_title *title)
{
	gui_sprite_destroy(&title->logo_left);
	gui_sprite_destroy(&title->logo_right);
	gui_button_destroy(&title->singleplayer);
	gui_button_destroy(&title->multiplayer);
	gui_button_destroy(&title->texture_packs);
	gui_button_destroy(&title->options);
	gui_button_destroy(&title->quit);
}

void gui_title_draw(struct gui_title *title)
{
	gui_sprite_set_pos(&title->logo_left,
	                   (g_voxel->gui->width - 276) / 2,
	                   19);
	gui_sprite_draw(&title->logo_left);
	gui_sprite_set_pos(&title->logo_right,
	                   (g_voxel->gui->width - 276) / 2 + 156,
	                   19);
	gui_sprite_draw(&title->logo_right);
	int32_t offset = 48 + 19 * 2;
	float left = (g_voxel->gui->width - 200) / 2;
	gui_button_set_pos(&title->singleplayer, left, offset);
	gui_button_draw(&title->singleplayer);
	gui_button_set_pos(&title->multiplayer, left, offset + 24);
	gui_button_draw(&title->multiplayer);
	gui_button_set_pos(&title->texture_packs, left, offset + 48);
	gui_button_draw(&title->texture_packs);
	gui_button_set_pos(&title->options, left, offset + 84);
	gui_button_draw(&title->options);
	gui_button_set_pos(&title->quit,
	                   g_voxel->gui->width / 2 + 2,
	                   offset + 84);
	gui_button_draw(&title->quit);
	gui_label_set_pos(&title->version, 1,
	                  g_voxel->gui->height - gui_label_get_height(&title->version) - 1);
	gui_label_draw(&title->version);
	gui_label_set_pos(&title->license,
	                  g_voxel->gui->width - gui_label_get_width(&title->license) - 1,
	                  g_voxel->gui->height - gui_label_get_height(&title->license) - 1);
	gui_label_draw(&title->license);
}

void gui_title_mouse_move(struct gui_title *title,
                          struct gfx_pointer_event *event)
{
	gui_button_mouse_move(&title->singleplayer, event);
	gui_button_mouse_move(&title->multiplayer, event);
	gui_button_mouse_move(&title->texture_packs, event);
	gui_button_mouse_move(&title->options, event);
	gui_button_mouse_move(&title->quit, event);
}

void gui_title_mouse_down(struct gui_title *title,
                          struct gfx_mouse_event *event)
{
	gui_button_mouse_down(&title->singleplayer, event);
	gui_button_mouse_down(&title->multiplayer, event);
	gui_button_mouse_down(&title->texture_packs, event);
	gui_button_mouse_down(&title->options, event);
	gui_button_mouse_down(&title->quit, event);
}

void gui_title_mouse_up(struct gui_title *title,
                        struct gfx_mouse_event *event)
{
	(void)title;
	(void)event;
}
