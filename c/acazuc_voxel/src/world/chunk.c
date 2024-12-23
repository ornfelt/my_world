#include "world/storage.h"
#include "world/block.h"
#include "world/chunk.h"
#include "world/world.h"

#include "biome/biomes.h"
#include "biome/biome.h"

#include "noise/worley.h"

#include "block/blocks.h"
#include "block/block.h"

#include "graphics.h"
#include "shaders.h"
#include "voxel.h"
#include "log.h"

#include <gfx/device.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

static void update_gfx_buffers(struct chunk *chunk);
static struct storage *create_storage(struct chunk *chunk, uint8_t id);
static void init_nbt(struct chunk *chunk, struct nbt_tag_compound *nbt);

struct chunk *chunk_new(struct world *world, int32_t x, int32_t z, struct nbt_tag_compound *nbt)
{
	struct chunk *chunk = calloc(sizeof(*chunk), 1);
	if (!chunk)
	{
		LOG_ERROR("chunk allocation failed");
		return NULL;
	}
	if (!particles_init(&chunk->particles, chunk))
	{
		LOG_ERROR("particles initialization failed");
		abort();
	}
	if (!entities_init(&chunk->entities, chunk))
	{
		LOG_ERROR("entities initialization failed");
		abort();
	}
	chunk->world = world;
	chunk->x = x;
	chunk->z = z;
	VEC3_SET(chunk->aabb.p0, x, 0, z);
	VEC3_SET(chunk->aabb.p1, x + CHUNK_WIDTH, CHUNK_HEIGHT, z + CHUNK_WIDTH);
	chunk->chunk_x_less_z_less = world_get_chunk(chunk->world, chunk->x - CHUNK_WIDTH, chunk->z - CHUNK_WIDTH);
	if (chunk->chunk_x_less_z_less)
		chunk->chunk_x_less_z_less->chunk_x_more_z_more = chunk;
	chunk->chunk_x_more_z_less = world_get_chunk(chunk->world, chunk->x + CHUNK_WIDTH, chunk->z - CHUNK_WIDTH);
	if (chunk->chunk_x_more_z_less)
		chunk->chunk_x_more_z_less->chunk_x_less_z_more = chunk;
	chunk->chunk_x_less_z_more = world_get_chunk(chunk->world, chunk->x - CHUNK_WIDTH, chunk->z + CHUNK_WIDTH);
	if (chunk->chunk_x_less_z_more)
		chunk->chunk_x_less_z_more->chunk_x_more_z_less = chunk;
	chunk->chunk_x_more_z_more = world_get_chunk(chunk->world, chunk->x + CHUNK_WIDTH, chunk->z + CHUNK_WIDTH);
	if (chunk->chunk_x_more_z_more)
		chunk->chunk_x_more_z_more->chunk_x_less_z_less = chunk;
	chunk->chunk_x_less = world_get_chunk(chunk->world, chunk->x - CHUNK_WIDTH, chunk->z);
	if (chunk->chunk_x_less)
		chunk->chunk_x_less->chunk_x_more = chunk;
	chunk->chunk_x_more = world_get_chunk(chunk->world, chunk->x + CHUNK_WIDTH, chunk->z);
	if (chunk->chunk_x_more)
		chunk->chunk_x_more->chunk_x_less = chunk;
	chunk->chunk_z_less = world_get_chunk(chunk->world, chunk->x, chunk->z - CHUNK_WIDTH);
	if (chunk->chunk_z_less)
		chunk->chunk_z_less->chunk_z_more = chunk;
	chunk->chunk_z_more = world_get_chunk(chunk->world, chunk->x, chunk->z + CHUNK_WIDTH);
	if (chunk->chunk_z_more)
		chunk->chunk_z_more->chunk_z_less = chunk;
	for (size_t i = 0; i < sizeof(chunk->layers) / sizeof(*chunk->layers); ++i)
	{
		chunk->layers[i].attributes_state = GFX_ATTRIBUTES_STATE_INIT();
		chunk->layers[i].vertexes_buffer = GFX_BUFFER_INIT();
		chunk->layers[i].indices_buffer = GFX_BUFFER_INIT();
		chunk->layers[i].indices_nb = 0;
		jks_array_init(&chunk->layers[i].tessellator.vertexes, sizeof(struct shader_blocks_vertex), NULL, NULL);
		jks_array_init(&chunk->layers[i].tessellator.indices, sizeof(uint32_t), NULL, NULL);
	}
	init_nbt(chunk, nbt);
	return chunk;
}

