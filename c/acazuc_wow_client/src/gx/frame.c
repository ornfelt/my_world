#include "ppe/render_target.h"

#include "gx/wmo_mliq.h"
#include "gx/skybox.h"
#include "gx/frame.h"
#include "gx/mcnk.h"
#include "gx/mclq.h"
#include "gx/text.h"
#include "gx/wmo.h"
#include "gx/blp.h"
#include "gx/m2.h"
#include "gx/gx.h"

#include "map/tile.h"
#include "map/map.h"

#include "shaders.h"
#include "camera.h"
#include "memory.h"
#include "loader.h"
#include "log.h"
#include "wow.h"

#include <gfx/device.h>

#include <stdlib.h>
#include <assert.h>
#include <math.h>

MEMORY_DECL(GX);

static const struct vec4f
light_direction = {-1.0f, -1.5f, 1.0f, 0.0f};

static void
list_init(struct gx_frame_list *list, size_t element_size)
{
	jks_array_init(&list->entries, element_size, NULL, &jks_array_memory_fn_GX);
	pthread_mutex_init(&list->mutex, NULL);
}

static void
list_destroy(struct gx_frame_list *list)
{
	jks_array_destroy(&list->entries);
	pthread_mutex_destroy(&list->mutex);
}

static void
list_clear(struct gx_frame_list *list)
{
	pthread_mutex_lock(&list->mutex);
	jks_array_resize(&list->entries, 0);
	pthread_mutex_unlock(&list->mutex);
}

static void
list_push(struct gx_frame_list *list, void *entry)
{
	pthread_mutex_lock(&list->mutex);
	if (!jks_array_push_back(&list->entries, entry))
	{
		LOG_ERROR("failed to add entry to list");
		abort();
		return;
	}
	pthread_mutex_unlock(&list->mutex);
}

void
gx_frame_init(struct gx_frame *frame, int id)
{
	for (size_t i = 0; i < sizeof(frame->render_lists.wmo_mliq) / sizeof(*frame->render_lists.wmo_mliq); ++i)
		list_init(&frame->render_lists.wmo_mliq[i], sizeof(struct gx_wmo_mliq*));
	for (size_t i = 0; i < sizeof(frame->render_lists.mclq) / sizeof(*frame->render_lists.mclq); ++i)
		list_init(&frame->render_lists.mclq[i], sizeof(struct gx_mclq*));
	list_init(&frame->render_lists.wmo, sizeof(struct gx_wmo*));
	list_init(&frame->render_lists.m2_particles, sizeof(struct gx_m2_instance*));
	list_init(&frame->render_lists.m2_ribbons, sizeof(struct gx_m2_instance*));
	list_init(&frame->render_lists.m2_transparent, sizeof(struct gx_m2_instance*));
	list_init(&frame->render_lists.m2_opaque, sizeof(struct gx_m2*));
	list_init(&frame->render_lists.m2_shadow, sizeof(struct gx_m2*));
	list_init(&frame->render_lists.m2_ground, sizeof(struct gx_m2_ground_batch*));
	list_init(&frame->render_lists.m2, sizeof(struct gx_m2*));
	list_init(&frame->render_lists.mcnk, sizeof(struct gx_mcnk*));
	list_init(&frame->render_lists.text, sizeof(struct gx_text*));
#ifdef WITH_DEBUG_RENDERING
	for (size_t i = 0; i < sizeof(frame->render_lists.aabb) / sizeof(*frame->render_lists.aabb); ++i)
	{
		list_init(&frame->render_lists.aabb[i].aabbs, sizeof(struct gx_aabb*));
		gx_aabb_batch_init(&frame->render_lists.aabb[i].batch, i + 1);
	}
#endif
	list_init(&frame->backrefs.m2, sizeof(struct gx_m2_instance*));
	list_init(&frame->backrefs.wmo, sizeof(struct gx_wmo_instance*));
	list_init(&frame->backrefs.wmo_mliq, sizeof(struct gx_wmo_mliq*));
	list_init(&frame->backrefs.tiles, sizeof(struct map_tile*));
	frame->m2_ground_shadow_uniform_buffer = GFX_BUFFER_INIT();
	frame->m2_ground_light_uniform_buffer = GFX_BUFFER_INIT();
	frame->m2_shadow_uniform_buffer = GFX_BUFFER_INIT();
	frame->m2_world_uniform_buffer = GFX_BUFFER_INIT();
	frame->particle_uniform_buffer = GFX_BUFFER_INIT();
	frame->ribbon_uniform_buffer = GFX_BUFFER_INIT();
	frame->river_uniform_buffer = GFX_BUFFER_INIT();
	frame->ocean_uniform_buffer = GFX_BUFFER_INIT();
	frame->magma_uniform_buffer = GFX_BUFFER_INIT();
	frame->mcnk_uniform_buffer = GFX_BUFFER_INIT();
	frame->mliq_uniform_buffer = GFX_BUFFER_INIT();
	frame->wmo_uniform_buffer = GFX_BUFFER_INIT();
	gfx_create_buffer(g_wow->device,
	                  &frame->m2_ground_shadow_uniform_buffer,
	                  GFX_BUFFER_UNIFORM,
	                  NULL,
	                  sizeof(struct shader_m2_scene_block),
	                  GFX_BUFFER_STREAM);
	gfx_create_buffer(g_wow->device,
	                  &frame->m2_ground_light_uniform_buffer,
	                  GFX_BUFFER_UNIFORM,
	                  NULL,
	                  sizeof(struct shader_m2_scene_block),
	                  GFX_BUFFER_STREAM);
	gfx_create_buffer(g_wow->device,
	                  &frame->m2_shadow_uniform_buffer,
	                  GFX_BUFFER_UNIFORM,
	                  NULL,
	                  sizeof(struct shader_m2_scene_block),
	                  GFX_BUFFER_STREAM);
	gfx_create_buffer(g_wow->device,
	                  &frame->m2_world_uniform_buffer,
	                  GFX_BUFFER_UNIFORM,
	                  NULL,
	                  sizeof(struct shader_m2_scene_block),
	                  GFX_BUFFER_STREAM);
	gfx_create_buffer(g_wow->device,
	                  &frame->particle_uniform_buffer,
	                  GFX_BUFFER_UNIFORM,
	                  NULL,
	                  sizeof(struct shader_particle_scene_block),
	                  GFX_BUFFER_STREAM);
	gfx_create_buffer(g_wow->device,
	                  &frame->ribbon_uniform_buffer,
	                  GFX_BUFFER_UNIFORM,
	                  NULL,
	                  sizeof(struct shader_ribbon_scene_block),
	                  GFX_BUFFER_STREAM);
	gfx_create_buffer(g_wow->device,
	                  &frame->river_uniform_buffer,
	                  GFX_BUFFER_UNIFORM,
	                  NULL,
	                  sizeof(struct shader_mclq_water_scene_block),
	                  GFX_BUFFER_STREAM);
	gfx_create_buffer(g_wow->device,
	                  &frame->ocean_uniform_buffer,
	                  GFX_BUFFER_UNIFORM,
	                  NULL,
	                  sizeof(struct shader_mclq_water_scene_block),
	                  GFX_BUFFER_STREAM);
	gfx_create_buffer(g_wow->device,
	                  &frame->magma_uniform_buffer,
	                  GFX_BUFFER_UNIFORM,
	                  NULL,
	                  sizeof(struct shader_mclq_magma_scene_block),
	                  GFX_BUFFER_STREAM);
	gfx_create_buffer(g_wow->device,
	                  &frame->mcnk_uniform_buffer,
	                  GFX_BUFFER_UNIFORM,
	                  NULL,
	                  sizeof(struct shader_mcnk_scene_block),
	                  GFX_BUFFER_STREAM);
	gfx_create_buffer(g_wow->device,
	                  &frame->mliq_uniform_buffer,
	                  GFX_BUFFER_UNIFORM,
	                  NULL,
	                  sizeof(struct shader_mliq_scene_block),
	                  GFX_BUFFER_STREAM);
	gfx_create_buffer(g_wow->device,
	                  &frame->wmo_uniform_buffer,
	                  GFX_BUFFER_UNIFORM,
	                  NULL,
	                  sizeof(struct shader_wmo_scene_block),
	                  GFX_BUFFER_STREAM);
#ifdef WITH_DEBUG_RENDERING
	gx_collisions_init(&frame->gx_collisions);
#endif
	frustum_init(&frame->shadow_frustum);
	frustum_init(&frame->wdl_frustum);
	frustum_init(&frame->frustum);
	frame->id = id;
}

