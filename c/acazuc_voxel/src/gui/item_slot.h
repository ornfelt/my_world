#ifndef GUI_ITEM_SLOT_H
#define GUI_ITEM_SLOT_H

#include "gui/sprite.h"

struct gfx_pointer_event;

struct gui_item_slot
{
	struct gui_sprite hover;
	bool hovered;
};

void gui_item_slot_init(struct gui_item_slot *slot);
void gui_item_slot_destroy(struct gui_item_slot *slot);
void gui_item_slot_draw(struct gui_item_slot *slot);
void gui_item_slot_set_pos(struct gui_item_slot *slot, float x, float y);
void gui_item_slot_mouse_move(struct gui_item_slot *slot,
                              struct gfx_pointer_event *event);

#endif
