#include "gx/wmo_group.h"
#include "gx/wmo_mliq.h"
#include "gx/frame.h"
#include "gx/wmo.h"
#include "gx/m2.h"

#include "performance.h"
#include "shaders.h"
#include "camera.h"
#include "loader.h"
#include "memory.h"
#include "cache.h"
#include "log.h"
#include "wow.h"

#include <jks/quaternion.h>

#include <wow/wmo_group.h>
#include <wow/mpq.h>

#include <gfx/device.h>

#include <inttypes.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>

MEMORY_DECL(GX);

struct gx_wmo *gx_wmo_new(char *filename)
{
	struct gx_wmo *wmo = mem_zalloc(MEM_GX, sizeof(*wmo));
	if (!wmo)
		return NULL;
	refcount_init(&wmo->refcount, 1);
	wmo->filename = filename;
	for (size_t i = 0; i < RENDER_FRAMES_COUNT; ++i)
	{
		jks_array_init(&wmo->frames[i].to_render, sizeof(struct gx_wmo_instance*), NULL, &jks_array_memory_fn_GX);
		pthread_mutex_init(&wmo->frames[i].mutex, NULL);
	}
	jks_array_init(&wmo->instances, sizeof(struct gx_wmo_instance*), NULL, &jks_array_memory_fn_GX);
	jks_array_init(&wmo->groups, sizeof(struct gx_wmo_group*), NULL, &jks_array_memory_fn_GX);
	jks_array_init(&wmo->modd, sizeof(struct wow_modd_data), NULL, &jks_array_memory_fn_GX);
	jks_array_init(&wmo->mods, sizeof(struct wow_mods_data), NULL, &jks_array_memory_fn_GX);
	jks_array_init(&wmo->molt, sizeof(struct wow_molt_data), NULL, &jks_array_memory_fn_GX);
	jks_array_init(&wmo->mopt, sizeof(struct wow_mopt_data), NULL, &jks_array_memory_fn_GX);
	jks_array_init(&wmo->mopr, sizeof(struct wow_mopr_data), NULL, &jks_array_memory_fn_GX);
	jks_array_init(&wmo->momt, sizeof(struct wow_momt_data), NULL, &jks_array_memory_fn_GX);
	jks_array_init(&wmo->mopv, sizeof(struct wow_vec3f), NULL, &jks_array_memory_fn_GX);
	jks_array_init(&wmo->modn, sizeof(char), NULL, &jks_array_memory_fn_GX);
	jks_array_init(&wmo->motx, sizeof(char), NULL, &jks_array_memory_fn_GX);
#ifdef WITH_DEBUG_RENDERING
	gx_wmo_portals_init(&wmo->gx_portals);
	gx_wmo_lights_init(&wmo->gx_lights);
#endif
	return wmo;
}

static void wmo_unload_task(struct wow_mpq_compound *mpq_compound, void *userdata)
{
	(void)mpq_compound;
	struct gx_wmo *wmo = userdata;
	for (size_t i = 0; i < RENDER_FRAMES_COUNT; ++i)
	{
		jks_array_destroy(&wmo->frames[i].to_render);
		pthread_mutex_destroy(&wmo->frames[i].mutex);
	}
	jks_array_destroy(&wmo->instances);
	jks_array_destroy(&wmo->groups);
	jks_array_destroy(&wmo->modd);
	jks_array_destroy(&wmo->mods);
	jks_array_destroy(&wmo->molt);
	jks_array_destroy(&wmo->mopt);
	jks_array_destroy(&wmo->mopr);
	jks_array_destroy(&wmo->momt);
	jks_array_destroy(&wmo->mopv);
	jks_array_destroy(&wmo->modn);
	jks_array_destroy(&wmo->motx);
#ifdef WITH_DEBUG_RENDERING
	gx_wmo_portals_destroy(&wmo->gx_portals);
	gx_wmo_lights_destroy(&wmo->gx_lights);
#endif
	assert(!wmo->instances.size);
	mem_free(MEM_GENERIC, wmo->filename);
	mem_free(MEM_GX, wmo);
}

void gx_wmo_free(struct gx_wmo *wmo)
{
	if (!wmo)
		return;
	if (refcount_dec(&wmo->refcount))
		return;
	cache_lock_wmo(g_wow->cache);
	if (refcount_get(&wmo->refcount))
	{
		cache_unlock_wmo(g_wow->cache);
		return;
	}
	cache_unref_unmutexed_wmo(g_wow->cache, wmo->filename);
	cache_unlock_wmo(g_wow->cache);
	for (size_t i = 0; i < wmo->groups.size; ++i)
		gx_wmo_group_free(*JKS_ARRAY_GET(&wmo->groups, i, struct gx_wmo_group*));
	loader_push(g_wow->loader, ASYNC_TASK_WMO_UNLOAD, wmo_unload_task, wmo);
}

