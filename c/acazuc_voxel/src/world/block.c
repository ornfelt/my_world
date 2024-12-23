#include "world/block.h"
#include "world/chunk.h"

#include "block/blocks.h"
#include "block/block.h"

#include "voxel.h"

#include <string.h>

#define AO_FACTOR 1

#define LIGHT_AO_TEST(light, ambient_occlusion) if (light < ambient_occlusion) {light = 0;} else {light -= ambient_occlusion;}

#define LIGHT_IDX(x, y, z) ((x * 3 + y) * 3 + z)

#define LIGHT_SMOOTH_ADD_X(x, y, z) \
	if (!blocks_transparent[LIGHT_IDX(1, y, z)]) \
	{ \
		total_light += blocks_lights[LIGHT_IDX(x, y, z)]; \
		lights_points++; \
	}

#define LIGHT_SMOOTH_ADD_Y(x, y, z) \
	if (!blocks_transparent[LIGHT_IDX(x, 1, z)]) \
	{ \
		total_light += blocks_lights[LIGHT_IDX(x, y, z)]; \
		lights_points++; \
	}

#define LIGHT_SMOOTH_ADD_Z(x, y, z) \
	if (!blocks_transparent[LIGHT_IDX(x, y, 1)]) \
	{ \
		total_light += blocks_lights[LIGHT_IDX(x, y, z)]; \
		lights_points++; \
	}

#define LIGHT_SMOOTH_TEST(x, y, z) blocks_transparent[LIGHT_IDX(x, y, z)]

#define LIGHT_SMOOTH_OP_X(y1, z1, y2, z2, y3, z3, x) \
	int8_t total_light = blocks_lights[LIGHT_IDX(x, 1, 1)]; \
	int8_t lights_points = 1; \
	if (LIGHT_SMOOTH_TEST(x, y1, z1)) \
	{ \
		LIGHT_SMOOTH_ADD_X(x, y1, z1); \
		if (LIGHT_SMOOTH_TEST(x, y2, z2)) \
			LIGHT_SMOOTH_ADD_X(x, y2, z2); \
		if (LIGHT_SMOOTH_TEST(x, y3, z3)) \
			LIGHT_SMOOTH_ADD_X(x, y3, z3); \
	} \
	else if (LIGHT_SMOOTH_TEST(x, y2, z2)) \
	{ \
		LIGHT_SMOOTH_ADD_X(x, y2, z2); \
		if (LIGHT_SMOOTH_TEST(x, y3, z3)) \
			LIGHT_SMOOTH_ADD_X(x, y3, z3); \
	} \
	int8_t average = total_light / lights_points;

#define LIGHT_SMOOTH_OP_Y(x1, z1, x2, z2, x3, z3, y) \
	int8_t total_light = blocks_lights[LIGHT_IDX(1, y, 1)]; \
	int8_t lights_points = 1; \
	if (LIGHT_SMOOTH_TEST(x1, y, z1)) \
	{ \
		LIGHT_SMOOTH_ADD_Y(x1, y, z1); \
		if (LIGHT_SMOOTH_TEST(x2, y, z2)) \
			LIGHT_SMOOTH_ADD_Y(x2, y, z2); \
		if (LIGHT_SMOOTH_TEST(x3, y, z3)) \
			LIGHT_SMOOTH_ADD_Y(x3, y, z3); \
	} \
	else if (LIGHT_SMOOTH_TEST(x2, y, z2)) \
	{ \
		LIGHT_SMOOTH_ADD_Y(x2, y, z2); \
		if (LIGHT_SMOOTH_TEST(x3, y, z3)) \
			LIGHT_SMOOTH_ADD_Y(x3, y, z3); \
	} \
	int8_t average = total_light / lights_points;

#define LIGHT_SMOOTH_OP_Z(x1, y1, x2, y2, x3, y3, z) \
	int8_t total_light = blocks_lights[LIGHT_IDX(1, 1, z)]; \
	int8_t lights_points = 1; \
	if (LIGHT_SMOOTH_TEST(x1, y1, z)) \
	{ \
		LIGHT_SMOOTH_ADD_Z(x1, y1, z); \
		if (LIGHT_SMOOTH_TEST(x2, y2, z)) \
			LIGHT_SMOOTH_ADD_Z(x2, y2, z); \
		if (LIGHT_SMOOTH_TEST(x3, y3, z)) \
			LIGHT_SMOOTH_ADD_Z(x3, y3, z); \
	} \
	else if (LIGHT_SMOOTH_TEST(x2, y2, z)) \
	{ \
		LIGHT_SMOOTH_ADD_Z(x2, y2, z); \
		if (LIGHT_SMOOTH_TEST(x3, y3, z)) \
			LIGHT_SMOOTH_ADD_Z(x3, y3, z); \
	} \
	int8_t average = total_light / lights_points;

