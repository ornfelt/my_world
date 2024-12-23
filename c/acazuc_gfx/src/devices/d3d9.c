#define COBJMACROS
#define CINTERFACE
#define D3D11_NO_HELPER
#define WIN32_LEAN_AND_MEAN
#define INITGUID

#include "device_vtable.h"
#include "devices.h"
#include "device.h"
#include "window.h"

#include <stdlib.h>
#include <assert.h>
#include <d3d9.h>

#define D3D9_DEVICE ((struct gfx_d3d9_device*)device)

struct gfx_d3d9_device
{
	struct gfx_d3d_device device;
	IDirect3DDevice9 *d3ddev;
	struct
	{
		enum gfx_texture_addressing addressing_u;
		enum gfx_texture_addressing addressing_v;
		enum gfx_texture_addressing addressing_w;
		enum gfx_filtering min_filtering;
		enum gfx_filtering mag_filtering;
		enum gfx_filtering mip_filtering;
		uint32_t anisotropy;
	} samplers[16];
	void *textures[16];
	/* blend */
	enum gfx_blend_equation blend_equation_c;
	enum gfx_blend_equation blend_equation_a;
	enum gfx_blend_function blend_src_c;
	enum gfx_blend_function blend_src_a;
	enum gfx_blend_function blend_dst_c;
	enum gfx_blend_function blend_dst_a;
	bool blending;
	/* stencil */
	enum gfx_stencil_operation stencil_fail;
	enum gfx_stencil_operation stencil_pass;
	enum gfx_stencil_operation stencil_zfail;
	enum gfx_compare_function stencil_compare;
	uint32_t stencil_compare_mask;
	uint32_t stencil_write_mask;
	uint32_t stencil_reference;
	bool stencil_enabled;
	/* depth */
	enum gfx_compare_function depth_func;
	bool depth_mask;
	bool depth_test;
	/* rasterizer */
	enum gfx_fill_mode fill_mode;
	enum gfx_cull_mode cull_mode;
	enum gfx_front_face front_face;
	uint64_t state_idx;
	uint64_t blend_state;
	uint64_t stencil_state;
	uint64_t depth_state;
	uint64_t rasterizer_state;
};

static const D3DPRIMITIVETYPE primitive_types[] =
{
	D3DPT_TRIANGLELIST,
	D3DPT_TRIANGLESTRIP,
	D3DPT_POINTLIST,
	D3DPT_POINTLIST,
	D3DPT_LINESTRIP,
};

static const D3DCMPFUNC compare_functions[] =
{
	D3DCMP_NEVER,
	D3DCMP_LESS,
	D3DCMP_LESSEQUAL,
	D3DCMP_EQUAL,
	D3DCMP_GREATEREQUAL,
	D3DCMP_GREATER,
	D3DCMP_NOTEQUAL,
	D3DCMP_ALWAYS,
};

static const D3DBLEND blend_functions[] =
{
	D3DBLEND_ZERO,
	D3DBLEND_ONE,
	D3DBLEND_SRCCOLOR,
	D3DBLEND_INVSRCCOLOR,
	D3DBLEND_DESTCOLOR,
	D3DBLEND_INVDESTCOLOR,
	D3DBLEND_SRCALPHA,
	D3DBLEND_INVSRCALPHA,
	D3DBLEND_DESTALPHA,
	D3DBLEND_INVDESTALPHA,
	D3DBLEND_BLENDFACTOR,
	D3DBLEND_INVBLENDFACTOR,
};

static const D3DBLENDOP blend_equations[] =
{
	D3DBLENDOP_ADD,
	D3DBLENDOP_SUBTRACT,
	D3DBLENDOP_REVSUBTRACT,
	D3DBLENDOP_MIN,
	D3DBLENDOP_MAX,
};

static const D3DSTENCILOP stencil_operations[] =
{
	D3DSTENCILOP_KEEP,
	D3DSTENCILOP_ZERO,
	D3DSTENCILOP_REPLACE,
	D3DSTENCILOP_INCRSAT,
	D3DSTENCILOP_INCR,
	D3DSTENCILOP_DECRSAT,
	D3DSTENCILOP_DECR,
	D3DSTENCILOP_INVERT,
};

