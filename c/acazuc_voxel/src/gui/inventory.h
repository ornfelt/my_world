#ifndef GUI_INVENTORY_H
#define GUI_INVENTORY_H

#include "gui/item_slot.h"
#include "gui/sprite.h"
#include "gui/label.h"

struct gfx_pointer_event;

struct gui_inventory
{
	struct gui_sprite background;
	struct gui_label label;
	struct gui_item_slot armory_slots[4];
	struct gui_item_slot crafting_slots[4];
	struct gui_item_slot result_slot;
	struct gui_item_slot inventory_slots[36];
};

void gui_inventory_init(struct gui_inventory *inventory);
void gui_inventory_destroy(struct gui_inventory *inventory);
void gui_inventory_draw(struct gui_inventory *inventory);
void gui_inventory_mouse_move(struct gui_inventory *inventory,
                              struct gfx_pointer_event *event);

#endif
