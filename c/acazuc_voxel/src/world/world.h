#ifndef WORLD_WORLD_H
#define WORLD_WORLD_H

#include "noise/simplex.h"

#include "world/clouds.h"
#include "world/skybox.h"

#include <jks/frustum.h>

#include <sys/queue.h>

#include <pthread.h>

struct player;
struct region;
struct chunk;
struct block;

struct world
{
	pthread_mutex_t chunks_mutex;
	pthread_t updater_thread;
	pthread_t loader_thread;
	TAILQ_HEAD(, chunk) chunks_to_update;
	uint16_t random[3];
	struct simplex_noise biome_temp_noise;
	struct simplex_noise biome_rain_noise;
	struct simplex_noise noise;
	gfx_buffer_t blocks_uniform_buffer;
	TAILQ_HEAD(, region) regions;
	struct frustum frustum;
	struct clouds clouds;
	struct skybox skybox;
	struct player *player;
	struct vec4f sky_color;
	struct vec4f fog_color;
	float fog_distance;
	float fog_density;
	uint64_t seed;
	int64_t last_region_check;
	bool running;
};

struct world *world_new(void);
void world_delete(struct world *world);
void world_save(struct world *world);
void world_tick(struct world *world);
void world_draw(struct world *world);
void world_get_aabbs(struct world *world, const struct aabb *aabb, struct jks_array *aabbs);
void world_set_block(struct world *world, int32_t x, int32_t y, int32_t z, uint16_t type);
void world_set_block_if_replaceable(struct world *world, int32_t x, int32_t y, int32_t z, uint16_t type);
void world_generate_chunk(struct world *world, int32_t x, int32_t y);
struct chunk *world_create_chunk(struct world *world, int32_t x, int32_t y);
void world_add_chunk(struct world *world, struct chunk *chunk);
struct chunk *world_get_chunk(struct world *world, int32_t x, int32_t y);
bool world_get_block(struct world *world, int32_t x, int32_t y, int32_t z, struct block *block);
uint8_t world_get_light(struct world *world, int32_t x, int32_t y, int32_t z);
void world_regenerate_buffers(struct world *world);

#endif