static const D3DFILLMODE fill_modes[] =
{
	D3DFILL_POINT,
	D3DFILL_WIREFRAME,
	D3DFILL_SOLID,
};

static const D3DTEXTUREADDRESS texture_addressings[] =
{
	D3DTADDRESS_CLAMP,
	D3DTADDRESS_WRAP,
	D3DTADDRESS_MIRROR,
	D3DTADDRESS_BORDER,
	D3DTADDRESS_MIRRORONCE,
};

static const D3DTEXTUREFILTERTYPE filterings[] =
{
	D3DTEXF_NONE,
	D3DTEXF_POINT,
	D3DTEXF_LINEAR,
};

static const D3DCULL cull_modes[] =
{
	D3DCULL_NONE, D3DCULL_CW, D3DCULL_CCW,
	D3DCULL_NONE, D3DCULL_CCW, D3DCULL_CW,
};

static void d3d9_ctr(struct gfx_device *device)
{
	for (uint32_t i = 0; i < sizeof(D3D9_DEVICE->samplers) / sizeof(*D3D9_DEVICE->samplers); ++i)
	{
		D3D9_DEVICE->samplers[i].addressing_u = GFX_TEXTURE_ADDRESSING_REPEAT;
		D3D9_DEVICE->samplers[i].addressing_v = GFX_TEXTURE_ADDRESSING_REPEAT;
		D3D9_DEVICE->samplers[i].addressing_w = GFX_TEXTURE_ADDRESSING_REPEAT;
		D3D9_DEVICE->samplers[i].min_filtering = GFX_FILTERING_NEAREST;
		D3D9_DEVICE->samplers[i].mag_filtering = GFX_FILTERING_NEAREST;
		D3D9_DEVICE->samplers[i].mip_filtering = GFX_FILTERING_NEAREST;
		D3D9_DEVICE->samplers[i].anisotropy = 1;
	}
	D3D9_DEVICE->d3ddev->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, true);
}

static void d3d9_dtr(struct gfx_device *device)
{
	(void)device;
}

static void d3d9_tick(struct gfx_device *device)
{
	(void)device;
}

static void d3d9_clear_color(struct gfx_device *device, const gfx_render_target_t *render_target, enum gfx_render_target_attachment attachment, vec4f_t color)
{
	uint32_t argb;
	argb  = (uint32_t)(color.x * 255) << 24;
	argb |= (uint32_t)(color.y * 255) << 16;
	argb |= (uint32_t)(color.z * 255) << 8;
	argb |= (uint32_t)(color.w * 255) << 0;
	if (render_target)
		D3D9_DEVICE->d3ddev->SetRenderTarget(0, (IDirect3DSurface9*)render_target->colors[attachment - GFX_RENDERTARGET_ATTACHMENT_COLOR0].ptr);
	else
		D3D9_DEVICE->d3ddev->SetRenderTarget(0, NULL);
	D3D9_DEVICE->d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, argb, 0, 0);
}

static void d3d9_clear_depth_stencil(struct gfx_device *device, const gfx_render_target_t *render_target, float depth, uint8_t stencil)
{
	if (render_target)
		D3D9_DEVICE->d3ddev->SetDepthStencilSurface((IDirect3DSurface9*)render_target->depth_stencil.ptr);
	else
		D3D9_DEVICE->d3ddev->SetDepthStencilSurface(NULL);
	D3D9_DEVICE->d3ddev->Clear(0, NULL, D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 0, depth, stencil);
}

static void d3d9_draw_indexed_instanced(struct gfx_device *device, enum gfx_primitive_type primitive, uint32_t count, uint32_t offset, uint32_t prim_count)
{
	(void)prim_count;
	D3D9_DEVICE->d3ddev->DrawIndexedPrimitive(primitive_types[primitive], 0, 0, 0, offset, count);
}

