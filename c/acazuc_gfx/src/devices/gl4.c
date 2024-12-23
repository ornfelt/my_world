#include <GL/glcorearb.h>

#include "devices/gl.h"

#include "device_vtable.h"
#include "devices.h"
#include "window.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define GL_DEVICE ((struct gfx_gl_device*)device)
#define GL4_DEVICE ((struct gfx_gl4_device*)device)

#define GL4_LOAD_PROC(fn) GL_LOAD_PROC(GL4_DEVICE, fn)

#ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT
# define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#endif

#define GL4_CALL(fn, ...) GL_CALL(GL4_DEVICE, fn, __VA_ARGS__)
#define GL4_CALL_RET(ret, fn, ...) GL_CALL_RET(ret, GL4_DEVICE, fn, __VA_ARGS__)

struct gfx_gl4_device
{
	struct gfx_gl_device gl;
	PFNGLPOINTSIZEPROC PointSize;
	PFNGLLINEWIDTHPROC LineWidth;
	PFNGLSCISSORPROC Scissor;
	PFNGLVIEWPORTPROC Viewport;
	PFNGLNAMEDFRAMEBUFFERDRAWBUFFERSPROC NamedFramebufferDrawBuffers;
	PFNGLCHECKNAMEDFRAMEBUFFERSTATUSPROC CheckNamedFramebufferStatus;
	PFNGLNAMEDFRAMEBUFFERRENDERBUFFERPROC NamedFramebufferRenderbuffer;
	PFNGLNAMEDFRAMEBUFFERTEXTUREPROC NamedFramebufferTexture;
	PFNGLBLITNAMEDFRAMEBUFFERPROC BlitNamedFramebuffer;
	PFNGLNAMEDFRAMEBUFFERDRAWBUFFERPROC NamedFramebufferDrawBuffer;
	PFNGLNAMEDFRAMEBUFFERREADBUFFERPROC NamedFramebufferReadBuffer;
	PFNGLBINDFRAMEBUFFERPROC BindFramebuffer;
	PFNGLCREATEFRAMEBUFFERSPROC CreateFramebuffers;
	PFNGLBINDBUFFERRANGEPROC BindBufferRange;
	PFNGLUSEPROGRAMPROC UseProgram;
	PFNGLUNIFORM1IPROC Uniform1i;
	PFNGLGETUNIFORMLOCATIONPROC GetUniformLocation;
	PFNGLUNIFORMBLOCKBINDINGPROC UniformBlockBinding;
	PFNGLGETUNIFORMBLOCKINDEXPROC GetUniformBlockIndex;
	PFNGLDETACHSHADERPROC DetachShader;
	PFNGLGETPROGRAMINFOLOGPROC GetProgramInfoLog;
	PFNGLGETPROGRAMIVPROC GetProgramiv;
	PFNGLLINKPROGRAMPROC LinkProgram;
	PFNGLATTACHSHADERPROC AttachShader;
	PFNGLCREATEPROGRAMPROC CreateProgram;
	PFNGLGETSHADERINFOLOGPROC GetShaderInfoLog;
	PFNGLGETSHADERIVPROC GetShaderiv;
	PFNGLCOMPILESHADERPROC CompileShader;
	PFNGLSHADERSOURCEPROC ShaderSource;
	PFNGLCREATESHADERPROC CreateShader;
	PFNGLNAMEDRENDERBUFFERSTORAGEPROC NamedRenderbufferStorage;
	PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEPROC NamedRenderbufferStorageMultisample;
	PFNGLBINDRENDERBUFFERPROC BindRenderbuffer;
	PFNGLCREATERENDERBUFFERSPROC CreateRenderbuffers;
	PFNGLTEXTUREPARAMETERIPROC TextureParameteri;
	PFNGLTEXTUREPARAMETERFPROC TextureParameterf;
	PFNGLCOMPRESSEDTEXTURESUBIMAGE3DPROC CompressedTextureSubImage3D;
	PFNGLCOMPRESSEDTEXTURESUBIMAGE2DPROC CompressedTextureSubImage2D;
	PFNGLTEXTURESUBIMAGE3DPROC TextureSubImage3D;
	PFNGLTEXTURESUBIMAGE2DPROC TextureSubImage2D;
	PFNGLTEXTURESTORAGE3DPROC TextureStorage3D;
	PFNGLTEXTURESTORAGE2DPROC TextureStorage2D;
	PFNGLCREATETEXTURESPROC CreateTextures;
	PFNGLVERTEXARRAYELEMENTBUFFERPROC VertexArrayElementBuffer;
	PFNGLENABLEVERTEXARRAYATTRIBPROC EnableVertexArrayAttrib;
	PFNGLVERTEXARRAYATTRIBIFORMATPROC VertexArrayAttribIFormat;
	PFNGLVERTEXARRAYATTRIBFORMATPROC VertexArrayAttribFormat;
	PFNGLVERTEXARRAYVERTEXBUFFERPROC VertexArrayVertexBuffer;
	PFNGLVERTEXARRAYATTRIBBINDINGPROC VertexArrayAttribBinding;
	PFNGLBINDVERTEXARRAYPROC BindVertexArray;
	PFNGLCREATEVERTEXARRAYSPROC CreateVertexArrays;
	PFNGLNAMEDBUFFERSUBDATAPROC NamedBufferSubData;
	PFNGLNAMEDBUFFERSTORAGEPROC NamedBufferStorage;
	PFNGLCREATEBUFFERSPROC CreateBuffers;
	PFNGLFRONTFACEPROC FrontFace;
	PFNGLCULLFACEPROC CullFace;
	PFNGLPOLYGONMODEPROC PolygonMode;
	PFNGLSTENCILOPPROC StencilOp;
	PFNGLSTENCILMASKPROC StencilMask;
	PFNGLSTENCILFUNCPROC StencilFunc;
	PFNGLDEPTHFUNCPROC DepthFunc;
	PFNGLDEPTHMASKPROC DepthMask;
	PFNGLBLENDEQUATIONSEPARATEPROC BlendEquationSeparate;
	PFNGLBLENDFUNCSEPARATEPROC BlendFuncSeparate;
	PFNGLDRAWARRAYSPROC DrawArrays;
	PFNGLDRAWELEMENTSPROC DrawElements;
	PFNGLDRAWARRAYSINSTANCEDPROC DrawArraysInstanced;
	PFNGLDRAWELEMENTSINSTANCEDPROC DrawElementsInstanced;
	PFNGLCLEARNAMEDFRAMEBUFFERFIPROC ClearNamedFramebufferfi;
	PFNGLCLEARNAMEDFRAMEBUFFERFVPROC ClearNamedFramebufferfv;
	PFNGLMAPNAMEDBUFFERRANGEPROC MapNamedBufferRange;
	PFNGLBINDTEXTURESPROC BindTextures;
	PFNGLTEXTURESTORAGE2DMULTISAMPLEPROC TextureStorage2DMultisample;
	PFNGLTEXTURESTORAGE3DMULTISAMPLEPROC TextureStorage3DMultisample;
	PFNGLBINDBUFFERPROC BindBuffer;
	PFNGLBINDVERTEXBUFFERSPROC BindVertexBuffers;
	PFNGLCOLORMASKPROC ColorMask;
	PFNGLVERTEXARRAYBINDINGDIVISORPROC VertexArrayBindingDivisor;
	PFNGLMULTIDRAWARRAYSINDIRECTPROC MultiDrawArraysIndirect;
	PFNGLMULTIDRAWELEMENTSINDIRECTPROC MultiDrawElementsIndirect;
	PFNGLGETINTEGERVPROC GetIntegerv;
	PFNGLGETSTRINGIPROC GetStringi;
	PFNGLGETTEXTUREHANDLEARBPROC GetTextureHandleARB;
	PFNGLMAKETEXTUREHANDLERESIDENTARBPROC MakeTextureHandleResidentARB;
	PFNGLUNIFORMHANDLEUI64VARBPROC UniformHandleui64vARB;
	enum gfx_primitive_type primitive;
	uint32_t indirect_buffer;
	bool bindless_textures;
};

