#include "block/render.h"
#include "block/block.h"

#include "world/tessellator.h"
#include "world/block.h"

#include <stdlib.h>

static const float tex_size = 1.0 / 16;

void block_base_get_destroy_values(struct block_def *def, struct vec2f *uv, struct vec4f *color)
{
	struct block_base *base = (struct block_base*)def;
	switch (rand() / (RAND_MAX / 6))
	{
		case 0:
			*uv = base->tex_left;
			break;
		case 1:
			*uv = base->tex_right;
			break;
		case 2:
			*uv = base->tex_front;
			break;
		case 3:
			*uv = base->tex_back;
			break;
		case 4:
			*uv = base->tex_top;
			break;
		case 5:
			*uv = base->tex_bottom;
			break;
	}
	VEC4_SET(*color, 1, 1, 1, 1);
}

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

static const struct block_def_vtable base_vtable =
{
	.draw = draw,
	.get_destroy_values = block_base_get_destroy_values,
};

bool block_base_init(struct blocks_def *blocks, struct block_base *base, uint16_t id, const char *name)
{
	if (!block_def_init(blocks, &base->block, id, name))
		return false;
	base->block.vtable = &base_vtable;
	block_base_set_tex(base, 0, 0);
	return true;
}

struct block_base *block_base_new(struct blocks_def *blocks, uint16_t id, const char *name)
{
	struct block_base *base = malloc(sizeof(*base));
	if (!base)
	{
		LOG_ERROR("allocation failed");
		return NULL;
	}
	if (!block_base_init(blocks, base, id, name))
	{
		free(base);
		return NULL;
	}
	return base;
}

void block_base_set_tex_side(struct block_base *base, float tex_side_x, float tex_side_y)
{
	block_base_set_tex_side_x(base, tex_side_x);
	block_base_set_tex_side_y(base, tex_side_y);
}

void block_base_set_tex_side_x(struct block_base *base, float tex_side_x)
{
	block_base_set_tex_front_x(base, tex_side_x);
	block_base_set_tex_right_x(base, tex_side_x);
	block_base_set_tex_back_x(base, tex_side_x);
	block_base_set_tex_left_x(base, tex_side_x);
}

void block_base_set_tex_side_y(struct block_base *base, float tex_side_y)
{
	block_base_set_tex_front_y(base, tex_side_y);
	block_base_set_tex_right_y(base, tex_side_y);
	block_base_set_tex_back_y(base, tex_side_y);
	block_base_set_tex_left_y(base, tex_side_y);
}

void block_base_set_tex_front(struct block_base *base, float tex_front_x, float tex_front_y)
{
	block_base_set_tex_front_x(base, tex_front_x);
	block_base_set_tex_front_y(base, tex_front_y);
}

void block_base_set_tex_front_x(struct block_base *base, float tex_front_x)
{
	base->tex_front.x = tex_front_x;
}

void block_base_set_tex_front_y(struct block_base *base, float tex_front_y)
{
	base->tex_front.y = tex_front_y;
}

void block_base_set_tex_right(struct block_base *base, float tex_right_x, float tex_right_y)
{
	block_base_set_tex_right_x(base, tex_right_x);
	block_base_set_tex_right_y(base, tex_right_y);
}

void block_base_set_tex_right_x(struct block_base *base, float tex_right_x)
{
	base->tex_right.x = tex_right_x;
}

void block_base_set_tex_right_y(struct block_base *base, float tex_right_y)
{
	base->tex_right.y = tex_right_y;
}

void block_base_set_tex_back(struct block_base *base, float tex_back_x, float tex_back_y)
{
	block_base_set_tex_back_x(base, tex_back_x);
	block_base_set_tex_back_y(base, tex_back_y);
}

void block_base_set_tex_back_x(struct block_base *base, float tex_back_x)
{
	base->tex_back.x = tex_back_x;
}

void block_base_set_tex_back_y(struct block_base *base, float tex_back_y)
{
	base->tex_back.y = tex_back_y;
}

void block_base_set_tex_left(struct block_base *base, float tex_left_x, float tex_left_y)
{
	block_base_set_tex_left_x(base, tex_left_x);
	block_base_set_tex_left_y(base, tex_left_y);
}

void block_base_set_tex_left_x(struct block_base *base, float tex_left_x)
{
	base->tex_left.x = tex_left_x;
}

void block_base_set_tex_left_y(struct block_base *base, float tex_left_y)
{
	base->tex_left.y = tex_left_y;
}

void block_base_set_tex_top(struct block_base *base, float tex_top_x, float tex_top_y)
{
	block_base_set_tex_top_x(base, tex_top_x);
	block_base_set_tex_top_y(base, tex_top_y);
}

void block_base_set_tex_top_x(struct block_base *base, float tex_top_x)
{
	base->tex_top.x = tex_top_x;
}

void block_base_set_tex_top_y(struct block_base *base, float tex_top_y)
{
	base->tex_top.y = tex_top_y;
}

void block_base_set_tex_bot(struct block_base *base, float tex_bot_x, float tex_bot_y)
{
	block_base_set_tex_bot_x(base, tex_bot_x);
	block_base_set_tex_bot_y(base, tex_bot_y);
}

void block_base_set_tex_bot_x(struct block_base *base, float tex_bot_x)
{
	base->tex_bottom.x = tex_bot_x;
}

void block_base_set_tex_bot_y(struct block_base *base, float tex_bot_y)
{
	base->tex_bottom.y = tex_bot_y;
}

void block_base_set_tex(struct block_base *base, float tex_x, float tex_y)
{
	block_base_set_tex_x(base, tex_x);
	block_base_set_tex_y(base, tex_y);
}

void block_base_set_tex_x(struct block_base *base, float tex_x)
{
	block_base_set_tex_side_x(base, tex_x);
	block_base_set_tex_top_x(base, tex_x);
	block_base_set_tex_bot_x(base, tex_x);
}

void block_base_set_tex_y(struct block_base *base, float tex_y)
{
	block_base_set_tex_side_y(base, tex_y);
	block_base_set_tex_top_y(base, tex_y);
	block_base_set_tex_bot_y(base, tex_y);
}
