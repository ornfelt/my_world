#include "gui/singleplayer.h"
#include "gui/gui.h"

#include "title_screen/title_screen.h"

#include "world_screen/world_screen.h"

#include "world/world.h"

#include "textures.h"
#include "voxel.h"

#include <dirent.h>
#include <string.h>
#include <stdio.h>

static void on_recreate(void *data)
{
	(void)data;
}

static void on_cancel(void *data)
{
	(void)data;
	((struct title_screen*)g_voxel->screen)->mode = TITLE_SCREEN_TITLE;
}

static void on_remove(void *data)
{
	(void)data;
}

static void on_create(void *data)
{
	(void)data;
}

static void on_edit(void *data)
{
	(void)data;
}

static void on_play(void *data)
{
	g_voxel->world = world_new();
	if (g_voxel->world)
		g_voxel->next_screen = world_screen_new();
}

static void entry_destroy(void *ptr)
{
	gui_singleplayer_entry_delete(*(struct gui_singleplayer_entry**)ptr);
}

void gui_singleplayer_init(struct gui_singleplayer *singleplayer)
{
	jks_array_init(&singleplayer->entries, sizeof(struct gui_singleplayer_entry*), entry_destroy, NULL);
	singleplayer->selected = -1;
	gui_sprite_init(&singleplayer->top_gradient);
	gui_sprite_set_texture(&singleplayer->top_gradient, &g_voxel->textures->whitepixel);
	gui_sprite_set_top_color(&singleplayer->top_gradient, 0, 0, 0, 1);
	gui_sprite_set_bot_color(&singleplayer->top_gradient, 0, 0, 0, 0);
	gui_sprite_init(&singleplayer->bot_gradient);
	gui_sprite_set_texture(&singleplayer->bot_gradient, &g_voxel->textures->whitepixel);
	gui_sprite_set_top_color(&singleplayer->bot_gradient, 0, 0, 0, 0);
	gui_sprite_set_bot_color(&singleplayer->bot_gradient, 0, 0, 0, 1);
	gui_sprite_init(&singleplayer->bg_darker);
	gui_sprite_set_texture(&singleplayer->bg_darker, &g_voxel->textures->bg);
	gui_sprite_set_color(&singleplayer->bg_darker, 0.125f, 0.125f, 0.125f, 1);
	DIR *dir = opendir("saves");
	if (dir)
	{
		struct dirent *dirent;
		while ((dirent = readdir(dir)))
		{
			if (!strcmp(dirent->d_name, ".")
			 || !strcmp(dirent->d_name, ".."))
				continue;
			struct gui_singleplayer_entry *entry = gui_singleplayer_entry_new(singleplayer, singleplayer->entries.size, dirent->d_name);
			if (entry)
				jks_array_push_back(&singleplayer->entries, &entry);
		}
		closedir(dir);
	}
	gui_button_init(&singleplayer->recreate);
	gui_button_set_size(&singleplayer->recreate, 72, 20);
	gui_button_set_text(&singleplayer->recreate, "Re-Create");
	gui_button_set_callback(&singleplayer->recreate, on_recreate, NULL);
	gui_button_init(&singleplayer->cancel);
	gui_button_set_size(&singleplayer->cancel, 72, 20);
	gui_button_set_text(&singleplayer->cancel, "Cancel");
	gui_button_set_callback(&singleplayer->cancel, on_cancel, NULL);
	gui_button_init(&singleplayer->remove);
	gui_button_set_size(&singleplayer->remove, 72, 20);
	gui_button_set_text(&singleplayer->remove, "Delete");
	gui_button_set_callback(&singleplayer->remove, on_remove, NULL);
	gui_button_init(&singleplayer->create);
	gui_button_set_size(&singleplayer->create, 150, 20);
	gui_button_set_text(&singleplayer->create, "Create New World");
	gui_button_set_callback(&singleplayer->create, on_create, NULL);
	gui_button_init(&singleplayer->edit);
	gui_button_set_size(&singleplayer->edit, 72, 20);
	gui_button_set_text(&singleplayer->edit, "Edit");
	gui_button_set_callback(&singleplayer->edit, on_edit, NULL);
	gui_button_init(&singleplayer->play);
	gui_button_set_size(&singleplayer->play, 150, 20);
	gui_button_set_text(&singleplayer->play, "Play Selected World");
	gui_button_set_callback(&singleplayer->play, on_play, NULL);
}

void gui_singleplayer_destroy(struct gui_singleplayer *singleplayer)
{
	jks_array_destroy(&singleplayer->entries);
	gui_sprite_destroy(&singleplayer->top_gradient);
	gui_sprite_destroy(&singleplayer->bot_gradient);
	gui_sprite_destroy(&singleplayer->background);
	gui_sprite_destroy(&singleplayer->bg_darker);
	gui_button_destroy(&singleplayer->recreate);
	gui_button_destroy(&singleplayer->cancel);
	gui_button_destroy(&singleplayer->remove);
	gui_button_destroy(&singleplayer->create);
	gui_button_destroy(&singleplayer->edit);
	gui_button_destroy(&singleplayer->play);
}

