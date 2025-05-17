#include "gx/skybox.h"
#include "gx/frame.h"
#include "gx/wdl.h"
#include "gx/gx.h"

#include "map/map.h"

#include "shaders.h"
#include "camera.h"
#include "memory.h"
#include "const.h"
#include "log.h"
#include "wow.h"

#include <gfx/device.h>

#include <wow/wdl.h>

#include <sys/types.h>

#include <string.h>
#include <math.h>

struct wdl_init_data
{
	uint32_t *indices;
	uint32_t indices_nb;
	struct vec3f *vertexes;
	uint32_t vertexes_nb;
};

static void clean_init_data(struct wdl_init_data *init_data)
{
	if (!init_data)
		return;
	mem_free(MEM_GX, init_data->indices);
	mem_free(MEM_GX, init_data->vertexes);
	mem_free(MEM_GX, init_data);
}

struct gx_wdl *gx_wdl_new(struct wow_wdl_file *file)
{
	struct gx_wdl *wdl = mem_malloc(MEM_GX, sizeof(*wdl));
	if (!wdl)
		return NULL;
	wdl->initialized = false;
	wdl->init_data = mem_zalloc(MEM_GX, sizeof(*wdl->init_data));
	if (!wdl->init_data)
		goto err;
	wdl->init_data->vertexes_nb = (17 * 17 + 16 * 16) * (64 * 64);
	wdl->init_data->vertexes = mem_malloc(MEM_GX, sizeof(*wdl->init_data->vertexes) * wdl->init_data->vertexes_nb);
	if (!wdl->init_data->vertexes)
		goto err;
	wdl->init_data->indices_nb = (16 * 16 * 12) * 64 * 64;
	wdl->init_data->indices = mem_malloc(MEM_GX, sizeof(*wdl->init_data->indices) * wdl->init_data->indices_nb);
	if (!wdl->init_data->indices)
		goto err;
	for (uint32_t i = 0; i < sizeof(wdl->chunks) / sizeof(*wdl->chunks); ++i)
	{
#ifdef WITH_DEBUG_RENDERING
		gx_aabb_init(&wdl->chunks[i].gx_aabb, (struct vec4f){0.5, 0, 0, 1}, 2);
#endif
	}
	for (size_t i = 0; i < RENDER_FRAMES_COUNT; ++i)
	{
		struct gx_wdl_frame *wdl_frame = &wdl->frames[i];
		memset(&wdl_frame->culled, 0xFF, sizeof(wdl_frame->culled));
		wdl_frame->uniform_buffer = GFX_BUFFER_INIT();
	}
	{
		uint32_t indices_pos = 0;
		for (size_t cz = 0; cz < 64; ++cz)
		{
			for (size_t cx = 0; cx < 64; ++cx)
			{
				uint32_t chunk_idx = cz * 64 + cx;
				struct wdl_chunk *chunk = &wdl->chunks[chunk_idx];
				if (!(g_wow->map->adt_exists[cz] & (1ull << cx)))
					continue;
				struct vec3f chunk_min;
				struct vec3f chunk_max;
				float baseX = -(1 + (ssize_t)cz - 32) * CHUNK_WIDTH * 16;
				float baseZ =  (1 + (ssize_t)cx - 32) * CHUNK_WIDTH * 16;
				size_t base = chunk_idx * (16 * 16 + 17 * 17);
				chunk->indices_offset = indices_pos;
				for (size_t i = 0; i < 17 * 17 + 16 * 16; ++i)
				{
					size_t y2 = i % 33;
					float z = i / 33 * 2;
					float x;
					if (y2 < 17)
					{
						x = y2 * 2;
					}
					else
					{
						z++;
						x = (y2 - 17) * 2 + 1;
					}
					wdl->init_data->vertexes[base + i].x = baseX + (32 - z) / 32.0 * CHUNK_WIDTH * 16;
					wdl->init_data->vertexes[base + i].z = baseZ - (32 - x) / 32.0 * CHUNK_WIDTH * 16;
				}
				{
					size_t i = 0;
					for (size_t zz = 0; zz < 17; ++zz)
					{
						for (size_t xx = 0; xx < 17; ++xx)
							wdl->init_data->vertexes[base + zz * (17 + 16) + xx].y = file->mare[cz][cx].data[i++];
					}
					for (size_t zz = 0; zz < 16; ++zz)
					{
						for (size_t xx = 0; xx < 16; ++xx)
							wdl->init_data->vertexes[base + zz * (17 + 16) + 17 + xx].y = file->mare[cz][cx].data[i++];
					}
				}
				chunk_min = wdl->init_data->vertexes[base];
				chunk_max = wdl->init_data->vertexes[base];
				for (size_t i = 1; i < 17 * 17 + 16 * 16; ++i)
				{
					VEC3_MIN(chunk_min, chunk_min, wdl->init_data->vertexes[base + i]);
					VEC3_MAX(chunk_max, chunk_max, wdl->init_data->vertexes[base + i]);
				}
				for (size_t z = 0; z < 16; ++z)
				{
					for (size_t x = 0; x < 16; ++x)
					{
						uint32_t idx = base + 17 + z * 33 + x;
						uint32_t p1 = idx - 17;
						uint32_t p2 = idx - 16;
						uint32_t p3 = idx + 17;
						uint32_t p4 = idx + 16;
						wdl->init_data->indices[indices_pos++] = p2;
						wdl->init_data->indices[indices_pos++] = p1;
						wdl->init_data->indices[indices_pos++] = idx;
						wdl->init_data->indices[indices_pos++] = p3;
						wdl->init_data->indices[indices_pos++] = p2;
						wdl->init_data->indices[indices_pos++] = idx;
						wdl->init_data->indices[indices_pos++] = p4;
						wdl->init_data->indices[indices_pos++] = p3;
						wdl->init_data->indices[indices_pos++] = idx;
						wdl->init_data->indices[indices_pos++] = p1;
						wdl->init_data->indices[indices_pos++] = p4;
						wdl->init_data->indices[indices_pos++] = idx;
					}
				}
				chunk->aabb.p0 = chunk_min;
				chunk->aabb.p1 = chunk_max;
				VEC3_ADD(chunk->center, chunk_min, chunk_max);
				VEC3_MULV(chunk->center, chunk->center, 0.5f);
#ifdef WITH_DEBUG_RENDERING
				gx_aabb_set_aabb(&chunk->gx_aabb, &chunk->aabb);
#endif
			}
		}
	}
	wdl->attributes_state = GFX_ATTRIBUTES_STATE_INIT();
	wdl->indices_buffer = GFX_BUFFER_INIT();
	wdl->vertex_buffer = GFX_BUFFER_INIT();
	return wdl;

err:
	if (wdl)
	{
		clean_init_data(wdl->init_data);
		mem_free(MEM_GX, wdl);
	}
	return NULL;
}