void
gx_frame_destroy(struct gx_frame *frame)
{
#ifdef WITH_DEBUG_RENDERING
	gx_collisions_destroy(&frame->gx_collisions);
#endif
	gfx_delete_buffer(g_wow->device, &frame->m2_ground_shadow_uniform_buffer);
	gfx_delete_buffer(g_wow->device, &frame->m2_ground_light_uniform_buffer);
	gfx_delete_buffer(g_wow->device, &frame->m2_shadow_uniform_buffer);
	gfx_delete_buffer(g_wow->device, &frame->m2_world_uniform_buffer);
	gfx_delete_buffer(g_wow->device, &frame->particle_uniform_buffer);
	gfx_delete_buffer(g_wow->device, &frame->ribbon_uniform_buffer);
	gfx_delete_buffer(g_wow->device, &frame->river_uniform_buffer);
	gfx_delete_buffer(g_wow->device, &frame->ocean_uniform_buffer);
	gfx_delete_buffer(g_wow->device, &frame->magma_uniform_buffer);
	gfx_delete_buffer(g_wow->device, &frame->mcnk_uniform_buffer);
	gfx_delete_buffer(g_wow->device, &frame->mliq_uniform_buffer);
	gfx_delete_buffer(g_wow->device, &frame->wmo_uniform_buffer);
	for (size_t i = 0; i < sizeof(frame->render_lists.wmo_mliq) / sizeof(*frame->render_lists.wmo_mliq); ++i)
		list_destroy(&frame->render_lists.wmo_mliq[i]);
	for (size_t i = 0; i < sizeof(frame->render_lists.mclq) / sizeof(*frame->render_lists.mclq); ++i)
		list_destroy(&frame->render_lists.mclq[i]);
	list_destroy(&frame->render_lists.wmo);
	list_destroy(&frame->render_lists.m2_particles);
	list_destroy(&frame->render_lists.m2_ribbons);
	list_destroy(&frame->render_lists.m2_transparent);
	list_destroy(&frame->render_lists.m2_opaque);
	list_destroy(&frame->render_lists.m2_shadow);
	list_destroy(&frame->render_lists.m2_ground);
	list_destroy(&frame->render_lists.m2);
	list_destroy(&frame->render_lists.mcnk);
	list_destroy(&frame->render_lists.text);
#ifdef WITH_DEBUG_RENDERING
	for (size_t i = 0; i < sizeof(frame->render_lists.aabb) / sizeof(*frame->render_lists.aabb); ++i)
	{
		list_destroy(&frame->render_lists.aabb[i].aabbs);
		gx_aabb_batch_destroy(&frame->render_lists.aabb[i].batch);
	}
#endif
	list_destroy(&frame->backrefs.m2);
	list_destroy(&frame->backrefs.wmo);
	list_destroy(&frame->backrefs.wmo_mliq);
	list_destroy(&frame->backrefs.tiles);
	frustum_destroy(&frame->shadow_frustum);
	frustum_destroy(&frame->wdl_frustum);
	frustum_destroy(&frame->frustum);
}

#define COPY_VEC3_A1(dst, src) \
do \
{ \
	VEC3_CPY(dst, src); \
	dst.w = 1; \
} while (0)

static void
build_particle_uniform_buffer(struct gx_frame *frame)
{
	struct shader_particle_scene_block scene_block;
	scene_block.fog_range.y = frame->view_distance * g_wow->map->gx_skybox->float_values[SKYBOX_FLOAT_FOG_END] / 36 / g_wow->map->fog_divisor;
	scene_block.fog_range.x = scene_block.fog_range.y * g_wow->map->gx_skybox->float_values[SKYBOX_FLOAT_FOG_START];
	gfx_set_buffer_data(&frame->particle_uniform_buffer, &scene_block, sizeof(scene_block), 0);
}

static void
build_ribbon_uniform_buffer(struct gx_frame *frame)
{
	struct shader_ribbon_scene_block scene_block;
	scene_block.fog_range.y = frame->view_distance * g_wow->map->gx_skybox->float_values[SKYBOX_FLOAT_FOG_END] / 36 / g_wow->map->fog_divisor;
	scene_block.fog_range.x = scene_block.fog_range.y * g_wow->map->gx_skybox->float_values[SKYBOX_FLOAT_FOG_START];
	gfx_set_buffer_data(&frame->ribbon_uniform_buffer, &scene_block, sizeof(scene_block), 0);
}

static void
build_ocean_uniform_buffer(struct gx_frame *frame)
{
	struct shader_mclq_water_scene_block scene_block;
	VEC4_CPY(scene_block.light_direction, light_direction);
	COPY_VEC3_A1(scene_block.specular_color, g_wow->map->gx_skybox->int_values[SKYBOX_INT_SUN]);
	COPY_VEC3_A1(scene_block.diffuse_color, g_wow->map->gx_skybox->int_values[SKYBOX_INT_DIFFUSE]);
	COPY_VEC3_A1(scene_block.final_color, g_wow->map->gx_skybox->int_values[SKYBOX_INT_OCEAN2]);
	COPY_VEC3_A1(scene_block.base_color, g_wow->map->gx_skybox->int_values[SKYBOX_INT_OCEAN1]);
	COPY_VEC3_A1(scene_block.fog_color, g_wow->map->gx_skybox->int_values[SKYBOX_INT_FOG]);
	scene_block.alphas.x = g_wow->map->gx_skybox->ocean_min_alpha;
	scene_block.alphas.y = g_wow->map->gx_skybox->ocean_max_alpha;
	scene_block.fog_range.y = frame->view_distance * g_wow->map->gx_skybox->float_values[SKYBOX_FLOAT_FOG_END] / 36 / g_wow->map->fog_divisor;
	scene_block.fog_range.x = scene_block.fog_range.y * g_wow->map->gx_skybox->float_values[SKYBOX_FLOAT_FOG_START];
	scene_block.screen_size.x = g_wow->render_width;
	scene_block.screen_size.y = g_wow->render_height;
	scene_block.t = frame->time / 1000000000.0;
	gfx_set_buffer_data(&frame->ocean_uniform_buffer, &scene_block, sizeof(scene_block), 0);
}

