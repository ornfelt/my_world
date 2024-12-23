#include "gx/wmo_group.h"
#include "gx/wmo_mliq.h"
#include "gx/frame.h"
#include "gx/wmo.h"
#include "gx/blp.h"
#include "gx/m2.h"

#include "performance.h"
#include "graphics.h"
#include "shaders.h"
#include "camera.h"
#include "loader.h"
#include "memory.h"
#include "cache.h"
#include "log.h"
#include "wow.h"

#include <wow/wmo_group.h>
#include <wow/mpq.h>

#include <gfx/device.h>

#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

MEMORY_DECL(GX);

struct gx_wmo_group_init_data
{
	struct jks_array vertexes; /* struct shader_wmo_input */
	struct jks_array indices; /* uint16_t */
	struct jks_array colors; /* struct vec4b */
};

static void init_data_init(struct gx_wmo_group_init_data *init_data)
{
	jks_array_init(&init_data->vertexes, sizeof(struct shader_wmo_input), NULL, &jks_array_memory_fn_GX);
	jks_array_init(&init_data->indices, sizeof(uint16_t), NULL, &jks_array_memory_fn_GX);
	jks_array_init(&init_data->colors, sizeof(struct vec4b), NULL, &jks_array_memory_fn_GX);
}

static void init_data_delete(struct gx_wmo_group_init_data *init_data)
{
	if (!init_data)
		return;
	jks_array_destroy(&init_data->vertexes);
	jks_array_destroy(&init_data->indices);
	jks_array_destroy(&init_data->colors);
	mem_free(MEM_GX, init_data);
}

static void batch_init(struct gx_wmo_batch *batch, struct gx_wmo_group *parent, struct wow_moba_data *moba)
{
	batch->parent = parent;
	batch->texture1 = NULL;
	batch->texture2 = NULL;
	struct vec3f p0 = {(float)moba->aabb0.x, (float)moba->aabb0.z, (float)-moba->aabb0.y};
	struct vec3f p1 = {(float)moba->aabb1.x, (float)moba->aabb1.z, (float)-moba->aabb1.y};
	VEC3_MIN(batch->aabb.p0, p0, p1);
	VEC3_MAX(batch->aabb.p1, p0, p1);
	batch->indices_offset = moba->start_index;
	batch->indices_nb = moba->count;
	struct wow_momt_data *momt = JKS_ARRAY_GET(&parent->parent->momt, moba->material_id, struct wow_momt_data);
	batch->shader = momt->shader;
	enum world_rasterizer_state rasterizer_state;
	enum world_blend_state blend_state;
	switch (momt->blend_mode)
	{
		case 0: /* opaque */
			blend_state = WORLD_BLEND_OPAQUE;
			batch->alpha_test = 0; /* 1. / 255.; */
			break;
		case 1: /* alpha key */
			blend_state = WORLD_BLEND_OPAQUE;
			batch->alpha_test = 224. / 255.;
			break;
		case 2: /* alpha */
			blend_state = WORLD_BLEND_ALPHA;
			batch->alpha_test = 224. / 255.;
			break;
		default:
			blend_state = WORLD_BLEND_ALPHA;
			batch->alpha_test = 224. / 255.;
			LOG_WARN("unsupported blend mode: %u", momt->blend_mode);
	}
	/* Mainly seen: 0, 1, 5
	 */
#if 0
	if (batch->shader)
		LOG_DEBUG("WMOshader: %u", batch->shader);
#endif
	batch->flags1 = momt->flags;
	rasterizer_state = batch->flags1 & WOW_MOMT_FLAGS_UNCULLED ? WORLD_RASTERIZER_UNCULLED : WORLD_RASTERIZER_CULLED;
	batch->pipeline_state = &g_wow->graphics->wmo_pipeline_states[rasterizer_state][blend_state] - &g_wow->graphics->wmo_pipeline_states[0][0];
	char texture_name[512];
	snprintf(texture_name, sizeof(texture_name), "%s", JKS_ARRAY_GET(&parent->parent->motx, momt->texture1, char));
	if (texture_name[0])
	{
		wow_mpq_normalize_blp_fn(texture_name, sizeof(texture_name));
		if (cache_ref_blp(g_wow->cache, texture_name, &batch->texture1))
		{
			gx_blp_ask_load(batch->texture1);
		}
		else
		{
			LOG_ERROR("texture not found: %s", texture_name);
			batch->texture1 = NULL;
		}
	}
	batch->flags2 = momt->flags2;
	snprintf(texture_name, sizeof(texture_name), "%s", JKS_ARRAY_GET(&parent->parent->motx, momt->texture2, char));
	if (texture_name[0])
	{
		wow_mpq_normalize_blp_fn(texture_name, sizeof(texture_name));
		if (cache_ref_blp(g_wow->cache, texture_name, &batch->texture2))
		{
			gx_blp_ask_load(batch->texture2);
		}
		else
		{
			LOG_ERROR("texture not found: %s", texture_name);
			batch->texture2 = NULL;
		}
	}
	for (size_t i = 0; i < RENDER_FRAMES_COUNT; ++i)
		batch->uniform_buffers[i] = GFX_BUFFER_INIT();
}

