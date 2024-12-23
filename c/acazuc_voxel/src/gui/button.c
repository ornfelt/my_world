#include "gui/button.h"
#include "gui/gui.h"

#include "textures.h"
#include "voxel.h"

#include <gfx/events.h>

#include <stddef.h>

void gui_button_init(struct gui_button *button)
{
	button->callback = NULL;
	button->height = 20;
	button->width = 200;
	button->x = 0;
	button->y = 0;
	button->disabled = false;
	button->hover = false;
	gui_sprite_init(&button->bg_right);
	gui_sprite_set_texture(&button->bg_right, &g_voxel->textures->gui);
	gui_sprite_set_tex_size(&button->bg_right, button->width / 2 / 256., 20 / 256.);
	gui_sprite_init(&button->bg_left);
	gui_sprite_set_texture(&button->bg_left, &g_voxel->textures->gui);
	gui_sprite_set_tex_size(&button->bg_left, button->width / 2 / 256., 20 / 256.);
	gui_label_init(&button->label);
}

void gui_button_destroy(struct gui_button *button)
{
	gui_sprite_destroy(&button->bg_right);
	gui_sprite_destroy(&button->bg_left);
	gui_label_destroy(&button->label);
}

void gui_button_draw(struct gui_button *button)
{
	if (button->disabled)
	{
		gui_label_set_color(&button->label, '7');
		gui_sprite_set_tex_pos(&button->bg_left, 0, 46 / 256.0f);
		gui_sprite_set_tex_pos(&button->bg_right, (200 - button->width / 2) / 256.0f, 46 / 256.0f);
	}
	else if (button->hover)
	{
		gui_label_set_color(&button->label, 'e');
		gui_sprite_set_tex_pos(&button->bg_left, 0, 86 / 256.0f);
		gui_sprite_set_tex_pos(&button->bg_right, (200 - button->width / 2) / 256.0f, 86 / 256.0f);
	}
	else
	{
		gui_label_set_color(&button->label, 'f');
		gui_sprite_set_tex_pos(&button->bg_left, 0, 66 / 256.0f);
		gui_sprite_set_tex_pos(&button->bg_right, (200 - button->width / 2) / 256.0f, 66 / 256.0f);
	}
	gui_sprite_set_size(&button->bg_right, button->width / 2, button->height);
	gui_sprite_draw(&button->bg_right);
	gui_sprite_set_size(&button->bg_left, button->width / 2, button->height);
	gui_sprite_draw(&button->bg_left);
	gui_label_draw(&button->label);
}

void gui_button_mouse_move(struct gui_button *button,
                           struct gfx_pointer_event *event)
{
	if (!event->used
	 && event->x >= button->x
	 && event->x <= button->x + button->width
	 && event->y >= button->y
	 && event->y <= button->y + button->height)
	{
		button->hover = true;
		event->used = true;
	}
	else
	{
		button->hover = false;
	}
}

void gui_button_mouse_down(struct gui_button *button,
                           struct gfx_mouse_event *event)
{
	if (!button->hover || event->button != GFX_MOUSE_BUTTON_LEFT || event->used)
		return;
	if (button->callback)
		button->callback(button->callback_data);
	event->used = true;
}

void gui_button_set_text(struct gui_button *button, const char *text)
{
	gui_label_set_text(&button->label, text);
	gui_label_set_pos(&button->label,
	                  button->x + (button->width - gui_label_get_width(&button->label)) / 2,
	                  button->y + (button->height - gui_label_get_height(&button->label)) / 2);
}

void gui_button_set_pos(struct gui_button *button, int32_t x, int32_t y)
{
	button->x = x;
	button->y = y;
	gui_sprite_set_pos(&button->bg_left, button->x, button->y);
	gui_sprite_set_pos(&button->bg_right, button->x + button->width / 2, button->y);
	gui_label_set_pos(&button->label,
	                  button->x + (button->width - gui_label_get_width(&button->label)) / 2,
	                  button->y + (button->height - gui_label_get_height(&button->label)) / 2);
}

void gui_button_set_size(struct gui_button *button, int32_t width, int32_t height)
{
	button->width = width;
	button->height = height;
	gui_sprite_set_tex_size(&button->bg_left, width / 2 / 256., 20 / 256.);
	gui_sprite_set_tex_size(&button->bg_right, width / 2 / 256., 20 / 256.);
	gui_sprite_set_tex_pos(&button->bg_right, (200 - width / 2) / 256., 0);
	gui_label_set_pos(&button->label,
	                  button->x + (button->width - gui_label_get_width(&button->label)) / 2,
	                  button->y + (button->height - gui_label_get_height(&button->label)) / 2);
}

void gui_button_set_disabled(struct gui_button *button, bool disabled)
{
	button->disabled = disabled;
}

void gui_button_set_callback(struct gui_button *button, gui_button_cb_t cb,
                             void *userdata)
{
	button->callback = cb;
	button->callback_data = userdata;
}
