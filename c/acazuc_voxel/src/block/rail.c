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
	struct block_rail *rail = (struct block_rail*)def;
	*uv = rail->tex;
	VEC4_SET(*color, 1, 1, 1, 1);
}

static void draw(struct block_def *def, struct chunk *chunk, struct vec3f pos, struct tessellator *tessellator, uint8_t visible_faces, float *lights)
{
	struct block_rail *rail = (struct block_rail*)def;
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
	VEC2_CPY(tex_org, rail->tex);
	VEC2_ADDV(tex_dst, tex_org, tex_size);
	struct vec3f colors[] = {color, color, color, color};
	add_face_up(tessellator, org, dst, tex_org, tex_dst, colors, 0);
}

static const struct block_def_vtable rail_vtable =
{
	.draw = draw,
	.get_destroy_values = get_destroy_values,
};

struct block_rail *block_rail_new(struct blocks_def *blocks, uint16_t id, const char *name)
{
	struct block_rail *rail = malloc(sizeof(*rail));
	if (!rail)
	{
		LOG_ERROR("allocation failed");
		return NULL;
	}
	if (!block_def_init(blocks, &rail->block, id, name))
	{
		free(rail);
		return NULL;
	}
	rail->block.vtable = &rail_vtable;
	VEC3_SET(rail->block.aabb.p0, 0, 0, 0);
	VEC3_SET(rail->block.aabb.p1, 1, 2.0 / 16, 1);
	rail->block.opacity = 0;
	rail->block.flags |= BLOCK_FLAG_TRANSPARENT;
	VEC2_SET(rail->tex, 0, 0);
	return rail;
}