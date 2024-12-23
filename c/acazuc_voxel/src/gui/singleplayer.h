#ifndef GUI_SINGLEPLAYER_H
#define GUI_SINGLEPLAYER_H

#include "gui/singleplayer_entry.h"
#include "gui/button.h"

#include <jks/array.h>

struct gfx_key_event;

struct gui_singleplayer
{
	struct jks_array entries;
	struct gui_sprite top_gradient;
	struct gui_sprite bot_gradient;
	struct gui_sprite background;
	struct gui_sprite bg_darker;
	struct gui_button recreate;
	struct gui_button cancel;
	struct gui_button remove;
	struct gui_button create;
	struct gui_button edit;
	struct gui_button play;
	int32_t selected;
};

void gui_singleplayer_init(struct gui_singleplayer *singleplayer);
void gui_singleplayer_destroy(struct gui_singleplayer *singleplayer);
void gui_singleplayer_draw(struct gui_singleplayer *singleplayer);
void gui_singleplayer_mouse_move(struct gui_singleplayer *singleplayer,
                                 struct gfx_pointer_event *event);
void gui_singleplayer_mouse_down(struct gui_singleplayer *singleplayer,
                                 struct gfx_mouse_event *event);
void gui_singleplayer_mouse_up(struct gui_singleplayer *singleplayer,
                               struct gfx_mouse_event *event);
void gui_singleplayer_key_down(struct gui_singleplayer *singleplayer,
                               struct gfx_key_event *event);

#endif
