#ifndef JKGL_JKG_H /* la fameuse économiste */
#define JKGL_JKG_H

#include <sys/types.h>
#include <sys/queue.h>

#include <stdbool.h>
#include <stdint.h>

struct jkg_depth_stencil_state;
struct jkg_rasterizer_state;
struct jkg_input_layout;
struct jkg_shader_state;
struct jkg_blend_state;
struct jkg_image_view;
struct jkg_sampler;
struct jkg_surface;
struct jkg_buffer;
struct jkg_shader;
struct jkg_image;
struct jkg_ctx;

enum jkg_clear_mask
{
	JKG_CLEAR_COLOR   = (1 << 0),
	JKG_CLEAR_DEPTH   = (1 << 1),
	JKG_CLEAR_STENCIL = (1 << 2),
};

enum jkg_blend_fn
{
	JKG_BLEND_ZERO,
	JKG_BLEND_ONE,
	JKG_BLEND_SRC_COLOR,
	JKG_BLEND_ONE_MINUS_SRC_COLOR,
	JKG_BLEND_DST_COLOR,
	JKG_BLEND_ONE_MINUS_DST_COLOR,
	JKG_BLEND_SRC_ALPHA,
	JKG_BLEND_ONE_MINUS_SRC_ALPHA,
	JKG_BLEND_DST_ALPHA,
	JKG_BLEND_ONE_MINUS_DST_ALPHA,
	JKG_BLEND_CONSTANT_COLOR,
	JKG_BLEND_ONE_MINUS_CONSTANT_COLOR,
	JKG_BLEND_CONSTANT_ALPHA,
	JKG_BLEND_ONE_MINUS_CONSTANT_ALPHA,
	JKG_BLEND_SRC_ALPHA_SATURATE,
	JKG_BLEND_SRC1_COLOR,
	JKG_BLEND_ONE_MINUS_SRC1_COLOR,
	JKG_BLEND_SRC1_ALPHA,
	JKG_BLEND_ONE_MINUS_SRC1_ALPHA,
};

enum jkg_blend_eq
{
	JKG_EQUATION_ADD,
	JKG_EQUATION_SUBTRACT,
	JKG_EQUATION_REV_SUBTRACT,
	JKG_EQUATION_MIN,
	JKG_EQUATION_MAX,
};

enum jkg_color_mask
{
	JKG_COLOR_MASK_R = (1 << 0),
	JKG_COLOR_MASK_G = (1 << 1),
	JKG_COLOR_MASK_B = (1 << 2),
	JKG_COLOR_MASK_A = (1 << 3),
};

enum jkg_logic_op
{
	JKG_LOGIC_CLEAR,
	JKG_LOGIC_SET,
	JKG_LOGIC_COPY,
	JKG_LOGIC_COPY_INVERTED,
	JKG_LOGIC_NOOP,
	JKG_LOGIC_INVERT,
	JKG_LOGIC_AND,
	JKG_LOGIC_NAND,
	JKG_LOGIC_OR,
	JKG_LOGIC_NOR,
	JKG_LOGIC_XOR,
	JKG_LOGIC_EQUIV,
	JKG_LOGIC_AND_REVERSE,
	JKG_LOGIC_AND_INVERTED,
	JKG_LOGIC_OR_REVERSE,
	JKG_LOGIC_OR_INVERTED,
};

enum jkg_compare_op
{
	JKG_COMPARE_NEVER,
	JKG_COMPARE_LOWER,
	JKG_COMPARE_LEQUAL,
	JKG_COMPARE_EQUAL,
	JKG_COMPARE_GEQUAL,
	JKG_COMPARE_GREATER,
	JKG_COMPARE_NOTEQUAL,
	JKG_COMPARE_ALWAYS,
};

enum jkg_stencil_op
{
	JKG_STENCIL_KEEP,
	JKG_STENCIL_ZERO,
	JKG_STENCIL_REPLACE,
	JKG_STENCIL_INC,
	JKG_STENCIL_INC_WRAP,
	JKG_STENCIL_DEC,
	JKG_STENCIL_DEC_WRAP,
	JKG_STENCIL_INV,
};

