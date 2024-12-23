#include "gui/item_slot.h"
#include "gui/gui.h"

#include "textures.h"
#include "voxel.h"

#include <gfx/events.h>

void gui_item_slot_init(struct gui_item_slot *slot)
{
	gui_sprite_init(&slot->hover);
	gui_sprite_set_texture(&slot->hover, &g_voxel->textures->whitepixel);
	gui_sprite_set_size(&slot->hover, 16, 16);
	gui_sprite_set_color(&slot->hover, 1, 1, 1, 0.25);
	slot->hovered = false;
}

void gui_item_slot_destroy(struct gui_item_slot *slot)
{
	gui_sprite_destroy(&slot->hover);
}

void gui_item_slot_draw(struct gui_item_slot *slot)
{
	if (slot->hovered)
		gui_sprite_draw(&slot->hover);
}

void gui_item_slot_set_pos(struct gui_item_slot *slot, float x, float y)
{
	gui_sprite_set_pos(&slot->hover, x, y);
}

void gui_item_slot_mouse_move(struct gui_item_slot *slot,
                              struct gfx_pointer_event *event)
{
	if (event->used)
		return;
	slot->hovered = (event->x >= slot->hover.x
	              && event->x < slot->hover.x + slot->hover.width
	              && event->y >= slot->hover.y
	              && event->y < slot->hover.y + slot->hover.height);
	if (slot->hovered)
		event->used = true;
}