static void d3d9_draw_instanced(struct gfx_device *device, enum gfx_primitive_type primitive, uint32_t count, uint32_t offset, uint32_t prim_count)
{
	(void)prim_count;
	D3D9_DEVICE->d3ddev->DrawPrimitive(primitive_types[primitive], offset, count);
}

static void d3d9_draw_indexed(struct gfx_device *device, enum gfx_primitive_type primitive, uint32_t count, uint32_t offset)
{
	D3D9_DEVICE->d3ddev->DrawIndexedPrimitive(primitive_types[primitive], 0, 0, 0, offset, count);
}

static void d3d9_draw(struct gfx_device *device, enum gfx_primitive_type primitive, uint32_t count, uint32_t offset)
{
	D3D9_DEVICE->d3ddev->DrawPrimitive(primitive_types[primitive], offset, count);
}

static void d3d9_create_blend_state(struct gfx_device *device, gfx_blend_state_t *state, bool enabled, enum gfx_blend_function src_c, enum gfx_blend_function dst_c, enum gfx_blend_function src_a, enum gfx_blend_function dst_a, enum gfx_blend_equation equation_c, enum gfx_blend_equation equation_a)
{
	assert(!state->handle.u64);
	state->device = device;
	state->handle.u64 = ++D3D9_DEVICE->state_idx;
	state->enabled = enabled;
	state->src_c = src_c;
	state->dst_c = dst_c;
	state->src_a = src_a;
	state->dst_a = dst_a;
	state->equation_c = equation_c;
	state->equation_a = equation_a;
}

static void d3d9_bind_blend_state(struct gfx_device *device, const gfx_blend_state_t *state)
{
	assert(state->handle.u64);
	if (state->handle.u64 == D3D9_DEVICE->blend_state)
		return;
	if (state->enabled)
	{
		if (!D3D9_DEVICE->blending)
		{
			D3D9_DEVICE->blending = true;
			D3D9_DEVICE->d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
		}
		if (D3D9_DEVICE->blend_src_c != state->src_c)
		{
			D3D9_DEVICE->blend_src_c = state->src_c;
			D3D9_DEVICE->d3ddev->SetRenderState(D3DRS_SRCBLEND, blend_functions[state->src_c]);
		}
		if (D3D9_DEVICE->blend_src_a != state->src_a)
		{
			D3D9_DEVICE->blend_src_a = state->src_a;
			D3D9_DEVICE->d3ddev->SetRenderState(D3DRS_SRCBLENDALPHA, blend_functions[state->src_a]);
		}
		if (D3D9_DEVICE->blend_dst_c != state->dst_c)
		{
			D3D9_DEVICE->blend_dst_c = state->dst_c;
			D3D9_DEVICE->d3ddev->SetRenderState(D3DRS_DESTBLEND, blend_functions[state->dst_c]);
		}
		if (D3D9_DEVICE->blend_dst_a != state->dst_a)
		{
			D3D9_DEVICE->blend_dst_a = state->dst_a;
			D3D9_DEVICE->d3ddev->SetRenderState(D3DRS_DESTBLENDALPHA, blend_functions[state->dst_a]);
		}
		if (D3D9_DEVICE->blend_equation_c != state->equation_c)
		{
			D3D9_DEVICE->blend_equation_c = state->equation_c;
			D3D9_DEVICE->d3ddev->SetRenderState(D3DRS_BLENDOP, blend_equations[state->equation_c]);
		}
		if (D3D9_DEVICE->blend_equation_a != state->equation_a)
		{
			D3D9_DEVICE->blend_equation_a = state->equation_a;
			D3D9_DEVICE->d3ddev->SetRenderState(D3DRS_BLENDOPALPHA, blend_equations[state->equation_a]);
		}
	}
	else
	{
		if (D3D9_DEVICE->blending)
		{
			D3D9_DEVICE->blending = false;
			D3D9_DEVICE->d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
		}
	}
}

static void d3d9_delete_blend_state(struct gfx_device *device, gfx_blend_state_t *state)
{
	(void)device;
	if (!state)
		return;
	state->handle.u64 = 0;
}

