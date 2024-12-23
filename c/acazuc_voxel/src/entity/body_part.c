#include "entity/body_part.h"

#include "player/player.h"

#include "world/world.h"

#include "graphics.h"
#include "shaders.h"
#include "voxel.h"
#include "log.h"

#include <gfx/device.h>

#include <stdlib.h>
#include <math.h>

static const struct vec2f tex_size = {64, 32};

struct body_part *body_part_new(struct vec3f org, struct vec3f size, struct vec2f uv, struct vec3f pos)
{
	struct body_part *body_part = malloc(sizeof(*body_part));
	if (!body_part)
	{
		LOG_ERROR("body part allocation failed");
		return NULL;
	}
	VEC3_CPY(body_part->pos, pos);
	VEC3_SETV(body_part->rot, 0);
	struct vec3f dst;
	VEC3_ADD(dst, org, size);
	body_part->attributes_state = GFX_ATTRIBUTES_STATE_INIT();
	body_part->vertexes_buffer = GFX_BUFFER_INIT();
	body_part->indices_buffer = GFX_BUFFER_INIT();
	body_part->uniform_buffer = GFX_BUFFER_INIT();
	struct shader_entity_vertex vertexes[24];
	/* front */
	VEC3_SET(vertexes[0].position , org.x, org.y, dst.z);
	VEC3_SET(vertexes[1].position , dst.x, org.y, dst.z);
	VEC3_SET(vertexes[2].position , dst.x, dst.y, dst.z);
	VEC3_SET(vertexes[3].position , org.x, dst.y, dst.z);
	/* back */
	VEC3_SET(vertexes[4].position , org.x, org.y, org.z);
	VEC3_SET(vertexes[5].position , dst.x, org.y, org.z);
	VEC3_SET(vertexes[6].position , dst.x, dst.y, org.z);
	VEC3_SET(vertexes[7].position , org.x, dst.y, org.z);
	/* left */
	VEC3_SET(vertexes[8].position , org.x, org.y, org.z);
	VEC3_SET(vertexes[9].position , org.x, dst.y, org.z);
	VEC3_SET(vertexes[10].position, org.x, dst.y, dst.z);
	VEC3_SET(vertexes[11].position, org.x, org.y, dst.z);
	/* right */
	VEC3_SET(vertexes[12].position, dst.x, org.y, org.z);
	VEC3_SET(vertexes[13].position, dst.x, dst.y, org.z);
	VEC3_SET(vertexes[14].position, dst.x, dst.y, dst.z);
	VEC3_SET(vertexes[15].position, dst.x, org.y, dst.z);
	/* up */
	VEC3_SET(vertexes[16].position, org.x, dst.y, org.z);
	VEC3_SET(vertexes[17].position, dst.x, dst.y, org.z);
	VEC3_SET(vertexes[18].position, dst.x, dst.y, dst.z);
	VEC3_SET(vertexes[19].position, org.x, dst.y, dst.z);
	/* down */
	VEC3_SET(vertexes[20].position, org.x, org.y, org.z);
	VEC3_SET(vertexes[21].position, dst.x, org.y, org.z);
	VEC3_SET(vertexes[22].position, dst.x, org.y, dst.z);
	VEC3_SET(vertexes[23].position, org.x, org.y, dst.z);
	/* front */
	VEC2_SET(vertexes[0].uv, (uv.x + size.z         ) / tex_size.x, (uv.y + size.z + size.y) / tex_size.y);
	VEC2_SET(vertexes[1].uv, (uv.x + size.z + size.x) / tex_size.x, (uv.y + size.z + size.y) / tex_size.y);
	VEC2_SET(vertexes[2].uv, (uv.x + size.z + size.x) / tex_size.x, (uv.y + size.z         ) / tex_size.y);
	VEC2_SET(vertexes[3].uv, (uv.x + size.z         ) / tex_size.x, (uv.y + size.z         ) / tex_size.y);
	/* back */
	VEC2_SET(vertexes[4].uv, (uv.x + size.z * 2 + size.x * 2) / tex_size.x, (uv.y + size.z + size.y) / tex_size.y);
	VEC2_SET(vertexes[5].uv, (uv.x + size.z * 2 + size.x * 1) / tex_size.x, (uv.y + size.z + size.y) / tex_size.y);
	VEC2_SET(vertexes[6].uv, (uv.x + size.z * 2 + size.x * 1) / tex_size.x, (uv.y + size.z         ) / tex_size.y);
	VEC2_SET(vertexes[7].uv, (uv.x + size.z * 2 + size.x * 2) / tex_size.x, (uv.y + size.z         ) / tex_size.y);
	/* left */
	VEC2_SET(vertexes[8].uv, (uv.x         ) / tex_size.x, (uv.y + size.z + size.y) / tex_size.y);
	VEC2_SET(vertexes[9].uv, (uv.x         ) / tex_size.x, (uv.y + size.z         ) / tex_size.y);
	VEC2_SET(vertexes[10].uv, (uv.x + size.z) / tex_size.x, (uv.y + size.z         ) / tex_size.y);
	VEC2_SET(vertexes[11].uv, (uv.x + size.z) / tex_size.x, (uv.y + size.z + size.y) / tex_size.y);
	/* right */
	VEC2_SET(vertexes[12].uv, (uv.x + size.x + size.z * 2) / tex_size.x, (uv.y + size.z + size.y) / tex_size.y);
	VEC2_SET(vertexes[13].uv, (uv.x + size.x + size.z * 2) / tex_size.x, (uv.y + size.z         ) / tex_size.y);
	VEC2_SET(vertexes[14].uv, (uv.x + size.x + size.z * 1) / tex_size.x, (uv.y + size.z         ) / tex_size.y);
	VEC2_SET(vertexes[15].uv, (uv.x + size.x + size.z * 1) / tex_size.x, (uv.y + size.z + size.y) / tex_size.y);
	/* up */
	VEC2_SET(vertexes[16].uv, (uv.x + size.z         ) / tex_size.x, (uv.y         ) / tex_size.y);
	VEC2_SET(vertexes[17].uv, (uv.x + size.z + size.x) / tex_size.x, (uv.y         ) / tex_size.y);
	VEC2_SET(vertexes[18].uv, (uv.x + size.z + size.x) / tex_size.x, (uv.y + size.z) / tex_size.y);
	VEC2_SET(vertexes[19].uv, (uv.x + size.z         ) / tex_size.x, (uv.y + size.z) / tex_size.y);
	/* down */
	VEC2_SET(vertexes[20].uv, (uv.x + size.z + size.x * 1) / tex_size.x, (uv.y + size.z * 1) / tex_size.y);
	VEC2_SET(vertexes[21].uv, (uv.x + size.z + size.x * 2) / tex_size.x, (uv.y + size.z * 1) / tex_size.y);
	VEC2_SET(vertexes[22].uv, (uv.x + size.z + size.x * 2) / tex_size.x, (uv.y             ) / tex_size.y);
	VEC2_SET(vertexes[23].uv, (uv.x + size.z + size.x * 1) / tex_size.x, (uv.y             ) / tex_size.y);
	gfx_create_buffer(g_voxel->device, &body_part->vertexes_buffer, GFX_BUFFER_VERTEXES, vertexes, sizeof(vertexes), GFX_BUFFER_IMMUTABLE);
	static const uint16_t indices[36] = 
	{
		4 * 0 + 0, 4 * 0 + 1, 4 * 0 + 3, 4 * 0 + 2, 4 * 0 + 3, 4 * 0 + 1, /* front */
		4 * 1 + 0, 4 * 1 + 3, 4 * 1 + 1, 4 * 1 + 2, 4 * 1 + 1, 4 * 1 + 3, /* back */
		4 * 2 + 0, 4 * 2 + 3, 4 * 2 + 1, 4 * 2 + 2, 4 * 2 + 1, 4 * 2 + 3, /* left */
		4 * 3 + 0, 4 * 3 + 1, 4 * 3 + 3, 4 * 3 + 2, 4 * 3 + 3, 4 * 3 + 1, /* right */
		4 * 4 + 0, 4 * 4 + 3, 4 * 4 + 1, 4 * 4 + 2, 4 * 4 + 1, 4 * 4 + 3, /* up */
		4 * 5 + 0, 4 * 5 + 1, 4 * 5 + 3, 4 * 5 + 2, 4 * 5 + 3, 4 * 5 + 1, /* down */
	};
	gfx_create_buffer(g_voxel->device, &body_part->indices_buffer, GFX_BUFFER_INDICES, indices, sizeof(indices), GFX_BUFFER_IMMUTABLE);
	gfx_create_buffer(g_voxel->device, &body_part->uniform_buffer, GFX_BUFFER_UNIFORM, NULL, sizeof(struct shader_entity_mesh_block), GFX_BUFFER_DYNAMIC);
	struct gfx_attribute_bind binds[] =
	{
		{&body_part->vertexes_buffer},
	};
	gfx_create_attributes_state(g_voxel->device, &body_part->attributes_state, binds, sizeof(binds) / sizeof(*binds), &body_part->indices_buffer, GFX_INDEX_UINT16);
	return body_part;
}

