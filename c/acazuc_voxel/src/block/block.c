#include "block/blocks.h"
#include "block/block.h"
#include "log.h"

#include <stdlib.h>
#include <assert.h>

static void get_destroy_values(struct block_def *def, struct vec2f *uv, struct vec4f *color)
{
	(void)def;
	VEC2_SET(*uv, 0, 0);
	VEC4_SET(*color, 1, 1, 1, 1);
}

static const struct block_def_vtable default_vtable =
{
	.get_destroy_values = get_destroy_values,
};

bool block_def_init(struct blocks_def *blocks, struct block_def *block, uint16_t id, const char *name)
{
	assert(id <= sizeof(blocks->blocks) / sizeof(*blocks->blocks));
	if (blocks->blocks[id])
	{
		LOG_ERROR("block %u alrady exists (replacing %s by %s)", id, blocks->blocks[id]->name, name);
		return false;
	}
	blocks->blocks[id] = block;
	block->vtable = &default_vtable;
	block->name = name;
	block->opacity = 15;
	block->light = 0;
	block->layer = 0;
	block->id = id;
	block->resistance = 0;
	block->hardness = 0;
	block->flags = BLOCK_FLAG_FOCUSABLE | BLOCK_FLAG_FULL_CUBE | BLOCK_FLAG_SOLID;
	block->tool = TOOL_NONE;
	VEC3_SETV(block->aabb.p0, 0);
	VEC3_SETV(block->aabb.p1, 1);
	return true;
}

void block_def_delete(struct block_def *block)
{
	/* XXX maybe a vtable destructor wil be needed at some point */
	free(block);
}