static void
build_river_uniform_buffer(struct gx_frame *frame)
{
	struct shader_mclq_water_scene_block scene_block;
	VEC4_CPY(scene_block.light_direction, light_direction);
	COPY_VEC3_A1(scene_block.specular_color, g_wow->map->gx_skybox->int_values[SKYBOX_INT_SUN]);
	COPY_VEC3_A1(scene_block.diffuse_color, g_wow->map->gx_skybox->int_values[SKYBOX_INT_DIFFUSE]);
	COPY_VEC3_A1(scene_block.base_color, g_wow->map->gx_skybox->int_values[SKYBOX_INT_RIVER1]);
	COPY_VEC3_A1(scene_block.final_color, g_wow->map->gx_skybox->int_values[SKYBOX_INT_RIVER2]);
	COPY_VEC3_A1(scene_block.fog_color, g_wow->map->gx_skybox->int_values[SKYBOX_INT_FOG]);
	scene_block.alphas.x = g_wow->map->gx_skybox->river_min_alpha;
	scene_block.alphas.y = g_wow->map->gx_skybox->river_max_alpha;
	scene_block.fog_range.y = frame->view_distance * g_wow->map->gx_skybox->float_values[SKYBOX_FLOAT_FOG_END] / 36 / g_wow->map->fog_divisor;
	scene_block.fog_range.x = scene_block.fog_range.y * g_wow->map->gx_skybox->float_values[SKYBOX_FLOAT_FOG_START];
	scene_block.screen_size.x = g_wow->render_width;
	scene_block.screen_size.y = g_wow->render_height;
	scene_block.t = frame->time / 1000000000.0;
	gfx_set_buffer_data(&frame->river_uniform_buffer, &scene_block, sizeof(scene_block), 0);
}

static void
build_magma_uniform_buffer(struct gx_frame *frame)
{
	struct shader_mclq_magma_scene_block scene_block;
	COPY_VEC3_A1(scene_block.diffuse_color, g_wow->map->gx_skybox->int_values[SKYBOX_INT_DIFFUSE]);
	COPY_VEC3_A1(scene_block.fog_color, g_wow->map->gx_skybox->int_values[SKYBOX_INT_FOG]);
	scene_block.fog_range.y = frame->view_distance * g_wow->map->gx_skybox->float_values[SKYBOX_FLOAT_FOG_END] / 36 / g_wow->map->fog_divisor;
	scene_block.fog_range.x = scene_block.fog_range.y * g_wow->map->gx_skybox->float_values[SKYBOX_FLOAT_FOG_START];
	gfx_set_buffer_data(&frame->magma_uniform_buffer, &scene_block, sizeof(scene_block), 0);
}

static void
build_mcnk_uniform_buffer(struct gx_frame *frame)
{
	struct shader_mcnk_scene_block scene_block;
	VEC4_CPY(scene_block.light_direction, light_direction);
	COPY_VEC3_A1(scene_block.specular_color, g_wow->map->gx_skybox->int_values[SKYBOX_INT_SUN]);
	COPY_VEC3_A1(scene_block.ambient_color, g_wow->map->gx_skybox->int_values[SKYBOX_INT_AMBIENT]);
	COPY_VEC3_A1(scene_block.diffuse_color, g_wow->map->gx_skybox->int_values[SKYBOX_INT_DIFFUSE]);
	COPY_VEC3_A1(scene_block.fog_color, g_wow->map->gx_skybox->int_values[SKYBOX_INT_FOG]);
	scene_block.fog_range.y = frame->view_distance * g_wow->map->gx_skybox->float_values[SKYBOX_FLOAT_FOG_END] / 36 / g_wow->map->fog_divisor;
	scene_block.fog_range.x = scene_block.fog_range.y * g_wow->map->gx_skybox->float_values[SKYBOX_FLOAT_FOG_START];
	scene_block.settings.x = !!(g_wow->gx->opt & GX_OPT_DYN_SHADOW);
	scene_block.settings.y = 0;
	scene_block.settings.z = 0;
	scene_block.settings.w = 0;
	gfx_set_buffer_data(&frame->mcnk_uniform_buffer, &scene_block, sizeof(scene_block), 0);
}

static void
build_mliq_uniform_buffer(struct gx_frame *frame)
{
	struct shader_mliq_scene_block scene_block;
	VEC4_CPY(scene_block.light_direction, light_direction);
	COPY_VEC3_A1(scene_block.specular_color, g_wow->map->gx_skybox->int_values[SKYBOX_INT_SUN]);
	COPY_VEC3_A1(scene_block.diffuse_color, g_wow->map->gx_skybox->int_values[SKYBOX_INT_DIFFUSE]);
	COPY_VEC3_A1(scene_block.final_color, g_wow->map->gx_skybox->int_values[SKYBOX_INT_OCEAN2]);
	COPY_VEC3_A1(scene_block.base_color, g_wow->map->gx_skybox->int_values[SKYBOX_INT_OCEAN1]);
	COPY_VEC3_A1(scene_block.fog_color, g_wow->map->gx_skybox->int_values[SKYBOX_INT_FOG]);
	scene_block.fog_range.y = frame->view_distance * g_wow->map->gx_skybox->float_values[SKYBOX_FLOAT_FOG_END] / 36 / g_wow->map->fog_divisor;
	scene_block.fog_range.x = scene_block.fog_range.y * g_wow->map->gx_skybox->float_values[SKYBOX_FLOAT_FOG_START];
	gfx_set_buffer_data(&frame->mliq_uniform_buffer, &scene_block, sizeof(scene_block), 0);
}

static void
build_wmo_uniform_buffer(struct gx_frame *frame)
{
	struct shader_wmo_scene_block scene_block;
	VEC4_CPY(scene_block.light_direction, light_direction);
	COPY_VEC3_A1(scene_block.specular_color, g_wow->map->gx_skybox->int_values[SKYBOX_INT_SUN]);
	COPY_VEC3_A1(scene_block.diffuse_color, g_wow->map->gx_skybox->int_values[SKYBOX_INT_DIFFUSE]);
	COPY_VEC3_A1(scene_block.ambient_color, g_wow->map->gx_skybox->int_values[SKYBOX_INT_AMBIENT]);
	COPY_VEC3_A1(scene_block.fog_color, g_wow->map->gx_skybox->int_values[SKYBOX_INT_FOG]);
	scene_block.fog_range.y = frame->view_distance * g_wow->map->gx_skybox->float_values[SKYBOX_FLOAT_FOG_END] / 36 / g_wow->map->fog_divisor;
	scene_block.fog_range.x = scene_block.fog_range.y * g_wow->map->gx_skybox->float_values[SKYBOX_FLOAT_FOG_START];
	gfx_set_buffer_data(&frame->wmo_uniform_buffer, &scene_block, sizeof(scene_block), 0);
}