static bool should_render_face_near(struct block block, struct block neighboor);
static void init_lights_levels(struct block_lights_levels *lights, uint8_t visible_faces, int8_t *blocks_lights);
static void calc_visible_faces(struct block block, struct chunk *chunk, struct vec3f pos, uint8_t *visible_faces);
static int8_t calc_light_level(struct chunk *chunk, struct vec3f pos, int8_t add_x, int8_t add_y, int8_t add_z);
static bool calc_transparent(struct chunk *chunk, struct vec3f pos, int8_t add_x, int8_t add_y, int8_t add_z);
static void calc_ambient_occlusion(struct vec3f pos, struct block_lights_levels *lights, uint8_t visible_faces, bool *blocks_transparent);
static void smooth_lights(struct block block, float *lights, uint8_t visible_faces, struct block_lights_levels *lights_levels, bool *blocks_transparent, int8_t *blocks_lights);
static bool is_transparent(struct block block);

void block_fill_buffers(struct block block, struct chunk *chunk, struct vec3f pos, struct tessellator *tessellator, uint8_t layer)
{
	if (block.type == 0)
		return;
	struct block_def *block_def = g_voxel->blocks->blocks[block.type];
	if (!block_def)
		return;
	if (block_def->layer != layer)
		return;
	if (!block_def->vtable || !block_def->vtable->draw)
		return;
	uint8_t visible_faces;
	calc_visible_faces(block, chunk, pos, &visible_faces);
	if (!visible_faces)
		return;
	bool blocks_transparent[27];
	int8_t blocks_lights[27];
	{
		int8_t i = 0;
		for (int8_t x = -1; x <= 1; ++x)
		{
			for (int8_t y = -1; y <= 1; ++y)
			{
				for (int8_t z = -1; z <= 1; ++z)
				{
					blocks_transparent[i] = calc_transparent(chunk, pos, x, y, z);
					blocks_lights[i++] = calc_light_level(chunk, pos, x, y, z);
				}
			}
		}
	}
	struct block_lights_levels lights_levels;
	memset(&lights_levels, 0, sizeof(lights_levels));
	init_lights_levels(&lights_levels, visible_faces, blocks_lights);
	if (g_voxel->ssao && block_def->flags & BLOCK_FLAG_SOLID)
		calc_ambient_occlusion(pos, &lights_levels, visible_faces, blocks_transparent);
#if 0
	struct vec3f color;
	VEC3_SET(color, 1, 1, 1);
	uint8_t biome = chunk->getBiome(pos.x - chunk->getX(), pos.z - chunk->getZ());
	color.x = (biome & 0x3) / 3.0;
	color.y = ((biome & 0xf) >> 2) / 3.0;
	color.z = ((biome & 0x3f) >> 4) / 3.0;
#endif
	float lights[24];
	smooth_lights(block, lights, visible_faces, &lights_levels, blocks_transparent, blocks_lights);
	block_def->vtable->draw(block_def, chunk, pos, tessellator, visible_faces, lights);
}

static bool should_render_face_near(struct block block, struct block neighboor)
{
	if (!is_transparent(neighboor))
		return false;
	if (neighboor.type == block.type)
	{
		struct block_def *block_def = g_voxel->blocks->blocks[block.type];
		if (!block_def || !(block_def->flags & BLOCK_FLAG_RENDER_SAME_NEIGHBOR))
			return false;
	}
	return true;
}

