#include "gui/slider.h"
#include "gui/gui.h"

#include "textures.h"
#include "voxel.h"

#include <gfx/events.h>

#include <stddef.h>

void gui_slider_init(struct gui_slider *slider)
{
	slider->callback = NULL;
	slider->width = 200;
	slider->height = 20;
	slider->x = 0;
	slider->y = 0;
	slider->per = 0;
	slider->disabled = false;
	slider->clicked = false;
	slider->hover = false;
	gui_sprite_init(&slider->bg_left);
	gui_sprite_set_texture(&slider->bg_left, &g_voxel->textures->gui);
	gui_sprite_set_tex_pos(&slider->bg_left, 0, 46 / 255.0f);
	gui_sprite_set_tex_size(&slider->bg_left, slider->width / 2 / 256.0f, 20 / 256.0f);
	gui_sprite_set_size(&slider->bg_left, slider->width / 2, slider->height);
	gui_sprite_init(&slider->bg_right);
	gui_sprite_set_texture(&slider->bg_right, &g_voxel->textures->gui);
	gui_sprite_set_tex_pos(&slider->bg_right, (200 - slider->width / 2) / 256.0f, 46 / 256.0f);
	gui_sprite_set_tex_size(&slider->bg_right, slider->width / 2 / 256.0f, 20 / 256.0f);
	gui_sprite_set_size(&slider->bg_right, slider->width / 2, slider->height);
	gui_sprite_init(&slider->button_left);
	gui_sprite_set_texture(&slider->button_left, &g_voxel->textures->gui);
	gui_sprite_set_tex_size(&slider->button_left, 8 / 2 / 256.0f, 20 / 256.0f);
	gui_sprite_set_size(&slider->button_left, 4, slider->height);
	gui_sprite_init(&slider->button_right);
	gui_sprite_set_texture(&slider->button_right, &g_voxel->textures->gui);
	gui_sprite_set_tex_size(&slider->button_right, 8 / 2 / 256.0f, 20 / 256.0f);
	gui_sprite_set_size(&slider->button_right, 4, slider->height);
	gui_label_init(&slider->label);
}

void gui_slider_destroy(struct gui_slider *slider)
{
	gui_sprite_destroy(&slider->button_right);
	gui_sprite_destroy(&slider->button_left);
	gui_sprite_destroy(&slider->bg_right);
	gui_sprite_destroy(&slider->bg_left);
	gui_label_destroy(&slider->label);
}

void gui_slider_draw(struct gui_slider *slider)
{
	if (slider->disabled)
	{
		gui_label_set_color(&slider->label, '7');
		gui_sprite_set_tex_pos(&slider->button_left, 0, 46 / 256.0f);
		gui_sprite_set_tex_pos(&slider->button_right, (200 - 4) / 256.0f, 46 / 256.0f);
	}
	else if (slider->hover)
	{
		gui_label_set_color(&slider->label, 'e');
	}
	else
	{
		gui_label_set_color(&slider->label, 'f');
		gui_sprite_set_tex_pos(&slider->button_left, 0, 66 / 256.0f);
		gui_sprite_set_tex_pos(&slider->button_right, (200 - 4) / 256.0f, 66 / 256.0f);
	}
	gui_sprite_draw(&slider->bg_left);
	gui_sprite_draw(&slider->bg_right);
	gui_sprite_draw(&slider->button_left);
	gui_sprite_draw(&slider->button_right);
	gui_label_draw(&slider->label);
}

void gui_slider_mouse_move(struct gui_slider *slider,
                           struct gfx_pointer_event *event)
{
	if (slider->clicked)
	{
		slider->per = (event->x - slider->x - 4) / (float)(slider->width - 8);
		if (slider->per < 0)
			slider->per = 0;
		else if (slider->per > 1)
			slider->per = 1;
		if (slider->callback)
			slider->callback(slider->per, slider->callback_data);
		gui_sprite_set_pos(&slider->button_left, slider->x + (slider->width - 8) * slider->per, slider->y);
		gui_sprite_set_pos(&slider->button_right, slider->x + (slider->width - 8) * slider->per + 4, slider->y);
	}
	if (!event->used
	 && event->x >= slider->x
	 && event->x < slider->x + slider->width
	 && event->y >= slider->y
	 && event->y < slider->y + slider->height)
	{
		slider->hover = true;
		event->used = true;
	}
	else
	{
		slider->hover = false;
	}
}

