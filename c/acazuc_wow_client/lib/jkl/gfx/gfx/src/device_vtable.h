#ifndef GFX_DEVICE_VTABLE_H
#define GFX_DEVICE_VTABLE_H

#include <stdbool.h>

#include <jks/vec4.h>

#include "objects.h"

struct gfx_device;
struct gfx_window;

struct gfx_device_vtable
{
	bool (*ctr)(struct gfx_device *device, struct gfx_window *window);
	void (*dtr)(struct gfx_device *device);
	void (*tick)(struct gfx_device *device);

	void (*clear_color)(struct gfx_device *device, const gfx_render_target_t *render_target, enum gfx_render_target_attachment attachment, vec4f_t color);
	void (*clear_depth_stencil)(struct gfx_device *device, const gfx_render_target_t *render_target, float depth, uint8_t stencil);

	void (*draw_indexed_indirect)(struct gfx_device *device, const gfx_buffer_t *buffer, uint32_t count, uint32_t offset);
	void (*draw_indirect)(struct gfx_device *device, const gfx_buffer_t *buffer, uint32_t count, uint32_t offset);
	void (*draw_indexed_instanced)(struct gfx_device *device, uint32_t count, uint32_t offset, uint32_t prim_count);
	void (*draw_instanced)(struct gfx_device *device, uint32_t count, uint32_t offset, uint32_t prim_count);
	void (*draw_indexed)(struct gfx_device *device, uint32_t count, uint32_t offset);
	void (*draw)(struct gfx_device *device, uint32_t count, uint32_t offset);

	bool (*create_blend_state)(struct gfx_device *device, gfx_blend_state_t *state, bool enabled, enum gfx_blend_function src_c, enum gfx_blend_function dst_c, enum gfx_blend_function src_a, enum gfx_blend_function dst_a, enum gfx_blend_equation equation_c, enum gfx_blend_equation equation_a, enum gfx_color_mask color_mask);
	void (*delete_blend_state)(struct gfx_device *device, gfx_blend_state_t *state);

	bool (*create_depth_stencil_state)(struct gfx_device *device, gfx_depth_stencil_state_t *state, bool depth_write, bool depth_test, enum gfx_compare_function depth_compare, bool stencil_enabled, uint32_t stencil_write_mask, enum gfx_compare_function stencil_compare, uint32_t stencil_reference, uint32_t stencil_compare_mask, enum gfx_stencil_operation stencil_fail, enum gfx_stencil_operation stencil_zfail, enum gfx_stencil_operation stencil_pass);
	void (*delete_depth_stencil_state)(struct gfx_device *device, gfx_depth_stencil_state_t *state);

	bool (*create_rasterizer_state)(struct gfx_device *device, gfx_rasterizer_state_t *state, enum gfx_fill_mode fill_mode, enum gfx_cull_mode cull_mode, enum gfx_front_face front_face, bool scissor);
	void (*delete_rasterizer_state)(struct gfx_device *device, gfx_rasterizer_state_t *state);

	bool (*create_buffer)(struct gfx_device *device, gfx_buffer_t *buffer, enum gfx_buffer_type type, const void *data, uint32_t size, enum gfx_buffer_usage usage);
	bool (*set_buffer_data)(struct gfx_device *device, gfx_buffer_t *buffer, const void *data, uint32_t size, uint32_t offset);
	void (*delete_buffer)(struct gfx_device *device, gfx_buffer_t *buffer);

	bool (*create_attributes_state)(struct gfx_device *device, gfx_attributes_state_t *state, const struct gfx_attribute_bind *binds, uint32_t count, const gfx_buffer_t *index_buffer, enum gfx_index_type index_type);
	void (*bind_attributes_state)(struct gfx_device *device, const gfx_attributes_state_t *state, const gfx_input_layout_t *input_layout);
	void (*delete_attributes_state)(struct gfx_device *device, gfx_attributes_state_t *state);

	bool (*create_input_layout)(struct gfx_device *device, gfx_input_layout_t *input_layout, const struct gfx_input_layout_bind *binds, uint32_t count, const gfx_shader_state_t *shader_state);
	void (*delete_input_layout)(struct gfx_device *device, gfx_input_layout_t *input_layout);

	bool (*create_texture)(struct gfx_device *device, gfx_texture_t *texture, enum gfx_texture_type type, enum gfx_format format, uint8_t lod, uint32_t width, uint32_t height, uint32_t depth);
	bool (*set_texture_data)(struct gfx_device *device, gfx_texture_t *texture, uint8_t lod, uint32_t offset, uint32_t width, uint32_t height, uint32_t depth, uint32_t size, const void *data);
	void (*set_texture_addressing)(struct gfx_device *device, gfx_texture_t *texture, enum gfx_texture_addressing addressing_s, enum gfx_texture_addressing addressing_t, enum gfx_texture_addressing addressing_r);
	void (*set_texture_filtering)(struct gfx_device *device, gfx_texture_t *texture, enum gfx_filtering min_filtering, enum gfx_filtering mag_filtering, enum gfx_filtering mip_filtering);
	void (*set_texture_anisotropy)(struct gfx_device *device, gfx_texture_t *texture, uint32_t anisotropy);
	void (*set_texture_levels)(struct gfx_device *device, gfx_texture_t *texture, uint32_t min_level, uint32_t max_level);
	bool (*finalize_texture)(struct gfx_device *device, gfx_texture_t *texture);
	void (*delete_texture)(struct gfx_device *device, gfx_texture_t *texture);

