#ifndef GFX_OBJECTS_H
#define GFX_OBJECTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#define GFX_MAX_ATTRIBUTES_COUNT    8
#define GFX_MAX_COLOR_TARGETS_COUNT 8
#define GFX_MAX_CONSTANTS_COUNT     8
#define GFX_MAX_SAMPLERS_COUNT      16

struct gfx_device;

enum gfx_buffer_type
{
	GFX_BUFFER_VERTEXES,
	GFX_BUFFER_INDICES,
	GFX_BUFFER_UNIFORM,
	GFX_BUFFER_INDIRECT,
};

enum gfx_buffer_usage
{
	GFX_BUFFER_IMMUTABLE, /* will never change */
	GFX_BUFFER_STATIC,    /* will change sometimes */
	GFX_BUFFER_DYNAMIC,   /* will change frequently */
	GFX_BUFFER_STREAM,    /* will change every frame */
};

enum gfx_texture_type
{
	GFX_TEXTURE_2D,
	GFX_TEXTURE_2D_MS,
	GFX_TEXTURE_2D_ARRAY,
	GFX_TEXTURE_2D_ARRAY_MS,
	GFX_TEXTURE_3D,
};

enum gfx_attribute_type
{
	GFX_ATTR_R32G32B32A32_FLOAT,
	GFX_ATTR_R32G32B32A32_UINT,
	GFX_ATTR_R32G32B32A32_SINT,
	GFX_ATTR_R32G32B32_FLOAT,
	GFX_ATTR_R32G32B32_UINT,
	GFX_ATTR_R32G32B32_SINT,
	GFX_ATTR_R32G32_FLOAT,
	GFX_ATTR_R32G32_UINT,
	GFX_ATTR_R32G32_SINT,
	GFX_ATTR_R32_FLOAT,
	GFX_ATTR_R32_UINT,
	GFX_ATTR_R32_SINT,
	GFX_ATTR_R16G16B16A16_FLOAT,
	GFX_ATTR_R16G16B16A16_UNORM,
	GFX_ATTR_R16G16B16A16_SNORM,
	GFX_ATTR_R16G16B16A16_UINT,
	GFX_ATTR_R16G16B16A16_SINT,
	GFX_ATTR_R16G16_FLOAT,
	GFX_ATTR_R16G16_UNORM,
	GFX_ATTR_R16G16_SNORM,
	GFX_ATTR_R16G16_UINT,
	GFX_ATTR_R16G16_SINT,
	GFX_ATTR_R8G8B8A8_UNORM,
	GFX_ATTR_R8G8B8A8_SNORM,
	GFX_ATTR_R8G8B8A8_UINT,
	GFX_ATTR_R8G8B8A8_SINT,
	GFX_ATTR_R8G8_UNORM,
	GFX_ATTR_R8G8_SNORM,
	GFX_ATTR_R8G8_UINT,
	GFX_ATTR_R8G8_SINT,
	GFX_ATTR_R8_UNORM,
	GFX_ATTR_R8_SNORM,
	GFX_ATTR_R8_UINT,
	GFX_ATTR_R8_SINT,
	GFX_ATTR_DISABLED = 0xFFFFFFFF,
};

enum gfx_index_type
{
	GFX_INDEX_UINT16,
	GFX_INDEX_UINT32,
};

enum gfx_compare_function
{
	GFX_CMP_NEVER,
	GFX_CMP_LOWER,
	GFX_CMP_LEQUAL,
	GFX_CMP_EQUAL,
	GFX_CMP_GEQUAL,
	GFX_CMP_GREATER,
	GFX_CMP_NOTEQUAL,
	GFX_CMP_ALWAYS,
};

enum gfx_blend_function
{
	GFX_BLEND_ZERO,
	GFX_BLEND_ONE,
	GFX_BLEND_SRC_COLOR,
	GFX_BLEND_ONE_MINUS_SRC_COLOR,
	GFX_BLEND_DST_COLOR,
	GFX_BLEND_ONE_MINUS_DST_COLOR,
	GFX_BLEND_SRC_ALPHA,
	GFX_BLEND_ONE_MINUS_SRC_ALPHA,
	GFX_BLEND_DST_ALPHA,
	GFX_BLEND_ONE_MINUS_DST_ALPHA,
	GFX_BLEND_FACTOR,
	GFX_BLEND_ONE_MINUS_FACTOR,
};

