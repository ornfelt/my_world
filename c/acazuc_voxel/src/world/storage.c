#include "world/storage.h"
#include "world/block.h"
#include "world/chunk.h"

#include "log.h"

#include <string.h>
#include <stdlib.h>

struct storage *storage_new(uint8_t id, struct nbt_tag_compound *nbt)
{
	struct storage *storage = calloc(sizeof(*storage), 1);
	if (!storage)
	{
		LOG_ERROR("storage allocation failed");
		return NULL;
	}
	storage->id = id;
	storage->nbt.nbt = nbt;
	const struct nbt_sanitize_entry entries[] =
	{
		NBT_SANITIZE_BYTE(&storage->nbt.Y, "Y", (int8_t)storage->id),
		NBT_SANITIZE_BYTE_ARRAY(&storage->nbt.Blocks, "Blocks", CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_STORAGE_HEIGHT),
		NBT_SANITIZE_BYTE_ARRAY(&storage->nbt.Add, "Add", CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_STORAGE_HEIGHT / 2),
		NBT_SANITIZE_BYTE_ARRAY(&storage->nbt.Data, "Data", CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_STORAGE_HEIGHT / 2),
		NBT_SANITIZE_BYTE_ARRAY(&storage->nbt.BlockLight, "BlockLight", CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_STORAGE_HEIGHT / 2),
		NBT_SANITIZE_BYTE_ARRAY(&storage->nbt.SkyLight, "SkyLight", CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_STORAGE_HEIGHT / 2),
		NBT_SANITIZE_END
	};
	if (!nbt_sanitize(storage->nbt.nbt, entries))
	{
		LOG_ERROR("failed to sanitize");
		abort();
	}
	return storage;
}

void storage_delete(struct storage *storage)
{
	if (!storage)
		return;
	free(storage);
}

void storage_fill_buffers(struct storage *storage, struct chunk *chunk, struct tessellator *tessellator, uint8_t layer)
{
	for (int32_t x = 0; x < CHUNK_WIDTH; ++x)
	{
		for (int32_t y = 0; y < CHUNK_STORAGE_HEIGHT; ++y)
		{
			for (int32_t z = 0; z < CHUNK_WIDTH; ++z)
			{
				struct block block = storage_get_block(storage, x, y, z);
				if (!block.type)
					continue;
				struct vec3f pos;
				VEC3_SET(pos, chunk->x + x, storage->id * CHUNK_STORAGE_HEIGHT + y, chunk->z + z);
				block_fill_buffers(block, chunk, pos, tessellator, layer);
			}
		}
	}
}

void storage_reset_lights(struct storage *storage)
{
	memset(storage->nbt.BlockLight->value, 0, storage->nbt.BlockLight->count);
	memset(storage->nbt.SkyLight->value, 0, storage->nbt.SkyLight->count);
}

void storage_set_block(struct storage *storage, int32_t x, int32_t y, int32_t z, uint16_t type)
{
	uint32_t id = storage_xyz_id(x, y, z);
	storage->nbt.Blocks->value[id] = type & 0xFF;
	if (id & 1)
		storage->nbt.Add->value[id / 2] = (uint8_t)(((uint8_t)storage->nbt.Add->value[id / 2] & 0xF0) | ((uint8_t)(type >> 8) & 0x0F));
	else
		storage->nbt.Add->value[id / 2] = (uint8_t)(((uint8_t)storage->nbt.Add->value[id / 2] & 0x0F) | ((uint8_t)(type >> 4) & 0xF0));
}

struct block storage_get_block(struct storage *storage, int32_t x, int32_t y, int32_t z)
{
	struct block block;
	uint32_t id = storage_xyz_id(x, y, z);
	block.type = (uint8_t)storage->nbt.Blocks->value[id];
	if (storage->nbt.Add)
	{
		if (id & 1)
			block.type |= ((uint8_t)storage->nbt.Add->value[id / 2] & 0x0F) << 8;
		else
			block.type |= ((uint8_t)storage->nbt.Add->value[id / 2] & 0xF0) << 4;
	}
	if (storage->nbt.Data)
	{
		if (id & 1)
			block.data = ((uint8_t)storage->nbt.Data->value[id / 2] & 0x0F);
		else
			block.data = ((uint8_t)storage->nbt.Data->value[id / 2] & 0xF0) >> 4;
	}
	else
	{
		block.data = 0;
	}
	return block;
}

uint8_t storage_get_light(struct storage *storage, int32_t x, int32_t y, int32_t z)
{
	uint8_t sky_light = storage_get_sky_light(storage, x, y, z);
	uint8_t block_light = storage_get_block_light(storage, x, y, z);
	if (block_light > sky_light)
		return block_light;
	return sky_light;
}

void storage_set_sky_light(struct storage *storage, int32_t x, int32_t y, int32_t z, uint8_t light)
{
	uint32_t id = storage_xyz_id(x, y, z);
	if (id & 1)
		storage->nbt.SkyLight->value[id / 2] = ((uint8_t)storage->nbt.SkyLight->value[id / 2] & 0xF0) | (light & 0xF);
	else
		storage->nbt.SkyLight->value[id / 2] = ((uint8_t)storage->nbt.SkyLight->value[id / 2] & 0x0F) | ((light & 0xF) << 4);
}

uint8_t storage_get_sky_light(struct storage *storage, int32_t x, int32_t y, int32_t z)
{
	uint32_t id = storage_xyz_id(x, y, z);
	if (id & 1)
		return (uint8_t)storage->nbt.SkyLight->value[id / 2] & 0xF;
	return ((uint8_t)storage->nbt.SkyLight->value[id / 2] >> 4) & 0xF;
}

void storage_set_block_light(struct storage *storage, int32_t x, int32_t y, int32_t z, uint8_t light)
{
	uint32_t id = storage_xyz_id(x, y, z);
	if (id & 1)
		storage->nbt.BlockLight->value[id / 2] = ((uint8_t)storage->nbt.BlockLight->value[id / 2] & 0xF0) | (light & 0xF);
	else
		storage->nbt.BlockLight->value[id / 2] = ((uint8_t)storage->nbt.BlockLight->value[id / 2] & 0x0F) | ((light & 0xF) << 4);
}

uint8_t storage_get_block_light(struct storage *storage, int32_t x, int32_t y, int32_t z)
{
	uint32_t id = storage_xyz_id(x, y, z);
	if (id & 1)
		return (uint8_t)storage->nbt.BlockLight->value[id / 2] & 0xF;
	return ((uint8_t)storage->nbt.BlockLight->value[id / 2] >> 4) & 0xF;
}
