#ifndef GFX_GL_DEVICE_H
#define GFX_GL_DEVICE_H

#include "device.h"

#include <jks/array.h>
#include <pthread.h>
#include <limits.h>

typedef void *(gfx_gl_load_addr_t)(const char *name);

struct gfx_gl_device
{
	struct gfx_device device;
	gfx_gl_load_addr_t *load_addr;
	uint32_t textures[16];
	struct jks_array delete_render_buffers; /* uint32_t */
	struct jks_array delete_frame_buffers; /* uint32_t */
	struct jks_array delete_vertex_arrays; /* uint32_t */
	struct jks_array delete_programs; /* uint32_t */
	struct jks_array delete_buffers; /* uint32_t */
	struct jks_array delete_shaders; /* uint32_t */
	struct jks_array delete_textures; /* uint32_t */
	pthread_mutex_t delete_mutex;
	/* blend */
	enum gfx_blend_equation blend_equation_c;
	enum gfx_blend_equation blend_equation_a;
	enum gfx_blend_function blend_src_c;
	enum gfx_blend_function blend_src_a;
	enum gfx_blend_function blend_dst_c;
	enum gfx_blend_function blend_dst_a;
	enum gfx_color_mask color_mask;
	/* stencil */
	enum gfx_stencil_operation stencil_fail;
	enum gfx_stencil_operation stencil_pass;
	enum gfx_stencil_operation stencil_zfail;
	enum gfx_compare_function stencil_compare;
	uint32_t stencil_compare_mask;
	uint32_t stencil_write_mask;
	uint32_t stencil_reference;
	/* depth */
	enum gfx_compare_function depth_func;
	bool depth_mask;
	/* rasterizer */
	enum gfx_fill_mode fill_mode;
	enum gfx_cull_mode cull_mode;
	enum gfx_front_face front_face;
	bool scissor;
	/* attributes */
	const gfx_attributes_state_t *attributes_state;
	uint32_t active_texture;
	uint32_t vertex_array;
	uint32_t program;
	float line_width;
	float point_size;
	uint64_t state_idx;
	uint64_t blend_state;
	uint64_t stencil_state;
	uint64_t depth_state;
	uint64_t rasterizer_state;
	uint64_t pipeline_state;
	PFNGLDELETEBUFFERSPROC DeleteBuffers;
	PFNGLDELETERENDERBUFFERSPROC DeleteRenderbuffers;
	PFNGLDELETEFRAMEBUFFERSPROC DeleteFramebuffers;
	PFNGLDELETEVERTEXARRAYSPROC DeleteVertexArrays;
	PFNGLDELETEPROGRAMPROC DeleteProgram;
	PFNGLDELETESHADERPROC DeleteShader;
	PFNGLDELETETEXTURESPROC DeleteTextures;
	PFNGLGETINTERNALFORMATIVPROC GetInternalformativ;
	PFNGLGETINTEGERVPROC GetIntegerv;
	PFNGLENABLEPROC Enable;
	PFNGLDISABLEPROC Disable;
	PFNGLGETERRORPROC GetError;
	uint8_t states[(USHRT_MAX + 7) / 8];
};

extern const struct gfx_device_vtable gfx_gl_device_vtable;

#ifndef NDEBUG

# define GL_CALL_DEBUG(fn) \
do \
{ \
	GLenum err; \
	while ((err = GL_DEVICE->GetError())) \
		gfx_gl_errors(err, #fn,  __FILE__, __LINE__); \
} while (0)

#else

# define GL_CALL_DEBUG(fn) \
do \
{ \
} while (0)

#endif

#define GL_CALL(device, fn, ...) do { device->fn(__VA_ARGS__); GL_CALL_DEBUG(fn); } while (0)
#define GL_CALL_RET(ret, device, fn, ...) do { ret = device->fn(__VA_ARGS__); GL_CALL_DEBUG(fn); } while (0)

#define GL_LOAD_PROC(device, fn) \
do \
{ \
	gl_load_proc((struct gfx_device*)device, "gl" #fn, (void**)&device->fn); \
	if (device->fn == NULL) \
		GFX_ERROR_CALLBACK("failed to load gl" #fn); \
} while (0)

static inline void gl_load_proc(struct gfx_device *device, const char *name, void **ptr)
{
	*ptr = ((struct gfx_gl_device*)device)->load_addr(name);
}

extern const GLuint gfx_gl_primitives[];
extern const GLenum gfx_gl_index_types[];
extern const GLenum gfx_gl_compare_functions[];
extern const GLenum gfx_gl_blend_functions[];
extern const GLenum gfx_gl_blend_equations[];
extern const GLenum gfx_gl_stencil_operations[];
extern const GLenum gfx_gl_fill_modes[];
extern const GLenum gfx_gl_front_faces[];
extern const GLenum gfx_gl_cull_modes[];
extern const GLenum gfx_gl_shader_types[];
extern const GLenum gfx_gl_buffer_types[];
extern const GLenum gfx_gl_texture_types[];
extern const GLenum gfx_gl_texture_addressings[];
extern const GLenum gfx_gl_mag_filterings[];
extern const GLenum gfx_gl_min_filterings[];
extern const GLenum gfx_gl_buffer_usages[];
extern const GLenum gfx_gl_render_target_attachments[];
extern const GLenum gfx_gl_internal_formats[];
extern const GLenum gfx_gl_formats[];
extern const GLenum gfx_gl_format_types[];
extern const GLint gfx_gl_index_sizes[];
extern const bool gfx_gl_attribute_normalized[];
extern const GLenum gfx_gl_attribute_types[];
extern const GLint gfx_gl_attribute_nb[];
extern const bool gfx_gl_attribute_float[];
extern const GLuint gfx_gl_step_modes[];

void gfx_gl_errors(uint32_t err, const char *fn, const char *file, int line);
void gfx_gl_enable(struct gfx_device *device, uint32_t value);
void gfx_gl_disable(struct gfx_device *device, uint32_t value);

#endif