void gx_wmo_ref(struct gx_wmo *wmo)
{
	refcount_inc(&wmo->refcount);
}

static bool initialize(void *userdata)
{
	struct gx_wmo *wmo = userdata;
#ifdef WITH_DEBUG_RENDERING
	gx_wmo_portals_initialize(&wmo->gx_portals);
	gx_wmo_lights_initialize(&wmo->gx_lights);
#endif
	gx_wmo_flag_set(wmo, GX_WMO_FLAG_INITIALIZED);
	gx_wmo_free(wmo);
	return true;
}

static void load(struct gx_wmo *wmo, struct wow_wmo_file *file)
{
	wmo->wow_flags = file->mohd.flags;
	VEC4_SET(wmo->ambient,
	         file->mohd.ambient.z / 255.0,
	         file->mohd.ambient.y / 255.0,
	         file->mohd.ambient.x / 255.0,
	         file->mohd.ambient.w / 255.0);
	if (!jks_array_reserve(&wmo->groups, file->mohd.groups_nb))
	{
		LOG_ERROR("failed to resize mohd array");
		return;
	}
	if (file->modd.size)
	{
		if (!jks_array_resize(&wmo->modd, file->modd.size))
		{
			LOG_ERROR("failed to resize modd array");
			return;
		}
		memcpy(wmo->modd.data, file->modd.data, sizeof(*file->modd.data) * file->modd.size);
		for (size_t i = 0; i < file->modd.size; ++i)
		{
			struct wow_modd_data *modd = JKS_ARRAY_GET(&wmo->modd, i, struct wow_modd_data);
			modd->position = (struct wow_vec3f){modd->position.x, modd->position.z, -modd->position.y};
			modd->rotation = (struct wow_quatf){modd->rotation.x, modd->rotation.z, -modd->rotation.y, modd->rotation.w};
		}
	}
	if (file->mods.size)
	{
		if (!jks_array_resize(&wmo->mods, file->mods.size))
		{
			LOG_ERROR("failled to resize mods array");
			return;
		}
		memcpy(wmo->mods.data, file->mods.data, sizeof(*file->mods.data) * file->mods.size);
	}
	if (file->modn.size)
	{
		if (!jks_array_resize(&wmo->modn, file->modn.size))
		{
			LOG_ERROR("failed to resize modn array");
			return;
		}
		memcpy(wmo->modn.data, file->modn.data, sizeof(*file->modn.data) * file->modn.size);
	}
	if (file->molt.size)
	{
		if (!jks_array_resize(&wmo->molt, file->molt.size))
		{
			LOG_ERROR("failed to resize molt array");
			return;
		}
		memcpy(wmo->molt.data, file->molt.data, sizeof(*file->molt.data) * file->molt.size);
		for (size_t i = 0; i < wmo->molt.size; ++i)
		{
			struct wow_molt_data *molt = JKS_ARRAY_GET(&wmo->molt, i, struct wow_molt_data);
			molt->position = (struct wow_vec3f){molt->position.x, molt->position.z, -molt->position.y};
			molt->color = (struct wow_vec4b){molt->color.z, molt->color.y, molt->color.x, molt->color.w};
		}
	}
	if (file->mopt.size)
	{
		if (!jks_array_resize(&wmo->mopt, file->mopt.size))
		{
			LOG_ERROR("failed to resize mopt array");
			return;
		}
		memcpy(wmo->mopt.data, file->mopt.data, sizeof(*file->mopt.data) * file->mopt.size);
		for (size_t i = 0; i < wmo->mopt.size; ++i)
		{
			struct wow_mopt_data *mopt = JKS_ARRAY_GET(&wmo->mopt, i, struct wow_mopt_data);
			mopt->normal = (struct wow_vec3f){mopt->normal.x, mopt->normal.z, -mopt->normal.y};
		}
	}
	if (file->momt.size)
	{
		if (!jks_array_resize(&wmo->momt, file->momt.size))
		{
			LOG_ERROR("failed to resize momt array");
			return;
		}
		memcpy(wmo->momt.data, file->momt.data, sizeof(*file->momt.data) * file->momt.size);
	}
	if (file->motx.size)
	{
		if (!jks_array_resize(&wmo->motx, file->motx.size))
		{
			LOG_ERROR("failed to resize motx array");
			return;
		}
		memcpy(wmo->motx.data, file->motx.data, sizeof(*file->motx.data) * file->motx.size);
	}
	if (file->mopv.size)
	{
		if (!jks_array_resize(&wmo->mopv, file->mopv.size))
		{
			LOG_ERROR("failed to resize mopv array");
			return;
		}
		memcpy(wmo->mopv.data, file->mopv.data, sizeof(*file->mopv.data) * file->mopv.size);
		for (size_t i = 0; i < wmo->mopv.size; ++i)
		{
			struct wow_vec3f *tmp = JKS_ARRAY_GET(&wmo->mopv, i, struct wow_vec3f);
			*tmp = (struct wow_vec3f){tmp->x, tmp->z, -tmp->y};
		}
	}
	if (file->mopr.size)
	{
		if (!jks_array_resize(&wmo->mopr, file->mopr.size))
		{
			LOG_ERROR("failed to resize mopr array");
			return;
		}
		memcpy(wmo->mopr.data, file->mopr.data, sizeof(*file->mopr.data) * file->mopr.size);
	}
	struct vec3f p0 = {file->mohd.aabb0.x, file->mohd.aabb0.z, -file->mohd.aabb0.y};
	struct vec3f p1 = {file->mohd.aabb1.x, file->mohd.aabb1.z, -file->mohd.aabb1.y};
	VEC3_MIN(wmo->aabb.p0, p0, p1);
	VEC3_MAX(wmo->aabb.p1, p0, p1);
	for (size_t i = 0; i < file->mohd.groups_nb; ++i)
	{
		struct gx_wmo_group *group = gx_wmo_group_new(wmo, i, file->mogi.data[i].flags);
		if (!group)
		{
			LOG_ERROR("failed to create wmo group");
			return;
		}
		if (!jks_array_push_back(&wmo->groups, &group))
		{
			LOG_ERROR("failed to push wmo group");
			return;
		}
		gx_wmo_group_ask_load(group);
	}
#ifdef WITH_DEBUG_RENDERING
	if (!gx_wmo_portals_load(&wmo->gx_portals, (struct wow_mopt_data*)wmo->mopt.data, wmo->mopt.size, (struct wow_vec3f*)wmo->mopv.data, wmo->mopv.size))
		return;
	if (!gx_wmo_lights_load(&wmo->gx_lights, (struct wow_molt_data*)wmo->molt.data, wmo->molt.size))
		return;
#endif
	cache_lock_wmo(g_wow->cache);
	for (size_t i = 0; i < wmo->instances.size; ++i)
		gx_wmo_instance_on_load(*JKS_ARRAY_GET(&wmo->instances, i, struct gx_wmo_instance*));
	gx_wmo_flag_set(wmo, GX_WMO_FLAG_LOADED);
	cache_unlock_wmo(g_wow->cache);
	gx_wmo_ref(wmo);
	loader_init_object(g_wow->loader, LOADER_WMO, initialize, wmo);
}

