#include "ppe/render_target.h"
#include "ppe/render_pass.h"

#include "gx/m2_particles.h"
#include "gx/m2_ribbons.h"
#include "gx/wmo_mliq.h"
#include "gx/skybox.h"
#include "gx/frame.h"
#include "gx/mcnk.h"
#include "gx/mclq.h"
#include "gx/text.h"
#include "gx/taxi.h"
#include "gx/wmo.h"
#include "gx/wdl.h"
#include "gx/blp.h"
#include "gx/m2.h"
#include "gx/gx.h"

#include "obj/player.h"

#include "map/tile.h"
#include "map/map.h"

#include "performance.h"
#include "shaders.h"
#include "camera.h"
#include "memory.h"
#include "loader.h"
#include "cache.h"
#include "const.h"
#include "log.h"
#include "wow.h"
#include "dbc.h"

#include <gfx/device.h>

#include <wow/wdt.h>
#include <wow/wdl.h>
#include <wow/mpq.h>
#include <wow/dbc.h>
#include <wow/blp.h>

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

MEMORY_DECL(GENERIC);

struct map *map_new(void)
{
	struct map *map = mem_zalloc(MEM_MAP, sizeof(*map));
	if (!map)
		return NULL;
	map->id = -1;
	map->fog_divisor = 1300;
	map->minimap.texture = GFX_TEXTURE_INIT();
	pthread_mutex_init(&map->minimap.mutex, NULL);
	return map;
}

void map_delete(struct map *map)
{
	if (!map)
		return;
	for (uint32_t i = 0; i < map->tiles_nb; ++i)
		map_tile_free(map->tile_array[map->tiles[i]]);
	gx_wmo_instance_free(map->wmo);
	mem_free(MEM_MAP, map->name);
	gx_skybox_delete(map->gx_skybox);
	gx_wdl_delete(map->gx_wdl);
	gfx_delete_texture(g_wow->device, &map->minimap.texture);
#ifdef WITH_DEBUG_RENDERING
	gx_taxi_delete(map->gx_taxi);
#endif
	pthread_mutex_destroy(&map->minimap.mutex);
	mem_free(MEM_MAP, map);
}

static bool load_tile(struct map *map, int32_t x, int32_t z)
{
	uint32_t adt_idx = z * 64 + x;
	if (!(map->adt_exists[z] & (1ull << x)))
		return true;
	if (map->tile_array[adt_idx])
		return true;
	char buf[256];
	snprintf(buf, sizeof(buf), "WORLD\\MAPS\\%s\\%s_%d_%d.adt", map->name, map->name, x, z);
	struct map_tile *tile = map_tile_new(buf, x, z);
	if (!tile)
		return false;
	map->tiles[map->tiles_nb++] = adt_idx;
	map->tile_array[adt_idx] = tile;
	map_tile_ask_load(tile);
	return true;
}

static void map_load_tick(struct map *map, struct gx_frame *frame)
{
	if (!map_flag_get(map, MAP_FLAG_HAS_ADT))
		return;
	struct vec3f pos = frame->cull_pos;
	struct vec3f delta;
	VEC3_SUB(delta, pos, map->last_pos);
	if (map->last_view_distance == frame->view_distance && sqrtf(delta.x * delta.x + delta.z * delta.z) < CHUNK_WIDTH)
		return;
	map->last_view_distance = frame->view_distance;
	map->last_pos = pos;
	int32_t distance = ceil(frame->view_distance / 16.0 / CHUNK_WIDTH);
	if (distance < 0)
		distance = 0;
	int32_t adt_x = floor(32 + pos.z / (16 * CHUNK_WIDTH));
	int32_t adt_z = floor(32 - pos.x / (16 * CHUNK_WIDTH));
	int32_t adt_start_x = floor(adt_x - distance);
	int32_t adt_start_z = floor(adt_z - distance);
	int32_t adt_end_x = ceil(adt_x + distance);
	int32_t adt_end_z = ceil(adt_z + distance);
	adt_start_x = (adt_start_x < 0 ? 0 : (adt_start_x > 63 ? 63 : adt_start_x));
	adt_start_z = (adt_start_z < 0 ? 0 : (adt_start_z > 63 ? 63 : adt_start_z));
	adt_end_x = (adt_end_x < 0 ? 0 : (adt_end_x > 63 ? 63 : adt_end_x));
	adt_end_z = (adt_end_z < 0 ? 0 : (adt_end_z > 63 ? 63 : adt_end_z));
	for (int32_t z = adt_start_z; z <= adt_end_z; ++z)
	{
		for (int32_t x = adt_start_x; x <= adt_end_x; ++x)
		{
			if (!load_tile(map, x, z))
				LOG_ERROR("failed load adt %" PRId32 "x%" PRId32, x, z);
		}
	}
	/* deleting them after because of instance caching (M2 & WMO) */
	for (uint32_t i = 0; i < map->tiles_nb; ++i)
	{
		uint16_t adt_id = map->tiles[i];
		struct map_tile *tile = map->tile_array[adt_id];
		if (tile->x < adt_start_x - 1
		 || tile->x > adt_end_x + 1
		 || tile->z < adt_start_z - 1
		 || tile->z > adt_end_z + 1)
		{
			map->tile_array[adt_id] = NULL;
			memmove(&map->tiles[i], &map->tiles[i + 1], sizeof(*map->tiles) * (map->tiles_nb - i - 1));
			map->tiles_nb--;
			map_tile_free(tile);
			i--;
		}
	}
}

