#ifndef GUI_OPTIONS_H
#define GUI_OPTIONS_H

#include "gui/button.h"
#include "gui/slider.h"

struct gfx_pointer_event;
struct gfx_mouse_event;
struct gfx_key_event;

struct gui_options
{
	struct gui_button resources;
	struct gui_button controls;
	struct gui_button language;
	struct gui_button sound;
	struct gui_button video;
	struct gui_button chat;
	struct gui_button done;
	struct gui_button skin;
	struct gui_slider fov;
	struct gui_label title;
};

void gui_options_init(struct gui_options *options);
void gui_options_destroy(struct gui_options *options);
void gui_options_draw(struct gui_options *options);
void gui_options_mouse_move(struct gui_options *options,
                            struct gfx_pointer_event *event);
void gui_options_mouse_down(struct gui_options *options,
                            struct gfx_mouse_event *event);
void gui_options_mouse_up(struct gui_options *options,
                          struct gfx_mouse_event *event);
void gui_options_key_down(struct gui_options *options,
                          struct gfx_key_event *event);

#endif