static void wmo_load_task(struct wow_mpq_compound *mpq_compound, void *userdata)
{
	struct gx_wmo *wmo = userdata;
	struct wow_mpq_file *mpq_file = NULL;
	struct wow_wmo_file *wmo_file = NULL;

	mpq_file = wow_mpq_get_file(mpq_compound, wmo->filename);
	if (!mpq_file)
	{
		LOG_WARN("WMO file not found: %s", wmo->filename);
		goto end;
	}
	wmo_file = wow_wmo_file_new(mpq_file);
	if (!wmo_file)
	{
		LOG_ERROR("failed to create wmo from file %s", wmo->filename);
		goto end;
	}
	load(wmo, wmo_file);

end:
	wow_mpq_file_delete(mpq_file);
	wow_wmo_file_delete(wmo_file);
	gx_wmo_free(wmo);
}

void gx_wmo_ask_load(struct gx_wmo *wmo)
{
	if (gx_wmo_flag_set(wmo, GX_WMO_FLAG_LOAD_ASKED))
		return;
	gx_wmo_ref(wmo);
	loader_push(g_wow->loader, ASYNC_TASK_WMO_LOAD, wmo_load_task, wmo);
}

void gx_wmo_clear_update(struct gx_wmo *wmo, struct gx_frame *frame)
{
	jks_array_resize(&wmo->frames[frame->id].to_render, 0);
}

