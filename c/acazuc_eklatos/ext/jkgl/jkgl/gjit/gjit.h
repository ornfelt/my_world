#ifndef JKGL_GJIT_H
#define JKGL_GJIT_H

#include "rast/rast.h"
#include "jkg/jkg.h"

#include <libgccjit.h>

#define GJIT_MAX_ATTACHMENTS         32
#define GJIT_MAX_ATTRIBS             32
#define GJIT_MAX_VERTEX_BUFFERS      32
#define GJIT_MAX_TEXTURE_IMAGE_UNITS 32
#define GJIT_MAX_UNIFORM_BLOCKS      32

typedef void (*gjit_blend_fn_t)(float *res, const float *src, const float *dst, const float *constant);
typedef void (*gjit_input_layout_fn_t)(float *vert, const struct jkg_vertex_buffer *buffers, uint32_t indice, uint32_t instance);
typedef void (*gjit_vertex_shader_fn_t)(struct rast_vert *vert, const void **uniform_blocks);
typedef bool (*gjit_fragment_shader_fn_t)(const struct rast_vert *vert, const void **uniform_blocks, float *color);
typedef void (*gjit_image_view_fn_t)(float *color, const int32_t *coord);
typedef void (*gjit_sampler_fn_t)(float *color, const float *coord, const int32_t *size, gjit_image_view_fn_t image_view_fn);

struct gjit_draw_ctx
{
	struct jkg_ctx *ctx;
	const struct jkg_draw_cmd *cmd;
	struct rast_ctx rast;
};

struct jkg_shader
{
	union
	{
		gjit_vertex_shader_fn_t vs_fn;
		gjit_fragment_shader_fn_t fs_fn;
	};
	uint32_t in;
	uint32_t out;
};

struct jkg_input_layout
{
	gcc_jit_result *res;
	gjit_input_layout_fn_t fn;
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
	gcc_jit_result *res;
	gjit_sampler_fn_t fn;
	enum jkg_filter_mode min_filter;
	enum jkg_filter_mode mag_filter;
	enum jkg_filter_mode mip_filter;
	bool compare_enable;
	enum jkg_compare_op compare_func;
	float lod_bias;
	float min_lod;
	float max_lod;
	uint32_t max_anisotropy;
};

struct jkg_image_view
{
	gcc_jit_result *res;
	gjit_image_view_fn_t fn;
	struct jkg_image *image;
	uint32_t min_level;
	uint32_t num_levels;
	uint32_t min_layer;
	uint32_t num_layers;
};

struct jkg_surface
{
	gcc_jit_result *res;
	gjit_image_view_fn_t fn;
	struct jkg_image *image;
	uint32_t level;
	uint32_t min_layer;
	uint32_t num_layers;
};

/* gjit defined */
struct jkg_buffer
{
	void *data;
	uint32_t size;
};

