#include "gx/wmo_group.h"
#include "gx/frame.h"
#include "gx/mcnk.h"
#include "gx/mclq.h"
#include "gx/wmo.h"
#include "gx/m2.h"

#include "map/tile.h"
#include "map/map.h"

#include "performance.h"
#include "loader.h"
#include "memory.h"
#include "const.h"
#include "cache.h"
#include "log.h"
#include "wow.h"
#include "dbc.h"

#include <wow/wmo_group.h>
#include <wow/adt.h>
#include <wow/mpq.h>

#include <limits.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define GROUND_DOODADS_RANGE_MIN (CHUNK_WIDTH * 5)
#define GROUND_DOODADS_RANGE_MAX (CHUNK_WIDTH * 6)

MEMORY_DECL(MAP);

struct map_tile_ground_effect
{
	struct gx_m2_ground_batch *light_batch;
	struct gx_m2_ground_batch *shadow_batch;
};

struct map_chunk_ground_effect
{
	size_t tile_index;
	struct jks_array light_doodads; /* struct gx_m2_ground */
	struct jks_array shadow_doodads; /* struct gx_m2_ground */
};

static void add_objects_to_render(struct map_tile *tile, struct gx_frame *frame);
static void init_ground_effects(struct map_chunk *chunk, struct wow_mcnk *wow_mcnk);

static void tile_ground_effect_destroy(void *data)
{
	struct map_tile_ground_effect *ground_effect = data;
	gx_m2_ground_batch_free(ground_effect->light_batch);
	gx_m2_ground_batch_free(ground_effect->shadow_batch);
}

static void chunk_ground_effect_destroy(void *data)
{
	struct map_chunk_ground_effect *ground_effect = data;
	jks_array_destroy(&ground_effect->light_doodads);
	jks_array_destroy(&ground_effect->shadow_doodads);
}

struct map_tile *map_tile_new(const char *filename, int32_t x, int32_t z)
{
	struct map_tile *tile = mem_zalloc(MEM_MAP, sizeof(*tile));
	if (!tile)
		return NULL;
	refcount_init(&tile->refcount, 1);
	tile->x = x;
	tile->z = z;
	tile->filename = mem_strdup(MEM_MAP, filename);
	VEC3_SET(tile->pos, -(z - 32) * CHUNK_WIDTH * 16 - 500, 0, (x - 32) * CHUNK_WIDTH * 16 + 500);
	for (size_t i = 0; i < CHUNKS_PER_TILE; ++i)
	{
		struct map_chunk *chunk = &tile->chunks[i];
		jks_array_init(&chunk->doodads_to_aabb, sizeof(uint32_t), NULL, &jks_array_memory_fn_MAP);
		jks_array_init(&chunk->wmos_to_aabb, sizeof(uint32_t), NULL, &jks_array_memory_fn_MAP);
		jks_array_init(&chunk->ground_effects, sizeof(struct map_chunk_ground_effect), chunk_ground_effect_destroy, &jks_array_memory_fn_MAP);
#ifdef WITH_DEBUG_RENDERING
		gx_aabb_init(&chunk->doodads_gx_aabb, (struct vec4f){1, 1, 1, 1}, 1);
		gx_aabb_init(&chunk->wmos_gx_aabb, (struct vec4f){0.5, 0.5, 1, 1}, 1);
		gx_aabb_init(&chunk->gx_aabb, (struct vec4f){0, 1, 1, 1}, 1);
#endif
	}
#ifdef WITH_DEBUG_RENDERING
	gx_aabb_init(&tile->doodads_gx_aabb, (struct vec4f){0.25, 0.25, 1, 1}, 3);
	gx_aabb_init(&tile->wmos_gx_aabb, (struct vec4f){1, 0.25, 0.25, 1}, 3);
	gx_aabb_init(&tile->gx_aabb, (struct vec4f){1, 0.4, 0, 1}, 3);
#endif
	jks_array_init(&tile->ground_effects, sizeof(struct map_tile_ground_effect), tile_ground_effect_destroy, &jks_array_memory_fn_MAP);
	if (!tile->filename)
		goto err;
	return tile;

err:
	mem_free(MEM_GX, tile);
	return NULL;
}

static void map_tile_unload_task(struct wow_mpq_compound *mpq_compound, void *userdata)
{
	(void)mpq_compound;
	struct map_tile *tile = userdata;
	for (size_t i = 0; i < CHUNKS_PER_TILE; ++i)
	{
		struct map_chunk *chunk = &tile->chunks[i];
		jks_array_destroy(&chunk->doodads_to_aabb);
		jks_array_destroy(&chunk->wmos_to_aabb);
		mem_free(MEM_GX, chunk->doodads);
		mem_free(MEM_GX, chunk->wmos);
		jks_array_destroy(&chunk->ground_effects);
#ifdef WITH_DEBUG_RENDERING
		gx_aabb_destroy(&chunk->doodads_gx_aabb);
		gx_aabb_destroy(&chunk->wmos_gx_aabb);
		gx_aabb_destroy(&chunk->gx_aabb);
#endif
	}
	gx_mclq_delete(tile->gx_mclq);
	gx_mcnk_delete(tile->gx_mcnk);
#ifdef WITH_DEBUG_RENDERING
	gx_aabb_destroy(&tile->doodads_gx_aabb);
	gx_aabb_destroy(&tile->wmos_gx_aabb);
	gx_aabb_destroy(&tile->gx_aabb);
#endif
	jks_array_destroy(&tile->ground_effects);
	mem_free(MEM_MAP, tile->filename);
	mem_free(MEM_MAP, tile);
}

void map_tile_free(struct map_tile *tile)
{
	if (!tile)
		return;
	if (refcount_dec(&tile->refcount))
		return;
	for (uint32_t i = 0; i < tile->wmo_nb; ++i)
		map_wmo_free(tile->wmo[i]);
	for (uint32_t i = 0; i < tile->m2_nb; ++i)
		map_m2_free(tile->m2[i]);
	mem_free(MEM_GX, tile->wmo);
	tile->wmo = NULL;
	tile->wmo_nb = 0;
	mem_free(MEM_GX, tile->m2);
	tile->m2 = NULL;
	tile->m2_nb = 0;
	loader_push(g_wow->loader, ASYNC_TASK_MAP_TILE_UNLOAD, map_tile_unload_task, tile);
}

void map_tile_ref(struct map_tile *tile)
{
	refcount_inc(&tile->refcount);
}

static bool initialize(void *userdata)
{
	struct map_tile *tile = userdata;
	assert(tile->flags & MAP_TILE_FLAG_LOADED);
	switch (gx_mcnk_initialize(tile->gx_mcnk))
	{
		case -1:
			return true;
		case 0:
			return false;
		case 1:
			break;
	}
	if (tile->gx_mclq)
	{
		switch (gx_mclq_initialize(tile->gx_mclq))
		{
			case -1:
				return true;
			case 0:
				return false;
			case 1:
				break;
		}
	}
	tile->flags |= MAP_TILE_FLAG_INITIALIZED;
	return true;
}