enum jkg_wrap_mode
{
	JKG_WRAP_REPEAT,
	JKG_WRAP_MIRRORED_REPEAT,
	JKG_WRAP_CLAMP_TO_EDGE,
	JKG_WRAP_CLAMP_TO_BORDER,
	JKG_WRAP_MIRROR_CLAMP_TO_EDGE,
};

enum jkg_filter_mode
{
	JKG_FILTER_NONE,
	JKG_FILTER_NEAREST,
	JKG_FILTER_LINEAR,
	JKG_FILTER_CUBIC,
};

enum jkg_format_type
{
	JKG_FORMAT_TYPE_UNKNOWN,
	JKG_FORMAT_TYPE_COLOR_UINT,
	JKG_FORMAT_TYPE_COLOR_SINT,
	JKG_FORMAT_TYPE_COLOR_UNORM,
	JKG_FORMAT_TYPE_COLOR_SNORM,
	JKG_FORMAT_TYPE_COLOR_USCALED,
	JKG_FORMAT_TYPE_COLOR_SSCALED,
	JKG_FORMAT_TYPE_COLOR_SFLOAT,
	JKG_FORMAT_TYPE_COLOR_UNORM_BLOCK,
	JKG_FORMAT_TYPE_COLOR_SNORM_BLOCK,
	JKG_FORMAT_TYPE_DEPTH_SFLOAT,
	JKG_FORMAT_TYPE_STENCIL_UINT,
	JKG_FORMAT_TYPE_DEPTH_UNORM_STENCIL_UINT,
};

