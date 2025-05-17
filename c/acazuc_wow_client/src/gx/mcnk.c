#include "gx/frame.h"
#include "gx/mcnk.h"
#include "gx/wmo.h"
#include "gx/blp.h"
#include "gx/m2.h"
#include "gx/gx.h"

#include "map/tile.h"
#include "map/map.h"

#include "ppe/render_target.h"

#include "shaders.h"
#include "camera.h"
#include "memory.h"
#include "cache.h"
#include "const.h"
#include "log.h"
#include "wow.h"
#include "dbc.h"

#include <jks/mat3.h>

#include <gfx/device.h>

#include <wow/wdt.h>
#include <wow/adt.h>
#include <wow/dbc.h>
#include <wow/mpq.h>

#include <string.h>
#include <stdio.h>
#include <math.h>

#if 0
# define USE_LOW_DEF_ALPHA
#endif

MEMORY_DECL(GX);

struct mcnk_init_data
{
	struct shader_mcnk_input *vertexes;
	uint32_t vertexes_nb;
	uint16_t *indices;
	uint32_t indices_nb;
	uint8_t *alpha[GX_MCNK_CHUNKS_PER_TILE];
	struct shader_mcnk_mesh_block mesh_blocks[GX_MCNK_CHUNKS_PER_TILE];
};

static void clean_init_data(struct mcnk_init_data *init_data)
{
	if (!init_data)
		return;
	mem_free(MEM_GX, init_data->vertexes);
	mem_free(MEM_GX, init_data->indices);
	for (uint32_t i = 0; i < GX_MCNK_CHUNKS_PER_TILE; ++i)
		mem_free(MEM_GX, init_data->alpha[i]);
	mem_free(MEM_GX, init_data);
}

static bool init_init_data(struct gx_mcnk *mcnk, struct wow_adt_file *file)
{
	mcnk->init_data->indices_nb = 0;
	for (size_t chunk = 0; chunk < GX_MCNK_CHUNKS_PER_TILE; ++chunk)
	{
		struct wow_mcnk *wow_mcnk = &file->mcnk[chunk];
		for (size_t i = 0; i < 16; ++i)
		{
			static const uint32_t lod_indices[16] =
			{
				12, 9, 9, 12,
				9 , 6, 6, 9 ,
				9 , 6, 6, 9 ,
				12, 9, 9, 12
			};
			if (!(wow_mcnk->header.holes & (1 << i)))
				mcnk->init_data->indices_nb += 18 * 4 + lod_indices[i];
		}
	}
	mcnk->init_data->indices = mem_malloc(MEM_GX, sizeof(*mcnk->init_data->indices) * mcnk->init_data->indices_nb);
	if (!mcnk->init_data->indices)
	{
		LOG_ERROR("allocation failed");
		return false;
	}
	mcnk->init_data->vertexes_nb = (9 * 9 + 8 * 8) * GX_MCNK_CHUNKS_PER_TILE;
	mcnk->init_data->vertexes = mem_malloc(MEM_GX, sizeof(*mcnk->init_data->vertexes) * mcnk->init_data->vertexes_nb);
	if (!mcnk->init_data->vertexes)
	{
		LOG_ERROR("allocation failed");
		return false;
	}
	return true;
}