void chunk_delete(struct chunk *chunk)
{
	if (!chunk)
		return;
	for (size_t i = 0; i < sizeof(chunk->storages) / sizeof(*chunk->storages); ++i)
		storage_delete(chunk->storages[i]);
	nbt_tag_delete((struct nbt_tag*)chunk->nbt.nbt);
	if (chunk->chunk_x_less_z_less)
	{
		chunk->chunk_x_less_z_less->chunk_x_more_z_more = NULL;
		chunk_regenerate_light_map(chunk->chunk_x_less_z_less);
	}
	if (chunk->chunk_x_more_z_less)
	{
		chunk->chunk_x_more_z_less->chunk_x_less_z_more = NULL;
		chunk_regenerate_light_map(chunk->chunk_x_more_z_less);
	}
	if (chunk->chunk_x_less_z_more)
	{
		chunk->chunk_x_less_z_more->chunk_x_more_z_less = NULL;
		chunk_regenerate_light_map(chunk->chunk_x_less_z_more);
	}
	if (chunk->chunk_x_more_z_more)
	{
		chunk->chunk_x_more_z_more->chunk_x_less_z_less = NULL;
		chunk_regenerate_light_map(chunk->chunk_x_more_z_more);
	}
	if (chunk->chunk_x_less)
	{
		chunk->chunk_x_less->chunk_x_more = NULL;
		chunk_regenerate_light_map(chunk->chunk_x_less);
	}
	if (chunk->chunk_x_more)
	{
		chunk->chunk_x_more->chunk_x_less = NULL;
		chunk_regenerate_light_map(chunk->chunk_x_more);
	}
	if (chunk->chunk_z_less)
	{
		chunk->chunk_z_less->chunk_z_more = NULL;
		chunk_regenerate_light_map(chunk->chunk_z_less);
	}
	if (chunk->chunk_z_more)
	{
		chunk->chunk_z_more->chunk_z_less = NULL;
		chunk_regenerate_light_map(chunk->chunk_z_more);
	}
	for (size_t i = 0; i < sizeof(chunk->layers) / sizeof(*chunk->layers); ++i)
	{
		gfx_delete_attributes_state(g_voxel->device, &chunk->layers[i].attributes_state);
		gfx_delete_buffer(g_voxel->device, &chunk->layers[i].vertexes_buffer);
		gfx_delete_buffer(g_voxel->device, &chunk->layers[i].indices_buffer);
		jks_array_destroy(&chunk->layers[i].tessellator.vertexes);
		jks_array_destroy(&chunk->layers[i].tessellator.indices);
	}
	particles_destroy(&chunk->particles);
	entities_destroy(&chunk->entities);
	free(chunk);
}

void chunk_generate(struct chunk *chunk)
{
	if (chunk_is_generated(chunk))
		return;
	chunk_set_generated(chunk, true);
	for (int32_t x = 0; x < CHUNK_WIDTH; ++x)
	{
		for (int32_t z = 0; z < CHUNK_WIDTH; ++z)
		{
			//float fac = 50;
			//float temp = chunk->world.getBiomeTempNoise().get2((chunk->x + x) * fac, (chunk->z + z) * fac) / 2 + .5;
			//float rain = chunk->world.getBiomeRainNoise().get2((chunk->x + x) * fac, (chunk->z + z) * fac) / 2 + .5;
			//float temp = std::min(1., std::max(-1., WorleyNoise::get2((chunk->x + x) * fac, (chunk->z + z) * fac))) / 2 + .5;
			//float rain = std::min(1., std::max(-1., WorleyNoise::get2((chunk->x + x) * fac, (chunk->z + z) * fac))) / 2 + .5;
			//chunk->nbt.Biomes->getValues()[getXZId(x, z)] = Biomes::getBiomeFor(temp, rain);
			//float noise_index = -.02;
			//float noise_index = 0;
			float noise_index = simplex_noise_get2(&chunk->world->noise, (chunk->x + x) * 100, (chunk->z + z) * 100);
			//float noise_index = std::min(1., std::max(-1., WorleyNoise::get2((chunk->x + x) / 50., (chunk->z + z) / 50.)));
			//noise_index *= chunk->world.getNoise().get2(chunk->x + x, chunk->z + z);
			//float noise_index = chunk->world.getNoise().get2(chunk->x + x, chunk->z + z) / 2;
			//noise_index += chunk->world.getNoise().get3(chunk->x + x, chunk->z + z, 3) / 3;
			//noise_index += chunk->world.getNoise().get3(chunk->x + x, chunk->z + z, 300000) / 4;
			noise_index = noise_index * CHUNK_HEIGHT / 5 + CHUNK_HEIGHT / 4;
			noise_index = roundf(noise_index);
			//if ((x + z) % 2 == 0)
			//	noise_index--;
			for (int32_t y = 0; y < CHUNK_HEIGHT; ++y)
			{
				if (y > noise_index && y > CHUNK_HEIGHT / 4)
					continue;
				uint16_t block_type = 1;
				if (y == 0)
					block_type = 7;
				else if (y == noise_index)
				{
					if (y <= CHUNK_HEIGHT / 4)
						block_type = 12;
					else
						block_type = 2;//rand() / (float)RAND_MAX * 255;//2;
				}
				else if (y > noise_index)
					block_type = 8;
				else if (y > noise_index - 3)
				{
					if (y < CHUNK_HEIGHT / 4)
						block_type = 12;
					else
						block_type = 3;
				}
				chunk_set_block_if_replaceable(chunk, x, y, z, block_type);
			}
		}
	}
	biome_generate(g_voxel->biomes->biomes[1], chunk);
	chunk_regenerate_light_map_rec(chunk);
}