static void calc_visible_faces(struct block block, struct chunk *chunk, struct vec3f pos, uint8_t *visible_faces)
{
	struct block neighboor;
	*visible_faces = 0;
	if (pos.z - chunk->z == CHUNK_WIDTH - 1)
	{
		if (!chunk->chunk_z_more)
			*visible_faces |= BLOCK_FACE_FRONT;
		else if (!chunk_get_block(chunk->chunk_z_more, pos.x - chunk->x, pos.y, 0, &neighboor)
		      || should_render_face_near(block, neighboor))
			*visible_faces |= BLOCK_FACE_FRONT;
	}
	else if (!chunk_get_block(chunk, pos.x - chunk->x, pos.y, pos.z - chunk->z + 1, &neighboor)
	      || should_render_face_near(block, neighboor))
	{
		*visible_faces |= BLOCK_FACE_FRONT;
	}
	if (pos.z - chunk->z == 0)
	{
		if (!chunk->chunk_z_less)
			*visible_faces |= BLOCK_FACE_BACK;
		else if (!chunk_get_block(chunk->chunk_z_less, pos.x - chunk->x, pos.y, CHUNK_WIDTH - 1, &neighboor)
		      || should_render_face_near(block, neighboor))
			*visible_faces |= BLOCK_FACE_BACK;
	}
	else if (!chunk_get_block(chunk, pos.x - chunk->x, pos.y, pos.z - chunk->z - 1, &neighboor)
	      || should_render_face_near(block, neighboor))
	{
		*visible_faces |= BLOCK_FACE_BACK;
	}
	if (pos.x - chunk->x == 0)
	{
		if (!chunk->chunk_x_less)
			*visible_faces |= BLOCK_FACE_LEFT;
		else if (!chunk_get_block(chunk->chunk_x_less, CHUNK_WIDTH - 1, pos.y, pos.z - chunk->z, &neighboor)
		      || should_render_face_near(block, neighboor))
			*visible_faces |= BLOCK_FACE_LEFT;
	}
	else if (!chunk_get_block(chunk, pos.x - chunk->x - 1, pos.y, pos.z - chunk->z, &neighboor)
	      || should_render_face_near(block, neighboor))
	{
		*visible_faces |= BLOCK_FACE_LEFT;
	}
	if (pos.x - chunk->x == CHUNK_WIDTH - 1)
	{
		if (!chunk->chunk_x_more)
			*visible_faces |= BLOCK_FACE_RIGHT;
		else if (!chunk_get_block(chunk->chunk_x_more, 0, pos.y, pos.z - chunk->z, &neighboor)
		      || should_render_face_near(block, neighboor))
			*visible_faces |= BLOCK_FACE_RIGHT;
	}
	else if (!chunk_get_block(chunk, pos.x - chunk->x + 1, pos.y, pos.z - chunk->z, &neighboor)
	      || should_render_face_near(block, neighboor))
	{
		*visible_faces |= BLOCK_FACE_RIGHT;
	}
	if (pos.y == CHUNK_HEIGHT - 1)
		*visible_faces |= BLOCK_FACE_TOP;
	else if (!chunk_get_block(chunk, pos.x - chunk->x, pos.y + 1, pos.z - chunk->z, &neighboor)
	      || should_render_face_near(block, neighboor))
		*visible_faces |= BLOCK_FACE_TOP;
	if (pos.y == 0)
		*visible_faces |= BLOCK_FACE_BOTTOM;
	else if (!chunk_get_block(chunk, pos.x - chunk->x, pos.y - 1, pos.z - chunk->z, &neighboor)
	      || should_render_face_near(block, neighboor))
		*visible_faces |= BLOCK_FACE_BOTTOM;
}

static int8_t calc_light_level(struct chunk *chunk, struct vec3f pos, int8_t add_x, int8_t add_y, int8_t add_z)
{
	if (pos.y + add_y < 0 || pos.y + add_y >= CHUNK_HEIGHT)
		return 15;
	int32_t new_x = pos.x - chunk->x + add_x;
	int32_t new_y = pos.y + add_y;
	int32_t new_z = pos.z - chunk->z + add_z;
	if (new_x < 0)
	{
		new_x += CHUNK_WIDTH;
		chunk = chunk->chunk_x_less;
		if (!chunk)
			return 15;
	}
	else if (new_x >= CHUNK_WIDTH)
	{
		new_x -= CHUNK_WIDTH;
		chunk = chunk->chunk_x_more;
		if (!chunk)
			return 15;
	}
	if (new_z < 0)
	{
		new_z += CHUNK_WIDTH;
		chunk = chunk->chunk_z_less;
		if (!chunk)
			return 15;
	}
	else if (new_z >= CHUNK_WIDTH)
	{
		new_z -= CHUNK_WIDTH;
		chunk = chunk->chunk_z_more;
		if (!chunk)
			return 15;
	}
	return chunk_get_light(chunk, new_x, new_y, new_z);
}

static bool calc_transparent(struct chunk *chunk, struct vec3f pos, int8_t add_x, int8_t add_y, int8_t add_z)
{
	if (pos.y + add_y < 0 || pos.y + add_y >= CHUNK_HEIGHT)
		return true;
	int32_t new_x = pos.x - chunk->x + add_x;
	int32_t new_y = pos.y + add_y;
	int32_t new_z = pos.z - chunk->z + add_z;
	if (new_x < 0)
	{
		new_x += CHUNK_WIDTH;
		chunk = chunk->chunk_x_less;
		if (!chunk)
			return true;
	}
	else if (new_x >= CHUNK_WIDTH)
	{
		new_x -= CHUNK_WIDTH;
		chunk = chunk->chunk_x_more;
		if (!chunk)
			return true;
	}
	if (new_z < 0)
	{
		new_z += CHUNK_WIDTH;
		chunk = chunk->chunk_z_less;
		if (!chunk)
			return true;
	}
	else if (new_z >= CHUNK_WIDTH)
	{
		new_z -= CHUNK_WIDTH;
		chunk = chunk->chunk_z_more;
		if (!chunk)
			return true;
	}
	struct block block;
	if (!chunk_get_block(chunk, new_x, new_y, new_z, &block))
		return true;
	return is_transparent(block);
}