static void minimap_texture_task(struct wow_mpq_compound *mpq_compound, void *userdata)
{
	struct map *map = userdata;
	if (!map->name)
		return;
	char mapname[256];
	snprintf(mapname, sizeof(mapname), "%s", map->name);
	wow_mpq_normalize_mpq_fn(mapname, sizeof(mapname));
	struct trs_dir *trs_dir = jks_hmap_get(g_wow->trs, JKS_HMAP_KEY_STR(mapname));
	if (!trs_dir)
	{
		LOG_WARN("no TRS found");
		return;
	}
	uint8_t *minimap_data = mem_malloc(MEM_GENERIC, 768 * 768 * 4);
	if (!minimap_data)
		return;
	memset(minimap_data, 0, 768 * 768 * 4);
	int last_x = map->minimap.last_x;
	int last_z = map->minimap.last_z;
	int orgx = 30 - last_x;
	int orgz = 31 + last_z;
	for (int y = 0; y < 3; ++y)
	{
		for (int x = 0; x < 3; ++x)
		{
			int mx = orgz + x;
			int mz = orgx + y;
			if (mx < 0 || mx >= 64 || mz < 0 || mx >= 64)
				continue;
			char name[512];
			snprintf(name, sizeof(name), "%s/map%02d_%02d.blp", mapname, mx, mz);
			wow_mpq_normalize_mpq_fn(name, sizeof(name));
			char **hashp = jks_hmap_get(trs_dir->entries, JKS_HMAP_KEY_STR(name));
			if (!hashp)
			{
				LOG_ERROR("trs entry not found: %s", name);
				continue;
			}
			char path[512];
			snprintf(path, sizeof(path), "TEXTURES\\MINIMAP\\%s", *hashp);
			struct wow_mpq_file *mpq = wow_mpq_get_file(mpq_compound, path);
			if (!mpq)
			{
				LOG_ERROR("failed to open %s", path);
				continue;
			}
			struct wow_blp_file *blp = wow_blp_file_new(mpq);
			wow_mpq_file_delete(mpq);
			if (!blp)
			{
				LOG_ERROR("failed to parse %s", path);
				continue;
			}
			uint8_t *blp_data;
			uint32_t blp_width;
			uint32_t blp_height;
			if (!wow_blp_decode_rgba(blp, 0, &blp_width, &blp_height, &blp_data))
			{
				LOG_ERROR("failed to decode %s", path);
				wow_blp_file_delete(blp);
				continue;
			}
			wow_blp_file_delete(blp);
			if (blp_width > 256 || blp_height > 256)
			{
				LOG_ERROR("image too big: %" PRIu32 "x%" PRIu32, blp_width, blp_height);
				mem_free(MEM_GENERIC, blp_data);
				continue;
			}
			for (uint32_t sy = 0; sy < blp_height; ++sy)
			{
				for (uint32_t sx = 0; sx < blp_width; ++sx)
				{
					uint8_t *s = &blp_data[(sy * blp_width + sx) * 4];
					uint8_t *d = &minimap_data[((sy + y * 256) * 768 + sx + x * 256) * 4];
					d[0] = s[0];
					d[1] = s[1];
					d[2] = s[2];
					d[3] = s[3];
				}
			}
			mem_free(MEM_LIBWOW, blp_data);
		}
	}
	pthread_mutex_lock(&map->minimap.mutex);
	mem_free(MEM_GENERIC, map->minimap.data);
	map->minimap.data = minimap_data;
	map->minimap.data_x = last_x;
	map->minimap.data_z = last_z;
	map->minimap.computing = false;
	pthread_mutex_unlock(&map->minimap.mutex);
}

static bool create_wdl(struct map *map)
{
	char buf[256];
	snprintf(buf, sizeof(buf), "WORLD\\MAPS\\%s\\%s.WDL", map->name, map->name);
	struct wow_mpq_file *file = wow_mpq_get_file(g_wow->mpq_compound, buf);
	if (!file)
	{
		LOG_ERROR("failed to get wdl file \"%s\"", buf);
		return false;
	}
	struct wow_wdl_file *wdl = wow_wdl_file_new(file);
	wow_mpq_file_delete(file);
	if (!wdl)
	{
		LOG_ERROR("failed to parse wdl file \"%s\"", buf);
		return false;
	}
	map->gx_wdl = gx_wdl_new(wdl);
	wow_wdl_file_delete(wdl);
	if (!map->gx_wdl)
	{
		LOG_ERROR("failed to create wdl renderer");
		return false;
	}
	return true;
}