static void
build_m2_ground_shadow_uniform_buffer(struct gx_frame *frame)
{
	struct shader_m2_scene_block scene_block;
	VEC4_CPY(scene_block.light_direction, light_direction);
	VEC4_SET(scene_block.specular_color, 0, 0, 0, 0);
	VEC4_SET(scene_block.diffuse_color, 0, 0, 0, 0);
	VEC3_CPY(scene_block.ambient_color, g_wow->map->gx_skybox->int_values[SKYBOX_INT_AMBIENT]);
	scene_block.ambient_color.w = 1;
	scene_block.fog_range.y = frame->view_distance * g_wow->map->gx_skybox->float_values[SKYBOX_FLOAT_FOG_END] / 36 / g_wow->map->fog_divisor;
	scene_block.fog_range.x = scene_block.fog_range.y * g_wow->map->gx_skybox->float_values[SKYBOX_FLOAT_FOG_START];
	gfx_set_buffer_data(&frame->m2_ground_shadow_uniform_buffer, &scene_block, sizeof(scene_block), 0);
}

static void
build_m2_ground_light_uniform_buffer(struct gx_frame *frame)
{
	struct shader_m2_scene_block scene_block;
	VEC4_CPY(scene_block.light_direction, light_direction);
	VEC4_SET(scene_block.specular_color, 0, 0, 0, 0);
	VEC4_SET(scene_block.diffuse_color, 0, 0, 0, 0);
	VEC3_ADD(scene_block.ambient_color, g_wow->map->gx_skybox->int_values[SKYBOX_INT_DIFFUSE], g_wow->map->gx_skybox->int_values[SKYBOX_INT_AMBIENT]);
	scene_block.ambient_color.w = 1;
	scene_block.fog_range.y = frame->view_distance * g_wow->map->gx_skybox->float_values[SKYBOX_FLOAT_FOG_END] / 36 / g_wow->map->fog_divisor;
	scene_block.fog_range.x = scene_block.fog_range.y * g_wow->map->gx_skybox->float_values[SKYBOX_FLOAT_FOG_START];
	gfx_set_buffer_data(&frame->m2_ground_light_uniform_buffer, &scene_block, sizeof(scene_block), 0);
}

static void
build_m2_shadow_uniform_buffer(struct gx_frame *frame)
{
	struct shader_m2_scene_block scene_block;
	VEC4_CPY(scene_block.light_direction, light_direction);
	COPY_VEC3_A1(scene_block.specular_color, g_wow->map->gx_skybox->int_values[SKYBOX_INT_SUN]);
	COPY_VEC3_A1(scene_block.diffuse_color, g_wow->map->gx_skybox->int_values[SKYBOX_INT_DIFFUSE]);
	COPY_VEC3_A1(scene_block.ambient_color, g_wow->map->gx_skybox->int_values[SKYBOX_INT_AMBIENT]);
	scene_block.fog_range.y = frame->view_distance * g_wow->map->gx_skybox->float_values[SKYBOX_FLOAT_FOG_END] / 36 / g_wow->map->fog_divisor;
	scene_block.fog_range.x = scene_block.fog_range.y * g_wow->map->gx_skybox->float_values[SKYBOX_FLOAT_FOG_START];
	scene_block.params.x = 1;
	gfx_set_buffer_data(&frame->m2_shadow_uniform_buffer, &scene_block, sizeof(scene_block), 0);
}

static void
build_m2_world_uniform_buffer(struct gx_frame *frame)
{
	struct shader_m2_scene_block scene_block;
	VEC4_CPY(scene_block.light_direction, light_direction);
	COPY_VEC3_A1(scene_block.specular_color, g_wow->map->gx_skybox->int_values[SKYBOX_INT_SUN]);
	COPY_VEC3_A1(scene_block.diffuse_color, g_wow->map->gx_skybox->int_values[SKYBOX_INT_DIFFUSE]);
	COPY_VEC3_A1(scene_block.ambient_color, g_wow->map->gx_skybox->int_values[SKYBOX_INT_AMBIENT]);
	scene_block.fog_range.y = frame->view_distance * g_wow->map->gx_skybox->float_values[SKYBOX_FLOAT_FOG_END] / 36 / g_wow->map->fog_divisor;
	scene_block.fog_range.x = scene_block.fog_range.y * g_wow->map->gx_skybox->float_values[SKYBOX_FLOAT_FOG_START];
	scene_block.params.x = 0;
	gfx_set_buffer_data(&frame->m2_world_uniform_buffer, &scene_block, sizeof(scene_block), 0);
}

#undef COPY_VEC3_A1

void
gx_frame_build_uniform_buffers(struct gx_frame *frame)
{
	build_particle_uniform_buffer(frame);
	build_ribbon_uniform_buffer(frame);
	build_ocean_uniform_buffer(frame);
	build_river_uniform_buffer(frame);
	build_magma_uniform_buffer(frame);
	build_mcnk_uniform_buffer(frame);
	build_mliq_uniform_buffer(frame);
	build_wmo_uniform_buffer(frame);
	build_m2_ground_shadow_uniform_buffer(frame);
	build_m2_ground_light_uniform_buffer(frame);
	build_m2_shadow_uniform_buffer(frame);
	build_m2_world_uniform_buffer(frame);
}

