#include "gui/options.h"
#include "gui/gui.h"

#include "title_screen/title_screen.h"

#include "voxel.h"

#include <stdio.h>

static void on_resources(void *data)
{
	(void)data;
}

static void on_controls(void *data)
{
	(void)data;
}

static void on_language(void *data)
{
	(void)data;
}

static void on_sound(void *data)
{
	(void)data;
}

static void on_video(void *data)
{
	(void)data;
}

static void on_chat(void *data)
{
	(void)data;
}

static void on_done(void *data)
{
	(void)data;
	((struct title_screen*)g_voxel->screen)->mode = TITLE_SCREEN_TITLE;
}

static void on_skin(void *data)
{
	(void)data;
}

static void on_fov(float per, void *data)
{
	struct gui_options *options = data;
	char text[64];
	snprintf(text, sizeof(text), "FOV: %u", 30 + (int)(per * (110 - 30)));
	gui_slider_set_text(&options->fov, text);
}

void gui_options_init(struct gui_options *options)
{
	gui_button_init(&options->resources);
	gui_button_set_text(&options->resources, "Resources Packs...");
	gui_button_set_callback(&options->resources, on_resources, NULL);
	gui_button_init(&options->controls);
	gui_button_set_text(&options->controls, "Controls...");
	gui_button_set_callback(&options->controls, on_controls, NULL);
	gui_button_init(&options->language);
	gui_button_set_text(&options->language, "Language...");
	gui_button_set_callback(&options->language, on_language, NULL);
	gui_button_init(&options->sound);
	gui_button_set_text(&options->sound, "Music & Sounds...");
	gui_button_set_callback(&options->sound, on_sound, NULL);
	gui_button_init(&options->video);
	gui_button_set_text(&options->video, "Video Settings...");
	gui_button_set_callback(&options->video, on_video, NULL);
	gui_button_init(&options->chat);
	gui_button_set_text(&options->chat, "Chat Settings...");
	gui_button_set_callback(&options->chat, on_chat, NULL);
	gui_button_init(&options->done);
	gui_button_set_text(&options->done, "Done");
	gui_button_set_callback(&options->done, on_done, NULL);
	gui_button_init(&options->skin);
	gui_button_set_text(&options->skin, "Skin Customization...");
	gui_button_set_callback(&options->skin, on_skin, NULL);
	gui_slider_init(&options->fov);
	gui_slider_set_text(&options->fov, "FOV: 30");
	gui_slider_set_callback(&options->fov, on_fov, options);
	gui_label_init(&options->title);
	gui_label_set_text(&options->title, "Options");
}

void gui_options_destroy(struct gui_options *options)
{
	gui_button_destroy(&options->resources);
	gui_button_destroy(&options->controls);
	gui_button_destroy(&options->language);
	gui_button_destroy(&options->sound);
	gui_button_destroy(&options->video);
	gui_button_destroy(&options->chat);
	gui_button_destroy(&options->done);
	gui_button_destroy(&options->skin);
	gui_slider_destroy(&options->fov);
	gui_label_destroy(&options->title);
}

void gui_options_draw(struct gui_options *options)
{
	int32_t left = g_voxel->gui->width / 2 - 205;
	int32_t right = g_voxel->gui->width / 2 + 5;
	gui_button_set_pos(&options->resources, right, 48);
	gui_button_draw(&options->resources);
	gui_button_set_pos(&options->controls, right, 120);
	gui_button_draw(&options->controls);
	gui_button_set_pos(&options->language, left, 144);
	gui_button_draw(&options->language);
	gui_button_set_pos(&options->sound, right, 96);
	gui_button_draw(&options->sound);
	gui_button_set_pos(&options->video, left, 120);
	gui_button_draw(&options->video);
	gui_button_set_pos(&options->chat, right, 144);
	gui_button_draw(&options->chat);
	gui_button_set_pos(&options->done, (g_voxel->gui->width - options->done.width) / 2, 200);
	gui_button_draw(&options->done);
	gui_button_set_pos(&options->skin, left, 96);
	gui_button_draw(&options->skin);
	gui_slider_set_pos(&options->fov, left, 48);
	gui_slider_draw(&options->fov);
	gui_label_set_pos(&options->title, (g_voxel->gui->width - gui_label_get_width(&options->title)) / 2, 15);
	gui_label_draw(&options->title);
}

void gui_options_mouse_move(struct gui_options *options,
                            struct gfx_pointer_event *event)
{
	gui_button_mouse_move(&options->resources, event);
	gui_button_mouse_move(&options->controls, event);
	gui_button_mouse_move(&options->language, event);
	gui_button_mouse_move(&options->sound, event);
	gui_button_mouse_move(&options->video, event);
	gui_button_mouse_move(&options->chat, event);
	gui_button_mouse_move(&options->done, event);
	gui_button_mouse_move(&options->skin, event);
	gui_slider_mouse_move(&options->fov, event);
}

void gui_options_mouse_down(struct gui_options *options,
                            struct gfx_mouse_event *event)
{
	gui_button_mouse_down(&options->resources, event);
	gui_button_mouse_down(&options->controls, event);
	gui_button_mouse_down(&options->language, event);
	gui_button_mouse_down(&options->sound, event);
	gui_button_mouse_down(&options->video, event);
	gui_button_mouse_down(&options->chat, event);
	gui_button_mouse_down(&options->done, event);
	gui_button_mouse_down(&options->skin, event);
	gui_slider_mouse_down(&options->fov, event);
}

void gui_options_mouse_up(struct gui_options *options,
                          struct gfx_mouse_event *event)
{
	gui_slider_mouse_up(&options->fov, event);
}

void gui_options_key_down(struct gui_options *options,
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