static void init_lights_levels(struct block_lights_levels *lights, uint8_t visible_faces, int8_t *blocks_lights)
{
	if (visible_faces & BLOCK_FACE_FRONT)
	{
		int8_t value = blocks_lights[LIGHT_IDX(1, 1, 2)];
		lights->f1p1 = value;
		lights->f1p2 = value;
		lights->f1p3 = value;
		lights->f1p4 = value;
	}
	if (visible_faces & BLOCK_FACE_BACK)
	{
		int8_t value = blocks_lights[LIGHT_IDX(1, 1, 0)];
		lights->f2p1 = value;
		lights->f2p2 = value;
		lights->f2p3 = value;
		lights->f2p4 = value;
	}
	if (visible_faces & BLOCK_FACE_LEFT)
	{
		int8_t value = blocks_lights[LIGHT_IDX(0, 1, 1)];
		lights->f3p1 = value;
		lights->f3p2 = value;
		lights->f3p3 = value;
		lights->f3p4 = value;
	}
	if (visible_faces & BLOCK_FACE_RIGHT)
	{
		int8_t value = blocks_lights[LIGHT_IDX(2, 1, 1)];
		lights->f4p1 = value;
		lights->f4p2 = value;
		lights->f4p3 = value;
		lights->f4p4 = value;
	}
	if (visible_faces & BLOCK_FACE_TOP)
	{
		int8_t value = blocks_lights[LIGHT_IDX(1, 2, 1)];
		lights->f5p1 = value;
		lights->f5p2 = value;
		lights->f5p3 = value;
		lights->f5p4 = value;
	}
	if (visible_faces & BLOCK_FACE_BOTTOM)
	{
		int8_t value = blocks_lights[LIGHT_IDX(1, 0, 1)];
		lights->f6p1 = value;
		lights->f6p2 = value;
		lights->f6p3 = value;
		lights->f6p4 = value;
	}
}

