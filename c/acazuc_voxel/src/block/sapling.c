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
	struct block_sapling *sapling = (struct block_sapling*)def;
	*uv = sapling->tex;
	VEC4_SET(*color, 1, 1, 1, 1);
}

static void draw(struct block_def *def, struct chunk *chunk, struct vec3f pos, struct tessellator *tessellator, uint8_t visible_faces, float *lights)
{
	struct block_sapling *sapling = (struct block_sapling*)def;
	(void)visible_faces;
	(void)lights;
	struct vec3f color;
	struct vec3f org;
	struct vec3f dst;
	struct vec2f tex_org;
	struct vec2f tex_dst;
	float diff = (1 - 0.707) / 2 * BLOCK_SIZE;
	VEC3_SET(org, pos.x + diff, pos.y, pos.z + diff);
	VEC3_SET(dst, pos.x + BLOCK_SIZE - diff, pos.y + BLOCK_SIZE, pos.z + BLOCK_SIZE - diff);
	VEC2_CPY(tex_org, sapling->tex);
	VEC2_ADDV(tex_dst, tex_org, tex_size);
	float light_value = voxel_light_value(chunk_get_light(chunk, pos.x - chunk->x, pos.y, pos.z - chunk->z));
	VEC3_SETV(color, light_value);
	{
		struct vec3f forg = {org.x, org.y, org.z};
		struct vec3f fdst = {dst.x, dst.y, dst.z};
		struct vec3f colors[] = {color, color, color, color};
		add_face_diag(tessellator, forg, fdst, tex_org, tex_dst, colors);
	}
	{
		struct vec3f forg = {org.x, org.y, dst.z};
		struct vec3f fdst = {dst.x, dst.y, org.z};
		struct vec3f colors[] = {color, color, color, color};
		add_face_diag(tessellator, forg, fdst, tex_org, tex_dst, colors);
	}
}

static const struct block_def_vtable sapling_vtable =
{
	.draw = draw,
	.get_destroy_values = get_destroy_values,
};

struct block_sapling *block_sapling_new(struct blocks_def *blocks, uint16_t id, const char *name)
{
	struct block_sapling *sapling = malloc(sizeof(*sapling));
	if (!sapling)
	{
		LOG_ERROR("allocation failed");
		return NULL;
	}
	if (!block_def_init(blocks, &sapling->block, id, name))
	{
		free(sapling);
		return NULL;
	}
	sapling->block.vtable = &sapling_vtable;
	VEC3_SET(sapling->block.aabb.p0, 0.1, 0.0, 0.1);
	VEC3_SET(sapling->block.aabb.p1, 0.9, 0.8, 0.9);
	sapling->block.hardness = 0;
	sapling->block.flags |= BLOCK_FLAG_TRANSPARENT | BLOCK_FLAG_REPLACEABLE;
	sapling->block.flags &= ~(BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_CUBE);
	sapling->block.opacity = 0;
	sapling->block.layer = 1;
	return sapling;
}