static void init_indices(struct gx_mcnk *mcnk, struct gx_mcnk_chunk *gx_chunk, struct wow_mcnk *wow_mcnk, uint32_t chunk_id, uint32_t *indices_nb_ptr)
{
	struct mcnk_init_data *init_data = mcnk->init_data;
	if (!(mcnk->flags & GX_MCNK_FLAG_HOLES))
	{
		gx_chunk->indices_offsets[0] = (8 * 8 * 4 * 3 + 8 * 8 * 2 * 3 + 48 * 3) * chunk_id;
		gx_chunk->indices_offsets[1] = gx_chunk->indices_offsets[0] + 8 * 8 * 4 * 3;
		gx_chunk->indices_offsets[2] = gx_chunk->indices_offsets[1] + 8 * 8 * 2 * 3;
		gx_chunk->indices_nbs[0] = 8 * 8 * 4 * 3;
		gx_chunk->indices_nbs[1] = 8 * 8 * 2 * 3;
		gx_chunk->indices_nbs[2] = 48 * 3;
		return;
	}
	uint32_t indices_nb = *indices_nb_ptr;
	gx_chunk->indices_offsets[0] = indices_nb;
	uint16_t base = chunk_id * (9 * 9 + 8 * 8);
	for (size_t z = 0; z < 8; ++z)
	{
		for (size_t x = 0; x < 8; ++x)
		{
			if (wow_mcnk->header.holes & (1 << (z / 2 * 4 + x / 2)))
				continue;
			uint16_t idx = base + 9 + z * 17 + x;
			uint16_t p1 = idx - 9;
			uint16_t p2 = idx - 8;
			uint16_t p3 = idx + 9;
			uint16_t p4 = idx + 8;
			init_data->indices[indices_nb++] = p2;
			init_data->indices[indices_nb++] = p1;
			init_data->indices[indices_nb++] = idx;
			init_data->indices[indices_nb++] = p3;
			init_data->indices[indices_nb++] = p2;
			init_data->indices[indices_nb++] = idx;
			init_data->indices[indices_nb++] = p4;
			init_data->indices[indices_nb++] = p3;
			init_data->indices[indices_nb++] = idx;
			init_data->indices[indices_nb++] = p1;
			init_data->indices[indices_nb++] = p4;
			init_data->indices[indices_nb++] = idx;
		}
	}
	gx_chunk->indices_nbs[0] = indices_nb - gx_chunk->indices_offsets[0];
	gx_chunk->indices_offsets[1] = indices_nb;
	for (size_t z = 0; z < 8; ++z)
	{
		for (size_t x = 0; x < 8; ++x)
		{
			if (wow_mcnk->header.holes & (1 << (z / 2 * 4 + x / 2)))
				continue;
			uint16_t idx = base + 9 + z * 17 + x;
			uint16_t p1 = idx - 9;
			uint16_t p2 = idx - 8;
			uint16_t p3 = idx + 9;
			uint16_t p4 = idx + 8;
			init_data->indices[indices_nb++] = p2;
			init_data->indices[indices_nb++] = p1;
			init_data->indices[indices_nb++] = p3;
			init_data->indices[indices_nb++] = p3;
			init_data->indices[indices_nb++] = p1;
			init_data->indices[indices_nb++] = p4;
		}
	}
	gx_chunk->indices_nbs[1] = indices_nb - gx_chunk->indices_offsets[1];
	gx_chunk->indices_offsets[2] = indices_nb;
	for (size_t z = 0; z < 4; ++z)
	{
		for (size_t x = 0; x < 4; ++x)
		{
			if (wow_mcnk->header.holes & (1 << (z * 4 + x)))
				continue;
			static const uint16_t points[16][13] =
			{
				{12, 1  , 0  , 17 , 36 , 17 , 34 , 36 , 2  , 1  , 36 , 1  , 17},
				{ 9, 36 , 3  , 2  , 38 , 4  , 3  , 38 , 3  , 36},
				{ 9, 38 , 5  , 4  , 40 , 6  , 5  , 40 , 5  , 38},
				{12, 25 , 8  , 7  , 40 , 42 , 25 , 40 , 7  , 6  , 40 , 25 , 7},
				{ 9, 70 , 51 , 68 , 36 , 34 , 51 , 36 , 51 , 70},
				{ 6, 36 , 70 , 72 , 72 , 38 , 36},
				{ 6, 40 , 38 , 72 , 72 , 74 , 40},
				{ 9, 40 , 59 , 42 , 74 , 76 , 59 , 74 , 59 , 40},
				{ 9, 104, 85 , 102, 70 , 68 , 85 , 70 , 85 , 104},
				{ 6, 104, 106, 72 , 72 , 70 , 104},
				{ 6, 72 , 106, 108, 108, 74 , 72},
				{ 9, 74 , 93 , 76 , 108, 110, 93 , 108, 93 , 74},
				{12, 119, 136, 137, 104, 102, 119, 104, 137, 138, 104, 119, 137},
				{ 9, 106, 139, 140, 104, 138, 139, 104, 139, 106},
				{ 9, 108, 141, 142, 106, 140, 141, 106, 141, 108},
				{12, 143, 144, 127, 108, 142, 143, 108, 127, 110, 108, 143, 127},
			};
			for (size_t j = 1; j <= points[z * 4 + x][0]; ++j)
				init_data->indices[indices_nb++] = base + points[z * 4 + x][j];
		}
	}
	gx_chunk->indices_nbs[2] = indices_nb - gx_chunk->indices_offsets[2];
	*indices_nb_ptr = indices_nb;
}

static bool init_textures(struct gx_mcnk *mcnk, struct wow_adt_file *file, uint16_t *textures, uint32_t textures_nb)
{
	mcnk->textures_nb = textures_nb;
	mcnk->diffuse_textures = mem_malloc(MEM_GX, sizeof(*mcnk->diffuse_textures) * mcnk->textures_nb);
	if (!mcnk->diffuse_textures)
		return false;
	mcnk->specular_textures = mem_malloc(MEM_GX, sizeof(*mcnk->specular_textures) * mcnk->textures_nb);
	if (!mcnk->specular_textures)
	{
		mem_free(MEM_GX, mcnk->diffuse_textures);
		return false;
	}
	for (uint32_t i = 0; i < textures_nb; ++i)
	{
		uint32_t texture = textures[i];
		if (texture >= file->textures_nb)
		{
			mcnk->diffuse_textures[i] = NULL;
			mcnk->specular_textures[i] = NULL;
			LOG_ERROR("invalid texture: %u / %u", texture, file->textures_nb);
			continue;
		}
		char texture_name[512];
		snprintf(texture_name, sizeof(texture_name), "%s", file->textures[texture]);
		wow_mpq_normalize_blp_fn(texture_name, sizeof(texture_name));
		if (cache_ref_blp(g_wow->cache, texture_name, &mcnk->diffuse_textures[i]))
		{
			gx_blp_ask_load(mcnk->diffuse_textures[i]);
		}
		else
		{
			LOG_WARN("can't get texture: %s", texture_name);
			mcnk->diffuse_textures[i] = NULL;
		}
		size_t len = strlen(texture_name);
		snprintf(texture_name + len - 4, sizeof(texture_name) - len + 4, "_s.blp");
		wow_mpq_normalize_blp_fn(texture_name, sizeof(texture_name));
		if (cache_ref_blp(g_wow->cache, texture_name, &mcnk->specular_textures[i]))
		{
			gx_blp_ask_load(mcnk->specular_textures[i]);
		}
		else
		{
			LOG_WARN("can't get texture: %s", texture_name);
			mcnk->specular_textures[i] = NULL;
		}
	}
	return true;
}