static bool load_childs(struct map_tile *tile, struct wow_adt_file *file)
{
	uint32_t wmo_nb = file->modf.size;
	struct map_wmo **wmo = NULL;
	uint32_t m2_nb = file->mddf.size;
	struct map_m2 **m2 = NULL;
	wmo = mem_malloc(MEM_GX, sizeof(*wmo) * wmo_nb);
	if (!wmo)
		goto err;
	m2 = mem_malloc(MEM_GX, sizeof(*m2) * m2_nb);
	if (!m2)
		goto err;
	for (uint32_t i = 0; i < wmo_nb; ++i)
	{
		struct wow_modf_data *modf = &file->modf.data[i];
		struct map_wmo *handle;
		cache_lock_map_wmo(g_wow->cache);
		if (!cache_ref_unmutexed_map_wmo(g_wow->cache, modf->unique_id, &handle))
		{
			cache_unlock_map_wmo(g_wow->cache);
			LOG_ERROR("failed to get wmo handle: %d", modf->unique_id);
			wmo[i] = NULL;
			continue;
		}
		if (!handle->instance)
		{
			char filename[512];
			snprintf(filename, sizeof(filename), "%s", &file->mwmo.data[file->mwid.data[modf->name_id]]);
			wow_mpq_normalize_mpq_fn(filename, sizeof(filename));
			map_wmo_load(handle, filename);
			cache_unlock_map_wmo(g_wow->cache);
			float offset = (32 * 16) * CHUNK_WIDTH;
			struct vec3f pos = {offset - modf->position.z, modf->position.y, -(offset - modf->position.x)};
			struct mat4f mat1;
			struct mat4f mat2;
			MAT4_IDENTITY(mat1);
			MAT4_TRANSLATE(mat2, mat1, pos);
			MAT4_ROTATEY(float, mat1, mat2,  (modf->rotation.y + 180.0f) / 180.0f * M_PI);
			MAT4_ROTATEZ(float, mat2, mat1, -(modf->rotation.x) / 180.0f * M_PI);
			MAT4_ROTATEX(float, mat1, mat2,  (modf->rotation.z) / 180.0f * M_PI);
			cache_lock_wmo(g_wow->cache);
			gx_wmo_instance_set_mat(handle->instance, &mat1);
			handle->instance->pos = pos;
			handle->instance->doodad_set = modf->doodad_set;
			if (gx_wmo_flag_get(handle->instance->parent, GX_WMO_FLAG_LOADED))
				gx_wmo_instance_on_load(handle->instance);
			else
				gx_wmo_ask_load(handle->instance->parent);
			cache_unlock_wmo(g_wow->cache);
		}
		else
		{
			cache_unlock_map_wmo(g_wow->cache);
		}
		wmo[i] = handle;
	}
	for (uint32_t i = 0; i < m2_nb; ++i)
	{
		struct wow_mddf_data *mddf = &file->mddf.data[i];
		struct map_m2 *handle;
		cache_lock_map_m2(g_wow->cache);
		if (!cache_ref_unmutexed_map_m2(g_wow->cache, mddf->unique_id, &handle))
		{
			cache_unlock_map_m2(g_wow->cache);
			LOG_ERROR("failed to get m2 handle: %d", mddf->unique_id);
			m2[i] = NULL;
			continue;
		}
		if (!handle->instance)
		{
			char filename[512];
			snprintf(filename, sizeof(filename), "%s", &file->mmdx.data[file->mmid.data[mddf->name_id]]);
			wow_mpq_normalize_m2_fn(filename, sizeof(filename));
			map_m2_load(handle, filename);
			cache_unlock_map_m2(g_wow->cache);
			float offset = (32 * 16) * CHUNK_WIDTH;
			struct vec3f pos = {offset - mddf->position.z, mddf->position.y, -(offset - mddf->position.x)};
			struct mat4f mat1;
			struct mat4f mat2;
			MAT4_IDENTITY(mat1);
			MAT4_TRANSLATE(mat2, mat1, pos);
			MAT4_ROTATEY(float, mat1, mat2,  (mddf->rotation.y + 180.0f) / 180.0f * M_PI);
			MAT4_ROTATEZ(float, mat2, mat1, -(mddf->rotation.x) / 180.0f * M_PI);
			MAT4_ROTATEX(float, mat1, mat2,  (mddf->rotation.z) / 180.0f * M_PI);
			MAT4_SCALEV(mat1, mat1, mddf->scale / 1024.0f);
			cache_lock_m2(g_wow->cache);
			handle->instance->scale = mddf->scale / 1024.0f;
			gx_m2_instance_set_mat(handle->instance, &mat1);
			handle->instance->pos = pos;
			gx_m2_ask_load(handle->instance->parent);
			cache_unlock_m2(g_wow->cache);
		}
		else
		{
			cache_unlock_map_m2(g_wow->cache);
		}
		m2[i] = handle;
	}
	tile->wmo_nb = wmo_nb;
	tile->wmo = wmo;
	tile->m2_nb = m2_nb;
	tile->m2 = m2;
	return true;

err:
	mem_free(MEM_GX, wmo);
	mem_free(MEM_GX, m2);
	return false;
}

static bool load(struct map_tile *tile, struct wow_adt_file *file)
{
	float tile_min_y = +9999;
	float tile_max_y = -9999;
	tile->pos = (struct vec3f){file->mcnk[0].header.position.x, 0, -file->mcnk[0].header.position.y};
	for (size_t i = 0; i < CHUNKS_PER_TILE; ++i)
	{
		struct wow_mcnk *wow_mcnk = &file->mcnk[i];
		struct map_chunk *chunk = &tile->chunks[i];
		VEC3_SETV(chunk->doodads_aabb.p0, -INFINITY);
		VEC3_SETV(chunk->doodads_aabb.p1, -INFINITY);
		VEC3_SETV(chunk->wmos_aabb.p0, -INFINITY);
		VEC3_SETV(chunk->wmos_aabb.p1, -INFINITY);
		if (wow_mcnk->header.flags & WOW_MCNK_FLAGS_MCSH)
			memcpy(chunk->shadow, wow_mcnk->mcsh.data, sizeof(chunk->shadow));
		else
			memset(chunk->shadow, 0, sizeof(chunk->shadow));
		chunk->holes = wow_mcnk->header.holes;
		chunk->doodads_nb = wow_mcnk->mcrf.doodads_nb;
		if (chunk->doodads_nb)
		{
			chunk->doodads = mem_malloc(MEM_GX, sizeof(*chunk->doodads) * chunk->doodads_nb);
			if (!chunk->doodads)
				return false;
			memcpy(chunk->doodads, wow_mcnk->mcrf.doodads, sizeof(*chunk->doodads) * chunk->doodads_nb);
			uint32_t *doodads_to_aabb = jks_array_grow(&chunk->doodads_to_aabb, chunk->doodads_nb);
			if (!doodads_to_aabb)
				return false;
			memcpy(doodads_to_aabb, wow_mcnk->mcrf.doodads, sizeof(*chunk->doodads) * chunk->doodads_nb);
		}
		chunk->wmos_nb = wow_mcnk->mcrf.wmos_nb;
		if (chunk->wmos_nb)
		{
			chunk->wmos = mem_malloc(MEM_GX, sizeof(*chunk->wmos) * chunk->wmos_nb);
			if (!chunk->wmos)
				return false;
			memcpy(chunk->wmos, wow_mcnk->mcrf.wmos, sizeof(*chunk->wmos) * chunk->wmos_nb);
			uint32_t *wmos_to_aabb = jks_array_grow(&chunk->wmos_to_aabb, chunk->wmos_nb);
			if (!wmos_to_aabb)
				return false;
			memcpy(wmos_to_aabb, wow_mcnk->mcrf.wmos, sizeof(*chunk->wmos) * chunk->wmos_nb);
		}
		float min_y = +999999;
		float max_y = -999999;
		for (size_t j = 0; j < 9 * 9 + 8 * 8; ++j)
		{
			float y = wow_mcnk->mcvt.data[j] + wow_mcnk->header.position.z;
			if (y > max_y)
				max_y = y;
			if (y < min_y)
				min_y = y;
			chunk->height[j] = y;
			chunk->norm[j * 3 + 0] =  wow_mcnk->mcnr.data[j * 3 + 0];
			chunk->norm[j * 3 + 1] =  wow_mcnk->mcnr.data[j * 3 + 2];
			chunk->norm[j * 3 + 2] = -wow_mcnk->mcnr.data[j * 3 + 1];
		}
		if (max_y > tile_max_y)
			tile_max_y= max_y;
		if (min_y < tile_min_y)
			tile_min_y = min_y;
		float base_x = -CHUNK_WIDTH * (i / 16);
		float base_z = +CHUNK_WIDTH * (i % 16);
		struct vec3f p0;
		struct vec3f p1;
		VEC3_SET(p0, base_x, min_y, base_z);
		VEC3_ADD(p0, p0, tile->pos);
		VEC3_SET(p1, base_x - CHUNK_WIDTH, max_y, base_z + CHUNK_WIDTH);
		VEC3_ADD(p1, p1, tile->pos);
		VEC3_MIN(chunk->aabb.p0, p0, p1);
		VEC3_MAX(chunk->aabb.p1, p0, p1);
		VEC3_ADD(chunk->center, chunk->aabb.p0, chunk->aabb.p1);
		VEC3_MULV(chunk->center, chunk->center, .5f);
		p0.y = -999999;
		p1.y = +999999;
		VEC3_MIN(chunk->objects_aabb.p0, p0, p1);
		VEC3_MAX(chunk->objects_aabb.p1, p0, p1);
#ifdef WITH_DEBUG_RENDERING
		gx_aabb_set_aabb(&chunk->gx_aabb, &chunk->aabb);
#endif
		init_ground_effects(chunk, wow_mcnk);
	}
	struct vec3f p0;
	struct vec3f p1;
	VEC3_SET(p0, 0, tile_min_y, 0);
	VEC3_ADD(p0, p0, tile->pos);
	VEC3_SET(p1, -CHUNK_WIDTH * 16, tile_max_y, CHUNK_WIDTH * 16);
	VEC3_ADD(p1, p1, tile->pos);
	VEC3_MIN(tile->aabb.p0, p0, p1);
	VEC3_MAX(tile->aabb.p1, p0, p1);
	p0.y = -9999;
	p1.y = +9999;
	VEC3_MIN(tile->objects_aabb.p0, p0, p1);
	VEC3_MAX(tile->objects_aabb.p1, p0, p1);
	VEC3_ADD(tile->center, tile->aabb.p0, tile->aabb.p1);
	VEC3_MULV(tile->center, tile->center, .5);
#ifdef WITH_DEBUG_RENDERING
	gx_aabb_set_aabb(&tile->gx_aabb, &tile->aabb);
#endif
	tile->gx_mcnk = gx_mcnk_new(tile, file);
	if (!tile->gx_mcnk)
	{
		LOG_ERROR("failed to load mcnk");
		return true;
	}
	{
		bool has_liquids = false;
		for (uint32_t i = 0; i < sizeof(file->mcnk) / sizeof(*file->mcnk); ++i)
		{
			struct wow_mcnk *mcnk = &file->mcnk[i];
			if (mcnk->header.size_mclq <= 8 || memcmp("QLCM", (uint8_t*)&mcnk->mclq.header.magic, 4))
				continue;
			has_liquids = true;
			break;
		}
		if (has_liquids)
		{
			tile->gx_mclq = gx_mclq_new(tile, file);
			if (!tile->gx_mclq)
			{
				LOG_ERROR("failed to load mclq");
				return true;
			}
		}
	}
	return true;
}