static void batch_destroy(struct gx_wmo_batch *batch)
{
	for (size_t i = 0; i < RENDER_FRAMES_COUNT; ++i)
		gfx_delete_buffer(g_wow->device, &batch->uniform_buffers[i]);
	gx_blp_free(batch->texture1);
	gx_blp_free(batch->texture2);
}

static void batch_prepare_draw(struct gx_wmo_group *group, struct gx_frame *frame, struct gx_wmo_batch *batch)
{
	if (!batch->uniform_buffers[frame->id].handle.u64)
		gfx_create_buffer(g_wow->device, &batch->uniform_buffers[frame->id], GFX_BUFFER_UNIFORM, NULL, sizeof(struct shader_wmo_mesh_block), GFX_BUFFER_STREAM);
	PERFORMANCE_BEGIN(WMO_RENDER_DATA);
	struct shader_wmo_mesh_block mesh_block;
	VEC4_SETV(mesh_block.emissive_color, 0);
	VEC4_SETV(mesh_block.combiners, 0);
	mesh_block.alpha_test = batch->alpha_test;
	mesh_block.settings.x = (group->wow_flags & WOW_MOGP_FLAGS_COLOR) ? 1 : 0;
	mesh_block.settings.y = (batch->flags1 & WOW_MOMT_FLAGS_UNLIT) ? 1 : 0;
	mesh_block.settings.z = (batch->flags1 & WOW_MOMT_FLAGS_UNFOGGED) ? 1 : 0;
	mesh_block.settings.w = batch->shader;
	gfx_set_buffer_data(&batch->uniform_buffers[frame->id], &mesh_block, sizeof(mesh_block), 0);
	PERFORMANCE_END(WMO_RENDER_DATA);
	PERFORMANCE_BEGIN(WMO_RENDER_BIND);
	const gfx_texture_t *textures[2];
	if (batch->texture1)
	{
		if (gx_blp_flag_get(batch->texture1, GX_BLP_FLAG_INITIALIZED))
		{
			enum gfx_texture_addressing addressing_s = (batch->flags1 & WOW_MOMT_FLAGS_CLAMP_S) ? GFX_TEXTURE_ADDRESSING_CLAMP : GFX_TEXTURE_ADDRESSING_REPEAT;
			enum gfx_texture_addressing addressing_t = (batch->flags1 & WOW_MOMT_FLAGS_CLAMP_T) ? GFX_TEXTURE_ADDRESSING_CLAMP : GFX_TEXTURE_ADDRESSING_REPEAT;
			gfx_set_texture_addressing(&batch->texture1->texture, addressing_s, addressing_t, GFX_TEXTURE_ADDRESSING_CLAMP);
			textures[0] = &batch->texture1->texture;
		}
		else
		{
			textures[0] = &g_wow->grey_texture->texture;
		}
	}
	else
	{
		textures[0] = &g_wow->grey_texture->texture;
	}
	if (batch->texture2)
	{
		if (gx_blp_flag_get(batch->texture2, GX_BLP_FLAG_INITIALIZED))
		{
			enum gfx_texture_addressing addressing_s = (batch->flags2 & WOW_MOMT_FLAGS_CLAMP_S) ? GFX_TEXTURE_ADDRESSING_CLAMP : GFX_TEXTURE_ADDRESSING_REPEAT;
			enum gfx_texture_addressing addressing_t = (batch->flags2 & WOW_MOMT_FLAGS_CLAMP_T) ? GFX_TEXTURE_ADDRESSING_CLAMP : GFX_TEXTURE_ADDRESSING_REPEAT;
			gfx_set_texture_addressing(&batch->texture2->texture, addressing_s, addressing_t, GFX_TEXTURE_ADDRESSING_CLAMP);
			textures[1] = &batch->texture2->texture;
		}
		else
		{
			textures[1] = &g_wow->grey_texture->texture;
		}
	}
	else
	{
		textures[1] = &g_wow->grey_texture->texture;
	}
	gfx_bind_samplers(g_wow->device, 0, 1, textures); /* XXX 2 textures */
	gfx_bind_constant(g_wow->device, 0, &batch->uniform_buffers[frame->id], sizeof(struct shader_wmo_mesh_block), 0);
	if (group->wow_flags & WOW_MOGP_FLAGS_COLOR)
		gfx_bind_pipeline_state(g_wow->device, &((gfx_pipeline_state_t*)g_wow->graphics->wmo_colored_pipeline_states)[batch->pipeline_state]);
	else
		gfx_bind_pipeline_state(g_wow->device, &((gfx_pipeline_state_t*)g_wow->graphics->wmo_pipeline_states)[batch->pipeline_state]);
	PERFORMANCE_END(WMO_RENDER_BIND);
}

