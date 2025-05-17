#ifndef JKGL_SOFT_H
#define JKGL_SOFT_H

#include "rast/rast.h"
#include "jkg/jkg.h"

#define SOFT_MAX_ATTACHMENTS         32
#define SOFT_MAX_ATTRIBS             32
#define SOFT_MAX_VERTEX_BUFFERS      32
#define SOFT_MAX_TEXTURE_IMAGE_UNITS 32
#define SOFT_MAX_UNIFORM_BLOCKS      32

typedef void (*soft_vertex_shader_fn_t)(struct rast_vert *vert, const void **uniform_blocks);
typedef bool (*soft_fragment_shader_fn_t)(const struct rast_vert *vert, const void **uniform_blocks, float *color);

struct soft_draw_ctx
{
	struct jkg_ctx *ctx;
	const struct jkg_draw_cmd *cmd;
	struct rast_ctx rast;
};

struct jkg_shader
{
	union
	{
		soft_vertex_shader_fn_t vs_fn;
		soft_fragment_shader_fn_t fs_fn;
	};
	uint32_t in;
	uint32_t out;
};

struct jkg_input_layout
{
	struct jkg_input_layout_attrib attribs[SOFT_MAX_ATTRIBS];
	uint32_t nattribs;
};

struct jkg_image
{
	void *data;
	struct jkg_extent size;
	uint32_t levels;
	uint32_t layers;
	enum jkg_format format;
	enum jkg_image_type type;
};

struct jkg_sampler
{
	enum jkg_wrap_mode wrap[3];
	enum jkg_filter_mode min_filter;
	enum jkg_filter_mode mag_filter;
	enum jkg_filter_mode mip_filter;
	bool compare_enable;
	enum jkg_compare_op compare_func;
	float lod_bias;
	float min_lod;
	float max_lod;
	uint32_t max_anisotropy;
	float border_color[4];
};

struct jkg_image_view
{
	struct jkg_image *image;
	uint32_t min_level;
	uint32_t num_levels;
	uint32_t min_layer;
	uint32_t num_layers;
	enum jkg_format format;
	enum jkg_swizzle swizzle[4];
};

struct jkg_surface
{
	struct jkg_image *image;
	uint32_t level;
	uint32_t min_layer;
	uint32_t num_layers;
};

struct jkg_buffer
{
	void *data;
	uint32_t size;
};

struct jkg_blend_state
{
	bool logic_enable;
	enum jkg_logic_op logic_op;
	struct jkg_blend_attachment attachments[SOFT_MAX_ATTACHMENTS];
	uint32_t nattachments;
};

struct jkg_depth_stencil_state
{
	bool depth_test_enable;
	bool depth_write_enable;
	enum jkg_compare_op depth_compare;
	bool stencil_enable;
	struct jkg_stencil_op_state front;
	struct jkg_stencil_op_state back;
};

struct jkg_shader_state
{
	struct jkg_shader *vs;
	struct jkg_shader *fs;
};

struct jkg_rasterizer_state
{
	float point_size;
	float line_width;
	bool point_smooth_enable;
	bool line_smooth_enable;
	bool scissor_enable;
	bool depth_clamp_enable;
	bool rasterizer_discard_enable;
	bool multisample_enable;
	bool front_ccw;
	enum jkg_cull_face cull_face;
	enum jkg_fill_mode fill_front;
	enum jkg_fill_mode fill_back;
};

