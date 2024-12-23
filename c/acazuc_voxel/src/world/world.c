#include "world/region.h"
#include "world/block.h"
#include "world/world.h"
#include "world/chunk.h"

#include "entity/dropped_block.h"

#include "player/player.h"

#include "block/blocks.h"
#include "block/block.h"

#include "textures.h"
#include "graphics.h"
#include "shaders.h"
#include "voxel.h"
#include "log.h"

#include <gfx/device.h>

#include <stdlib.h>
#include <math.h>

#define LOAD_DISTANCE 16

uint64_t g_seed = 1337;

static void *updater_run(void *data);
static void *loader_run(void *data);

struct world *world_new(void)
{
	struct world *world = calloc(sizeof(*world), 1);
	if (!world)
	{
		LOG_ERROR("world allocation failed");
		return NULL;
	}
	TAILQ_INIT(&world->chunks_to_update);
	world->seed = g_seed;
	world->last_region_check = g_voxel->frametime;
	TAILQ_INIT(&world->regions);
	world->player = player_new(world);
	world->blocks_uniform_buffer = GFX_BUFFER_INIT();
	gfx_create_buffer(g_voxel->device, &world->blocks_uniform_buffer, GFX_BUFFER_UNIFORM, NULL, sizeof(struct shader_blocks_model_block), GFX_BUFFER_DYNAMIC);
	frustum_init(&world->frustum);
	clouds_init(&world->clouds, world);
	skybox_init(&world->skybox, world);
	simplex_noise_init(&world->biome_temp_noise, 16, 0.70, 1334538);
	simplex_noise_init(&world->biome_rain_noise, 16, 0.70, 1222222339);
	simplex_noise_init(&world->noise, 16, 0.5, world->seed);
	pthread_mutexattr_t mutex_attr;
	if (pthread_mutexattr_init(&mutex_attr)
	 || pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE_NP)
	 || pthread_mutex_init(&world->chunks_mutex, &mutex_attr)
	 || pthread_mutexattr_destroy(&mutex_attr))
	{
		LOG_ERROR("chunks mutex creation failed");
		abort();
	}
	world->random[0] = world->seed;
	world->random[1] = world->seed >> 16;
	world->random[2] = world->seed >> 32;
	world->running = true;
	pthread_create(&world->updater_thread, NULL, updater_run, world);
	pthread_create(&world->loader_thread, NULL, loader_run, world);
	return world;
}

void world_delete(struct world *world)
{
	if (!world)
		return;
	world->running = false;
	pthread_join(world->updater_thread, NULL);
	pthread_join(world->loader_thread, NULL);
	struct region *region = TAILQ_FIRST(&world->regions);
	while (region)
	{
		TAILQ_REMOVE(&world->regions, region, chain);
		region_delete(region);
		region = TAILQ_FIRST(&world->regions);
	}
	simplex_noise_destroy(&world->biome_temp_noise);
	simplex_noise_destroy(&world->biome_rain_noise);
	simplex_noise_destroy(&world->noise);
	clouds_destroy(&world->clouds);
	skybox_destroy(&world->skybox);
	frustum_destroy(&world->frustum);
	gfx_delete_buffer(g_voxel->device, &world->blocks_uniform_buffer);
	pthread_mutex_destroy(&world->chunks_mutex);
	entity_delete(&world->player->entity);
	free(world);
}

void world_save(struct world *world)
{
	struct region *region;
	TAILQ_FOREACH(region, &world->regions, chain)
		region_save(region);
}

static void check_empty_regions(struct world *world)
{
	if (g_voxel->frametime - world->last_region_check < 5000000000)
		return;
	world->last_region_check = g_voxel->frametime;
	struct region *region = TAILQ_FIRST(&world->regions);
	while (region)
	{
		struct region *next = TAILQ_NEXT(region, chain);
		region_check_clear(region);
		region = next;
	}
}

void world_tick(struct world *world)
{
	pthread_mutex_lock(&world->chunks_mutex);
	check_empty_regions(world);
	struct region *region;
	TAILQ_FOREACH(region, &world->regions, chain)
		region_tick(region);
	entity_tick(&world->player->entity);
	clouds_tick(&world->clouds);
	pthread_mutex_unlock(&world->chunks_mutex);
}

