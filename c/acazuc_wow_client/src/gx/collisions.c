#include "gx/collisions.h"
#include "gx/frame.h"

#include "map/map.h"

#include "graphics.h"
#include "shaders.h"
#include "memory.h"
#include "camera.h"
#include "wow.h"
#include "log.h"

#include <gfx/device.h>

#include <math.h>

void gx_collisions_init(struct gx_collisions *collisions)
{
	for (size_t i = 0; i < RENDER_FRAMES_COUNT; ++i)
	{
		struct gx_collisions_frame *collisions_frame = &collisions->frames[i];
		collisions_frame->attributes_state = GFX_ATTRIBUTES_STATE_INIT();
		collisions_frame->triangles_uniform_buffer = GFX_BUFFER_INIT();
		collisions_frame->lines_uniform_buffer = GFX_BUFFER_INIT();
		collisions_frame->positions_buffer = GFX_BUFFER_INIT();
		collisions_frame->points = NULL;
		collisions_frame->points_nb = 0;
		collisions_frame->buffer_size = 0;
	}
	collisions->initialized = false;
}

void gx_collisions_destroy(struct gx_collisions *collisions)
{
	for (size_t i = 0; i < RENDER_FRAMES_COUNT; ++i)
	{
		struct gx_collisions_frame *collisions_frame = &collisions->frames[i];
		gfx_delete_attributes_state(g_wow->device, &collisions_frame->attributes_state);
		gfx_delete_buffer(g_wow->device, &collisions_frame->lines_uniform_buffer);
		gfx_delete_buffer(g_wow->device, &collisions_frame->triangles_uniform_buffer);
		gfx_delete_buffer(g_wow->device, &collisions_frame->positions_buffer);
		mem_free(MEM_GX, collisions->frames[i].points);
	}
}

void gx_collisions_update(struct gx_collisions *collisions, struct gx_frame *frame, struct collision_triangle *triangles, size_t triangles_nb)
{
	struct gx_collisions_frame *collisions_frame = &collisions->frames[frame->id];
	mem_free(MEM_GX, collisions_frame->points);
	collisions_frame->points = mem_malloc(MEM_GX, sizeof(*collisions_frame->points) * triangles_nb * 3);
	if (!collisions_frame->points)
	{
		LOG_ERROR("points allocation failed");
		collisions_frame->points_nb = 0;
		return;
	}
	for (size_t i = 0; i < triangles_nb; ++i)
	{
		struct vec3f *dst = &collisions_frame->points[i * 3];
		dst[0] = triangles[i].points[0];
		dst[1] = triangles[i].points[1];
		dst[2] = triangles[i].points[2];
	}
	collisions_frame->points_nb = triangles_nb * 3;
}

static void update(struct gx_collisions *collisions, struct gx_frame *frame)
{
	struct gx_collisions_frame *collisions_frame = &collisions->frames[frame->id];
	if (collisions_frame->points_nb > collisions_frame->buffer_size
	 || collisions_frame->points_nb < collisions_frame->buffer_size / 2)
	{
		collisions_frame->buffer_size = npot32(collisions_frame->points_nb + 1);
		gfx_delete_attributes_state(g_wow->device, &collisions_frame->attributes_state);
		gfx_delete_buffer(g_wow->device, &collisions_frame->positions_buffer);
		collisions_frame->attributes_state = GFX_ATTRIBUTES_STATE_INIT();
		collisions_frame->positions_buffer = GFX_BUFFER_INIT();
		gfx_create_buffer(g_wow->device, &collisions_frame->positions_buffer, GFX_BUFFER_VERTEXES, NULL, sizeof(*collisions_frame->points) * collisions_frame->buffer_size, GFX_BUFFER_STREAM);
		const struct gfx_attribute_bind binds[] =
		{
			{&collisions_frame->positions_buffer},
		};
		gfx_create_attributes_state(g_wow->device, &collisions_frame->attributes_state, binds, sizeof(binds) / sizeof(*binds), NULL, GFX_INDEX_UINT16);
	}
	gfx_set_buffer_data(&collisions_frame->positions_buffer, collisions_frame->points, sizeof(*collisions_frame->points) * collisions_frame->points_nb, 0);
	mem_free(MEM_GX, collisions_frame->points);
	collisions_frame->points = NULL;
}

static void initialize(struct gx_collisions *collisions)
{
	for (size_t i = 0; i < RENDER_FRAMES_COUNT; ++i)
	{
		struct gx_collisions_frame *collisions_frame = &collisions->frames[i];
		gfx_create_buffer(g_wow->device, &collisions_frame->triangles_uniform_buffer, GFX_BUFFER_UNIFORM, NULL, sizeof(struct shader_collisions_mesh_block), GFX_BUFFER_STREAM);
		gfx_create_buffer(g_wow->device, &collisions_frame->lines_uniform_buffer, GFX_BUFFER_UNIFORM, NULL, sizeof(struct shader_collisions_mesh_block), GFX_BUFFER_STREAM);
	}
	collisions->initialized = true;
}

void gx_collisions_render(struct gx_collisions *collisions, struct gx_frame *frame, bool triangles)
{
	struct gx_collisions_frame *collisions_frame = &collisions->frames[frame->id];
	if (!collisions_frame->points_nb)
		return;
	if (!collisions->initialized)
		initialize(collisions);
	if (collisions_frame->points)
		update(collisions, frame);
	gfx_bind_attributes_state(g_wow->device, &collisions_frame->attributes_state, &g_wow->graphics->collisions_input_layout);
	struct shader_collisions_mesh_block mesh_block;
	if (triangles)
	{
		float f = sinf(frame->time / 50000000.0);
		VEC4_SET(mesh_block.color, 1.0, 0.1 + 0.1 * f, 0.1 + 0.1 * f, 0.1);
	}
	else
	{
		VEC4_SET(mesh_block.color, 1.0, 0.2, 0.2, 0.5);
	}
	mesh_block.mvp = g_wow->view_camera->vp;
	gfx_buffer_t *uniform_buffer = triangles ? &collisions_frame->triangles_uniform_buffer : &collisions_frame->lines_uniform_buffer;
	gfx_set_buffer_data(uniform_buffer, &mesh_block, sizeof(mesh_block), 0);
	gfx_bind_constant(g_wow->device, 0, uniform_buffer, sizeof(mesh_block), 0);
	gfx_draw(g_wow->device, collisions_frame->points_nb, 0);
}