void gx_wdl_delete(struct gx_wdl *wdl)
{
	if (!wdl)
		return;
	for (uint32_t i = 0; i < sizeof(wdl->chunks) / sizeof(*wdl->chunks); ++i)
	{
#ifdef WITH_DEBUG_RENDERING
		struct wdl_chunk *chunk = &wdl->chunks[i];
		gx_aabb_destroy(&chunk->gx_aabb);
#endif
	}
	for (size_t i = 0; i < RENDER_FRAMES_COUNT; ++i)
		gfx_delete_buffer(g_wow->device, &wdl->frames[i].uniform_buffer);
	gfx_delete_buffer(g_wow->device, &wdl->indices_buffer);
	gfx_delete_buffer(g_wow->device, &wdl->vertex_buffer);
	gfx_delete_attributes_state(g_wow->device, &wdl->attributes_state);
	clean_init_data(wdl->init_data);
	mem_free(MEM_GX, wdl);
}

static void initialize(struct gx_wdl *wdl)
{
	gfx_create_buffer(g_wow->device, &wdl->vertex_buffer, GFX_BUFFER_VERTEXES, wdl->init_data->vertexes, wdl->init_data->vertexes_nb * sizeof(*wdl->init_data->vertexes), GFX_BUFFER_IMMUTABLE);
	gfx_create_buffer(g_wow->device, &wdl->indices_buffer, GFX_BUFFER_INDICES, wdl->init_data->indices, wdl->init_data->indices_nb * sizeof(*wdl->init_data->indices), GFX_BUFFER_IMMUTABLE);
	for (size_t i = 0; i < RENDER_FRAMES_COUNT; ++i)
		gfx_create_buffer(g_wow->device, &wdl->frames[i].uniform_buffer, GFX_BUFFER_UNIFORM, NULL, sizeof(struct shader_wdl_model_block), GFX_BUFFER_STREAM);
	clean_init_data(wdl->init_data);
	wdl->init_data = NULL;
	const struct gfx_attribute_bind binds[] =
	{
		{&wdl->vertex_buffer},
	};
	gfx_create_attributes_state(g_wow->device, &wdl->attributes_state, binds, sizeof(binds) / sizeof(*binds), &wdl->indices_buffer, GFX_INDEX_UINT32);
}

