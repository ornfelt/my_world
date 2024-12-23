#ifndef GUI_SINGLEPLAYER_ENTRY_H
#define GUI_SINGLEPLAYER_ENTRY_H

#include "gui/sprite.h"
#include "gui/label.h"

struct gfx_pointer_event;
struct gui_singleplayer;
struct gfx_mouse_event;

struct gui_singleplayer_entry
{
	struct gui_singleplayer *parent;
	struct gui_sprite background;
	struct gui_sprite border;
	struct gui_sprite image;
	struct gui_label dir_date;
	struct gui_label other;
	struct gui_label name;
	int32_t pos;
	bool hover;
};

struct gui_singleplayer_entry *gui_singleplayer_entry_new(struct gui_singleplayer *parent,
                                                          int32_t pos,
                                                          const char *name);
void gui_singleplayer_entry_delete(struct gui_singleplayer_entry *entry);
void gui_singleplayer_entry_draw(struct gui_singleplayer_entry *entry);
void gui_singleplayer_entry_mouse_move(struct gui_singleplayer_entry *entry,
                                       struct gfx_pointer_event *event);
void gui_singleplayer_entry_mouse_down(struct gui_singleplayer_entry *entry,
                                       struct gfx_mouse_event *event);

#endif
