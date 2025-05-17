#include "gx/wmo_mliq.h"
#include "gx/frame.h"
#include "gx/wmo.h"
#include "gx/gx.h"

#include "shaders.h"
#include "camera.h"
#include "memory.h"
#include "const.h"
#include "log.h"
#include "wow.h"

#include <wow/wmo_group.h>

#include <gfx/device.h>

#include <sys/types.h>

MEMORY_DECL(GX);

struct gx_wmo_mliq_init_data
{
	struct jks_array indices[WMO_MLIQ_LIQUIDS_COUNT]; /* uint16_t */
	struct jks_array vertexes; /* struct shader_mliq_input */
};

static void init_data_init(struct gx_wmo_mliq_init_data *init_data)
{
	for (size_t i = 0; i < WMO_MLIQ_LIQUIDS_COUNT; ++i)
		jks_array_init(&init_data->indices[i], sizeof(uint16_t), NULL, &jks_array_memory_fn_GX);
	jks_array_init(&init_data->vertexes, sizeof(struct shader_mliq_input), NULL, &jks_array_memory_fn_GX);
}

static void init_data_delete(struct gx_wmo_mliq_init_data *init_data)
{
	if (!init_data)
		return;
	for (size_t i = 0; i < WMO_MLIQ_LIQUIDS_COUNT; ++i)
		jks_array_destroy(&init_data->indices[i]);
	jks_array_destroy(&init_data->vertexes);
	mem_free(MEM_GX, init_data);
}

struct gx_wmo_mliq *gx_wmo_mliq_new(struct wow_wmo_group_file *file)
{
	struct gx_wmo_mliq *mliq = mem_zalloc(MEM_GX, sizeof(*mliq));
	if (!mliq)
		return NULL;
	struct wow_mliq *wow_mliq = &file->mliq;
	VEC3_SET(mliq->position, wow_mliq->header.coords.x, 0, -wow_mliq->header.coords.y);
	for (size_t x = 0; x < wow_mliq->header.xtiles; ++x)
	{
		for (size_t z = 0; z < wow_mliq->header.ytiles; ++z)
		{
			uint8_t liquid_id = wow_mliq->tiles[x + z * wow_mliq->header.xtiles] & WOW_MLIQ_TILE_LIQUID_TYPE;
			if (liquid_id == 0xF) /* no liquid flag */
				continue;
			if (liquid_id >= WMO_MLIQ_LIQUIDS_COUNT)
			{
				LOG_ERROR("unknown WMO MLIQ id: %u", (int)liquid_id);
				continue;
			}
			if (liquid_id == 5)
			{
				LOG_ERROR("invalid WMO MLIQ id: 5");
				continue;
			}
			if (!mliq->init_data)
			{
				mliq->init_data = mem_malloc(MEM_GX, sizeof(*mliq->init_data));
				if (!mliq->init_data)
					goto err;
				init_data_init(mliq->init_data);
			}
			uint16_t *data = jks_array_grow(&mliq->init_data->indices[liquid_id], 6);
			if (!data)
				goto err;
			size_t p1 = x + z * wow_mliq->header.xverts;
			size_t p2 = p1 + 1;
			size_t p3 = p1 + wow_mliq->header.xverts + 1;
			size_t p4 = p1 + wow_mliq->header.xverts;
			data[0] = p1;
			data[1] = p2;
			data[2] = p3;
			data[3] = p1;
			data[4] = p3;
			data[5] = p4;
		}
	}
	mliq->empty = true;
	for (size_t i = 0; i < RENDER_FRAMES_COUNT; ++i)
	{
		for (size_t j = 0; j < WMO_MLIQ_LIQUIDS_COUNT; ++j)
		{
			jks_array_init(&mliq->frames[i].to_render[j], sizeof(struct gx_wmo_instance*), NULL, &jks_array_memory_fn_GX);
		}
	}
	for (size_t i = 0; i < WMO_MLIQ_LIQUIDS_COUNT; ++i)
	{
		mliq->liquids[i].indices_nb = mliq->init_data->indices[i].size;
		if (mliq->liquids[i].indices_nb)
			mliq->empty = false; /* shouln't break to continue indices_nb */
	}
	if (!mliq->empty)
	{
		if (!jks_array_resize(&mliq->init_data->vertexes, wow_mliq->header.xverts * wow_mliq->header.yverts))
			goto err;
		struct shader_mliq_input *vertex = JKS_ARRAY_GET(&mliq->init_data->vertexes, 0, struct shader_mliq_input);
		for (uint32_t z = 0; z < wow_mliq->header.yverts; ++z)
		{
			for (uint32_t x = 0; x < wow_mliq->header.xverts; ++x)
			{
				static const float xz_factor = CHUNK_WIDTH / 8.;
				static const float uv_factor = 2 / TILE_WIDTH;
				size_t n = z * wow_mliq->header.xverts + x;
				vertex[n].position.x = x * xz_factor;
				vertex[n].position.y = wow_mliq->vertexes[n].magma.height;
				vertex[n].position.z = -(ssize_t)z * xz_factor;
				vertex[n].uv.x = wow_mliq->vertexes[n].magma.s * uv_factor;
				vertex[n].uv.y = wow_mliq->vertexes[n].magma.t * uv_factor;
				vertex[n].uv.z = x;
				vertex[n].uv.w = z;
			}
		}
	}
	for (size_t i = 0; i < WMO_MLIQ_LIQUIDS_COUNT; ++i)
	{
		struct gx_wmo_mliq_liquid *liquid = &mliq->liquids[i];
		for (size_t j = 0; j < RENDER_FRAMES_COUNT; ++j)
			liquid->uniform_buffers[j] = GFX_BUFFER_INIT();
	}
	mliq->attributes_state = GFX_ATTRIBUTES_STATE_INIT();
	for (size_t i = 0; i < RENDER_FRAMES_COUNT; ++i)
		mliq->frames[i].uniform_buffer = GFX_BUFFER_INIT();
	mliq->vertexes_buffer = GFX_BUFFER_INIT();
	mliq->indices_buffer = GFX_BUFFER_INIT();
	return mliq;

err:
	if (mliq)
	{
		init_data_delete(mliq->init_data);
		mem_free(MEM_GX, mliq);
	}
	return NULL;
}