void world_draw(struct world *world)
{
	pthread_mutex_lock(&world->chunks_mutex);
	player_update(world->player);
	VEC4_SET(world->sky_color, 0.71, 0.82, 1, 1);
	if (world->player->eye_in_water)
	{
		VEC4_SET(world->fog_color, 0, 0, 0.05, 1);
		world->fog_distance = 0;
		world->fog_density = 0.2 - 0.1 * (world->player->eye_light / 15.0);
	}
	else
	{
		VEC4_CPY(world->fog_color, world->sky_color);
		world->fog_distance = 16 * 8;
		world->fog_density = 0.1;
	}
	struct shader_blocks_model_block model_block;
	model_block.mvp = world->player->mat_vp;
	model_block.mv = world->player->mat_v;
	VEC4_CPY(model_block.fog_color, world->fog_color);
	model_block.fog_distance = world->fog_distance;
	model_block.fog_density = world->fog_density;
	model_block.time_factor = g_voxel->frametime / 1000000000.0;
	model_block.disable_tex = g_voxel->disable_tex;
	gfx_set_buffer_data(&world->blocks_uniform_buffer, &model_block, sizeof(model_block), 0);
	gfx_bind_pipeline_state(g_voxel->device, &g_voxel->graphics->blocks.first_pipeline_state);
	gfx_bind_constant(g_voxel->device, 1, &world->blocks_uniform_buffer, sizeof(model_block), 0);
	texture_bind(&g_voxel->textures->terrain);
	struct region *region;
	TAILQ_FOREACH(region, &world->regions, chain)
		region_draw(region, 0);
	TAILQ_FOREACH(region, &world->regions, chain)
		region_draw(region, 1);
	entity_draw(&world->player->entity);
	skybox_draw(&world->skybox);
	TAILQ_FOREACH(region, &world->regions, chain)
		region_draw_entities(region);
	gfx_bind_pipeline_state(g_voxel->device, &g_voxel->graphics->blocks.second_pipeline_state);
	texture_bind(&g_voxel->textures->terrain);
	gfx_bind_constant(g_voxel->device, 1, &world->blocks_uniform_buffer, sizeof(model_block), 0);
	TAILQ_FOREACH(region, &world->regions, chain)
		region_draw(region, 2);
	clouds_draw(&world->clouds);
	pthread_mutex_unlock(&world->chunks_mutex);
}

void world_get_aabbs(struct world *world, const struct aabb *aabb,
                     struct jks_array *aabbs)
{
	if (aabb->p1.y < 0 || aabb->p0.y >= CHUNK_HEIGHT)
		return;
	struct vec3f p0 = aabb->p0;
	struct vec3f p1 = aabb->p1;
	int32_t chunk_start_x = chunk_get_coord(floorf(p0.x));
	int32_t chunk_end_x = chunk_get_coord(ceilf(p1.x));
	int32_t chunk_start_z = chunk_get_coord(floorf(p0.z));
	int32_t chunk_end_z = chunk_get_coord(ceilf(p1.z));
	for (int32_t chunk_x = chunk_start_x; chunk_x <= chunk_end_x; chunk_x += CHUNK_WIDTH)
	{
		for (int32_t chunk_z = chunk_start_z; chunk_z <= chunk_end_z; chunk_z += CHUNK_WIDTH)
		{
			struct chunk *chunk = world_get_chunk(world, chunk_x, chunk_z);
			if (!chunk)
				continue;
			chunk_get_aabbs(chunk, aabb, aabbs);
		}
	}
}

void world_set_block(struct world *world, int32_t x, int32_t y, int32_t z, uint16_t type)
{
	if (y < 0 || y >= CHUNK_HEIGHT)
		return;
	struct chunk *chunk = world_get_chunk(world, x, z);
	if (!chunk)
	{
		int32_t chunk_x = chunk_get_coord(x);
		int32_t chunk_z = chunk_get_coord(z);
		chunk = world_create_chunk(world, chunk_x, chunk_z);
	}
	chunk_set_block(chunk, x - chunk->x, y, z - chunk->z, type);
}

void world_set_block_if_replaceable(struct world *world, int32_t x, int32_t y, int32_t z, uint16_t type)
{
	if (y < 0 || y >= CHUNK_HEIGHT)
		return;
	struct chunk *chunk = world_get_chunk(world, x, z);
	if (!chunk)
	{
		int32_t chunk_x = chunk_get_coord(x);
		int32_t chunk_z = chunk_get_coord(z);
		chunk = world_create_chunk(world, chunk_x, chunk_z);
	}
	chunk_set_block_if_replaceable(chunk, x - chunk->x, y, z - chunk->z, type);
}