static void batch_render(struct gx_wmo_batch *batch)
{
	PERFORMANCE_BEGIN(WMO_RENDER_DRAW);
	gfx_draw_indexed(g_wow->device, batch->indices_nb, batch->indices_offset);
	PERFORMANCE_END(WMO_RENDER_DRAW);
}

struct gx_wmo_group *gx_wmo_group_new(struct gx_wmo *parent, uint32_t index, uint32_t flags)
{
	struct gx_wmo_group *group = mem_malloc(MEM_GX, sizeof(*group));
	if (!group)
		return NULL;
	char tmp[512];
	snprintf(tmp, sizeof(tmp), "%.*s_%03d.WMO", (int)strlen(parent->filename) - 4, parent->filename, index);
	group->filename = mem_strdup(MEM_GX, tmp);
	if (!group->filename)
	{
		mem_free(MEM_GX, group);
		return NULL;
	}
	group->gx_mliq = NULL;
	group->parent = parent;
	group->index = index;
	group->init_data = NULL;
	group->wow_flags = flags;
	group->flags = 0;
	jks_array_init(&group->batches, sizeof(struct gx_wmo_batch), (jks_array_destructor_t)batch_destroy, &jks_array_memory_fn_GX);
	jks_array_init(&group->doodads, sizeof(uint16_t), NULL, &jks_array_memory_fn_GX);
	jks_array_init(&group->lights, sizeof(uint16_t), NULL, &jks_array_memory_fn_GX);
	jks_array_init(&group->mobn, sizeof(struct wow_mobn_node), NULL, &jks_array_memory_fn_GX);
	jks_array_init(&group->mobr, sizeof(uint16_t), NULL, &jks_array_memory_fn_GX);
	jks_array_init(&group->movi, sizeof(uint16_t), NULL, &jks_array_memory_fn_GX);
	jks_array_init(&group->movt, sizeof(struct wow_vec3f), NULL, &jks_array_memory_fn_GX);
	jks_array_init(&group->mopy, sizeof(struct wow_mopy_data), NULL, &jks_array_memory_fn_GX);
	group->attributes_state = GFX_ATTRIBUTES_STATE_INIT();
	group->vertexes_buffer = GFX_BUFFER_INIT();
	group->indices_buffer = GFX_BUFFER_INIT();
	group->colors_buffer = GFX_BUFFER_INIT();
#ifdef WITH_DEBUG_RENDERING
	gx_wmo_collisions_init(&group->gx_collisions);
#endif
	return group;
}

static void wmo_group_unload_task(struct wow_mpq_compound *mpq_compound, void *userdata)
{
	(void)mpq_compound;
	struct gx_wmo_group *group = userdata;
#ifdef WITH_DEBUG_RENDERING
	gx_wmo_collisions_destroy(&group->gx_collisions);
#endif
	jks_array_destroy(&group->batches);
	jks_array_destroy(&group->doodads);
	jks_array_destroy(&group->lights);
	jks_array_destroy(&group->mobn);
	jks_array_destroy(&group->mobr);
	jks_array_destroy(&group->movi);
	jks_array_destroy(&group->movt);
	jks_array_destroy(&group->mopy);
	gx_wmo_mliq_delete(group->gx_mliq);
	gfx_delete_buffer(g_wow->device, &group->vertexes_buffer);
	gfx_delete_buffer(g_wow->device, &group->indices_buffer);
	gfx_delete_buffer(g_wow->device, &group->colors_buffer);
	gfx_delete_attributes_state(g_wow->device, &group->attributes_state);
	init_data_delete(group->init_data);
	mem_free(MEM_GX, group->filename);
	mem_free(MEM_GX, group);
}

void gx_wmo_group_free(struct gx_wmo_group *group)
{
	if (!group)
		return;
	loader_push(g_wow->loader, ASYNC_TASK_WMO_GROUP_UNLOAD, wmo_group_unload_task, group);
}

static bool initialize(void *userdata)
{
	struct gx_wmo_group *group = userdata;
	gfx_create_buffer(g_wow->device, &group->vertexes_buffer, GFX_BUFFER_VERTEXES, group->init_data->vertexes.data, group->init_data->vertexes.size * sizeof(struct shader_wmo_input), GFX_BUFFER_IMMUTABLE);
	gfx_create_buffer(g_wow->device, &group->indices_buffer, GFX_BUFFER_INDICES, group->init_data->indices.data, group->init_data->indices.size * sizeof(uint16_t), GFX_BUFFER_IMMUTABLE);
	if (group->wow_flags & WOW_MOGP_FLAGS_COLOR)
		gfx_create_buffer(g_wow->device, &group->colors_buffer, GFX_BUFFER_VERTEXES, group->init_data->colors.data, group->init_data->colors.size * sizeof(vec4b_t), GFX_BUFFER_IMMUTABLE);
	init_data_delete(group->init_data);
	group->init_data = NULL;
	const struct gfx_attribute_bind binds[] =
	{
		{&group->vertexes_buffer},
		{(group->wow_flags & WOW_MOGP_FLAGS_COLOR) ? &group->colors_buffer : NULL},
	};
	gfx_create_attributes_state(g_wow->device, &group->attributes_state, binds, sizeof(binds) / sizeof(*binds), &group->indices_buffer, GFX_INDEX_UINT16);
	if (group->gx_mliq)
		gx_wmo_mliq_initialize(group->gx_mliq);
#ifdef WITH_DEBUG_RENDERING
	gx_wmo_collisions_initialize(&group->gx_collisions);
#endif
	gx_wmo_group_flag_set(group, GX_WMO_GROUP_FLAG_INITIALIZED);
	return true;
}