static bool gl4_ctr(struct gfx_device *device, struct gfx_window *window)
{
	if (!gfx_gl_device_vtable.ctr(device, window))
		return false;
	GL4_LOAD_PROC(PointSize);
	GL4_LOAD_PROC(LineWidth);
	GL4_LOAD_PROC(Scissor);
	GL4_LOAD_PROC(Viewport);
	GL4_LOAD_PROC(NamedFramebufferDrawBuffers);
	GL4_LOAD_PROC(CheckNamedFramebufferStatus);
	GL4_LOAD_PROC(NamedFramebufferRenderbuffer);
	GL4_LOAD_PROC(NamedFramebufferTexture);
	GL4_LOAD_PROC(BlitNamedFramebuffer);
	GL4_LOAD_PROC(NamedFramebufferDrawBuffer);
	GL4_LOAD_PROC(NamedFramebufferReadBuffer);
	GL4_LOAD_PROC(BindFramebuffer);
	GL4_LOAD_PROC(CreateFramebuffers);
	GL4_LOAD_PROC(BindBufferRange);
	GL4_LOAD_PROC(UseProgram);
	GL4_LOAD_PROC(Uniform1i);
	GL4_LOAD_PROC(GetUniformLocation);
	GL4_LOAD_PROC(UniformBlockBinding);
	GL4_LOAD_PROC(GetUniformBlockIndex);
	GL4_LOAD_PROC(DetachShader);
	GL4_LOAD_PROC(GetProgramInfoLog);
	GL4_LOAD_PROC(GetProgramiv);
	GL4_LOAD_PROC(AttachShader);
	GL4_LOAD_PROC(LinkProgram);
	GL4_LOAD_PROC(CreateProgram);
	GL4_LOAD_PROC(GetShaderInfoLog);
	GL4_LOAD_PROC(GetShaderiv);
	GL4_LOAD_PROC(CompileShader);
	GL4_LOAD_PROC(ShaderSource);
	GL4_LOAD_PROC(CreateShader);
	GL4_LOAD_PROC(NamedRenderbufferStorage);
	GL4_LOAD_PROC(NamedRenderbufferStorageMultisample);
	GL4_LOAD_PROC(BindRenderbuffer);
	GL4_LOAD_PROC(CreateRenderbuffers);
	GL4_LOAD_PROC(TextureParameteri);
	GL4_LOAD_PROC(TextureParameterf);
	GL4_LOAD_PROC(CompressedTextureSubImage3D);
	GL4_LOAD_PROC(CompressedTextureSubImage2D);
	GL4_LOAD_PROC(TextureSubImage3D);
	GL4_LOAD_PROC(TextureSubImage2D);
	GL4_LOAD_PROC(TextureStorage3D);
	GL4_LOAD_PROC(TextureStorage2D);
	GL4_LOAD_PROC(CreateTextures);
	GL4_LOAD_PROC(VertexArrayElementBuffer);
	GL4_LOAD_PROC(EnableVertexArrayAttrib);
	GL4_LOAD_PROC(EnableVertexArrayAttrib);
	GL4_LOAD_PROC(VertexArrayAttribIFormat);
	GL4_LOAD_PROC(VertexArrayAttribFormat);
	GL4_LOAD_PROC(VertexArrayVertexBuffer);
	GL4_LOAD_PROC(VertexArrayAttribBinding);
	GL4_LOAD_PROC(BindVertexArray);
	GL4_LOAD_PROC(CreateVertexArrays);
	GL4_LOAD_PROC(NamedBufferSubData);
	GL4_LOAD_PROC(NamedBufferStorage);
	GL4_LOAD_PROC(CreateBuffers);
	GL4_LOAD_PROC(FrontFace);
	GL4_LOAD_PROC(CullFace);
	GL4_LOAD_PROC(PolygonMode);
	GL4_LOAD_PROC(StencilOp);
	GL4_LOAD_PROC(StencilMask);
	GL4_LOAD_PROC(StencilFunc);
	GL4_LOAD_PROC(DepthFunc);
	GL4_LOAD_PROC(DepthMask);
	GL4_LOAD_PROC(BlendEquationSeparate);
	GL4_LOAD_PROC(BlendFuncSeparate);
	GL4_LOAD_PROC(DrawArrays);
	GL4_LOAD_PROC(DrawElements);
	GL4_LOAD_PROC(DrawArraysInstanced);
	GL4_LOAD_PROC(DrawElementsInstanced);
	GL4_LOAD_PROC(ClearNamedFramebufferfi);
	GL4_LOAD_PROC(ClearNamedFramebufferfv);
	GL4_LOAD_PROC(MapNamedBufferRange);
	GL4_LOAD_PROC(BindTextures);
	GL4_LOAD_PROC(TextureStorage2DMultisample);
	GL4_LOAD_PROC(TextureStorage3DMultisample);
	GL4_LOAD_PROC(BindBuffer);
	GL4_LOAD_PROC(BindVertexBuffers);
	GL4_LOAD_PROC(ColorMask);
	GL4_LOAD_PROC(VertexArrayBindingDivisor);
	GL4_LOAD_PROC(MultiDrawArraysIndirect);
	GL4_LOAD_PROC(MultiDrawElementsIndirect);
	GL4_LOAD_PROC(GetIntegerv);
	GL4_LOAD_PROC(GetStringi);
	GLint num_extensions;
	GL4_CALL(GetIntegerv, GL_NUM_EXTENSIONS, &num_extensions);
	GL4_DEVICE->indirect_buffer = 0;
	GL4_DEVICE->bindless_textures = false;
#if 0
	for (GLint i = 0; i < num_extensions; ++i)
	{
		const GLubyte *string;
		GL4_CALL_RET(string, GetStringi, GL_EXTENSIONS, i);
		if (!strcmp((char*)string, "GL_ARB_bindless_texture"))
		{
			GL4_DEVICE->bindless_textures = true;
			GL4_LOAD_PROC(GetTextureHandleARB);
			GL4_LOAD_PROC(MakeTextureHandleResidentARB);
			GL4_LOAD_PROC(UniformHandleui64vARB);
			break;
		}
	}
#endif
	return true;
}

static void gl4_dtr(struct gfx_device *device)
{
	gfx_gl_device_vtable.dtr(device);
}

static void gl4_tick(struct gfx_device *device)
{
	gfx_gl_device_vtable.tick(device);
}

static void gl4_clear_color(struct gfx_device *device, const gfx_render_target_t *render_target, enum gfx_render_target_attachment attachment, vec4f_t color)
{
	GL4_CALL(ClearNamedFramebufferfv, render_target ? render_target->handle.u32[0] : 0, GL_COLOR, render_target ? (attachment - GFX_RENDERTARGET_ATTACHMENT_COLOR0) : 0, &color.x);
}

static void gl4_clear_depth_stencil(struct gfx_device *device, const gfx_render_target_t *render_target, float depth, uint8_t stencil)
{
	GL4_CALL(ClearNamedFramebufferfi, render_target ? render_target->handle.u32[0] : 0, GL_DEPTH_STENCIL, 0, depth, stencil);
}