static void d3d9_create_depth_stencil_state(struct gfx_device *device, gfx_depth_stencil_state_t *state, bool depth_write, bool depth_test, enum gfx_compare_function depth_compare, bool stencil_enabled, uint32_t stencil_write_mask, enum gfx_compare_function stencil_compare, uint32_t stencil_reference, uint32_t stencil_compare_mask, enum gfx_stencil_operation stencil_fail, enum gfx_stencil_operation stencil_zfail, enum gfx_stencil_operation stencil_pass)
{
	assert(!state->handle.u64);
	state->device = device;
	state->handle.u64 = ++D3D9_DEVICE->state_idx;
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
}

static void d3d9_bind_depth_stencil_state(struct gfx_device *device, const gfx_depth_stencil_state_t *state)
{
	assert(state->handle.u64);
	if (state->handle.u64 == D3D9_DEVICE->stencil_state)
		return;
	if (state->depth_test)
	{
		if (!D3D9_DEVICE->depth_test)
		{
			D3D9_DEVICE->depth_test = true;
			D3D9_DEVICE->d3ddev->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
		}
		if (D3D9_DEVICE->depth_mask != state->depth_write)
		{
			D3D9_DEVICE->depth_mask = state->depth_write;
			D3D9_DEVICE->d3ddev->SetRenderState(D3DRS_ZWRITEENABLE, state->depth_write);
		}
		if (D3D9_DEVICE->depth_func != state->depth_compare)
		{
			D3D9_DEVICE->depth_func = state->depth_compare;
			D3D9_DEVICE->d3ddev->SetRenderState(D3DRS_ZFUNC, compare_functions[state->depth_compare]);
		}
	}
	else
	{
		if (D3D9_DEVICE->depth_test)
		{
			D3D9_DEVICE->depth_test = false;
			D3D9_DEVICE->d3ddev->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
		}
	}
	if (state->stencil_enabled)
	{
		if (!D3D9_DEVICE->stencil_enabled)
		{
			D3D9_DEVICE->stencil_enabled = true;
			D3D9_DEVICE->d3ddev->SetRenderState(D3DRS_STENCILENABLE, true);
		}
		if (D3D9_DEVICE->stencil_compare != state->stencil_compare)
		{
			D3D9_DEVICE->stencil_compare = state->stencil_compare;
			D3D9_DEVICE->d3ddev->SetRenderState(D3DRS_STENCILFUNC, compare_functions[state->stencil_compare]);
		}
		if (D3D9_DEVICE->stencil_reference != state->stencil_reference)
		{
			D3D9_DEVICE->stencil_reference = state->stencil_reference;
			D3D9_DEVICE->d3ddev->SetRenderState(D3DRS_STENCILREF, state->stencil_reference);
		}
		if (D3D9_DEVICE->stencil_compare_mask != state->stencil_compare_mask)
		{
			D3D9_DEVICE->stencil_compare_mask = state->stencil_compare_mask;
			D3D9_DEVICE->d3ddev->SetRenderState(D3DRS_STENCILMASK, state->stencil_compare_mask);
		}
		if (D3D9_DEVICE->stencil_write_mask != state->stencil_write_mask)
		{
			D3D9_DEVICE->stencil_write_mask = state->stencil_write_mask;
			D3D9_DEVICE->d3ddev->SetRenderState(D3DRS_STENCILWRITEMASK, state->stencil_write_mask);
		}
		if (D3D9_DEVICE->stencil_fail != state->stencil_fail)
		{
			D3D9_DEVICE->stencil_fail = state->stencil_fail;
			D3D9_DEVICE->d3ddev->SetRenderState(D3DRS_STENCILFAIL, stencil_operations[state->stencil_fail]);
		}
		if (D3D9_DEVICE->stencil_zfail != state->stencil_zfail)
		{
			D3D9_DEVICE->stencil_zfail = state->stencil_zfail;
			D3D9_DEVICE->d3ddev->SetRenderState(D3DRS_STENCILZFAIL, stencil_operations[state->stencil_zfail]);
		}
		if (D3D9_DEVICE->stencil_pass != state->stencil_pass)
		{
			D3D9_DEVICE->stencil_pass = state->stencil_pass;
			D3D9_DEVICE->d3ddev->SetRenderState(D3DRS_STENCILPASS, stencil_operations[state->stencil_pass]);
		}
	}
	else
	{
		if (D3D9_DEVICE->stencil_enabled)
		{
			D3D9_DEVICE->stencil_enabled = false;
			D3D9_DEVICE->d3ddev->SetRenderState(D3DRS_STENCILENABLE, false);
		}
	}
}