static void fix_mocv(struct gx_wmo_group *group, struct wow_wmo_group_file *file)
{
	uint32_t int_batch_start;
	if (file->mogp.trans_batch_count > 0)
		int_batch_start = file->moba.data[file->mogp.trans_batch_count - 1].max_index + 1;
	else
		int_batch_start = 0;
	for (uint32_t i = 0; i < group->init_data->colors.size; ++i)
	{
		vec4b_t *color = JKS_ARRAY_GET(&group->init_data->colors, i, vec4b_t);
		if (i >= int_batch_start)
		{
			int32_t v6 = (color->x + (color->w * color->x / 64)) / 2;
			int32_t v7 = (color->y + (color->w * color->y / 64)) / 2;
			int32_t v8 = (color->z + (color->w * color->z / 64)) / 2;
			if (v6 > 0xff)
				v6 = 0xff;
			if (v7 > 0xff)
				v7 = 0xff;
			if (v8 > 0xff)
				v8 = 0xff;
			color->x = v6;
			color->y = v7;
			color->z = v8;
			color->w = 0xff;
		}
		else
		{
			color->x /= 2;
			color->y /= 2;
			color->z /= 2;
		}
	}
}

static void update_mocv_portals(struct gx_wmo_group *group)
{
	for (size_t j = 0; j < group->init_data->colors.size; ++j)
	{
		struct vec4b *color = JKS_ARRAY_GET(&group->init_data->colors, j, struct vec4b);
		struct shader_wmo_input *vertex = JKS_ARRAY_GET(&group->init_data->vertexes, j, struct shader_wmo_input);
		uint8_t min = 255;
		for (size_t i = 0; i < group->portal_count; ++i)
		{
			if (group->portal_start + i >= group->parent->mopr.size)
			{
				LOG_WARN("invalid portal id: %" PRIu32 " / %" PRIu32, (uint32_t)(group->portal_start + i), (uint32_t)group->parent->mopr.size);
				continue;
			}
			struct wow_mopr_data *mopr = JKS_ARRAY_GET(&group->parent->mopr, group->portal_start + i, struct wow_mopr_data);
			struct gx_wmo_group *neighboor = *JKS_ARRAY_GET(&group->parent->groups, mopr->group_index, struct gx_wmo_group*);
			if ((neighboor->wow_flags & WOW_MOGP_FLAGS_INDOOR)
			 && !(neighboor->wow_flags & WOW_MOGP_FLAGS_EXT_LIGHT)
			 && !(neighboor->wow_flags & WOW_MOGP_FLAGS_OUTDOOR))
				continue;
			struct wow_mopt_data *mopt = JKS_ARRAY_GET(&group->parent->mopt, mopr->portal_index, struct wow_mopt_data);
			float delta = fabs(VEC3_DOT(vertex->position, mopt->normal) + mopt->distance);
			delta /= 6;
			if (delta < 0)
				delta = 0;
			if (delta > 1)
				delta = 1;
			delta *= 255;
			if (delta < min)
				min = delta;
		}
#if 0
		VEC3_SET(*color, 0, 255, 0);
#endif
		color->w = 255 - min;
	}
}