void chunk_tick(struct chunk *chunk)
{
	entities_tick(&chunk->entities);
	particles_tick(&chunk->particles);
}

void chunk_draw_entities(struct chunk *chunk)
{
	if (!chunk_is_generated(chunk) || !chunk->visible)
		return;
	particles_draw(&chunk->particles);
	entities_draw(&chunk->entities);
}

void chunk_draw(struct chunk *chunk, uint8_t layer)
{
	if (!chunk_is_generated(chunk))
		return;
	if (layer == 0)
		chunk->visible = frustum_check_fast(&chunk->world->frustum, &chunk->aabb);
	if (!chunk->visible)
		return;
	if (layer == 0)
	{
		if (chunk->must_update_buffers)
			update_gfx_buffers(chunk);
	}
	if (!chunk->layers[layer].indices_nb)
		return;
	gfx_bind_attributes_state(g_voxel->device, &chunk->layers[layer].attributes_state, &g_voxel->graphics->blocks.input_layout);
	gfx_draw_indexed(g_voxel->device, chunk->layers[layer].indices_nb, 0);
}

static uint8_t add_near_light_value(struct chunk *chunk, int32_t x, int32_t y, int32_t z, uint8_t light)
{
	if (x > 0)
	{
		if (y > chunk_get_top_block(chunk, x - 1, z))
			return 0xE;
	}
	else
	{
		if (chunk->chunk_x_less && y > chunk_get_top_block(chunk->chunk_x_less, CHUNK_WIDTH - 1, z))
			return 0xE;
	}
	if (x < CHUNK_WIDTH - 1)
	{
		if (y > chunk_get_top_block(chunk, x + 1, z))
			return 0xE;
	}
	else
	{
		if (chunk->chunk_x_more && y > chunk_get_top_block(chunk->chunk_x_more, 0, z))
			return 0xE;
	}
	if (z > 0)
	{
		if (y > chunk_get_top_block(chunk, x, z - 1))
			return 0xE;
	}
	else
	{
		if (chunk->chunk_z_less && y > chunk_get_top_block(chunk->chunk_z_less, x, CHUNK_WIDTH - 1))
			return 0xE;
	}
	if (z < CHUNK_WIDTH - 1)
	{
		if (y > chunk_get_top_block(chunk, x, z + 1))
			return 0xE;
	}
	else
	{
		if (chunk->chunk_z_more && y > chunk_get_top_block(chunk->chunk_z_more, x, 0))
			return 0xE;
	}
	if (x == 0 && chunk->chunk_x_less)
	{
		uint8_t near_light = chunk_get_sky_light(chunk->chunk_x_less, CHUNK_WIDTH - 1, y, z);
		if (near_light > 0)
		{
			near_light--;
			if (near_light > light)
				light = near_light;
		}
	}
	else if (x == CHUNK_WIDTH - 1 && chunk->chunk_x_more)
	{
		uint8_t near_light = chunk_get_sky_light(chunk->chunk_x_more, 0, y, z);
		if (near_light > 0)
		{
			near_light--;
			if (near_light > light)
				light = near_light;
		}
	}
	if (z == 0 && chunk->chunk_z_less)
	{
		uint8_t near_light = chunk_get_sky_light(chunk->chunk_z_less, x, y, CHUNK_WIDTH - 1);
		if (near_light > 0)
		{
			near_light--;
			if (near_light > light)
				light = near_light;
		}
	}
	else if (z == CHUNK_WIDTH - 1 && chunk->chunk_z_more)
	{
		uint8_t near_light = chunk_get_sky_light(chunk->chunk_z_more, x, y, 0);
		if (near_light > 0)
		{
			near_light--;
			if (near_light > light)
				light = near_light;
		}
	}
	return light;
}