static void gl4_draw_indexed_indirect(struct gfx_device *device, const gfx_buffer_t *buffer, uint32_t count, uint32_t offset)
{
	if (GL4_DEVICE->indirect_buffer != buffer->handle.u32[0])
	{
		GL4_CALL(BindBuffer, GL_DRAW_INDIRECT_BUFFER, buffer->handle.u32[0]);
		GL4_DEVICE->indirect_buffer = buffer->handle.u32[0];
	}
	GL4_CALL(MultiDrawElementsIndirect, gfx_gl_primitives[GL4_DEVICE->primitive], gfx_gl_index_types[GL_DEVICE->attributes_state->index_type], (void*)(intptr_t)offset, count, 0);
	gfx_add_draw_stats(device, GL4_DEVICE->primitive, count, count);
}

static void gl4_draw_indirect(struct gfx_device *device, const gfx_buffer_t *buffer, uint32_t count, uint32_t offset)
{
	if (GL4_DEVICE->indirect_buffer != buffer->handle.u32[0])
	{
		GL4_CALL(BindBuffer, GL_DRAW_INDIRECT_BUFFER, buffer->handle.u32[0]);
		GL4_DEVICE->indirect_buffer = buffer->handle.u32[0];
	}
	GL4_CALL(MultiDrawArraysIndirect, gfx_gl_primitives[GL4_DEVICE->primitive], (void*)(intptr_t)offset, count, 0);
	gfx_add_draw_stats(device, GL4_DEVICE->primitive, count, count);
}

static void gl4_draw_indexed_instanced(struct gfx_device *device, uint32_t count, uint32_t offset, uint32_t prim_count)
{
	GL4_CALL(DrawElementsInstanced, gfx_gl_primitives[GL4_DEVICE->primitive], count, gfx_gl_index_types[GL_DEVICE->attributes_state->index_type], (void*)(intptr_t)(offset * gfx_gl_index_sizes[GL_DEVICE->attributes_state->index_type]), prim_count);
	gfx_add_draw_stats(device, GL4_DEVICE->primitive, count, prim_count);
}

static void gl4_draw_instanced(struct gfx_device *device, uint32_t count, uint32_t offset, uint32_t prim_count)
{
	GL4_CALL(DrawArraysInstanced, gfx_gl_primitives[GL4_DEVICE->primitive], offset, count, prim_count);
	gfx_add_draw_stats(device, GL4_DEVICE->primitive, count, prim_count);
}

static void gl4_draw_indexed(struct gfx_device *device, uint32_t count, uint32_t offset)
{
	GL4_CALL(DrawElements, gfx_gl_primitives[GL4_DEVICE->primitive], count, gfx_gl_index_types[GL_DEVICE->attributes_state->index_type], (void*)(intptr_t)(offset * gfx_gl_index_sizes[GL_DEVICE->attributes_state->index_type]));
	gfx_add_draw_stats(device, GL4_DEVICE->primitive, count, 1);
}

static void gl4_draw(struct gfx_device *device, uint32_t count, uint32_t offset)
{
	GL4_CALL(DrawArrays, gfx_gl_primitives[GL4_DEVICE->primitive], offset, count);
	gfx_add_draw_stats(device, GL4_DEVICE->primitive, count, 1);
}

static bool gl4_create_blend_state(struct gfx_device *device, gfx_blend_state_t *state, bool enabled, enum gfx_blend_function src_c, enum gfx_blend_function dst_c, enum gfx_blend_function src_a, enum gfx_blend_function dst_a, enum gfx_blend_equation equation_c, enum gfx_blend_equation equation_a, enum gfx_color_mask color_mask)
{
	assert(!state->handle.u64);
	state->device = device;
	state->handle.u64 = ++GL_DEVICE->state_idx;
	state->enabled = enabled;
	state->src_c = src_c;
	state->dst_c = dst_c;
	state->src_a = src_a;
	state->dst_a = dst_a;
	state->equation_c = equation_c;
	state->equation_a = equation_a;
	state->color_mask = color_mask;
	return true;
}

static void gl4_bind_blend_state(struct gfx_device *device, const gfx_blend_state_t *state)
{
	assert(state->handle.u64);
	if (state->handle.u64 == GL_DEVICE->blend_state)
		return;
	if (state->enabled)
	{
		gfx_gl_enable(device, GL_BLEND);
		if (GL_DEVICE->blend_src_c != state->src_c || GL_DEVICE->blend_src_a != state->src_a || GL_DEVICE->blend_dst_c != state->dst_c || GL_DEVICE->blend_dst_a != state->dst_a)
		{
			GL_DEVICE->blend_src_c = state->src_c;
			GL_DEVICE->blend_src_a = state->src_a;
			GL_DEVICE->blend_dst_c = state->dst_c;
			GL_DEVICE->blend_dst_a = state->dst_a;
			GL4_CALL(BlendFuncSeparate, gfx_gl_blend_functions[state->src_c], gfx_gl_blend_functions[state->dst_c], gfx_gl_blend_functions[state->src_a], gfx_gl_blend_functions[state->dst_a]);
		}
		if (GL_DEVICE->blend_equation_c != state->equation_c || GL_DEVICE->blend_equation_a != state->equation_a)
		{
			GL_DEVICE->blend_equation_c = state->equation_c;
			GL_DEVICE->blend_equation_a = state->equation_a;
			GL4_CALL(BlendEquationSeparate, gfx_gl_blend_equations[state->equation_c], gfx_gl_blend_equations[state->equation_a]);
		}
	}
	else
	{
		gfx_gl_disable(device, GL_BLEND);
	}
	if (GL_DEVICE->color_mask != state->color_mask)
	{
		GL_DEVICE->color_mask = state->color_mask;
		GL4_CALL(ColorMask, state->color_mask & GFX_COLOR_MASK_R, state->color_mask & GFX_COLOR_MASK_G, state->color_mask & GFX_COLOR_MASK_B, state->color_mask & GFX_COLOR_MASK_A);
	}
}

static void gl4_delete_blend_state(struct gfx_device *device, gfx_blend_state_t *state)
{
	(void)device;
	if (!state)
		return;
	state->handle.u64 = 0;
}

static bool gl4_create_depth_stencil_state(struct gfx_device *device, gfx_depth_stencil_state_t *state, bool depth_write, bool depth_test, enum gfx_compare_function depth_compare, bool stencil_enabled, uint32_t stencil_write_mask, enum gfx_compare_function stencil_compare, uint32_t stencil_reference, uint32_t stencil_compare_mask, enum gfx_stencil_operation stencil_fail, enum gfx_stencil_operation stencil_zfail, enum gfx_stencil_operation stencil_pass)
{
	assert(!state->handle.u64);
	state->device = device;
	state->handle.u64 = ++GL_DEVICE->state_idx;
	state->depth_write = depth_write;
	state->depth_test = depth_test;
	state->depth_compare = depth_compare;
	state->stencil_enabled = stencil_enabled;
	state->stencil_write_mask = stencil_write_mask;
	state->stencil_compare = stencil_compare;
	state->stencil_reference = stencil_reference;
	state->stencil_compare_mask = stencil_compare_mask;
	state->stencil_fail = stencil_fail;
	state->stencil_zfail = stencil_zfail;
	state->stencil_pass = stencil_pass;
	return true;
}