enum jkg_format
{
	JKG_FORMAT_UNKNOWN,
	JKG_FORMAT_R8_UINT,
	JKG_FORMAT_R8_SINT,
	JKG_FORMAT_R8_UNORM,
	JKG_FORMAT_R8_SNORM,
	JKG_FORMAT_R8_USCALED,
	JKG_FORMAT_R8_SSCALED,
	JKG_FORMAT_R8G8_UINT,
	JKG_FORMAT_R8G8_SINT,
	JKG_FORMAT_R8G8_UNORM,
	JKG_FORMAT_R8G8_SNORM,
	JKG_FORMAT_R8G8_USCALED,
	JKG_FORMAT_R8G8_SSCALED,
	JKG_FORMAT_R8G8B8_UINT,
	JKG_FORMAT_R8G8B8_SINT,
	JKG_FORMAT_R8G8B8_UNORM,
	JKG_FORMAT_R8G8B8_SNORM,
	JKG_FORMAT_R8G8B8_USCALED,
	JKG_FORMAT_R8G8B8_SSCALED,
	JKG_FORMAT_B8G8R8_UINT,
	JKG_FORMAT_B8G8R8_SINT,
	JKG_FORMAT_B8G8R8_UNORM,
	JKG_FORMAT_B8G8R8_SNORM,
	JKG_FORMAT_B8G8R8_USCALED,
	JKG_FORMAT_B8G8R8_SSCALED,
	JKG_FORMAT_R8G8B8A8_UINT,
	JKG_FORMAT_R8G8B8A8_SINT,
	JKG_FORMAT_R8G8B8A8_UNORM,
	JKG_FORMAT_R8G8B8A8_SNORM,
	JKG_FORMAT_R8G8B8A8_USCALED,
	JKG_FORMAT_R8G8B8A8_SSCALED,
	JKG_FORMAT_B8G8R8A8_UINT,
	JKG_FORMAT_B8G8R8A8_SINT,
	JKG_FORMAT_B8G8R8A8_UNORM,
	JKG_FORMAT_B8G8R8A8_SNORM,
	JKG_FORMAT_B8G8R8A8_USCALED,
	JKG_FORMAT_B8G8R8A8_SSCALED,
	JKG_FORMAT_R16_UINT,
	JKG_FORMAT_R16_SINT,
	JKG_FORMAT_R16_UNORM,
	JKG_FORMAT_R16_SNORM,
	JKG_FORMAT_R16_USCALED,
	JKG_FORMAT_R16_SSCALED,
	JKG_FORMAT_R16_SFLOAT,
	JKG_FORMAT_R16G16_UINT,
	JKG_FORMAT_R16G16_SINT,
	JKG_FORMAT_R16G16_UNORM,
	JKG_FORMAT_R16G16_SNORM,
	JKG_FORMAT_R16G16_USCALED,
	JKG_FORMAT_R16G16_SSCALED,
	JKG_FORMAT_R16G16_SFLOAT,
	JKG_FORMAT_R16G16B16_UINT,
	JKG_FORMAT_R16G16B16_SINT,
	JKG_FORMAT_R16G16B16_UNORM,
	JKG_FORMAT_R16G16B16_SNORM,
	JKG_FORMAT_R16G16B16_USCALED,
	JKG_FORMAT_R16G16B16_SSCALED,
	JKG_FORMAT_R16G16B16_SFLOAT,
	JKG_FORMAT_R16G16B16A16_UINT,
	JKG_FORMAT_R16G16B16A16_SINT,
	JKG_FORMAT_R16G16B16A16_UNORM,
	JKG_FORMAT_R16G16B16A16_SNORM,
	JKG_FORMAT_R16G16B16A16_USCALED,
	JKG_FORMAT_R16G16B16A16_SSCALED,
	JKG_FORMAT_R16G16B16A16_SFLOAT,
	JKG_FORMAT_R32_UINT,
	JKG_FORMAT_R32_SINT,
	JKG_FORMAT_R32_SFLOAT,
	JKG_FORMAT_R32G32_UINT,
	JKG_FORMAT_R32G32_SINT,
	JKG_FORMAT_R32G32_SFLOAT,
	JKG_FORMAT_R32G32B32_UINT,
	JKG_FORMAT_R32G32B32_SINT,
	JKG_FORMAT_R32G32B32_SFLOAT,
	JKG_FORMAT_R32G32B32A32_UINT,
	JKG_FORMAT_R32G32B32A32_SINT,
	JKG_FORMAT_R32G32B32A32_SFLOAT,
	JKG_FORMAT_R64_UINT,
	JKG_FORMAT_R64_SINT,
	JKG_FORMAT_R64_SFLOAT,
	JKG_FORMAT_R64G64_UINT,
	JKG_FORMAT_R64G64_SINT,
	JKG_FORMAT_R64G64_SFLOAT,
	JKG_FORMAT_R64G64B64_UINT,
	JKG_FORMAT_R64G64B64_SINT,
	JKG_FORMAT_R64G64B64_SFLOAT,
	JKG_FORMAT_R64G64B64A64_UINT,
	JKG_FORMAT_R64G64B64A64_SINT,
	JKG_FORMAT_R64G64B64A64_SFLOAT,
	JKG_FORMAT_D24_UNORM_S8_UINT,
	JKG_FORMAT_D32_SFLOAT,
	JKG_FORMAT_S8_UINT,
	JKG_FORMAT_BC1_RGB_UNORM_BLOCK,
	JKG_FORMAT_BC1_RGBA_UNORM_BLOCK,
	JKG_FORMAT_BC2_UNORM_BLOCK,
	JKG_FORMAT_BC3_UNORM_BLOCK,
	JKG_FORMAT_BC4_UNORM_BLOCK,
	JKG_FORMAT_BC4_SNORM_BLOCK,
	JKG_FORMAT_BC5_UNORM_BLOCK,
	JKG_FORMAT_BC5_SNORM_BLOCK,
	JKG_FORMAT_R4G4_UNORM_PACK8,
	JKG_FORMAT_R4G4B4A4_UNORM_PACK16,
	JKG_FORMAT_B4G4R4A4_UNORM_PACK16,
	JKG_FORMAT_A4R4G4B4_UNORM_PACK16,
	JKG_FORMAT_A4B4G4R4_UNORM_PACK16,
	JKG_FORMAT_R5G6B5_UNORM_PACK16,
	JKG_FORMAT_B5G6R5_UNORM_PACK16,
	JKG_FORMAT_R5G5B5A1_UNORM_PACK16,
	JKG_FORMAT_B5G5R5A1_UNORM_PACK16,
	JKG_FORMAT_A1R5G5B5_UNORM_PACK16,
	JKG_FORMAT_A1B5G5R5_UNORM_PACK16,
};