static void init_holes(struct gx_mcnk *mcnk, struct wow_adt_file *file)
{
	for (size_t chunk = 0; chunk < GX_MCNK_CHUNKS_PER_TILE; ++chunk)
	{
		struct wow_mcnk *wow_mcnk = &file->mcnk[chunk];
		if (!wow_mcnk->header.holes)
			continue;
		mcnk->flags |= GX_MCNK_FLAG_HOLES;
		break;
	}
}

static void init_chunk_vertices(struct gx_mcnk *mcnk, struct map_chunk *chunk, uint32_t chunk_id)
{
	struct shader_mcnk_input *iter = mcnk->init_data->vertexes + (9 * 9 + 8 * 8) * chunk_id;
	for (size_t i = 0; i < 9 * 9 + 8 * 8; ++i)
	{
		float y = chunk->height[i];
		(iter++)->y = y;
	}
}

static void init_chunk_normals(struct gx_mcnk *mcnk, struct map_chunk *chunk, uint32_t chunk_id)
{
	struct shader_mcnk_input *iter = mcnk->init_data->vertexes + (9 * 9 + 8 * 8) * chunk_id;
	size_t j = 0;
	for (size_t i = 0; i < 9 * 9 + 8 * 8; ++i)
	{
		iter->norm.x = chunk->norm[j + 0];
		iter->norm.y = chunk->norm[j + 1];
		iter->norm.z = chunk->norm[j + 2];
		iter++;
		j += 3;
	}
}

static void init_chunk_animations(struct wow_mcnk *mcnk, struct vec2f *uv_offsets)
{
	for (size_t l = 0; l < mcnk->header.layers; ++l)
	{
		struct wow_mcly_data *mcly = &mcnk->mcly.data[l];
		if (!mcly->flags.animation_enabled)
		{
			uv_offsets[l] = (struct vec2f){0, 0};
			continue;
		}
		static const float speeds[] = {1, 2, 4, 8, 16, 32, 48, 64};
		uv_offsets[l].x = speeds[mcly->flags.animation_speed] * 0.176776695; /* sqrt(2) / 8 */
		uv_offsets[l].y = uv_offsets[l].x;
		struct mat3f mat;
		struct mat3f tmp;
		MAT3_IDENTITY(tmp);
		MAT3_ROTATEZ(float, mat, tmp, M_PI * .25 + mcly->flags.animation_rotation * M_PI * .25);
		struct vec3f tmp_animation = {uv_offsets[l].x, uv_offsets[l].y, 0};
		struct vec3f res;
		MAT3_VEC3_MUL(res, mat, tmp_animation);
		uv_offsets[l] = (struct vec2f){res.x, res.y};
	}
}

#ifdef USE_LOW_DEF_ALPHA

static void init_chunk_alpha_low(struct gx_mcnk *mcnk, struct wow_mcnk *wow_mcnk, uint32_t chunk)
{
	uint8_t *iter = mcnk->init_data->alpha[chunk];
	if (g_wow->map->wow_flags & WOW_MPHD_FLAG_BIG_ALPHA)
	{
		for (size_t z = 0; z < 64; ++z)
		{
			for (size_t x = 0; x < 64; ++x)
			{
				uint8_t val = ((wow_mcnk->header.low_quality_texture[z / 8] >> (x / 8 * 2)) & 0x3);
				switch (val)
				{
					case 0:
						iter += 4;
						break;
					case 1:
						iter += 3;
						*iter = 0xFF;
						iter += 1;
						break;
					case 2:
						iter += 2;
						*iter = 0xFF;
						iter += 2;
						break;
					case 3:
						iter += 1;
						*iter = 0xFF;
						iter += 3;
						break;
				}
			}
		}
	}
	else
	{
		for (size_t z = 0; z < 64; ++z)
		{
			for (size_t x = 0; x < 64; ++x)
			{
				uint8_t val = ((wow_mcnk->header.low_quality_texture[z / 8] >> (x / 8 * 2)) & 0x3);
				switch (val)
				{
					case 0:
						iter += 2;
						break;
					case 1:
						++iter;
						*iter = 0x0F;
						++iter;
						break;
					case 2:
						*iter = 0xF0;
						iter += 2;
						break;
					case 3:
						*iter = 0x0F;
						iter += 2;
						break;
				}
			}
		}
	}
}

#else

