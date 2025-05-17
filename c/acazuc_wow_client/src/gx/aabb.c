#include "gx/frame.h"
#include "gx/aabb.h"
#include "gx/gx.h"

#include "map/map.h"

#include "memory.h"
#include "wow.h"
#include "log.h"

#include <gfx/device.h>

MEMORY_DECL(GX);

void gx_aabb_init(struct gx_aabb *aabb, struct vec4f color, uint8_t line_width)
{
	aabb->data.color = color;
	VEC4_SET(aabb->data.min_pos, 0, 0, 0, 1);
	VEC4_SET(aabb->data.max_pos, 0, 0, 0, 1);
	MAT4_IDENTITY(aabb->data.mvp);
	aabb->line_width = line_width;
	aabb->flags = 0;
	aabb->in_render_list = false;
}

void gx_aabb_destroy(struct gx_aabb *aabb)
{
	(void)aabb;
}

void gx_aabb_update(struct gx_aabb *aabb, const struct mat4f *mvp)
{
	aabb->data.mvp = *mvp;
}

void gx_aabb_set_aabb(struct gx_aabb *aabb, const struct aabb *value)
{
	VEC3_CPY(aabb->data.min_pos, value->p0);
	VEC3_CPY(aabb->data.max_pos, value->p1);
}

void gx_aabb_set_color(struct gx_aabb *aabb, const struct vec4f *color)
{
	aabb->data.color = *color;
}

void gx_aabb_add_to_render(struct gx_aabb *aabb, struct gx_frame *frame, const struct mat4f *mvp)
{
	if (aabb->in_render_list)
		return;
	aabb->in_render_list = true;
	gx_aabb_update(aabb, mvp);
	gx_frame_add_aabb(frame, aabb);
}

void gx_aabb_batch_init(struct gx_aabb_batch *batch, uint8_t line_width)
{
	batch->attributes_state = GFX_ATTRIBUTES_STATE_INIT();
	batch->buffer = GFX_BUFFER_INIT();
	batch->line_width = line_width;
	jks_array_init(&batch->data, sizeof(struct shader_aabb_input), NULL, &jks_array_memory_fn_GX);
	batch->buffer_size = 0;
	pthread_mutex_init(&batch->mutex, NULL);
}

void gx_aabb_batch_destroy(struct gx_aabb_batch *batch)
{
	if (!batch)
		return;
	gfx_delete_attributes_state(g_wow->device, &batch->attributes_state);
	gfx_delete_buffer(g_wow->device, &batch->buffer);
	jks_array_destroy(&batch->data);
	pthread_mutex_destroy(&batch->mutex);
}

void gx_aabb_batch_clear_update(struct gx_aabb_batch *batch)
{
	jks_array_resize(&batch->data, 0);
}

void gx_aabb_batch_add(struct gx_aabb_batch *batch, const struct gx_aabb *aabb)
{
	pthread_mutex_lock(&batch->mutex);
	if (!jks_array_push_back(&batch->data, &aabb->data))
		LOG_ERROR("failed to add aabb to batch");
	pthread_mutex_unlock(&batch->mutex);
}

void gx_aabb_batch_render(struct gx_aabb_batch *batch)
{
	if (batch->data.size > batch->buffer_size
	 || batch->data.size < batch->buffer_size / 2)
	{
		gfx_delete_buffer(g_wow->device, &batch->buffer);
		gfx_delete_attributes_state(g_wow->device, &batch->attributes_state);
		batch->buffer = GFX_BUFFER_INIT();
		batch->attributes_state = GFX_ATTRIBUTES_STATE_INIT();
		batch->buffer_size = npot32(batch->data.size + 1);
		gfx_create_buffer(g_wow->device, &batch->buffer, GFX_BUFFER_VERTEXES, NULL, sizeof(struct shader_aabb_input) * batch->buffer_size, GFX_BUFFER_STREAM);
		const struct gfx_attribute_bind binds[] =
		{
			{&g_wow->gx->aabb_vertexes_buffer},
			{&batch->buffer},
		};
		gfx_create_attributes_state(g_wow->device, &batch->attributes_state, binds, sizeof(binds) / sizeof(*binds), &g_wow->gx->aabb_indices_buffer, GFX_INDEX_UINT16);
	}
	if (!batch->data.size)
		return;
	gfx_set_buffer_data(&batch->buffer, batch->data.data, sizeof(struct shader_aabb_input) * batch->data.size, 0);
	gfx_bind_attributes_state(g_wow->device, &batch->attributes_state, &g_wow->gx->aabb_input_layout);
	gfx_set_line_width(g_wow->device, batch->line_width);
	gfx_draw_indexed_instanced(g_wow->device, 24, 0, batch->data.size);
}