void gx_wmo_render(struct gx_wmo *wmo, struct gx_frame *frame)
{
	if (!gx_wmo_flag_get(wmo, GX_WMO_FLAG_INITIALIZED))
		return;
	struct gx_wmo_frame *wmo_frame = &wmo->frames[frame->id];
	for (size_t i = 0; i < wmo_frame->to_render.size; ++i)
	{
		struct gx_wmo_instance *instance = *JKS_ARRAY_GET(&wmo_frame->to_render, i, struct gx_wmo_instance*);
		struct gx_wmo_instance_frame *instance_frame = &instance->frames[frame->id];
		PERFORMANCE_BEGIN(WMO_RENDER_DATA);
		struct shader_wmo_model_block model_block;
		model_block.v = *(struct mat4f*)&frame->view_v;
		model_block.mv = instance_frame->mv;
		model_block.mvp = instance_frame->mvp;
		if (!instance_frame->uniform_buffer.handle.u64)
			gfx_create_buffer(g_wow->device, &instance_frame->uniform_buffer, GFX_BUFFER_UNIFORM, NULL, sizeof(struct shader_wmo_model_block), GFX_BUFFER_STREAM);
		gfx_set_buffer_data(&instance_frame->uniform_buffer, &model_block, sizeof(model_block), 0);
		PERFORMANCE_END(WMO_RENDER_DATA);
	}
	for (size_t i = 0; i < wmo->groups.size; ++i)
		gx_wmo_group_render(*JKS_ARRAY_GET(&wmo->groups, i, struct gx_wmo_group*), frame, &wmo_frame->to_render);
}

#ifdef WITH_DEBUG_RENDERING
void gx_wmo_render_portals(struct gx_wmo *wmo, struct gx_frame *frame)
{
	if (!gx_wmo_flag_get(wmo, GX_WMO_FLAG_INITIALIZED) || !wmo->mopt.size)
		return;
	struct gx_wmo_frame *wmo_frame = &wmo->frames[frame->id];
	for (size_t i = 0; i < wmo_frame->to_render.size; ++i)
	{
		struct gx_wmo_instance *instance = *JKS_ARRAY_GET(&wmo_frame->to_render, i, struct gx_wmo_instance*);
		gx_wmo_portals_render(&wmo->gx_portals, frame, instance);
	}
}

void gx_wmo_render_lights(struct gx_wmo *wmo, struct gx_frame *frame)
{
	if (!gx_wmo_flag_get(wmo, GX_WMO_FLAG_INITIALIZED) || !wmo->molt.size)
		return;
	struct gx_wmo_frame *wmo_frame = &wmo->frames[frame->id];
	for (size_t i = 0; i < wmo_frame->to_render.size; ++i)
	{
		struct gx_wmo_instance *instance = *JKS_ARRAY_GET(&wmo_frame->to_render, i, struct gx_wmo_instance*);
		struct gx_wmo_instance_frame *instance_frame = &instance->frames[frame->id];
		gx_wmo_lights_update(&wmo->gx_lights, &instance_frame->mvp);
		gx_wmo_lights_render(&wmo->gx_lights, frame);
	}
}

void gx_wmo_render_collisions(struct gx_wmo *wmo, struct gx_frame *frame, bool triangles)
{
	if (!gx_wmo_flag_get(wmo, GX_WMO_FLAG_INITIALIZED))
		return;
	struct gx_wmo_frame *wmo_frame = &wmo->frames[frame->id];
	for (size_t i = 0; i < wmo->groups.size; ++i)
	{
		struct gx_wmo_group *group = *JKS_ARRAY_GET(&wmo->groups, i, struct gx_wmo_group*);
		if (!group || !gx_wmo_group_flag_get(group, GX_WMO_GROUP_FLAG_LOADED))
			continue;
		if (!(group->wow_flags & WOW_MOGP_FLAGS_BSP))
			continue;
		gx_wmo_collisions_render(&group->gx_collisions, frame, wmo_frame->to_render.data, wmo_frame->to_render.size, i, triangles);
	}
}
#endif

static void check_frustum(struct gx_wmo *wmo, struct gx_wmo_instance *instance, struct gx_frame *frame)
{
	if (!gx_wmo_flag_get(wmo, GX_WMO_FLAG_INITIALIZED))
		return;
	struct vec4f tmp;
	VEC3_CPY(tmp, frame->cull_pos);
	tmp.w = 1;
	struct vec4f rpos;
	MAT4_VEC4_MUL(rpos, instance->m_inv, tmp);
	struct vec3f p3 = {rpos.x, rpos.y, rpos.z};
	for (size_t i = 0; i < wmo->groups.size; ++i)
	{
		struct gx_wmo_group_instance *group_instance = JKS_ARRAY_GET(&instance->groups, i, struct gx_wmo_group_instance);
		struct gx_wmo_group_instance_frame *group_instance_frame = &group_instance->frames[frame->id];
		struct gx_wmo_group *group = *JKS_ARRAY_GET(&wmo->groups, i, struct gx_wmo_group*);
		if (!gx_wmo_group_flag_get(group, GX_WMO_GROUP_FLAG_LOADED))
		{
			group_instance_frame->culled = true;
			continue;
		}
		if (group->wow_flags & WOW_MOGP_FLAGS_INDOOR)
		{
			if (!aabb_contains(&group->aabb, p3))
			{
				group_instance_frame->culled = true;
				continue;
			}
		}
		group_instance_frame->culled = !frustum_check_fast(&instance->frustum, &group->aabb);
		if (group_instance_frame->culled)
			continue;
		group_instance_frame->cull_source = true;
		for (size_t j = 0; j < group_instance->batches.size; ++j)
		{
			struct gx_wmo_batch_instance *batch = JKS_ARRAY_GET(&group_instance->batches, j, struct gx_wmo_batch_instance);
			batch->frames[frame->id].culled = true;
		}
	}
}