struct jkg_ctx
{
	const struct jkg_op *op;
	const struct jkg_caps *caps;
	const struct jkg_depth_stencil_state *depth_stencil_state;
	const struct jkg_blend_state *blend_state;
	float blend_color[4];
	float viewport[6];
	float scissor[4];
	uint8_t stencil_ref[2];
	struct jkg_surface *color_surfaces[SOFT_MAX_ATTACHMENTS];
	struct jkg_surface *depth_surface;
	struct jkg_surface *stencil_surface;
	struct jkg_shader_state *shader_state;
	enum jkg_index_type index_type;
	uint32_t index_offset;
	struct jkg_buffer *index_buffer;
	struct jkg_rasterizer_state *rasterizer_state;
	struct jkg_input_layout *input_layout;
	struct jkg_vertex_buffer vertex_buffers[SOFT_MAX_VERTEX_BUFFERS];
	struct jkg_sampler *samplers[SOFT_MAX_TEXTURE_IMAGE_UNITS];
	struct jkg_image_view *image_views[SOFT_MAX_TEXTURE_IMAGE_UNITS];
	struct jkg_uniform_block uniform_blocks[SOFT_MAX_UNIFORM_BLOCKS];
	const void *uniform_data[SOFT_MAX_UNIFORM_BLOCKS];
	struct jkg_image *default_color_images[2];
	struct jkg_image *default_depth_stencil_image;
	uint32_t width;
	uint32_t height;
};

void soft_destroy(struct jkg_ctx *ctx);
int soft_flush(struct jkg_ctx *ctx);
int soft_resize(struct jkg_ctx *ctx, uint32_t width, uint32_t height);
int soft_clear(struct jkg_ctx *ctx, const struct jkg_clear_cmd *cmd);
int soft_draw(struct jkg_ctx *ctx, const struct jkg_draw_cmd *cmd);
int soft_get_default_images(struct jkg_ctx *ctx, struct jkg_image **color, enum jkg_format *color_format, struct jkg_image **depth, enum jkg_format *depth_format, struct jkg_image **stencil, enum jkg_format *stencil_format, uint32_t *width, uint32_t *height);
int soft_set_index_buffer(struct jkg_ctx *ctx, struct jkg_buffer *buffer, enum jkg_index_type type, uint32_t offset);
int soft_set_vertex_buffers(struct jkg_ctx *ctx, const struct jkg_vertex_buffer *vertex_buffers, uint32_t count);
int soft_set_uniform_blocks(struct jkg_ctx *ctx, const struct jkg_uniform_block *uniform_blocks, uint32_t count);
int soft_set_blend_color(struct jkg_ctx *ctx, const float *color);
int soft_set_viewport(struct jkg_ctx *ctx, const float *viewport);
int soft_set_scissor(struct jkg_ctx *ctx, const float *scissor);
int soft_set_stencil_ref(struct jkg_ctx *ctx, uint8_t front, uint8_t back);
void soft_sample(struct jkg_ctx *ctx, uint32_t id, const float coord[3], float color[4]);

int soft_buffer_alloc(struct jkg_ctx *ctx, const struct jkg_buffer_create_info *create_info, struct jkg_buffer **bufferp);
int soft_buffer_read(struct jkg_ctx *ctx, struct jkg_buffer *buffer, void *data, uint32_t size, uint32_t offset);
int soft_buffer_write(struct jkg_ctx *ctx, struct jkg_buffer *buffer, const void *data, uint32_t size, uint32_t offset);
int soft_buffer_copy(struct jkg_ctx *ctx, struct jkg_buffer *dst, struct jkg_buffer *src, uint32_t dst_off, uint32_t src_off, uint32_t size);
void soft_buffer_free(struct jkg_ctx *ctx, struct jkg_buffer *buffer);

int soft_shader_alloc(struct jkg_ctx *ctx, const struct jkg_shader_create_info *create_info, struct jkg_shader **shaderp);
void soft_shader_free(struct jkg_ctx *ctx, struct jkg_shader *shader);

int soft_shader_state_alloc(struct jkg_ctx *ctx, const struct jkg_shader_state_create_info *create_info, struct jkg_shader_state **statep);
int soft_shader_state_bind(struct jkg_ctx *ctx, struct jkg_shader_state *state);
void soft_shader_state_free(struct jkg_ctx *ctx, struct jkg_shader_state *state);