static void calc_ambient_occlusion(struct vec3f pos, struct block_lights_levels *lights, uint8_t visible_faces, bool *blocks_transparent)
{
	struct block_lights_levels ambient_occlusion;
	memset(&ambient_occlusion, 0, sizeof(ambient_occlusion));
	if (visible_faces & BLOCK_FACE_FRONT)
	{
		if (!blocks_transparent[LIGHT_IDX(2, 0, 2)])
			ambient_occlusion.f1p4 += AO_FACTOR;
		if (!blocks_transparent[LIGHT_IDX(1, 0, 2)])
		{
			ambient_occlusion.f1p4 += AO_FACTOR;
			ambient_occlusion.f1p1 += AO_FACTOR;
		}
		if (!blocks_transparent[LIGHT_IDX(0, 0, 2)])
			ambient_occlusion.f1p1 += AO_FACTOR;
		if (!blocks_transparent[LIGHT_IDX(0, 2, 2)])
			ambient_occlusion.f1p2 += AO_FACTOR;
		if (!blocks_transparent[LIGHT_IDX(1, 2, 2)])
		{
			ambient_occlusion.f1p2 += AO_FACTOR;
			ambient_occlusion.f1p3 += AO_FACTOR;
		}
		if (!blocks_transparent[LIGHT_IDX(2, 2, 2)])
			ambient_occlusion.f1p3 += AO_FACTOR;
		if (!blocks_transparent[LIGHT_IDX(2, 1, 2)])
		{
			ambient_occlusion.f1p3 += AO_FACTOR;
			ambient_occlusion.f1p4 += AO_FACTOR;
		}
		if (!blocks_transparent[LIGHT_IDX(0, 1, 2)])
		{
			ambient_occlusion.f1p1 += AO_FACTOR;
			ambient_occlusion.f1p2 += AO_FACTOR;
		}
	}
	if (visible_faces & BLOCK_FACE_BACK)
	{
		if (!blocks_transparent[LIGHT_IDX(2, 0, 0)])
			ambient_occlusion.f2p4 += AO_FACTOR;
		if (!blocks_transparent[LIGHT_IDX(1, 0, 0)])
		{
			ambient_occlusion.f2p4 += AO_FACTOR;
			ambient_occlusion.f2p1 += AO_FACTOR;
		}
		if (!blocks_transparent[LIGHT_IDX(0, 0, 0)])
			ambient_occlusion.f2p1 += AO_FACTOR;
		if (!blocks_transparent[LIGHT_IDX(0, 2, 0)])
			ambient_occlusion.f2p2 += AO_FACTOR;
		if (!blocks_transparent[LIGHT_IDX(1, 2, 0)])
		{
			ambient_occlusion.f2p2 += AO_FACTOR;
			ambient_occlusion.f2p3 += AO_FACTOR;
		}
		if (!blocks_transparent[LIGHT_IDX(2, 2, 0)])
			ambient_occlusion.f2p3 += AO_FACTOR;
		if (!blocks_transparent[LIGHT_IDX(2, 1, 0)])
		{
			ambient_occlusion.f2p3 += AO_FACTOR;
			ambient_occlusion.f2p4 += AO_FACTOR;
		}
		if (!blocks_transparent[LIGHT_IDX(0, 1, 0)])
		{
			ambient_occlusion.f2p1 += AO_FACTOR;
			ambient_occlusion.f2p2 += AO_FACTOR;
		}
	}
	if (visible_faces & BLOCK_FACE_LEFT)
	{
		if (!blocks_transparent[LIGHT_IDX(0, 0, 2)])
			ambient_occlusion.f3p4 += AO_FACTOR;
		if (!blocks_transparent[LIGHT_IDX(0, 0, 1)])
		{
			ambient_occlusion.f3p4 += AO_FACTOR;
			ambient_occlusion.f3p1 += AO_FACTOR;
		}
		if (!blocks_transparent[LIGHT_IDX(0, 0, 0)])
			ambient_occlusion.f3p1 += AO_FACTOR;
		if (!blocks_transparent[LIGHT_IDX(0, 2, 0)])
			ambient_occlusion.f3p2 += AO_FACTOR;
		if (!blocks_transparent[LIGHT_IDX(0, 2, 1)])
		{
			ambient_occlusion.f3p2 += AO_FACTOR;
			ambient_occlusion.f3p3 += AO_FACTOR;
		}
		if (!blocks_transparent[LIGHT_IDX(0, 2, 2)])
			ambient_occlusion.f3p3 += AO_FACTOR;
		if (!blocks_transparent[LIGHT_IDX(0, 1, 2)])
		{
			ambient_occlusion.f3p3 += AO_FACTOR;
			ambient_occlusion.f3p4 += AO_FACTOR;
		}
		if (!blocks_transparent[LIGHT_IDX(0, 1, 0)])
		{
			ambient_occlusion.f3p1 += AO_FACTOR;
			ambient_occlusion.f3p2 += AO_FACTOR;
		}
	}
	if (visible_faces & BLOCK_FACE_RIGHT)
	{
		if (!blocks_transparent[LIGHT_IDX(2, 0, 2)])
			ambient_occlusion.f4p4 += AO_FACTOR;
		if (!blocks_transparent[LIGHT_IDX(2, 0, 1)])
		{
			ambient_occlusion.f4p4 += AO_FACTOR;
			ambient_occlusion.f4p1 += AO_FACTOR;
		}
		if (!blocks_transparent[LIGHT_IDX(2, 0, 0)])
			ambient_occlusion.f4p1 += AO_FACTOR;
		if (!blocks_transparent[LIGHT_IDX(2, 2, 0)])
			ambient_occlusion.f4p2 += AO_FACTOR;
		if (!blocks_transparent[LIGHT_IDX(2, 2, 1)])
		{
			ambient_occlusion.f4p2 += AO_FACTOR;
			ambient_occlusion.f4p3 += AO_FACTOR;
		}
		if (!blocks_transparent[LIGHT_IDX(2, 2, 2)])
			ambient_occlusion.f4p3 += AO_FACTOR;
		if (!blocks_transparent[LIGHT_IDX(2, 1, 2)])
		{
			ambient_occlusion.f4p3 += AO_FACTOR;
			ambient_occlusion.f4p4 += AO_FACTOR;
		}
		if (!blocks_transparent[LIGHT_IDX(2, 1, 0)])
		{
			ambient_occlusion.f4p1 += AO_FACTOR;
			ambient_occlusion.f4p2 += AO_FACTOR;
		}
	}
	if (visible_faces & BLOCK_FACE_TOP && pos.y < CHUNK_HEIGHT - 1)
	{
		if (!blocks_transparent[LIGHT_IDX(0, 2, 0)])
			ambient_occlusion.f5p2 += AO_FACTOR;
		if (!blocks_transparent[LIGHT_IDX(0, 2, 1)])
		{
			ambient_occlusion.f5p1 += AO_FACTOR;
			ambient_occlusion.f5p2 += AO_FACTOR;
		}
		if (!blocks_transparent[LIGHT_IDX(0, 2, 2)])
			ambient_occlusion.f5p1 += AO_FACTOR;
		if (!blocks_transparent[LIGHT_IDX(2, 2, 0)])
			ambient_occlusion.f5p3 += AO_FACTOR;
		if (!blocks_transparent[LIGHT_IDX(2, 2, 1)])
		{
			ambient_occlusion.f5p3 += AO_FACTOR;
			ambient_occlusion.f5p4 += AO_FACTOR;
		}
		if (!blocks_transparent[LIGHT_IDX(2, 2, 2)])
			ambient_occlusion.f5p4 += AO_FACTOR;
		if (!blocks_transparent[LIGHT_IDX(1, 2, 0)])
		{
			ambient_occlusion.f5p2 += AO_FACTOR;
			ambient_occlusion.f5p3 += AO_FACTOR;
		}
		if (!blocks_transparent[LIGHT_IDX(1, 2, 2)])
		{
			ambient_occlusion.f5p4 += AO_FACTOR;
			ambient_occlusion.f5p1 += AO_FACTOR;
		}
	}
	if (visible_faces & BLOCK_FACE_BOTTOM && pos.y > 0)
	{
		if (!blocks_transparent[LIGHT_IDX(0, 0, 2)])
			ambient_occlusion.f6p1 += AO_FACTOR;
		if (!blocks_transparent[LIGHT_IDX(0, 0, 1)])
		{
			ambient_occlusion.f6p1 += AO_FACTOR;
			ambient_occlusion.f6p2 += AO_FACTOR;
		}
		if (!blocks_transparent[LIGHT_IDX(0, 0, 0)])
			ambient_occlusion.f6p2 += AO_FACTOR;
		if (!blocks_transparent[LIGHT_IDX(2, 0, 0)])
			ambient_occlusion.f6p3 += AO_FACTOR;
		if (!blocks_transparent[LIGHT_IDX(2, 0, 1)])
		{
			ambient_occlusion.f6p3 += AO_FACTOR;
			ambient_occlusion.f6p4 += AO_FACTOR;
		}
		if (!blocks_transparent[LIGHT_IDX(2, 0, 2)])
			ambient_occlusion.f6p4 += AO_FACTOR;
		if (!blocks_transparent[LIGHT_IDX(1, 0, 0)])
		{
			ambient_occlusion.f6p2 += AO_FACTOR;
			ambient_occlusion.f6p3 += AO_FACTOR;
		}
		if (!blocks_transparent[LIGHT_IDX(1, 0, 2)])
		{
			ambient_occlusion.f6p4 += AO_FACTOR;
			ambient_occlusion.f6p1 += AO_FACTOR;
		}
	}
	LIGHT_AO_TEST(lights->f1p1, ambient_occlusion.f1p1);
	LIGHT_AO_TEST(lights->f1p2, ambient_occlusion.f1p2);
	LIGHT_AO_TEST(lights->f1p3, ambient_occlusion.f1p3);
	LIGHT_AO_TEST(lights->f1p4, ambient_occlusion.f1p4);
	LIGHT_AO_TEST(lights->f2p1, ambient_occlusion.f2p1);
	LIGHT_AO_TEST(lights->f2p2, ambient_occlusion.f2p2);
	LIGHT_AO_TEST(lights->f2p3, ambient_occlusion.f2p3);
	LIGHT_AO_TEST(lights->f2p4, ambient_occlusion.f2p4);
	LIGHT_AO_TEST(lights->f3p1, ambient_occlusion.f3p1);
	LIGHT_AO_TEST(lights->f3p2, ambient_occlusion.f3p2);
	LIGHT_AO_TEST(lights->f3p3, ambient_occlusion.f3p3);
	LIGHT_AO_TEST(lights->f3p4, ambient_occlusion.f3p4);
	LIGHT_AO_TEST(lights->f4p1, ambient_occlusion.f4p1);
	LIGHT_AO_TEST(lights->f4p2, ambient_occlusion.f4p2);
	LIGHT_AO_TEST(lights->f4p3, ambient_occlusion.f4p3);
	LIGHT_AO_TEST(lights->f4p4, ambient_occlusion.f4p4);
	LIGHT_AO_TEST(lights->f5p1, ambient_occlusion.f5p1);
	LIGHT_AO_TEST(lights->f5p2, ambient_occlusion.f5p2);
	LIGHT_AO_TEST(lights->f5p3, ambient_occlusion.f5p3);
	LIGHT_AO_TEST(lights->f5p4, ambient_occlusion.f5p4);
	LIGHT_AO_TEST(lights->f6p1, ambient_occlusion.f6p1);
	LIGHT_AO_TEST(lights->f6p2, ambient_occlusion.f6p2);
	LIGHT_AO_TEST(lights->f6p3, ambient_occlusion.f6p3);
	LIGHT_AO_TEST(lights->f6p4, ambient_occlusion.f6p4);
}

