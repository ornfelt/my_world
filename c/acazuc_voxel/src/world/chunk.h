#ifndef WORLD_CHUNK_H
#define WORLD_CHUNK_H

#include "particle/particles.h"

#include "entity/entities.h"

#include "world/tessellator.h"

#include "const.h"

#include <nbt/nbt.h>

#include <jks/aabb.h>

#define CHUNK_RECURSIVE_XLESS 0x1
#define CHUNK_RECURSIVE_XMORE 0x2
#define CHUNK_RECURSIVE_ZLESS 0x4
#define CHUNK_RECURSIVE_ZMORE 0x8
#define CHUNK_RECURSIVE_XLESS_ZLESS 0x10
#define CHUNK_RECURSIVE_XMORE_ZLESS 0x20
#define CHUNK_RECURSIVE_XLESS_ZMORE 0x40
#define CHUNK_RECURSIVE_XMORE_ZMORE 0x80

struct storage;
struct block;
struct world;

struct chunk_nbt
{
	struct nbt_tag_compound *nbt;
	struct nbt_tag_int *DataVersion;
	struct nbt_tag_compound *Level;
	struct nbt_tag_int *xPos;
	struct nbt_tag_int *zPos;
	struct nbt_tag_long *LastUpdate;
	struct nbt_tag_byte *LightPopulated;
	struct nbt_tag_byte *TerrainPopulated;
	struct nbt_tag_byte *V;
	struct nbt_tag_long *InhabitedTime;
	struct nbt_tag_byte_array *Biomes;
	struct nbt_tag_int_array *HeightMap;
	struct nbt_tag_list *Sections;
	struct nbt_tag_list *Entities;
	struct nbt_tag_list *TileEntities;
	struct nbt_tag_list *TileTicks;
};

struct chunk_layer
{
	struct tessellator tessellator;
	gfx_attributes_state_t attributes_state;
	gfx_buffer_t vertexes_buffer;
	gfx_buffer_t indices_buffer;
	uint32_t indices_nb;
};

struct chunk
{
	struct storage *storages[16];
	struct chunk_layer layers[3];
	struct particles particles;
	struct entities entities;
	struct chunk_nbt nbt;
	struct chunk *chunk_x_less_z_less;
	struct chunk *chunk_x_less_z_more;
	struct chunk *chunk_x_more_z_less;
	struct chunk *chunk_x_more_z_more;
	struct chunk *chunk_x_less;
	struct chunk *chunk_x_more;
	struct chunk *chunk_z_less;
	struct chunk *chunk_z_more;
	struct world *world;
	struct aabb aabb;
	uint8_t recursive_light_map;
	int32_t x;
	int32_t z;
	bool must_generate_light_map;
	bool must_generate_buffers;
	bool must_update_buffers;
	bool deleted;
	bool changed;
	bool visible;
	TAILQ_ENTRY(chunk) update_chain;
};

struct chunk *chunk_new(struct world *world, int32_t x, int32_t y, struct nbt_tag_compound *nbt);
void chunk_delete(struct chunk *chunk);
void chunk_tick(struct chunk *chunk);
void chunk_generate(struct chunk *chunk);
void chunk_draw_entities(struct chunk *chunk);
void chunk_draw(struct chunk *chunk, uint8_t layer);
void chunk_generate_buffers(struct chunk *chunk);
void chunk_regenerate_buffers(struct chunk *chunk);
void chunk_generate_light_map(struct chunk *chunk);
void chunk_regenerate_light_map_rec(struct chunk *chunk);
void chunk_regenerate_light_map(struct chunk *chunk);
void chunk_set_block(struct chunk *chunk, int32_t x, int32_t y, int32_t z, uint16_t type);
void chunk_set_block_if_replaceable(struct chunk *chunk, int32_t x, int32_t y, int32_t z, uint16_t type);
bool chunk_get_block(struct chunk *chunk, int32_t x, int32_t y, int32_t z, struct block *block);
uint8_t chunk_get_light(struct chunk *chunk, int32_t x, int32_t y, int32_t z);
void chunk_set_sky_light(struct chunk *chunk, int32_t x, int32_t y, int32_t z, uint8_t light);
uint8_t chunk_get_sky_light_val(struct chunk *chunk, int32_t x, int32_t y, int32_t z);
uint8_t chunk_get_sky_light(struct chunk *chunk, int32_t x, int32_t y, int32_t z);
void chunk_set_block_light(struct chunk *chunk, int32_t x, int32_t y, int32_t z, uint8_t light);
uint8_t chunk_get_block_light(struct chunk *chunk, int32_t x, int32_t y, int32_t z);
void chunk_set_top_block(struct chunk *chunk, int32_t x, int32_t y, uint8_t top);
uint8_t chunk_get_top_block(struct chunk *chunk, int32_t x, int32_t z);
void chunk_set_biome(struct chunk *chunk, int32_t x, int32_t y, uint8_t biome);
uint8_t chunk_get_biome(struct chunk *chunk, int32_t x, int32_t z);
void chunk_destroy_block(struct chunk *chunk, int32_t x, int32_t y, int32_t z);
void chunk_set_generated(struct chunk *chunk, bool generated);
bool chunk_is_generated(struct chunk *chunk);
void chunk_get_aabbs(struct chunk *chunk, const struct aabb *aabb,
                     struct jks_array *aabbs);

static inline int32_t chunk_get_coord(int32_t coord)
{
	return coord & (~(CHUNK_WIDTH - 1));
}

static inline int32_t chunk_xyz_id(int32_t x, int32_t y, int32_t z)
{
	return (x * CHUNK_HEIGHT + y) * CHUNK_WIDTH + z;
}

static inline int32_t chunk_xz_id(int32_t x, int32_t z)
{
	return x * CHUNK_WIDTH + z;
}

#endif