enum gfx_blend_equation
{
	GFX_EQUATION_ADD,
	GFX_EQUATION_SUBTRACT,
	GFX_EQUATION_REV_SUBTRACT,
	GFX_EQUATION_MIN,
	GFX_EQUATION_MAX,
};

enum gfx_stencil_operation
{
	GFX_STENCIL_KEEP,
	GFX_STENCIL_ZERO,
	GFX_STENCIL_REPLACE,
	GFX_STENCIL_INC,
	GFX_STENCIL_INC_WRAP,
	GFX_STENCIL_DEC,
	GFX_STENCIL_DEC_WRAP,
	GFX_STENCIL_INV,
};

enum gfx_filtering
{
	GFX_FILTERING_NONE,
	GFX_FILTERING_NEAREST,
	GFX_FILTERING_LINEAR,
};

enum gfx_texture_addressing
{
	GFX_TEXTURE_ADDRESSING_CLAMP,
	GFX_TEXTURE_ADDRESSING_REPEAT,
	GFX_TEXTURE_ADDRESSING_MIRROR,
	GFX_TEXTURE_ADDRESSING_BORDER,
	GFX_TEXTURE_ADDRESSING_MIRRORONCE,
};

enum gfx_format
{
	GFX_DEPTH24_STENCIL8,
	GFX_RGBA32F,
	GFX_RGBA16F,
	GFX_RGB32F,
	GFX_R8G8B8A8,
	GFX_R5G5B5A1,
	GFX_R4G4B4A4,
	GFX_R5G6B5,
	GFX_R8G8,
	GFX_R8,
	GFX_BC1_RGB,
	GFX_BC1_RGBA,
	GFX_BC2_RGBA,
	GFX_BC3_RGBA,
};

enum gfx_shader_type
{
	GFX_SHADER_VERTEX,
	GFX_SHADER_FRAGMENT,
	GFX_SHADER_GEOMETRY,
};

enum gfx_buffer_bit
{
	GFX_BUFFER_COLOR_BIT   = 0x1,
	GFX_BUFFER_DEPTH_BIT   = 0x2,
	GFX_BUFFER_STENCIL_BIT = 0x4,
};

enum gfx_primitive_type
{
	GFX_PRIMITIVE_TRIANGLES,
	GFX_PRIMITIVE_TRIANGLE_STRIP,
	GFX_PRIMITIVE_POINTS,
	GFX_PRIMITIVE_LINES,
	GFX_PRIMITIVE_LINE_STRIP,
};

enum gfx_fill_mode
{
	GFX_FILL_POINT,
	GFX_FILL_LINE,
	GFX_FILL_SOLID,
};

enum gfx_render_target_attachment
{
	GFX_RENDERTARGET_ATTACHMENT_NONE,
	GFX_RENDERTARGET_ATTACHMENT_DEPTH_STENCIL,
	GFX_RENDERTARGET_ATTACHMENT_COLOR0,
	GFX_RENDERTARGET_ATTACHMENT_COLOR1,
	GFX_RENDERTARGET_ATTACHMENT_COLOR2,
	GFX_RENDERTARGET_ATTACHMENT_COLOR3,
};

enum gfx_cull_mode
{
	GFX_CULL_NONE,
	GFX_CULL_FRONT,
	GFX_CULL_BACK,
};

enum gfx_front_face
{
	GFX_FRONT_CW,
	GFX_FRONT_CCW,
};

enum gfx_color_mask
{
	GFX_COLOR_MASK_NONE = 0x0,
	GFX_COLOR_MASK_R    = 0x1,
	GFX_COLOR_MASK_G    = 0x2,
	GFX_COLOR_MASK_B    = 0x4,
	GFX_COLOR_MASK_A    = 0x8,
	GFX_COLOR_MASK_ALL  = 0xF,
};

enum gfx_step_mode
{
	GFX_STEP_VERTEX,
	GFX_STEP_INSTANCE,
};

typedef union gfx_native_handle
{
	struct
	{
		uint16_t u16[4];
	};
	struct
	{
		uint32_t u32[2];
	};
	void *ptr;
	uint64_t u64;
} gfx_native_handle_t;

