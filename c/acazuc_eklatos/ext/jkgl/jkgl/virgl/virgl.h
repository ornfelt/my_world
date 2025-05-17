#ifndef JKGL_VIRGL_H
#define JKGL_VIRGL_H

#include "virgl_protocol.h"
#include "mesa_pipe.h"
#include "virgl_hw.h"
#include "jkg/jkg.h"

struct virgl_res_info
{
	uint32_t target;
	uint32_t format;
	uint32_t bind;
	struct jkg_extent size;
	uint32_t array_size;
	uint32_t last_level;
	uint32_t nr_samples;
	uint32_t flags;
	uint32_t bytes;
};

struct virgl_res
{
	struct virgl_res_info info;
	int fd;
	uint32_t id;
	void *data;
};

struct jkg_blend_state
{
	uint32_t id;
};

struct jkg_rasterizer_state
{
	uint32_t id;
};

struct jkg_buffer
{
	struct virgl_res *res;
};

struct jkg_input_layout
{
	uint32_t id;
};

struct jkg_sampler
{
	uint32_t id;
};

struct jkg_depth_stencil_state
{
	uint32_t id;
};

struct jkg_image_view
{
	uint32_t id;
};

struct jkg_image
{
	struct virgl_res *res;
	enum jkg_image_type type;
	enum jkg_format format;
};

struct jkg_shader
{
	uint32_t id;
	enum jkg_shader_type type;
};

struct jkg_shader_state
{
	struct jkg_shader *vs;
	struct jkg_shader *fs;
};

struct jkg_surface
{
	struct jkg_image *image;
	uint32_t id;
};

struct cmd_buf
{
	uint8_t *data;
	uint32_t size;
	uint32_t pos;
};

struct jkg_ctx
{
	const struct jkg_op *op;
	const struct jkg_caps *caps_ptr;
	struct cmd_buf cmd_buf;
	struct jkg_id_list id_list;
	int fd;
	int ctx_fd;
	int fb;
	uint32_t capset_id;
	uint32_t capset_version;
	uint32_t capset_size;
	union
	{
		struct virgl_caps_v1 capset_v1;
		struct virgl_caps_v2 capset_v2;
		uint8_t capset[0];
	};
	struct jkg_caps caps;
	struct jkg_image *default_color_images[2];
	struct jkg_image *default_depth_stencil_image;
	uint32_t width;
	uint32_t height;
};

int virgl_alloc_id(struct jkg_ctx *ctx, uint32_t *id);
int virgl_free_id(struct jkg_ctx *ctx, uint32_t id);
int virgl_req_alloc(struct jkg_ctx *ctx, enum virgl_context_cmd cmd, enum virgl_object_type type, uint32_t size, uint32_t **requestp);
int virgl_bind_object(struct jkg_ctx *ctx, enum virgl_object_type type, uint32_t id);
int virgl_destroy_object(struct jkg_ctx *ctx, enum virgl_object_type type, uint32_t id);
enum virgl_formats virgl_get_format(enum jkg_format format);
int virgl_res_alloc(struct jkg_ctx *ctx, const struct virgl_res_info *info, struct virgl_res **resp);
void virgl_res_free(struct jkg_ctx *ctx, struct virgl_res *res);

void virgl_destroy(struct jkg_ctx *ctx);
int virgl_flush(struct jkg_ctx *ctx);
int virgl_resize(struct jkg_ctx *ctx, uint32_t width, uint32_t height);
int virgl_clear(struct jkg_ctx *ctx, const struct jkg_clear_cmd *cmd);
int virgl_draw(struct jkg_ctx *ctx, const struct jkg_draw_cmd *cmd);
int virgl_get_default_images(struct jkg_ctx *ctx, struct jkg_image **colors, enum jkg_format *color_format, struct jkg_image **depth, enum jkg_format *depth_format, struct jkg_image **stencil, enum jkg_format *stencil_format, uint32_t *width, uint32_t *height);
int virgl_set_index_buffer(struct jkg_ctx *ctx, struct jkg_buffer *buffer, enum jkg_index_type type, uint32_t offset);
int virgl_set_vertex_buffers(struct jkg_ctx *ctx, const struct jkg_vertex_buffer *vertex_buffers, uint32_t count);
int virgl_set_uniform_blocks(struct jkg_ctx *ctx, const struct jkg_uniform_block *uniform_blocks, uint32_t count);
int virgl_set_blend_color(struct jkg_ctx *ctx, const float *color);
int virgl_set_viewport(struct jkg_ctx *ctx, const float *viewport);
int virgl_set_scissor(struct jkg_ctx *ctx, const float *scissor);
int virgl_set_stencil_ref(struct jkg_ctx *ctx, uint8_t front, uint8_t back);

int virgl_buffer_alloc(struct jkg_ctx *ctx, const struct jkg_buffer_create_info *create_info, struct jkg_buffer **bufferp);
int virgl_buffer_read(struct jkg_ctx *ctx, struct jkg_buffer *buffer, void *data, uint32_t size, uint32_t offset);
int virgl_buffer_write(struct jkg_ctx *ctx, struct jkg_buffer *buffer, const void *data, uint32_t size, uint32_t offset);
int virgl_buffer_copy(struct jkg_ctx *ctx, struct jkg_buffer *dst, struct jkg_buffer *src, uint32_t dst_off, uint32_t src_off, uint32_t size);
void virgl_buffer_free(struct jkg_ctx *ctx, struct jkg_buffer *buffer);

int virgl_shader_alloc(struct jkg_ctx *ctx, const struct jkg_shader_create_info *create_info, struct jkg_shader **shaderp);
void virgl_shader_free(struct jkg_ctx *ctx, struct jkg_shader *shader);

