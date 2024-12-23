#include "gui/cross.h"
#include "gui/gui.h"

#include "textures.h"
#include "graphics.h"
#include "voxel.h"

#include <gfx/device.h>

void gui_cross_init(struct gui_cross *cross)
{
	gui_sprite_init(&cross->sprite);
	gui_sprite_set_texture(&cross->sprite, &g_voxel->textures->gui);
	gui_sprite_set_size(&cross->sprite, 15, 15);
	gui_sprite_set_tex_pos(&cross->sprite, 240 / 256.0f, 0);
	gui_sprite_set_tex_size(&cross->sprite, 15 / 256.0f, 15 / 256.0f);
}

void gui_cross_destroy(struct gui_cross *cross)
{
	gui_sprite_destroy(&cross->sprite);
}

void gui_cross_draw(struct gui_cross *cross)
{
	gfx_bind_pipeline_state(g_voxel->device,
	                        &g_voxel->graphics->gui.cross_pipeline_state);
	gui_sprite_set_pos(&cross->sprite,
	                   (g_voxel->gui->width - 15) / 2,
	                   (g_voxel->gui->height - 15) / 2);
	gui_sprite_draw(&cross->sprite);
	gfx_bind_pipeline_state(g_voxel->device,
	                        &g_voxel->graphics->gui.pipeline_state);
}