	bool (*create_shader)(struct gfx_device *device, gfx_shader_t *shader, enum gfx_shader_type type, const uint8_t *data, uint32_t size);
	void (*delete_shader)(struct gfx_device *device, gfx_shader_t *shader);
	bool (*create_shader_state)(struct gfx_device *device, gfx_shader_state_t *shader_state, const gfx_shader_t **shaders, uint32_t shaders_count, const struct gfx_shader_attribute *attributes, const struct gfx_shader_constant *constants, const struct gfx_shader_sampler *samplers);
	void (*delete_shader_state)(struct gfx_device *device, gfx_shader_state_t *shader_state);
	void (*bind_constant)(struct gfx_device *device, uint32_t bind, const gfx_buffer_t *buffer, uint32_t size, uint32_t offset);
	void (*bind_samplers)(struct gfx_device *device, uint32_t start, uint32_t count, const gfx_texture_t **textures);

	bool (*create_constant_state)(struct gfx_device *device, gfx_constant_state_t *state, const gfx_shader_state_t *shader_state);
	void (*set_constant)(struct gfx_device *device, gfx_constant_state_t *state, uint32_t bind, const gfx_buffer_t *buffer, uint32_t size, uint32_t offset);
	void (*bind_constant_state)(struct gfx_device *device, gfx_constant_state_t *state);
	void (*delete_constant_state)(struct gfx_device *device, gfx_constant_state_t *state);

	bool (*create_sampler_state)(struct gfx_device *device, gfx_sampler_state_t *state, const gfx_shader_state_t *shader_state);
	void (*set_sampler)(struct gfx_device *device, gfx_sampler_state_t *state, uint32_t bind, const gfx_texture_t *texture);
	void (*bind_sampler_state)(struct gfx_device *device, gfx_sampler_state_t *state);
	void (*delete_sampler_state)(struct gfx_device *device, gfx_sampler_state_t *state);

	bool (*create_render_target)(struct gfx_device *device, gfx_render_target_t *render_target);
	void (*delete_render_target)(struct gfx_device *device, gfx_render_target_t *render_target);
	void (*bind_render_target)(struct gfx_device *device, const gfx_render_target_t *render_target);
	void (*set_render_target_texture)(struct gfx_device *device, gfx_render_target_t *render_target, enum gfx_render_target_attachment attachment, const gfx_texture_t *texture);
	void (*set_render_target_draw_buffers)(struct gfx_device *device, gfx_render_target_t *render_target, uint32_t *draw_buffers, uint32_t draw_buffers_count);
	void (*resolve_render_target)(struct gfx_device *device, const gfx_render_target_t *src, const gfx_render_target_t *dst, uint32_t buffers, uint32_t src_color, uint32_t dst_color);

	bool (*create_pipeline_state)(struct gfx_device *device, gfx_pipeline_state_t *state, const gfx_shader_state_t *shader_state, const gfx_rasterizer_state_t *rasterizer, const gfx_depth_stencil_state_t *depth_stencil, const gfx_blend_state_t *blend, const gfx_input_layout_t *input_layout, enum gfx_primitive_type primitive);
	void (*delete_pipeline_state)(struct gfx_device *device, gfx_pipeline_state_t *state);
	void (*bind_pipeline_state)(struct gfx_device *device, const gfx_pipeline_state_t *state);

	void (*set_viewport)(struct gfx_device *device, int32_t x, int32_t y, uint32_t width, uint32_t height);
	void (*set_scissor)(struct gfx_device *device, int32_t x, int32_t y, uint32_t width, uint32_t height);
	void (*set_line_width)(struct gfx_device *device, float line_width);
	void (*set_point_size)(struct gfx_device *device, float point_size);
};

extern const struct gfx_device_vtable gfx_device_vtable;

#define GFX_DEVICE_VTABLE_FN_DEF(prefix, fn) .fn = prefix##_##fn

#define GFX_DEVICE_VTABLE_DEF(prefix) \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, ctr), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, dtr), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, tick), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, clear_color), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, clear_depth_stencil), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, draw_indexed_indirect), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, draw_indirect), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, draw_indexed_instanced), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, draw_instanced), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, draw_indexed), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, draw), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, create_blend_state), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, delete_blend_state), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, create_depth_stencil_state), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, delete_depth_stencil_state), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, create_rasterizer_state), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, delete_rasterizer_state), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, create_buffer), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, set_buffer_data), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, delete_buffer), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, create_attributes_state), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, bind_attributes_state), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, delete_attributes_state), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, create_input_layout), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, delete_input_layout), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, create_texture), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, set_texture_data), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, set_texture_addressing), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, set_texture_filtering), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, set_texture_anisotropy), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, set_texture_levels), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, finalize_texture), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, delete_texture), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, create_shader), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, delete_shader), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, create_shader_state), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, delete_shader_state), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, bind_constant), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, bind_samplers), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, create_constant_state), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, set_constant), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, bind_constant_state), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, delete_constant_state), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, create_sampler_state), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, set_sampler), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, bind_sampler_state), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, delete_sampler_state), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, create_render_target), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, delete_render_target), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, bind_render_target), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, set_render_target_texture), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, set_render_target_draw_buffers), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, resolve_render_target), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, create_pipeline_state), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, delete_pipeline_state), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, bind_pipeline_state), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, set_viewport), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, set_scissor), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, set_line_width), \
	GFX_DEVICE_VTABLE_FN_DEF(prefix, set_point_size), \

#endif
