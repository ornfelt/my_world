#ifndef GUI_CRAFTING_H
#define GUI_CRAFTING_H

#include "gui/item_slot.h"
#include "gui/sprite.h"
#include "gui/label.h"

struct gfx_pointer_event;

struct gui_crafting
{
	struct gui_sprite background;
	struct gui_label label;
	struct gui_item_slot crafting_slots[9];
	struct gui_item_slot result_slot;
	struct gui_item_slot inventory_slots[36];
};

void gui_crafting_init(struct gui_crafting *crafting);
void gui_crafting_destroy(struct gui_crafting *crafting);
void gui_crafting_draw(struct gui_crafting *crafting);
void gui_crafting_mouse_move(struct gui_crafting *crafting,
                             struct gfx_pointer_event *event);

#endif