void
gx_frame_copy_cameras(struct gx_frame *frame,
                      struct camera *cull_camera,
                      struct camera *view_camera)
{
	frustum_copy(&frame->shadow_frustum, &cull_camera->shadow_frustum);
	frustum_copy(&frame->wdl_frustum, &cull_camera->wdl_frustum);
	frustum_copy(&frame->frustum, &cull_camera->frustum);
	frame->view_shadow_vp = view_camera->shadow_vp;
	frame->view_shadow_p = view_camera->shadow_p;
	frame->view_shadow_v = view_camera->shadow_v;
	frame->view_wdl_vp = view_camera->wdl_vp;
	frame->view_wdl_p = view_camera->wdl_p;
	frame->view_pos = view_camera->pos;
	frame->view_rot = view_camera->rot;
	frame->view_vp = view_camera->vp;
	frame->view_v = view_camera->v;
	frame->view_p = view_camera->p;
	frame->cull_wdl_vp = cull_camera->wdl_vp;
	frame->cull_wdl_p = cull_camera->wdl_p;
	frame->cull_pos = cull_camera->pos;
	frame->cull_rot = cull_camera->rot;
	frame->cull_vp = cull_camera->vp;
	frame->cull_v = cull_camera->v;
	frame->cull_p = cull_camera->p;
	frame->view_distance = cull_camera->view_distance;
	frame->fov = view_camera->fov;
	const struct vec4f right = {1, 0, 0, 0};
	const struct vec4f bottom = {0, -1, 0, 0};
	struct mat4f tmp1;
	struct mat4f tmp2;
	MAT4_IDENTITY(tmp1);
	MAT4_ROTATEZ(float, tmp2, tmp1, -view_camera->rot.z);
	MAT4_ROTATEY(float, tmp1, tmp2, -view_camera->rot.y);
	MAT4_ROTATEX(float, tmp2, tmp1, -view_camera->rot.x);
	MAT4_VEC4_MUL(frame->view_right, tmp2, right);
	MAT4_VEC4_MUL(frame->view_bottom, tmp2, bottom);
	frame->m2_params.shadow_vp = frame->view_shadow_vp;
	frame->m2_params.shadow_v = frame->view_shadow_v;
	frame->m2_params.shadow_p = frame->view_shadow_p;
	frame->m2_params.vp = frame->view_vp;
	frame->m2_params.v = frame->view_v;
	frame->m2_params.p = frame->view_p;
	VEC3_CPY(frame->m2_params.fog_color, g_wow->map->gx_skybox->int_values[SKYBOX_INT_FOG]);
	VEC3_CPY(frame->m2_params.view_right, frame->view_right);
	VEC3_CPY(frame->m2_params.view_bottom, frame->view_bottom);
}

void
gx_frame_clear_scene(struct gx_frame *frame)
{
	for (size_t i = 0; i < frame->render_lists.m2.entries.size; ++i)
		gx_m2_clear_update(*JKS_ARRAY_GET(&frame->render_lists.m2.entries, i, struct gx_m2*), frame);
	for (size_t i = 0; i < frame->render_lists.m2_shadow.entries.size; ++i)
		gx_m2_clear_update(*JKS_ARRAY_GET(&frame->render_lists.m2_shadow.entries, i, struct gx_m2*), frame); /* XXX avoid double clear update */
	for (size_t i = 0; i < frame->render_lists.wmo.entries.size; ++i)
		gx_wmo_clear_update(*JKS_ARRAY_GET(&frame->render_lists.wmo.entries, i, struct gx_wmo*), frame);
#ifdef WITH_DEBUG_RENDERING
	for (size_t i = 0; i < sizeof(frame->render_lists.aabb) / sizeof(*frame->render_lists.aabb); ++i)
		gx_aabb_batch_clear_update(&frame->render_lists.aabb[i].batch);
#endif
	for (size_t type = 0; type < sizeof(frame->render_lists.wmo_mliq) / sizeof(*frame->render_lists.wmo_mliq); ++type)
	{
		for (size_t i = 0; i < frame->render_lists.wmo_mliq[type].entries.size; ++i)
			gx_wmo_mliq_clear_update(*JKS_ARRAY_GET(&frame->render_lists.wmo_mliq[type].entries, i, struct gx_wmo_mliq*), frame);
	}
	for (size_t i = 0; i < frame->backrefs.tiles.entries.size; ++i)
	{
		struct map_tile *tile = *JKS_ARRAY_GET(&frame->backrefs.tiles.entries, i, struct map_tile*);
		map_tile_ground_clear(tile, frame);
		map_tile_free(tile);
	}
	for (size_t i = 0; i < frame->backrefs.m2.entries.size; ++i)
		gx_m2_instance_free(*JKS_ARRAY_GET(&frame->backrefs.m2.entries, i, struct gx_m2_instance*));
	for (size_t i = 0; i < frame->backrefs.wmo.entries.size; ++i)
		gx_wmo_instance_free(*JKS_ARRAY_GET(&frame->backrefs.wmo.entries, i, struct gx_wmo_instance*));
	for (size_t i = 0; i < frame->render_lists.text.entries.size; ++i)
		gx_text_free(*JKS_ARRAY_GET(&frame->render_lists.text.entries, i, struct gx_text*));
	list_clear(&frame->render_lists.m2_particles);
	list_clear(&frame->render_lists.m2_ribbons);
	list_clear(&frame->render_lists.m2_transparent);
	list_clear(&frame->render_lists.m2_opaque);
	list_clear(&frame->render_lists.m2_shadow);
	for (size_t i = 0; i < frame->render_lists.m2_ground.entries.size; ++i)
		gx_m2_ground_batch_free(*JKS_ARRAY_GET(&frame->render_lists.m2_ground.entries, i, struct gx_m2_ground_batch*));
	list_clear(&frame->render_lists.m2_ground);
	list_clear(&frame->render_lists.m2);
	list_clear(&frame->render_lists.mcnk);
	for (size_t i = 0; i < sizeof(frame->render_lists.mclq) / sizeof(*frame->render_lists.mclq); ++i)
		list_clear(&frame->render_lists.mclq[i]);
	list_clear(&frame->render_lists.wmo);
	list_clear(&frame->render_lists.text);
	for (size_t type = 0; type < sizeof(frame->render_lists.wmo_mliq) / sizeof(*frame->render_lists.wmo_mliq); ++type)
		list_clear(&frame->render_lists.wmo_mliq[type]);
#ifdef WITH_DEBUG_RENDERING
	for (size_t i = 0; i < sizeof(frame->render_lists.aabb) / sizeof(*frame->render_lists.aabb); ++i)
		list_clear(&frame->render_lists.aabb[i].aabbs);
#endif
	list_clear(&frame->backrefs.m2);
	list_clear(&frame->backrefs.wmo);
	list_clear(&frame->backrefs.wmo_mliq);
	list_clear(&frame->backrefs.tiles);
}