static void gl4_bind_depth_stencil_state(struct gfx_device *device, const gfx_depth_stencil_state_t *state)
{
	assert(state->handle.u64);
	if (state->handle.u64 == GL_DEVICE->stencil_state)
		return;
	if (state->depth_test)
	{
		gfx_gl_enable(device, GL_DEPTH_TEST);
		if (GL_DEVICE->depth_mask != state->depth_write)
		{
			GL_DEVICE->depth_mask = state->depth_write;
			GL4_CALL(DepthMask, state->depth_write ? GL_TRUE : GL_FALSE);
		}
		if (GL_DEVICE->depth_func != state->depth_compare)
		{
			GL_DEVICE->depth_func = state->depth_compare;
			GL4_CALL(DepthFunc, gfx_gl_compare_functions[state->depth_compare]);
		}
	}
	else
	{
		gfx_gl_disable(device, GL_DEPTH_TEST);
	}
	if (state->stencil_enabled)
	{
		gfx_gl_enable(device, GL_STENCIL_TEST);
		if (GL_DEVICE->stencil_compare != state->stencil_compare || GL_DEVICE->stencil_reference != state->stencil_reference || GL_DEVICE->stencil_compare_mask != state->stencil_compare_mask)
		{
			GL_DEVICE->stencil_compare = state->stencil_compare;
			GL_DEVICE->stencil_reference = state->stencil_reference;
			GL_DEVICE->stencil_compare_mask = state->stencil_compare_mask;
			GL4_CALL(StencilFunc, gfx_gl_compare_functions[state->stencil_compare], state->stencil_reference, state->stencil_compare_mask);
		}
		if (GL_DEVICE->stencil_write_mask != state->stencil_write_mask)
		{
			GL_DEVICE->stencil_write_mask = state->stencil_write_mask;
			GL4_CALL(StencilMask, state->stencil_write_mask);
		}
		if (GL_DEVICE->stencil_fail != state->stencil_fail || GL_DEVICE->stencil_zfail != state->stencil_zfail || GL_DEVICE->stencil_pass != state->stencil_pass)
		{
			GL_DEVICE->stencil_fail = state->stencil_fail;
			GL_DEVICE->stencil_zfail = state->stencil_zfail;
			GL_DEVICE->stencil_pass = state->stencil_pass;
			GL4_CALL(StencilOp, gfx_gl_stencil_operations[state->stencil_fail], gfx_gl_stencil_operations[state->stencil_zfail], gfx_gl_stencil_operations[state->stencil_pass]);
		}
	}
	else
	{
		gfx_gl_disable(device, GL_STENCIL_TEST);
	}
}

static void gl4_delete_depth_stencil_state(struct gfx_device *device, gfx_depth_stencil_state_t *state)
{
	(void)device;
	if (!state)
		return;
	state->handle.u64 = 0;
}

static bool gl4_create_rasterizer_state(struct gfx_device *device, gfx_rasterizer_state_t *state, enum gfx_fill_mode fill_mode, enum gfx_cull_mode cull_mode, enum gfx_front_face front_face, bool scissor)
{
	assert(!state->handle.u64);
	state->device = device;
	state->handle.u64 = ++GL_DEVICE->state_idx;
	state->fill_mode = fill_mode;
	state->cull_mode = cull_mode;
	state->front_face = front_face;
	state->scissor = scissor;
	return true;
}

static void gl4_bind_rasterizer_state(struct gfx_device *device, const gfx_rasterizer_state_t *state)
{
	assert(state->handle.u64);
	if (state->handle.u64 == GL_DEVICE->rasterizer_state)
		return;
	if (GL_DEVICE->fill_mode != state->fill_mode)
	{
		GL_DEVICE->fill_mode = state->fill_mode;
		GL4_CALL(PolygonMode, GL_FRONT_AND_BACK, gfx_gl_fill_modes[state->fill_mode]);
	}
	if (GL_DEVICE->cull_mode != state->cull_mode)
	{
		GL_DEVICE->cull_mode = state->cull_mode;
		if (state->cull_mode == GFX_CULL_NONE)
		{
			gfx_gl_disable(device, GL_CULL_FACE);
		}
		else
		{
			gfx_gl_enable(device, GL_CULL_FACE);
			GL4_CALL(CullFace, gfx_gl_cull_modes[state->cull_mode]);
		}
	}
	if (GL_DEVICE->front_face != state->front_face)
	{
		GL_DEVICE->front_face = state->front_face;
		GL4_CALL(FrontFace, gfx_gl_front_faces[state->front_face]);
	}
	if (GL_DEVICE->scissor != state->scissor)
	{
		GL_DEVICE->scissor = state->scissor;
		if (state->scissor)
			gfx_gl_enable(device, GL_SCISSOR_TEST);
		else
			gfx_gl_disable(device, GL_SCISSOR_TEST);
	}
}

static void gl4_delete_rasterizer_state(struct gfx_device *device, gfx_rasterizer_state_t *state)
{
	(void)device;
	if (!state)
		return;
	state->handle.u64 = 0;
}

static bool gl4_create_buffer(struct gfx_device *device, gfx_buffer_t *buffer, enum gfx_buffer_type type, const void *data, uint32_t size, enum gfx_buffer_usage usage)
{
	assert(!buffer->handle.u64);
	buffer->device = device;
	buffer->usage = usage;
	buffer->type = type;
	buffer->size = size;
	GL4_CALL(CreateBuffers, 1, &buffer->handle.u32[0]);
	if (size != 0)
	{
		uint32_t flags;
		if (usage == GFX_BUFFER_IMMUTABLE)
			flags = 0;
		else if (usage == GFX_BUFFER_STREAM)
			flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
		else
			flags = GL_DYNAMIC_STORAGE_BIT;
		GL4_CALL(NamedBufferStorage, buffer->handle.u32[0], size, data, flags);
		if (usage == GFX_BUFFER_STREAM)
			GL4_CALL_RET(buffer->map, MapNamedBufferRange, buffer->handle.u32[0], 0, size, flags);
	}
	return true; //XXX
}

static bool gl4_set_buffer_data(struct gfx_device *device, gfx_buffer_t *buffer, const void *data, uint32_t size, uint32_t offset)
{
	(void)device;
	assert(buffer->handle.u64);
	if (buffer->usage == GFX_BUFFER_STREAM)
		memcpy(((uint8_t*)buffer->map) + offset, data, size);
	else
		GL4_CALL(NamedBufferSubData, buffer->handle.u32[0], offset, size, data);
	return true; //XXX
}

static void gl4_delete_buffer(struct gfx_device *device, gfx_buffer_t *buffer)
{
	if (!buffer || !buffer->handle.u64)
		return;
	pthread_mutex_lock(&GL_DEVICE->delete_mutex);
	if (!jks_array_push_back(&GL_DEVICE->delete_buffers, &buffer->handle.u32[0]))
		assert(!"failed to push buffer gc");
	pthread_mutex_unlock(&GL_DEVICE->delete_mutex);
	buffer->handle.u32[0] = 0;
}

static bool gl4_create_attributes_state(struct gfx_device *device, gfx_attributes_state_t *state, const struct gfx_attribute_bind *binds, uint32_t count, const gfx_buffer_t *index_buffer, enum gfx_index_type index_type)
{
	assert(!state->handle.u64);
	state->device = device;
	memcpy(state->binds, binds, sizeof(*binds) * count);
	state->count = count;
	state->index_buffer = index_buffer;
	state->index_type = index_type;
	state->handle.u32[1] = 1;
	return true;
}