enum jkg_image_type
{
	JKG_IMAGE_1D,
	JKG_IMAGE_1D_ARRAY,
	JKG_IMAGE_2D,
	JKG_IMAGE_2D_ARRAY,
	JKG_IMAGE_3D,
};

enum jkg_primitive
{
	JKG_PRIMITIVE_POINTS,
	JKG_PRIMITIVE_LINES,
	JKG_PRIMITIVE_LINE_LOOP,
	JKG_PRIMITIVE_LINE_STRIP,
	JKG_PRIMITIVE_TRIANGLES,
	JKG_PRIMITIVE_TRIANGLE_FAN,
	JKG_PRIMITIVE_TRIANGLE_STRIP,
	JKG_PRIMITIVE_QUADS,
	JKG_PRIMITIVE_QUAD_STRIP,
};

enum jkg_index_type
{
	JKG_INDEX_UINT8,
	JKG_INDEX_UINT16,
	JKG_INDEX_UINT32,
};

enum jkg_shader_type
{
	JKG_SHADER_VERTEX,
	JKG_SHADER_FRAGMENT,
};

enum jkg_cull_face
{
	JKG_CULL_FRONT = (1 << 0),
	JKG_CULL_BACK  = (1 << 1),
};

enum jkg_fill_mode
{
	JKG_FILL_POINT,
	JKG_FILL_LINE,
	JKG_FILL_SOLID,
};

enum jkg_swizzle
{
	JKG_SWIZZLE_R,
	JKG_SWIZZLE_G,
	JKG_SWIZZLE_B,
	JKG_SWIZZLE_A,
	JKG_SWIZZLE_ZERO,
	JKG_SWIZZLE_ONE,
};

enum jkg_buffer_type
{
	JKG_BUFFER_VERTEX,
	JKG_BUFFER_INDEX,
	JKG_BUFFER_UNIFORM,
};

struct jkg_extent
{
	uint32_t x;
	uint32_t y;
	uint32_t z;
};

struct jkg_stencil_op_state
{
	enum jkg_stencil_op fail_op;
	enum jkg_stencil_op pass_op;
	enum jkg_stencil_op zfail_op;
	enum jkg_compare_op compare_op;
	uint32_t compare_mask;
	uint32_t write_mask;
};

struct jkg_depth_stencil_state_create_info
{
	bool depth_test_enable;
	bool depth_write_enable;
	enum jkg_compare_op depth_compare;
	bool stencil_enable;
	struct jkg_stencil_op_state front;
	struct jkg_stencil_op_state back;
};

struct jkg_rasterizer_state_create_info
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

struct jkg_blend_attachment
{
	bool enable;
	enum jkg_blend_fn rgb_src;
	enum jkg_blend_fn rgb_dst;
	enum jkg_blend_eq rgb_eq;
	enum jkg_blend_fn alpha_src;
	enum jkg_blend_fn alpha_dst;
	enum jkg_blend_eq alpha_eq;
	enum jkg_color_mask color_mask;
};

struct jkg_blend_state_create_info
{
	bool logic_enable;
	enum jkg_logic_op logic_op;
	const struct jkg_blend_attachment *attachments;
	uint32_t nattachments;
};

struct jkg_buffer_create_info
{
	uint32_t size;
	enum jkg_buffer_type type;
};

struct jkg_input_layout_attrib
{
	bool enable;
	uint32_t offset;
	uint32_t binding;
	uint32_t divisor;
	enum jkg_format format;
};

struct jkg_input_layout_create_info
{
	const struct jkg_input_layout_attrib *attribs;
	uint32_t nattribs;
};

struct jkg_shader_create_info
{
	enum jkg_shader_type type;
	const void *data;
	uint32_t size;
};

struct jkg_shader_state_create_info
{
	struct jkg_shader *vs;
	struct jkg_shader *fs;
};

struct jkg_image_create_info
{
	struct jkg_extent size;
	uint32_t levels;
	uint32_t layers;
	enum jkg_format format;
	enum jkg_image_type type;
};

struct jkg_sampler_create_info
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

