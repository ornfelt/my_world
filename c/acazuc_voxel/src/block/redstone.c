#include "block/render.h"
#include "block/block.h"

#include "world/tessellator.h"
#include "world/block.h"
#include "world/chunk.h"

#include "shaders.h"
#include "voxel.h"
#include "log.h"

static const float tex_size = 1.0 / 16;

static void get_destroy_values(struct block_def *def, struct vec2f *uv, struct vec4f *color)
{
	struct block_redstone *redstone = (struct block_redstone*)def;
	*uv = redstone->tex;
	VEC4_SET(*color, 1, 1, 1, 1);
}

static void draw(struct block_def *def, struct chunk *chunk, struct vec3f pos, struct tessellator *tessellator, uint8_t visible_faces, float *lights)
{
	struct block_redstone *redstone = (struct block_redstone*)def;
	(void)visible_faces;
	(void)lights;
	struct vec3f color;
	struct vec3f org;
	struct vec3f dst;
	struct vec2f tex_org;
	struct vec2f tex_dst;
	float light_value = voxel_light_value(chunk_get_light(chunk, pos.x - chunk->x, pos.y, pos.z - chunk->z));
	VEC3_SETV(color, light_value);
	VEC3_CPY(org, pos);
	org.y += 1.0 / 16;
	VEC3_CPY(dst, org);
	dst.x += BLOCK_SIZE;
	dst.z += BLOCK_SIZE;
	VEC2_CPY(tex_org, redstone->tex);
	VEC2_ADDV(tex_dst, tex_org, tex_size);
	struct vec3f colors[] = {color, color, color, color};
	add_face_up(tessellator, org, dst, tex_org, tex_dst, colors, 0);
}

static const struct block_def_vtable redstone_vtable =
{
	.draw = draw,
	.get_destroy_values = get_destroy_values,
};

struct block_redstone *block_redstone_new(struct blocks_def *blocks, uint16_t id, const char *name)
{
	struct block_redstone *redstone = malloc(sizeof(*redstone));
	if (!redstone)
	{
		LOG_ERROR("allocation failed");
		return NULL;
	}
	if (!block_def_init(blocks, &redstone->block, id, name))
	{
		free(redstone);
		return NULL;
	}
	redstone->block.vtable = &redstone_vtable;
	VEC3_SET(redstone->block.aabb.p0, 0, 0, 0);
	VEC3_SET(redstone->block.aabb.p1, 1, 2.0 / 16, 1);
	VEC2_SET(redstone->tex, 0, 0);
	return redstone;
}