static void map_tile_load_task(struct wow_mpq_compound *mpq_compound, void *userdata)
{
	struct map_tile *tile = userdata;
	struct wow_mpq_file *mpq_file = NULL;
	struct wow_adt_file *adt_file = NULL;

	mpq_file = wow_mpq_get_file(mpq_compound, tile->filename);
	if (!mpq_file)
	{
		LOG_WARN("ADT file not found: %s", tile->filename);
		goto end;
	}
	adt_file = wow_adt_file_new(mpq_file);
	if (!adt_file)
	{
		LOG_ERROR("failed to create adt from file %s", tile->filename);
		goto end;
	}
	if (!load_childs(tile, adt_file))
	{
		LOG_ERROR("failed to load adt childs");
		goto end;
	}
	if (!load(tile, adt_file))
	{
		LOG_ERROR("failed to load adt");
		goto end;
	}
	tile->flags |= MAP_TILE_FLAG_LOADED;
	loader_init_object(g_wow->loader, LOADER_MAP_TILE, initialize, tile);

end:
	wow_adt_file_delete(adt_file);
	wow_mpq_file_delete(mpq_file);
	tile->flags &= ~MAP_TILE_FLAG_LOADING;
	map_tile_free(tile);
}

void map_tile_ask_load(struct map_tile *tile)
{
	if (tile->flags & (MAP_TILE_FLAG_LOADING | MAP_TILE_FLAG_LOADED))
		return;
	tile->flags |= MAP_TILE_FLAG_LOADING;
	map_tile_ref(tile);
	loader_push(g_wow->loader, ASYNC_TASK_MAP_TILE_LOAD, map_tile_load_task, tile);
}

#ifdef WITH_DEBUG_RENDERING
static void cull_aabb(struct map_tile *tile, struct gx_mcnk *mcnk, struct gx_frame *frame)
{
	if (tile->flags & MAP_TILE_FLAG_DOODADS_AABB)
	{
		switch (tile->doodads_frustum_result)
		{
			case FRUSTUM_OUTSIDE:
				gx_aabb_set_color(&tile->doodads_gx_aabb, &(struct vec4f){1, 0, 0, 1});
				break;
			case FRUSTUM_COLLIDE:
				gx_aabb_set_color(&tile->doodads_gx_aabb, &(struct vec4f){1, .5, 0, 1});
				break;
			case FRUSTUM_INSIDE:
				gx_aabb_set_color(&tile->doodads_gx_aabb, &(struct vec4f){0, 1, 0, 1});
				break;
		}
		gx_aabb_add_to_render(&tile->doodads_gx_aabb, frame, &frame->view_vp);
	}
	if (tile->flags & MAP_TILE_FLAG_WMOS_AABB)
		gx_aabb_add_to_render(&tile->wmos_gx_aabb, frame, &frame->view_vp);
	gx_aabb_add_to_render(&tile->gx_aabb, frame, &frame->view_vp);
	for (uint32_t i = 0; i < CHUNKS_PER_TILE; ++i)
	{
		struct gx_mcnk_chunk *gx_chunk = &mcnk->chunks[i];
		struct map_chunk *chunk = &tile->chunks[i];
		if (chunk->doodads_to_aabb.size != chunk->doodads_nb)
		{
			switch (chunk->doodads_frustum_result)
			{
				case FRUSTUM_OUTSIDE:
					gx_aabb_set_color(&chunk->doodads_gx_aabb, &(struct vec4f){1, 0, 0, 1});
					break;
				case FRUSTUM_COLLIDE:
					gx_aabb_set_color(&chunk->doodads_gx_aabb, &(struct vec4f){1, .5, 0, 1});
					break;
				case FRUSTUM_INSIDE:
					gx_aabb_set_color(&chunk->doodads_gx_aabb, &(struct vec4f){0, 1, 0, 1});
					break;
			}
			gx_aabb_add_to_render(&chunk->doodads_gx_aabb, frame, &frame->view_vp);
		}
		if (chunk->wmos_to_aabb.size != chunk->wmos_nb)
			gx_aabb_add_to_render(&chunk->wmos_gx_aabb, frame, &frame->view_vp);
		if (gx_chunk->frames[frame->id].culled)
			continue;
		gx_aabb_add_to_render(&chunk->gx_aabb, frame, &frame->view_vp);
	}
}
#endif