static struct region *fetch_region(struct world *world, int32_t x, int32_t z)
{
	int32_t region_x = region_get_coord(x);
	int32_t region_z = region_get_coord(z);
	struct region *region;
	TAILQ_FOREACH(region, &world->regions, chain)
	{
		if (region->x == region_x && region->z == region_z)
			return region;
	}
	region = region_new(world, region_x, region_z);
	if (!region)
		abort();
	TAILQ_INSERT_TAIL(&world->regions, region, chain);
	return region;
}

void world_generate_chunk(struct world *world, int32_t x, int32_t z)
{
	struct region *region = fetch_region(world, x, z);
	region_generate_chunk(region,
	                      (x - region->x) / CHUNK_WIDTH,
	                      (z - region->z) / CHUNK_WIDTH);
}

struct chunk *world_create_chunk(struct world *world, int32_t x, int32_t z)
{
	struct region *region = fetch_region(world, x, z);
	return region_create_chunk(region,
	                           (x - region->x) / CHUNK_WIDTH,
	                           (z - region->z) / CHUNK_WIDTH);
}

void world_add_chunk(struct world *world, struct chunk *chunk)
{
	struct region *region = fetch_region(world, chunk->x, chunk->z);
	region_set_chunk(region,
	                 (chunk->x - region->x) / CHUNK_WIDTH,
	                 (chunk->z - region->z) / CHUNK_WIDTH, chunk);
}

struct chunk *world_get_chunk(struct world *world, int32_t x, int32_t z)
{
	int32_t region_x = region_get_coord(x);
	int32_t region_z = region_get_coord(z);
	struct region *region;
	TAILQ_FOREACH(region, &world->regions, chain)
	{
		if (region->x == region_x && region->z == region_z)
			return region_get_chunk(region,
			                        (x - region_x) / CHUNK_WIDTH,
			                        (z - region_z) / CHUNK_WIDTH);
	}
	return NULL;
}

bool world_get_block(struct world *world, int32_t x, int32_t y, int32_t z, struct block *block)
{
	if (y < 0 || y >= CHUNK_HEIGHT)
		return false;
	struct chunk *chunk = world_get_chunk(world, x, z);
	if (!chunk)
		return false;
	return chunk_get_block(chunk, x - chunk->x, y, z - chunk->z, block);
}

uint8_t world_get_light(struct world *world, int32_t x, int32_t y, int32_t z)
{
	if (y < 0 || y >= CHUNK_HEIGHT)
		return 0;
	struct chunk *chunk = world_get_chunk(world, x, z);
	if (!chunk)
		return 0;
	return chunk_get_light(chunk, x - chunk->x, y, z - chunk->z);
}

void world_regenerate_buffers(struct world *world)
{
	struct region *region;
	TAILQ_FOREACH(region, &world->regions, chain)
		region_regenerate_buffers(region);
}

static void *updater_run(void *data)
{
	int64_t last_update = g_voxel->frametime / 1000000000 * 1000000000;
	int64_t updates_count = 0;
	struct world *world = data;
	struct timespec ts;
	while (world->running)
	{
		if (g_voxel->frametime - last_update >= 1000000000)
		{
			last_update += 1000000000;
			g_voxel->chunk_updates = updates_count;
			updates_count = 0;
		}
		pthread_mutex_lock(&world->chunks_mutex);
		if (TAILQ_EMPTY(&world->chunks_to_update))
		{
			pthread_mutex_unlock(&world->chunks_mutex);
			ts.tv_sec = 0;
			ts.tv_nsec = 20000000;
			nanosleep(&ts, NULL);
			continue;
		}
		struct chunk *chunk = TAILQ_FIRST(&world->chunks_to_update);
		TAILQ_REMOVE(&world->chunks_to_update, chunk, update_chain);
		if (chunk->must_generate_light_map)
		{
			++updates_count;
			chunk_generate_light_map(chunk);
			chunk_generate_buffers(chunk);
		}
		else if (chunk->must_generate_buffers)
		{
			++updates_count;
			chunk_generate_buffers(chunk);
		}
		else
		{
			pthread_mutex_unlock(&world->chunks_mutex);
			continue;
		}
		pthread_mutex_unlock(&world->chunks_mutex);
		ts.tv_sec = 0;
		ts.tv_nsec = 5000;
		nanosleep(&ts, NULL);
	}
	return NULL;
}