struct jkg_image_view_create_info
{
	struct jkg_image *image;
	uint32_t min_level;
	uint32_t num_levels;
	uint32_t min_layer;
	uint32_t num_layers;
	enum jkg_format format;
	enum jkg_swizzle swizzle[4];
};

struct jkg_surface_create_info
{
	struct jkg_image *image;
	uint32_t level;
	uint32_t min_layer;
	uint32_t num_layers;
};

/* gjit defined */
struct jkg_vertex_buffer
{
	uint32_t stride;
	uint32_t offset;
	struct jkg_buffer *buffer;
};

struct jkg_uniform_block
{
	struct jkg_buffer *buffer;
	uint32_t offset;
	uint32_t length;
};

struct jkg_clear_cmd
{
	enum jkg_clear_mask mask;
	float color[4];
	float depth;
	uint32_t stencil;
};

struct jkg_draw_cmd
{
	enum jkg_primitive primitive;
	uint32_t start;
	uint32_t count;
	uint32_t instance_start;
	uint32_t instance_count;
	uint32_t base_vertex;
	uint32_t primitive_restart_index;
	bool indexed;
	bool primitive_restart_enable;
};

struct jkg_op
{
	void (*destroy)(struct jkg_ctx *ctx);
	int (*flush)(struct jkg_ctx *ctx);
	int (*resize)(struct jkg_ctx *ctx, uint32_t width, uint32_t height);
	int (*clear)(struct jkg_ctx *ctx, const struct jkg_clear_cmd *cmd);
	int (*draw)(struct jkg_ctx *ctx, const struct jkg_draw_cmd *cmd);
	int (*get_default_images)(struct jkg_ctx *ctx, struct jkg_image **color, enum jkg_format *color_format, struct jkg_image **depth, enum jkg_format *depth_format, struct jkg_image **stencil, enum jkg_format *stencil_format, uint32_t *width, uint32_t *height);
	int (*set_index_buffer)(struct jkg_ctx *ctx, struct jkg_buffer *buffer, enum jkg_index_type type, uint32_t offset);
	int (*set_vertex_buffers)(struct jkg_ctx *ctx, const struct jkg_vertex_buffer *vertex_buffers, uint32_t count);
	int (*set_uniform_blocks)(struct jkg_ctx *ctx, const struct jkg_uniform_block *uniform_blocks, uint32_t count);
	int (*set_blend_color)(struct jkg_ctx *ctx, const float *color);
	int (*set_viewport)(struct jkg_ctx *ctx, const float *viewport);
	int (*set_scissor)(struct jkg_ctx *ctx, const float *scissor);
	int (*set_stencil_ref)(struct jkg_ctx *ctx, uint8_t front, uint8_t back);
	void (*sample)(struct jkg_ctx *ctx, uint32_t id, const float coord[3], float color[4]);

	int (*buffer_alloc)(struct jkg_ctx *ctx, const struct jkg_buffer_create_info *create_info, struct jkg_buffer **bufferp);
	int (*buffer_write)(struct jkg_ctx *ctx, struct jkg_buffer *buffer, const void *data, uint32_t size, uint32_t offset);
	int (*buffer_read)(struct jkg_ctx *ctx, struct jkg_buffer *buffer, void *data, uint32_t size, uint32_t offset);
	int (*buffer_copy)(struct jkg_ctx *ctx, struct jkg_buffer *dst, struct jkg_buffer *src, uint32_t dst_off, uint32_t src_off, uint32_t size);
	void (*buffer_free)(struct jkg_ctx *ctx, struct jkg_buffer *buffer);

	int (*shader_alloc)(struct jkg_ctx *ctx, const struct jkg_shader_create_info *create_info, struct jkg_shader **shaderp);
	void (*shader_free)(struct jkg_ctx *ctx, struct jkg_shader *shader);

	int (*shader_state_alloc)(struct jkg_ctx *ctx, const struct jkg_shader_state_create_info *create_info, struct jkg_shader_state **statep);
	int (*shader_state_bind)(struct jkg_ctx *ctx, struct jkg_shader_state *state);
	void (*shader_state_free)(struct jkg_ctx *ctx, struct jkg_shader_state *state);