#define GFX_HANDLE_INIT {.u64 = 0}

#define GFX_BUFFER_INIT() (gfx_buffer_t){.handle = GFX_HANDLE_INIT}

typedef struct gfx_buffer
{
	struct gfx_device *device;
	gfx_native_handle_t handle;
	gfx_native_handle_t allocation;
	gfx_native_handle_t descriptor;
	enum gfx_buffer_usage usage;
	enum gfx_buffer_type type;
	uint32_t size;
	void *map;
} gfx_buffer_t;

#define GFX_TEXTURE_INIT() (gfx_texture_t){.handle = GFX_HANDLE_INIT}

typedef struct gfx_texture
{
	struct gfx_device *device;
	gfx_native_handle_t sampler;
	gfx_native_handle_t handle;
	gfx_native_handle_t allocation;
	gfx_native_handle_t view;
	gfx_native_handle_t descriptor;
	enum gfx_texture_addressing addressing_s;
	enum gfx_texture_addressing addressing_t;
	enum gfx_texture_addressing addressing_r;
	enum gfx_texture_type type;
	enum gfx_filtering min_filtering;
	enum gfx_filtering mag_filtering;
	enum gfx_filtering mip_filtering;
	enum gfx_format format;
	uint32_t anisotropy;
	uint32_t width;
	uint32_t height;
	uint32_t depth;
	uint32_t min_level;
	uint32_t max_level;
	uint8_t lod;
	uint8_t samples;
} gfx_texture_t;

#define GFX_BLEND_STATE_INIT() (gfx_blend_state_t){.handle = GFX_HANDLE_INIT}

typedef struct gfx_blend_state
{
	struct gfx_device *device;
	gfx_native_handle_t handle;
	enum gfx_blend_function src_c;
	enum gfx_blend_function src_a;
	enum gfx_blend_function dst_c;
	enum gfx_blend_function dst_a;
	enum gfx_blend_equation equation_c;
	enum gfx_blend_equation equation_a;
	bool enabled;
	enum gfx_color_mask color_mask;
} gfx_blend_state_t;

#define GFX_DEPTH_STENCIL_STATE_INIT() (gfx_depth_stencil_state_t){.handle = GFX_HANDLE_INIT}

typedef struct gfx_depth_stencil_state
{
	struct gfx_device *device;
	gfx_native_handle_t handle;
	enum gfx_compare_function depth_compare;
	bool depth_write;
	bool depth_test;
	enum gfx_stencil_operation stencil_fail;
	enum gfx_stencil_operation stencil_pass;
	enum gfx_stencil_operation stencil_zfail;
	enum gfx_compare_function stencil_compare;
	uint32_t stencil_compare_mask;
	uint32_t stencil_write_mask;
	uint32_t stencil_reference;
	bool stencil_enabled;
} gfx_depth_stencil_state_t;

#define GFX_RENDER_TARGET_INIT() (gfx_render_target_t){.handle = GFX_HANDLE_INIT}

typedef struct gfx_render_target
{
	struct gfx_device *device;
	gfx_native_handle_t handle;
	struct
	{
		gfx_native_handle_t handle;
		const gfx_texture_t *texture;
	} colors[GFX_MAX_COLOR_TARGETS_COUNT];
	struct
	{
		gfx_native_handle_t handle;
		const gfx_texture_t *texture;
	} depth_stencil;
	uint8_t draw_buffers[GFX_MAX_COLOR_TARGETS_COUNT];
	uint32_t draw_buffers_nb;
} gfx_render_target_t;

#define GFX_SHADER_INIT() (gfx_shader_t){.handle = GFX_HANDLE_INIT}

typedef struct gfx_shader
{
	struct gfx_device *device;
	gfx_native_handle_t handle;
	enum gfx_shader_type type;
	uint8_t *code;
	uint32_t code_size;
} gfx_shader_t;

#define GFX_SHADER_STATE_INIT() (gfx_shader_state_t){.handle = GFX_HANDLE_INIT}

