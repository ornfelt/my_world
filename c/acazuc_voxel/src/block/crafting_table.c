#include "block/render.h"
#include "block/block.h"

#include "world/tessellator.h"
#include "world/block.h"

#include "world_screen/world_screen.h"
#include "world_screen/gui.h"

#include "voxel.h"

#include <stdlib.h>

static const float tex_size = 1.0 / 16;

static void draw(struct block_def *def, struct chunk *chunk, struct vec3f pos, struct tessellator *tessellator, uint8_t visible_faces, float *lights)
{
	struct block_base *base = (struct block_base*)def;
	(void)chunk;
	struct vec3f color;
	struct vec3f org;
	struct vec3f dst;
	struct vec2f tex_org;
	struct vec2f tex_dst;
	struct vec3f col;
	VEC3_SETV(color, 1);
	VEC3_CPY(org, pos);
	VEC3_ADDV(dst, pos, 1);
	if (visible_faces & BLOCK_FACE_FRONT)
	{
		VEC2_CPY(tex_org, base->tex_front);
		VEC2_ADDV(tex_dst, tex_org, tex_size);
		struct vec3f colors[4];
		VEC3_MULV(col, color, FRONT_COLOR_FACTOR);
		VEC3_MULV(colors[0], col, lights[F1P1]);
		VEC3_MULV(colors[1], col, lights[F1P2]);
		VEC3_MULV(colors[2], col, lights[F1P3]);
		VEC3_MULV(colors[3], col, lights[F1P4]);
		add_face_front(tessellator, org, dst, tex_org, tex_dst, colors, lights[F1P2] + lights[F1P4] > lights[F1P1] + lights[F1P3]);
	}
	if (visible_faces & BLOCK_FACE_BACK)
	{
		VEC2_CPY(tex_org, base->tex_left);
		VEC2_ADDV(tex_dst, tex_org, tex_size);
		struct vec3f colors[4];
		VEC3_MULV(col, color, BACK_COLOR_FACTOR);
		VEC3_MULV(colors[0], col, lights[F2P1]);
		VEC3_MULV(colors[1], col, lights[F2P2]);
		VEC3_MULV(colors[2], col, lights[F2P3]);
		VEC3_MULV(colors[3], col, lights[F2P4]);
		add_face_back(tessellator, org, dst, tex_org, tex_dst, colors, lights[F2P2] + lights[F2P4] <= lights[F2P1] + lights[F2P3]);
	}
	if (visible_faces & BLOCK_FACE_LEFT)
	{
		VEC2_CPY(tex_org, base->tex_left);
		VEC2_ADDV(tex_dst, tex_org, tex_size);
		struct vec3f colors[4];
		VEC3_MULV(col, color, LEFT_COLOR_FACTOR);
		VEC3_MULV(colors[0], col, lights[F3P1]);
		VEC3_MULV(colors[1], col, lights[F3P2]);
		VEC3_MULV(colors[2], col, lights[F3P3]);
		VEC3_MULV(colors[3], col, lights[F3P4]);
		add_face_left(tessellator, org, dst, tex_org, tex_dst, colors, lights[F3P2] + lights[F3P4] > lights[F3P1] + lights[F3P3]);
	}
	if (visible_faces & BLOCK_FACE_RIGHT)
	{
		VEC2_CPY(tex_org, base->tex_right);
		VEC2_ADDV(tex_dst, tex_org, tex_size);
		struct vec3f colors[4];
		VEC3_MULV(col, color, RIGHT_COLOR_FACTOR);
		VEC3_MULV(colors[0], col, lights[F4P1]);
		VEC3_MULV(colors[1], col, lights[F4P2]);
		VEC3_MULV(colors[2], col, lights[F4P3]);
		VEC3_MULV(colors[3], col, lights[F4P4]);
		add_face_right(tessellator, org, dst, tex_org, tex_dst, colors, lights[F4P2] + lights[F4P4] < lights[F4P1] + lights[F4P3]);
	}
	if (visible_faces & BLOCK_FACE_TOP)
	{
		VEC2_CPY(tex_org, base->tex_top);
		VEC2_ADDV(tex_dst, tex_org, tex_size);
		struct vec3f colors[4];
		VEC3_MULV(col, color, TOP_COLOR_FACTOR);
		VEC3_MULV(colors[0], col, lights[F5P1]);
		VEC3_MULV(colors[1], col, lights[F5P2]);
		VEC3_MULV(colors[2], col, lights[F5P3]);
		VEC3_MULV(colors[3], col, lights[F5P4]);
		add_face_up(tessellator, org, dst, tex_org, tex_dst, colors, lights[F5P2] + lights[F5P4] > lights[F5P1] + lights[F5P3]);
	}
	if (visible_faces & BLOCK_FACE_BOTTOM)
	{
		VEC2_CPY(tex_org, base->tex_bottom);
		VEC2_ADDV(tex_dst, tex_org, tex_size);
		struct vec3f colors[4];
		VEC3_MULV(col, color, BOTTOM_COLOR_FACTOR);
		VEC3_MULV(colors[0], col, lights[F6P1]);
		VEC3_MULV(colors[1], col, lights[F6P2]);
		VEC3_MULV(colors[2], col, lights[F6P3]);
		VEC3_MULV(colors[3], col, lights[F6P4]);
		add_face_down(tessellator, org, dst, tex_org, tex_dst, colors, lights[F6P2] + lights[F6P4] < lights[F6P1] + lights[F6P3]);
	}
}

static bool on_right_click(struct block_def *def, struct chunk *chunk, struct vec3f pos)
{
	struct world_screen_gui *gui = &((struct world_screen*)g_voxel->screen)->gui;
	world_screen_gui_set_state(gui, WORLD_SCREEN_GUI_CRAFTING);
	return true;
}

static const struct block_def_vtable crafting_table_vtable =
{
	.draw = draw,
	.get_destroy_values = block_base_get_destroy_values,
	.on_right_click = on_right_click,
};

struct block_base *block_crafting_table_new(struct blocks_def *blocks, uint16_t id, const char *name)
{
	struct block_base *crafting_table = block_base_new(blocks, id, name);
	if (!crafting_table)
		return NULL;
	crafting_table->block.vtable = &crafting_table_vtable;
	return crafting_table;
}