static void init_layer_big_alpha(struct gx_mcnk *mcnk, struct wow_mcnk *wow_mcnk, uint32_t chunk, uint32_t l, uint8_t *mcal_iter, uint8_t *tmp_mcal)
{
	uint8_t *large_data;
	if (wow_mcnk->mcly.data[l].flags.alpha_map_compressed)
	{
		large_data = tmp_mcal;
		for (size_t pos = 0; pos < 4096;)
		{
			uint8_t meta = *(mcal_iter++);
			if (!meta)
			{
				LOG_DEBUG("invalid meta");
				break;
			}
			if (meta & 0x80)
			{
				uint8_t data = *(mcal_iter++);
				for (size_t k = 0; k < (meta & 0x7F) && pos < 4096; ++k)
					tmp_mcal[pos++] = data;
			}
			else
			{
				for (size_t k = 0; k < (meta & 0x7F) && pos < 4096; ++k)
					tmp_mcal[pos++] = *(mcal_iter++);
			}
		}
	}
	else
	{
		large_data = mcal_iter;
	}
	uint8_t *iter = mcnk->init_data->alpha[chunk] + 3 - l;
	for (size_t z = 0; z < 64; ++z)
	{
		for (size_t x = 0; x < 64; ++x)
		{
			size_t tx;
			size_t tz;
			if (!(wow_mcnk->header.flags & WOW_MCNK_FLAGS_FIX_MCAL))
			{
				tx = x == 63 ? 62 : x;
				tz = z == 63 ? 62 : z;
			}
			else
			{
				tx = x;
				tz = z;
			}
			*iter = large_data[tz * 64 + tx];
			iter += 4;
		}
	}
}

static void init_layer_std_alpha(struct gx_mcnk *mcnk, struct wow_mcnk *wow_mcnk, uint32_t chunk, uint32_t l, uint8_t *mcal_iter)
{
	uint8_t *iter = mcnk->init_data->alpha[chunk];
	l = 4 - l;
	uint8_t shifter = (l % 2) * 4;
	if (l > 1)
		++iter;
	for (size_t z = 0; z < 64; ++z)
	{
		for (size_t x = 0; x < 64; ++x)
		{
			size_t tx;
			size_t tz;
			if (!(wow_mcnk->header.flags & WOW_MCNK_FLAGS_FIX_MCAL))
			{
				tx = x == 63 ? 62 : x;
				tz = z == 63 ? 62 : z;
			}
			else
			{
				tx = x;
				tz = z;
			}
			*iter |= ((mcal_iter[tz * 32 + tx / 2] >> ((tx & 1) << 2)) & 0xF) << shifter;
			iter += 2;
		}
	}
}

static void init_chunk_alpha(struct gx_mcnk *mcnk, struct wow_mcnk *wow_mcnk, uint32_t chunk, uint8_t *tmp_mcal)
{
	for (uint32_t l = 1; l < wow_mcnk->header.layers; ++l)
	{
		if (!wow_mcnk->mcly.data[l].flags.use_alpha_map)
			continue;
		uint8_t *mcal_iter = wow_mcnk->mcal.data + wow_mcnk->mcly.data[l].offset_in_mcal;
		if (g_wow->map->wow_flags & WOW_MPHD_FLAG_BIG_ALPHA)
		{
			init_layer_big_alpha(mcnk, wow_mcnk, chunk, l, mcal_iter, tmp_mcal);
		}
		else
		{
			init_layer_std_alpha(mcnk, wow_mcnk, chunk, l, mcal_iter);
		}
	}
}

#endif

static void init_chunk_shadow(struct gx_mcnk *mcnk, struct wow_mcnk *wow_mcnk, uint32_t chunk)
{
	if (wow_mcnk->header.flags & WOW_MCNK_FLAGS_MCSH)
	{
		uint8_t *iter = mcnk->init_data->alpha[chunk];
		if (g_wow->map->wow_flags & WOW_MPHD_FLAG_BIG_ALPHA)
		{
			iter += 3;
			for (size_t z = 0; z < 64; ++z)
			{
				for (size_t x = 0; x < 64; ++x)
				{
					*iter = ((wow_mcnk->mcsh.data[z][x / 8] >> (x % 8)) & 1) ? 0 : 0xFF;
					iter += 4;
				}
			}
		}
		else
		{
			for (size_t z = 0; z < 64; ++z)
			{
				for (size_t x = 0; x < 64; ++x)
				{
					*iter |= ((wow_mcnk->mcsh.data[z][x / 8] >> (x % 8)) & 1) ? 0 : 0xF;
					iter += 2;
				}
			}
		}
	}
	else
	{
		uint8_t *iter = mcnk->init_data->alpha[chunk];
		if (g_wow->map->wow_flags & WOW_MPHD_FLAG_BIG_ALPHA)
		{
			iter += 3;
			for (size_t z = 0; z < 64; ++z)
			{
				for (size_t x = 0; x < 64; ++x)
				{
					*iter = 0xFF;
					iter += 4;
				}
			}
		}
		else
		{
			for (size_t z = 0; z < 64; ++z)
			{
				for (size_t x = 0; x < 64; ++x)
				{
					*iter |= 0xF;
					iter += 2;
				}
			}
		}
	}
}