void gx_wmo_mliq_delete(struct gx_wmo_mliq *mliq)
{
	if (!mliq)
		return;
	for (size_t i = 0; i < WMO_MLIQ_LIQUIDS_COUNT; ++i)
	{
		struct gx_wmo_mliq_liquid *liquid = &mliq->liquids[i];
		for (size_t j = 0; j < RENDER_FRAMES_COUNT; ++j)
			gfx_delete_buffer(g_wow->device, &liquid->uniform_buffers[j]);
	}
	for (size_t i = 0; i < RENDER_FRAMES_COUNT; ++i)
	{
		for (size_t j = 0; j < WMO_MLIQ_LIQUIDS_COUNT; ++j)
			jks_array_destroy(&mliq->frames[i].to_render[j]);
		gfx_delete_buffer(g_wow->device, &mliq->frames[i].uniform_buffer);
	}
	gfx_delete_buffer(g_wow->device, &mliq->vertexes_buffer);
	gfx_delete_buffer(g_wow->device, &mliq->indices_buffer);
	gfx_delete_attributes_state(g_wow->device, &mliq->attributes_state);
	init_data_delete(mliq->init_data);
	mem_free(MEM_GX, mliq);
}

void gx_wmo_mliq_initialize(struct gx_wmo_mliq *mliq)
{
	size_t indices_total = 0;
	for (size_t i = 0; i < WMO_MLIQ_LIQUIDS_COUNT; ++i)
	{
		struct gx_wmo_mliq_liquid *liquid = &mliq->liquids[i];
		liquid->indices_offset = indices_total;
		indices_total += liquid->indices_nb;
	}
	gfx_create_buffer(g_wow->device, &mliq->indices_buffer, GFX_BUFFER_INDICES, NULL, indices_total * sizeof(uint16_t), GFX_BUFFER_STATIC);
	size_t indices_pos = 0;
	for (size_t i = 0; i < WMO_MLIQ_LIQUIDS_COUNT; ++i)
	{
		struct gx_wmo_mliq_liquid *liquid = &mliq->liquids[i];
		if (!liquid->indices_nb)
			continue;
		gfx_set_buffer_data(&mliq->indices_buffer, mliq->init_data->indices[i].data, mliq->init_data->indices[i].size * sizeof(uint16_t), indices_pos * sizeof(uint16_t));
		for (size_t j = 0; j < RENDER_FRAMES_COUNT; ++j)
			gfx_create_buffer(g_wow->device, &liquid->uniform_buffers[j], GFX_BUFFER_UNIFORM, NULL, sizeof(struct shader_mliq_mesh_block), GFX_BUFFER_STREAM);
		indices_pos += liquid->indices_nb;
	}
	gfx_create_buffer(g_wow->device, &mliq->vertexes_buffer, GFX_BUFFER_VERTEXES, mliq->init_data->vertexes.data, mliq->init_data->vertexes.size * sizeof(struct shader_mliq_input), GFX_BUFFER_IMMUTABLE);
	for (size_t i = 0; i < RENDER_FRAMES_COUNT; ++i)
		gfx_create_buffer(g_wow->device, &mliq->frames[i].uniform_buffer, GFX_BUFFER_UNIFORM, NULL, sizeof(struct shader_mliq_model_block), GFX_BUFFER_STREAM);
	init_data_delete(mliq->init_data);
	mliq->init_data = NULL;
	const struct gfx_attribute_bind binds[] =
	{
		{&mliq->vertexes_buffer},
	};
	gfx_create_attributes_state(g_wow->device, &mliq->attributes_state, binds, sizeof(binds) / sizeof(*binds), &mliq->indices_buffer, GFX_INDEX_UINT16);
}