void body_part_delete(struct body_part *body_part)
{
	if (!body_part)
		return;
	gfx_delete_buffer(g_voxel->device, &body_part->uniform_buffer);
	gfx_delete_buffer(g_voxel->device, &body_part->indices_buffer);
	gfx_delete_buffer(g_voxel->device, &body_part->vertexes_buffer);
	gfx_delete_attributes_state(g_voxel->device, &body_part->attributes_state);
	free(body_part);
}

void body_part_draw(struct body_part *body_part, struct world *world, const struct mat4f *model)
{
	struct shader_entity_mesh_block mesh_block;
	struct mat4f tmp1;
	struct mat4f tmp2;
	MAT4_TRANSLATE(tmp1, *model, body_part->pos);
	MAT4_ROTATEZ(float, tmp2, tmp1, body_part->rot.z);
	MAT4_ROTATEY(float, tmp1, tmp2, body_part->rot.y);
	MAT4_ROTATEX(float, tmp2, tmp1, body_part->rot.x);
	MAT4_MUL(mesh_block.mvp, world->player->mat_vp, tmp2);
	MAT4_MUL(mesh_block.mv, world->player->mat_v, tmp2);
	gfx_set_buffer_data(&body_part->uniform_buffer, &mesh_block, sizeof(mesh_block), 0);
	gfx_bind_constant(g_voxel->device, 0, &body_part->uniform_buffer, sizeof(mesh_block), 0);
	gfx_bind_attributes_state(g_voxel->device, &body_part->attributes_state, &g_voxel->graphics->entity.input_layout);
	gfx_draw_indexed(g_voxel->device, 36, 0);
}
