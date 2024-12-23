#ifndef GUI_BUTTON_H
#define GUI_BUTTON_H

#include "gui/sprite.h"
#include "gui/label.h"

struct gfx_pointer_event;
struct gfx_mouse_event;

typedef void (*gui_button_cb_t)(void *userdata);

struct gui_button
{
	struct gui_sprite bg_right;
	struct gui_sprite bg_left;
	struct gui_label label;
	gui_button_cb_t callback;
	int32_t height;
	int32_t width;
	int32_t x;
	int32_t y;
	void *callback_data;
	bool disabled;
	bool hover;
};

void gui_button_init(struct gui_button *button);
void gui_button_destroy(struct gui_button *button);
void gui_button_draw(struct gui_button *button);
void gui_button_mouse_move(struct gui_button *button, struct gfx_pointer_event *event);
void gui_button_mouse_down(struct gui_button *button, struct gfx_mouse_event *event);
void gui_button_set_text(struct gui_button *button, const char *text);
void gui_button_set_pos(struct gui_button *button, int32_t x, int32_t y);
void gui_button_set_size(struct gui_button *button, int32_t width, int32_t height);
void gui_button_set_disabled(struct gui_button *button, bool disabled);
void gui_button_set_callback(struct gui_button *button, gui_button_cb_t cb,
                             void *userdata);

#endif
