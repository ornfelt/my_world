#ifndef MAP_H
#define MAP_H

#include <gfx/objects.h>

#include <jks/array.h>
#include <jks/aabb.h>
#include <jks/vec3.h>

#include <wow/dbc.h>

#include <stdbool.h>
#include <pthread.h>

struct taxi_node;
struct gx_skybox;
struct gx_frame;
struct gx_taxi;
struct gx_wdl;

enum map_flag
{
	MAP_FLAG_HAS_ADT      = (1 << 0),
	MAP_FLAG_TILES_LOADED = (1 << 1),
	MAP_FLAG_WDL_CULLED   = (1 << 2),
	MAP_FLAG_WMO_CULLED   = (1 << 3),
};

struct taxi_link
{
	struct taxi_node *dst;
	uint32_t price;
};

struct taxi_node
{
	struct wow_dbc_row dbc;
	struct taxi_link *links;
	size_t links_count;
	size_t pf_dist;
	size_t pf_price;
	struct taxi_node *pf_prev;
	bool pf_visited;
};

struct minimap
{
	gfx_texture_t texture;
	pthread_mutex_t mutex;
	uint8_t *data;
	int texture_x;
	int texture_z;
	int data_x;
	int data_z;
	int last_x;
	int last_z;
	bool computing;
};

struct worldmap
{
	float minx;
	float miny;
	float maxx;
	float maxy;
	float width;
	float height;
};

struct taxi_info
{
	struct taxi_node *nodes;
	size_t nodes_count;
	struct taxi_node **path_nodes;
	size_t path_nodes_count;
	uint32_t path_src;
	uint32_t path_dst;
};

struct map
{
	struct map_tile *tile_array[64 * 64];
	uint16_t tiles[64 * 64]; /* List of loaded adts */
	uint32_t tiles_nb;
	struct minimap minimap;
#ifdef WITH_DEBUG_RENDERING
	struct gx_taxi *gx_taxi;
#endif
	struct gx_skybox *gx_skybox;
	struct gx_wdl *gx_wdl;
	char *name;
	uint32_t id;
	float fog_divisor;
	uint64_t adt_exists[64];
	struct gx_wmo_instance *wmo;
	struct vec3f last_pos;
	uint32_t wow_flags;
	int64_t last_check;
	float last_view_distance;
	struct worldmap worldmap;
	struct taxi_info taxi;
	enum map_flag flags;
};

struct collision_params
{
	struct aabb aabb;
	struct vec3f center;
	float radius;
	bool wmo_cam;
};

struct collision_triangle
{
	struct vec3f points[3];
	bool touched;
};

struct collision_state
{
	struct jks_array wmo;
	struct jks_array m2;
};

static inline bool map_flag_get(struct map *map, enum map_flag flag)
{
	return (__atomic_load_n(&map->flags, __ATOMIC_RELAXED) & flag) != 0;
}

static inline bool map_flag_set(struct map *map, enum map_flag flag)
{
	return (__atomic_fetch_or(&map->flags, flag, __ATOMIC_RELAXED) & flag) != 0;
}

static inline bool map_flag_clear(struct map *map, enum map_flag flag)
{
	return (__atomic_fetch_and(&map->flags, ~flag, __ATOMIC_RELAXED) & flag) != 0;
}

struct map *map_new(void);
void map_delete(struct map *map);
bool map_setid(struct map *map, uint32_t mapid);
void map_cull(struct map *map, struct gx_frame *frame);
void map_render(struct map *map, struct gx_frame *frame);
void map_gen_taxi_path(struct map *map, uint32_t src, uint32_t dst);
void map_collect_collision_triangles(struct map *map, const struct collision_params *param, struct jks_array *triangles);

#endif