static void set_block_light_rec(struct chunk *chunk, int32_t x, int32_t y, int32_t z, uint8_t light)
{
	struct block block;
	if (!chunk_get_block(chunk, x, y, z, &block))
		block.type = 0;
	struct block_def *block_def = g_voxel->blocks->blocks[block.type];
	if (!block_def)
		block_def = g_voxel->blocks->blocks[0];
	if (block_def->opacity == 0xF)
		return;
	bool is_top_block = false;
	if (light == 0xF)
		goto end_near_top;
	if (y >= chunk_get_top_block(chunk, x, z))
	{
		is_top_block = true;
		light = 0xF;
		goto end_near_top;
	}
#if 0
	for (int8_t xx = -1; xx <= 1; ++xx)
	{
		for (int8_t yy = -1; yy <= 1; ++yy)
		{
			for (int8_t zz = -1; zz <= 1; ++zz)
			{
				struct block tmp;
				if (world_get_block(chunk->world, chunk->x + xx + x, yy + y, chunk->z + zz + z, &tmp)
				 && tmp.type)
					goto end_check_around;
			}
		}
	}
	return;
end_check_around:
#endif
	light = add_near_light_value(chunk, x, y, z, light);
end_near_top:
	if (chunk_get_sky_light_val(chunk, x, y, z) >= light)
		return;
	chunk_set_sky_light(chunk, x, y, z, light);
	if (block_def->opacity >= light)
		return;
	light -= block_def->opacity;
	if (x > 0)
		set_block_light_rec(chunk, x - 1, y, z, light);
	else if (chunk->chunk_x_less && !is_top_block)
		{}//chunk->chunk_x_less->setBlockLightRec(CHUNK_WIDTH - 1, y, z, light);
	if (x < CHUNK_WIDTH - 1)
		set_block_light_rec(chunk, x + 1, y, z, light);
	else if (chunk->chunk_x_more && !is_top_block)
		{}//chunk->chunk_x_more->setBlockLightRec(0, y, z, light);
	if (y > 0)
		set_block_light_rec(chunk, x, y - 1, z, light);
	if (y < CHUNK_HEIGHT - 1 && !is_top_block)
		set_block_light_rec(chunk, x, y + 1, z, light);
	if (z > 0)
		set_block_light_rec(chunk, x, y, z - 1, light);
	else if (chunk->chunk_z_less && !is_top_block)
		{}//chunk->chunk_z_less->setBlockLightRec(x, y, CHUNK_WIDTH - 1, light);
	if (z < CHUNK_WIDTH - 1)
		set_block_light_rec(chunk, x, y, z + 1, light);
	else if (chunk->chunk_z_more && !is_top_block)
		{}//chunk->chunk_z_more->setBlockLightRec(x, y, 0, light);
}

void chunk_generate_light_map(struct chunk *chunk)
{
	chunk->must_generate_light_map = false;
	for (size_t i = 0; i < sizeof(chunk->storages) / sizeof(*chunk->storages); ++i)
	{
		struct storage *storage = chunk->storages[i];
		if (!storage)
			continue;
		storage_reset_lights(storage);
	}
	for (int32_t x = 0; x < CHUNK_WIDTH; ++x)
	{
		for (int32_t z = 0; z < CHUNK_WIDTH; ++z)
		{
			uint8_t top_block = chunk_get_top_block(chunk, x, z);
			if (top_block == CHUNK_HEIGHT)
				set_block_light_rec(chunk, x, top_block, z, 0xF);
			else
				set_block_light_rec(chunk, x, top_block + 1, z, 0xE);
		}
	}
	if (chunk->recursive_light_map)
	{
		if ((chunk->recursive_light_map & CHUNK_RECURSIVE_XLESS_ZLESS) && chunk->chunk_x_less_z_less)
			chunk_regenerate_light_map(chunk->chunk_x_less_z_less);
		if ((chunk->recursive_light_map & CHUNK_RECURSIVE_XMORE_ZLESS) && chunk->chunk_x_more_z_less)
			chunk_regenerate_light_map(chunk->chunk_x_more_z_less);
		if ((chunk->recursive_light_map & CHUNK_RECURSIVE_XLESS_ZMORE) && chunk->chunk_x_less_z_more)
			chunk_regenerate_light_map(chunk->chunk_x_less_z_more);
		if ((chunk->recursive_light_map & CHUNK_RECURSIVE_XMORE_ZMORE) && chunk->chunk_x_more_z_more)
			chunk_regenerate_light_map(chunk->chunk_x_more_z_more);
		if ((chunk->recursive_light_map & CHUNK_RECURSIVE_XLESS) && chunk->chunk_x_less)
			chunk_regenerate_light_map(chunk->chunk_x_less);
		if ((chunk->recursive_light_map & CHUNK_RECURSIVE_XMORE) && chunk->chunk_x_more)
			chunk_regenerate_light_map(chunk->chunk_x_more);
		if ((chunk->recursive_light_map & CHUNK_RECURSIVE_ZLESS) && chunk->chunk_z_less)
			chunk_regenerate_light_map(chunk->chunk_z_less);
		if ((chunk->recursive_light_map & CHUNK_RECURSIVE_ZMORE) && chunk->chunk_z_more)
			chunk_regenerate_light_map(chunk->chunk_z_more);
	}
	chunk->recursive_light_map = 0;
}