int soft_depth_stencil_state_alloc(struct jkg_ctx *ctx, const struct jkg_depth_stencil_state_create_info *create_info, struct jkg_depth_stencil_state **statep);
int soft_depth_stencil_state_bind(struct jkg_ctx *ctx, struct jkg_depth_stencil_state *state);
void soft_depth_stencil_state_free(struct jkg_ctx *ctx, struct jkg_depth_stencil_state *state);

int soft_rasterizer_state_alloc(struct jkg_ctx *ctx, const struct jkg_rasterizer_state_create_info *create_info, struct jkg_rasterizer_state **statep);
int soft_rasterizer_state_bind(struct jkg_ctx *ctx, struct jkg_rasterizer_state *state);
void soft_rasterizer_state_free(struct jkg_ctx *ctx, struct jkg_rasterizer_state *state);

int soft_blend_state_alloc(struct jkg_ctx *ctx, const struct jkg_blend_state_create_info *create_info, struct jkg_blend_state **statep);
int soft_blend_state_bind(struct jkg_ctx *ctx, struct jkg_blend_state *state);
void soft_blend_state_free(struct jkg_ctx *ctx, struct jkg_blend_state *state);

int soft_input_layout_alloc(struct jkg_ctx *ctx, const struct jkg_input_layout_create_info *create_info, struct jkg_input_layout **input_layoutp);
int soft_input_layout_bind(struct jkg_ctx *ctx, struct jkg_input_layout *input_layout);
void soft_input_layout_free(struct jkg_ctx *ctx, struct jkg_input_layout *input_layout);

int soft_sampler_alloc(struct jkg_ctx *ctx, const struct jkg_sampler_create_info *create_info, struct jkg_sampler **samplerp);
int soft_sampler_bind(struct jkg_ctx *ctx, struct jkg_sampler **samplers, uint32_t first, uint32_t count);
void soft_sampler_free(struct jkg_ctx *ctx, struct jkg_sampler *sampler);

int soft_image_view_alloc(struct jkg_ctx *ctx, const struct jkg_image_view_create_info *create_info, struct jkg_image_view **image_viewp);
int soft_image_view_bind(struct jkg_ctx *ctx, struct jkg_image_view **image_view, uint32_t first, uint32_t count);
void soft_image_view_free(struct jkg_ctx *ctx, struct jkg_image_view *image_view);

int soft_image_alloc(struct jkg_ctx *ctx, const struct jkg_image_create_info *create_info, struct jkg_image **imagep);
int soft_image_read(struct jkg_ctx *ctx, struct jkg_image *image, uint32_t level, void *data, enum jkg_format format, const struct jkg_extent *size, const struct jkg_extent *offset);
int soft_image_write(struct jkg_ctx *ctx, struct jkg_image *image, uint32_t level, const void *data, enum jkg_format format, const struct jkg_extent *size, const struct jkg_extent *offset);
int soft_image_copy(struct jkg_ctx *ctx, struct jkg_image *dst, struct jkg_image *src, uint32_t dst_level, uint32_t src_level, const struct jkg_extent *dst_off, const struct jkg_extent *src_off, const struct jkg_extent *size);
void soft_image_free(struct jkg_ctx *ctx, struct jkg_image *image);

int soft_surface_alloc(struct jkg_ctx *ctx, const struct jkg_surface_create_info *create_info, struct jkg_surface **surfacep);
int soft_surface_bind(struct jkg_ctx *ctx, struct jkg_surface **colors, uint32_t count, struct jkg_surface *depth, struct jkg_surface *stencil);
int soft_surface_clear(struct jkg_ctx *ctx, struct jkg_surface *surface, const struct jkg_clear_cmd *cmd);
void soft_surface_free(struct jkg_ctx *ctx, struct jkg_surface *surface);

void soft_texture_sample(struct jkg_ctx *ctx, const struct jkg_image_view *image_view, const struct jkg_sampler *sampler, const float *coord, float *color);
void soft_fragment(struct rast_ctx *rast_ctx, struct rast_vert *vert);

#endif
