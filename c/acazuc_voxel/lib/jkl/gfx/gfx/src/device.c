#include "device.h"
#include "device_vtable.h"
#include "config.h"
#include "window.h"
#include <stdlib.h>

#if 0
# include <stdio.h>
# define DEV_DEBUG() printf("%s@%s:%d\n", __func__, __FILE__, __LINE__)
#else
# define DEV_DEBUG()
#endif

#define DEVICE_CALL(fn, device, ...) \
do \
{ \
	DEV_DEBUG(); \
	(device)->vtable->fn(device, ##__VA_ARGS__); \
	DEV_DEBUG(); \
} while (0)

#define DEVICE_CALL_RET(ret_type, fn, device, ...) \
do \
{ \
	DEV_DEBUG(); \
	ret_type ret = (device)->vtable->fn(device, ##__VA_ARGS__); \
	DEV_DEBUG(); \
	return ret; \
} while (0)

void gfx_device_delete(struct gfx_device *device)
{
	if (!device)
		return;
	device->vtable->dtr(device);
	GFX_FREE(device);
}

static bool ctr(struct gfx_device *device, struct gfx_window *window)
{
	device->window = window;
	device->stats.draw_calls_count = 0;
	device->stats.triangles_count = 0;
	device->stats.points_count = 0;
	device->stats.lines_count = 0;
	device->capabilities.constant_alignment = 0;
	device->capabilities.max_samplers = 0;
	device->capabilities.max_msaa = 0;
	return true;
}

static void dtr(struct gfx_device *device)
{
	(void)device;
}

static void tick(struct gfx_device *device)
{
	device->stats.draw_calls_count = 0;
	device->stats.triangles_count = 0;
	device->stats.points_count = 0;
	device->stats.lines_count = 0;
}

const struct gfx_device_vtable gfx_device_vtable =
{
	.ctr  = ctr,
	.dtr  = dtr,
	.tick = tick,
};

uint32_t gfx_get_uniform_buffer_size(struct gfx_device *device, uint32_t buffer_size)
{
	buffer_size += device->capabilities.constant_alignment - 1;
	buffer_size -= buffer_size % device->capabilities.constant_alignment;
	return buffer_size;
}

void gfx_device_tick(struct gfx_device *device)
{
	DEVICE_CALL(tick, device);
}

void gfx_clear_color(struct gfx_device *device, const gfx_render_target_t *render_target, enum gfx_render_target_attachment attachment, vec4f_t color)
{
	DEVICE_CALL(clear_color, device, render_target, attachment, color);
}

void gfx_clear_depth_stencil(struct gfx_device *device, const gfx_render_target_t *render_target, float depth, uint8_t stencil)
{
	DEVICE_CALL(clear_depth_stencil, device, render_target, depth, stencil);
}

void gfx_add_draw_stats(struct gfx_device *device, enum gfx_primitive_type primitive, uint32_t count, uint32_t prim_count)
{
#ifndef NDEBUG
	switch (primitive)
	{
		case GFX_PRIMITIVE_TRIANGLES:
			device->stats.triangles_count += count / 3 * prim_count;
			break;
		case GFX_PRIMITIVE_TRIANGLE_STRIP:
			device->stats.triangles_count += (count - 2) * prim_count;
			break;
		case GFX_PRIMITIVE_POINTS:
			device->stats.points_count += count * prim_count;
			break;
		case GFX_PRIMITIVE_LINES:
			device->stats.lines_count += count / 2 * prim_count;
			break;
		case GFX_PRIMITIVE_LINE_STRIP:
			device->stats.lines_count += (count - 1) * prim_count;
			break;
	}
	device->stats.draw_calls_count++;
#endif
}

void gfx_draw_indexed_indirect(struct gfx_device *device, const gfx_buffer_t *buffer, uint32_t count, uint32_t offset)
{
	DEVICE_CALL(draw_indexed_indirect, device, buffer, count, offset);
}

void gfx_draw_indirect(struct gfx_device *device, const gfx_buffer_t *buffer, uint32_t count, uint32_t offset)
{
	DEVICE_CALL(draw_indirect, device, buffer, count, offset);
}

void gfx_draw_indexed_instanced(struct gfx_device *device, uint32_t count, uint32_t offset, uint32_t prim_count)
{
	DEVICE_CALL(draw_indexed_instanced, device, count, offset, prim_count);
}

void gfx_draw_instanced(struct gfx_device *device, uint32_t count, uint32_t offset, uint32_t prim_count)
{
	DEVICE_CALL(draw_instanced, device, count, offset, prim_count);
}

void gfx_draw_indexed(struct gfx_device *device, uint32_t count, uint32_t offset)
{
	DEVICE_CALL(draw_indexed, device, count, offset);
}

void gfx_draw(struct gfx_device *device, uint32_t count, uint32_t offset)
{
	DEVICE_CALL(draw, device, count, offset);
}

bool gfx_create_blend_state(struct gfx_device *device, gfx_blend_state_t *state, bool enabled, enum gfx_blend_function src_c, enum gfx_blend_function dst_c, enum gfx_blend_function src_a, enum gfx_blend_function dst_a, enum gfx_blend_equation equation_c, enum gfx_blend_equation equation_a, enum gfx_color_mask color_mask)
{
	DEVICE_CALL_RET(bool, create_blend_state, device, state, enabled, src_c, dst_c, src_a, dst_a, equation_c, equation_a, color_mask);
}

void gfx_delete_blend_state(struct gfx_device *device, gfx_blend_state_t *state)
{
	DEVICE_CALL(delete_blend_state, device, state);
}

bool gfx_create_depth_stencil_state(struct gfx_device *device, gfx_depth_stencil_state_t *state, bool depth_write, bool depth_test, enum gfx_compare_function depth_compare, bool stencil_enabled, uint32_t stencil_write_mask, enum gfx_compare_function stencil_compare, uint32_t stencil_reference, uint32_t stencil_compare_mask, enum gfx_stencil_operation stencil_fail, enum gfx_stencil_operation stencil_zfail, enum gfx_stencil_operation stencil_pass)
{
	DEVICE_CALL_RET(bool, create_depth_stencil_state, device, state, depth_write, depth_test, depth_compare, stencil_enabled, stencil_write_mask, stencil_compare, stencil_reference, stencil_compare_mask, stencil_fail, stencil_zfail, stencil_pass);
}

void gfx_delete_depth_stencil_state(struct gfx_device *device, gfx_depth_stencil_state_t *state)
{
	DEVICE_CALL(delete_depth_stencil_state, device, state);
}

bool gfx_create_rasterizer_state(struct gfx_device *device, gfx_rasterizer_state_t *state, enum gfx_fill_mode fill_mode, enum gfx_cull_mode cull_mode, enum gfx_front_face front_face, bool scissor)
{
	DEVICE_CALL_RET(bool, create_rasterizer_state, device, state, fill_mode, cull_mode, front_face, scissor);
}

void gfx_delete_rasterizer_state(struct gfx_device *device, gfx_rasterizer_state_t *state)
{
	DEVICE_CALL(delete_rasterizer_state, device, state);
}

bool gfx_create_buffer(struct gfx_device *device, gfx_buffer_t *buffer, enum gfx_buffer_type type, const void *data, uint32_t size, enum gfx_buffer_usage usage)
{
	DEVICE_CALL_RET(bool, create_buffer, device, buffer, type, data, size, usage);
}

bool gfx_set_buffer_data(gfx_buffer_t *buffer, const void *data, uint32_t size, uint32_t offset)
{
	DEVICE_CALL_RET(bool, set_buffer_data, buffer->device, buffer, data, size, offset);
}

void gfx_delete_buffer(struct gfx_device *device, gfx_buffer_t *buffer)
{
	DEVICE_CALL(delete_buffer, device, buffer);
}

bool gfx_create_attributes_state(struct gfx_device *device, gfx_attributes_state_t *state, const struct gfx_attribute_bind *binds, uint32_t count, const gfx_buffer_t *index_buffer, enum gfx_index_type index_type)
{
	DEVICE_CALL_RET(bool, create_attributes_state, device, state, binds, count, index_buffer, index_type);
}

void gfx_bind_attributes_state(struct gfx_device *device, const gfx_attributes_state_t *state, const gfx_input_layout_t *input_layout)
{
	DEVICE_CALL(bind_attributes_state, device, state, input_layout);
}

void gfx_delete_attributes_state(struct gfx_device *device, gfx_attributes_state_t *state)
{
	DEVICE_CALL(delete_attributes_state, device, state);
}

bool gfx_create_input_layout(struct gfx_device *device, gfx_input_layout_t *input_layout, const struct gfx_input_layout_bind *binds, uint32_t count, const gfx_shader_state_t *shader_state)
{
	DEVICE_CALL_RET(bool, create_input_layout, device, input_layout, binds, count, shader_state);
}

void gfx_delete_input_layout(struct gfx_device *device, gfx_input_layout_t *input_layout)
{
	DEVICE_CALL(delete_input_layout, device, input_layout);
}

bool gfx_create_texture(struct gfx_device *device, gfx_texture_t *texture, enum gfx_texture_type type, enum gfx_format format, uint8_t lod, uint32_t width, uint32_t height, uint32_t depth)
{
	DEVICE_CALL_RET(bool, create_texture, device, texture, type, format, lod, width, height, depth);
}

bool gfx_set_texture_data(gfx_texture_t *texture, uint8_t lod, uint32_t offset, uint32_t width, uint32_t height, uint32_t depth, uint32_t size, const void *data)
{
	DEVICE_CALL_RET(bool, set_texture_data, texture->device, texture, lod, offset, width, height, depth, size, data);
}

void gfx_set_texture_addressing(gfx_texture_t *texture, enum gfx_texture_addressing addressing_s, enum gfx_texture_addressing addressing_t, enum gfx_texture_addressing addressing_r)
{
	DEVICE_CALL(set_texture_addressing, texture->device, texture, addressing_s, addressing_t, addressing_r);
}

void gfx_set_texture_filtering(gfx_texture_t *texture, enum gfx_filtering min_filtering, enum gfx_filtering mag_filtering, enum gfx_filtering mip_filtering)
{
	DEVICE_CALL(set_texture_filtering, texture->device, texture, min_filtering, mag_filtering, mip_filtering);
}

void gfx_set_texture_anisotropy(gfx_texture_t *texture, uint32_t anisotropy)
{
	DEVICE_CALL(set_texture_anisotropy, texture->device, texture, anisotropy);
}

void gfx_set_texture_levels(gfx_texture_t *texture, uint32_t min_level, uint32_t max_level)
{
	DEVICE_CALL(set_texture_levels, texture->device, texture, min_level, max_level);
}

bool gfx_finalize_texture(gfx_texture_t *texture)
{
	DEVICE_CALL_RET(bool, finalize_texture, texture->device, texture);
}

void gfx_delete_texture(struct gfx_device *device, gfx_texture_t *texture)
{
	DEVICE_CALL(delete_texture, device, texture);
}

bool gfx_create_shader(struct gfx_device *device, gfx_shader_t *shader, enum gfx_shader_type type, const uint8_t *data, uint32_t size)
{
	DEVICE_CALL_RET(bool, create_shader, device, shader, type, data, size);
}

void gfx_delete_shader(struct gfx_device *device, gfx_shader_t *shader)
{
	DEVICE_CALL(delete_shader, device, shader);
}

bool gfx_create_shader_state(struct gfx_device *device, gfx_shader_state_t *state, const gfx_shader_t **shaders, uint32_t shaders_count, const struct gfx_shader_attribute *attributes, const struct gfx_shader_constant *constants, const struct gfx_shader_sampler *samplers)
{
	DEVICE_CALL_RET(bool, create_shader_state, device, state, shaders, shaders_count, attributes, constants, samplers);
}

void gfx_delete_shader_state(struct gfx_device *device, gfx_shader_state_t *state)
{
	DEVICE_CALL(delete_shader_state, device, state);
}

void gfx_bind_constant(struct gfx_device *device, uint32_t bind, const gfx_buffer_t *buffer, uint32_t size, uint32_t offset)
{
	DEVICE_CALL(bind_constant, device, bind, buffer, size, offset);
}

void gfx_bind_samplers(struct gfx_device *device, uint32_t start, uint32_t count, const gfx_texture_t **textures)
{
	DEVICE_CALL(bind_samplers, device, start, count, textures);
}

bool gfx_create_constant_state(struct gfx_device *device, gfx_constant_state_t *state, const gfx_shader_state_t *shader_state)
{
	DEVICE_CALL_RET(bool, create_constant_state, device, state, shader_state);
}

void gfx_set_constant(gfx_constant_state_t *state, uint32_t bind, const gfx_buffer_t *buffer, uint32_t size, uint32_t offset)
{
	DEVICE_CALL(set_constant, state->device, state, bind, buffer, size, offset);
}

void gfx_bind_constant_state(struct gfx_device *device, gfx_constant_state_t *state)
{
	DEVICE_CALL(bind_constant_state, device, state);
}

void gfx_delete_constant_state(struct gfx_device *device, gfx_constant_state_t *state)
{
	DEVICE_CALL(delete_constant_state, device, state);
}

bool gfx_create_sampler_state(struct gfx_device *device, gfx_sampler_state_t *state, const gfx_shader_state_t *shader_state)
{
	DEVICE_CALL_RET(bool, create_sampler_state, device, state, shader_state);
}

void gfx_set_sampler(gfx_sampler_state_t *state, uint32_t bind, const gfx_texture_t *texture)
{
	DEVICE_CALL(set_sampler, state->device, state, bind, texture);
}

void gfx_bind_sampler_state(struct gfx_device *device, gfx_sampler_state_t *state)
{
	DEVICE_CALL(bind_sampler_state, device, state);
}

void gfx_delete_sampler_state(struct gfx_device *device, gfx_sampler_state_t *state)
{
	DEVICE_CALL(delete_sampler_state, device, state);
}

bool gfx_create_render_target(struct gfx_device *device, gfx_render_target_t *render_target)
{
	DEVICE_CALL_RET(bool, create_render_target, device, render_target);
}

void gfx_delete_render_target(struct gfx_device *device, gfx_render_target_t *render_target)
{
	DEVICE_CALL(delete_render_target, device, render_target);
}

void gfx_bind_render_target(struct gfx_device *device, const gfx_render_target_t *render_target)
{
	DEVICE_CALL(bind_render_target, device, render_target);
}

void gfx_set_render_target_texture(gfx_render_target_t *render_target, enum gfx_render_target_attachment attachment, const gfx_texture_t *texture)
{
	DEVICE_CALL(set_render_target_texture, render_target->device, render_target, attachment, texture);
}

void gfx_set_render_target_draw_buffers(gfx_render_target_t *render_target, uint32_t *draw_buffers, uint32_t draw_buffers_count)
{
	DEVICE_CALL(set_render_target_draw_buffers, render_target->device, render_target, draw_buffers, draw_buffers_count);
}

void gfx_resolve_render_target(const gfx_render_target_t *src, const gfx_render_target_t *dst, uint32_t buffers, uint32_t color_src, uint32_t color_dst)
{
	DEVICE_CALL(resolve_render_target, src->device, src, dst, buffers, color_src, color_dst);
}

bool gfx_create_pipeline_state(struct gfx_device *device, gfx_pipeline_state_t *state, const gfx_shader_state_t *shader_state, const gfx_rasterizer_state_t *rasterizer, const gfx_depth_stencil_state_t *depth_stencil, const gfx_blend_state_t *blend, const gfx_input_layout_t *input_layout, enum gfx_primitive_type primitive)
{
	DEVICE_CALL_RET(bool, create_pipeline_state, device, state, shader_state, rasterizer, depth_stencil, blend, input_layout, primitive);
}

void gfx_delete_pipeline_state(struct gfx_device *device, gfx_pipeline_state_t *state)
{
	DEVICE_CALL(delete_pipeline_state, device, state);
}

void gfx_bind_pipeline_state(struct gfx_device *device, const gfx_pipeline_state_t *state)
{
	DEVICE_CALL(bind_pipeline_state, device, state);
}

void gfx_set_viewport(struct gfx_device *device, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
	DEVICE_CALL(set_viewport, device, x, y, width, height);
}

void gfx_set_scissor(struct gfx_device *device, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
	DEVICE_CALL(set_scissor, device, x, y, width, height);
}

void gfx_set_line_width(struct gfx_device *device, float line_width)
{
	DEVICE_CALL(set_line_width, device, line_width);
}

void gfx_set_point_size(struct gfx_device *device, float point_size)
{
	DEVICE_CALL(set_point_size, device, point_size);
}
