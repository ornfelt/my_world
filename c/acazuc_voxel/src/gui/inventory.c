#include "gui/inventory.h"
#include "gui/gui.h"

#include "textures.h"
#include "voxel.h"

void gui_inventory_init(struct gui_inventory *inventory)
{
	gui_sprite_init(&inventory->background);
	gui_sprite_set_texture(&inventory->background, &g_voxel->textures->inventory);
	gui_sprite_set_size(&inventory->background, 176, 166);
	gui_sprite_set_tex_pos(&inventory->background, 0, 0);
	gui_sprite_set_tex_size(&inventory->background, 176 / 256.0, 166 / 256.0);
	gui_label_init(&inventory->label);
	gui_label_set_text(&inventory->label, "Crafting");
	gui_label_set_color(&inventory->label, '0');
	gui_label_set_shadow(&inventory->label, false);
	for (int i = 0; i < 4; ++i)
		gui_item_slot_init(&inventory->armory_slots[i]);
	for (int i = 0; i < 9; ++i)
		gui_item_slot_init(&inventory->crafting_slots[i]);
	gui_item_slot_init(&inventory->result_slot);
	for (int i = 0; i < 36; ++i)
		gui_item_slot_init(&inventory->inventory_slots[i]);
}

void gui_inventory_destroy(struct gui_inventory *inventory)
{
	for (int i = 0; i < 4; ++i)
		gui_item_slot_destroy(&inventory->armory_slots[i]);
	for (int i = 0; i < 9; ++i)
		gui_item_slot_destroy(&inventory->crafting_slots[i]);
	gui_item_slot_destroy(&inventory->result_slot);
	for (int i = 0; i < 36; ++i)
		gui_item_slot_destroy(&inventory->inventory_slots[i]);
	gui_sprite_destroy(&inventory->background);
	gui_label_destroy(&inventory->label);
}

void gui_inventory_draw(struct gui_inventory *inventory)
{
	int32_t x = (g_voxel->gui->width - 176) / 2;
	int32_t y = (g_voxel->gui->height - 166) / 2;
	gui_sprite_set_pos(&inventory->background, x, y);
	gui_sprite_draw(&inventory->background);
	gui_label_set_pos(&inventory->label, x + 86, y + 16);
	gui_label_draw(&inventory->label);
	for (int i = 0; i < 4; ++i)
	{
		gui_item_slot_set_pos(&inventory->armory_slots[i],
		                      x + 8,
		                      y + 8 + i * 18);
		gui_item_slot_draw(&inventory->armory_slots[i]);
	}
	for (int i = 0; i < 4; ++i)
	{
		gui_item_slot_set_pos(&inventory->crafting_slots[i],
		                      x + 88 + (i % 2) * 18,
		                      y + 26 + (i / 2) * 18);
		gui_item_slot_draw(&inventory->crafting_slots[i]);
	}
	gui_item_slot_set_pos(&inventory->result_slot, x + 144, y + 36);
	gui_item_slot_draw(&inventory->result_slot);
	for (int i = 0; i < 36; ++i)
	{
		gui_item_slot_set_pos(&inventory->inventory_slots[i],
		                      x + 8 + (i % 9) * 18,
		                      y + 84 + (i / 9) * 18 + (i / 27) * 4);
		gui_item_slot_draw(&inventory->inventory_slots[i]);
	}
}

void gui_inventory_mouse_move(struct gui_inventory *inventory,
                              struct gfx_pointer_event *event)
{
	for (int i = 0; i < 4; ++i)
		gui_item_slot_mouse_move(&inventory->armory_slots[i], event);
	for (int i = 0; i < 4; ++i)
		gui_item_slot_mouse_move(&inventory->crafting_slots[i], event);
	gui_item_slot_mouse_move(&inventory->result_slot, event);
	for (int i = 0; i < 36; ++i)
		gui_item_slot_mouse_move(&inventory->inventory_slots[i], event);
}
