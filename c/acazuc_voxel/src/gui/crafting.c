#include "gui/crafting.h"
#include "gui/gui.h"

#include "textures.h"
#include "voxel.h"

void gui_crafting_init(struct gui_crafting *crafting)
{
	gui_sprite_init(&crafting->background);
	gui_sprite_set_texture(&crafting->background, &g_voxel->textures->crafting);
	gui_sprite_set_size(&crafting->background, 176, 166);
	gui_sprite_set_tex_pos(&crafting->background, 0, 0);
	gui_sprite_set_tex_size(&crafting->background, 176 / 256.0, 166 / 256.0);
	gui_label_init(&crafting->label);
	gui_label_set_text(&crafting->label, "Crafting");
	gui_label_set_color(&crafting->label, '0');
	gui_label_set_shadow(&crafting->label, false);
	for (int i = 0; i < 9; ++i)
		gui_item_slot_init(&crafting->crafting_slots[i]);
	gui_item_slot_init(&crafting->result_slot);
	for (int i = 0; i < 36; ++i)
		gui_item_slot_init(&crafting->inventory_slots[i]);
}

void gui_crafting_destroy(struct gui_crafting *crafting)
{
	for (int i = 0; i < 9; ++i)
		gui_item_slot_destroy(&crafting->crafting_slots[i]);
	gui_item_slot_destroy(&crafting->result_slot);
	for (int i = 0; i < 36; ++i)
		gui_item_slot_destroy(&crafting->inventory_slots[i]);
	gui_sprite_destroy(&crafting->background);
	gui_label_destroy(&crafting->label);
}

void gui_crafting_draw(struct gui_crafting *crafting)
{
	int32_t x = (g_voxel->gui->width - 176) / 2;
	int32_t y = (g_voxel->gui->height - 166) / 2;
	gui_sprite_set_pos(&crafting->background, x, y);
	gui_sprite_draw(&crafting->background);
	gui_label_set_pos(&crafting->label, x + 27, y + 7);
	gui_label_draw(&crafting->label);
	for (int i = 0; i < 9; ++i)
	{
		gui_item_slot_set_pos(&crafting->crafting_slots[i],
		                      x + 30 + (i % 3) * 18,
		                      y + 17 + (i / 3) * 18);
		gui_item_slot_draw(&crafting->crafting_slots[i]);
	}
	gui_item_slot_set_pos(&crafting->result_slot, x + 124, y + 35);
	gui_item_slot_draw(&crafting->result_slot);
	for (int i = 0; i < 36; ++i)
	{
		gui_item_slot_set_pos(&crafting->inventory_slots[i],
		                      x + 8 + (i % 9) * 18,
		                      y + 84 + (i / 9) * 18 + (i / 27) * 4);
		gui_item_slot_draw(&crafting->inventory_slots[i]);
	}
}

void gui_crafting_mouse_move(struct gui_crafting *crafting,
                             struct gfx_pointer_event *event)
{
	for (int i = 0; i < 9; ++i)
		gui_item_slot_mouse_move(&crafting->crafting_slots[i], event);
	gui_item_slot_mouse_move(&crafting->result_slot, event);
	for (int i = 0; i < 36; ++i)
		gui_item_slot_mouse_move(&crafting->inventory_slots[i], event);
}