void gx_wmo_batch_instance_init(struct gx_wmo_batch_instance *instance)
{
#ifdef WITH_DEBUG_RENDERING
	gx_aabb_init(&instance->gx_aabb, (struct vec4f){1, 0, 0, 1}, 1);
#endif
	for (size_t i = 0; i < sizeof(instance->frames) / sizeof(*instance->frames); ++i)
		instance->frames[i].culled = true;
}

void gx_wmo_batch_instance_destroy(struct gx_wmo_batch_instance *instance)
{
	(void)instance;
#ifdef WITH_DEBUG_RENDERING
	gx_aabb_destroy(&instance->gx_aabb);
#endif
}

void gx_wmo_group_instance_init(struct gx_wmo_group_instance *instance)
{
	jks_array_init(&instance->batches, sizeof(struct gx_wmo_batch_instance), (jks_array_destructor_t)gx_wmo_batch_instance_destroy, &jks_array_memory_fn_GX);
	for (size_t i = 0; i < RENDER_FRAMES_COUNT; ++i)
	{
		struct gx_wmo_group_instance_frame *instance_frame = &instance->frames[i];
		instance_frame->culled = true;
		instance_frame->cull_source = false;
	}
#ifdef WITH_DEBUG_RENDERING
	gx_aabb_init(&instance->gx_aabb, (struct vec4f){0, 0, 0, 0}, 1);
#endif
}

void gx_wmo_group_instance_destroy(struct gx_wmo_group_instance *instance)
{
	jks_array_destroy(&instance->batches);
#ifdef WITH_DEBUG_RENDERING
	gx_aabb_destroy(&instance->gx_aabb);
#endif
}

bool gx_wmo_group_instance_on_load(struct gx_wmo_instance *instance, struct gx_wmo_group *group, struct gx_wmo_group_instance *group_instance)
{
	gx_wmo_group_set_m2_lighting(group, instance);
	gx_wmo_group_instance_init(group_instance);
	if (!jks_array_resize(&group_instance->batches, group->batches.size))
	{
		LOG_ERROR("failed to resize wmo group instance batches array");
		return true;
	}
	for (size_t k = 0; k < group_instance->batches.size; ++k)
		gx_wmo_batch_instance_init(JKS_ARRAY_GET(&group_instance->batches, k, struct gx_wmo_batch_instance));
	group_instance->aabb = group->aabb;
	aabb_transform(&group_instance->aabb, &instance->m);
#ifdef WITH_DEBUG_RENDERING
	gx_aabb_set_aabb(&group_instance->gx_aabb, &group_instance->aabb);
#endif
	for (size_t j = 0; j < group_instance->batches.size; ++j)
	{
		struct gx_wmo_batch_instance *batch_instance = JKS_ARRAY_GET(&group_instance->batches, j, struct gx_wmo_batch_instance);
		struct gx_wmo_batch *batch = JKS_ARRAY_GET(&group->batches, j, struct gx_wmo_batch);
		batch_instance->aabb = batch->aabb;
		aabb_transform(&batch_instance->aabb, &instance->m);
#ifdef WITH_DEBUG_RENDERING
		gx_aabb_set_aabb(&batch_instance->gx_aabb, &batch_instance->aabb);
#endif
	}
	return true;
}

static void delete_m2_instance(struct gx_m2_instance **instance)
{
	gx_m2_instance_free(*instance);
}

struct gx_wmo_instance *gx_wmo_instance_new(const char *filename)
{
	struct gx_wmo_instance *instance = mem_zalloc(MEM_GX, sizeof(*instance));
	if (!instance)
		return NULL;
	refcount_init(&instance->refcount, 1);
	instance->traversed_portals = NULL;
	for (size_t i = 0; i < RENDER_FRAMES_COUNT; ++i)
	{
		instance->frames[i].uniform_buffer = GFX_BUFFER_INIT();
		instance->frames[i].culled = true;
	}
	jks_array_init(&instance->groups, sizeof(struct gx_wmo_group_instance), (jks_array_destructor_t)gx_wmo_group_instance_destroy, &jks_array_memory_fn_GX);
	jks_array_init(&instance->m2, sizeof(struct gx_m2_instance*), (jks_array_destructor_t)delete_m2_instance, &jks_array_memory_fn_GX);
	frustum_init(&instance->frustum);
#ifdef WITH_DEBUG_RENDERING
	gx_aabb_init(&instance->gx_aabb, (struct vec4f){0, 0, 1, 1}, 2);
#endif
	cache_lock_wmo(g_wow->cache);
	if (!cache_ref_unmutexed_wmo(g_wow->cache, filename, &instance->parent))
	{
		cache_unlock_wmo(g_wow->cache);
		goto err;
	}
	if (!jks_array_push_back(&instance->parent->instances, &instance))
	{
		gx_wmo_free(instance->parent);
		cache_unlock_wmo(g_wow->cache);
		goto err;
	}
	cache_unlock_wmo(g_wow->cache);
	return instance;

err:
	return NULL;
}

