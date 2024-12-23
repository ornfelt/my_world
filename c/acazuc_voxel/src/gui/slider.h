#ifndef GUI_SLIDER_H
#define GUI_SLIDER_H

#include "gui/sprite.h"
#include "gui/label.h"

struct gfx_pointer_event;
struct gfx_mouse_event;

typedef void (*gui_slider_cb_t)(float percent, void *userdata);

struct gui_slider
{
	struct gui_sprite button_right;
	struct gui_sprite button_left;
	struct gui_sprite bg_right;
	struct gui_sprite bg_left;
	struct gui_label label;
	gui_slider_cb_t callback;
	int32_t height;
	int32_t width;
	int32_t x;
	int32_t y;
	float per;
	void *callback_data;
	bool disabled;
	bool clicked;
	bool hover;
};

void gui_slider_init(struct gui_slider *slider);
void gui_slider_destroy(struct gui_slider *slider);
void gui_slider_draw(struct gui_slider *slider);
void gui_slider_mouse_move(struct gui_slider *slider,
                           struct gfx_pointer_event *event);
void gui_slider_mouse_down(struct gui_slider *slider,
                           struct gfx_mouse_event *event);
void gui_slider_mouse_up(struct gui_slider *slider,
                         struct gfx_mouse_event *event);
void gui_slider_set_text(struct gui_slider *slider, const char *text);
void gui_slider_set_pos(struct gui_slider *slider, int32_t x, int32_t y);
void gui_slider_set_size(struct gui_slider *slider, int32_t width, int32_t height);
void gui_slider_set_disabled(struct gui_slider *slider, bool disabled);
void gui_slider_set_callback(struct gui_slider *slider, gui_slider_cb_t cb,
                             void *userdata);

#endif