	int (*depth_stencil_state_alloc)(struct jkg_ctx *ctx, const struct jkg_depth_stencil_state_create_info *create_info, struct jkg_depth_stencil_state **statep);
	int (*depth_stencil_state_bind)(struct jkg_ctx *ctx, struct jkg_depth_stencil_state *state);
	void (*depth_stencil_state_free)(struct jkg_ctx *ctx, struct jkg_depth_stencil_state *state);

	int (*rasterizer_state_alloc)(struct jkg_ctx *ctx, const struct jkg_rasterizer_state_create_info *create_info, struct jkg_rasterizer_state **statep);
	int (*rasterizer_state_bind)(struct jkg_ctx *ctx, struct jkg_rasterizer_state *state);
	void (*rasterizer_state_free)(struct jkg_ctx *ctx, struct jkg_rasterizer_state *state);

	int (*blend_state_alloc)(struct jkg_ctx *ctx, const struct jkg_blend_state_create_info *create_info, struct jkg_blend_state **statep);
	int (*blend_state_bind)(struct jkg_ctx *ctx, struct jkg_blend_state *state);
	void (*blend_state_free)(struct jkg_ctx *ctx, struct jkg_blend_state *state);

	int (*input_layout_alloc)(struct jkg_ctx *ctx, const struct jkg_input_layout_create_info *create_info, struct jkg_input_layout **input_layoutp);
	int (*input_layout_bind)(struct jkg_ctx *ctx, struct jkg_input_layout *input_layout);
	void (*input_layout_free)(struct jkg_ctx *ctx, struct jkg_input_layout *input_layout);

	int (*sampler_alloc)(struct jkg_ctx *ctx, const struct jkg_sampler_create_info *create_info, struct jkg_sampler **samplerp);
	int (*sampler_bind)(struct jkg_ctx *ctx, struct jkg_sampler **samplers, uint32_t first, uint32_t count);
	void (*sampler_free)(struct jkg_ctx *ctx, struct jkg_sampler *sampler);

	int (*image_view_alloc)(struct jkg_ctx *ctx, const struct jkg_image_view_create_info *create_info, struct jkg_image_view **image_viewp);
	int (*image_view_bind)(struct jkg_ctx *ctx, struct jkg_image_view **image_view, uint32_t first, uint32_t count);
	void (*image_view_free)(struct jkg_ctx *ctx, struct jkg_image_view *image_view);

	int (*image_alloc)(struct jkg_ctx *ctx, const struct jkg_image_create_info *create_info, struct jkg_image **imagep);
	int (*image_read)(struct jkg_ctx *ctx, struct jkg_image *image, uint32_t level, void *data, enum jkg_format format, const struct jkg_extent *size, const struct jkg_extent *offset);
	int (*image_write)(struct jkg_ctx *ctx, struct jkg_image *image, uint32_t level, const void *data, enum jkg_format format, const struct jkg_extent *size, const struct jkg_extent *offset);
	int (*image_copy)(struct jkg_ctx *ctx, struct jkg_image *dst, struct jkg_image *src, uint32_t dst_level, uint32_t src_level, const struct jkg_extent *dst_off, const struct jkg_extent *src_off, const struct jkg_extent *size);
	void (*image_free)(struct jkg_ctx *ctx, struct jkg_image *image);

	int (*surface_alloc)(struct jkg_ctx *ctx, const struct jkg_surface_create_info *create_info, struct jkg_surface **surfacep);
	int (*surface_bind)(struct jkg_ctx *ctx, struct jkg_surface **colors, uint32_t count, struct jkg_surface *depth, struct jkg_surface *stencil);
	int (*surface_clear)(struct jkg_ctx *ctx, struct jkg_surface *surface, const struct jkg_clear_cmd *cmd);
	void (*surface_free)(struct jkg_ctx *ctx, struct jkg_surface *surface);
};

#define JKG_CAPS1_SURFACE_CLEAR (1 << 0)
#define JKG_CAPS1_DRAW_QUADS    (1 << 1)