typedef struct gfx_shader_state
{
	struct gfx_device *device;
	gfx_native_handle_t handle;
	gfx_native_handle_t vertex_shader;
	gfx_native_handle_t fragment_shader;
	gfx_native_handle_t geometry_shader;
	uint8_t *code;
	uint32_t code_size;
	gfx_native_handle_t descriptors[2];
	uint32_t constants_count;
	uint32_t samplers_count;
} gfx_shader_state_t;

#define GFX_ATTRIBUTES_STATE_INIT() (gfx_attributes_state_t){.handle = GFX_HANDLE_INIT}

struct gfx_attribute_bind
{
	const gfx_buffer_t *buffer;
};

typedef struct gfx_attributes_state
{
	struct gfx_device *device;
	gfx_native_handle_t handle;
	struct gfx_attribute_bind binds[GFX_MAX_ATTRIBUTES_COUNT];
	uint32_t count;
	enum gfx_index_type index_type;
	const gfx_buffer_t *index_buffer;
} gfx_attributes_state_t;

struct gfx_shader_attribute
{
	const char *name;
	uint32_t bind;
};

struct gfx_shader_constant
{
	const char *name;
	uint32_t bind;
};

struct gfx_shader_sampler
{
	const char *name;
	uint32_t bind;
};

typedef struct gfx_rasterizer_state
{
	struct gfx_device *device;
	gfx_native_handle_t handle;
	enum gfx_fill_mode fill_mode;
	enum gfx_cull_mode cull_mode;
	enum gfx_front_face front_face;
	bool scissor;
} gfx_rasterizer_state_t;

#define GFX_RASTERIZER_STATE_INIT() (gfx_rasterizer_state_t){.handle = GFX_HANDLE_INIT}

struct gfx_input_layout_bind
{
	uint32_t buffer;
	enum gfx_attribute_type type;
	uint32_t stride;
	uint32_t offset;
	enum gfx_step_mode step_mode;
};

typedef struct gfx_input_layout
{
	struct gfx_device *device;
	gfx_native_handle_t handle;
	struct gfx_input_layout_bind binds[GFX_MAX_ATTRIBUTES_COUNT];
	uint32_t count;
} gfx_input_layout_t;

#define GFX_INPUT_LAYOUT_INIT() (gfx_input_layout_t){.handle = GFX_HANDLE_INIT}

typedef struct gfx_pipeline_state
{
	struct gfx_device *device;
	gfx_native_handle_t handle;
	const gfx_shader_state_t *shader_state;
	const gfx_rasterizer_state_t *rasterizer_state;
	const gfx_depth_stencil_state_t *depth_stencil_state;
	const gfx_blend_state_t *blend_state;
	const gfx_input_layout_t *input_layout;
	enum gfx_primitive_type primitive;
} gfx_pipeline_state_t;

#define GFX_PIPELINE_STATE_INIT() (gfx_pipeline_state_t){.handle = GFX_HANDLE_INIT}

struct gfx_descriptor_constant
{
	const gfx_buffer_t *buffer;
	uint32_t offset;
	uint32_t size;
};

typedef struct gfx_constant_state
{
	struct gfx_device *device;
	gfx_native_handle_t handle;
	struct gfx_descriptor_constant constants[GFX_MAX_CONSTANTS_COUNT];
	const gfx_shader_state_t *shader_state;
} gfx_constant_state_t;

#define GFX_CONSTANT_STATE_INIT() (gfx_constant_state_t){.handle = GFX_HANDLE_INIT}

struct gfx_descriptor_sampler
{
	const gfx_texture_t *texture;
};

typedef struct gfx_sampler_state
{
	struct gfx_device *device;
	gfx_native_handle_t handle;
	struct gfx_descriptor_sampler samplers[GFX_MAX_SAMPLERS_COUNT];
	const gfx_shader_state_t *shader_state;
} gfx_sampler_state_t;

#define GFX_SAMPLER_STATE_INIT() (gfx_sampler_state_t){.handle = GFX_HANDLE_INIT}

struct gfx_draw_indexed_indirect_cmd
{
	uint32_t index_count;
	uint32_t instance_count;
	uint32_t base_index;
	int32_t base_vertex;
	uint32_t base_instance;
};

struct gfx_draw_indirect_cmd
{
	uint32_t vertex_count;
	uint32_t instance_count;
	uint32_t base_vertex;
	uint32_t base_instance;
};

#ifdef __cplusplus
}
#endif

#endif