static void instance_delete_task(struct wow_mpq_compound *mpq_compound, void *userdata)
{
	(void)mpq_compound;
	struct gx_wmo_instance *instance = userdata;
	frustum_destroy(&instance->frustum);
	for (size_t i = 0; i < RENDER_FRAMES_COUNT; ++i)
		gfx_delete_buffer(g_wow->device, &instance->frames[i].uniform_buffer);
	jks_array_destroy(&instance->groups);
	jks_array_destroy(&instance->m2);
#ifdef WITH_DEBUG_RENDERING
	gx_aabb_destroy(&instance->gx_aabb);
#endif
	mem_free(MEM_GX, instance->traversed_portals);
	mem_free(MEM_GX, instance);
}

void gx_wmo_instance_free(struct gx_wmo_instance *instance)
{
	if (!instance)
		return;
	if (refcount_dec(&instance->refcount))
		return;
	{
		cache_lock_wmo(g_wow->cache);
		for (size_t i = 0; i < instance->parent->instances.size; ++i)
		{
			struct gx_wmo_instance *tmp = *JKS_ARRAY_GET(&instance->parent->instances, i, struct gx_wmo_instance*);
			if (tmp != instance)
				continue;
			jks_array_erase(&instance->parent->instances, i);
			break;
		}
		cache_unlock_wmo(g_wow->cache);
	}
	gx_wmo_free(instance->parent);
	loader_push(g_wow->loader, ASYNC_TASK_WMO_INSTANCE_DELETE, instance_delete_task, instance);
}

void gx_wmo_instance_ref(struct gx_wmo_instance *instance)
{
	refcount_inc(&instance->refcount);
}

void gx_wmo_instance_on_load(struct gx_wmo_instance *instance)
{
	gx_wmo_instance_load_doodad_set(instance);
	instance->traversed_portals = mem_zalloc(MEM_GX, (instance->parent->mopr.size + 7) / 8);
	if (!instance->traversed_portals)
		LOG_ERROR("failed to allocate traversed portals array");
	if (!jks_array_resize(&instance->groups, instance->parent->groups.size))
		LOG_ERROR("failed to allocate groups array");
	for (size_t i = 0; i < instance->parent->groups.size; ++i)
	{
		struct gx_wmo_group_instance *group_instance = JKS_ARRAY_GET(&instance->groups, i, struct gx_wmo_group_instance);
		struct gx_wmo_group *group = *JKS_ARRAY_GET(&instance->parent->groups, i, struct gx_wmo_group*);
		gx_wmo_group_instance_init(group_instance);
		if (gx_wmo_group_flag_get(group, GX_WMO_GROUP_FLAG_LOADED))
			gx_wmo_group_instance_on_load(instance, group, group_instance);
	}
	gx_wmo_instance_update_aabb(instance);
}

