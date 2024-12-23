#ifndef MAP_TILE_H
#define MAP_TILE_H

#include "refcount.h"

#include <jks/vec3.h>
#include <jks/mat4.h>

#include <stdbool.h>
#include <stdint.h>

#define CHUNKS_PER_TILE 256

enum map_tile_flag
{
	MAP_TILE_FLAG_INITIALIZED    = (1 << 0),
	MAP_TILE_FLAG_LOADING        = (1 << 1),
	MAP_TILE_FLAG_LOADED         = (1 << 2),
	MAP_TILE_FLAG_DOODADS_AABB   = (1 << 3),
	MAP_TILE_FLAG_WMOS_AABB      = (1 << 4),
	MAP_TILE_FLAG_IN_RENDER_LIST = (1 << 5),
};

struct map_chunk_ground_effect;
struct collision_params;
struct collision_state;
struct gx_wmo_instance;
struct gx_m2_instance;
struct wow_adt_file;
struct jks_array;
struct gx_frame;
struct gx_mcnk;
struct gx_mclq;

struct map_wmo
{
	struct gx_wmo_instance *instance;
	refcount_t refcount;
	uint32_t id;
};

struct map_m2
{
	struct gx_m2_instance *instance;
	refcount_t refcount;
	uint32_t id;
};

struct map_chunk
{
	float height[145];
	int8_t norm[145 * 3];
	union
	{
		uint8_t no_effect_doodad[8];
		uint64_t no_effect_doodads;
	};
	uint16_t effect_id[8][8];
	uint64_t shadow[64];
	uint32_t *doodads;
	uint32_t *wmos;
	uint32_t doodads_nb;
	uint32_t wmos_nb;
	uint16_t holes;
	struct jks_array ground_effects; /* struct map_chunk_ground_effect */
	struct jks_array doodads_to_aabb; /* uint32_t */
	struct jks_array wmos_to_aabb; /* uint32_t */
	struct aabb objects_aabb;
	struct aabb doodads_aabb;
	struct aabb wmos_aabb;
	struct vec3f center;
	struct aabb aabb;
	enum frustum_result doodads_frustum_result;
	enum frustum_result wmos_frustum_result;
#ifdef WITH_DEBUG_RENDERING
	struct gx_aabb doodads_gx_aabb;
	struct gx_aabb wmos_gx_aabb;
	struct gx_aabb gx_aabb;
#endif
	bool ground_effect_loaded;
};

struct map_tile
{
	struct map_chunk chunks[CHUNKS_PER_TILE];
	struct map_wmo **wmo;
	uint32_t wmo_nb;
	struct map_m2 **m2;
	uint32_t m2_nb;
	struct gx_mcnk *gx_mcnk;
	struct gx_mclq *gx_mclq;
	struct jks_array ground_effects; /* struct map_tile_ground_effect */
	enum map_tile_flag flags;
	char *filename;
	struct vec3f pos;
	struct aabb objects_aabb;
	struct aabb doodads_aabb;
	struct aabb wmos_aabb;
	struct vec3f center;
	struct aabb aabb;
	int32_t x;
	int32_t z;
	enum frustum_result doodads_frustum_result;
	enum frustum_result wmos_frustum_result;
#ifdef WITH_DEBUG_RENDERING
	struct gx_aabb doodads_gx_aabb;
	struct gx_aabb wmos_gx_aabb;
	struct gx_aabb gx_aabb;
#endif
	refcount_t refcount;
};

static inline bool map_tile_flag_get(struct map_tile *tile, enum map_tile_flag flag)
{
	return (__atomic_load_n(&tile->flags, __ATOMIC_RELAXED) & flag) != 0;
}

static inline bool map_tile_flag_set(struct map_tile *tile, enum map_tile_flag flag)
{
	return (__atomic_fetch_or(&tile->flags, flag, __ATOMIC_RELAXED) & flag) != 0;
}

static inline bool map_tile_flag_clear(struct map_tile *tile, enum map_tile_flag flag)
{
	return (__atomic_fetch_and(&tile->flags, ~flag, __ATOMIC_RELAXED) & flag) != 0;
}

struct map_tile *map_tile_new(const char *filename, int32_t x, int32_t z);
void map_tile_free(struct map_tile *tile);
void map_tile_ref(struct map_tile *tile);
void map_tile_ask_load(struct map_tile *tile);
void map_tile_cull(struct map_tile *tile, struct gx_frame *frame);
void map_tile_collect_collision_triangles(struct map_tile *tile, const struct collision_params *params, struct collision_state *state, struct jks_array *triangles);
void map_tile_ground_end(struct map_tile *tile, struct gx_frame *frame);
void map_tile_ground_clear(struct map_tile *tile, struct gx_frame *frame);

struct map_wmo *map_wmo_new(uint32_t id);
void map_wmo_free(struct map_wmo *handle);
void map_wmo_ref(struct map_wmo *handle);
void map_wmo_load(struct map_wmo *handle, const char *filename);

struct map_m2 *map_m2_new(uint32_t id);
void map_m2_free(struct map_m2 *handle);
void map_m2_ref(struct map_m2 *handle);
void map_m2_load(struct map_m2 *handle, const char *filename);

void map_chunk_get_interp_points(float px, float pz, uint8_t *points);
void map_chunk_get_interp_factors(float px, float pz, uint8_t *points, float *factors);
void map_chunk_get_y(struct map_chunk *chunk, float px, float pz, uint8_t *points, float *factors, float *y);
void map_chunk_get_normal(struct map_chunk *chunk, float px, float pz, uint8_t *points, float *factors, struct vec3f *n);

#endif