static bool load_wdt(struct map *map, struct wow_wdt_file *file)
{
	for (uint32_t i = 0; i < map->tiles_nb; ++i)
	{
		map_tile_free(map->tile_array[map->tiles[i]]);
		map->tile_array[map->tiles[i]] = NULL;
	}
	map->tiles_nb = 0;
	map->wmo = NULL;
	map->last_pos = (struct vec3f){-999999, -999999, -999999};
	map->wow_flags = file->mphd.flags;
	map->last_check = 0;
	map->last_view_distance = 0;
	map->flags = 0;
	LOG_INFO("WDT flags: %x", map->wow_flags);
	for (size_t z = 0; z < 64; ++z)
	{
		map->adt_exists[z] = 0;
		for (size_t x = 0; x < 64; ++x)
		{
			if (file->main.data[z * 64 + x].flags & 1)
			{
				map->adt_exists[z] |= 1ull << x;
				map_flag_set(map, MAP_FLAG_HAS_ADT);
			}
		}
	}
	if (map_flag_get(map, MAP_FLAG_HAS_ADT))
		return true;
	char filename[512];
	snprintf(filename, sizeof(filename), "%s", &file->mwmo.data[0]);
	wow_mpq_normalize_mpq_fn(filename, sizeof(filename));
	map->wmo = gx_wmo_instance_new(filename);
	if (!map->wmo)
	{
		LOG_ERROR("failed to create wmo world instance");
		return true;
	}
	float offset = 0;/* (32 * 16 + 1) * CHUNK_WIDTH; */
	struct vec3f pos;
	VEC3_SET(pos, offset - file->modf.data[0].position.z, file->modf.data[0].position.y, -(offset - file->modf.data[0].position.x));
	struct mat4f mat1;
	struct mat4f mat2;
	MAT4_IDENTITY(mat1);
	MAT4_TRANSLATE(mat2, mat1, pos);
	MAT4_ROTATEY(float, mat1, mat2, (file->modf.data[0].rotation.y + 180) / 180.0 * M_PI);
	MAT4_ROTATEZ(float, mat2, mat1, -(file->modf.data[0].rotation.x) / 180.0 * M_PI);
	MAT4_ROTATEX(float, mat1, mat2, (file->modf.data[0].rotation.z) / 180.0 * M_PI);
	gx_wmo_instance_set_mat(map->wmo, &mat1);
	VEC3_CPY(map->wmo->pos, pos);
	map->wmo->doodad_set = file->modf.data[0].doodad_set;
	gx_wmo_ask_load(map->wmo->parent);
	return true;
}

static bool create_wdt(struct map *map)
{
	char buf[256];
	snprintf(buf, sizeof(buf), "WORLD\\MAPS\\%s\\%s.WDT", map->name, map->name);
	struct wow_mpq_file *file = wow_mpq_get_file(g_wow->mpq_compound, buf);
	if (!file)
	{
		LOG_ERROR("failed to get wdt file \"%s\"", buf);
		return false;
	}
	struct wow_wdt_file *wdt = wow_wdt_file_new(file);
	wow_mpq_file_delete(file);
	if (!wdt)
	{
		LOG_ERROR("failed to parse wdt file \"%s\"", buf);
		return false;
	}
	if (!load_wdt(map, wdt))
	{
		wow_wdt_file_delete(wdt);
		LOG_ERROR("failed to create wdt renderer");
		return false;
	}
	wow_wdt_file_delete(wdt);
	return true;
}

static void load_worldmap(struct map *map)
{
	for (size_t i = 0; i < g_wow->dbc.world_map_continent->file->header.record_count; ++i)
	{
		struct wow_dbc_row row = wow_dbc_get_row(g_wow->dbc.world_map_continent->file, i);
		uint32_t continent = wow_dbc_get_u32(&row, 4);
		if (continent != map->id)
			continue;
		map->worldmap.miny = wow_dbc_get_flt(&row, 36);
		map->worldmap.minx = wow_dbc_get_flt(&row, 40);
		map->worldmap.maxy = wow_dbc_get_flt(&row, 44);
		map->worldmap.maxx = wow_dbc_get_flt(&row, 48);
		map->worldmap.width = map->worldmap.maxx - map->worldmap.minx;
		map->worldmap.height = map->worldmap.maxy - map->worldmap.miny;
		return;
	}
	map->worldmap.minx = 0;
	map->worldmap.miny = 0;
	map->worldmap.maxx = 34133.4016;
	map->worldmap.maxy = 34133.4016;
	map->worldmap.width = map->worldmap.maxx - map->worldmap.minx;
	map->worldmap.height = map->worldmap.maxy - map->worldmap.miny;
	LOG_WARN("no worldmap found");
}

static struct taxi_node *find_taxi_node(struct map *map, uint32_t id)
{
	for (size_t i = 0; i < map->taxi.nodes_count; ++i)
	{
		if (wow_dbc_get_u32(&map->taxi.nodes[i].dbc, 0) == id)
			return &map->taxi.nodes[i];
	}
	return NULL;
}