static void cull_objects(struct map_tile *tile, struct gx_frame *frame)
{
	uint8_t doodads_chunks[CHUNKS_PER_TILE];
	size_t doodads_chunks_count = 0;
	uint8_t wmos_chunks[CHUNKS_PER_TILE];
	size_t wmos_chunks_count = 0;
	for (uint32_t i = 0; i < CHUNKS_PER_TILE; ++i)
	{
		struct map_chunk *chunk = &tile->chunks[i];
		for (size_t j = 0; j < chunk->doodads_to_aabb.size; ++j)
		{
			struct gx_m2_instance *m2 = tile->m2[*JKS_ARRAY_GET(&chunk->doodads_to_aabb, j, uint32_t)]->instance;
			if (!gx_m2_flag_get(m2->parent, GX_M2_FLAG_LOADED))
				continue;
			if (chunk->doodads_to_aabb.size == chunk->doodads_nb)
				aabb_set_min_max(&chunk->doodads_aabb, &m2->aabb);
			else
				aabb_add_min_max(&chunk->doodads_aabb, &m2->aabb);
			aabb_sub_min_max(&chunk->doodads_aabb, &chunk->objects_aabb); /* avoid aabb from going outside of chunk */
			if (!(tile->flags & MAP_TILE_FLAG_DOODADS_AABB))
			{
				aabb_set_min_max(&tile->doodads_aabb, &chunk->doodads_aabb);
				tile->flags |= MAP_TILE_FLAG_DOODADS_AABB;
			}
			else
			{
				aabb_add_min_max(&tile->doodads_aabb, &chunk->doodads_aabb);
			}
#ifdef WITH_DEBUG_RENDERING
			gx_aabb_set_aabb(&tile->doodads_gx_aabb, &tile->doodads_aabb);
			gx_aabb_set_aabb(&chunk->doodads_gx_aabb, &chunk->doodads_aabb);
#endif
			jks_array_erase(&chunk->doodads_to_aabb, j);
			j--;
			if (!chunk->doodads_to_aabb.size)
				jks_array_shrink(&chunk->doodads_to_aabb);
		}
		for (size_t j = 0; j < chunk->wmos_to_aabb.size; ++j)
		{
			struct gx_wmo_instance *wmo = tile->wmo[*JKS_ARRAY_GET(&chunk->wmos_to_aabb, j, uint32_t)]->instance;
			if (!gx_wmo_flag_get(wmo->parent, GX_WMO_FLAG_LOADED))
				continue;
			if (chunk->wmos_to_aabb.size == chunk->wmos_nb)
				aabb_set_min_max(&chunk->wmos_aabb, &wmo->aabb);
			else
				aabb_add_min_max(&chunk->wmos_aabb, &wmo->aabb);
			aabb_sub_min_max(&chunk->wmos_aabb, &chunk->objects_aabb); /* avoid aabb from going outside of chunk */
			if (!(tile->flags & MAP_TILE_FLAG_WMOS_AABB))
			{
				aabb_set_min_max(&tile->wmos_aabb, &chunk->wmos_aabb);
				tile->flags |= MAP_TILE_FLAG_WMOS_AABB;
			}
			else
			{
				aabb_add_min_max(&tile->wmos_aabb, &chunk->wmos_aabb);
			}
#ifdef WITH_DEBUG_RENDERING
			gx_aabb_set_aabb(&tile->wmos_gx_aabb, &tile->wmos_aabb);
			gx_aabb_set_aabb(&chunk->wmos_gx_aabb, &chunk->wmos_aabb);
#endif
			jks_array_erase(&chunk->wmos_to_aabb, j);
			j--;
			if (!chunk->wmos_to_aabb.size)
				jks_array_shrink(&chunk->wmos_to_aabb);
		}
		if (chunk->doodads_to_aabb.size == chunk->doodads_nb)
			chunk->doodads_frustum_result = FRUSTUM_OUTSIDE;
		else if (chunk->doodads_nb < 3) /* threshold where multiplying frustums harms */
			chunk->doodads_frustum_result = FRUSTUM_COLLIDE;
		else
			doodads_chunks[doodads_chunks_count++] = i;
		if (chunk->wmos_to_aabb.size == chunk->wmos_nb)
			chunk->wmos_frustum_result = FRUSTUM_OUTSIDE;
		else if (chunk->wmos_nb < 3) /* threshold where multiplying frustums harms */
			chunk->wmos_frustum_result = FRUSTUM_COLLIDE;
		else
			wmos_chunks[wmos_chunks_count++] = i;
	}
	if (tile->flags & MAP_TILE_FLAG_DOODADS_AABB)
	{
		tile->doodads_frustum_result = frustum_check(&frame->frustum, &tile->doodads_aabb);
		if (tile->doodads_frustum_result == FRUSTUM_COLLIDE)
		{
			for (size_t i = 0; i < doodads_chunks_count; ++i)
			{
				struct map_chunk *chunk = &tile->chunks[doodads_chunks[i]];
				chunk->doodads_frustum_result = frustum_check(&frame->frustum, &tile->chunks[doodads_chunks[i]].doodads_aabb);
			}
		}
	}
	else
	{
		tile->doodads_frustum_result = FRUSTUM_OUTSIDE;
	}
	if (tile->flags & MAP_TILE_FLAG_WMOS_AABB)
	{
		tile->wmos_frustum_result = frustum_check(&frame->frustum, &tile->wmos_aabb);
		if (tile->wmos_frustum_result == FRUSTUM_COLLIDE)
		{
			for (size_t i = 0; i < wmos_chunks_count; ++i)
			{
				struct map_chunk *chunk = &tile->chunks[wmos_chunks[i]];
				chunk->wmos_frustum_result = frustum_check(&frame->frustum, &tile->chunks[wmos_chunks[i]].wmos_aabb);
			}
		}
	}
	else
	{
		tile->wmos_frustum_result = FRUSTUM_OUTSIDE;
	}
}

void map_tile_cull(struct map_tile *tile, struct gx_frame *frame)
{
	if (!(tile->flags & MAP_TILE_FLAG_INITIALIZED))
		return;
	if (map_tile_flag_set(tile, MAP_TILE_FLAG_IN_RENDER_LIST))
		return;
	map_tile_ref(tile);
	gx_frame_add_tile(frame, tile);
	gx_mcnk_cull(tile->gx_mcnk, frame);
	if (frustum_check_fast(&frame->frustum, &tile->objects_aabb))
		add_objects_to_render(tile, frame);
	if (g_wow->wow_opt & WOW_OPT_AABB_OPTIMIZE)
		cull_objects(tile, frame);
#ifdef WITH_DEBUG_RENDERING
	if (g_wow->render_opt & RENDER_OPT_MCNK_AABB)
		cull_aabb(tile, tile->gx_mcnk, frame);
#endif
	if (tile->gx_mclq)
		gx_mclq_cull(tile->gx_mclq, frame);
}

static void add_point(struct map_tile *tile, struct map_chunk *chunk, uint8_t chunk_id, uint16_t idx, struct vec3f *p)
{
	size_t y = idx % 17;
	float z = idx / 17 * 2;
	float x;
	if (y < 9)
	{
		x = y * 2;
	}
	else
	{
		z++;
		x = (y - 9) * 2 + 1;
	}
	p->x = tile->pos.x + (-1 - (ssize_t)(chunk_id / 16) + (16 - z) / 16.f) * CHUNK_WIDTH;
	p->z = tile->pos.z + ((1 + (ssize_t)(chunk_id % 16) - (16 - x) / 16.f) * CHUNK_WIDTH);
	p->y = chunk->height[idx];
}

static void add_chunk(struct map_tile *tile, struct map_chunk *chunk, uint8_t chunk_id, const struct collision_params *params, struct jks_array *triangles)
{
	float xmin = tile->pos.x - (1 + (ssize_t)(chunk_id / 16)) * CHUNK_WIDTH;
	ssize_t z_end = floorf((params->aabb.p0.x - xmin) / (CHUNK_WIDTH / 8));
	z_end = 8 - z_end;
	if (z_end > 8)
		z_end = 8;
	else if (z_end < 8)
		z_end++;
	ssize_t z_start = ceilf((params->aabb.p1.x - xmin) / (CHUNK_WIDTH / 8));
	z_start = 8 - z_start;
	if (z_start < 0)
		z_start = 0;
	else if (z_start > 0)
		z_start--;
	if (z_start >= z_end)
		return;
	float zmin = tile->pos.z + (chunk_id % 16) * CHUNK_WIDTH;
	ssize_t x_end = ceilf((params->aabb.p1.z - zmin) / (CHUNK_WIDTH / 8));
	if (x_end > 8)
		x_end = 8;
	else if (x_end < 8)
		x_end++;
	ssize_t x_start = floorf((params->aabb.p0.z - zmin) / (CHUNK_WIDTH / 8));
	if (x_start < 0)
		x_start = 0;
	else if (x_start > 0)
		x_start--;
	if (x_start >= x_end)
		return;
	size_t triangles_nb = triangles->size;
	struct collision_triangle *tmp = jks_array_grow(triangles, (z_end - z_start) * (x_end - x_start) * 4);
	if (!tmp)
	{
		LOG_ERROR("triangles allocation failed");
		return;
	}
	size_t n = 0;
	for (ssize_t z = z_start; z < z_end; ++z)
	{
		for (ssize_t x = x_start; x < x_end; ++x)
		{
			if (chunk->holes & (1 << (z / 2 * 4 + x / 2)))
				continue;
			n++;
			uint16_t idx = 9 + z * 17 + x;
			uint16_t p1 = idx - 9;
			uint16_t p2 = idx - 8;
			uint16_t p3 = idx + 9;
			uint16_t p4 = idx + 8;
			add_point(tile, chunk, chunk_id, p2, &tmp->points[0]);
			add_point(tile, chunk, chunk_id, p1, &tmp->points[1]);
			add_point(tile, chunk, chunk_id, idx, &tmp->points[2]);
			tmp->touched = false;
			tmp++;
			add_point(tile, chunk, chunk_id, p3, &tmp->points[0]);
			add_point(tile, chunk, chunk_id, p2, &tmp->points[1]);
			add_point(tile, chunk, chunk_id, idx, &tmp->points[2]);
			tmp->touched = false;
			tmp++;
			add_point(tile, chunk, chunk_id, p4, &tmp->points[0]);
			add_point(tile, chunk, chunk_id, p3, &tmp->points[1]);
			add_point(tile, chunk, chunk_id, idx, &tmp->points[2]);
			tmp->touched = false;
			tmp++;
			add_point(tile, chunk, chunk_id, p1, &tmp->points[0]);
			add_point(tile, chunk, chunk_id, p4, &tmp->points[1]);
			add_point(tile, chunk, chunk_id, idx, &tmp->points[2]);
			tmp->touched = false;
			tmp++;
		}
	}
	jks_array_resize(triangles, triangles_nb + n * 4);
}