void chunk_generate_buffers(struct chunk *chunk)
{
	chunk->must_generate_buffers = false;
	for (uint8_t layer = 0; layer < sizeof(chunk->layers) / sizeof(*chunk->layers); ++layer)
	{
		for (uint8_t i = 0; i < sizeof(chunk->storages) / sizeof(*chunk->storages); ++i)
		{
			struct storage *storage = chunk->storages[i];
			if (!storage)
				continue;
			storage_fill_buffers(storage, chunk, &chunk->layers[layer].tessellator, layer);
		}
	}
	chunk->must_update_buffers = true;
}

static void update_gfx_buffer(struct chunk *chunk, uint8_t layer)
{
	gfx_delete_attributes_state(g_voxel->device, &chunk->layers[layer].attributes_state);
	gfx_delete_buffer(g_voxel->device, &chunk->layers[layer].vertexes_buffer);
	gfx_delete_buffer(g_voxel->device, &chunk->layers[layer].indices_buffer);
	gfx_create_buffer(g_voxel->device, &chunk->layers[layer].vertexes_buffer, GFX_BUFFER_VERTEXES, chunk->layers[layer].tessellator.vertexes.data, sizeof(struct shader_blocks_vertex) * chunk->layers[layer].tessellator.vertexes.size, GFX_BUFFER_IMMUTABLE);
	gfx_create_buffer(g_voxel->device, &chunk->layers[layer].indices_buffer, GFX_BUFFER_INDICES, chunk->layers[layer].tessellator.indices.data, sizeof(uint32_t) * chunk->layers[layer].tessellator.indices.size, GFX_BUFFER_IMMUTABLE);
	const struct gfx_attribute_bind binds[] =
	{
		{&chunk->layers[layer].vertexes_buffer},
	};
	gfx_create_attributes_state(g_voxel->device, &chunk->layers[layer].attributes_state, binds, sizeof(binds) / sizeof(*binds), &chunk->layers[layer].indices_buffer, GFX_INDEX_UINT32);
	chunk->layers[layer].indices_nb = chunk->layers[layer].tessellator.indices.size;
	jks_array_resize(&chunk->layers[layer].tessellator.vertexes, 0);
	jks_array_shrink(&chunk->layers[layer].tessellator.vertexes);
	jks_array_resize(&chunk->layers[layer].tessellator.indices, 0);
	jks_array_shrink(&chunk->layers[layer].tessellator.indices);
}

static void update_gfx_buffers(struct chunk *chunk)
{
	chunk->must_update_buffers = false;
	for (size_t layer = 0; layer < sizeof(chunk->layers) / sizeof(*chunk->layers); ++layer)
		update_gfx_buffer(chunk, layer);
}

void chunk_set_block_if_replaceable(struct chunk *chunk, int32_t x, int32_t y, int32_t z, uint16_t type)
{
	struct block block;
	if (chunk_get_block(chunk, x, y, z, &block))
	{
		struct block_def *block_def = g_voxel->blocks->blocks[block.type];
		if (block_def && !(block_def->flags & BLOCK_FLAG_REPLACEABLE))
			return;
	}
	chunk_set_block(chunk, x, y, z, type);
}

void chunk_set_block(struct chunk *chunk, int32_t x, int32_t y, int32_t z, uint16_t type)
{
	if (type)
	{
		uint8_t top = chunk_get_top_block(chunk, x, z);
		if (y > top)
			chunk_set_top_block(chunk, x, z, y);
	}
	uint8_t storage_id = y / CHUNK_STORAGE_HEIGHT;
	uint8_t storage_y = y % CHUNK_STORAGE_HEIGHT;
	struct storage *storage = chunk->storages[storage_id];
	if (!storage)
	{
		if (!type)
			return;
		storage = create_storage(chunk, storage_id);
	}
	storage_set_block(storage, x, storage_y, z, type);
	chunk->changed = true;
	if (x == 0)
	{
		chunk->recursive_light_map |= CHUNK_RECURSIVE_XLESS;
		if (z == 0)
			chunk->recursive_light_map |= CHUNK_RECURSIVE_XLESS_ZLESS;
		else if (z == CHUNK_WIDTH - 1)
			chunk->recursive_light_map |= CHUNK_RECURSIVE_XLESS_ZMORE;
	}
	else if (x == CHUNK_WIDTH - 1)
	{
		chunk->recursive_light_map |= CHUNK_RECURSIVE_XMORE;
		if (z == 0)
			chunk->recursive_light_map |= CHUNK_RECURSIVE_XMORE_ZLESS;
		else if (z == CHUNK_WIDTH - 1)
			chunk->recursive_light_map |= CHUNK_RECURSIVE_XMORE_ZMORE;
	}
	if (z == 0)
		chunk->recursive_light_map |= CHUNK_RECURSIVE_ZLESS;
	else if (z == CHUNK_WIDTH - 1)
		chunk->recursive_light_map |= CHUNK_RECURSIVE_ZMORE;
	chunk_regenerate_light_map(chunk);
}