void
gx_frame_release_obj(struct gx_frame *frame)
{
	for (size_t i = 0; i < frame->backrefs.tiles.entries.size; ++i)
	{
		struct map_tile *tile = *JKS_ARRAY_GET(&frame->backrefs.tiles.entries, i, struct map_tile*);
		map_tile_ground_end(tile, frame);
		map_tile_flag_clear(tile, MAP_TILE_FLAG_IN_RENDER_LIST);
	}
	for (size_t i = 0; i < frame->backrefs.m2.entries.size; ++i)
	{
		struct gx_m2_instance *instance = *JKS_ARRAY_GET(&frame->backrefs.m2.entries, i, struct gx_m2_instance*);
		gx_m2_instance_clear_bones(instance);
		gx_m2_instance_flag_clear(instance, GX_M2_INSTANCE_FLAG_IN_RENDER_LIST);
	}
	for (size_t i = 0; i < frame->backrefs.wmo_mliq.entries.size; ++i)
		(*JKS_ARRAY_GET(&frame->backrefs.wmo_mliq.entries, i, struct gx_wmo_mliq*))->in_render_list = false;
	for (size_t i = 0; i < frame->backrefs.wmo.entries.size; ++i)
		gx_wmo_instance_flag_clear(*JKS_ARRAY_GET(&frame->backrefs.wmo.entries, i, struct gx_wmo_instance*), GX_WMO_INSTANCE_FLAG_IN_RENDER_LIST);
	for (size_t type = 0; type < sizeof(frame->render_lists.wmo_mliq) / sizeof(*frame->render_lists.wmo_mliq); ++type)
	{
		for (size_t i = 0; i < frame->render_lists.wmo_mliq[type].entries.size; ++i)
			(*JKS_ARRAY_GET(&frame->render_lists.wmo_mliq[type].entries, i, struct gx_wmo_mliq*))->in_render_lists[type] = false;
	}
	for (size_t i = 0; i < frame->render_lists.m2_shadow.entries.size; ++i)
		gx_m2_flag_clear(*JKS_ARRAY_GET(&frame->render_lists.m2_shadow.entries, i, struct gx_m2*), GX_M2_FLAG_IN_SHADOW_RENDER_LIST);
	for (size_t i = 0; i < frame->render_lists.m2.entries.size; ++i)
		gx_m2_flag_clear(*JKS_ARRAY_GET(&frame->render_lists.m2.entries, i, struct gx_m2*), GX_M2_FLAG_IN_RENDER_LIST);
	for (size_t i = 0; i < frame->render_lists.wmo.entries.size; ++i)
		gx_wmo_flag_clear(*JKS_ARRAY_GET(&frame->render_lists.wmo.entries, i, struct gx_wmo*), GX_WMO_FLAG_IN_RENDER_LIST);
	for (size_t i = 0; i < frame->render_lists.mcnk.entries.size; ++i)
		gx_mcnk_flag_clear(*JKS_ARRAY_GET(&frame->render_lists.mcnk.entries, i, struct gx_mcnk*), GX_MCNK_FLAG_IN_RENDER_LIST);
	for (size_t type = 0; type < sizeof(frame->render_lists.mclq) / sizeof(*frame->render_lists.mclq); ++type)
	{
		for (size_t i = 0; i < frame->render_lists.mclq[type].entries.size; ++i)
			gx_mclq_liquid_flag_clear(&(*JKS_ARRAY_GET(&frame->render_lists.mclq[type].entries, i, struct gx_mclq*))->liquids[type], GX_MCLQ_FLAG_IN_RENDER_LIST);
	}
	for (size_t i = 0; i < frame->render_lists.text.entries.size; ++i)
		gx_text_flag_clear(*JKS_ARRAY_GET(&frame->render_lists.text.entries, i, struct gx_text*), GX_TEXT_FLAG_IN_RENDER_LIST);
#ifdef WITH_DEBUG_RENDERING
	for (size_t line_width = 0; line_width < sizeof(frame->render_lists.aabb) / sizeof(*frame->render_lists.aabb); ++line_width)
	{
		for (size_t i = 0; i < frame->render_lists.aabb[line_width].aabbs.entries.size; ++i)
			(*JKS_ARRAY_GET(&frame->render_lists.aabb[line_width].aabbs.entries, i, struct gx_aabb*))->in_render_list = false;
	}
#endif
}

void
gx_frame_add_mcnk(struct gx_frame *frame, struct gx_mcnk *mcnk)
{
	list_push(&frame->render_lists.mcnk, &mcnk);
}

void
gx_frame_add_mclq(struct gx_frame *frame, uint8_t type, struct gx_mclq *mclq)
{
	list_push(&frame->render_lists.mclq[type], &mclq);
}

void
gx_frame_add_wmo(struct gx_frame *frame, struct gx_wmo *wmo)
{
	list_push(&frame->render_lists.wmo, &wmo);
}

void
gx_frame_add_wmo_backref(struct gx_frame *frame, struct gx_wmo_instance *instance)
{
	list_push(&frame->backrefs.wmo, &instance);
}

void
gx_frame_add_wmo_mliq(struct gx_frame *frame, struct gx_wmo_mliq *mliq, uint8_t liquid)
{
	list_push(&frame->render_lists.wmo_mliq[liquid], &mliq);
}

void
gx_frame_add_wmo_mliq_backref(struct gx_frame *frame, struct gx_wmo_mliq *mliq)
{
	list_push(&frame->backrefs.wmo_mliq, &mliq);
}

void
gx_frame_add_m2(struct gx_frame *frame, struct gx_m2 *m2)
{
	list_push(&frame->render_lists.m2, &m2);
}

void
gx_frame_add_m2_particles(struct gx_frame *frame, struct gx_m2_instance *instance)
{
	list_push(&frame->render_lists.m2_particles, &instance);
}

void
gx_frame_add_m2_ribbons(struct gx_frame *frame, struct gx_m2_instance *instance)
{
	list_push(&frame->render_lists.m2_ribbons, &instance);
}

void
gx_frame_add_m2_transparent(struct gx_frame *frame, struct gx_m2_instance *instance)
{
	list_push(&frame->render_lists.m2_transparent, &instance);
}

void
gx_frame_add_m2_opaque(struct gx_frame *frame, struct gx_m2 *m2)
{
	list_push(&frame->render_lists.m2_opaque, &m2);
}

void
gx_frame_add_m2_shadow(struct gx_frame *frame, struct gx_m2 *m2)
{
	list_push(&frame->render_lists.m2_shadow, &m2);
}

void
gx_frame_add_m2_backref(struct gx_frame *frame, struct gx_m2_instance *instance)
{
	list_push(&frame->backrefs.m2, &instance);
}

void
gx_frame_add_m2_ground(struct gx_frame *frame, struct gx_m2_ground_batch *batch)
{
	list_push(&frame->render_lists.m2_ground, &batch);
}

void
gx_frame_add_text(struct gx_frame *frame, struct gx_text *text)
{
	list_push(&frame->render_lists.text, &text);
}

void
gx_frame_add_tile(struct gx_frame *frame, struct map_tile *tile)
{
	list_push(&frame->backrefs.tiles, &tile);
}

#ifdef WITH_DEBUG_RENDERING
void
gx_frame_add_aabb(struct gx_frame *frame, struct gx_aabb *aabb)
{
	assert(aabb->line_width > 0 && aabb->line_width <= sizeof(frame->render_lists.aabb) / sizeof(*frame->render_lists.aabb));
	list_push(&frame->render_lists.aabb[aabb->line_width - 1].aabbs, &aabb);
	gx_aabb_batch_add(&frame->render_lists.aabb[aabb->line_width - 1].batch, aabb);
}
#endif

void
gx_frame_render_mcnk(struct gx_frame *frame)
{
	if (!(g_wow->gx->opt & GX_OPT_MCNK) || !frame->render_lists.mcnk.entries.size)
		return;
	gfx_bind_pipeline_state(g_wow->device, &g_wow->gx->mcnk_pipeline_state);
	gfx_bind_constant(g_wow->device, 2, &frame->mcnk_uniform_buffer, sizeof(struct shader_mcnk_scene_block), 0);
	for (uint32_t i = 0; i < frame->render_lists.mcnk.entries.size; ++i)
		gx_mcnk_render(*JKS_ARRAY_GET(&frame->render_lists.mcnk.entries, i, struct gx_mcnk*), frame);
}

