#ifndef BLOCK_BLOCK_H
#define BLOCK_BLOCK_H

#include "item/tool_type.h"

#include <jks/aabb.h>
#include <jks/vec2.h>

struct tessellator;
struct blocks_def;
struct block_def;
struct chunk;

enum block_flag
{
	BLOCK_FLAG_SOLID                = (1 << 0),
	BLOCK_FLAG_FULL_CUBE            = (1 << 1),
	BLOCK_FLAG_FOCUSABLE            = (1 << 2),
	BLOCK_FLAG_REPLACEABLE          = (1 << 3),
	BLOCK_FLAG_TRANSPARENT          = (1 << 4),
	BLOCK_FLAG_RENDER_SAME_NEIGHBOR = (1 << 5),
};

struct block_def_vtable
{
	void (*destroy)(struct block_def *def);
	void (*draw)(struct block_def *def, struct chunk *chunk, struct vec3f pos, struct tessellator *tessellator, uint8_t visible_faces, float *lights);
	void (*get_destroy_values)(struct block_def *def, struct vec2f *uv, struct vec4f *color);
	bool (*on_right_click)(struct block_def *def, struct chunk *chunk, struct vec3f pos);
};

struct block_def
{
	const struct block_def_vtable *vtable;
	const char *name;
	enum tool_type tool;
	struct aabb aabb;
	uint8_t opacity;
	uint8_t light;
	uint8_t layer;
	uint16_t id;
	enum block_flag flags;
	float resistance;
	float hardness;
};

struct block_base
{
	struct block_def block;
	struct vec2f tex_bottom;
	struct vec2f tex_front;
	struct vec2f tex_right;
	struct vec2f tex_back;
	struct vec2f tex_left;
	struct vec2f tex_top;
};

struct block_rail
{
	struct block_def block;
	struct vec2f tex;
};

struct block_liquid
{
	struct block_def block;
	struct vec2f tex;
};

struct block_redstone
{
	struct block_def block;
	struct vec2f tex;
};

struct block_sapling
{
	struct block_def block;
	struct vec2f tex;
};

struct block_torch
{
	struct block_def block;
	struct vec2f tex;
};

bool block_def_init(struct blocks_def *blocks, struct block_def *block, uint16_t id, const char *name);
void block_def_delete(struct block_def *block);
struct block_rail *block_rail_new(struct blocks_def *blocks, uint16_t id, const char *name);
struct block_liquid *block_water_new(struct blocks_def *blocks, uint16_t id, const char *name);
struct block_liquid *block_lava_new(struct blocks_def *blocks, uint16_t id, const char *name);
struct block_base *block_slab_new(struct blocks_def *blocks, uint16_t id, const char *name);
struct block_base *block_double_slab_new(struct blocks_def *blocks, uint16_t id, const char *name);
struct block_base *block_grass_new(struct blocks_def *blocks, uint16_t id, const char *name);
struct block_redstone *block_redstone_new(struct blocks_def *blocks, uint16_t id, const char *name);
struct block_base *block_leaves_new(struct blocks_def *blocks, uint16_t id, const char *name);
struct block_sapling *block_sapling_new(struct blocks_def *blocks, uint16_t id, const char *name);
struct block_torch *block_torch_new(struct blocks_def *blocks, uint16_t id, const char *name);
struct block_base *block_crafting_table_new(struct blocks_def *blocks, uint16_t id, const char *name);

bool block_base_init(struct blocks_def *blocks, struct block_base *base, uint16_t id, const char *name);
struct block_base *block_base_new(struct blocks_def *blocks, uint16_t id, const char *name);

void block_base_set_tex_side(struct block_base *base, float tex_side_x, float tex_side_y);
void block_base_set_tex_side_x(struct block_base *base, float tex_side_x);
void block_base_set_tex_side_y(struct block_base *base, float tex_side_y);
void block_base_set_tex_front(struct block_base *base, float tex_front_x, float tex_front_y);
void block_base_set_tex_front_x(struct block_base *base, float tex_front_x);
void block_base_set_tex_front_y(struct block_base *base, float tex_front_y);
void block_base_set_tex_right(struct block_base *base, float tex_right_x, float tex_right_y);
void block_base_set_tex_right_x(struct block_base *base, float tex_right_x);
void block_base_set_tex_right_y(struct block_base *base, float tex_right_y);
void block_base_set_tex_back(struct block_base *base, float tex_back_x, float tex_back_y);
void block_base_set_tex_back_x(struct block_base *base, float tex_back_x);
void block_base_set_tex_back_y(struct block_base *base, float tex_back_y);
void block_base_set_tex_left(struct block_base *base, float tex_left_x, float tex_left_y);
void block_base_set_tex_left_x(struct block_base *base, float tex_left_x);
void block_base_set_tex_left_y(struct block_base *base, float tex_left_y);
void block_base_set_tex_top(struct block_base *base, float tex_top_x, float tex_top_y);
void block_base_set_tex_top_x(struct block_base *base, float tex_top_x);
void block_base_set_tex_top_y(struct block_base *base, float tex_top_y);
void block_base_set_tex_bot(struct block_base *base, float tex_bot_x, float tex_bot_y);
void block_base_set_tex_bot_x(struct block_base *base, float tex_bot_x);
void block_base_set_tex_bot_y(struct block_base *base, float tex_bot_y);
void block_base_set_tex(struct block_base *base, float tex_x, float tex_y);
void block_base_set_tex_x(struct block_base *base, float tex_x);
void block_base_set_tex_y(struct block_base *base, float tex_y);

void block_base_get_destroy_values(struct block_def *def, struct vec2f *uv, struct vec4f *color);

#endif