struct jkg_caps
{
	const char *vendor;
	const char *renderer;
	const char *version;
	uint32_t max_vertex_attrib_relative_offset;
	uint32_t max_vertex_attrib_bindings;
	uint32_t max_vertex_attrib_stride;
	uint32_t max_vertex_attribs;
	uint32_t max_draw_buffers;
	uint32_t max_texture_image_units;
	uint32_t max_texture_2d_size;
	uint32_t max_texture_3d_size;
	uint32_t max_texture_layers;
	uint32_t max_anisotropy;
	uint32_t max_color_attachments;
	uint32_t max_renderbuffer_size;
	uint32_t max_framebuffer_width;
	uint32_t max_framebuffer_height;
	uint32_t max_uniform_blocks;
	float max_point_size;
	float max_line_width;
	uint32_t caps1;
};

struct jkg_id_range
{
	uint32_t first;
	uint32_t last;
	TAILQ_ENTRY(jkg_id_range) chain;
};

struct jkg_id_list
{
	uint32_t first;
	uint32_t last;
	TAILQ_HEAD(, jkg_id_range) ranges;
};

enum jkg_format_type jkg_get_format_type(enum jkg_format format);
uint32_t jkg_get_stride(enum jkg_format format);
uint32_t jkg_get_red_size(enum jkg_format format);
uint32_t jkg_get_green_size(enum jkg_format format);
uint32_t jkg_get_blue_size(enum jkg_format format);
uint32_t jkg_get_alpha_size(enum jkg_format format);
uint32_t jkg_get_depth_size(enum jkg_format format);
uint32_t jkg_get_stencil_size(enum jkg_format format);

void jkg_image_copy(void *dst, const void *src, enum jkg_format dst_format, enum jkg_format src_format, const struct jkg_extent *dst_size, const struct jkg_extent *src_size, const struct jkg_extent *dst_offset, const struct jkg_extent *src_offset, const struct jkg_extent *size);

void jkg_clear_vec4f(void *datav, uint32_t width, uint32_t height, enum jkg_format format, enum jkg_color_mask color_mask, int32_t rect[4], const float color[4]);
void jkg_get_vec4f(float * restrict color, const void * restrict datav, enum jkg_format format);
void jkg_set_vec4f(void * restrict datav, const float * restrict color, enum jkg_format format, enum jkg_color_mask mask);
void jkg_clear_vec4u(void *datav, uint32_t width, uint32_t height, enum jkg_format format, enum jkg_color_mask color_mask, int32_t rect[4], const uint32_t color[4]);
void jkg_get_vec4u(uint32_t * restrict color, const void * restrict datav, enum jkg_format format);
void jkg_set_vec4u(void * restrict datav, const uint32_t * restrict color, enum jkg_format format, enum jkg_color_mask mask);
void jkg_clear_depth(void *datav, uint32_t width, uint32_t height, enum jkg_format format, int32_t rect[4], float depth);
float jkg_get_depth(const void *data, enum jkg_format format);
void jkg_set_depth(void *datav, float value, enum jkg_format format);
void jkg_clear_stencil(void *datav, uint32_t width, uint32_t height, enum jkg_format format, int32_t rect[4], uint8_t stencil);
uint8_t jkg_get_stencil(const void *data, enum jkg_format format);
void jkg_set_stencil(void *datav, uint8_t value, enum jkg_format format);
void jkg_clear_depth_stencil(void *datav, uint32_t width, uint32_t height, enum jkg_format format, int32_t rect[4], float depth, uint8_t stencil);

void jkg_get_bc1_rgb(float *color, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void *data);
void jkg_get_bc1_rgba(float *color, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void *data);
void jkg_get_bc2(float *color, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void *data);
void jkg_get_bc3(float *color, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void *data);
void jkg_get_bc4_unorm(float *color, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void *data);
void jkg_get_bc4_snorm(float *color, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void *data);
void jkg_get_bc5_unorm(float *color, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void *data);
void jkg_get_bc5_snorm(float *color, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void *data);

bool jkg_id_init(struct jkg_id_list *list, uint32_t first, uint32_t last);
void jkg_id_destroy(struct jkg_id_list *list);
bool jkg_id_alloc(struct jkg_id_list *list, uint32_t *id);
bool jkg_id_free(struct jkg_id_list *list, uint32_t id);

#endif
