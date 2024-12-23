#ifndef GUI_TITLE_H
#define GUI_TITLE_H

#include "gui/button.h"

struct gui_title
{
	struct gui_sprite logo_left;
	struct gui_sprite logo_right;
	struct gui_button singleplayer;
	struct gui_button multiplayer;
	struct gui_button texture_packs;
	struct gui_button options;
	struct gui_button quit;
	struct gui_label version;
	struct gui_label license;
};

void gui_title_init(struct gui_title *title);
void gui_title_destroy(struct gui_title *title);
void gui_title_draw(struct gui_title *title);
void gui_title_mouse_move(struct gui_title *title,
                          struct gfx_pointer_event *event);
void gui_title_mouse_down(struct gui_title *title,
                          struct gfx_mouse_event *event);
void gui_title_mouse_up(struct gui_title *title,
                        struct gfx_mouse_event *event);

#endif