static void smooth_lights(struct block block, float *lights, uint8_t visible_faces, struct block_lights_levels *lights_levels, bool *blocks_transparent, int8_t *blocks_lights)
{
	struct block_def *block_def = g_voxel->blocks->blocks[block.type];
	if (!g_voxel->smooth || !(block_def->flags & BLOCK_FLAG_FULL_CUBE) || is_transparent(block))
	{
		lights[F1P1] = voxel_light_value(lights_levels->f1p1);
		lights[F1P2] = voxel_light_value(lights_levels->f1p2);
		lights[F1P3] = voxel_light_value(lights_levels->f1p3);
		lights[F1P4] = voxel_light_value(lights_levels->f1p4);
		lights[F2P1] = voxel_light_value(lights_levels->f2p1);
		lights[F2P2] = voxel_light_value(lights_levels->f2p2);
		lights[F2P3] = voxel_light_value(lights_levels->f2p3);
		lights[F2P4] = voxel_light_value(lights_levels->f2p4);
		lights[F3P1] = voxel_light_value(lights_levels->f3p1);
		lights[F3P2] = voxel_light_value(lights_levels->f3p2);
		lights[F3P3] = voxel_light_value(lights_levels->f3p3);
		lights[F3P4] = voxel_light_value(lights_levels->f3p4);
		lights[F4P1] = voxel_light_value(lights_levels->f4p1);
		lights[F4P2] = voxel_light_value(lights_levels->f4p2);
		lights[F4P3] = voxel_light_value(lights_levels->f4p3);
		lights[F4P4] = voxel_light_value(lights_levels->f4p4);
		lights[F5P1] = voxel_light_value(lights_levels->f5p1);
		lights[F5P2] = voxel_light_value(lights_levels->f5p2);
		lights[F5P3] = voxel_light_value(lights_levels->f5p3);
		lights[F5P4] = voxel_light_value(lights_levels->f5p4);
		lights[F6P1] = voxel_light_value(lights_levels->f6p1);
		lights[F6P2] = voxel_light_value(lights_levels->f6p2);
		lights[F6P3] = voxel_light_value(lights_levels->f6p3);
		lights[F6P4] = voxel_light_value(lights_levels->f6p4);
		return;
	}
	if (visible_faces & BLOCK_FACE_FRONT)
	{
		{
			LIGHT_SMOOTH_OP_Z(0, 1, 1, 0, 0, 0, 2);
			lights[F1P1] = voxel_light_value(average + lights_levels->f1p1 - blocks_lights[LIGHT_IDX(1, 1, 2)]);
		}
		{
			LIGHT_SMOOTH_OP_Z(1, 0, 2, 1, 2, 0, 2);
			lights[F1P4] = voxel_light_value(average + lights_levels->f1p4 - blocks_lights[LIGHT_IDX(1, 1, 2)]);
		}
		{
			LIGHT_SMOOTH_OP_Z(2, 1, 1, 2, 2, 2, 2);
			lights[F1P3] = voxel_light_value(average + lights_levels->f1p3 - blocks_lights[LIGHT_IDX(1, 1, 2)]);
		}
		{
			LIGHT_SMOOTH_OP_Z(0, 1, 1, 2, 0, 2, 2);
			lights[F1P2] = voxel_light_value(average + lights_levels->f1p2 - blocks_lights[LIGHT_IDX(1, 1, 2)]);
		}
	}
	if (visible_faces & BLOCK_FACE_BACK)
	{
		{
			LIGHT_SMOOTH_OP_Z(0, 1, 1, 0, 0, 0, 0);
			lights[F2P1] = voxel_light_value(average + lights_levels->f2p1 - blocks_lights[LIGHT_IDX(1, 1, 0)]);
		}
		{
			LIGHT_SMOOTH_OP_Z(1, 0, 2, 1, 2, 0, 0);
			lights[F2P4] = voxel_light_value(average + lights_levels->f2p4 - blocks_lights[LIGHT_IDX(1, 1, 0)]);
		}
		{
			LIGHT_SMOOTH_OP_Z(2, 1, 1, 2, 2, 2, 0);
			lights[F2P3] = voxel_light_value(average + lights_levels->f2p3 - blocks_lights[LIGHT_IDX(1, 1, 0)]);
		}
		{
			LIGHT_SMOOTH_OP_Z(0, 1, 1, 2, 0, 2, 0);
			lights[F2P2] = voxel_light_value(average + lights_levels->f2p2 - blocks_lights[LIGHT_IDX(1, 1, 0)]);
		}
	}
	if (visible_faces & BLOCK_FACE_LEFT)
	{
		{
			LIGHT_SMOOTH_OP_X(0, 1, 1, 0, 0, 0, 0);
			lights[F3P1] = voxel_light_value(average + lights_levels->f3p1 - blocks_lights[LIGHT_IDX(0, 1, 1)]);
		}
		{
			LIGHT_SMOOTH_OP_X(1, 0, 2, 1, 2, 0, 0);
			lights[F3P2] = voxel_light_value(average + lights_levels->f3p2 - blocks_lights[LIGHT_IDX(0, 1, 1)]);
		}
		{
			LIGHT_SMOOTH_OP_X(1, 2, 2, 1, 2, 2, 0);
			lights[F3P3] = voxel_light_value(average + lights_levels->f3p3 - blocks_lights[LIGHT_IDX(0, 1, 1)]);
		}
		{
			LIGHT_SMOOTH_OP_X(0, 1, 1, 2, 0, 2, 0);
			lights[F3P4] = voxel_light_value(average + lights_levels->f3p4 - blocks_lights[LIGHT_IDX(0, 1, 1)]);
		}
	}
	if (visible_faces & BLOCK_FACE_RIGHT)
	{
		{
			LIGHT_SMOOTH_OP_X(0, 1, 1, 0, 0, 0, 2);
			lights[F4P1] = voxel_light_value(average + lights_levels->f4p1 - blocks_lights[LIGHT_IDX(2, 1, 1)]);
		}
		{
			LIGHT_SMOOTH_OP_X(1, 0, 2, 1, 2, 0, 2);
			lights[F4P2] = voxel_light_value(average + lights_levels->f4p2 - blocks_lights[LIGHT_IDX(2, 1, 1)]);
		}
		{
			LIGHT_SMOOTH_OP_X(2, 1, 1, 2, 2, 2, 2);
			lights[F4P3] = voxel_light_value(average + lights_levels->f4p3 - blocks_lights[LIGHT_IDX(2, 1, 1)]);
		}
		{
			LIGHT_SMOOTH_OP_X(0, 1, 1, 2, 0, 2, 2);
			lights[F4P4] = voxel_light_value(average + lights_levels->f4p4 - blocks_lights[LIGHT_IDX(2, 1, 1)]);
		}
	}
	if (visible_faces & BLOCK_FACE_TOP)
	{
		{
			LIGHT_SMOOTH_OP_Y(0, 1, 1, 0, 0, 0, 2);
			lights[F5P2] = voxel_light_value(average + lights_levels->f5p2 - blocks_lights[LIGHT_IDX(1, 2, 1)]);
		}
		{
			LIGHT_SMOOTH_OP_Y(1, 0, 2, 1, 2, 0, 2);
			lights[F5P3] = voxel_light_value(average + lights_levels->f5p3 - blocks_lights[LIGHT_IDX(1, 2, 1)]);
		}
		{
			LIGHT_SMOOTH_OP_Y(1, 2, 2, 1, 2, 2, 2);
			lights[F5P4] = voxel_light_value(average + lights_levels->f5p4 - blocks_lights[LIGHT_IDX(1, 2, 1)]);
		}
		{
			LIGHT_SMOOTH_OP_Y(0, 1, 1, 2, 0, 2, 2);
			lights[F5P1] = voxel_light_value(average + lights_levels->f5p1 - blocks_lights[LIGHT_IDX(1, 2, 1)]);
		}
	}
	if (visible_faces & BLOCK_FACE_BOTTOM)
	{
		{
			LIGHT_SMOOTH_OP_Y(0, 1, 1, 0, 0, 0, 0);
			lights[F6P2] = voxel_light_value(average + lights_levels->f6p2 - blocks_lights[LIGHT_IDX(1, 0, 1)]);
		}
		{
			LIGHT_SMOOTH_OP_Y(1, 0, 2, 1, 2, 0, 0);
			lights[F6P3] = voxel_light_value(average + lights_levels->f6p3 - blocks_lights[LIGHT_IDX(1, 0, 1)]);
		}
		{
			LIGHT_SMOOTH_OP_Y(2, 1, 1, 2, 2, 2, 0);
			lights[F6P4] = voxel_light_value(average + lights_levels->f6p4 - blocks_lights[LIGHT_IDX(1, 0, 1)]);
		}
		{
			LIGHT_SMOOTH_OP_Y(0, 1, 1, 2, 0, 2, 0);
			lights[F6P1] = voxel_light_value(average + lights_levels->f6p1 - blocks_lights[LIGHT_IDX(1, 0, 1)]);
		}
	}
}

static bool is_transparent(struct block block)
{
	if (block.type == 0)
		return true;
	struct block_def *block_def = g_voxel->blocks->blocks[block.type];
	if (!block_def)
		return true;
	if (block_def->flags & BLOCK_FLAG_TRANSPARENT)
		return true;
	return false;
}