bool chunk_get_block(struct chunk *chunk, int32_t x, int32_t y, int32_t z, struct block *block)
{
	uint8_t storage_id = y / CHUNK_STORAGE_HEIGHT;
	uint8_t storage_y = y % CHUNK_STORAGE_HEIGHT;
	struct storage *storage = chunk->storages[storage_id];
	if (!storage)
		return false;
	*block = storage_get_block(storage, x, storage_y, z);
	return true;
}

uint8_t chunk_get_light(struct chunk *chunk, int32_t x, int32_t y, int32_t z)
{
	if (!chunk_is_generated(chunk))
		return 0;
	if (y > chunk_get_top_block(chunk, x, z))
		return 0xF;
	uint8_t storage_id = y / CHUNK_STORAGE_HEIGHT;
	uint8_t storage_y = y % CHUNK_STORAGE_HEIGHT;
	struct storage *storage = chunk->storages[storage_id];
	if (!storage)
		return 0xF;
	return storage_get_light(storage, x, storage_y, z);
}

void chunk_set_sky_light(struct chunk *chunk, int32_t x, int32_t y, int32_t z, uint8_t light)
{
	uint8_t storage_id = y / CHUNK_STORAGE_HEIGHT;
	uint8_t storage_y = y % CHUNK_STORAGE_HEIGHT;
	struct storage *storage = chunk->storages[storage_id];
	if (!storage)
		storage = create_storage(chunk, storage_id);
	storage_set_sky_light(storage, x, storage_y, z, light);
	chunk->changed = true;
}

uint8_t chunk_get_sky_light_val(struct chunk *chunk, int32_t x, int32_t y, int32_t z)
{
	if (!chunk_is_generated(chunk))
		return 0;
	uint8_t storage_id = y / CHUNK_STORAGE_HEIGHT;
	uint8_t storage_y = y % CHUNK_STORAGE_HEIGHT;
	struct storage *storage = chunk->storages[storage_id];
	if (!storage)
		return 0;
	return storage_get_sky_light(storage, x, storage_y, z);
}

uint8_t chunk_get_sky_light(struct chunk *chunk, int32_t x, int32_t y, int32_t z)
{
	if (!chunk_is_generated(chunk))
		return 0;
	if (y > chunk_get_top_block(chunk, x, z))
		return 0xF;
	uint8_t storage_id = y / CHUNK_STORAGE_HEIGHT;
	uint8_t storage_y = y % CHUNK_STORAGE_HEIGHT;
	struct storage *storage = chunk->storages[storage_id];
	if (!storage)
		return 0xF;
	return storage_get_sky_light(storage, x, storage_y, z);
}

void chunk_set_block_light(struct chunk *chunk, int32_t x, int32_t y, int32_t z, uint8_t light)
{
	uint8_t storage_id = y / CHUNK_STORAGE_HEIGHT;
	uint8_t storage_y = y % CHUNK_STORAGE_HEIGHT;
	struct storage *storage = chunk->storages[storage_id];
	if (!storage)
		return;
	storage_set_block_light(storage, x, storage_y, z, light);
	chunk->changed = true;
}

uint8_t chunk_get_block_light(struct chunk *chunk, int32_t x, int32_t y, int32_t z)
{
	if (!chunk_is_generated(chunk))
		return 0;
	uint8_t storage_id = y / CHUNK_STORAGE_HEIGHT;
	uint8_t storage_y = y % CHUNK_STORAGE_HEIGHT;
	struct storage *storage = chunk->storages[storage_id];
	if (!storage)
		return 0;
	return storage_get_block_light(storage, x, storage_y, z);
}

void chunk_set_top_block(struct chunk *chunk, int32_t x, int32_t z, uint8_t top)
{
	chunk->nbt.HeightMap->value[chunk_xz_id(x, z)] = top;
}

uint8_t chunk_get_top_block(struct chunk *chunk, int32_t x, int32_t z)
{
	if (!chunk_is_generated(chunk))
		return CHUNK_HEIGHT;
	return chunk->nbt.HeightMap->value[chunk_xz_id(x, z)];
}

void chunk_set_biome(struct chunk *chunk, int32_t x, int32_t z, uint8_t biome)
{
	chunk->nbt.Biomes->value[chunk_xz_id(x, z)] = biome;
}

uint8_t chunk_get_biome(struct chunk *chunk, int32_t x, int32_t z)
{
	return chunk->nbt.Biomes->value[chunk_xz_id(x, z)];
}

