#include "gui/bar.h"
#include "gui/gui.h"

#include "world_screen/world_screen.h"

#include "world/world.h"

#include "player/player.h"

#include "textures.h"
#include "voxel.h"

void gui_bar_init(struct gui_bar *bar)
{
	gui_sprite_init(&bar->background);
	gui_sprite_set_texture(&bar->background, &g_voxel->textures->gui);
	gui_sprite_set_size(&bar->background, 182, 22);
	gui_sprite_set_tex_size(&bar->background, 182 / 256.0f, 22 / 256.0f);
	gui_sprite_set_texture(&bar->selected, &g_voxel->textures->gui);
	gui_sprite_set_size(&bar->selected, 24, 24);
	gui_sprite_set_tex_pos(&bar->selected, 0, 22 / 256.0f);
	gui_sprite_set_tex_size(&bar->selected, 24 / 256.0f, 24 / 256.0f);
}

void gui_bar_destroy(struct gui_bar *bar)
{
	gui_sprite_destroy(&bar->background);
	gui_sprite_destroy(&bar->selected);
}

void gui_bar_draw(struct gui_bar *bar)
{
	gui_sprite_set_pos(&bar->background,
	                   (g_voxel->gui->width - 182) / 2,
	                   g_voxel->gui->height - 22);
	gui_sprite_draw(&bar->background);
	gui_sprite_set_pos(&bar->selected,
	                   (g_voxel->gui->width - 182) / 2 - 20 * g_voxel->world->player->inventory.bar_pos,
	                   g_voxel->gui->height - 23);
	gui_sprite_draw(&bar->selected);
}