void
gx_frame_render_mclq(struct gx_frame *frame)
{
	if (!(g_wow->gx->opt & GX_OPT_MCLQ))
		return;
	if (g_wow->gx->opt & GX_OPT_DYN_WATER)
	{
		/* XXX remove from here ? */
		const gfx_texture_t *textures[] =
		{
			&g_wow->post_process.dummy1->color_texture,
			&g_wow->post_process.dummy1->normal_texture,
			&g_wow->post_process.dummy1->position_texture,
		};
		gfx_bind_samplers(g_wow->device, 1, 3, textures);
	}
	uint8_t idx = (frame->time / 30000000) % 30;
	for (size_t type = 0; type < 4; ++type)
	{
		if (!frame->render_lists.mclq[type].entries.size)
			continue;
		switch (type)
		{
			case 0:
				if (g_wow->gx->opt & GX_OPT_DYN_WATER)
					gfx_bind_pipeline_state(g_wow->device, &g_wow->gx->mclq_water_dyn_pipeline_state);
				else
					gfx_bind_pipeline_state(g_wow->device, &g_wow->gx->mclq_water_pipeline_state);
				gfx_bind_constant(g_wow->device, 2, &frame->river_uniform_buffer, sizeof(struct shader_mclq_water_scene_block), 0);
				break;
			case 1:
				if (g_wow->gx->opt & GX_OPT_DYN_WATER)
					gfx_bind_pipeline_state(g_wow->device, &g_wow->gx->mclq_water_dyn_pipeline_state);
				else
					gfx_bind_pipeline_state(g_wow->device, &g_wow->gx->mclq_water_pipeline_state);
				gfx_bind_constant(g_wow->device, 2, &frame->ocean_uniform_buffer, sizeof(struct shader_mclq_water_scene_block), 0);
				break;
			case 2:
			case 3:
				gfx_bind_pipeline_state(g_wow->device, &g_wow->gx->mclq_magma_pipeline_state);
				gfx_bind_constant(g_wow->device, 2, &frame->magma_uniform_buffer, sizeof(struct shader_mclq_magma_scene_block), 0);
				break;
		}
		switch (type)
		{
			case 0:
				gx_blp_bind(g_wow->gx->river_textures[idx], 0);
				break;
			case 1:
				gx_blp_bind(g_wow->gx->ocean_textures[idx], 0);
				break;
			case 2:
				if (g_wow->map->id == 530)
					gx_blp_bind(g_wow->gx->lavag_textures[idx], 0);
				else
					gx_blp_bind(g_wow->gx->magma_textures[idx], 0);
				break;
			case 3:
				gx_blp_bind(g_wow->gx->slime_textures[idx], 0);
				break;
		}
		for (uint32_t i = 0; i < frame->render_lists.mclq[type].entries.size; ++i)
			gx_mclq_render(*JKS_ARRAY_GET(&frame->render_lists.mclq[type].entries, i, struct gx_mclq*), frame, type);
	}
}

void
gx_frame_render_wmo(struct gx_frame *frame)
{
	if (!(g_wow->gx->opt & GX_OPT_WMO) || !frame->render_lists.wmo.entries.size)
		return;
	gfx_bind_constant(g_wow->device, 2, &frame->wmo_uniform_buffer, sizeof(struct shader_wmo_scene_block), 0);
	for (size_t i = 0; i < frame->render_lists.wmo.entries.size; ++i)
		gx_wmo_render(*JKS_ARRAY_GET(&frame->render_lists.wmo.entries, i, struct gx_wmo*), frame);
}

void
gx_frame_render_opaque_m2(struct gx_frame *frame)
{
	if (!(g_wow->gx->opt & GX_OPT_M2) || !frame->render_lists.m2_opaque.entries.size)
		return;
	frame->m2_lighting_type = GX_M2_LIGHTING_WORLD;
	gfx_bind_constant(g_wow->device, 2, &frame->m2_world_uniform_buffer, sizeof(struct shader_m2_scene_block), 0);
	for (size_t i = 0; i < frame->render_lists.m2_opaque.entries.size; ++i)
		gx_m2_render(*JKS_ARRAY_GET(&frame->render_lists.m2_opaque.entries, i, struct gx_m2*), frame, false);
}

void
gx_frame_render_shadow_m2(struct gx_frame *frame)
{
	if (!(g_wow->gx->opt & GX_OPT_M2) || !frame->render_lists.m2_shadow.entries.size)
		return;
	frame->m2_lighting_type = GX_M2_LIGHTING_WORLD;
	gfx_bind_constant(g_wow->device, 2, &frame->m2_shadow_uniform_buffer, sizeof(struct shader_m2_scene_block), 0);
	for (size_t i = 0; i < frame->render_lists.m2_shadow.entries.size; ++i)
		gx_m2_render(*JKS_ARRAY_GET(&frame->render_lists.m2_shadow.entries, i, struct gx_m2*), frame, false);
}

void
gx_frame_render_transparent_m2(struct gx_frame *frame)
{
	if (!(g_wow->gx->opt & GX_OPT_M2) || !frame->render_lists.m2_transparent.entries.size)
		return;
	frame->m2_lighting_type = GX_M2_LIGHTING_WORLD;
	gfx_bind_constant(g_wow->device, 2, &frame->m2_world_uniform_buffer, sizeof(struct shader_m2_scene_block), 0);
	for (size_t i = 0; i < frame->render_lists.m2_transparent.entries.size; ++i)
		gx_m2_instance_render(*JKS_ARRAY_GET(&frame->render_lists.m2_transparent.entries, i, struct gx_m2_instance*), frame, true, &frame->m2_params);
}

void
gx_frame_render_ground_m2(struct gx_frame *frame)
{
	if (!(g_wow->gx->opt & GX_OPT_M2) || !frame->render_lists.m2_ground.entries.size)
		return;
	frame->m2_lighting_type = GX_M2_LIGHTING_WORLD;
	gfx_bind_constant(g_wow->device, 2, &frame->m2_world_uniform_buffer, sizeof(struct shader_m2_scene_block), 0);
	for (size_t i = 0; i < frame->render_lists.m2_ground.entries.size; ++i)
		gx_m2_ground_batch_render(*JKS_ARRAY_GET(&frame->render_lists.m2_ground.entries, i, struct gx_m2_ground_batch*), frame);
}

void
gx_frame_render_m2_particles(struct gx_frame *frame)
{
	if (!(g_wow->gx->opt & GX_OPT_M2_PARTICLES) || !frame->render_lists.m2_particles.entries.size)
		return;
	gfx_bind_constant(g_wow->device, 2, &frame->particle_uniform_buffer, sizeof(struct shader_particle_scene_block), 0);
	for (size_t i = 0; i < frame->render_lists.m2_particles.entries.size; ++i)
		gx_m2_instance_render_particles(*JKS_ARRAY_GET(&frame->render_lists.m2_particles.entries, i, struct gx_m2_instance*), frame, &frame->m2_params);
}