static void load(struct gx_wmo_group *group, struct wow_wmo_group_file *file)
{
	group->init_data = mem_malloc(MEM_GX, sizeof(*group->init_data));
	if (!group->init_data)
	{
		LOG_ERROR("failed to malloc wmo group init data");
		return;
	}
	init_data_init(group->init_data);
	struct wow_mogp *mogp = &file->mogp;
#if 0
	LOG_ERROR("%s flags: %x", group->filename, mogp->flags);
#endif
	group->portal_start = mogp->portal_start;
	group->portal_count = mogp->portal_count;
	group->wow_flags = mogp->flags;
	if (group->wow_flags & WOW_MOGP_FLAGS_LIQUID)
	{
		group->gx_mliq = gx_wmo_mliq_new(file);
		if (!group->gx_mliq)
		{
			LOG_ERROR("failed to create mliq group");
			return;
		}
	}
	if (!jks_array_resize(&group->init_data->vertexes, file->movt.size))
	{
		LOG_ERROR("failed to resize wmo group vertexes");
		return;
	}
	for (size_t i = 0; i < file->movt.size; ++i)
	{
		struct shader_wmo_input *vertex = JKS_ARRAY_GET(&group->init_data->vertexes, i, struct shader_wmo_input);
		VEC3_CPY(vertex->position, file->movt.data[i]);
		vertex->position = (struct vec3f){vertex->position.x, vertex->position.z, -vertex->position.y};
		VEC3_CPY(vertex->norm, file->monr.data[i]);
		vertex->norm = (struct vec3f){vertex->norm.x, vertex->norm.z, -vertex->norm.y};
		VEC2_CPY(vertex->uv, file->motv.data[i]);
	}
	if (!jks_array_resize(&group->init_data->indices, file->movi.size))
	{
		LOG_ERROR("failed to resize wmo group indices");
		return;
	}
	memcpy(group->init_data->indices.data, file->movi.data, file->movi.size * sizeof(*file->movi.data));
	if (group->wow_flags & WOW_MOGP_FLAGS_DOODAD)
	{
		if (!jks_array_resize(&group->doodads, file->modr.size))
		{
			LOG_ERROR("failed to resize wmo group doodads");
			return;
		}
		memcpy(group->doodads.data, file->modr.data, file->modr.size * sizeof(*file->modr.data));
	}
	if (group->wow_flags & WOW_MOGP_FLAGS_LIGHT)
	{
		if (!jks_array_resize(&group->lights, file->molr.size))
		{
			LOG_ERROR("failed to resize wmo group lights");
			return;
		}
		memcpy(group->lights.data, file->molr.data, file->molr.size * sizeof(*file->molr.data));
	}
	if (group->wow_flags & WOW_MOGP_FLAGS_COLOR)
	{
		if (!jks_array_resize(&group->init_data->colors, file->mocv.size))
		{
			LOG_ERROR("failed to resize wmo group colors");
			return;
		}
		memcpy(group->init_data->colors.data, file->mocv.data, file->mocv.size * sizeof(*file->mocv.data));
		if (group->parent->wow_flags & WOW_MOHD_FLAGS_LIGHTEN_INTERIORS)
			fix_mocv(group, file);
		if (!(group->parent->wow_flags & WOW_MOHD_FLAGS_ATTENUATE_PORTAL_DISTANCE))
			update_mocv_portals(group);
	}
	struct vec3f p0 = {mogp->aabb0.x, mogp->aabb0.z, -mogp->aabb0.y};
	struct vec3f p1 = {mogp->aabb1.x, mogp->aabb1.z, -mogp->aabb1.y};
	VEC3_MIN(group->aabb.p0, p0, p1);
	VEC3_MAX(group->aabb.p1, p0, p1);
	if (!jks_array_resize(&group->batches, file->moba.size))
	{
		LOG_ERROR("failed to resize wmo group batches");
		return;
	}
	for (uint32_t i = 0; i < file->moba.size; ++i)
		batch_init(JKS_ARRAY_GET(&group->batches, i, struct gx_wmo_batch), group, &file->moba.data[i]);
	/* wmo cache is already locked in loader */
	for (size_t i = 0; i < group->parent->instances.size; ++i)
	{
		struct gx_wmo_instance *instance = *JKS_ARRAY_GET(&group->parent->instances, i, struct gx_wmo_instance*);
		if (!instance->groups.size) /* XXX better not loaded detection */
			continue;
		struct gx_wmo_group_instance *group_instance = JKS_ARRAY_GET(&instance->groups, group->index, struct gx_wmo_group_instance);
		if (!gx_wmo_group_instance_on_load(instance, group, group_instance))
			LOG_ERROR("failed to load group instance");
	}
	if (file->mobn.size)
	{
		if (!jks_array_resize(&group->mobn, file->mobn.size))
		{
			LOG_ERROR("failed to resize mobn array");
			return;
		}
		memcpy(group->mobn.data, file->mobn.data, sizeof(*file->mobn.data) * file->mobn.size);
	}
	if (file->mobr.size)
	{
		if (!jks_array_resize(&group->mobr, file->mobr.size))
		{
			LOG_ERROR("failed to resize mobr array");
			return;
		}
		memcpy(group->mobr.data, file->mobr.data, sizeof(*file->mobr.data) * file->mobr.size);
	}
	if (file->movi.size)
	{
		if (!jks_array_resize(&group->movi, file->movi.size))
		{
			LOG_ERROR("failed to resize movi array");
			return;
		}
		memcpy(group->movi.data, file->movi.data, sizeof(*file->movi.data) * file->movi.size);
	}
	if (file->movt.size)
	{
		if (!jks_array_resize(&group->movt, file->movt.size))
		{
			LOG_ERROR("failed to resize movt array");
			return;
		}
		memcpy(group->movt.data, file->movt.data, sizeof(*file->movt.data) * file->movt.size);
		for (size_t i = 0; i < group->movt.size; ++i)
		{
			struct wow_vec3f *tmp = JKS_ARRAY_GET(&group->movt, i, struct wow_vec3f);
			*tmp = (struct wow_vec3f){tmp->x, tmp->z, -tmp->y};
		}
	}
	if (file->mopy.size)
	{
		if (!jks_array_resize(&group->mopy, file->mopy.size))
		{
			LOG_ERROR("failed to resize mopy array");
			return;
		}
		memcpy(group->mopy.data, file->mopy.data, sizeof(*file->mopy.data) * file->mopy.size);
	}
#ifdef WITH_DEBUG_RENDERING
	if (group->wow_flags & WOW_MOGP_FLAGS_BSP)
		gx_wmo_collisions_load(&group->gx_collisions, group->mobr.data, group->mobr.size, group->movi.data, group->movt.data, group->mopy.data);
#endif
	gx_wmo_group_flag_set(group, GX_WMO_GROUP_FLAG_LOADED);
	loader_init_object(g_wow->loader, LOADER_WMO_GROUP, initialize, group);
}

