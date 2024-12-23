#include "gui/hearts.h"
#include "gui/gui.h"

#include "textures.h"
#include "voxel.h"

void gui_hearts_init(struct gui_hearts *hearts)
{
	for (int i = 0; i < 10; ++i)
	{
		gui_sprite_init(&hearts->backgrounds[i]);
		gui_sprite_set_texture(&hearts->backgrounds[i], &g_voxel->textures->icons);
		gui_sprite_set_size(&hearts->backgrounds[i], 9, 9);
		gui_sprite_set_tex_size(&hearts->backgrounds[i], 9 / 256.0f, 9 / 2556.0f);
		gui_sprite_set_tex_pos(&hearts->backgrounds[i], 16 / 256.0f, 0);
		gui_sprite_init(&hearts->hearts[i]);
		gui_sprite_set_texture(&hearts->hearts[i], &g_voxel->textures->icons);
		gui_sprite_set_size(&hearts->hearts[i], 9, 9);
		gui_sprite_set_tex_size(&hearts->hearts[i], 9 / 256.0f, 9 / 256.0f);
		gui_sprite_set_tex_pos(&hearts->hearts[i], 52 / 256.0f, 0);
	}
}

void gui_hearts_destroy(struct gui_hearts *hearts)
{
	for (int i = 0; i < 10; ++i)
	{
		gui_sprite_destroy(&hearts->backgrounds[i]);
		gui_sprite_destroy(&hearts->hearts[i]);
	}
}

void gui_hearts_draw(struct gui_hearts *hearts)
{
	int32_t x = (g_voxel->gui->width - 182) / 2;
	int32_t y = g_voxel->gui->height - (22 + 1 + 9);
	for (int i = 0; i < 10; ++i)
	{
		gui_sprite_set_pos(&hearts->backgrounds[i], x + 8 * i, y);
		gui_sprite_draw(&hearts->backgrounds[i]);
		gui_sprite_set_pos(&hearts->hearts[i], x + 8 * i, y);
		gui_sprite_draw(&hearts->hearts[i]);
	}
}