void
gx_frame_render_m2_ribbons(struct gx_frame *frame)
{
	if (!(g_wow->gx->opt & GX_OPT_M2_RIBBONS) || !frame->render_lists.m2_ribbons.entries.size)
		return;
	gfx_bind_constant(g_wow->device, 2, &frame->ribbon_uniform_buffer, sizeof(struct shader_ribbon_scene_block), 0);
	for (size_t i = 0; i < frame->render_lists.m2_ribbons.entries.size; ++i)
		gx_m2_instance_render_ribbons(*JKS_ARRAY_GET(&frame->render_lists.m2_ribbons.entries, i, struct gx_m2_instance*), frame, &frame->m2_params);
}

void
gx_frame_render_texts(struct gx_frame *frame)
{
	if (!frame->render_lists.text.entries.size)
		return;
	gfx_bind_pipeline_state(g_wow->device, &g_wow->gx->text_pipeline_state);
	for (size_t i = 0; i < frame->render_lists.text.entries.size; ++i)
		gx_text_render(*JKS_ARRAY_GET(&frame->render_lists.text.entries, i, struct gx_text*), frame);
}

#ifdef WITH_DEBUG_RENDERING
void
gx_frame_render_aabb(struct gx_frame *frame)
{
	gfx_bind_pipeline_state(g_wow->device, &g_wow->gx->aabb_pipeline_state);
	for (size_t line_width = 0; line_width < sizeof(frame->render_lists.aabb) / sizeof(*frame->render_lists.aabb); ++line_width)
		gx_aabb_batch_render(&frame->render_lists.aabb[line_width].batch);
}

void
gx_frame_render_wmo_portals(struct gx_frame *frame)
{
	if (!(g_wow->gx->opt & GX_OPT_WMO_PORTALS) || !frame->render_lists.wmo.entries.size)
		return;
	gfx_bind_pipeline_state(g_wow->device, &g_wow->gx->wmo_portals_pipeline_state);
	for (size_t i = 0; i < frame->render_lists.wmo.entries.size; ++i)
		gx_wmo_render_portals(*JKS_ARRAY_GET(&frame->render_lists.wmo.entries, i, struct gx_wmo*), frame);
}

void
gx_frame_render_wmo_lights(struct gx_frame *frame)
{
	if (!(g_wow->gx->opt & GX_OPT_WMO_LIGHTS) || !frame->render_lists.wmo.entries.size)
		return;
	gfx_bind_pipeline_state(g_wow->device, &g_wow->gx->wmo_lights_pipeline_state);
	for (size_t i = 0; i < frame->render_lists.wmo.entries.size; ++i)
		gx_wmo_render_lights(*JKS_ARRAY_GET(&frame->render_lists.wmo.entries, i, struct gx_wmo*), frame);
}

void
gx_frame_render_m2_lights(struct gx_frame *frame)
{
	if (!(g_wow->gx->opt & GX_OPT_M2_LIGHTS) || !frame->render_lists.m2.entries.size)
		return;
	gfx_set_point_size(g_wow->device, 5);
	gfx_bind_pipeline_state(g_wow->device, &g_wow->gx->m2_lights_pipeline_state);
	for (size_t i = 0; i < frame->render_lists.m2.entries.size; ++i)
		gx_m2_render_lights(*JKS_ARRAY_GET(&frame->render_lists.m2.entries, i, struct gx_m2*), frame);
}

void
gx_frame_render_m2_bones(struct gx_frame *frame)
{
	if (!(g_wow->gx->opt & GX_OPT_M2_BONES) || !frame->render_lists.m2.entries.size)
		return;
	gfx_set_line_width(g_wow->device, 1);
	gfx_bind_pipeline_state(g_wow->device, &g_wow->gx->m2_bones_lines_pipeline_state);
	for (size_t i = 0; i < frame->render_lists.m2.entries.size; ++i)
		gx_m2_render_bones_lines(*JKS_ARRAY_GET(&frame->render_lists.m2.entries, i, struct gx_m2*), frame);
	gfx_set_point_size(g_wow->device, 5);
	gfx_bind_pipeline_state(g_wow->device, &g_wow->gx->m2_bones_points_pipeline_state);
	for (size_t i = 0; i < frame->render_lists.m2.entries.size; ++i)
		gx_m2_render_bones_points(*JKS_ARRAY_GET(&frame->render_lists.m2.entries, i, struct gx_m2*), frame);
}

void
gx_frame_render_m2_collisions(struct gx_frame *frame)
{
	if (!(g_wow->gx->opt & GX_OPT_M2_COLLISIONS) || !frame->render_lists.m2.entries.size)
		return;
	gfx_bind_pipeline_state(g_wow->device, &g_wow->gx->m2_collisions_lines_pipeline_state);
	for (size_t i = 0; i < frame->render_lists.m2.entries.size; ++i)
		gx_m2_render_collisions(*JKS_ARRAY_GET(&frame->render_lists.m2.entries, i, struct gx_m2*), frame, false);
	gfx_bind_pipeline_state(g_wow->device, &g_wow->gx->m2_collisions_triangles_pipeline_state);
	for (size_t i = 0; i < frame->render_lists.m2.entries.size; ++i)
		gx_m2_render_collisions(*JKS_ARRAY_GET(&frame->render_lists.m2.entries, i, struct gx_m2*), frame, true);
}

void
gx_frame_render_wmo_collisions(struct gx_frame *frame)
{
	if (!(g_wow->gx->opt & GX_OPT_WMO_COLLISIONS) || !frame->render_lists.wmo.entries.size)
		return;
	gfx_bind_pipeline_state(g_wow->device, &g_wow->gx->wmo_collisions_lines_pipeline_state);
	for (size_t i = 0; i < frame->render_lists.wmo.entries.size; ++i)
		gx_wmo_render_collisions(*JKS_ARRAY_GET(&frame->render_lists.wmo.entries, i, struct gx_wmo*), frame, false);
	gfx_bind_pipeline_state(g_wow->device, &g_wow->gx->wmo_collisions_triangles_pipeline_state);
	for (size_t i = 0; i < frame->render_lists.wmo.entries.size; ++i)
		gx_wmo_render_collisions(*JKS_ARRAY_GET(&frame->render_lists.wmo.entries, i, struct gx_wmo*), frame, true);
}

void
gx_frame_render_collisions(struct gx_frame *frame)
{
	if (!(g_wow->gx->opt & GX_OPT_COLLISIONS))
		return;
	gfx_set_line_width(g_wow->device, 1);
	gfx_bind_pipeline_state(g_wow->device, &g_wow->gx->collisions_lines_pipeline_state);
	gx_collisions_render(&frame->gx_collisions, frame, false);
	gfx_bind_pipeline_state(g_wow->device, &g_wow->gx->collisions_triangles_pipeline_state);
	gx_collisions_render(&frame->gx_collisions, frame, true);
}
#endif