void chunk_destroy_block(struct chunk *chunk, int32_t x, int32_t y, int32_t z)
{
	uint8_t top = chunk_get_top_block(chunk, x, z);
	if (y == top)
	{
		for (int16_t i = top - 1; i >= 0; --i)
		{
			struct block block;
			if (chunk_get_block(chunk, x, i, z, &block)
			 && block.type)
			{
				chunk_set_top_block(chunk, x, z, i);
				break;
			}
		}
	}
	chunk_set_block(chunk, x, y, z, 0);
	chunk_regenerate_light_map(chunk);
	if (x == 0)
	{
		if (chunk->chunk_x_less)
			chunk_regenerate_light_map(chunk->chunk_x_less);
	}
	else if (x == CHUNK_WIDTH - 1)
	{
		if (chunk->chunk_x_more)
			chunk_regenerate_light_map(chunk->chunk_x_more);
	}
	if (z == 0)
	{
		if (chunk->chunk_z_less)
			chunk_regenerate_light_map(chunk->chunk_z_less);
	}
	else if (z == CHUNK_WIDTH - 1)
	{
		if (chunk->chunk_z_more)
			chunk_regenerate_light_map(chunk->chunk_z_more);
	}
}

void chunk_regenerate_buffers(struct chunk *chunk)
{
	if (!chunk_is_generated(chunk))
		return;
	if (chunk->must_generate_buffers)
		return;
	chunk->must_generate_buffers = true;
	pthread_mutex_lock(&chunk->world->chunks_mutex);
	TAILQ_INSERT_TAIL(&chunk->world->chunks_to_update, chunk, update_chain);
	pthread_mutex_unlock(&chunk->world->chunks_mutex);
}

void chunk_regenerate_light_map(struct chunk *chunk)
{
	if (!chunk_is_generated(chunk))
		return;
	if (chunk->must_generate_light_map)
		return;
	chunk->must_generate_light_map = true;
	chunk_regenerate_buffers(chunk);
}

void chunk_regenerate_light_map_rec(struct chunk *chunk)
{
	chunk->recursive_light_map = 0xFF;
	chunk_regenerate_light_map(chunk);
}

static struct storage *create_storage(struct chunk *chunk, uint8_t id)
{
	assert(!chunk->storages[id]);
	struct nbt_tag_compound *nbt = (struct nbt_tag_compound*)nbt_tag_new(NBT_TAG_COMPOUND, "");
	if (!nbt)
	{
		LOG_ERROR("failed to create section compound");
		abort();
	}
	if (!nbt_list_add(chunk->nbt.Sections, (struct nbt_tag*)nbt))
	{
		LOG_ERROR("failed to add section nbt to list");
		abort();
	}
	chunk->storages[id] = storage_new(id, nbt);
	if (!chunk->storages[id])
	{
		LOG_ERROR("storage allocation failed");
		abort();
	}
	return chunk->storages[id];
}