static void wmo_group_load_task(struct wow_mpq_compound *mpq_compound, void *userdata)
{
	struct gx_wmo_group *group = userdata;
	struct wow_mpq_file *mpq_file = NULL;
	struct wow_wmo_group_file *wmo_group_file = NULL;

	mpq_file = wow_mpq_get_file(mpq_compound, group->filename);
	if (!mpq_file)
	{
		LOG_WARN("WMO group file not found: %s", group->filename);
		goto end;
	}
	wmo_group_file = wow_wmo_group_file_new(mpq_file);
	if (!wmo_group_file)
	{
		LOG_ERROR("failed to create wmo group from file %s", group->filename);
		return;
	}
	cache_lock_wmo(g_wow->cache);
	load(group, wmo_group_file);
	cache_unlock_wmo(g_wow->cache);

end:
	wow_wmo_group_file_delete(wmo_group_file);
	wow_mpq_file_delete(mpq_file);
}

void gx_wmo_group_ask_load(struct gx_wmo_group *group)
{
	if (gx_wmo_group_flag_set(group, GX_WMO_GROUP_FLAG_LOAD_ASKED))
		return;
	loader_push(g_wow->loader, ASYNC_TASK_WMO_GROUP_LOAD, wmo_group_load_task, group);
}

void gx_wmo_group_render(struct gx_wmo_group *group, struct gx_frame *frame, struct jks_array *instances)
{
	if (!gx_wmo_group_flag_get(group, GX_WMO_GROUP_FLAG_INITIALIZED))
		return;
	gfx_bind_attributes_state(g_wow->device, &group->attributes_state, (group->wow_flags & WOW_MOGP_FLAGS_COLOR) ? &g_wow->graphics->wmo_colored_input_layout : &g_wow->graphics->wmo_input_layout);
	for (size_t i = 0; i < group->batches.size; ++i)
	{
		bool initialized = false;
		for (size_t j = 0; j < instances->size; ++j)
		{
			struct gx_wmo_instance *instance = *JKS_ARRAY_GET(instances, j, struct gx_wmo_instance*);
			struct gx_wmo_instance_frame *instance_frame = &instance->frames[frame->id];
			struct gx_wmo_group_instance *group_instance = JKS_ARRAY_GET(&instance->groups, group->index, struct gx_wmo_group_instance);
			struct gx_wmo_group_instance_frame *group_instance_frame = &group_instance->frames[frame->id];
			if (group_instance_frame->culled)
				continue;
			struct gx_wmo_batch_instance *batch_instance = JKS_ARRAY_GET(&group_instance->batches, i, struct gx_wmo_batch_instance);
			struct gx_wmo_batch_instance_frame *batch_instance_frame = &batch_instance->frames[frame->id];
			if (batch_instance_frame->culled)
				continue;
			struct gx_wmo_batch *batch = JKS_ARRAY_GET(&group->batches, i, struct gx_wmo_batch);
			if (!initialized)
			{
				batch_prepare_draw(group, frame, batch);
				initialized = true;
			}
			PERFORMANCE_BEGIN(WMO_RENDER_BIND);
			gfx_bind_constant(g_wow->device, 1, &instance_frame->uniform_buffer, sizeof(struct shader_wmo_model_block), 0);
			PERFORMANCE_END(WMO_RENDER_BIND);
			batch_render(batch);
		}
	}
}