static bool batch_can_accept_chunk(struct gx_mcnk_batch *batch, struct wow_mcnk *mcnk)
{
	uint32_t missing = 0;
	for (size_t i = 0; i < mcnk->header.layers; ++i)
	{
		uint32_t texture_id = mcnk->mcly.data[i].texture_id;
		bool found = false;
		for (size_t j = 0; j < batch->textures_nb; ++j)
		{
			if (batch->textures[j] == texture_id)
			{
				found = true;
				break;
			}
		}
		if (!found)
			missing++;
	}
	return batch->textures_nb + missing <= sizeof(batch->textures) / sizeof(*batch->textures);
}

static void batch_add_chunk(struct gx_mcnk_batch *batch, struct wow_mcnk *mcnk, uint8_t chunk_id, struct vec4u *batch_textures)
{
	for (size_t i = 0; i < mcnk->header.layers; ++i)
	{
		uint32_t texture_id = mcnk->mcly.data[i].texture_id;
		bool found = false;
		for (size_t j = 0; j < batch->textures_nb; ++j)
		{
			if (batch->textures[j] == texture_id)
			{
				(&batch_textures[0].x)[i] = j * 2;
				(&batch_textures[1].x)[i] = j * 2 + 1;
				found = true;
				break;
			}
		}
		if (!found)
		{
			(&batch_textures[0].x)[i] = batch->textures_nb * 2;
			(&batch_textures[1].x)[i] = batch->textures_nb * 2 + 1;
			batch->textures[batch->textures_nb++] = texture_id;
		}
	}
	batch->chunks[batch->chunks_nb++] = chunk_id;
}

struct gx_mcnk *gx_mcnk_new(struct map_tile *tile, struct wow_adt_file *file)
{
	uint8_t tmp_mcal[4096];
	uint16_t textures[16 * 16 * 4];
	size_t textures_nb = 0;
	uint32_t indices_nb = 0;
	size_t alpha_bpp;
	struct mat4f m;
	MAT4_IDENTITY(m);
	struct gx_mcnk *mcnk = mem_zalloc(MEM_GX, sizeof(*mcnk));
	if (!mcnk)
		return NULL;
	jks_array_init(&mcnk->batches, sizeof(struct gx_mcnk_batch), NULL, &jks_array_memory_fn_GX);
	mcnk->parent = tile;
	mcnk->init_data = mem_zalloc(MEM_GX, sizeof(*mcnk->init_data));
	if (!mcnk->init_data)
		goto err;
	MAT4_TRANSLATE(mcnk->m, m, tile->pos);
	init_holes(mcnk, file);
	if (!init_init_data(mcnk, file))
		goto err;
	alpha_bpp = (g_wow->map->wow_flags & WOW_MPHD_FLAG_BIG_ALPHA) ? 4 : 2;
	for (size_t chunk = 0; chunk < GX_MCNK_CHUNKS_PER_TILE; ++chunk)
	{
		struct gx_mcnk_batch *batch = NULL;
		for (size_t i = 0; i < mcnk->batches.size; ++i)
		{
			struct gx_mcnk_batch *tmp = JKS_ARRAY_GET(&mcnk->batches, i, struct gx_mcnk_batch);
			if (batch_can_accept_chunk(tmp, &file->mcnk[chunk]))
			{
				batch = tmp;
				break;
			}
		}
		if (!batch)
		{
			batch = jks_array_grow(&mcnk->batches, 1);
			if (!batch)
			{
				LOG_ERROR("batches allocation failed");
				goto err;
			}
			memset(batch, 0, sizeof(*batch));
		}
		batch_add_chunk(batch, &file->mcnk[chunk], chunk, mcnk->init_data->mesh_blocks[chunk].textures);
	}
	for (size_t i = 0; i < mcnk->batches.size; ++i)
	{
		struct gx_mcnk_batch *batch = JKS_ARRAY_GET(&mcnk->batches, i, struct gx_mcnk_batch);
		for (size_t j = 0; j < batch->textures_nb; ++j)
		{
			bool found = false;
			for (size_t k = 0; k < textures_nb; ++k)
			{
				if (textures[k] == batch->textures[j])
				{
					found = true;
					break;
				}
			}
			if (found)
				continue;
			textures[textures_nb++] = batch->textures[j];
		}
	}
	for (size_t i = 0; i < mcnk->batches.size; ++i)
	{
		struct gx_mcnk_batch *batch = JKS_ARRAY_GET(&mcnk->batches, i, struct gx_mcnk_batch);
		for (size_t j = 0; j < batch->textures_nb; ++j)
		{
			for (size_t k = 0; k < textures_nb; ++k)
			{
				if (textures[k] == batch->textures[j])
				{
					batch->textures[j] = k;
					break;
				}
			}
		}
	}
	if (!init_textures(mcnk, file, textures, textures_nb))
		goto err;
	for (size_t chunk = 0; chunk < GX_MCNK_CHUNKS_PER_TILE; ++chunk)
	{
		struct wow_mcnk *wow_mcnk = &file->mcnk[chunk];
		struct gx_mcnk_chunk *gx_chunk = &mcnk->chunks[chunk];
		init_indices(mcnk, gx_chunk, wow_mcnk, chunk, &indices_nb);
		init_chunk_animations(wow_mcnk, mcnk->init_data->mesh_blocks[chunk].uv_offsets);
		mcnk->init_data->alpha[chunk] = mem_malloc(MEM_GX, 64 * 64 * alpha_bpp);
		if (!mcnk->init_data->alpha[chunk])
			goto err;
		memset(mcnk->init_data->alpha[chunk], 0, 64 * 64 * alpha_bpp);
		/* alpha */
		{
#ifdef USE_LOW_DEF_ALPHA
			init_chunk_alpha_low(mcnk, wow_mcnk, chunk);
#else
			init_chunk_alpha(mcnk, wow_mcnk, chunk, tmp_mcal);
#endif
			init_chunk_shadow(mcnk, wow_mcnk, chunk);
		}
		init_chunk_normals(mcnk, &tile->chunks[chunk], chunk);
		init_chunk_vertices(mcnk, &tile->chunks[chunk], chunk);
	}
	mcnk->alpha_texture = GFX_TEXTURE_INIT();
	mcnk->attributes_state = GFX_ATTRIBUTES_STATE_INIT();
	mcnk->chunks_uniform_buffer = GFX_BUFFER_INIT();
	for (size_t i = 0; i < RENDER_FRAMES_COUNT; ++i)
	{
		struct gx_mcnk_frame *mcnk_frame = &mcnk->frames[i];
		mcnk_frame->indirect_buffer = GFX_BUFFER_INIT();
		mcnk_frame->uniform_buffer = GFX_BUFFER_INIT();
	}
	mcnk->vertexes_buffer = GFX_BUFFER_INIT();
	mcnk->indices_buffer = GFX_BUFFER_INIT();
	return mcnk;

err:
	clean_init_data(mcnk->init_data);
	jks_array_destroy(&mcnk->batches);
	mem_free(MEM_GX, mcnk);
	return NULL;
}

