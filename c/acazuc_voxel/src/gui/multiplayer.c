#include "gui/multiplayer_entry.h"
#include "gui/multiplayer.h"
#include "gui/gui.h"

#include "title_screen/title_screen.h"

#include "textures.h"
#include "voxel.h"

static void on_join(void *data)
{
	(void)data;
}

static void on_direct(void *data)
{
	(void)data;
}

static void on_add(void *data)
{
	(void)data;
}

static void on_edit(void *data)
{
	(void)data;
}

static void on_remove(void *data)
{
	(void)data;
}

static void on_refresh(void *data)
{
	(void)data;
}

static void on_cancel(void *data)
{
	(void)data;
	((struct title_screen*)g_voxel->screen)->mode = TITLE_SCREEN_TITLE;
}

static void entry_destroy(void *ptr)
{
	gui_multiplayer_entry_delete(*(struct gui_multiplayer_entry**)ptr);
}

void gui_multiplayer_init(struct gui_multiplayer *multiplayer)
{
	jks_array_init(&multiplayer->entries, sizeof(struct gui_multiplayer_entry*), entry_destroy, NULL);
	multiplayer->selected = -1;
	gui_sprite_init(&multiplayer->top_gradient);
	gui_sprite_set_texture(&multiplayer->top_gradient, &g_voxel->textures->whitepixel);
	gui_sprite_set_top_color(&multiplayer->top_gradient, 0, 0, 0, 1);
	gui_sprite_set_bot_color(&multiplayer->top_gradient, 0, 0, 0, 0);
	gui_sprite_init(&multiplayer->bot_gradient);
	gui_sprite_set_texture(&multiplayer->bot_gradient, &g_voxel->textures->whitepixel);
	gui_sprite_set_top_color(&multiplayer->bot_gradient, 0, 0, 0, 0);
	gui_sprite_set_bot_color(&multiplayer->bot_gradient, 0, 0, 0, 1);
	gui_sprite_init(&multiplayer->bg_darker);
	gui_sprite_set_texture(&multiplayer->bg_darker, &g_voxel->textures->bg);
	gui_sprite_set_color(&multiplayer->bg_darker, 0.125f, 0.125f, 0.125f, 1);
	for (int i = 0; i < 3; ++i)
	{
		struct gui_multiplayer_entry *entry = gui_multiplayer_entry_new(multiplayer, i);
		jks_array_push_back(&multiplayer->entries, &entry);
	}
	gui_button_init(&multiplayer->refresh);
	gui_button_set_size(&multiplayer->refresh, 70, 20);
	gui_button_set_text(&multiplayer->refresh, "Refresh");
	gui_button_set_callback(&multiplayer->refresh, on_refresh, NULL);
	gui_button_init(&multiplayer->direct);
	gui_button_set_size(&multiplayer->direct, 100, 20);
	gui_button_set_text(&multiplayer->direct, "Direct Connect");
	gui_button_set_callback(&multiplayer->direct, on_direct, NULL);
	gui_button_init(&multiplayer->remove);
	gui_button_set_size(&multiplayer->remove, 70, 20);
	gui_button_set_text(&multiplayer->remove, "Delete");
	gui_button_set_callback(&multiplayer->remove, on_remove, NULL);
	gui_button_init(&multiplayer->cancel);
	gui_button_set_size(&multiplayer->cancel, 74, 20);
	gui_button_set_text(&multiplayer->cancel, "Cancel");
	gui_button_set_callback(&multiplayer->cancel, on_cancel, NULL);
	gui_button_init(&multiplayer->join);
	gui_button_set_size(&multiplayer->join, 100, 20);
	gui_button_set_text(&multiplayer->join, "Join Server");
	gui_button_set_callback(&multiplayer->join, on_join, NULL);
	gui_button_init(&multiplayer->edit);
	gui_button_set_size(&multiplayer->edit, 70, 20);
	gui_button_set_text(&multiplayer->edit, "Edit");
	gui_button_set_callback(&multiplayer->edit, on_edit, NULL);
	gui_button_init(&multiplayer->add);
	gui_button_set_size(&multiplayer->add, 100, 20);
	gui_button_set_text(&multiplayer->add, "Add Server");
	gui_button_set_callback(&multiplayer->add, on_add, NULL);
}

void gui_multiplayer_destroy(struct gui_multiplayer *multiplayer)
{
	jks_array_destroy(&multiplayer->entries);
	gui_sprite_destroy(&multiplayer->top_gradient);
	gui_sprite_destroy(&multiplayer->bot_gradient);
	gui_sprite_destroy(&multiplayer->background);
	gui_sprite_destroy(&multiplayer->bg_darker);
	gui_button_destroy(&multiplayer->refresh);
	gui_button_destroy(&multiplayer->direct);
	gui_button_destroy(&multiplayer->remove);
	gui_button_destroy(&multiplayer->cancel);
	gui_button_destroy(&multiplayer->join);
	gui_button_destroy(&multiplayer->edit);
	gui_button_destroy(&multiplayer->add);
}

