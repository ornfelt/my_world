#ifndef WORLD_STORAGE_H
#define WORLD_STORAGE_H

#include "world/block.h"

#include "const.h"

#include <nbt/nbt.h>

#include <stdint.h>

#define CHUNK_STORAGE_HEIGHT 16

struct tessellator;
struct block;
struct chunk;

struct storage_nbt
{
	struct nbt_tag_compound *nbt;
	struct nbt_tag_byte *Y;
	struct nbt_tag_byte_array *Blocks;
	struct nbt_tag_byte_array *Add;
	struct nbt_tag_byte_array *Data;
	struct nbt_tag_byte_array *BlockLight;
	struct nbt_tag_byte_array *SkyLight;
};

struct storage
{
	struct storage_nbt nbt;
	uint8_t id;
};

struct storage *storage_new(uint8_t id, struct nbt_tag_compound *nbt);
void storage_delete(struct storage *storage);
void storage_fill_buffers(struct storage *storage, struct chunk *chunk, struct tessellator *tessellator, uint8_t layer);
void storage_reset_lights(struct storage *storage);
void storage_set_block(struct storage *storage, int32_t x, int32_t y, int32_t z, uint16_t type);
struct block storage_get_block(struct storage *storage, int32_t x, int32_t y, int32_t z);
uint8_t storage_get_light(struct storage *storage, int32_t x, int32_t y, int32_t z);
void storage_set_sky_light(struct storage *storage, int32_t x, int32_t y, int32_t z, uint8_t light);
uint8_t storage_get_sky_light(struct storage *storage, int32_t x, int32_t y, int32_t z);
void storage_set_block_light(struct storage *storage, int32_t x, int32_t y, int32_t z, uint8_t light);
uint8_t storage_get_block_light(struct storage *storage, int32_t x, int32_t y, int32_t z);

static inline uint32_t storage_xyz_id(int32_t x, int32_t y, int32_t z)
{
	return (y * CHUNK_WIDTH + z) * CHUNK_WIDTH + x;
}

#endif
