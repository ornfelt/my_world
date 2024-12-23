#ifndef GX_AABB_H
#define GX_AABB_H

#include "shaders.h"

#include <jks/array.h>
#include <jks/aabb.h>

#include <gfx/objects.h>

#include <pthread.h>

struct gx_aabb
{
	struct shader_aabb_input data;
	uint8_t line_width;
	uint8_t flags;
	bool in_render_list;
};

struct gx_aabb_batch
{
	gfx_attributes_state_t attributes_state;
	gfx_buffer_t buffer;
	pthread_mutex_t mutex;
	struct jks_array data;
	uint8_t line_width;
	size_t buffer_size;
};

void gx_aabb_init(struct gx_aabb *aabb, struct vec4f color, uint8_t line_width);
void gx_aabb_destroy(struct gx_aabb *aabb);
void gx_aabb_update(struct gx_aabb *aabb, const struct mat4f *mvp);
void gx_aabb_set_aabb(struct gx_aabb *aabb, const struct aabb *value);
void gx_aabb_set_color(struct gx_aabb *aabb, const struct vec4f *color);
void gx_aabb_add_to_render(struct gx_aabb *aabb, struct gx_frame *frame, const struct mat4f *mvp);

void gx_aabb_batch_init(struct gx_aabb_batch *batch, uint8_t line_width);
void gx_aabb_batch_destroy(struct gx_aabb_batch *batch);
void gx_aabb_batch_clear_update(struct gx_aabb_batch *batch);
void gx_aabb_batch_add(struct gx_aabb_batch *batch, const struct gx_aabb *aabb);
void gx_aabb_batch_render(struct gx_aabb_batch *batch);

#endif