static bool cull_portal_rec(struct gx_wmo_group *group, struct gx_wmo_instance *instance, struct gx_frame *frame, struct jks_array *frustums, struct vec4f rpos, struct jks_array *transformed)
{
	if (!gx_wmo_group_flag_get(group, GX_WMO_GROUP_FLAG_LOADED))
		return true;
	bool ret = false;
	struct frustum *new_frustum;
	struct gx_wmo_group_instance *group_instance = JKS_ARRAY_GET(&instance->groups, group->index, struct gx_wmo_group_instance);
	struct gx_wmo_group_instance_frame *group_instance_frame = &group_instance->frames[frame->id];
	for (size_t i = 0; i < group->doodads.size; ++i)
	{
		uint16_t doodad = *JKS_ARRAY_GET(&group->doodads, i, uint16_t);
		if (doodad < instance->doodad_start || doodad >= instance->doodad_end)
			continue;
		struct gx_m2_instance *m2_instance = *JKS_ARRAY_GET(&instance->m2, doodad - instance->doodad_start, struct gx_m2_instance*);
		if (gx_m2_instance_flag_get(m2_instance, GX_M2_INSTANCE_FLAG_IN_RENDER_LIST))
			continue;
		bool outside = false;
		for (size_t j = 0; j < frustums->size; ++j)
		{
			if (!frustum_check_fast(JKS_ARRAY_GET(frustums, j, struct frustum), &m2_instance->aabb))
			{
				outside = true;
				break;
			}
		}
		if (outside)
			continue;
		gx_m2_instance_add_to_render(m2_instance, frame, true, &frame->m2_params);
	}
	for (size_t k = 0; k < group->batches.size; ++k)
	{
		struct gx_wmo_batch_instance *batch_instance = JKS_ARRAY_GET(&group_instance->batches, k, struct gx_wmo_batch_instance);
		struct gx_wmo_batch_instance_frame *batch_instance_frame = &batch_instance->frames[frame->id];
		if (group_instance_frame->cull_source)
		{
			if (!batch_instance_frame->culled)
				continue;
		}
		bool outside = false;
		for (size_t i = 0; i < frustums->size; ++i)
		{
			if (!frustum_check_fast(JKS_ARRAY_GET(frustums, i, struct frustum), &batch_instance->aabb))
			{
				outside = true;
				break;
			}
		}
		if (outside)
			continue;
		batch_instance_frame->culled = false;
	}
	group_instance_frame->culled = false;
	for (size_t i = 0; i < group->portal_count; ++i)
	{
		if (group->portal_start + i >= group->parent->mopr.size)
		{
			LOG_WARN("invalid portal id: %" PRIu32 " / %" PRIu32 " (%s)", (uint32_t)(group->portal_start + i), (uint32_t)group->parent->mopr.size, group->parent->filename);
			continue;
		}
		size_t portal = group->portal_start + i;
		if (instance->traversed_portals[portal / 8] & (1 << (portal % 8)))
			continue;
		instance->traversed_portals[portal / 8] |= 1 << (portal % 8);
		struct wow_mopr_data *mopr = JKS_ARRAY_GET(&group->parent->mopr, group->portal_start + i, struct wow_mopr_data);
		struct wow_mopt_data *mopt = JKS_ARRAY_GET(&group->parent->mopt, mopr->portal_index, struct wow_mopt_data);
		if ((VEC3_DOT(mopt->normal, rpos) + mopt->distance < 0) != (mopr->side < 0))
			continue;
		uint32_t base = mopt->start_vertex;
		if (!jks_array_resize(transformed, mopt->count))
			goto err;
		for (uint32_t j = 0; j < transformed->size; ++j)
		{
			struct vec4f *dst = JKS_ARRAY_GET(transformed, j, struct vec4f);
			struct wow_vec3f *vec = JKS_ARRAY_GET(&group->parent->mopv, base + j, struct wow_vec3f);
			struct vec4f tmp;
			VEC4_SET(tmp, vec->x, vec->y, vec->z, 1);
			MAT4_VEC4_MUL(*dst, instance->m, tmp);
		}
		bool outside = false;
		for (size_t j = 0; j < frustums->size; ++j)
		{
			if (!frustum_check_points(JKS_ARRAY_GET(frustums, j, struct frustum), (struct vec4f*)transformed->data, transformed->size))
			{
				outside = true;
				break;
			}
		}
		if (outside)
		{
			instance->traversed_portals[portal / 8] &= ~(1 << (portal % 8));
			continue;
		}
		new_frustum = jks_array_grow(frustums, 1);
		if (!new_frustum)
			goto err;
		frustum_init(new_frustum);
		for (uint32_t j = 0; j < transformed->size; ++j)
		{
			struct vec4f *t1 = JKS_ARRAY_GET(transformed, j, struct vec4f);
			struct vec4f *t2 = JKS_ARRAY_GET(transformed, (j + 1) % transformed->size, struct vec4f);
			struct vec3f p1 = {t1->x, t1->y, t1->z};
			struct vec3f p2 = {t2->x, t2->y, t2->z};
			if (mopr->side < 0)
			{
				if (!frustum_add_plane(new_frustum, frame->cull_pos, p1, p2))
					goto err;
			}
			else
			{
				if (!frustum_add_plane(new_frustum, frame->cull_pos, p2, p1))
					goto err;
			}
		}
		cull_portal_rec(*JKS_ARRAY_GET(&group->parent->groups, mopr->group_index, struct gx_wmo_group*), instance, frame, frustums, rpos, transformed);
		if (!jks_array_resize(frustums, frustums->size - 1))
			goto err;
		instance->traversed_portals[portal / 8] &= ~(1 << (portal % 8));
	}
	ret = true;
err:
	return ret;
}

