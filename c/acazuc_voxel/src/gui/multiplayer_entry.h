#ifndef GUI_NULTIPLAYER_ENTRY_H
#define GUI_MULTIPLAYER_ENTRY_H

#include <stdint.h>

struct gfx_pointer_event;
struct gfx_mouse_event;
struct gui_multiplayer;

struct gui_multiplayer_entry
{
	struct gui_multiplayer *parent;
	int32_t pos;
};

struct gui_multiplayer_entry *gui_multiplayer_entry_new(struct gui_multiplayer *parent, int32_t pos);
void gui_multiplayer_entry_delete(struct gui_multiplayer_entry *entry);
void gui_multiplayer_entry_draw(struct gui_multiplayer_entry *entry);
void gui_multiplayer_entry_mouse_move(struct gui_multiplayer_entry *entry,
                                      struct gfx_pointer_event *event);
void gui_multiplayer_entry_mouse_down(struct gui_multiplayer_entry *entry,
                                      struct gfx_mouse_event *event);

#endif