static bool load_taxi(struct map *map)
{
	for (size_t i = 0; i < g_wow->dbc.taxi_nodes->file->header.record_count; ++i)
	{
		struct wow_dbc_row row = wow_dbc_get_row(g_wow->dbc.taxi_nodes->file, i);
		uint32_t continent = wow_dbc_get_u32(&row, 4);
		if (continent != map->id)
			continue;
		float x = wow_dbc_get_flt(&row, 12);
		float y = wow_dbc_get_flt(&row, 8);
		if (x < map->worldmap.minx || x > map->worldmap.maxx
		 || y < map->worldmap.miny || y > map->worldmap.maxy)
			continue;
		if (continent == 530)
		{
			uint32_t nodeid = wow_dbc_get_u32(&row, 0);
			if (nodeid != 99
			 && nodeid != 100
			 && nodeid != 102
			 && nodeid != 117
			 && nodeid != 118
			 && nodeid != 119
			 && nodeid != 120
			 && nodeid != 121
			 && nodeid != 122
			 && nodeid != 123
			 && nodeid != 124
			 && nodeid != 125
			 && nodeid != 126
			 && nodeid != 127
			 && nodeid != 128
			 && nodeid != 129
			 && nodeid != 130
			 && nodeid != 139
			 && nodeid != 140
			 && nodeid != 141
			 && nodeid != 149
			 && nodeid != 150
			 && nodeid != 156
			 && nodeid != 159
			 && nodeid != 160
			 && nodeid != 163
			 && nodeid != 164)
				continue;
		}
		struct taxi_node *tmp = mem_realloc(MEM_UI, map->taxi.nodes, sizeof(*tmp) * (map->taxi.nodes_count + 1));
		if (!tmp)
		{
			LOG_ERROR("nodes allocation failed");
			return false;
		}
		map->taxi.nodes = tmp;
		map->taxi.nodes[map->taxi.nodes_count].dbc = row;
		map->taxi.nodes[map->taxi.nodes_count].links = NULL;
		map->taxi.nodes[map->taxi.nodes_count].links_count = 0;
		map->taxi.nodes_count++;
	}
	for (size_t i = 0; i < g_wow->dbc.taxi_path->file->header.record_count; ++i)
	{
		struct wow_dbc_row row = wow_dbc_get_row(g_wow->dbc.taxi_path->file, i);
		struct taxi_node *src = find_taxi_node(map, wow_dbc_get_u32(&row, 4));
		if (!src)
			continue;
		struct taxi_node *dst = find_taxi_node(map, wow_dbc_get_u32(&row, 8));
		if (!dst)
			continue;
		struct taxi_link *tmp = mem_realloc(MEM_UI, src->links, sizeof(*tmp) * (src->links_count + 1));
		if (!tmp)
		{
			LOG_ERROR("links allocation failed");
			return false;
		}
		src->links = tmp;
		src->links[src->links_count].dst = dst;
		src->links[src->links_count].price = wow_dbc_get_u32(&row, 12);
		src->links_count++;
	}
	return true;
}

static void setup_pf_dist(void)
{
	while (1)
	{
		struct taxi_node *cur = NULL;
		for (size_t i = 0; i < g_wow->map->taxi.nodes_count; ++i)
		{
			if (g_wow->map->taxi.nodes[i].pf_visited
			 || g_wow->map->taxi.nodes[i].pf_dist == (size_t)-1
			 || (cur && g_wow->map->taxi.nodes[i].pf_dist >= cur->pf_dist))
				continue;
			cur = &g_wow->map->taxi.nodes[i];
		}
		if (!cur)
			break;
		cur->pf_visited = true;
		for (size_t i = 0; i < cur->links_count; ++i)
		{
			struct taxi_node *dst = cur->links[i].dst;
			size_t nxt_dist = cur->pf_dist + 1;
			size_t nxt_price = cur->pf_price + cur->links[i].price;
			if (dst->pf_visited
			 || (dst->pf_dist != (size_t)-1 && (dst->pf_dist < nxt_dist || dst->pf_price < nxt_price)))
				continue;
			dst->pf_price = nxt_price;
			dst->pf_prev = cur;
			dst->pf_dist = nxt_dist;
		}
	}
}

void map_gen_taxi_path(struct map *map, uint32_t srcid, uint32_t dstid)
{
	if (srcid == map->taxi.path_src
	 && dstid == map->taxi.path_dst)
		return;
	map->taxi.path_src = srcid;
	map->taxi.path_dst = dstid;
	mem_free(MEM_UI, map->taxi.path_nodes);
	map->taxi.path_nodes = NULL;
	map->taxi.path_nodes_count = 0;
	if (srcid >= map->taxi.nodes_count)
	{
		LOG_ERROR("invalid src node");
		return;
	}
	if (dstid >= map->taxi.nodes_count)
	{
		LOG_ERROR("invalid dst node");
		return;
	}
	struct taxi_node *src = &map->taxi.nodes[srcid];
	struct taxi_node *dst = &map->taxi.nodes[dstid];
	for (size_t i = 0; i < g_wow->map->taxi.nodes_count; ++i)
	{
		map->taxi.nodes[i].pf_visited = false;
		map->taxi.nodes[i].pf_dist = (size_t)-1;
		map->taxi.nodes[i].pf_prev = NULL;
	}
	src->pf_dist = 0;
	setup_pf_dist();
	if (!dst->pf_prev)
	{
		LOG_ERROR("not path found for %u -> %u", srcid, dstid);
		return;
	}
	struct taxi_node *node = dst;
	while (node)
	{
		struct taxi_node **tmp = mem_realloc(MEM_UI, map->taxi.path_nodes, sizeof(*tmp) * (map->taxi.path_nodes_count + 1));
		if (!tmp)
		{
			LOG_ERROR("path nodes allocation failed");
			return;
		}
		map->taxi.path_nodes = tmp;
		map->taxi.path_nodes[g_wow->map->taxi.path_nodes_count] = node;
		map->taxi.path_nodes_count++;
		node = node->pf_prev;
	}
}