void gui_singleplayer_draw(struct gui_singleplayer *singleplayer)
{
	gui_sprite_set_pos(&singleplayer->bg_darker, 0, 16 * 2);
	gui_sprite_set_size(&singleplayer->bg_darker,
	                    g_voxel->gui->width,
	                    g_voxel->gui->height - 4 * 24);
	gui_sprite_set_tex_size(&singleplayer->bg_darker,
	                        g_voxel->gui->width / (float)(16 * 2),
	                        (g_voxel->gui->height - 4 * 24) / (float)(16 * 2));
	gui_sprite_draw(&singleplayer->bg_darker);
	for (size_t i = 0; i < singleplayer->entries.size; ++i)
		gui_singleplayer_entry_draw(*JKS_ARRAY_GET(&singleplayer->entries, i, struct gui_singleplayer_entry*));
	gui_sprite_set_pos(&singleplayer->top_gradient, 0, 16 * 2);
	gui_sprite_set_size(&singleplayer->top_gradient, g_voxel->gui->width, 4);
	gui_sprite_draw(&singleplayer->top_gradient);
	gui_sprite_set_pos(&singleplayer->bot_gradient, 0, g_voxel->gui->height - 16 * 2 * 2 - 4);
	gui_sprite_set_size(&singleplayer->bot_gradient, g_voxel->gui->width, 4);
	gui_sprite_draw(&singleplayer->bot_gradient);
	int32_t top = g_voxel->gui->height - 26 * 2;
	int32_t bot = g_voxel->gui->height - 14 * 2;
	gui_button_set_pos(&singleplayer->play, g_voxel->gui->width / 2 - (4 + singleplayer->play.width), top);
	gui_button_draw(&singleplayer->play);
	gui_button_set_pos(&singleplayer->edit, g_voxel->gui->width / 2 - (4 + singleplayer->play.width), bot);
	gui_button_draw(&singleplayer->edit);
	gui_button_set_pos(&singleplayer->remove, g_voxel->gui->width / 2 - (4 + singleplayer->play.width - singleplayer->edit.width - 6), bot);
	gui_button_draw(&singleplayer->remove);
	gui_button_set_pos(&singleplayer->create, g_voxel->gui->width / 2 + 4, top);
	gui_button_draw(&singleplayer->create);
	gui_button_set_pos(&singleplayer->recreate, g_voxel->gui->width / 2 + 4, bot);
	gui_button_draw(&singleplayer->recreate);
	gui_button_set_pos(&singleplayer->cancel, g_voxel->gui->width / 2 + 4 + singleplayer->recreate.width + 6, bot);
	gui_button_draw(&singleplayer->cancel);
}

void gui_singleplayer_mouse_move(struct gui_singleplayer *singleplayer,
                                 struct gfx_pointer_event *event)
{
	for (size_t i = 0; i < singleplayer->entries.size; ++i)
		gui_singleplayer_entry_mouse_move(*JKS_ARRAY_GET(&singleplayer->entries, i, struct gui_singleplayer_entry*), event);
	gui_button_mouse_move(&singleplayer->create, event);
	gui_button_mouse_move(&singleplayer->edit, event);
	gui_button_mouse_move(&singleplayer->remove, event);
	gui_button_mouse_move(&singleplayer->play, event);
	gui_button_mouse_move(&singleplayer->recreate, event);
	gui_button_mouse_move(&singleplayer->cancel, event);
}

void gui_singleplayer_mouse_down(struct gui_singleplayer *singleplayer,
                                 struct gfx_mouse_event *event)
{
	for (size_t i = 0; i < singleplayer->entries.size; ++i)
		gui_singleplayer_entry_mouse_down(*JKS_ARRAY_GET(&singleplayer->entries, i, struct gui_singleplayer_entry*), event);
	gui_button_mouse_down(&singleplayer->create, event);
	gui_button_mouse_down(&singleplayer->edit, event);
	gui_button_mouse_down(&singleplayer->remove, event);
	gui_button_mouse_down(&singleplayer->play, event);
	gui_button_mouse_down(&singleplayer->recreate, event);
	gui_button_mouse_down(&singleplayer->cancel, event);
}

void gui_singleplayer_mouse_up(struct gui_singleplayer *singleplayer,
                               struct gfx_mouse_event *event)
{
	(void)event;
}

void gui_singleplayer_key_down(struct gui_singleplayer *singleplayer,
                               struct gfx_key_event *event)
{
	if (event->used)
		return;
	if (event->key == GFX_KEY_ESCAPE)
	{
		((struct title_screen*)g_voxel->screen)->mode = TITLE_SCREEN_TITLE;
		event->used = true;
	}
}