void gx_wmo_instance_load_doodad_set(struct gx_wmo_instance *instance)
{
	if (instance->doodad_set >= instance->parent->mods.size)
	{
		LOG_WARN("invalid doodad set: %" PRIu32 " / %" PRIu32 " (%s)", (uint32_t)instance->doodad_set, (uint32_t)instance->parent->mods.size, instance->parent->filename);
		return;
	}
	struct wow_mods_data *mods = JKS_ARRAY_GET(&instance->parent->mods, instance->doodad_set, struct wow_mods_data);
	instance->doodad_start = mods->start_index;
	instance->doodad_end = mods->start_index + mods->count;
	for (size_t i = mods->start_index; i < mods->start_index + mods->count; ++i)
	{
		struct wow_modd_data *modd = JKS_ARRAY_GET(&instance->parent->modd, i, struct wow_modd_data);
		char filename[512];
		snprintf(filename, sizeof(filename), "%s", JKS_ARRAY_GET(&instance->parent->modn, modd->name_flags & WOW_MODD_NAME_MASK, char));
		if (!filename[0])
			continue;
		wow_mpq_normalize_m2_fn(filename, sizeof(filename));
		struct gx_m2_instance *m2 = gx_m2_instance_new_filename(filename);
		struct mat4f mat1;
		struct mat4f mat2;
		MAT4_TRANSLATE(mat1, instance->m, modd->position);
		struct mat4f quat;
		QUATERNION_TO_MAT4(float, quat, modd->rotation);
		MAT4_MUL(mat2, mat1, quat);
		MAT4_SCALEV(mat1, mat2, modd->scale);
		{
			cache_lock_m2(g_wow->cache);
			m2->scale = modd->scale;
			gx_m2_instance_set_mat(m2, &mat1);
			struct vec4f tmp1 = {modd->position.x, modd->position.y, modd->position.z, 1};
			struct vec4f tmp3;
			MAT4_VEC4_MUL(tmp3, instance->m, tmp1);
			VEC3_CPY(m2->pos, tmp3);
			if (gx_m2_flag_get(m2->parent, GX_M2_FLAG_LOADED))
				gx_m2_instance_on_parent_loaded(m2);
			else
				gx_m2_ask_load(m2->parent);
			cache_unlock_m2(g_wow->cache);
		}
		if (!jks_array_push_back(&instance->m2, &m2))
			LOG_ERROR("failed to add m2 to wmo m2 list");
	}
}

void gx_wmo_instance_update(struct gx_wmo_instance *instance, struct gx_frame *frame, bool bypass_frustum)
{
	struct gx_wmo_instance_frame *instance_frame = &instance->frames[frame->id];
	if (!gx_wmo_flag_get(instance->parent, GX_WMO_FLAG_LOADED))
	{
		instance_frame->culled = true;
		return;
	}
	if (!bypass_frustum)
	{
		if (!frustum_check_fast(&frame->frustum, &instance->aabb))
		{
			instance_frame->culled = true;
			return;
		}
	}
	struct gx_wmo_frame *wmo_frame = &instance->parent->frames[frame->id];
	pthread_mutex_lock(&wmo_frame->mutex);
	if (!jks_array_push_back(&wmo_frame->to_render, &instance))
		LOG_ERROR("failed to add wmo instance to parent render list");
	pthread_mutex_unlock(&wmo_frame->mutex);
	instance_frame->culled = false;
	MAT4_MUL(instance_frame->mv, frame->view_v, instance->m);
	MAT4_MUL(instance_frame->mvp, frame->view_p, instance_frame->mv);
	if (g_wow->view_camera != g_wow->frustum_camera)
	{
		struct mat4f tmp1;
		struct mat4f tmp2;
		MAT4_MUL(tmp1, frame->cull_v, instance->m);
		MAT4_MUL(tmp2, frame->cull_p, tmp1);
		if (!frustum_update(&instance->frustum, &tmp2))
			LOG_ERROR("failed to update frustum");
	}
	else
	{
		if (!frustum_update(&instance->frustum, &instance_frame->mvp))
			LOG_ERROR("failed to update frustum");
	}
	check_frustum(instance->parent, instance, frame);
}

static void gx_wmo_instance_cull_portal(struct gx_wmo_instance *instance, struct gx_frame *frame)
{
	struct gx_wmo *wmo = instance->parent;
	memset(instance->traversed_portals, 0, (wmo->mopr.size + 7) / 8);
	struct vec4f tmp;
	VEC3_CPY(tmp, frame->cull_pos);
	tmp.w = 1;
	struct vec4f rpos;
	MAT4_VEC4_MUL(rpos, instance->m_inv, tmp);
	for (size_t i = 0; i < wmo->groups.size; ++i)
	{
		struct gx_wmo_group *group = *JKS_ARRAY_GET(&wmo->groups, i, struct gx_wmo_group*);
		struct gx_wmo_group_instance *group_instance = JKS_ARRAY_GET(&instance->groups, i, struct gx_wmo_group_instance);
		if ((group->wow_flags & WOW_MOGP_FLAGS_INDOOR)
		 && (group_instance->frames[frame->id].culled
		  || !group_instance->frames[frame->id].cull_source))
			continue;
		group_instance->frames[frame->id].cull_source = !(group->wow_flags & WOW_MOGP_FLAGS_INDOOR);
		gx_wmo_group_cull_portal(group, instance, frame, rpos);
	}
}

void gx_wmo_instance_calculate_distance_to_camera(struct gx_wmo_instance *instance, struct gx_frame *frame)
{
	struct vec3f tmp;
	VEC3_SUB(tmp, instance->pos, frame->cull_pos);
	instance->frames[frame->id].distance_to_camera = VEC3_NORM(tmp);
}