void gui_multiplayer_draw(struct gui_multiplayer *multiplayer)
{
	gui_sprite_set_pos(&multiplayer->bg_darker, 0, 16 * 2);
	gui_sprite_set_size(&multiplayer->bg_darker,
	                    g_voxel->gui->width,
	                    g_voxel->gui->height - 4 * 24);
	gui_sprite_set_tex_size(&multiplayer->bg_darker,
	                        g_voxel->gui->width / (float)(16 * 2),
	                        (g_voxel->gui->height - 4 * 24) / (float)(16 * 2));
	gui_sprite_draw(&multiplayer->bg_darker);
	for (size_t i = 0; i < multiplayer->entries.size; ++i)
		gui_multiplayer_entry_draw(*JKS_ARRAY_GET(&multiplayer->entries, i, struct gui_multiplayer_entry*));
	gui_sprite_set_pos(&multiplayer->top_gradient, 0, 16 * 2);
	gui_sprite_set_size(&multiplayer->top_gradient, g_voxel->gui->width, 4);
	gui_sprite_draw(&multiplayer->top_gradient);
	gui_sprite_set_pos(&multiplayer->bot_gradient, 0, g_voxel->gui->height - 16 * 2 * 2 - 4);
	gui_sprite_set_size(&multiplayer->bot_gradient, g_voxel->gui->width, 4);
	gui_sprite_draw(&multiplayer->bot_gradient);
	int32_t top = g_voxel->gui->height - 26 * 2;
	int32_t bot = g_voxel->gui->height - 14 * 2;
	gui_button_set_pos(&multiplayer->refresh, g_voxel->gui->width / 2 + 4, bot);
	gui_button_draw(&multiplayer->refresh);
	gui_button_set_pos(&multiplayer->direct, g_voxel->gui->width / 2 - 50, top);
	gui_button_draw(&multiplayer->direct);
	gui_button_set_pos(&multiplayer->remove, g_voxel->gui->width / 2 - 74, bot);
	gui_button_draw(&multiplayer->remove);
	gui_button_set_pos(&multiplayer->cancel, g_voxel->gui->width / 2 + 80, bot);
	gui_button_draw(&multiplayer->cancel);
	gui_button_set_pos(&multiplayer->join, g_voxel->gui->width / 2 - 154, top);
	gui_button_draw(&multiplayer->join);
	gui_button_set_pos(&multiplayer->edit, g_voxel->gui->width / 2 - 154, bot);
	gui_button_draw(&multiplayer->edit);
	gui_button_set_pos(&multiplayer->add, g_voxel->gui->width / 2 + 54, top);
	gui_button_draw(&multiplayer->add);
}

void gui_multiplayer_mouse_move(struct gui_multiplayer *multiplayer,
                                struct gfx_pointer_event *event)
{
	for (size_t i = 0; i < multiplayer->entries.size; ++i)
		gui_multiplayer_entry_mouse_move(*JKS_ARRAY_GET(&multiplayer->entries, i, struct gui_multiplayer_entry*), event);
	gui_button_mouse_move(&multiplayer->join, event);
	gui_button_mouse_move(&multiplayer->direct, event);
	gui_button_mouse_move(&multiplayer->add, event);
	gui_button_mouse_move(&multiplayer->edit, event);
	gui_button_mouse_move(&multiplayer->remove, event);
	gui_button_mouse_move(&multiplayer->refresh, event);
	gui_button_mouse_move(&multiplayer->cancel, event);
}

void gui_multiplayer_mouse_down(struct gui_multiplayer *multiplayer,
                                struct gfx_mouse_event *event)
{
	for (size_t i = 0; i < multiplayer->entries.size; ++i)
		gui_multiplayer_entry_mouse_down(*JKS_ARRAY_GET(&multiplayer->entries, i, struct gui_multiplayer_entry*), event);
	gui_button_mouse_down(&multiplayer->join, event);
	gui_button_mouse_down(&multiplayer->direct, event);
	gui_button_mouse_down(&multiplayer->add, event);
	gui_button_mouse_down(&multiplayer->edit, event);
	gui_button_mouse_down(&multiplayer->remove, event);
	gui_button_mouse_down(&multiplayer->refresh, event);
	gui_button_mouse_down(&multiplayer->cancel, event);
}

void gui_multiplayer_mouse_up(struct gui_multiplayer *multiplayer,
                              struct gfx_mouse_event *event)
{
	(void)multiplayer;
	(void)event;
}

void gui_multiplayer_key_down(struct gui_multiplayer *multiplayer,
                              struct gfx_key_event *event)
{
	(void)multiplayer;
	if (event->used)
		return;
	if (event->key == GFX_KEY_ESCAPE)
	{
		((struct title_screen*)g_voxel->screen)->mode = TITLE_SCREEN_TITLE;
		event->used = true;
	}
}