static void add_object_point(struct gx_m2_instance *m2, struct vec3f *point, uint16_t idx)
{
	struct vec4f tmp;
	VEC3_CPY(tmp, m2->parent->collision_vertexes[idx]);
	tmp.w = 1;
	struct vec4f out;
	MAT4_VEC4_MUL(out, m2->m, tmp);
	VEC3_CPY(*point, out);
}

static void add_object(struct gx_m2_instance *m2, struct jks_array *triangles)
{
	if (!m2->parent->collision_triangles_nb)
		return;
	struct collision_triangle *tmp = jks_array_grow(triangles, m2->parent->collision_triangles_nb / 3);
	if (!tmp)
	{
		LOG_ERROR("triangles allocation failed");
		return;
	}
	for (size_t i = 0; i < m2->parent->collision_triangles_nb;)
	{
		add_object_point(m2, &tmp->points[0], m2->parent->collision_triangles[i++]);
		add_object_point(m2, &tmp->points[1], m2->parent->collision_triangles[i++]);
		add_object_point(m2, &tmp->points[2], m2->parent->collision_triangles[i++]);
		tmp->touched = false;
		tmp++;
	}
}

static void add_objects(struct map_tile *tile, struct map_chunk *chunk, const struct collision_params *params, struct collision_state *state, struct jks_array *triangles)
{
	for (size_t i = 0; i < chunk->doodads_nb; ++i)
	{
		struct map_m2 *m2 = tile->m2[chunk->doodads[i]];
		if (!gx_m2_flag_get(m2->instance->parent, GX_M2_FLAG_LOADED))
			continue;
		bool m2_found = false;
		for (size_t j = 0; j < state->m2.size; ++j)
		{
			if (*JKS_ARRAY_GET(&state->m2, j, struct gx_m2_instance*) == m2->instance)
			{
				m2_found = true;
				break;
			}
		}
		if (m2_found)
			continue;
		if (!jks_array_push_back(&state->m2, &m2->instance))
			LOG_WARN("failed to add m2 to visited list");
#if 0 /* doesn't work on some objects */
		struct vec3f delta;
		VEC3_SUB(delta, m2->instance->pos, params->center);
		if (VEC3_NORM(delta) > m2->instance->parent->collision_sphere_radius * m2->instance->scale + params->radius)
			continue;
#endif
		if (!aabb_intersect_sphere(&m2->instance->caabb, params->center, params->radius)
		 || !aabb_intersect_aabb(&m2->instance->caabb, &params->aabb))
			continue;
		add_object(m2->instance, triangles);
	}
}

static void add_wmo_point(struct gx_wmo_instance *wmo, struct gx_wmo_group *group, struct vec3f *point, uint32_t indice)
{
	struct wow_vec3f *src = JKS_ARRAY_GET(&group->movt, *JKS_ARRAY_GET(&group->movi, indice, uint16_t), struct wow_vec3f);
	struct vec4f tmp;
	VEC3_CPY(tmp, *src);
	tmp.w = 1;
	struct vec4f out;
	MAT4_VEC4_MUL(out, wmo->m, tmp);
	VEC3_CPY(*point, out);
}

enum bsp_side
{
	BSP_POS = (1 << 0),
	BSP_NEG = (1 << 1),
};

static enum bsp_side get_aabb_bsp_side(const struct aabb *aabb, const struct wow_mobn_node *node)
{
	enum bsp_side side = 0;
	uint8_t plane = node->flags & WOW_MOBN_NODE_FLAGS_AXIS_MASK;
	if (((float*)&aabb->p0)[plane] <= node->plane_dist)
		side |= BSP_NEG;
	if (((float*)&aabb->p1)[plane] >= node->plane_dist)
		side |= BSP_POS;
	return side;
}

static void bsp_add_triangles(struct gx_wmo_instance *wmo, struct gx_wmo_group *group, const struct wow_mobn_node *node, const struct collision_params *params, struct jks_array *triangles, struct jks_array *triangles_tracker)
{
	if (!node->faces_nb)
		return;
	struct collision_triangle *tmp = jks_array_grow(triangles, node->faces_nb);
	if (!tmp)
	{
		LOG_ERROR("triangles allocation failed");
		return;
	}
	size_t n = 0;
	for (size_t i = 0; i < node->faces_nb; ++i)
	{
		assert(node->face_start + i < group->mobr.size);
		uint32_t indice = *JKS_ARRAY_GET(&group->mobr, node->face_start + i, uint16_t);
		uint8_t flags = JKS_ARRAY_GET(&group->mopy, indice, struct wow_mopy_data)->flags;
		if (!((flags & WOW_MOPY_FLAGS_COLLISION) || ((flags & WOW_MOPY_FLAGS_RENDER) && !(flags & WOW_MOPY_FLAGS_DETAIL))))
			continue;
		if (params->wmo_cam && (flags & WOW_MOPY_FLAGS_NOCAMCOLLIDE))
			continue;
		bool triangle_found = false;
		for (size_t j = 0; j < triangles_tracker->size; ++j)
		{
			if (*JKS_ARRAY_GET(triangles_tracker, j, uint32_t) == indice)
			{
				triangle_found = true;
				break;
			}
		}
		if (triangle_found)
			continue;
		if (!jks_array_push_back(triangles_tracker, &indice))
			LOG_ERROR("failed to add triangle to triangles tracker");
		indice *= 3;
		add_wmo_point(wmo, group, &tmp->points[0], indice + 0);
		add_wmo_point(wmo, group, &tmp->points[1], indice + 1);
		add_wmo_point(wmo, group, &tmp->points[2], indice + 2);
		tmp->touched = false;
		tmp++;
		n++;
	}
	jks_array_resize(triangles, triangles->size - (node->faces_nb - n));
}

static void bsp_traverse(struct gx_wmo_instance *wmo, struct gx_wmo_group *group, const struct wow_mobn_node *node, const struct aabb *aabb, const struct collision_params *params, struct jks_array *triangles, struct jks_array *triangles_tracker)
{
	if (node->flags & WOW_MOBN_NODE_FLAGS_LEAF)
	{
		bsp_add_triangles(wmo, group, node, params, triangles, triangles_tracker);
		return;
	}
	enum bsp_side side = get_aabb_bsp_side(aabb, node);
	if ((side & BSP_POS) && node->pos_child != -1)
		bsp_traverse(wmo, group, JKS_ARRAY_GET(&group->mobn, node->pos_child, struct wow_mobn_node), aabb, params, triangles, triangles_tracker);
	if ((side & BSP_NEG) && node->neg_child != -1)
		bsp_traverse(wmo, group, JKS_ARRAY_GET(&group->mobn, node->neg_child, struct wow_mobn_node), aabb, params, triangles, triangles_tracker);
}

static void add_wmo_group(struct gx_wmo_instance *wmo, struct gx_wmo_group *group, const struct collision_params *params, struct collision_state *state, struct jks_array *triangles, struct jks_array *triangles_tracker)
{
	for (size_t i = 0; i < group->doodads.size; ++i)
	{
		uint16_t doodad = *JKS_ARRAY_GET(&group->doodads, i, uint16_t);
		if (doodad < wmo->doodad_start || doodad >= wmo->doodad_end)
			continue;
		struct gx_m2_instance *m2 = *JKS_ARRAY_GET(&wmo->m2, doodad - wmo->doodad_start, struct gx_m2_instance*);
		if (!gx_m2_flag_get(m2->parent, GX_M2_FLAG_LOADED))
			continue;
		bool m2_found = false;
		for (size_t j = 0; j < state->m2.size; ++j)
		{
			if (*JKS_ARRAY_GET(&state->m2, j, struct gx_m2_instance*) == m2)
			{
				m2_found = true;
				break;
			}
		}
		if (m2_found)
			continue;
		if (!jks_array_push_back(&state->m2, &m2))
			LOG_WARN("failed to add m2 to visited list");
		if (!aabb_intersect_sphere(&m2->caabb, params->center, params->radius)
		 || !aabb_intersect_aabb(&m2->caabb, &params->aabb))
			continue;
		add_object(m2, triangles);
	}
	if (group->mobn.size)
	{
		struct vec4f p0;
		struct vec4f p1;
		struct vec4f tmp0;
		struct vec4f tmp1;
		struct aabb aabb;
		VEC3_CPY(p0, params->aabb.p0);
		VEC3_CPY(p1, params->aabb.p1);
		p0.w = 1;
		p1.w = 1;
		MAT4_VEC4_MUL(tmp0, wmo->m_inv, p0);
		MAT4_VEC4_MUL(tmp1, wmo->m_inv, p1);
		p0.x = tmp0.x;
		p0.y = -tmp0.z;
		p0.z = tmp0.y;
		p1.x = tmp1.x;
		p1.y = -tmp1.z;
		p1.z = tmp1.y;
		VEC3_MIN(aabb.p0, p0, p1);
		VEC3_MAX(aabb.p1, p0, p1);
		jks_array_resize(triangles_tracker, 0);
		bsp_traverse(wmo, group, group->mobn.data, &aabb, params, triangles, triangles_tracker);
	}
}