static void d3d9_delete_depth_stencil_state(struct gfx_device *device, gfx_depth_stencil_state_t *state)
{
	(void)device;
	if (!state)
		return;
	state->handle.u64 = 0;
}

static void d3d9_create_rasterizer_state(struct gfx_device *device, gfx_rasterizer_state_t *state, enum gfx_fill_mode fill_mode, enum gfx_cull_mode cull_mode, enum gfx_front_face front_face, bool scissor)
{
	assert(!state->handle.u64);
	state->device = device;
	state->handle.u64 = ++D3D9_DEVICE->state_idx;
	state->fill_mode = fill_mode;
	state->cull_mode = cull_mode;
	state->front_face = front_face;
	state->scissor = scissor;
}

static void d3d9_bind_rasterizer_state(struct gfx_device *device, const gfx_rasterizer_state_t *state)
{
	assert(state->handle.u64);
	if (state->handle.u64 == D3D9_DEVICE->rasterizer_state)
		return;
	if (D3D9_DEVICE->fill_mode != state->fill_mode)
	{
		D3D9_DEVICE->fill_mode = state->fill_mode;
		D3D9_DEVICE->d3ddev->SetRenderState(D3DRS_FILLMODE, fill_modes[state->fill_mode]);
	}
	if (D3D9_DEVICE->cull_mode != state->cull_mode || D3D9_DEVICE->front_face != state->front_face)
	{
		D3D9_DEVICE->cull_mode = state->cull_mode;
		D3D9_DEVICE->front_face = state->front_face;
		D3D9_DEVICE->d3ddev->SetRenderState(D3DRS_CULLMODE, cull_modes[state->front_face * 3 + state->cull_mode]);
	}
}

static void d3d9_delete_rasterizer_state(struct gfx_device *device, gfx_rasterizer_state_t *state)
{
	(void)device;
	if (!state)
		return;
	state->handle.u64 = 0;
}

static void d3d9_create_buffer(struct gfx_device *device, gfx_buffer_t *buffer, enum gfx_buffer_type type, const void *data, uint32_t size, enum gfx_buffer_usage usage)
{
	assert(!buffer->handle.ptr);
	buffer->device = device;
	buffer->usage = usage;
	buffer->type = type;
	buffer->size = size;
	UINT d3d_usage = D3DUSAGE_WRITEONLY;
	switch (type)
	{
		case GFX_BUFFER_VERTEXES:
			D3D9_CALL(D3D9_DEVICE->d3ddev->CreateVertexBuffer, size, d3d_usage, 0, pools[usage], (IDirect3DIndexBuffer9**)&buffer->handle.ptr, NULL);
			break;
		case GFX_BUFFER_INDICES:
			D3D9_CALL(D3D9_DEVICE->d3ddev->CreateIndexBuffer, size, d3d_usage, formats[format], pools[usage], (IDirect3DIndexBuffer9**)&buffer->handle.ptr, NULL);
			break;
		case GFX_BUFFER_UNIFORM:
			//XXX: emulate
			break;
	}
}

static bool d3d9_set_buffer_data(struct gfx_device *device, gfx_buffer_t *buffer, const void *data, uint32_t size, uint32_t offset)
{
	assert(buffer->handle.ptr);
	void *ptr;
	switch (buffer->type)
	{
		case GFX_BUFFER_VERTEXES:
			((IDirect3DVertexBuffer9*)buffer->handle.ptr)->Lock(offset, size, &data, 0);
			memcpy(ptr, data, size);
			((IDirect3DVertexBuffer9*)buffer->handle.ptr)->Unlock();
			break;
		case GFX_BUFFER_INDICES:
			((IDirect3DIndexBuffer9*)buffer->handle.ptr)->Lock(offset, size, &data, 0);
			memcpy(ptr, data, size);
			((IDirect3DIndexBuffer9*)buffer->handle.ptr)->Unlock();
			break;
		case GFX_BUFFER_UNIFORM;
			//XXX: emulate
			break;
	}
	return true; //XXX
}

