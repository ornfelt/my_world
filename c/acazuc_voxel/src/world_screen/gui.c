#include "world_screen/world_screen.h"
#include "world_screen/gui.h"

#include "player/player.h"

#include "world/world.h"
#include "world/block.h"

#include "block/blocks.h"
#include "block/block.h"

#include "gui/gui.h"

#include "textures.h"
#include "graphics.h"
#include "voxel.h"

#include <gfx/device.h>
#include <gfx/events.h>

#include <limits.h>
#include <stdio.h>

void world_screen_gui_init(struct world_screen_gui *gui,
                           struct world_screen *world_screen)
{
	gui->world_screen = world_screen;
	gui_inventory_init(&gui->inventory);
	gui_crafting_init(&gui->crafting);
	gui_hearts_init(&gui->hearts);
	gui_sprite_init(&gui->water);
	gui_sprite_set_texture(&gui->water, &g_voxel->textures->water);
	gui_label_init(&gui->focused_label);
	gui_label_init(&gui->light_label);
	gui_label_init(&gui->fps_label);
	gui_label_init(&gui->pos_label);
	gui_cross_init(&gui->cross);
	gui_bar_init(&gui->bar);
	gui->last_chunk_updates = UINT64_MAX;
	gui->last_fps = UINT64_MAX;
	gui->state = WORLD_SCREEN_GUI_NONE;
#if 0
	this->water.setTexture(Gui::getWaterTex());
	this->water.setProgram(Gui::getShaderSpriteProgram());
	Color color(.825, .5);
	this->water.setColor(color);
#endif
}

void world_screen_gui_destroy(struct world_screen_gui *gui)
{
	gui_inventory_destroy(&gui->inventory);
	gui_crafting_destroy(&gui->crafting);
	gui_hearts_destroy(&gui->hearts);
	gui_sprite_destroy(&gui->water);
	gui_label_destroy(&gui->focused_label);
	gui_label_destroy(&gui->light_label);
	gui_label_destroy(&gui->fps_label);
	gui_label_destroy(&gui->pos_label);
	gui_cross_destroy(&gui->cross);
	gui_bar_destroy(&gui->bar);
}

void world_screen_gui_draw(struct world_screen_gui *gui)
{
	gfx_bind_pipeline_state(g_voxel->device,
	                        &g_voxel->graphics->gui.pipeline_state);
	if (g_voxel->world->player->eye_in_water)
	{
		uint8_t light = g_voxel->world->player->eye_light;
		float color = voxel_light_value(light);
		float alpha = 0.25f - 0.25f * ((15 - light) / 15.0f);
		gui_sprite_set_color(&gui->water, color, color, color, alpha);
		gui_sprite_set_size(&gui->water, g_voxel->gui->width, g_voxel->gui->height);
		gui_sprite_draw(&gui->water);
	}
	gui_hearts_draw(&gui->hearts);
	gui_cross_draw(&gui->cross);
	char text[128];
	if (gui->last_fps != g_voxel->fps
	 || gui->last_chunk_updates != g_voxel->chunk_updates)
	{
		gui->last_fps = g_voxel->fps;
		gui->last_chunk_updates = g_voxel->chunk_updates;
		snprintf(text, sizeof(text), "voxel (%lu fps, %lu chunk updates)",
		         (unsigned long)g_voxel->fps,
		         (unsigned long)g_voxel->chunk_updates);
		gui_label_set_text(&gui->fps_label, text);
	}
	gui_label_draw(&gui->fps_label);
	gui_label_set_pos(&gui->pos_label, 0, 12);
	struct vec3f player_pos = g_voxel->world->player->entity.pos;
	snprintf(text, sizeof(text), "x: %f\ny: %f\nz: %f",
	         player_pos.x, player_pos.y, player_pos.z);
	gui_label_draw(&gui->pos_label);
	gui_label_set_pos(&gui->focused_label, 0, 46);
	const char *focused_name = "none";
	if (g_voxel->world->player->raycast.found)
	{
		struct vec3f pos = g_voxel->world->player->raycast.pos;
		struct block block;
		if (world_get_block(g_voxel->world, pos.x, pos.y, pos.z, &block))
		{
			struct block_def *def = g_voxel->blocks->blocks[block.type];
			if (def)
				focused_name = def->name;
		}
	}
	snprintf(text, sizeof(text), "focused: %s", focused_name);
	gui_label_set_text(&gui->focused_label, text);
	gui_label_draw(&gui->focused_label);
	gui_label_set_pos(&gui->light_label, 0, 58);
	snprintf(text, sizeof(text), "light: %d",
	         world_get_light(g_voxel->world, player_pos.x, player_pos.y, player_pos.z));
	gui_label_set_text(&gui->light_label, text);
	gui_label_draw(&gui->light_label);
	gui_bar_draw(&gui->bar);
	switch (gui->state)
	{
		case WORLD_SCREEN_GUI_NONE:
			break;
		case WORLD_SCREEN_GUI_INVENTORY:
			gui_inventory_draw(&gui->inventory);
			break;
		case WORLD_SCREEN_GUI_CRAFTING:
			gui_crafting_draw(&gui->crafting);
			break;
	}
}

void world_screen_gui_mouse_move(struct world_screen_gui *gui,
                                 struct gfx_pointer_event *event)
{
	switch (gui->state)
	{
		case WORLD_SCREEN_GUI_NONE:
			break;
		case WORLD_SCREEN_GUI_INVENTORY:
			gui_inventory_mouse_move(&gui->inventory, event);
			break;
		case WORLD_SCREEN_GUI_CRAFTING:
			gui_crafting_mouse_move(&gui->crafting, event);
			break;
	}
}

void world_screen_gui_mouse_scroll(struct world_screen_gui *gui,
                                   struct gfx_scroll_event *event)
{
	(void)gui;
	int8_t *bar_pos = &g_voxel->world->player->inventory.bar_pos;
	*bar_pos = (*bar_pos - event->y) % 9;
	if (*bar_pos < 0)
		*bar_pos += 9;
}

void world_screen_gui_key_down(struct world_screen_gui *gui,
                               struct gfx_key_event *event)
{
	if (event->used)
		return;
	if (event->key == GFX_KEY_E)
	{
		if (gui->state != WORLD_SCREEN_GUI_NONE)
			world_screen_gui_set_state(gui, WORLD_SCREEN_GUI_NONE);
		else
			world_screen_gui_set_state(gui, WORLD_SCREEN_GUI_INVENTORY);
		event->used = true;
	}
	else if (event->key == GFX_KEY_ESCAPE)
	{
		if (gui->state != WORLD_SCREEN_GUI_NONE)
		{
			world_screen_gui_set_state(gui, WORLD_SCREEN_GUI_NONE);
			event->used = true;
		}
	}
}

void world_screen_gui_set_state(struct world_screen_gui *gui,
                                enum world_screen_gui_state state)
{
	if (gui->state == state)
		return;
	gui->state = state;
	if (state == WORLD_SCREEN_GUI_NONE)
		voxel_grab_cursor(g_voxel);
	else
		voxel_ungrab_cursor(g_voxel);
}