static void add_wmo(struct gx_wmo_instance *wmo, const struct collision_params *params, struct collision_state *state, struct jks_array *triangles)
{
	struct jks_array triangles_tracker;
	jks_array_init(&triangles_tracker, sizeof(uint32_t), NULL, NULL);
	for (size_t i = 0; i < wmo->groups.size; ++i)
	{
		struct gx_wmo_group *group = *JKS_ARRAY_GET(&wmo->parent->groups, i, struct gx_wmo_group*);
		if (!gx_wmo_group_flag_get(group, GX_WMO_GROUP_FLAG_LOADED))
			continue;
		struct gx_wmo_group_instance *group_instance = JKS_ARRAY_GET(&wmo->groups, i, struct gx_wmo_group_instance);
		if (!aabb_intersect_sphere(&group_instance->aabb, params->center, params->radius))
			continue;
		add_wmo_group(wmo, group, params, state, triangles, &triangles_tracker);
	}
	jks_array_destroy(&triangles_tracker);
}

static void add_wmos(struct map_tile *tile, struct map_chunk *chunk, const struct collision_params *params, struct collision_state *state, struct jks_array *triangles)
{
	for (size_t i = 0; i < chunk->wmos_nb; ++i)
	{
		struct map_wmo *wmo = tile->wmo[chunk->wmos[i]];
		if (!gx_wmo_flag_get(wmo->instance->parent, GX_WMO_FLAG_LOADED))
			continue;
		bool wmo_found = false;
		for (size_t j = 0; j < state->wmo.size; ++j)
		{
			if (*JKS_ARRAY_GET(&state->wmo, j, struct gx_wmo_instance*) == wmo->instance)
			{
				wmo_found = true;
				break;
			}
		}
		if (wmo_found)
			continue;
		if (!jks_array_push_back(&state->wmo, &wmo->instance))
			LOG_WARN("failed to add wmo to visited list");
		if (!aabb_intersect_sphere(&wmo->instance->aabb, params->center, params->radius))
			continue;
		add_wmo(wmo->instance, params, state, triangles);
	}
}

void map_tile_collect_collision_triangles(struct map_tile *tile, const struct collision_params *params, struct collision_state *state, struct jks_array *triangles)
{
	if (!(tile->flags & MAP_TILE_FLAG_LOADED))
		return;
	float xmin = tile->pos.x - CHUNK_WIDTH * 16;
	ssize_t z_end = floorf((params->aabb.p0.x - xmin) / CHUNK_WIDTH);
	z_end = 16 - z_end;
	if (z_end > 16)
		z_end = 16;
	else if (z_end < 16)
		z_end++;
	ssize_t z_start = ceilf((params->aabb.p1.x - xmin) / CHUNK_WIDTH);
	z_start = 16 - z_start;
	if (z_start < 0)
		z_start = 0;
	else if (z_start > 0)
		z_start--;
	if (z_start >= z_end)
		return;
	float zmin = tile->pos.z;
	ssize_t x_end = ceilf((params->aabb.p1.z - zmin) / CHUNK_WIDTH);
	if (x_end > 16)
		x_end = 16;
	else if (x_end < 16)
		x_end++;
	ssize_t x_start = floorf((params->aabb.p0.z - zmin) / CHUNK_WIDTH);
	if (x_start < 0)
		x_start = 0;
	else if (x_start > 0)
		x_start--;
	if (x_start >= x_end)
		return;
	if (aabb_intersect_aabb(&tile->aabb, &params->aabb))
	{
		for (ssize_t x = x_start; x < x_end; ++x)
		{
			for (ssize_t z = z_start; z < z_end; ++z)
			{
				uint8_t chunk_id = z * 16 + x;
				struct map_chunk *chunk = &tile->chunks[chunk_id];
				if (aabb_intersect_aabb(&chunk->aabb, &params->aabb))
					add_chunk(tile, chunk, chunk_id, params, triangles);
			}
		}
	}
	if (aabb_intersect_aabb(&tile->objects_aabb, &params->aabb))
	{
		for (ssize_t x = x_start; x < x_end; ++x)
		{
			for (ssize_t z = z_start; z < z_end; ++z)
			{
				uint8_t chunk_id = z * 16 + x;
				struct map_chunk *chunk = &tile->chunks[chunk_id];
				if (chunk->doodads_nb && aabb_intersect_aabb(&chunk->objects_aabb, &params->aabb))
					add_objects(tile, chunk, params, state, triangles);
			}
		}
	}
	if (aabb_intersect_aabb(&tile->wmos_aabb, &params->aabb))
	{
		for (ssize_t x = x_start; x < x_end; ++x)
		{
			for (ssize_t z = z_start; z < z_end; ++z)
			{
				uint8_t chunk_id = z * 16 + x;
				struct map_chunk *chunk = &tile->chunks[chunk_id];
				if (chunk->wmos_nb && aabb_intersect_aabb(&chunk->wmos_aabb, &params->aabb))
					add_wmos(tile, chunk, params, state, triangles);
			}
		}
	}
}

static void init_ground_effects(struct map_chunk *chunk, struct wow_mcnk *wow_mcnk)
{
	chunk->no_effect_doodads = wow_mcnk->header.no_effect_doodads;
	chunk->ground_effect_loaded = false;
	for (uint32_t z = 0; z < 8; ++z)
	{
		for (uint32_t x = 0; x < 8; ++x)
		{
			if (wow_mcnk->header.layers)
			{
				uint8_t layer = (wow_mcnk->header.low_quality_texture[z] >> (x * 2)) & 0x3;
				chunk->effect_id[z][x] = wow_mcnk->mcly.data[layer].effect_id;
			}
			else
			{
				chunk->effect_id[z][x] = 0;
			}
		}
	}
}