static void d3d9_delete_buffer(struct gfx_device *device, gfx_buffer_t *buffer)
{
	if (!buffer || !buffer-handle.ptr)
		return;
	switch (buffer->type)
	{
		case GFX_BUFFER_VERTEXES:
			((IDirect3DVertexBuffer9*)buffer->handle.ptr)->Release();
			break;
		case GFX_BUFFER_INDICES:
			((IDirect3DIndexBuffer9*)buffer->handle.ptr)->Release();
			break;
		case GFX_BUFFER_UNIFORM:
				//XXX: emulate
			break;
	}
	//queue ?
	buffer->handle.ptr = NULL;
}

static void d3d9_create_attributes_state(struct gfx_device *device, gfx_attributes_state_t *state)
{
}

static void d3d9_bind_attributes_state(struct gfx_device *device, const gfx_attributes_state_t *state, const gfx_program_t *program)
{
}

static void d3d9_bind_attribute(struct gfx_device *device, gfx_attributes_state_t *state, uint32_t bind, const gfx_buffer_t *buffer, enum gfx_attribute_type type, uint32_t stride, uint32_t offset)
{
}

static void d3d9_bind_index(struct gfx_device *device, gfx_attributes_state_t *state, const gfx_buffer_t *buffer, enum gfx_index_type type)
{
}

static void d3d9_delete_attributes_state(struct gfx_device *device, gfx_attributes_state_t *state)
{
}

static void d3d9_create_texture(struct gfx_device *device, gfx_texture_t *texture, enum gfx_texture_type type, enum gfx_format format, uint8_t lod, uint32_t width, uint32_t height, uint32_t depth)
{
}

static bool d3d9_set_texture_data(struct gfx_device *device, gfx_texture_t *texture, uint8_t lod, uint32_t offset, uint32_t width, uint32_t height, uint32_t depth, uint32_t size, const void *data)
{
}

static void d3d9_set_texture_addressing(struct gfx_device *device, gfx_texture_t *texture, enum gfx_texture_addressing addressing_s, enum gfx_texture_addressing addressing_t, enum gfx_texture_addressing addressing_r)
{
	(void)device;
	assert(texture->handle.u64);
	texture->addressing_s = addressing_s;
	texture->addressing_t = addressing_t;
	texture->addressing_r = addressing_r;
}

static void d3d9_set_texture_filtering(struct gfx_device *device, gfx_texture_t *texture, enum gfx_filtering min_filtering, enum gfx_filtering mag_filtering, enum gfx_filtering mip_filtering)
{
	(void)device;
	assert(texture->handle.u64);
	texture->min_filtering = min_filtering;
	texture->mag_filtering = mag_filtering;
	texture->mip_filtering = mip_filtering;
}

static void d3d9_set_texture_anisotropy(struct gfx_device *device, gfx_texture_t *texture, uint32_t anisotropy)
{
	(void)device;
	assert(texture->handle.u64);
	texture->anisotropy = anisotropy;
}

static void d3d9_set_texture_levels(struct gfx_device *device, gfx_texture_t *texture, uint32_t min_level, uint32_t max_level)
{
	(void)device;
	assert(texture->handle.u64);
}

static void d3d9_delete_texture(struct gfx_device *device, gfx_texture_t *texture)
{
}

static void d3d9_create_render_buffer(struct gfx_device *device, gfx_render_buffer_t *render_buffer)
{
}

static void d3d9_set_render_buffer_storage(struct gfx_device *device, gfx_render_buffer_t *render_buffer, enum gfx_format format, uint32_t width, uint32_t height, uint32_t color_samples, uint32_t coverage_samples)
{
}