static void init_nbt(struct chunk *chunk, struct nbt_tag_compound *nbt)
{
	chunk->nbt.nbt = nbt;
	if (!chunk->nbt.nbt)
	{
		chunk->nbt.nbt = (struct nbt_tag_compound*)nbt_tag_new(NBT_TAG_COMPOUND, "");
		if (!chunk->nbt.nbt)
		{
			LOG_ERROR("nbt allocation failed");
			abort();
		}
	}
	const struct nbt_sanitize_entry level_entries[] =
	{
		NBT_SANITIZE_INT(&chunk->nbt.xPos, "xPos", chunk->x),
		NBT_SANITIZE_INT(&chunk->nbt.zPos, "zPos", chunk->z),
		NBT_SANITIZE_LONG(&chunk->nbt.LastUpdate, "LastUpdate", 0),
		NBT_SANITIZE_BYTE(&chunk->nbt.LightPopulated, "LightPopulated", 0),
		NBT_SANITIZE_BYTE(&chunk->nbt.TerrainPopulated, "TerrainPopulated", 0),
		NBT_SANITIZE_BYTE(&chunk->nbt.V, "V", 0),
		NBT_SANITIZE_LONG(&chunk->nbt.InhabitedTime, "InhabitedTime", 0),
		NBT_SANITIZE_BYTE_ARRAY(&chunk->nbt.Biomes, "Biomes", CHUNK_WIDTH * CHUNK_WIDTH),
		NBT_SANITIZE_INT_ARRAY(&chunk->nbt.HeightMap, "HeightMap", CHUNK_WIDTH * CHUNK_WIDTH),
		NBT_SANITIZE_LIST(&chunk->nbt.Sections, "Sections", NBT_TAG_COMPOUND),
		NBT_SANITIZE_LIST(&chunk->nbt.Entities, "Entities", NBT_TAG_COMPOUND),
		NBT_SANITIZE_LIST(&chunk->nbt.TileEntities, "TileEntities", NBT_TAG_COMPOUND),
		NBT_SANITIZE_LIST(&chunk->nbt.TileTicks, "TileTicks", NBT_TAG_COMPOUND),
		NBT_SANITIZE_END
	};
	const struct nbt_sanitize_entry root_entries[] =
	{
		NBT_SANITIZE_INT(&chunk->nbt.DataVersion, "DataVersion", 510),
		NBT_SANITIZE_COMPOUND(&chunk->nbt.Level, "Level", level_entries),
		NBT_SANITIZE_END
	};
	if (!nbt_sanitize(chunk->nbt.nbt, root_entries))
	{
		LOG_ERROR("failed to sanitize");
		abort();
	}
	bool section_added = false;
	for (int32_t i = 0; i < chunk->nbt.Sections->count; ++i)
	{
		struct nbt_tag *tmp = chunk->nbt.Sections->tags[i];
		bool found = false;
		struct nbt_tag_compound *section = (struct nbt_tag_compound*)(tmp);
		for (int32_t j = 0; j < section->count; ++j)
		{
			if (strcmp(section->tags[j]->name, "Y"))
				continue;
			if (section->tags[j]->type != NBT_TAG_BYTE)
				break;
			struct nbt_tag_byte *Y = (struct nbt_tag_byte*)section->tags[j];
			if (Y->value >= 0 && Y->value <= 15 && !chunk->storages[Y->value])
			{
				chunk->storages[Y->value] = storage_new(Y->value, section);
				if (!chunk->storages[Y->value])
				{
					LOG_ERROR("storage allocation failed");
					abort();
				}
				found = true;
				section_added = true;
			}
			break;
		}
		if (found)
			continue;
		nbt_list_remove(chunk->nbt.Sections, i);
		nbt_tag_delete(tmp);
		i--;
	}
	if (section_added)
		chunk_regenerate_light_map_rec(chunk);
}

void chunk_set_generated(struct chunk *chunk, bool generated)
{
	chunk->nbt.TerrainPopulated->value = generated ? 1 : 0;
}

bool chunk_is_generated(struct chunk *chunk)
{
	return chunk->nbt.TerrainPopulated->value;
}

void chunk_get_aabbs(struct chunk *chunk, const struct aabb *aabb,
                     struct jks_array *aabbs)
{
	struct vec3f p0 = aabb->p0;
	struct vec3f p1 = aabb->p1;
	int32_t start_x = p0.x - chunk->x;
	if (start_x < 0)
		start_x = 0;
	else if (start_x >= CHUNK_WIDTH)
		start_x = CHUNK_WIDTH - 1;
	int32_t start_y = p0.y;
	if (start_y < 0)
		start_y = 0;
	else if (start_y >= CHUNK_HEIGHT)
		start_y = CHUNK_HEIGHT - 1;
	int32_t start_z = p0.z - chunk->z;
	if (start_z < 0)
		start_z = 0;
	else if (start_z >= CHUNK_WIDTH)
		start_z = CHUNK_WIDTH - 1;
	int32_t end_x = p1.x - chunk->x;
	if (end_x < 0)
		end_x = 0;
	else if (end_x >= CHUNK_WIDTH)
		end_x = CHUNK_WIDTH - 1;
	int32_t end_y = p1.y;
	if (end_y < 0)
		end_y = 0;
	else if (end_y >= CHUNK_HEIGHT)
		end_y = CHUNK_HEIGHT - 1;
	int32_t end_z = p1.z - chunk->z;
	if (end_z < 0)
		end_z = 0;
	else if (end_z >= CHUNK_WIDTH)
		end_z = CHUNK_WIDTH - 1;
	for (int32_t x = start_x; x <= end_x; ++x)
	{
		for (int32_t y = start_y; y <= end_y; ++y)
		{
			for (int32_t z = start_z; z <= end_z; ++z)
			{
				struct block block;
				if (!chunk_get_block(chunk, x, y, z, &block)
				 || !block.type)
					continue;
				struct block_def *block_def = g_voxel->blocks->blocks[block.type];
				if (!block_def || !(block_def->flags & BLOCK_FLAG_SOLID))
					continue;
				struct aabb ret;
				ret.p0 = block_def->aabb.p0;
				ret.p0.x += chunk->x + x;
				ret.p0.y += y;
				ret.p0.z += chunk->z + z;
				ret.p1 = block_def->aabb.p1;
				ret.p1.x += chunk->x + x;
				ret.p1.y += y;
				ret.p1.z += chunk->z + z;
				if (!jks_array_push_back(aabbs, &ret))
				{
					LOG_ERROR("failed to push aabb");
					abort();
				}
			}
		}
	}
}