static void load_ground_effect_doodads(struct map_tile *tile, struct map_chunk *chunk, uint32_t chunk_id)
{
	if (!(g_wow->render_opt & RENDER_OPT_GROUND_EFFECT))
		return;
	if (chunk->ground_effect_loaded)
		return;
	chunk->ground_effect_loaded = true;
	if (chunk->no_effect_doodads == (uint64_t)-1)
		return;
	uint32_t random_generator = rand();
	float chunk_x = +CHUNK_WIDTH - CHUNK_WIDTH * (chunk_id / 16);
	float chunk_z = -CHUNK_WIDTH + CHUNK_WIDTH * (chunk_id % 16);
	for (size_t z = 0; z < 8; ++z)
	{
		for (size_t x = 0; x < 8; ++x)
		{
			uint16_t effect_id = chunk->effect_id[z][x];
			if (effect_id == UINT16_MAX)
				continue;
			if ((chunk->no_effect_doodad[z] >> x) & 1)
				continue;
			if (chunk->holes & (1 << (z / 2 * 4 + x / 2)))
				continue;
			struct wow_dbc_row row; /* XXX use cache for dbc_row of ground_effect_texture & ground_effect_doodad to lower CPU usage */
			if (!dbc_get_row_indexed(g_wow->dbc.ground_effect_texture, &row, effect_id))
				continue;
			uint32_t density = wow_dbc_get_u32(&row, 20);
			if (!density)
				continue;
			for (uint32_t i = 0; i < 4; ++i)
			{
				uint32_t doodad_id = wow_dbc_get_u32(&row, 4 + i * 4);
				if (doodad_id == UINT32_MAX)
					break;
				struct wow_dbc_row doodad_row;
				if (!dbc_get_row_indexed(g_wow->dbc.ground_effect_doodad, &doodad_row, doodad_id))
					continue;
				char path[512];
				snprintf(path, sizeof(path), "World/NoDXT/Detail/%s", wow_dbc_get_str(&doodad_row, 8));
				struct gx_m2 *m2;
				wow_mpq_normalize_m2_fn(path, sizeof(path));
				if (!cache_ref_m2(g_wow->cache, path, &m2))
				{
					LOG_ERROR("failed to get m2 instance ref: %s", path);
					continue;
				}
				gx_m2_ask_load(m2);
				struct map_tile_ground_effect *tile_ground_effect = NULL;
				size_t tile_ground_index = 0;
				for (size_t k = 0; k < tile->ground_effects.size; ++k)
				{
					struct map_tile_ground_effect *it = JKS_ARRAY_GET(&tile->ground_effects, k, struct map_tile_ground_effect);
					if (it->light_batch->m2 == m2)
					{
						tile_ground_effect = it;
						tile_ground_index = k;
						break;
					}
				}
				if (!tile_ground_effect)
				{
					tile_ground_effect = jks_array_grow(&tile->ground_effects, 1);
					assert(tile_ground_effect);
					tile_ground_index = tile->ground_effects.size - 1;
					tile_ground_effect->light_batch = gx_m2_ground_batch_new(m2, true);
					assert(tile_ground_effect->light_batch);
					tile_ground_effect->shadow_batch = gx_m2_ground_batch_new(m2, false);
					assert(tile_ground_effect->shadow_batch);
				}
				for (uint32_t n = 0; n < density / 4; ++n)
				{
					float px = x + random_generator / (float)UINT_MAX;
					random_generator = random_generator * 69069 + 1;
					float pz = z + random_generator / (float)UINT_MAX;
					random_generator = random_generator * 69069 + 1;
					float base_x = chunk_x - pz / 8.0f * CHUNK_WIDTH - CHUNK_WIDTH;
					float base_z = chunk_z + px / 8.0f * CHUNK_WIDTH + CHUNK_WIDTH;
					uint8_t points[3];
					float factors[3];
					struct vec3f normal;
					float y;
					map_chunk_get_interp_points(px, pz, points);
					map_chunk_get_interp_factors(px, pz, points, factors);
					map_chunk_get_y(chunk, px, pz, points, factors, &y);
					map_chunk_get_normal(chunk, px, pz, points, factors, &normal);
					struct vec3f pos = {base_x, y, base_z};
					VEC3_ADD(pos, pos, tile->pos);
					struct mat4f mat;
					struct mat4f m;
					struct mat4f rot;
					MAT4_IDENTITY(mat);
					MAT4_ROTATEY(float, rot, mat, M_PI * (random_generator / (float)UINT_MAX) - 0.5f);
					random_generator = random_generator * 69069 + 1;
					static const struct vec3f right = {1.0f, 0.0, 0.0f};
					struct vec3f t0;
					VEC3_CROSS(t0, normal, right);
					if (VEC3_NORM(t0) < 0.001)
					{
						static const struct vec3f up = {0.0f, 1.0f, 0.0f};
						VEC3_CROSS(t0, normal, up);
					}
					VEC3_NORMALIZE(float, t0, t0);
					struct vec3f t1;
					VEC3_CROSS(t1, normal, t0);
					VEC3_NORMALIZE(float, t1, t1);
					VEC3_CPY(m.x, t0);
					m.x.w = 0;
					VEC3_CPY(m.y, normal);
					m.y.w = 0;
					VEC3_CPY(m.z, t1);
					m.z.w = 0;
					VEC3_CPY(m.w, pos);
					m.w.w = 1;
					MAT4_MUL(mat, m, rot);
					float render_distance = (0.2f + 0.8f * random_generator / (float)UINT_MAX) * (GROUND_DOODADS_RANGE_MIN - CHUNK_WIDTH);
					random_generator = random_generator * 69069 + 1;
					int32_t shadow_x = px * 8.0f;
					int32_t shadow_z = pz * 8.0f;
					struct map_chunk_ground_effect *ground_effect = NULL;
					for (size_t k = 0; k < chunk->ground_effects.size; ++k)
					{
						struct map_chunk_ground_effect *it = JKS_ARRAY_GET(&chunk->ground_effects, k, struct map_chunk_ground_effect);
						struct map_tile_ground_effect *tile_it = JKS_ARRAY_GET(&tile->ground_effects, it->tile_index, struct map_tile_ground_effect);
						if (tile_it->light_batch->m2 == m2)
						{
							ground_effect = it;
							break;
						}
					}
					if (!ground_effect)
					{
						ground_effect = jks_array_grow(&chunk->ground_effects, 1);
						assert(ground_effect);
						ground_effect->tile_index = tile_ground_index;
						jks_array_init(&ground_effect->light_doodads, sizeof(struct gx_m2_ground), NULL, &jks_array_memory_fn_MAP);
						jks_array_init(&ground_effect->shadow_doodads, sizeof(struct gx_m2_ground), NULL, &jks_array_memory_fn_MAP);
					}
					struct gx_m2_ground *doodad;
					if (chunk->shadow[shadow_z] & (1ULL << shadow_x))
						doodad = jks_array_grow(&ground_effect->shadow_doodads, 1);
					else
						doodad = jks_array_grow(&ground_effect->light_doodads, 1);
					assert(doodad);
					doodad->mat = mat;
					doodad->position = pos;
					doodad->render_distance = render_distance;
				}
				gx_m2_free(m2);
			}
		}
	}
}

static void unload_ground_effect_doodads(struct map_chunk *chunk)
{
	if (!chunk->ground_effect_loaded)
		return;
	jks_array_resize(&chunk->ground_effects, 0);
	chunk->ground_effect_loaded = false;
}

static void add_objects_to_render(struct map_tile *tile, struct gx_frame *frame)
{
	if (!(tile->gx_mcnk->flags & GX_MCNK_FLAG_INITIALIZED))
		return;
	for (size_t i = 0; i < CHUNKS_PER_TILE; ++i)
	{
		struct gx_mcnk_chunk *gx_chunk = &tile->gx_mcnk->chunks[i];
		struct map_chunk *chunk = &tile->chunks[i];
		if (gx_chunk->frames[frame->id].distance_to_camera > GROUND_DOODADS_RANGE_MAX)
			unload_ground_effect_doodads(chunk);
		if (gx_chunk->frames[frame->id].distance_to_camera < GROUND_DOODADS_RANGE_MIN)
			load_ground_effect_doodads(tile, chunk, i);
		if (gx_chunk->frames[frame->id].distance_to_camera > frame->view_distance + (CHUNK_WIDTH * 1.4142))
			continue;
		if ((g_wow->render_opt & RENDER_OPT_GROUND_EFFECT)
		 && !gx_chunk->frames[frame->id].culled)
		{
			for (size_t j = 0; j < chunk->ground_effects.size; ++j)
			{
				struct map_chunk_ground_effect *ground_effects = JKS_ARRAY_GET(&chunk->ground_effects, j, struct map_chunk_ground_effect);
				struct map_tile_ground_effect *tile_ground_effects = JKS_ARRAY_GET(&tile->ground_effects, ground_effects->tile_index, struct map_tile_ground_effect);
				gx_m2_ground_batch_cull(tile_ground_effects->light_batch, frame, ground_effects->light_doodads.data, ground_effects->light_doodads.size);
				gx_m2_ground_batch_cull(tile_ground_effects->shadow_batch, frame, ground_effects->shadow_doodads.data, ground_effects->shadow_doodads.size);
			}
		}
		if (g_wow->wow_opt & WOW_OPT_AABB_OPTIMIZE)
		{
			if (chunk->doodads_nb && tile->doodads_frustum_result != FRUSTUM_OUTSIDE)
			{
				if (tile->doodads_frustum_result == FRUSTUM_INSIDE || chunk->doodads_frustum_result != FRUSTUM_OUTSIDE)
				{
					PERFORMANCE_BEGIN(M2_CULL);
					bool bypass = tile->doodads_frustum_result == FRUSTUM_INSIDE || chunk->doodads_frustum_result == FRUSTUM_INSIDE;
					for (uint32_t doodad = 0; doodad < chunk->doodads_nb; ++doodad)
						gx_m2_instance_add_to_render(tile->m2[chunk->doodads[doodad]]->instance, frame, bypass, &frame->m2_params);
					PERFORMANCE_END(M2_CULL);
				}
			}
			if (chunk->wmos_nb && tile->wmos_frustum_result != FRUSTUM_OUTSIDE)
			{
				if (tile->wmos_frustum_result == FRUSTUM_INSIDE || chunk->wmos_frustum_result != FRUSTUM_OUTSIDE)
				{
					PERFORMANCE_BEGIN(WMO_CULL);
					bool bypass = tile->wmos_frustum_result == FRUSTUM_INSIDE || chunk->wmos_frustum_result == FRUSTUM_INSIDE;
					for (uint32_t wmo = 0; wmo < chunk->wmos_nb; ++wmo)
						gx_wmo_instance_add_to_render(tile->wmo[chunk->wmos[wmo]]->instance, frame, bypass);
					PERFORMANCE_END(WMO_CULL);
				}
			}
		}
		else
		{
			for (uint32_t doodad = 0; doodad < chunk->doodads_nb; ++doodad)
				gx_m2_instance_add_to_render(tile->m2[chunk->doodads[doodad]]->instance, frame, false, &frame->m2_params);
			for (uint32_t wmo = 0; wmo < chunk->wmos_nb; ++wmo)
				gx_wmo_instance_add_to_render(tile->wmo[chunk->wmos[wmo]]->instance, frame, false);
		}
	}
}