void gx_wmo_instance_add_to_render(struct gx_wmo_instance *instance, struct gx_frame *frame, bool bypass_frustum)
{
	if (gx_wmo_instance_flag_set(instance, GX_WMO_INSTANCE_FLAG_IN_RENDER_LIST))
		return;
	gx_wmo_instance_ref(instance);
	gx_frame_add_wmo_backref(frame, instance);
	gx_wmo_instance_calculate_distance_to_camera(instance, frame);
	gx_wmo_instance_update(instance, frame, bypass_frustum);
	if (instance->frames[frame->id].culled)
		return;
	if (!gx_wmo_flag_set(instance->parent, GX_WMO_FLAG_IN_RENDER_LIST))
		gx_frame_add_wmo(frame, instance->parent);
#ifdef WITH_DEBUG_RENDERING
	if (g_wow->render_opt & RENDER_OPT_WMO_AABB)
		gx_aabb_add_to_render(&instance->gx_aabb, frame, &frame->view_vp);
#endif
	for (size_t i = 0; i < instance->parent->groups.size; ++i)
	{
		struct gx_wmo_group *group = *JKS_ARRAY_GET(&instance->parent->groups, i, struct gx_wmo_group*);
		if (!group || !gx_wmo_group_flag_get(group, GX_WMO_GROUP_FLAG_LOADED))
			continue;
		if (group->gx_mliq)
			gx_wmo_mliq_add_to_render(group->gx_mliq, instance, frame);
#ifdef WITH_DEBUG_RENDERING
		struct gx_wmo_group_instance *group_instance = JKS_ARRAY_GET(&instance->groups, i, struct gx_wmo_group_instance);
		if (!group_instance || group_instance->frames[frame->id].culled)
			continue;
		if (g_wow->render_opt & RENDER_OPT_WMO_AABB)
		{
			if (group->wow_flags & WOW_MOGP_FLAGS_INDOOR)
				gx_aabb_set_color(&group_instance->gx_aabb, &(struct vec4f){0, 1, 1, 1});
			else
				gx_aabb_set_color(&group_instance->gx_aabb, &(struct vec4f){0, 1, 0, 1});
			gx_aabb_add_to_render(&group_instance->gx_aabb, frame, &frame->view_vp);
			for (size_t j = 0; j < group_instance->batches.size; ++j)
			{
				struct gx_wmo_batch_instance *batch_instance = JKS_ARRAY_GET(&group_instance->batches, j, struct gx_wmo_batch_instance);
				gx_aabb_add_to_render(&batch_instance->gx_aabb, frame, &frame->view_vp);
			}
		}
#endif
	}
	PERFORMANCE_BEGIN(WMO_PORTALS_CULL);
	gx_wmo_instance_cull_portal(instance, frame);
	PERFORMANCE_END(WMO_PORTALS_CULL);
}

void gx_wmo_instance_set_mat(struct gx_wmo_instance *instance, const struct mat4f *m)
{
	instance->m = *m;
	MAT4_INVERSE(float, instance->m_inv, instance->m);
	if (gx_wmo_flag_get(instance->parent, GX_WMO_FLAG_LOADED))
		gx_wmo_instance_update_aabb(instance);
}

void gx_wmo_instance_update_aabb(struct gx_wmo_instance *instance)
{
	instance->aabb = instance->parent->aabb;
	aabb_transform(&instance->aabb, &instance->m);
#ifdef WITH_DEBUG_RENDERING
	gx_aabb_set_aabb(&instance->gx_aabb, &instance->aabb);
#endif
	for (size_t i = 0; i < instance->groups.size; ++i)
	{
		struct gx_wmo_group *group = *JKS_ARRAY_GET(&instance->parent->groups, i, struct gx_wmo_group*);
		if (!gx_wmo_group_flag_get(group, GX_WMO_GROUP_FLAG_LOADED))
			continue;
		struct gx_wmo_group_instance *group_instance = JKS_ARRAY_GET(&instance->groups, i, struct gx_wmo_group_instance);
		group_instance->aabb = group->aabb;
		aabb_transform(&group_instance->aabb, &instance->m);
#ifdef WITH_DEBUG_RENDERING
		gx_aabb_set_aabb(&group_instance->gx_aabb, &group_instance->aabb);
#endif
		for (size_t j = 0; j < group_instance->batches.size; ++j)
		{
			struct gx_wmo_batch_instance *batch_instance = JKS_ARRAY_GET(&group_instance->batches, j, struct gx_wmo_batch_instance);
			struct gx_wmo_batch *batch = JKS_ARRAY_GET(&group->batches, j, struct gx_wmo_batch);
			batch_instance->aabb = batch->aabb;
			aabb_transform(&batch_instance->aabb, &instance->m);
#ifdef WITH_DEBUG_RENDERING
			gx_aabb_set_aabb(&batch_instance->gx_aabb, &batch_instance->aabb);
#endif
		}
	}
}