void gx_mcnk_delete(struct gx_mcnk *mcnk)
{
	if (!mcnk)
		return;
	for (uint32_t i = 0; i < mcnk->textures_nb; ++i)
	{
		gx_blp_free(mcnk->specular_textures[i]);
		gx_blp_free(mcnk->diffuse_textures[i]);
	}
	mem_free(MEM_GX, mcnk->diffuse_textures);
	mem_free(MEM_GX, mcnk->specular_textures);
	gfx_delete_texture(g_wow->device, &mcnk->alpha_texture);
	gfx_delete_buffer(g_wow->device, &mcnk->chunks_uniform_buffer);
	for (size_t i = 0; i < RENDER_FRAMES_COUNT; ++i)
	{
		struct gx_mcnk_frame *mcnk_frame = &mcnk->frames[i];
		gfx_delete_buffer(g_wow->device, &mcnk_frame->indirect_buffer);
		gfx_delete_buffer(g_wow->device, &mcnk_frame->uniform_buffer);
	}
	gfx_delete_buffer(g_wow->device, &mcnk->vertexes_buffer);
	gfx_delete_buffer(g_wow->device, &mcnk->indices_buffer);
	gfx_delete_attributes_state(g_wow->device, &mcnk->attributes_state);
	clean_init_data(mcnk->init_data);
	jks_array_destroy(&mcnk->batches);
	mem_free(MEM_GX, mcnk);
}

int gx_mcnk_initialize(struct gx_mcnk *mcnk)
{
	if (mcnk->flags & GX_MCNK_FLAG_INITIALIZED)
		return 1;
	if (!mcnk->alpha_texture.handle.u64)
	{
		gfx_create_texture(g_wow->device, &mcnk->alpha_texture, GFX_TEXTURE_2D_ARRAY, (g_wow->map->wow_flags & WOW_MPHD_FLAG_BIG_ALPHA) ? GFX_R8G8B8A8 : GFX_R4G4B4A4, 1, 64, 64, GX_MCNK_CHUNKS_PER_TILE);
		gfx_set_texture_filtering(&mcnk->alpha_texture, GFX_FILTERING_LINEAR, GFX_FILTERING_LINEAR, GFX_FILTERING_NONE);
		gfx_set_texture_anisotropy(&mcnk->alpha_texture, 16);
		gfx_set_texture_addressing(&mcnk->alpha_texture, GFX_TEXTURE_ADDRESSING_CLAMP, GFX_TEXTURE_ADDRESSING_CLAMP, GFX_TEXTURE_ADDRESSING_CLAMP);
		gfx_set_texture_levels(&mcnk->alpha_texture, 0, 0);
	}
	for (uint32_t i = 0; i < GX_MCNK_CHUNKS_PER_TILE; ++i)
	{
		if (!mcnk->init_data->alpha[i])
			continue;
		size_t alpha_bpp = (g_wow->map->wow_flags & WOW_MPHD_FLAG_BIG_ALPHA) ? 4 : 2;
		gfx_set_texture_data(&mcnk->alpha_texture, 0, i, 64, 64, 1, 64 * 64 * alpha_bpp, mcnk->init_data->alpha[i]);
		mem_free(MEM_GX, mcnk->init_data->alpha[i]);
		mcnk->init_data->alpha[i] = NULL;
	}
	gfx_finalize_texture(&mcnk->alpha_texture);
	gfx_create_buffer(g_wow->device, &mcnk->vertexes_buffer, GFX_BUFFER_VERTEXES, mcnk->init_data->vertexes, mcnk->init_data->vertexes_nb * sizeof(*mcnk->init_data->vertexes), GFX_BUFFER_IMMUTABLE);
	if (mcnk->flags & GX_MCNK_FLAG_HOLES)
		gfx_create_buffer(g_wow->device, &mcnk->indices_buffer, GFX_BUFFER_INDICES, mcnk->init_data->indices, mcnk->init_data->indices_nb * sizeof(*mcnk->init_data->indices), GFX_BUFFER_IMMUTABLE);
	for (size_t i = 0; i < RENDER_FRAMES_COUNT; ++i)
	{
		struct gx_mcnk_frame *mcnk_frame = &mcnk->frames[i];
		gfx_create_buffer(g_wow->device, &mcnk_frame->indirect_buffer, GFX_BUFFER_INDIRECT, NULL, sizeof(struct gfx_draw_indexed_indirect_cmd) * GX_MCNK_CHUNKS_PER_TILE, GFX_BUFFER_STREAM);
		gfx_create_buffer(g_wow->device, &mcnk_frame->uniform_buffer, GFX_BUFFER_UNIFORM, NULL, sizeof(struct shader_mcnk_model_block), GFX_BUFFER_STREAM);
	}
	gfx_create_buffer(g_wow->device, &mcnk->chunks_uniform_buffer, GFX_BUFFER_UNIFORM, mcnk->init_data->mesh_blocks, sizeof(mcnk->init_data->mesh_blocks), GFX_BUFFER_IMMUTABLE);
	clean_init_data(mcnk->init_data);
	mcnk->init_data = NULL;
	const struct gfx_attribute_bind binds[] =
	{
		{&mcnk->vertexes_buffer},
		{&g_wow->gx->mcnk_vertexes_buffer},
	};
	gfx_create_attributes_state(g_wow->device, &mcnk->attributes_state, binds, sizeof(binds) / sizeof(*binds), (mcnk->flags & GX_MCNK_FLAG_HOLES) ? &mcnk->indices_buffer : &g_wow->gx->mcnk_indices_buffer, GFX_INDEX_UINT16);
	mcnk->flags |= GX_MCNK_FLAG_INITIALIZED;
	return 1;
}