int virgl_shader_state_alloc(struct jkg_ctx *ctx, const struct jkg_shader_state_create_info *create_info, struct jkg_shader_state **statep);
int virgl_shader_state_bind(struct jkg_ctx *ctx, struct jkg_shader_state *state);
void virgl_shader_state_free(struct jkg_ctx *ctx, struct jkg_shader_state *state);

int virgl_depth_stencil_state_alloc(struct jkg_ctx *ctx, const struct jkg_depth_stencil_state_create_info *create_info, struct jkg_depth_stencil_state **statep);
int virgl_depth_stencil_state_bind(struct jkg_ctx *ctx, struct jkg_depth_stencil_state *state);
void virgl_depth_stencil_state_free(struct jkg_ctx *ctx, struct jkg_depth_stencil_state *state);

int virgl_rasterizer_state_alloc(struct jkg_ctx *ctx, const struct jkg_rasterizer_state_create_info *create_info, struct jkg_rasterizer_state **statep);
int virgl_rasterizer_state_bind(struct jkg_ctx *ctx, struct jkg_rasterizer_state *state);
void virgl_rasterizer_state_free(struct jkg_ctx *ctx, struct jkg_rasterizer_state *state);

int virgl_blend_state_alloc(struct jkg_ctx *ctx, const struct jkg_blend_state_create_info *create_info, struct jkg_blend_state **statep);
int virgl_blend_state_bind(struct jkg_ctx *ctx, struct jkg_blend_state *state);
void virgl_blend_state_free(struct jkg_ctx *ctx, struct jkg_blend_state *state);

int virgl_input_layout_alloc(struct jkg_ctx *ctx, const struct jkg_input_layout_create_info *create_info, struct jkg_input_layout **input_layoutp);
int virgl_input_layout_bind(struct jkg_ctx *ctx, struct jkg_input_layout *input_layout);
void virgl_input_layout_free(struct jkg_ctx *ctx, struct jkg_input_layout *input_layout);

int virgl_sampler_alloc(struct jkg_ctx *ctx, const struct jkg_sampler_create_info *create_info, struct jkg_sampler **samplerp);
int virgl_sampler_bind(struct jkg_ctx *ctx, struct jkg_sampler **samplers, uint32_t first, uint32_t count);
void virgl_sampler_free(struct jkg_ctx *ctx, struct jkg_sampler *sampler);

int virgl_image_view_alloc(struct jkg_ctx *ctx, const struct jkg_image_view_create_info *create_info, struct jkg_image_view **image_viewp);
int virgl_image_view_bind(struct jkg_ctx *ctx, struct jkg_image_view **image_view, uint32_t first, uint32_t count);
void virgl_image_view_free(struct jkg_ctx *ctx, struct jkg_image_view *image_view);

int virgl_image_alloc(struct jkg_ctx *ctx, const struct jkg_image_create_info *create_info, struct jkg_image **imagep);
int virgl_image_read(struct jkg_ctx *ctx, struct jkg_image *image, uint32_t level, void *data, enum jkg_format format, const struct jkg_extent *size, const struct jkg_extent *offset);
int virgl_image_write(struct jkg_ctx *ctx, struct jkg_image *image, uint32_t level, const void *data, enum jkg_format format, const struct jkg_extent *size, const struct jkg_extent *offset);
int virgl_image_copy(struct jkg_ctx *ctx, struct jkg_image *dst, struct jkg_image *src, uint32_t dst_level, uint32_t src_level, const struct jkg_extent *dst_off, const struct jkg_extent *src_off, const struct jkg_extent *size);
void virgl_image_free(struct jkg_ctx *ctx, struct jkg_image *image);

int virgl_surface_alloc(struct jkg_ctx *ctx, const struct jkg_surface_create_info *create_info, struct jkg_surface **surfacep);
int virgl_surface_bind(struct jkg_ctx *ctx, struct jkg_surface **colors, uint32_t count, struct jkg_surface *depth, struct jkg_surface *stencil);
int virgl_surface_clear(struct jkg_ctx *ctx, struct jkg_surface *surface, const struct jkg_clear_cmd *cmd);
void virgl_surface_free(struct jkg_ctx *ctx, struct jkg_surface *surface);

int vgl_flush(struct jkg_ctx *ctx);
int vgl_submit(struct jkg_ctx *ctx);
int vgl_create_resource(struct jkg_ctx *ctx, struct virgl_res *res);
int vgl_transfer_in(struct jkg_ctx *ctx, struct virgl_res *res, const struct jkg_extent *offset, const struct jkg_extent *size, uint64_t buffer_offset, uint32_t level, uint32_t stride, uint32_t layer_stride);
int vgl_transfer_out(struct jkg_ctx *ctx, struct virgl_res *res, const struct jkg_extent *offset, const struct jkg_extent *size, uint64_t buffer_offset, uint32_t level, uint32_t stride, uint32_t layer_stride);
int vgl_create_context(struct jkg_ctx *ctx);
int vgl_get_capset_info(struct jkg_ctx *ctx, uint32_t *id, uint32_t *version, uint32_t *size);
int vgl_get_capset(struct jkg_ctx *ctx, void *data, size_t size);

static inline uint32_t
fconv(float f)
{
	union
	{
		float f;
		uint32_t u;
	} u =
	{
		.f = f,
	};
	return u.u;
}

static inline uint64_t
dconv(double d)
{
	union
	{
		double d;
		uint64_t u;
	} u =
	{
		.d = d,
	};
	return u.u;
}

#endif