struct jkg_blend_state
{
	bool logic_enable;
	enum jkg_logic_op logic_op;
	struct jkg_blend_attachment attachments[GJIT_MAX_ATTACHMENTS];
	uint32_t nattachments;
	gcc_jit_result *res;
	gjit_blend_fn_t fn;
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

struct jit_ctx
{
	gcc_jit_context *ctx;
	gcc_jit_type *type_void;
	gcc_jit_type *type_voidp;
	gcc_jit_type *type_float;
	gcc_jit_type *type_floatp;
	gcc_jit_type *type_const_floatp;
	gcc_jit_type *type_restrict_floatp;
	gcc_jit_type *type_const_restrict_floatp;
	gcc_jit_type *type_float_3;
	gcc_jit_type *type_float_4;
	gcc_jit_type *type_float_8;
	gcc_jit_type *type_float_16;
	gcc_jit_type *type_double;
	gcc_jit_type *type_doublep;
	gcc_jit_type *type_const_doublep;
	gcc_jit_type *type_restrict_doublep;
	gcc_jit_type *type_const_restrict_doublep;
	gcc_jit_type *type_int;
	gcc_jit_type *type_int8;
	gcc_jit_type *type_int8p;
	gcc_jit_type *type_const_int8p;
	gcc_jit_type *type_const_restrict_int8p;
	gcc_jit_type *type_uint8;
	gcc_jit_type *type_uint8p;
	gcc_jit_type *type_const_uint8p;
	gcc_jit_type *type_const_restrict_uint8p;
	gcc_jit_type *type_int16;
	gcc_jit_type *type_int16p;
	gcc_jit_type *type_const_int16p;
	gcc_jit_type *type_const_restrict_int16p;
	gcc_jit_type *type_uint16;
	gcc_jit_type *type_uint16p;
	gcc_jit_type *type_const_uint16p;
	gcc_jit_type *type_const_restrict_uint16p;
	gcc_jit_type *type_int32;
	gcc_jit_type *type_int32p;
	gcc_jit_type *type_const_int32p;
	gcc_jit_type *type_const_restrict_int32p;
	gcc_jit_type *type_int32_1;
	gcc_jit_type *type_int32_2;
	gcc_jit_type *type_int32_3;
	gcc_jit_type *type_int32_4;
	gcc_jit_type *type_int32_6;
	gcc_jit_type *type_int32_12;
	gcc_jit_type *type_uint32;
	gcc_jit_type *type_uint32p;
	gcc_jit_type *type_const_uint32p;
	gcc_jit_type *type_const_restrict_uint32p;
	gcc_jit_type *type_size_t;
	gcc_jit_field *type_buffer_field_data;
	gcc_jit_field *type_buffer_field_size;
	gcc_jit_struct *type_buffer;
	gcc_jit_type *type_bufferp;
	gcc_jit_field *type_vertex_buffer_field_stride;
	gcc_jit_field *type_vertex_buffer_field_offset;
	gcc_jit_field *type_vertex_buffer_field_buffer;
	gcc_jit_struct *type_vertex_buffer;
	gcc_jit_type *type_vertex_bufferp;
	gcc_jit_type *type_image_view_fn;
	gcc_jit_rvalue *value_inv_65535f;
	gcc_jit_rvalue *value_inv_32767f;
	gcc_jit_rvalue *value_inv_255f;
	gcc_jit_rvalue *value_inv_127f;
	gcc_jit_rvalue *value_inv_63f;
	gcc_jit_rvalue *value_inv_31f;
	gcc_jit_rvalue *value_inv_15f;
	gcc_jit_rvalue *value_minus_one;
	gcc_jit_rvalue *value_f[4];
	gcc_jit_rvalue *value_i32[5];
	struct
	{
		gcc_jit_context *ctx;
		gcc_jit_function *wrap[JKG_WRAP_MIRROR_CLAMP_TO_EDGE + 1];
		gcc_jit_function *get_filter_coord;
		gcc_jit_function *linear_interpolate;
		gcc_jit_function *cubic_interpolate;
		gcc_jit_function *fetch;
		gcc_jit_function *nearest[3];
		gcc_jit_function *linear[3];
		gcc_jit_function *cubic[3];
	} sampler;
	struct
	{
		gcc_jit_context *ctx;
		gcc_jit_function *fn_rgb[JKG_BLEND_ONE_MINUS_SRC1_ALPHA + 1];
		gcc_jit_function *fn_alpha[JKG_BLEND_ONE_MINUS_SRC1_ALPHA + 1];
		gcc_jit_function *eq_rgb[JKG_EQUATION_MAX + 1];
		gcc_jit_function *eq_alpha[JKG_EQUATION_MAX + 1];
	} blend;
};

struct jkg_ctx
{
	const struct jkg_op *op;
	const struct jkg_caps *caps;
	struct jit_ctx jit;
	const struct jkg_depth_stencil_state *depth_stencil_state;
	struct jkg_blend_state *blend_state;
	float blend_color[4];
	float viewport[6];
	float scissor[4];
	uint8_t stencil_ref[2];
	struct jkg_surface *color_surfaces[GJIT_MAX_ATTACHMENTS];
	struct jkg_surface *depth_surface;
	struct jkg_surface *stencil_surface;
	struct jkg_shader_state *shader_state;
	enum jkg_index_type index_type;
	uint32_t index_offset;
	struct jkg_buffer *index_buffer;
	struct jkg_rasterizer_state *rasterizer_state;
	struct jkg_input_layout *input_layout;
	struct jkg_vertex_buffer vertex_buffers[GJIT_MAX_VERTEX_BUFFERS];
	struct jkg_sampler *samplers[GJIT_MAX_TEXTURE_IMAGE_UNITS];
	struct jkg_image_view *image_views[GJIT_MAX_TEXTURE_IMAGE_UNITS];
	struct jkg_uniform_block uniform_blocks[GJIT_MAX_UNIFORM_BLOCKS];
	const void *uniform_data[GJIT_MAX_UNIFORM_BLOCKS];
	struct jkg_image *default_color_images[2];
	struct jkg_image *default_depth_stencil_image;
	uint32_t width;
	uint32_t height;
};

void gjit_destroy(struct jkg_ctx *ctx);
int gjit_flush(struct jkg_ctx *ctx);
int gjit_resize(struct jkg_ctx *ctx, uint32_t width, uint32_t height);
int gjit_clear(struct jkg_ctx *ctx, const struct jkg_clear_cmd *cmd);
int gjit_draw(struct jkg_ctx *ctx, const struct jkg_draw_cmd *cmd);
int gjit_get_default_images(struct jkg_ctx *ctx, struct jkg_image **colors, enum jkg_format *color_format, struct jkg_image **depth, enum jkg_format *depth_format, struct jkg_image **stencil, enum jkg_format *stencil_format, uint32_t *width, uint32_t *height);
int gjit_set_index_buffer(struct jkg_ctx *ctx, struct jkg_buffer *buffer, enum jkg_index_type type, uint32_t offset);
int gjit_set_vertex_buffers(struct jkg_ctx *ctx, const struct jkg_vertex_buffer *vertex_buffers, uint32_t count);
int gjit_set_uniform_blocks(struct jkg_ctx *ctx, const struct jkg_uniform_block *uniform_blocks, uint32_t count);
int gjit_set_blend_color(struct jkg_ctx *ctx, const float *color);
int gjit_set_viewport(struct jkg_ctx *ctx, const float *viewport);
int gjit_set_scissor(struct jkg_ctx *ctx, const float *scissor);
int gjit_set_stencil_ref(struct jkg_ctx *ctx, uint8_t front, uint8_t back);
void gjit_sample(struct jkg_ctx *ctx, uint32_t id, const float coord[3], float color[4]);

int gjit_buffer_alloc(struct jkg_ctx *ctx, const struct jkg_buffer_create_info *create_info, struct jkg_buffer **bufferp);
int gjit_buffer_read(struct jkg_ctx *ctx, struct jkg_buffer *buffer, void *data, uint32_t size, uint32_t offset);
int gjit_buffer_write(struct jkg_ctx *ctx, struct jkg_buffer *buffer, const void *data, uint32_t size, uint32_t offset);
int gjit_buffer_copy(struct jkg_ctx *ctx, struct jkg_buffer *dst, struct jkg_buffer *src, uint32_t dst_off, uint32_t src_off, uint32_t size);
void gjit_buffer_free(struct jkg_ctx *ctx, struct jkg_buffer *buffer);

int gjit_shader_alloc(struct jkg_ctx *ctx, const struct jkg_shader_create_info *create_info, struct jkg_shader **shaderp);
void gjit_shader_free(struct jkg_ctx *ctx, struct jkg_shader *shader);

int gjit_shader_state_alloc(struct jkg_ctx *ctx, const struct jkg_shader_state_create_info *create_info, struct jkg_shader_state **statep);
int gjit_shader_state_bind(struct jkg_ctx *ctx, struct jkg_shader_state *state);
void gjit_shader_state_free(struct jkg_ctx *ctx, struct jkg_shader_state *state);

int gjit_depth_stencil_state_alloc(struct jkg_ctx *ctx, const struct jkg_depth_stencil_state_create_info *create_info, struct jkg_depth_stencil_state **statep);
int gjit_depth_stencil_state_bind(struct jkg_ctx *ctx, struct jkg_depth_stencil_state *state);
void gjit_depth_stencil_state_free(struct jkg_ctx *ctx, struct jkg_depth_stencil_state *state);

int gjit_rasterizer_state_alloc(struct jkg_ctx *ctx, const struct jkg_rasterizer_state_create_info *create_info, struct jkg_rasterizer_state **statep);
int gjit_rasterizer_state_bind(struct jkg_ctx *ctx, struct jkg_rasterizer_state *state);
void gjit_rasterizer_state_free(struct jkg_ctx *ctx, struct jkg_rasterizer_state *state);

void gjit_blend_init(struct jkg_ctx *ctx);
int gjit_blend_state_alloc(struct jkg_ctx *ctx, const struct jkg_blend_state_create_info *create_info, struct jkg_blend_state **statep);
int gjit_blend_state_bind(struct jkg_ctx *ctx, struct jkg_blend_state *state);
void gjit_blend_state_free(struct jkg_ctx *ctx, struct jkg_blend_state *state);

int gjit_input_layout_alloc(struct jkg_ctx *ctx, const struct jkg_input_layout_create_info *create_info, struct jkg_input_layout **input_layoutp);
int gjit_input_layout_bind(struct jkg_ctx *ctx, struct jkg_input_layout *input_layout);
void gjit_input_layout_free(struct jkg_ctx *ctx, struct jkg_input_layout *input_layout);

void gjit_sampler_init(struct jkg_ctx *ctx);
int gjit_sampler_alloc(struct jkg_ctx *ctx, const struct jkg_sampler_create_info *create_info, struct jkg_sampler **samplerp);
int gjit_sampler_bind(struct jkg_ctx *ctx, struct jkg_sampler **samplers, uint32_t first, uint32_t count);
void gjit_sampler_free(struct jkg_ctx *ctx, struct jkg_sampler *sampler);

int gjit_image_view_alloc(struct jkg_ctx *ctx, const struct jkg_image_view_create_info *create_info, struct jkg_image_view **image_viewp);
int gjit_image_view_bind(struct jkg_ctx *ctx, struct jkg_image_view **image_view, uint32_t first, uint32_t count);
void gjit_image_view_free(struct jkg_ctx *ctx, struct jkg_image_view *image_view);

int gjit_image_alloc(struct jkg_ctx *ctx, const struct jkg_image_create_info *create_info, struct jkg_image **imagep);
int gjit_image_read(struct jkg_ctx *ctx, struct jkg_image *image, uint32_t level, void *data, enum jkg_format format, const struct jkg_extent *size, const struct jkg_extent *offset);
int gjit_image_write(struct jkg_ctx *ctx, struct jkg_image *image, uint32_t level, const void *data, enum jkg_format format, const struct jkg_extent *size, const struct jkg_extent *offset);
int gjit_image_copy(struct jkg_ctx *ctx, struct jkg_image *dst, struct jkg_image *src, uint32_t dst_level, uint32_t src_level, const struct jkg_extent *dst_off, const struct jkg_extent *src_off, const struct jkg_extent *size);
void gjit_image_free(struct jkg_ctx *ctx, struct jkg_image *image);

int gjit_surface_alloc(struct jkg_ctx *ctx, const struct jkg_surface_create_info *create_info, struct jkg_surface **surfacep);
int gjit_surface_bind(struct jkg_ctx *ctx, struct jkg_surface **colors, uint32_t count, struct jkg_surface *depth, struct jkg_surface *stencil);
int gjit_surface_clear(struct jkg_ctx *ctx, struct jkg_surface *surface, const struct jkg_clear_cmd *cmd);
void gjit_surface_free(struct jkg_ctx *ctx, struct jkg_surface *surface);

void gjit_get_vec4f(struct jkg_ctx *ctx, gcc_jit_context *jit_ctx, gcc_jit_rvalue **dst, gcc_jit_rvalue *data, enum jkg_format format);
void gjit_fragment(struct rast_ctx *rast_ctx, struct rast_vert *vert);

#endif