void gx_mcnk_cull(struct gx_mcnk *mcnk, struct gx_frame *frame)
{
	if (!(mcnk->flags & GX_MCNK_FLAG_INITIALIZED))
		return;
	struct gx_mcnk_frame *mcnk_frame = &mcnk->frames[frame->id];
	struct vec3f delta;
	VEC3_SUB(delta, mcnk->parent->center, frame->cull_pos);
	mcnk->distance_to_camera = sqrtf(delta.x * delta.x + delta.z * delta.z);
	if (mcnk->distance_to_camera > frame->view_distance * 1.4142) /* sqrt(2) is the worst case of frustum plane */
		return;
	enum frustum_result result = frustum_check(&frame->frustum, &mcnk->parent->aabb);
	MAT4_TRANSLATE(mcnk_frame->mv, frame->view_v, mcnk->parent->pos);
	MAT4_MUL(mcnk_frame->mvp, frame->view_p, mcnk_frame->mv);
	MAT4_TRANSLATE(mcnk_frame->shadow_mv, frame->view_shadow_v, mcnk->parent->pos);
	MAT4_MUL(mcnk_frame->shadow_mvp, frame->view_shadow_p, mcnk_frame->shadow_mv);
	bool has_unculled_chunk = false;
	for (size_t i = 0; i < GX_MCNK_CHUNKS_PER_TILE; ++i)
	{
		struct gx_mcnk_chunk *gx_chunk = &mcnk->chunks[i];
		struct map_chunk *chunk = &mcnk->parent->chunks[i];
		struct gx_mcnk_chunk_frame *chunk_frame = &gx_chunk->frames[frame->id];
		VEC3_SUB(delta, chunk->center, frame->cull_pos);
		chunk_frame->distance_to_camera = sqrtf(delta.x * delta.x + delta.z * delta.z);
		if (chunk_frame->distance_to_camera > frame->view_distance * 1.4142)
		{
			chunk_frame->culled = true;
			continue;
		}
		switch (result)
		{
			case FRUSTUM_INSIDE:
				gx_chunk->frustum_result = FRUSTUM_INSIDE;
				chunk_frame->culled = false;
				has_unculled_chunk = true;
				break;
			case FRUSTUM_OUTSIDE:
				chunk_frame->culled = true;
				break;
			case FRUSTUM_COLLIDE:
				gx_chunk->frustum_result = frustum_check(&frame->frustum, &chunk->aabb);
				if (!(chunk_frame->culled = !gx_chunk->frustum_result))
					has_unculled_chunk = true;
				break;
		}
	}
	if (!has_unculled_chunk)
		return;
	struct gfx_draw_indexed_indirect_cmd *draw_cmds = mcnk_frame->draw_cmds;
	size_t *draw_cmd_nb = &mcnk_frame->draw_cmd_nb;
	uint16_t *batches_draw_cmds = mcnk_frame->batches_draw_cmds;
	*draw_cmd_nb = 0;
	for (size_t i = 0; i < mcnk->batches.size; ++i)
	{
		struct gx_mcnk_batch *batch = JKS_ARRAY_GET(&mcnk->batches, i, struct gx_mcnk_batch);
		batches_draw_cmds[i] = 0;
		for (size_t j = 0; j < batch->chunks_nb; ++j)
		{
			uint8_t chunk_id = batch->chunks[j];
			struct gx_mcnk_chunk *gx_chunk = &mcnk->chunks[chunk_id];
			struct gx_mcnk_chunk_frame *chunk_frame = &gx_chunk->frames[frame->id];
			uint32_t indices_buffer;
			if (chunk_frame->culled)
				continue;
			if (chunk_frame->distance_to_camera < CHUNK_WIDTH * 16)
				indices_buffer = 0;
			else if (chunk_frame->distance_to_camera < CHUNK_WIDTH * 32)
				indices_buffer = 1;
			else
				indices_buffer = 2;
			if (!gx_chunk->indices_nbs[indices_buffer])
				continue;
			draw_cmds[*draw_cmd_nb].index_count = gx_chunk->indices_nbs[indices_buffer];
			draw_cmds[*draw_cmd_nb].instance_count = 1;
			draw_cmds[*draw_cmd_nb].base_index = gx_chunk->indices_offsets[indices_buffer];
			draw_cmds[*draw_cmd_nb].base_vertex = 0;
			draw_cmds[*draw_cmd_nb].base_instance = 0;
			(*draw_cmd_nb)++;
			batches_draw_cmds[i]++;
		}
	}
	if (gx_mcnk_flag_set(mcnk, GX_MCNK_FLAG_IN_RENDER_LIST))
		return;
	gx_frame_add_mcnk(frame, mcnk);
}