static bool d3d9_create_shader(struct gfx_device *device, gfx_shader_t *shader, enum gfx_shader_type type, const uint8_t *data, uint32_t size)
{
}

static void d3d9_delete_shader(struct gfx_device *device, gfx_shader_t *shader)
{
}

static bool d3d9_create_program(struct gfx_device *device, gfx_program_t *program, const gfx_shader_t *vertex_shader, const gfx_shader_t *fragment_shader, const gfx_shader_t *geometry_shader, const gfx_program_attribute_t *attributes, const gfx_program_constant_t *constants, const gfx_program_sampler_t *samplers)
{
}

static void d3d9_bind_program(struct gfx_device *device, const gfx_program_t *program)
{
}

static void d3d9_delete_program(struct gfx_device *device, gfx_program_t *program)
{
}

static void d3d9_bind_constant(struct gfx_device *device, uint32_t bind, const gfx_buffer_t *buffer, uint32_t size, uint32_t offset)
{
}

static void d3d9_bind_samplers(struct gfx_device *device, uint32_t start, uint32_t count, const gfx_texture_t **textures)
{
	for (size_t i = 0; i < count; ++i)
	{
		if (textures[i])
		{
			if (textures[i]->addressing_s != D3D9_DEVICE->samplers[start + i].addressing_s)
			{
				D3D9_DEVICE->samplers[start + i].addressing_s = textures[i]->addressing_s;
				D3D9_DEVICE->d3ddev->SetSamplerState(start + i, D3DSAMP_ADDRESSU, texture_addressings[textures[i]->addressing_s]);
			}
			if (textures[i]->addressing_t != D3D9_DEVICE->samplers[start + i].addressing_t)
			{
				D3D9_DEVICE->samplers[start + i].addressing_t = textures[i]->addressing_t;
				D3D9_DEVICE->d3ddev->SetSamplerState(start + i, D3DSAMP_ADDRESSV, texture_addressings[textures[i]->addressing_t]);
			}
			if (textures[i]->addressing_r != D3D9_DEVICE->samplers[start + i].addressing_r)
			{
				D3D9_DEVICE->samplers[start + i].addressing_r = textures[i]->addressing_r;
				D3D9_DEVICE->d3ddev->SetSamplerState(start + i, D3DSAMP_ADDRESSW, texture_addressings[textures[i]->addressing_r]);
			}
			if (textures[i]->min_filtering != D3D9_DEVICE->samplers[start + i].min_filtering)
			{
				D3D9_DEVICE->samplers[start + i].min_filtering = textures[i]->min_filtering;
				D3D9_DEVICE->d3ddev->SetSamplerState(start + i, D3DSAMP_MINFILTER, filterings[textures[i]->min_filtering]);
			}
			if (textures[i]->mag_filtering != D3D9_DEVICE->samplers[start + i].mag_filtering)
			{
				D3D9_DEVICE->samplers[start + i].mag_filtering = textures[i]->mag_filtering;
				D3D9_DEVICE->d3ddev->SetSamplerState(start + i, D3DSAMP_MAGFILTER, filterings[textures[i]->mag_filtering]);
			}
			if (textures[i]->mip_filtering != D3D9_DEVICE->samplers[start + i].mip_filtering)
			{
				D3D9_DEVICE->samplers[start + i].mip_filtering = textures[i]->mip_filtering;
				D3D9_DEVICE->d3ddev->SetSamplerState(start + i, D3DSAMP_MIPFILTER, filterings[textures[i]->mip_filtering]);
			}
			if (textures[i]->anisotropy != D3D9_DEVICE->samplers[start + i].anisotropy)
			{
				D3D9_DEVICE->samplers[start + i].anisotropy = textures[i]->anisotropy;
				D3D9_DEVICE->d3ddev->SetSamplerState(start + i, D3DSAMP_MAXANISOTROPY, textures[i]->anisotropy);
			}
			if (texture[i]->handle.ptr != D3D9_DEVICE->textures[start + i])
			{
				D3D9_DEVICE->textures[i] = texture[i]->handle.ptr;
				D3D9_DEVICE->d3ddev->SetTexture(start + i, textures[i]->handle.ptr);
			}
		}
		else
		{
			if (D3D9_DEVICE->textures[start + i] != NULL)
			{
				D3D9_DEVICE->textures[start + i] = NULL;
				D3D9_DEVICE->d3ddev->SetTexture(start + i, NULL);
			}
		}
	}
}