void gx_wmo_mliq_clear_update(struct gx_wmo_mliq *mliq, struct gx_frame *frame)
{
	for (size_t i = 0; i < WMO_MLIQ_LIQUIDS_COUNT; ++i)
		jks_array_resize(&mliq->frames[frame->id].to_render[i], 0);
}

void gx_wmo_mliq_add_to_render(struct gx_wmo_mliq *mliq, struct gx_wmo_instance *instance, struct gx_frame *frame)
{
	if (mliq->empty)
		return;
	for (size_t i = 0; i < WMO_MLIQ_LIQUIDS_COUNT; ++i)
	{
		if (!mliq->liquids[i].indices_nb)
			continue;
		if (!mliq->in_render_list)
		{
			gx_frame_add_wmo_mliq_backref(frame, mliq);
			mliq->in_render_list = true;
		}
		if (!mliq->in_render_lists[i])
		{
			gx_frame_add_wmo_mliq(frame, mliq, i);
			mliq->in_render_lists[i] = true;
		}
		if (!jks_array_push_back(&mliq->frames[frame->id].to_render[i], &instance))
		{
			LOG_ERROR("failed to add instance to mliq render list");
			continue;
		}
	}
}

static void render_liquid(struct gx_wmo_mliq_liquid *liquid, struct gx_frame *frame, uint8_t type)
{
	const float alphas[9] = {0.5f, 1.0f, 1.0f, 1.0f, 0.5f, 0.0f, 1.0f, 1.0f, 0.5f};
	struct shader_mliq_mesh_block mesh_block;
	mesh_block.alpha = alphas[type];
	mesh_block.type = type;
	gfx_set_buffer_data(&liquid->uniform_buffers[frame->id], &mesh_block, sizeof(mesh_block), 0);
	gfx_bind_constant(g_wow->device, 0, &liquid->uniform_buffers[frame->id], sizeof(mesh_block), 0);
	gfx_draw_indexed(g_wow->device, liquid->indices_nb, liquid->indices_offset);
}

void gx_wmo_mliq_render(struct gx_wmo_mliq *mliq, struct gx_frame *frame, uint8_t type)
{
	if (!mliq->attributes_state.handle.ptr)
		return;
	struct gx_wmo_mliq_frame *mliq_frame = &mliq->frames[frame->id];
	gfx_bind_attributes_state(g_wow->device, &mliq->attributes_state, &g_wow->gx->wmo_mliq_input_layout);
	for (size_t i = 0; i < mliq_frame->to_render[type].size; ++i)
	{
		struct gx_wmo_instance *instance = *JKS_ARRAY_GET(&mliq_frame->to_render[type], i, struct gx_wmo_instance*);
		struct mat4f tmp;
		MAT4_TRANSLATE(tmp, instance->m, mliq->position);
		struct shader_mliq_model_block model_block;
		model_block.v = frame->view_v;
		MAT4_MUL(model_block.mv, model_block.v, tmp);
		MAT4_MUL(model_block.mvp, frame->view_p, model_block.mv);
		gfx_set_buffer_data(&mliq_frame->uniform_buffer, &model_block, sizeof(model_block), 0);
		gfx_bind_constant(g_wow->device, 1, &mliq_frame->uniform_buffer, sizeof(model_block), 0);
		render_liquid(&mliq->liquids[type], frame, type);
	}
}