bool map_setid(struct map *map, uint32_t id)
{
	if (map->id == id)
		return true;
	const char *map_name = NULL;
	for (uint32_t i = 0; i < g_wow->dbc.map->file->header.record_count; ++i)
	{
		struct wow_dbc_row row = dbc_get_row(g_wow->dbc.map, i);
		if (wow_dbc_get_u32(&row, 0) != id)
			continue;
		map_name = wow_dbc_get_str(&row, 4);
		break;
	}
	if (!map_name)
	{
		LOG_ERROR("invalid mapid: %u", id);
		return true;
	}
	char *new_name = mem_strdup(MEM_MAP, map_name);
	if (!new_name)
	{
		LOG_ERROR("failed to allocate map name");
		return false;
	}
	mem_free(MEM_GX, map->name);
	map->name = new_name;
	LOG_INFO("mapid: %u, mapname: %s", id, map->name);
	gx_skybox_delete(map->gx_skybox);
	map->gx_skybox = NULL;
	gx_wdl_delete(map->gx_wdl);
	map->gx_wdl = NULL;
	for (size_t i = 0; i < map->taxi.nodes_count; ++i)
		mem_free(MEM_UI, map->taxi.nodes[i].links);
	mem_free(MEM_UI, map->taxi.nodes);
	map->taxi.nodes = NULL;
	map->taxi.nodes_count = 0;
	mem_free(MEM_UI, map->taxi.path_nodes);
	map->taxi.path_nodes = NULL;
	map->taxi.path_nodes_count = 0;
	map->id = id;
	load_worldmap(map);
	if (!load_taxi(map))
	{
		LOG_ERROR("failed to load taxi nodes");
		return false;
	}
	map->gx_skybox = gx_skybox_new(id);
	if (!map->gx_skybox)
	{
		LOG_ERROR("failed to create skybox renderer");
		return false;
	}
	if (!create_wdt(map))
	{
		LOG_ERROR("failed to create wdt renderer");
		return false;
	}
	if (!create_wdl(map))
		LOG_ERROR("failed to create wdl renderer");
#ifdef WITH_DEBUG_RENDERING
	map->gx_taxi = gx_taxi_new(id);
	if (!map->gx_taxi)
	{
		LOG_ERROR("failed to create taxi renderer");
		return false;
	}
#endif
	return true;
}

static void render_skybox(struct map *map, struct gx_frame *frame)
{
	if (!(g_wow->gx->opt & GX_OPT_SKYBOX))
		return;
	gfx_bind_pipeline_state(g_wow->device, &g_wow->gx->skybox_pipeline_state);
	gx_skybox_render(map->gx_skybox, frame);
}

static void render_wdl(struct map *map, struct gx_frame *frame)
{
	if (!map->gx_wdl)
		return;
	if (!(g_wow->gx->opt & GX_OPT_WDL))
		return;
	gfx_bind_pipeline_state(g_wow->device, &g_wow->gx->wdl_pipeline_state);
	gx_wdl_render(map->gx_wdl, frame);
}

#ifdef WITH_DEBUG_RENDERING
static void render_taxi(struct map *map, struct gx_frame *frame)
{
	if (!(g_wow->gx->opt & GX_OPT_TAXI))
		return;
	gfx_bind_pipeline_state(g_wow->device, &g_wow->gx->taxi_pipeline_state);
	gx_taxi_render(map->gx_taxi, frame);
}
#endif

static void render_wmo_mliq(struct map *map, struct gx_frame *frame)
{
	if (!(g_wow->gx->opt & GX_OPT_WMO_LIQUIDS) || !frame->render_lists.wmo.entries.size)
		return;
	PERFORMANCE_BEGIN(WMO_LIQUIDS_RENDER);
	gfx_bind_pipeline_state(g_wow->device, &g_wow->gx->wmo_mliq_pipeline_state);
	gfx_bind_constant(g_wow->device, 2, &frame->mliq_uniform_buffer, sizeof(struct shader_mliq_scene_block), 0);
	uint8_t idx = (g_wow->frametime / 30000000) % 30;
	for (size_t type = 0; type < 9; ++type)
	{
		if (!frame->render_lists.wmo_mliq[type].entries.size)
			continue;
		switch (type)
		{
			case 0:
				gx_blp_bind(g_wow->gx->river_textures[idx], 0);
				break;
			case 1:
				gx_blp_bind(g_wow->gx->ocean_textures[idx], 0);
				break;
			case 2:
				if (map->id == 530)
					gx_blp_bind(g_wow->gx->lavag_textures[idx], 0);
				else
					gx_blp_bind(g_wow->gx->magma_textures[idx], 0);
				break;
			case 3:
				gx_blp_bind(g_wow->gx->slime_textures[idx], 0);
				break;
			case 4:
				gx_blp_bind(g_wow->gx->river_textures[idx], 0);
				break;
			case 5:
				continue;
			case 6:
				if (map->id == 530)
					gx_blp_bind(g_wow->gx->lavag_textures[idx], 0);
				else
					gx_blp_bind(g_wow->gx->magma_textures[idx], 0);
				break;
			case 7:
				gx_blp_bind(g_wow->gx->slime_textures[idx], 0);
				break;
			case 8:
				continue;
			default:
				continue;
		}
		for (size_t i = 0; i < frame->render_lists.wmo_mliq[type].entries.size; ++i)
			gx_wmo_mliq_render(*JKS_ARRAY_GET(&frame->render_lists.wmo_mliq[type].entries, i, struct gx_wmo_mliq*), frame, type);
	}
	PERFORMANCE_END(WMO_LIQUIDS_RENDER);
}

