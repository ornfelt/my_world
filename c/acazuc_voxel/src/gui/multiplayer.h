#ifndef GUI_MULTIPLAYER_H
#define GUI_MULTIPLAYER_H

#include "gui/sprite.h"
#include "gui/button.h"

#include <jks/array.h>

struct gfx_pointer_event;
struct gfx_mouse_event;
struct gfx_key_event;

struct gui_multiplayer
{
	struct jks_array entries; /* struct gui_multiplayer_entry* */
	struct gui_sprite top_gradient;
	struct gui_sprite bot_gradient;
	struct gui_sprite background;
	struct gui_sprite bg_darker;
	struct gui_button refresh;
	struct gui_button direct;
	struct gui_button remove;
	struct gui_button cancel;
	struct gui_button join;
	struct gui_button edit;
	struct gui_button add;
	int32_t selected;
};

void gui_multiplayer_init(struct gui_multiplayer *multiplayer);
void gui_multiplayer_destroy(struct gui_multiplayer *multiplayer);
void gui_multiplayer_draw(struct gui_multiplayer *multiplayer);
void gui_multiplayer_mouse_move(struct gui_multiplayer *multiplayer,
                                struct gfx_pointer_event *event);
void gui_multiplayer_mouse_down(struct gui_multiplayer *multiplayer,
                                struct gfx_mouse_event *event);
void gui_multiplayer_mouse_up(struct gui_multiplayer *multiplayer,
                              struct gfx_mouse_event *event);
void gui_multiplayer_key_down(struct gui_multiplayer *multiplayer,
                              struct gfx_key_event *event);

#endif
