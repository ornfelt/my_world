#ifndef WORLD_REGION_H
#define WORLD_REGION_H

#include "const.h"

#include <sys/queue.h>

#include <stdint.h>
#include <stdio.h>

struct chunk;
struct world;

struct region
{
	uint8_t *sectors;
	size_t sectors_count;
	char *filename;
	struct chunk *chunks[REGION_WIDTH * REGION_WIDTH];
	struct world *world;
	uint32_t storage_timestamp[REGION_WIDTH * REGION_WIDTH];
	uint32_t storage_header[REGION_WIDTH * REGION_WIDTH];
	int32_t x;
	int32_t z;
	FILE *file;
	TAILQ_ENTRY(region) chain;
};

struct region *region_new(struct world *world, int32_t x, int32_t y);
void region_delete(struct region *region);
void region_check_clear(struct region *region);
void region_load(struct region *region);
void region_save(struct region *region);
void region_save_chunk(struct region *region, struct chunk *chunk);
void region_tick(struct region *region);
void region_draw_entities(struct region *region);
void region_draw(struct region *region, uint8_t layer);
void region_generate_chunk(struct region *region, int32_t x, int32_t z);
struct chunk *region_create_chunk(struct region *region, int32_t x, int32_t z);
void region_set_chunk(struct region *region, int32_t x, int32_t z, struct chunk *chunk);
struct chunk *region_get_chunk(struct region *region, int32_t x, int32_t z);
void region_regenerate_buffers(struct region *region);

static inline int32_t region_xz_id(int32_t x, int32_t z)
{
	return x + z * REGION_WIDTH;
}

static inline int32_t region_get_coord(int32_t coord)
{
	return coord & (~(REGION_WIDTH * CHUNK_WIDTH - 1));
}

#endif