static void cull_wdl(struct map *map, struct gx_frame *frame)
{
	if (!map->gx_wdl)
		return;
	PERFORMANCE_BEGIN(WDL_CULL);
	gx_wdl_cull(map->gx_wdl, frame);
	PERFORMANCE_END(WDL_CULL);
}

static void cull_tiles(struct map *map, struct gx_frame *frame)
{
	PERFORMANCE_BEGIN(ADT_CULL);
	for (uint32_t i = 0; i < map->tiles_nb; ++i)
		map_tile_cull(map->tile_array[map->tiles[i]], frame);
	PERFORMANCE_END(ADT_CULL);
}

void map_cull(struct map *map, struct gx_frame *frame)
{
	if (!map_flag_set(map, MAP_FLAG_TILES_LOADED))
		map_load_tick(map, frame);
	if (!map_flag_set(map, MAP_FLAG_WDL_CULLED))
		cull_wdl(map, frame);
	if (map->wmo && !map_flag_set(map, MAP_FLAG_WMO_CULLED))
		gx_wmo_instance_add_to_render(map->wmo, frame, true);
	cull_tiles(map, frame);
}

static void update_minimap_texture(struct map *map)
{
	int player_x = floor(((struct worldobj*)g_wow->player)->position.x / 533.3333334f);
	int player_z = floor(((struct worldobj*)g_wow->player)->position.z / 533.3333334f);
	pthread_mutex_lock(&map->minimap.mutex);
	if (!map->minimap.computing && (player_x != map->minimap.last_x || player_z != map->minimap.last_z))
	{
		map->minimap.computing = true;
		map->minimap.last_x = player_x;
		map->minimap.last_z = player_z;
		loader_push(g_wow->loader, ASYNC_TASK_MINIMAP_TEXTURE, minimap_texture_task, map);
	}
	if (!map->minimap.data)
	{
		pthread_mutex_unlock(&map->minimap.mutex);
		return;
	}
	gfx_delete_texture(g_wow->device, &map->minimap.texture);
	gfx_create_texture(g_wow->device, &map->minimap.texture, GFX_TEXTURE_2D, GFX_R8G8B8A8, 1, 768, 768, 0);
	gfx_set_texture_levels(&map->minimap.texture, 0, 0);
	gfx_set_texture_anisotropy(&map->minimap.texture, g_wow->anisotropy);
	gfx_set_texture_filtering(&map->minimap.texture, GFX_FILTERING_LINEAR, GFX_FILTERING_LINEAR, GFX_FILTERING_LINEAR);
	gfx_set_texture_addressing(&map->minimap.texture, GFX_TEXTURE_ADDRESSING_CLAMP, GFX_TEXTURE_ADDRESSING_CLAMP, GFX_TEXTURE_ADDRESSING_CLAMP);
	gfx_finalize_texture(&map->minimap.texture);
	gfx_set_texture_data(&map->minimap.texture, 0, 0, 768, 768, 0, 768 * 768 * 4, map->minimap.data);
	map->minimap.texture_x = map->minimap.data_x;
	map->minimap.texture_z = map->minimap.data_z;
	mem_free(MEM_GENERIC, map->minimap.data);
	map->minimap.data = NULL;
	pthread_mutex_unlock(&map->minimap.mutex);
}

static void
render_shadow_pass(struct map *map, struct gx_frame *frame)
{
	float shadow_width = 4000;
	render_target_resize(g_wow->post_process.shadow, shadow_width, shadow_width);
	render_target_bind(g_wow->post_process.shadow, 0);
	gfx_set_viewport(g_wow->device, 0, 0, shadow_width, shadow_width);
	gfx_set_scissor(g_wow->device, 0, 0, shadow_width, shadow_width);
	gfx_clear_depth_stencil(g_wow->device, &g_wow->post_process.shadow->render_target, 1, 0);
	gfx_set_scissor(g_wow->device, 1, 1, shadow_width - 2, shadow_width - 2);
	gx_frame_render_shadow_m2(frame);
}