static void gl4_bind_attributes_state(struct gfx_device *device, const gfx_attributes_state_t *state, const gfx_input_layout_t *input_layout)
{
	assert(state->handle.u64);
	assert(input_layout->handle.u64);
	if (state->handle.u32[1] == 1)
		GL4_CALL(CreateVertexArrays, 1, (GLuint*)&state->handle.u32[0]);
	if (GL_DEVICE->vertex_array == state->handle.u32[0])
		return;
	GL_DEVICE->vertex_array = state->handle.u32[0];
	GL4_CALL(BindVertexArray, state->handle.u32[0]);
	GL_DEVICE->attributes_state = state;
	if (state->handle.u32[1] != 1)
		return;
	((gfx_attributes_state_t*)state)->handle.u32[1] = 0;
	for (size_t i = 0; i < GFX_MAX_ATTRIBUTES_COUNT; ++i)
	{
		if (input_layout->binds[i].type == GFX_ATTR_DISABLED)
			continue;
		uint32_t buffer_id = input_layout->binds[i].buffer;
		const gfx_buffer_t *buffer = state->binds[buffer_id].buffer;
		enum gfx_attribute_type type = input_layout->binds[i].type;
		GL4_CALL(VertexArrayAttribBinding, state->handle.u32[0], i, i);
		GL4_CALL(VertexArrayVertexBuffer, state->handle.u32[0], i, buffer->handle.u32[0], input_layout->binds[i].offset, input_layout->binds[i].stride);
		if (gfx_gl_attribute_normalized[type])
			GL4_CALL(VertexArrayAttribFormat, state->handle.u32[0], i, gfx_gl_attribute_nb[type], gfx_gl_attribute_types[type], true, 0);
		else if (gfx_gl_attribute_float[type])
			GL4_CALL(VertexArrayAttribFormat, state->handle.u32[0], i, gfx_gl_attribute_nb[type], gfx_gl_attribute_types[type], false, 0);
		else
			GL4_CALL(VertexArrayAttribIFormat, state->handle.u32[0], i, gfx_gl_attribute_nb[type], gfx_gl_attribute_types[type], 0);
		GL4_CALL(VertexArrayBindingDivisor, state->handle.u32[0], i, gfx_gl_step_modes[input_layout->binds[i].step_mode]);
		GL4_CALL(EnableVertexArrayAttrib, state->handle.u32[0], i);
	}
	if (state->index_buffer)
		GL4_CALL(BindBuffer, GL_ELEMENT_ARRAY_BUFFER, state->index_buffer->handle.u32[0]);
}

static void gl4_delete_attributes_state(struct gfx_device *device, gfx_attributes_state_t *state)
{
	(void)device;
	if (!state || !state->handle.u64)
		return;
	if (state->handle.u32[1] == 1)
	{
		state->handle.u64 = 0;
		return;
	}
	pthread_mutex_lock(&GL_DEVICE->delete_mutex);
	if (!jks_array_push_back(&GL_DEVICE->delete_vertex_arrays, &state->handle.u32[0]))
		assert(!"failed to queue attribute_state gc");
	state->handle.u64 = 0;
	pthread_mutex_unlock(&GL_DEVICE->delete_mutex);
}

static bool gl4_create_input_layout(struct gfx_device *device, gfx_input_layout_t *input_layout, const struct gfx_input_layout_bind *binds, uint32_t count, const gfx_shader_state_t *shader_state)
{
	(void)shader_state;
	assert(!input_layout->handle.u64);
	input_layout->device = device;
	memcpy(input_layout->binds, binds, sizeof(*binds) * count);
	input_layout->count = count;
	input_layout->handle.u64 = ++GL_DEVICE->state_idx;
	return true;
}

static void gl4_delete_input_layout(struct gfx_device *device, gfx_input_layout_t *input_layout)
{
	(void)device;
	if (!input_layout || !input_layout->handle.u64)
		return;
	input_layout->handle.u64 = 0;
}

static bool gl4_create_texture(struct gfx_device *device, gfx_texture_t *texture, enum gfx_texture_type type, enum gfx_format format, uint8_t lod, uint32_t width, uint32_t height, uint32_t depth)
{
	assert(!texture->handle.u64);
	texture->device = device;
	texture->format = format;
	texture->type = type;
	texture->width = width;
	texture->height = height;
	texture->depth = depth;
	texture->lod = lod;
	texture->addressing_s = GFX_TEXTURE_ADDRESSING_REPEAT;
	texture->addressing_t = GFX_TEXTURE_ADDRESSING_REPEAT;
	texture->addressing_r = GFX_TEXTURE_ADDRESSING_REPEAT;
	texture->min_filtering = GFX_FILTERING_NEAREST;
	texture->mag_filtering = GFX_FILTERING_LINEAR;
	texture->mip_filtering = GFX_FILTERING_LINEAR;
	texture->anisotropy = 1;
	texture->min_level = 0;
	texture->max_level = 1000;
	GL4_CALL(CreateTextures, gfx_gl_texture_types[type], 1, &texture->handle.u32[0]);
	switch (type)
	{
		case GFX_TEXTURE_2D:
			GL4_CALL(TextureStorage2D, texture->handle.u32[0], lod, gfx_gl_internal_formats[format], width, height);
			break;
		case GFX_TEXTURE_2D_MS:
			GL4_CALL(TextureStorage2DMultisample, texture->handle.u32[0], lod, gfx_gl_internal_formats[format], width, height, true);
			break;
		case GFX_TEXTURE_2D_ARRAY:
			GL4_CALL(TextureStorage3D, texture->handle.u32[0], lod, gfx_gl_internal_formats[format], width, height, depth);
			break;
		case GFX_TEXTURE_2D_ARRAY_MS:
			GL4_CALL(TextureStorage3DMultisample, texture->handle.u32[0], lod, gfx_gl_internal_formats[format], width, height, depth, true);
			break;
		case GFX_TEXTURE_3D:
			GL4_CALL(TextureStorage3D, texture->handle.u32[0], lod, gfx_gl_internal_formats[format], width, height, depth);
			break;
	}
	return true; //XXX
}

static bool gl4_set_texture_data(struct gfx_device *device, gfx_texture_t *texture, uint8_t lod, uint32_t offset, uint32_t width, uint32_t height, uint32_t depth, uint32_t size, const void *data)
{
	(void)device;
	assert(texture->handle.u64);
	switch (texture->format)
	{
		case GFX_DEPTH24_STENCIL8:
		case GFX_RGBA32F:
		case GFX_RGBA16F:
		case GFX_RGB32F:
		case GFX_R8G8B8A8:
		case GFX_R5G5B5A1:
		case GFX_R4G4B4A4:
		case GFX_R5G6B5:
		case GFX_R8G8:
		case GFX_R8:
			switch (texture->type)
			{
				case GFX_TEXTURE_2D:
					GL4_CALL(TextureSubImage2D, texture->handle.u32[0], lod, 0, offset, width, height, gfx_gl_formats[texture->format], gfx_gl_format_types[texture->format], data);
					break;
				case GFX_TEXTURE_2D_MS:
				case GFX_TEXTURE_2D_ARRAY_MS:
					assert(!"invalid type");
					break;
				case GFX_TEXTURE_2D_ARRAY:
				case GFX_TEXTURE_3D:
					GL4_CALL(TextureSubImage3D, texture->handle.u32[0], lod, 0, 0, offset, width, height, depth, gfx_gl_formats[texture->format], gfx_gl_format_types[texture->format], data);
					break;
			}
			break;
		case GFX_BC1_RGB:
		case GFX_BC1_RGBA:
		case GFX_BC2_RGBA:
		case GFX_BC3_RGBA:
			switch (texture->type)
			{
				case GFX_TEXTURE_2D:
					GL4_CALL(CompressedTextureSubImage2D, texture->handle.u32[0], lod, 0, offset, width, height, gfx_gl_internal_formats[texture->format], size, data);
					break;
				case GFX_TEXTURE_2D_MS:
				case GFX_TEXTURE_2D_ARRAY_MS:
					assert(!"invalid type");
					break;
				case GFX_TEXTURE_2D_ARRAY:
				case GFX_TEXTURE_3D:
					GL4_CALL(CompressedTextureSubImage3D, texture->handle.u32[0], lod, 0, 0, offset, width, height, depth, gfx_gl_internal_formats[texture->format], size, data);
					break;
			}
			break;
	}
	return true; //XXX
}