void gui_slider_mouse_down(struct gui_slider *slider,
                           struct gfx_mouse_event *event)
{
	if (!slider->hover)
	{
		slider->clicked = false;
		return;
	}
	if (event->used)
		return;
	slider->clicked = true;
	slider->per = (event->x - slider->x - 4) / (float)(slider->width - 8);
	if (slider->per < 0)
		slider->per = 0;
	else if (slider->per > 1)
		slider->per = 1;
	if (slider->callback)
		slider->callback(slider->per, slider->callback_data);
	gui_sprite_set_pos(&slider->button_left, slider->x + (slider->width - 8) * slider->per, slider->y);
	gui_sprite_set_pos(&slider->button_right, slider->x + (slider->width - 8) * slider->per + 4, slider->y);
	event->used = true;
}

void gui_slider_mouse_up(struct gui_slider *slider,
                         struct gfx_mouse_event *event)
{
	if (event->used)
		return;
	if (slider->clicked || slider->hover)
	{
		slider->clicked = false;
		event->used = true;
	}
}

void gui_slider_set_text(struct gui_slider *slider, const char *text)
{
	gui_label_set_text(&slider->label, text);
	if (slider->label.dirty) /* XXX do better */
		gui_label_set_pos(&slider->label,
		                  slider->x + (slider->width - gui_label_get_width(&slider->label)) / 2.0f,
		                  slider->y + (slider->height - gui_label_get_height(&slider->label)) / 2.0f);
}

void gui_slider_set_pos(struct gui_slider *slider, int32_t x, int32_t y)
{
	if (slider->x == x && slider->y == y)
		return;
	slider->x = x;
	slider->y = y;
	gui_sprite_set_pos(&slider->bg_left, slider->x, slider->y);
	gui_sprite_set_pos(&slider->bg_right, slider->x + slider->width / 2, slider->y);
	gui_sprite_set_pos(&slider->button_left, slider->x + (slider->width - 8) * slider->per, slider->y);
	gui_sprite_set_pos(&slider->button_right, slider->x + (slider->width - 8) * slider->per + 4, slider->y);
	gui_label_set_pos(&slider->label,
	                  slider->x + (slider->width - gui_label_get_width(&slider->label)) / 2.0f,
	                  slider->y + (slider->height - gui_label_get_height(&slider->label)) / 2.0f);
}

void gui_slider_set_size(struct gui_slider *slider, int32_t width, int32_t height)
{
	if (slider->width == width && slider->height == height)
		return;
	slider->width = width;
	slider->height = height;
	gui_sprite_set_tex_size(&slider->bg_left, slider->width / 2 / 256.0f, slider->height / 256.0f);
	gui_sprite_set_tex_size(&slider->bg_right, slider->width / 2 / 256.0f, slider->height / 256.0f);
	gui_sprite_set_size(&slider->bg_left, slider->width / 2, slider->height);
	gui_sprite_set_size(&slider->bg_right, slider->width / 2, slider->height);
	gui_sprite_set_pos(&slider->bg_right, slider->x + slider->width / 2, slider->y);
	gui_sprite_set_tex_pos(&slider->bg_right, (200 - slider->width / 2) / 256.0f, 0);
	gui_sprite_set_size(&slider->button_left, 4, slider->height);
	gui_sprite_set_size(&slider->button_right, 4, slider->height);
	gui_sprite_set_pos(&slider->button_left, slider->x + (slider->width - 8) * slider->per, slider->y);
	gui_sprite_set_pos(&slider->button_right, slider->x + (slider->width - 8) * slider->per + 4, slider->y);
	gui_label_set_pos(&slider->label,
	                  slider->x + (slider->width - gui_label_get_width(&slider->label)) / 2.0f,
	                  slider->y + (slider->height - gui_label_get_height(&slider->label)) / 2.0f);
}

void gui_slider_set_disabled(struct gui_slider *slider, bool disabled)
{
	slider->disabled = disabled;
}

void gui_slider_set_callback(struct gui_slider *slider, gui_slider_cb_t cb,
                             void *userdata)
{
	slider->callback = cb;
	slider->callback_data = userdata;
}