void
map_render(struct map *map, struct gx_frame *frame)
{
	if (map->id == (uint32_t)-1)
		return;
	update_minimap_texture(map);
	render_shadow_pass(map, frame);
	struct render_pass_def
	{
		struct render_pass *render_pass;
		uint32_t targets;
	} render_passes[20];
	struct render_target *render_target;
	size_t render_passes_nb = 0;
	bool post_process;
	uint32_t render_buffer_bits = RENDER_TARGET_COLOR_BUFFER_BIT;
	if (g_wow->post_process.ssao->enabled)
		render_passes[render_passes_nb++] = (struct render_pass_def){g_wow->post_process.ssao, RENDER_TARGET_NORMAL_BUFFER_BIT | RENDER_TARGET_POSITION_BUFFER_BIT};
	if (g_wow->post_process.sobel->enabled)
		render_passes[render_passes_nb++] = (struct render_pass_def){g_wow->post_process.sobel, RENDER_TARGET_NORMAL_BUFFER_BIT | RENDER_TARGET_POSITION_BUFFER_BIT};
	if (g_wow->post_process.glow->enabled)
		render_passes[render_passes_nb++] = (struct render_pass_def){g_wow->post_process.glow, 0};
	if (g_wow->post_process.bloom->enabled)
		render_passes[render_passes_nb++] = (struct render_pass_def){g_wow->post_process.bloom, 0};
	if (g_wow->post_process.cel->enabled)
		render_passes[render_passes_nb++] = (struct render_pass_def){g_wow->post_process.cel, 0};
	if (g_wow->post_process.fxaa->enabled)
		render_passes[render_passes_nb++] = (struct render_pass_def){g_wow->post_process.fxaa, 0};
	if (g_wow->post_process.sharpen->enabled)
		render_passes[render_passes_nb++] = (struct render_pass_def){g_wow->post_process.sharpen, 0};
	if (g_wow->post_process.chromaber->enabled)
		render_passes[render_passes_nb++] = (struct render_pass_def){g_wow->post_process.chromaber, 0};
	if (g_wow->post_process.death->enabled)
		render_passes[render_passes_nb++] = (struct render_pass_def){g_wow->post_process.death, 0};
	if (g_wow->gx->opt & GX_OPT_DYN_WATER)
		render_buffer_bits |= RENDER_TARGET_NORMAL_BUFFER_BIT | RENDER_TARGET_POSITION_BUFFER_BIT;
	for (size_t i = render_passes_nb; i > 0; --i)
	{
		render_buffer_bits |= render_passes[i - 1].targets;
		render_passes[i - 1].targets = render_buffer_bits;
	}
	uint32_t render_width = g_wow->render_width * g_wow->fsaa;
	uint32_t render_height = g_wow->render_height * g_wow->fsaa;
	for (size_t i = 0; i < render_passes_nb; ++i)
		render_pass_resize(render_passes[i].render_pass,
		                   render_width,
		                   render_height);
	if (g_wow->post_process.msaa->enabled)
		render_target_resize(g_wow->post_process.msaa,
		                     render_width,
		                     render_height);
	if (render_passes_nb || g_wow->fsaa != 1 || (g_wow->gx->opt & GX_OPT_DYN_WATER))
	{
		render_target_resize(g_wow->post_process.dummy1,
		                     render_width,
		                     render_height);
		render_target_resize(g_wow->post_process.dummy2,
		                     render_width,
		                     render_height);
	}
	if (g_wow->post_process.msaa->enabled)
	{
		render_target = g_wow->post_process.msaa;
		render_target_bind(g_wow->post_process.msaa, render_buffer_bits);
		post_process = true;
	}
	else if (render_passes_nb || g_wow->fsaa != 1 || (g_wow->gx->opt & GX_OPT_DYN_WATER))
	{
		render_target = g_wow->post_process.dummy1;
		render_target_bind(g_wow->post_process.dummy1, render_buffer_bits);
		post_process = true;
	}
	else
	{
		render_target = NULL;
		gfx_bind_render_target(g_wow->device, NULL);
		post_process = false;
	}

	gfx_set_viewport(g_wow->device, 0, 0, render_width, render_height);
	gfx_set_scissor(g_wow->device, 0, 0, render_width, render_height);
	/* XXX: gfx_bind_depth_stencil_state(g_wow->device, &world->depth_stencil_states[WORLD_DEPTH_STENCIL_RW_RW]); */
	if (render_target)
	{
		render_target_clear(render_target, render_buffer_bits);
	}
	else
	{
		gfx_clear_color(g_wow->device, NULL, GFX_RENDERTARGET_ATTACHMENT_COLOR0, (struct vec4f){0, 1, 0, 1});
		gfx_clear_depth_stencil(g_wow->device, NULL, 1, 0);
	}

	gx_skybox_update(map->gx_skybox, frame);
	gx_frame_build_uniform_buffers(frame);

	PERFORMANCE_BEGIN(WMO_RENDER);
	gx_frame_render_wmo(frame);
	PERFORMANCE_END(WMO_RENDER);

	PERFORMANCE_BEGIN(M2_RENDER);
	gx_frame_render_opaque_m2(frame);
	PERFORMANCE_END(M2_RENDER);

	PERFORMANCE_BEGIN(M2_RENDER);
	gx_frame_render_ground_m2(frame);
	PERFORMANCE_END(M2_RENDER);

	PERFORMANCE_BEGIN(MCNK_RENDER);
	gx_frame_render_mcnk(frame);
	PERFORMANCE_END(MCNK_RENDER);

	/* XXX: SSAO & sobel here (how to handle MSAA / CSAA ?) */
	if (post_process)
	{
		uint32_t draw_buffers[3] = {GFX_RENDERTARGET_ATTACHMENT_COLOR0, GFX_RENDERTARGET_ATTACHMENT_NONE, GFX_RENDERTARGET_ATTACHMENT_NONE};
		gfx_set_render_target_draw_buffers(&render_target->render_target, draw_buffers, 3);
	}

	PERFORMANCE_BEGIN(WDL_RENDER);
	render_wdl(map, frame);
	PERFORMANCE_END(WDL_RENDER);

	PERFORMANCE_BEGIN(SKYBOX_RENDER);
	render_skybox(map, frame);
	PERFORMANCE_END(SKYBOX_RENDER);

#ifdef WITH_DEBUG_RENDERING
	PERFORMANCE_BEGIN(AABB_RENDER);
	gx_frame_render_aabb(frame);
	PERFORMANCE_END(AABB_RENDER);

	PERFORMANCE_BEGIN(COLLISIONS_RENDER);
	gx_frame_render_collisions(frame);
	PERFORMANCE_END(COLLISIONS_RENDER);

	PERFORMANCE_BEGIN(WMO_PORTALS_RENDER);
	gx_frame_render_wmo_portals(frame);
	PERFORMANCE_END(WMO_PORTALS_RENDER);

	PERFORMANCE_BEGIN(WMO_LIGHTS_RENDER);
	gx_frame_render_wmo_lights(frame);
	PERFORMANCE_END(WMO_LIGHTS_RENDER);

	PERFORMANCE_BEGIN(M2_LIGHTS_RENDER);
	gx_frame_render_m2_lights(frame);
	PERFORMANCE_END(M2_LIGHTS_RENDER);

#endif
	if (g_wow->gx->opt & GX_OPT_DYN_WATER)
	{
		gfx_bind_render_target(g_wow->device, NULL);
		gfx_set_viewport(g_wow->device, 0, 0, g_wow->render_width, g_wow->render_height);
		gfx_clear_color(g_wow->device, NULL, GFX_RENDERTARGET_ATTACHMENT_COLOR0, (struct vec4f){0, 0, 0, 1});
		gfx_clear_depth_stencil(g_wow->device, NULL, 1, 0);
		render_target_resolve(g_wow->post_process.dummy1, NULL, RENDER_TARGET_COLOR_BUFFER_BIT | RENDER_TARGET_DEPTH_BUFFER_BIT);
		post_process = false;
	}
	render_wmo_mliq(map, frame);

	PERFORMANCE_BEGIN(MCLQ_RENDER);
	gx_frame_render_mclq(frame);
	PERFORMANCE_END(MCLQ_RENDER);

	PERFORMANCE_BEGIN(M2_PARTICLES_RENDER);
	gx_frame_render_m2_particles(frame);
	PERFORMANCE_END(M2_PARTICLES_RENDER);

	PERFORMANCE_BEGIN(M2_RIBBONS_RENDER);
	gx_frame_render_m2_ribbons(frame);
	PERFORMANCE_END(M2_RIBBONS_RENDER);

	PERFORMANCE_BEGIN(M2_RENDER);
	gx_frame_render_transparent_m2(frame);
	PERFORMANCE_END(M2_RENDER);

	PERFORMANCE_BEGIN(TEXT_RENDER);
	gx_frame_render_texts(frame);
	PERFORMANCE_END(TEXT_RENDER);

#ifdef WITH_DEBUG_RENDERING
	PERFORMANCE_BEGIN(WMO_COLLISIONS_RENDER);
	gx_frame_render_wmo_collisions(frame);
	PERFORMANCE_END(WMO_COLLISIONS_RENDER);

	PERFORMANCE_BEGIN(M2_COLLISIONS_RENDER);
	gx_frame_render_m2_collisions(frame);
	PERFORMANCE_END(M2_COLLISIONS_RENDER);

	PERFORMANCE_BEGIN(M2_BONES_RENDER);
	gx_frame_render_m2_bones(frame);
	PERFORMANCE_END(M2_BONES_RENDER);

	PERFORMANCE_BEGIN(TAXI_RENDER);
	render_taxi(map, frame);
	PERFORMANCE_END(TAXI_RENDER);
#endif

	gfx_set_scissor(g_wow->device, 0, 0, render_width, render_height);
	if (!post_process)
		return;
	struct render_target *output = (render_passes_nb || g_wow->fsaa != 1) ? g_wow->post_process.dummy1 : NULL;
	if (g_wow->post_process.msaa->enabled)
		render_target_resolve(g_wow->post_process.msaa, output, render_buffer_bits);
	if (!render_passes_nb)
	{
		if (g_wow->fsaa != 1)
			render_pass_process(g_wow->post_process.fsaa, g_wow->post_process.dummy1, NULL, RENDER_TARGET_COLOR_BUFFER_BIT);
		return;
	}
	struct render_target *target_in = g_wow->post_process.dummy1;
	struct render_target *target_out = g_wow->post_process.dummy2;
	for (size_t i = 0; i < render_passes_nb; ++i)
	{
		struct render_target *out = (i == render_passes_nb - 1) ? NULL : target_out;
		render_pass_process(render_passes[i].render_pass, target_in, out, render_passes[i].targets);
		struct render_target *tmp;
		tmp = target_in;
		target_in = target_out;
		target_out = tmp;
	}
}

void map_collect_collision_triangles(struct map *map, const struct collision_params *params, struct jks_array *triangles)
{
	struct collision_state state;
	jks_array_init(&state.wmo, sizeof(struct gx_wmo_instance*), NULL, &jks_array_memory_fn_GENERIC);
	jks_array_init(&state.m2, sizeof(struct gx_m2_instance*), NULL, &jks_array_memory_fn_GENERIC);
	for (size_t i = 0; i < map->tiles_nb; ++i)
		map_tile_collect_collision_triangles(map->tile_array[map->tiles[i]], params, &state, triangles);
	jks_array_destroy(&state.wmo);
	jks_array_destroy(&state.m2);
}