static void d3d9_create_render_target(struct gfx_device *device, gfx_render_target_t *render_target)
{
}

static void d3d9_delete_render_target(struct gfx_device *device, gfx_render_target_t *render_target)
{
}

static void d3d9_bind_render_target(struct gfx_device *device, const gfx_render_target_t *render_target)
{
}

static void d3d9_blit_render_target(struct gfx_device *device, const gfx_render_target_t *src, vec2f_t src_pos, vec2f_t src_size, const gfx_render_target_t *dst, vec2f_t dst_pos, vec2f_t dst_size, uint32_t buffers, uint32_t color_src, uint32_t color_dst, bool linear)
{
}

static void d3d9_set_render_target_texture(struct gfx_device *device, gfx_render_target_t *render_target, enum gfx_render_target_attachment attachment, const gfx_texture_t *texture)
{
}

static void d3d9_set_render_target_render_buffer(struct gfx_device *device, gfx_render_target_t *render_target, enum gfx_render_target_attachment attachment, const gfx_render_buffer_t *render_buffer)
{
}

static void d3d9_set_render_target_draw_buffers(struct gfx_device *device, gfx_render_target_t *render_target, uint32_t *draw_buffers, uint32_t draw_buffers_count)
{
}

static void d3d9_set_viewport(struct gfx_device *device, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
	D3DVIEWPORT9 viewport;
	viewport.X = x;
	viewport.Y = y; //XXX: window->getHeight() - y
	viewport.Width = width;
	viewport.Height = height;
	viewport.MinZ = 0;
	viewport.MaxZ = 1;
	D3D9_DEVICE->d3ddev->SetViewport(&viewport);
}

static static void d3d9_set_scissor(struct gfx_device *device, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
	RECT rect;
	rect.left = x;
	rect.right = x + width;
	rect.top = y; //XXX: window->height - y
	rect.bottom = y + height;
	D3D9_DEVICE->d3ddev->SetScissorRect(&rect);
	D3D9_DEVICE->d3ddev->SetRenderState(D3DRS_SCISSORTESTENABLE, enabled);
}

static void d3d9_set_line_width(struct gfx_device *device, float line_width)
{
	(void)device;
	(void)line_width;
}

static void d3d9_set_point_size(struct gfx_device *device, float pointSize)
{
	D3D9_DEVICE->d3ddev->SetRenderState(D3DRS_POINTSIZE, pointSize);
}

static const struct gfx_device_vtable d3d9_vtable =
{
	GFX_DEVICE_VTABLE_DEF(d3d9)
};

struct gfx_device *gfx_d3d9_device_new(struct gfx_window *window, DXGI_SWAP_CHAIN_DESC *swap_chain_desc, IDXGISwapChain **swap_chain)
{
	struct gfx_d3d9_device *device = GFX_MALLOC(sizeof(*device));
	if (!device)
		return NULL;
	struct gfx_device *dev = &device->device;
	dev->vtable = &d3d11_vtable;
	D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_9_3;
	uint32_t creation_flags = 0;
#ifndef NDEBUG
	creation_flags |= D3D9_CREATE_DEVICE_DEBUG;
#endif
	HRESULT result = D3D9CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, creation_flags, &feature_level, 1, D3D11_SDK_VERSION, swap_chain_desc, swap_chain, (ID3D11Device**)&device->d3ddev, NULL, (ID3D11DeviceContext**)&device->d3dctx);
	if (FAILED(result))
	{
		GFX_ERROR_CALLBAKC("failed to create d3d9 device: ");
		goto err;
	}
	device->swap_chain = *swap_chain;
	if (!dev->vtable->ctr(dev, window))
		goto err;
	return dev;

err:
	dev->vtable->dtr(dev);
	GFX_FREE(device);
	return NULL;
}