static bool loader_check_chunk(struct world *world, const struct frustum *frustum, int32_t chunk_x, int32_t chunk_z)
{
	if (!world->running)
		return true;
	int32_t part1 = world->player->entity.pos.x - (chunk_x + CHUNK_WIDTH / 2);
	int32_t part2 = world->player->entity.pos.z - (chunk_z + CHUNK_WIDTH / 2);
	int32_t distance = sqrtf(part1 * part1 + part2 * part2);
	if (distance > LOAD_DISTANCE * CHUNK_WIDTH)
		return false;
	if (distance > 2 * 16)
	{
		struct aabb aabb;
		VEC3_SET(aabb.p0, chunk_x, 0, chunk_z);
		VEC3_SET(aabb.p1, chunk_x + CHUNK_WIDTH, CHUNK_HEIGHT, chunk_z + CHUNK_WIDTH);
		if (!frustum_check_fast(frustum, &aabb))
			return false;
	}
	pthread_mutex_lock(&world->chunks_mutex);
	world_generate_chunk(world, chunk_x, chunk_z);
	pthread_mutex_unlock(&world->chunks_mutex);
	sched_yield();
	return true;
}

static void check_out_of_range_chunks(struct world *world, int32_t player_x, int32_t player_z)
{
	struct region *region = TAILQ_FIRST(&world->regions);
	while (region)
	{
		struct chunk **chunks = region->chunks;
		for (size_t i = 0; i < REGION_WIDTH * REGION_WIDTH; ++i)
		{
			struct chunk *chunk = chunks[i];
			if (!chunk)
				continue;
			int32_t part1 = player_x - (chunk->x + CHUNK_WIDTH / 2);
			int32_t part2 = player_z - (chunk->z + CHUNK_WIDTH / 2);
			int32_t distance = sqrtf(part1 * part1 + part2 * part2);
			if (distance < LOAD_DISTANCE * 1.5 * CHUNK_WIDTH)
				continue;
			pthread_mutex_lock(&world->chunks_mutex);
			region_set_chunk(region,
			                 (chunk->x - region->x) / CHUNK_WIDTH,
			                 (chunk->z - region->z) / CHUNK_WIDTH,
			                 NULL);
			struct chunk *to_update = TAILQ_FIRST(&world->chunks_to_update);
			while (to_update)
			{
				struct chunk *next = TAILQ_NEXT(to_update, update_chain);
				if (to_update == chunk)
					TAILQ_REMOVE(&world->chunks_to_update, to_update, update_chain);
				to_update = next;
			}
			chunk_delete(chunk);
			pthread_mutex_unlock(&world->chunks_mutex);
		}
		struct region *next = TAILQ_NEXT(region, chain);
		region_check_clear(region);
		region = next;
	}
}

static void *loader_run(void *data)
{
	struct frustum frustum;
	frustum_init(&frustum);
	struct world *world = data;
	while (world->running)
	{
		pthread_mutex_lock(&world->chunks_mutex);
		int32_t player_x = world->player->entity.pos.x;
		int32_t player_z = world->player->entity.pos.z;
		frustum_update(&frustum, &world->player->mat_vp);
		pthread_mutex_unlock(&world->chunks_mutex);
		int32_t player_chunk_x = chunk_get_coord(player_x);
		int32_t player_chunk_z = chunk_get_coord(player_z);
		check_out_of_range_chunks(world, player_x, player_z);
		loader_check_chunk(world, &frustum, player_chunk_x, player_chunk_z);
		for (int32_t i = 0; i <= LOAD_DISTANCE; ++i)
		{
			int32_t chunk_x = player_chunk_x - i * CHUNK_WIDTH;
			int32_t chunk_z = player_chunk_z - i * CHUNK_WIDTH;
			loader_check_chunk(world, &frustum, chunk_x, chunk_z);
			for (int32_t j = 0; j <= i * 2; ++j)
			{
				chunk_x += CHUNK_WIDTH;
				loader_check_chunk(world, &frustum, chunk_x, chunk_z);
			}
			for (int32_t j = 0; j <= i * 2; ++j)
			{
				chunk_z += CHUNK_WIDTH;
				loader_check_chunk(world, &frustum, chunk_x, chunk_z);
			}
			for (int32_t j = 0; j <= i * 2; ++j)
			{
				chunk_x -= CHUNK_WIDTH;
				loader_check_chunk(world, &frustum, chunk_x, chunk_z);
			}
			for (int32_t j = 0; j <= i * 2 - 1; ++j)
			{
				chunk_z -= CHUNK_WIDTH;
				loader_check_chunk(world, &frustum, chunk_x, chunk_z);
			}
		}
		{
			struct timespec ts;
			ts.tv_sec = 0;
			ts.tv_nsec = 100000000;
			nanosleep(&ts, NULL);
		}
	}
	frustum_destroy(&frustum);
	return NULL;
}