void gx_wmo_group_cull_portal(struct gx_wmo_group *group, struct gx_wmo_instance *instance, struct gx_frame *frame, struct vec4f rpos)
{
	if (!gx_wmo_group_flag_get(group, GX_WMO_GROUP_FLAG_LOADED))
		return;
	struct jks_array frustums;
	jks_array_init(&frustums, sizeof(frustum_t), (jks_array_destructor_t)frustum_destroy, &jks_array_memory_fn_GX);
	frustum_t *new_frustum = jks_array_grow(&frustums, 1);
	if (!new_frustum)
		return;
	frustum_init(new_frustum);
	if (!frustum_copy(new_frustum, &frame->frustum))
	{
		jks_array_destroy(&frustums);
		return;
	}
	struct jks_array transformed; /* struct vec4f */
	jks_array_init(&transformed, sizeof(struct vec4f), NULL, &jks_array_memory_fn_GX);
	if (!cull_portal_rec(group, instance, frame, &frustums, rpos, &transformed))
		LOG_INFO("cull failed");
	jks_array_destroy(&transformed);
	jks_array_destroy(&frustums);
}

void gx_wmo_group_set_m2_lighting(struct gx_wmo_group *group, struct gx_wmo_instance *instance)
{
	if (!(group->wow_flags & WOW_MOGP_FLAGS_INDOOR)
	 || (group->wow_flags & WOW_MOGP_FLAGS_EXT_LIGHT)
	 || (group->wow_flags & WOW_MOGP_FLAGS_OUTDOOR))
	{
		cache_lock_m2(g_wow->cache); /* XXX change this mutex */
		for (size_t i = 0; i < group->doodads.size; ++i)
		{
			uint16_t doodad = *JKS_ARRAY_GET(&group->doodads, i, uint16_t);
			if (doodad < instance->doodad_start || doodad >= instance->doodad_end)
				continue;
			struct gx_m2_instance *m2 = *JKS_ARRAY_GET(&instance->m2, doodad - instance->doodad_start, struct gx_m2_instance*);
			m2->lighting_type = GX_M2_LIGHTING_WMO_OUTDOOR;
		}
		cache_unlock_m2(g_wow->cache);
		return;
	}
	cache_lock_m2(g_wow->cache); /* XXX change this mutex */
	for (size_t i = 0; i < group->doodads.size; ++i)
	{
		uint16_t doodad = *JKS_ARRAY_GET(&group->doodads, i, uint16_t);
		if (doodad < instance->doodad_start || doodad >= instance->doodad_end)
			continue;
		struct wow_modd_data *modd = JKS_ARRAY_GET(&instance->parent->modd, doodad - instance->doodad_start, struct wow_modd_data);
		struct gx_m2_instance *m2 = *JKS_ARRAY_GET(&instance->m2, doodad - instance->doodad_start, struct gx_m2_instance*);
		if (m2->lighting_type == GX_M2_LIGHTING_WMO_OUTDOOR)
			continue;
		struct wow_vec4b *color = &modd->color;
		struct gx_m2_lighting *loc = mem_malloc(MEM_GX, sizeof(*loc));
		if (!loc)
		{
			LOG_ERROR("failed to allocate m2 local lighting");
			continue;
		}
		loc->uniform_buffer = GFX_BUFFER_INIT();
		if (m2->local_lighting)
		{
			/* detect m2 which belongs to at least two groups */
			VEC4_SET(loc->diffuse_color, 0, 1, 0, 1);
			VEC4_SET(loc->light_direction, 1, -1, 1, 0);
		}
		else if (color->w == 0xFF)
		{
			struct vec3f group_center;
			VEC3_ADD(group_center, group->aabb.p0, group->aabb.p1);
			VEC3_MULV(group_center, group_center, .5f);
			VEC3_SUB(loc->light_direction, modd->position, group_center);
			VEC3_NORMALIZE(float, loc->light_direction, loc->light_direction);
			loc->light_direction.w = 0;
			VEC4_SET(loc->diffuse_color, color->z / 255., color->y / 255., color->x / 255., 1);
		}
		else
		{
			/* XXX use MOLT */
			VEC4_SET(loc->diffuse_color, 0, 1, 0, 1);
			VEC4_SET(loc->light_direction, 0, 1, 0, 0);
		}
		VEC4_SETV(loc->ambient_color, 0);
		/* m2->setAmbient((Vec4&)(group->parent.getAmbient()).rgb() * group->parent.getAmbient().a); */
		/*for (size_t i = 0; i < group->lights.size(); ++i)
		{
			MOLTData &light = group->parent.molt[group->lights[i]];
			m2->addLight(M2RendererLight((instance->M * Vec4(light.position, 1)).xyz(), Vec3(light.color.r / 255., light.color.g / 255., light.color.b / 255.) * (light.color.a / 255.f) * light.intensity, Vec2(light.attenStart, light.attenEnd)));
		}*/
		loc->lights_count = 0;
		if (m2->local_lighting)
		{
#if 0
			LOG_WARN("replacing m2 local lighting");
#endif
			gfx_delete_buffer(g_wow->device, &m2->local_lighting->uniform_buffer);
			mem_free(MEM_GX, m2->local_lighting);
		}
		m2->local_lighting = loc;
	}
	cache_unlock_m2(g_wow->cache);
}