void map_tile_ground_end(struct map_tile *tile, struct gx_frame *frame)
{
	for (size_t i = 0; i < tile->ground_effects.size; ++i)
	{
		struct map_tile_ground_effect *ground_effect = JKS_ARRAY_GET(&tile->ground_effects, i, struct map_tile_ground_effect);
		gx_m2_ground_batch_cull_end(ground_effect->light_batch, frame);
		gx_m2_ground_batch_cull_end(ground_effect->shadow_batch, frame);
	}
}

void map_tile_ground_clear(struct map_tile *tile, struct gx_frame *frame)
{
	for (size_t i = 0; i < tile->ground_effects.size; ++i)
	{
		struct map_tile_ground_effect *ground_effect = JKS_ARRAY_GET(&tile->ground_effects, i, struct map_tile_ground_effect);
		gx_m2_ground_batch_cull_start(ground_effect->light_batch, frame);
		gx_m2_ground_batch_cull_start(ground_effect->shadow_batch, frame);
	}
}

struct map_wmo *map_wmo_new(uint32_t id)
{
	struct map_wmo *handle = mem_malloc(MEM_GX, sizeof(*handle));
	if (!handle)
		return NULL;
	handle->instance = NULL;
	handle->id = id;
	refcount_init(&handle->refcount, 1);
	return handle;
}

void map_wmo_free(struct map_wmo *handle)
{
	if (!handle)
		return;
	if (refcount_dec(&handle->refcount))
		return;
	cache_lock_map_wmo(g_wow->cache);
	if (refcount_get(&handle->refcount))
	{
		cache_unlock_map_wmo(g_wow->cache);
		return;
	}
	cache_unref_unmutexed_map_wmo(g_wow->cache, handle->id);
	cache_unlock_map_wmo(g_wow->cache);
	gx_wmo_instance_free(handle->instance);
	mem_free(MEM_GX, handle);
}

void map_wmo_ref(struct map_wmo *handle)
{
	refcount_inc(&handle->refcount);
}

void map_wmo_load(struct map_wmo *handle, const char *filename)
{
	if (handle->instance)
		return;
	handle->instance = gx_wmo_instance_new(filename);
}

struct map_m2 *map_m2_new(uint32_t id)
{
	struct map_m2 *handle = mem_malloc(MEM_GX, sizeof(*handle));
	if (!handle)
		return NULL;
	handle->instance = NULL;
	handle->id = id;
	refcount_init(&handle->refcount, 1);
	return handle;
}

void map_m2_free(struct map_m2 *handle)
{
	if (!handle)
		return;
	if (refcount_dec(&handle->refcount))
		return;
	cache_lock_map_m2(g_wow->cache);
	if (refcount_get(&handle->refcount))
	{
		cache_unlock_map_m2(g_wow->cache);
		return;
	}
	cache_unref_unmutexed_map_m2(g_wow->cache, handle->id);
	cache_unlock_map_m2(g_wow->cache);
	gx_m2_instance_free(handle->instance);
	mem_free(MEM_GX, handle);
}

void map_m2_ref(struct map_m2 *handle)
{
	refcount_inc(&handle->refcount);
}

void map_m2_load(struct map_m2 *handle, const char *filename)
{
	if (handle->instance)
		return;
	handle->instance = gx_m2_instance_new_filename(filename);
	gx_m2_instance_flag_set(handle->instance, GX_M2_INSTANCE_FLAG_DYN_SHADOW);
}

void map_chunk_get_interp_points(float px, float pz, uint8_t *points)
{
	float pxf = fmodf(px, 1.0);
	float pzf = fmodf(pz, 1.0);
	if (pxf < 0.5)
	{
		if (pzf < 0.5)
		{
			if (pxf > pzf) /* bottom */
			{
				points[0] = 0;
				points[1] = 4;
				points[2] = 1;
			}
			else /* left */
			{
				points[0] = 0;
				points[1] = 4;
				points[2] = 2;
			}
		}
		else
		{
			if (pxf > 1.0 - pzf) /* top */
			{
				points[0] = 2;
				points[1] = 4;
				points[2] = 3;
			}
			else /* left */
			{
				points[0] = 0;
				points[1] = 4;
				points[2] = 2;
			}
		}
	}
	else
	{
		if (pzf < 0.5)
		{
			if (1.0 - pxf > pzf) /* bottom */
			{
				points[0] = 0;
				points[1] = 4;
				points[2] = 1;
			}
			else /* right */
			{
				points[0] = 1;
				points[1] = 4;
				points[2] = 3;
			}
		}
		else
		{
			if (pxf > pzf) /* right */
			{
				points[0] = 1;
				points[1] = 4;
				points[2] = 3;
			}
			else /* top */
			{
				points[0] = 2;
				points[1] = 4;
				points[2] = 3;
			}
		}
	}
}

void map_chunk_get_interp_factors(float px, float pz, uint8_t *points, float *factors)
{
	float pxf = fmod(px, 1);
	float pzf = fmod(pz, 1);
	static const struct vec2f pos[5] =
	{
		{0.0, 0.0},
		{1.0, 0.0},
		{0.0, 1.0},
		{1.0, 1.0},
		{0.5, 0.5},
	};
	const struct vec2f *p1 = &pos[points[0]];
	const struct vec2f *p2 = &pos[points[1]];
	const struct vec2f *p3 = &pos[points[2]];
	float f0n = (p2->y - p3->y) * (pxf - p3->x) + (p3->x - p2->x) * (pzf - p3->y);
	float f0d = (p2->y - p3->y) * (p1->x - p3->x) + (p3->x - p2->x) * (p1->y - p3->y);
	float f1n = (p3->y - p1->y) * (pxf - p3->x) + (p1->x - p3->x) * (pzf - p3->y);
	float f1d = (p2->y - p3->y) * (p1->x - p3->x) + (p3->x - p2->x) * (p1->y - p3->y);
	factors[0] = f0n / f0d;
	factors[1] = f1n / f1d;
	factors[2] = 1.0 - factors[0] - factors[1];
}

void map_chunk_get_y(struct map_chunk *chunk, float px, float pz, uint8_t *points, float *factors, float *y)
{
	static const uint8_t pos[5] = {0, 1, 17, 18, 9};
	uint8_t base = (9 + 8) * (int)pz + (int)px;
	float y0 = chunk->height[base + pos[points[0]]];
	float y1 = chunk->height[base + pos[points[1]]];
	float y2 = chunk->height[base + pos[points[2]]];
	*y = y0 * factors[0] + y1 * factors[1] + y2 * factors[2];
}

void map_chunk_get_normal(struct map_chunk *chunk, float px, float pz, uint8_t *points, float *factors, struct vec3f *n)
{
	static const uint8_t pos[5] = {0, 1, 17, 18, 9};
	uint8_t base = (9 + 8) * (int)pz + (int)px;
	int8_t *n0 = &chunk->norm[3 * (base + pos[points[0]])];
	int8_t *n1 = &chunk->norm[3 * (base + pos[points[1]])];
	int8_t *n2 = &chunk->norm[3 * (base + pos[points[2]])];
	for (int i = 0; i < 3; ++i)
		((float*)n)[i] = n0[i] / 127.0 * factors[0] + n1[i] / 127.0 * factors[1] + n2[i] / 127.0 * factors[2];
}