void gx_mcnk_render(struct gx_mcnk *mcnk, struct gx_frame *frame)
{
	if (!(mcnk->flags & GX_MCNK_FLAG_INITIALIZED))
		return;
	struct gx_mcnk_frame *mcnk_frame = &mcnk->frames[frame->id];
	if (!mcnk_frame->draw_cmd_nb)
		return;
	{
		struct shader_mcnk_model_block model_block;
		model_block.v = *(struct mat4f*)&frame->view_v;
		model_block.mv = mcnk_frame->mv;
		model_block.mvp = mcnk_frame->mvp;
		model_block.shadow_mvp = mcnk_frame->shadow_mvp;
		model_block.offset_time = frame->time / 333000000000.0;
		gfx_set_buffer_data(&mcnk_frame->uniform_buffer, &model_block, sizeof(model_block), 0);
		gfx_set_buffer_data(&mcnk_frame->indirect_buffer, mcnk_frame->draw_cmds, sizeof(*mcnk_frame->draw_cmds) * mcnk_frame->draw_cmd_nb, 0);
	}
	{
		gfx_bind_constant(g_wow->device, 0, &mcnk->chunks_uniform_buffer, sizeof(struct shader_mcnk_mesh_block) * GX_MCNK_CHUNKS_PER_TILE, 0);
		gfx_bind_constant(g_wow->device, 1, &mcnk_frame->uniform_buffer, sizeof(struct shader_mcnk_model_block), 0);
		gfx_bind_attributes_state(g_wow->device, &mcnk->attributes_state, &g_wow->gx->mcnk_input_layout);
	}
	uint32_t draw_cmd_offset = 0;
	const gfx_texture_t *textures[2 + GX_MCNK_MAX_BATCH_TEXTURES * 2] = {0};
	textures[0] = &mcnk->alpha_texture;
	if (g_wow->gx->opt & GX_OPT_DYN_SHADOW)
		textures[1] = &g_wow->post_process.shadow->depth_stencil_texture;
	else
		textures[1] = NULL;
	for (size_t i = 0; i < mcnk->batches.size; ++i)
	{
		struct gx_mcnk_batch *batch = JKS_ARRAY_GET(&mcnk->batches, i, struct gx_mcnk_batch);
		if (!mcnk_frame->batches_draw_cmds[i])
			continue;
		size_t n = 2;
		for (size_t j = 0; j < batch->textures_nb; ++j)
		{
			uint16_t texture_index = batch->textures[j];
			if (gx_blp_flag_get(mcnk->diffuse_textures[texture_index], GX_BLP_FLAG_INITIALIZED))
				textures[n++] = &mcnk->diffuse_textures[texture_index]->texture;
			else
				textures[n++] = &g_wow->grey_texture->texture;
			if (gx_blp_flag_get(mcnk->specular_textures[texture_index], GX_BLP_FLAG_INITIALIZED))
				textures[n++] = &mcnk->specular_textures[texture_index]->texture;
			else
				textures[n++] = &g_wow->black_texture->texture;
		}
		while (n < sizeof(textures) / sizeof(*textures))
		{
			textures[n++] = &g_wow->grey_texture->texture;
			textures[n++] = &g_wow->black_texture->texture;
		}
		gfx_bind_samplers(g_wow->device, 0, sizeof(textures) / sizeof(*textures), textures);
		gfx_draw_indexed_indirect(g_wow->device, &mcnk_frame->indirect_buffer, mcnk_frame->batches_draw_cmds[i], draw_cmd_offset);
		draw_cmd_offset += mcnk_frame->batches_draw_cmds[i] * sizeof(*mcnk_frame->draw_cmds);
	}
}