static void gl4_set_texture_addressing(struct gfx_device *device, gfx_texture_t *texture, enum gfx_texture_addressing addressing_s, enum gfx_texture_addressing addressing_t, enum gfx_texture_addressing addressing_r)
{
	(void)device;
	assert(texture->handle.u64);
	if (texture->addressing_s != addressing_s)
	{
		texture->addressing_s = addressing_s;
		GL4_CALL(TextureParameteri, texture->handle.u32[0], GL_TEXTURE_WRAP_S, gfx_gl_texture_addressings[addressing_s]);
	}
	if (texture->addressing_t != addressing_t)
	{
		texture->addressing_t = addressing_t;
		GL4_CALL(TextureParameteri, texture->handle.u32[0], GL_TEXTURE_WRAP_T, gfx_gl_texture_addressings[addressing_t]);
	}
	if (texture->addressing_r != addressing_r)
	{
		texture->addressing_r = addressing_r;
		GL4_CALL(TextureParameteri, texture->handle.u32[0], GL_TEXTURE_WRAP_R, gfx_gl_texture_addressings[addressing_r]);
	}
}

static void gl4_set_texture_filtering(struct gfx_device *device, gfx_texture_t *texture, enum gfx_filtering min_filtering, enum gfx_filtering mag_filtering, enum gfx_filtering mip_filtering)
{
	(void)device;
	assert(texture->handle.u64);
	if (texture->min_filtering != min_filtering || texture->mip_filtering != mip_filtering)
	{
		texture->min_filtering = min_filtering;
		texture->mip_filtering = mip_filtering;
		GL4_CALL(TextureParameteri, texture->handle.u32[0], GL_TEXTURE_MIN_FILTER, gfx_gl_min_filterings[mip_filtering * 3 + min_filtering]);
	}
	if (texture->mag_filtering != mag_filtering)
	{
		texture->mag_filtering = mag_filtering;
		GL4_CALL(TextureParameteri, texture->handle.u32[0], GL_TEXTURE_MAG_FILTER, gfx_gl_mag_filterings[mag_filtering]);
	}
}