void gx_wdl_cull(struct gx_wdl *wdl, struct gx_frame *frame)
{
	struct gx_wdl_frame *wdl_frame = &wdl->frames[frame->id];
	for (size_t i = 0; i < sizeof(wdl->chunks) / sizeof(*wdl->chunks); ++i)
	{
		struct wdl_chunk *chunk = &wdl->chunks[i];
		if (!(g_wow->map->adt_exists[i / 64] & (1ull << (i % 64))))
		{
			wdl_frame->culled[i / 8] |= (1 << (i % 8));
			continue;
		}
		struct vec3f delta;
		VEC3_SUB(delta, frame->cull_pos, chunk->center);
		if (delta.x > frame->view_distance * 2
		 || delta.y > frame->view_distance * 2)
		{
			wdl_frame->culled[i / 8] |= (1 << (i % 8));
			continue;
		}
		float distance_to_camera = sqrtf(delta.x * delta.x + delta.z * delta.z);
		if (distance_to_camera > frame->view_distance * 2 * 1.4142)
		{
			wdl_frame->culled[i / 8] |= (1 << (i % 8));
			continue;
		}
		if (!frustum_check_fast(&frame->wdl_frustum, &chunk->aabb))
		{
			wdl_frame->culled[i / 8] |= (1 << (i % 8));
			continue;
		}
		wdl_frame->culled[i / 8] &= ~(1 << (i % 8));
#ifdef WITH_DEBUG_RENDERING
		if (g_wow->gx->opt & GX_OPT_WDL_AABB)
			gx_aabb_add_to_render(&chunk->gx_aabb, frame, (struct mat4f*)&frame->view_wdl_vp);
#endif
	}
}

void gx_wdl_render(struct gx_wdl *wdl, struct gx_frame *frame)
{
	struct gx_wdl_frame *wdl_frame = &wdl->frames[frame->id];
	if (!wdl->initialized)
	{
		initialize(wdl);
		wdl->initialized = true;
	}
	bool initialized = false;
	for (size_t i = 0; i < sizeof(wdl->chunks) / sizeof(*wdl->chunks); ++i)
	{
		struct wdl_chunk *chunk = &wdl->chunks[i];
		if (wdl_frame->culled[i / 8] & (1 << (i % 8)))
			continue;
		if (!initialized)
		{
			struct shader_wdl_model_block model_block;
			VEC3_CPY(model_block.color, g_wow->map->gx_skybox->int_values[SKYBOX_INT_FOG]);
			model_block.color.w = 1;
			model_block.mvp = frame->view_wdl_vp;
			model_block.mv = frame->view_v;
			gfx_set_buffer_data(&wdl_frame->uniform_buffer, &model_block, sizeof(model_block), 0);
			gfx_bind_constant(g_wow->device, 1, &wdl_frame->uniform_buffer, sizeof(model_block), 0);
			gfx_bind_attributes_state(g_wow->device, &wdl->attributes_state, &g_wow->gx->wdl_input_layout);
			initialized = true;
		}
		gfx_draw_indexed(g_wow->device, 3072, chunk->indices_offset);
	}
}