static void gl4_set_texture_anisotropy(struct gfx_device *device, gfx_texture_t *texture, uint32_t anisotropy)
{
	(void)device;
	assert(texture->handle.u64);
	if (texture->anisotropy != anisotropy)
	{
		texture->anisotropy = anisotropy;
		GL4_CALL(TextureParameterf, texture->handle.u32[0], GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
	}
}

static void gl4_set_texture_levels(struct gfx_device *device, gfx_texture_t *texture, uint32_t min_level, uint32_t max_level)
{
	(void)device;
	assert(texture->handle.u64);
	if (texture->min_level != min_level)
	{
		texture->min_level = min_level;
		GL4_CALL(TextureParameteri, texture->handle.u32[0], GL_TEXTURE_BASE_LEVEL, min_level);
	}
	if (texture->max_level != max_level)
	{
		texture->max_level = max_level;
		GL4_CALL(TextureParameteri, texture->handle.u32[0], GL_TEXTURE_MAX_LEVEL, max_level);
	}
}

static bool gl4_finalize_texture(struct gfx_device *device, gfx_texture_t *texture)
{
	assert(texture->handle.u64);
	if (GL4_DEVICE->bindless_textures)
	{
		GL4_CALL_RET(texture->sampler.u64, GetTextureHandleARB, texture->handle.u32[0]);
		GL4_CALL(MakeTextureHandleResidentARB, texture->sampler.u64);
	}
	return true; /* XXX */
}

static void gl4_delete_texture(struct gfx_device *device, gfx_texture_t *texture)
{
	if (!texture || !texture->handle.u64)
		return;
	pthread_mutex_lock(&GL_DEVICE->delete_mutex);
	if (!jks_array_push_back(&GL_DEVICE->delete_textures, &texture->handle.u32[0]))
		assert(!"failed to queue texture gc");
	texture->handle.u32[0] = 0;
	pthread_mutex_unlock(&GL_DEVICE->delete_mutex);
}

static bool gl4_create_shader(struct gfx_device *device, gfx_shader_t *shader, enum gfx_shader_type type, const uint8_t *data, uint32_t size)
{
	assert(!shader->handle.u64);
	shader->device = device;
	shader->type = type;
	GL4_CALL_RET(shader->handle.u32[0], CreateShader, gfx_gl_shader_types[type]);
	GL4_CALL(ShaderSource, shader->handle.u32[0], 1, (const GLchar* const*)&data, (GLint*)&size);
	GL4_CALL(CompileShader, shader->handle.u32[0]);
	GLint result = GL_FALSE;
	GL4_CALL(GetShaderiv, shader->handle.u32[0], GL_COMPILE_STATUS, &result);
	if (!result)
	{
#ifndef NDEBUG
		/*int info_log_length;
		GL4_CALL(GetShaderiv, shader->handle.u32[0], GL_INFO_LOG_LENGTH, &info_log_length);*/
		char error[4096] = "";
		GL4_CALL(GetShaderInfoLog, shader->handle.u32[0], sizeof(error), NULL, error);
		GFX_ERROR_CALLBACK("can't compile GLSL shader: %s", error);
#endif
		return false;
	}
	return true;
}

static void gl4_delete_shader(struct gfx_device *device, gfx_shader_t *shader)
{
	if (!shader || !shader->handle.u64)
		return;
	pthread_mutex_lock(&GL_DEVICE->delete_mutex);
	if (!jks_array_push_back(&GL_DEVICE->delete_shaders, &shader->handle.u32[0]))
		assert(!"failed to queue shader gc");
	shader->handle.u32[0] = 0;
	pthread_mutex_unlock(&GL_DEVICE->delete_mutex);
}

static bool gl4_create_shader_state(struct gfx_device *device, gfx_shader_state_t *shader_state, const gfx_shader_t **shaders, uint32_t shaders_count, const struct gfx_shader_attribute *attributes, const struct gfx_shader_constant *constants, const struct gfx_shader_sampler *samplers)
{
	(void)attributes;
	(void)constants;
	(void)samplers;
	assert(!shader_state->handle.u64);
	const gfx_shader_t *vertex_shader = NULL;
	const gfx_shader_t *fragment_shader = NULL;
	const gfx_shader_t *geometry_shader = NULL;
	for (uint32_t i = 0; i < shaders_count; ++i)
	{
		if (!shaders[i])
			continue;
		switch (shaders[i]->type)
		{
			case GFX_SHADER_VERTEX:
				if (vertex_shader)
				{
					GFX_ERROR_CALLBACK("multiple vertex shaders given");
					return false;
				}
				vertex_shader = shaders[i];
				break;
			case GFX_SHADER_FRAGMENT:
				if (fragment_shader)
				{
					GFX_ERROR_CALLBACK("multiple fragment shaders given");
					return false;
				}
				fragment_shader = shaders[i];
				break;
			case GFX_SHADER_GEOMETRY:
				if (geometry_shader)
				{
					GFX_ERROR_CALLBACK("multiple geometry shaders given");
					return false;
				}
				geometry_shader = shaders[i];
				break;
		}
	}
	if (!vertex_shader)
	{
		GFX_ERROR_CALLBACK("no vertex shader given");
		return false;
	}
	if (!fragment_shader)
	{
		GFX_ERROR_CALLBACK("no fragment shader given");
		return false;
	}

	assert(vertex_shader->handle.u32[0]);
	assert(fragment_shader->handle.u32[0]);
	if (geometry_shader)
		assert(geometry_shader->handle.u32[0]);

	shader_state->device = device;
	GL4_CALL_RET(shader_state->handle.u32[0], CreateProgram);
	GL4_CALL(AttachShader, shader_state->handle.u32[0], vertex_shader->handle.u32[0]);
	GL4_CALL(AttachShader, shader_state->handle.u32[0], fragment_shader->handle.u32[0]);
	if (geometry_shader)
		GL4_CALL(AttachShader, shader_state->handle.u32[0], geometry_shader->handle.u32[0]);
	/* OpenGL 2
	for (uint32_t i = 0; attributes[i].name; ++i)
		GL_CALL(glBindAttribLocation, shader_state->handle.u32[0], attributes[i].bind, attributes[i].name);
	*/
	GL4_CALL(LinkProgram, shader_state->handle.u32[0]);
	GLint result = GL_FALSE;
	GL4_CALL(GetProgramiv, shader_state->handle.u32[0], GL_LINK_STATUS, &result);
	if (!result)
	{
#ifndef NDEBUG
		//int info_log_length;
		//GL4_CALL(GetProgramiv, shader_state->handle.u32[0], GL_INFO_LOG_LENGTH, &info_log_length);
		char error[4096] = "";
		GL4_CALL(GetProgramInfoLog, shader_state->handle.u32[0], sizeof(error), NULL, error);
		GFX_ERROR_CALLBACK("can't compile GLSL program: %s", error);
#endif
		return false;
	}
	GL4_CALL(DetachShader, shader_state->handle.u32[0], vertex_shader->handle.u32[0]);
	GL4_CALL(DetachShader, shader_state->handle.u32[0], fragment_shader->handle.u32[0]);
	if (geometry_shader)
		GL4_CALL(DetachShader, shader_state->handle.u32[0], geometry_shader->handle.u32[0]);
	GL4_CALL(UseProgram, shader_state->handle.u32[0]);
	return true;
}

static void gl4_bind_shader_state(struct gfx_device *device, const gfx_shader_state_t *shader_state)
{
	assert(shader_state->handle.u64);
	if (GL_DEVICE->program == shader_state->handle.u32[0])
		return;
	GL_DEVICE->program = shader_state->handle.u32[0];
	GL4_CALL(UseProgram, shader_state->handle.u32[0]);
}

static void gl4_delete_shader_state(struct gfx_device *device, gfx_shader_state_t *shader_state)
{
	if (!shader_state || !shader_state->handle.u64)
		return;
	pthread_mutex_lock(&GL_DEVICE->delete_mutex);
	if (!jks_array_push_back(&GL_DEVICE->delete_programs, &shader_state->handle.u32[0]))
		assert(!"failed to queue program gc");
	shader_state->handle.u32[0] = 0;
	pthread_mutex_unlock(&GL_DEVICE->delete_mutex);
}

static void gl4_bind_constant(struct gfx_device *device, uint32_t bind, const gfx_buffer_t *buffer, uint32_t size, uint32_t offset)
{
	(void)device;
	assert(buffer->handle.u64);
	GL4_CALL(BindBufferRange, GL_UNIFORM_BUFFER, bind, buffer->handle.u32[0], offset, size);
}

static void gl4_bind_samplers(struct gfx_device *device, uint32_t start, uint32_t count, const gfx_texture_t **textures)
{
	if (GL4_DEVICE->bindless_textures)
	{
		uint64_t textures_ids[GFX_MAX_SAMPLERS_COUNT];
		for (uint32_t i = 0; i < count; ++i)
			textures_ids[i] = textures[i]->sampler.u64;
		GL4_CALL(UniformHandleui64vARB, 16 + start, count, textures_ids);
	}
	else
	{
		uint32_t textures_ids[GFX_MAX_SAMPLERS_COUNT];
		for (uint32_t i = 0; i < count; ++i)
			textures_ids[i] = textures[i] ? textures[i]->handle.u32[0] : 0;
		if (!memcmp(textures_ids, &GL_DEVICE->textures[start], count * sizeof(*textures_ids)))
			return;
		memcpy(&GL_DEVICE->textures[start], textures_ids, count * sizeof(*textures_ids));
		GL4_CALL(BindTextures, start, count, textures_ids);
	}
}

static bool gl4_create_constant_state(struct gfx_device *device, gfx_constant_state_t *state, const gfx_shader_state_t *shader_state)
{
	assert(!state->handle.u64);
	state->device = device;
	state->shader_state = shader_state;
	state->handle.u64 = 1;
	for (size_t i = 0; i < GFX_MAX_CONSTANTS_COUNT; ++i)
		state->constants[i].buffer = NULL;
	return true;
}

static void gl4_set_constant(struct gfx_device *device, gfx_constant_state_t *state, uint32_t bind, const gfx_buffer_t *buffer, uint32_t size, uint32_t offset)
{
	assert(bind < GFX_MAX_CONSTANTS_COUNT);
	state->constants[bind].buffer = buffer;
	state->constants[bind].size = size;
	state->constants[bind].offset = offset;
}

static void gl4_bind_constant_state(struct gfx_device *device, gfx_constant_state_t *state)
{
	assert(state->handle.u64);
	for (size_t i = 0; i < GFX_MAX_CONSTANTS_COUNT; ++i)
	{
		struct gfx_descriptor_constant *constant = &state->constants[i];
		if (!constant->buffer)
			continue;
		GL4_CALL(BindBufferRange, GL_UNIFORM_BUFFER, i, constant->buffer->handle.u32[0], constant->offset, constant->size);
		/* XXX only one glBindBuffersRange call */
	}
}

static void gl4_delete_constant_state(struct gfx_device *device, gfx_constant_state_t *state)
{
	if (!state || !state->handle.u64)
		return;
	state->handle.u64 = 0;
}

static bool gl4_create_sampler_state(struct gfx_device *device, gfx_sampler_state_t *state, const gfx_shader_state_t *shader_state)
{
	assert(!state->handle.u64);
	state->device = device;
	state->shader_state = shader_state;
	state->handle.u64 = 1;
	for (size_t i = 0; i < GFX_MAX_CONSTANTS_COUNT; ++i)
		state->samplers[i].texture = NULL;
	return true;
}

static void gl4_set_sampler(struct gfx_device *device, gfx_sampler_state_t *state, uint32_t bind, const gfx_texture_t *texture)
{
	assert(bind < GFX_MAX_SAMPLERS_COUNT);
	state->samplers[bind].texture = texture;
}

static void gl4_bind_sampler_state(struct gfx_device *device, gfx_sampler_state_t *state)
{
	assert(state->handle.u64);
	for (size_t i = 0; i < GFX_MAX_SAMPLERS_COUNT; ++i)
	{
		struct gfx_descriptor_sampler *sampler = &state->samplers[i];
		if (!sampler->texture)
			continue;
		GL4_CALL(BindTextures, i, 1, &sampler->texture->handle.u32[0]);
		/* XXX only one glBindTextures call */
	}
}

static void gl4_delete_sampler_state(struct gfx_device *device, gfx_sampler_state_t *state)
{
	if (!state || !state->handle.u64)
		return;
	state->handle.u64 = 0;
}

static bool gl4_create_render_target(struct gfx_device *device, gfx_render_target_t *render_target)
{
	assert(!render_target->handle.u64);
	render_target->device = device;
	for (size_t i = 0; i < GFX_MAX_COLOR_TARGETS_COUNT; ++i)
		render_target->colors[i].texture = NULL;
	render_target->depth_stencil.texture = NULL;
	GL4_CALL(CreateFramebuffers, 1, &render_target->handle.u32[0]);
	return true; /* XXX */
}

static void gl4_delete_render_target(struct gfx_device *device, gfx_render_target_t *render_target)
{
	if (!render_target || !render_target->handle.u64)
		return;
	pthread_mutex_lock(&GL_DEVICE->delete_mutex);
	if (!jks_array_push_back(&GL_DEVICE->delete_frame_buffers, &render_target->handle.u32[0]))
		assert(!"failed to queue render_target gc");
	render_target->handle.u32[0] = 0;
	pthread_mutex_unlock(&GL_DEVICE->delete_mutex);
}

static void gl4_bind_render_target(struct gfx_device *device, const gfx_render_target_t *render_target)
{
	(void)device;
	if (render_target)
	{
		assert(render_target->handle.u64);
		GL4_CALL(BindFramebuffer, GL_FRAMEBUFFER, render_target->handle.u32[0]);
	}
	else
	{
		GL4_CALL(BindFramebuffer, GL_FRAMEBUFFER, 0);
	}
}

static void gl4_set_render_target_texture(struct gfx_device *device, gfx_render_target_t *render_target, enum gfx_render_target_attachment attachment, const gfx_texture_t *texture)
{
	(void)device;
	assert(render_target->handle.u64);
	if (attachment == GFX_RENDERTARGET_ATTACHMENT_DEPTH_STENCIL)
	{
		render_target->depth_stencil.texture = texture;
		GL4_CALL(NamedFramebufferTexture, render_target->handle.u32[0], GL_DEPTH_ATTACHMENT, texture->handle.u32[0], 0);
		GL4_CALL(NamedFramebufferTexture, render_target->handle.u32[0], GL_STENCIL_ATTACHMENT, texture->handle.u32[0], 0);
	}
	else
	{
		render_target->colors[attachment - GFX_RENDERTARGET_ATTACHMENT_COLOR0].texture = texture;
		GL4_CALL(NamedFramebufferTexture, render_target->handle.u32[0], gfx_gl_render_target_attachments[attachment], texture->handle.u32[0], 0);
	}
#ifndef NDEBUG
	{
		GLuint status = GL4_DEVICE->CheckNamedFramebufferStatus(render_target->handle.u32[0], GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
			GFX_ERROR_CALLBACK("invalid FBO init: %d", status);
	}
#endif
}

static void gl4_set_render_target_draw_buffers(struct gfx_device *device, gfx_render_target_t *render_target, uint32_t *render_buffers, uint32_t render_buffers_count)
{
	(void)device;
	uint32_t translated[GFX_MAX_COLOR_TARGETS_COUNT];
	for (uint32_t i = 0; i < render_buffers_count; ++i)
		translated[i] = gfx_gl_render_target_attachments[render_buffers[i]];
	GL4_CALL(NamedFramebufferDrawBuffers, render_target->handle.u32[0], render_buffers_count, translated);
}

static void gl4_resolve_render_target(struct gfx_device *device, const gfx_render_target_t *src, const gfx_render_target_t *dst, uint32_t buffers, uint32_t color_src, uint32_t color_dst)
{
	(void)device;
	if (!(buffers & (GFX_BUFFER_COLOR_BIT | GFX_BUFFER_DEPTH_BIT | GFX_BUFFER_STENCIL_BIT)))
		return;
	if (src)
		assert(src->handle.u64);
	if (dst)
		assert(dst->handle.u64);
	uint32_t width = 0;
	uint32_t height = 0;
	if (buffers & GFX_BUFFER_COLOR_BIT)
	{
		if (src)
		{
			GL4_CALL(NamedFramebufferReadBuffer, src->handle.u32[0], GL_COLOR_ATTACHMENT0 + color_src);
			if (src->colors[color_src].texture)
			{
				width = src->colors[color_src].texture->width;
				height = src->colors[color_src].texture->height;
			}
		}
		else
		{
			GL4_CALL(NamedFramebufferReadBuffer, 0, GL_BACK);
		}
		if (dst)
		{
			GL4_CALL(NamedFramebufferDrawBuffer, dst->handle.u32[0], GL_COLOR_ATTACHMENT0 + color_dst);
			if (dst->colors[color_dst].texture)
			{
				width = dst->colors[color_dst].texture->width;
				height = dst->colors[color_dst].texture->height;
			}
		}
		else
		{
			GL4_CALL(NamedFramebufferDrawBuffer, 0, GL_BACK);
		}
	}
	else
	{
		if (src && src->depth_stencil.texture)
		{
			width = src->depth_stencil.texture->width;
			height = src->depth_stencil.texture->height;
		}
		else if (dst && dst->depth_stencil.texture)
		{
			width = dst->depth_stencil.texture->width;
			height = dst->depth_stencil.texture->height;
		}
	}
	uint32_t gl_buffers = 0;
	if (buffers & GFX_BUFFER_COLOR_BIT)
		gl_buffers |= GL_COLOR_BUFFER_BIT;
	if (buffers & GFX_BUFFER_DEPTH_BIT)
		gl_buffers |= GL_DEPTH_BUFFER_BIT;
	if (buffers & GFX_BUFFER_STENCIL_BIT)
		gl_buffers |= GL_STENCIL_BUFFER_BIT;
	assert(width != 0 && height != 0);
	if (width == 0 || height == 0)
		return;
	GL4_CALL(BlitNamedFramebuffer, src ? src->handle.u32[0] : 0, dst ? dst->handle.u32[0] : 0, 0, 0, width, height, 0, 0, width, height, gl_buffers, GL_NEAREST);
}

static bool gl4_create_pipeline_state(struct gfx_device *device, gfx_pipeline_state_t *state, const gfx_shader_state_t *shader_state, const gfx_rasterizer_state_t *rasterizer, const gfx_depth_stencil_state_t *depth_stencil, const gfx_blend_state_t *blend, const gfx_input_layout_t *input_layout, enum gfx_primitive_type primitive)
{
	assert(!state->handle.u64);
	state->handle.u64 = ++GL_DEVICE->state_idx;
	state->shader_state = shader_state;
	state->rasterizer_state = rasterizer;
	state->depth_stencil_state = depth_stencil;
	state->blend_state = blend;
	state->input_layout = input_layout;
	state->primitive = primitive;
	return true;
}

static void gl4_delete_pipeline_state(struct gfx_device *device, gfx_pipeline_state_t *state)
{
	(void)device;
	if (!state || !state->handle.u64)
		return;
	state->handle.u64 = 0;
}

static void gl4_bind_pipeline_state(struct gfx_device *device, const gfx_pipeline_state_t *state)
{
	assert(state->handle.u64);
	if (GL_DEVICE->pipeline_state == state->handle.u64)
		return;
	GL_DEVICE->pipeline_state = state->handle.u64;
	gl4_bind_shader_state(device, state->shader_state);
	gl4_bind_rasterizer_state(device, state->rasterizer_state);
	gl4_bind_depth_stencil_state(device, state->depth_stencil_state);
	gl4_bind_blend_state(device, state->blend_state);
	//gl4_bind_input_layout(device, state->input_layout);
	GL4_DEVICE->primitive = state->primitive;
}

static void gl4_set_viewport(struct gfx_device *device, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
	(void)device;
	GL4_CALL(Viewport, x, y, width, height);
}

static void gl4_set_scissor(struct gfx_device *device, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
	(void)device;
	GL4_CALL(Scissor, x, y, width, height);
}

static void gl4_set_line_width(struct gfx_device *device, float line_width)
{
	if (GL_DEVICE->line_width == line_width)
		return;
	GL_DEVICE->line_width = line_width;
	GL4_CALL(LineWidth, line_width);
}

static void gl4_set_point_size(struct gfx_device *device, float point_size)
{
	if (GL_DEVICE->point_size == point_size)
		return;
	GL_DEVICE->point_size = point_size;
	GL4_CALL(PointSize, point_size);
}

static const struct gfx_device_vtable gl4_vtable =
{
	GFX_DEVICE_VTABLE_DEF(gl4)
};

struct gfx_device *gfx_gl4_device_new(struct gfx_window *window, gfx_gl_load_addr_t *load_addr)
{
	struct gfx_gl4_device *device = GFX_MALLOC(sizeof(*device));
	if (!device)
		return NULL;
	GL_DEVICE->load_addr = load_addr;
	struct gfx_device *dev = &device->gl.device;
	dev->vtable = &gl4_vtable;
	if (!dev->vtable->ctr(dev, window))
	{
		dev->vtable->dtr(dev);
		GFX_FREE(device);
		return NULL;
	}
	return dev;
}
