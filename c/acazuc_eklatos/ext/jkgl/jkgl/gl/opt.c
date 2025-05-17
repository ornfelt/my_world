#include "internal.h"

static void
set(struct gl_ctx *ctx, GLenum cap, GLboolean v)
{
	switch (cap)
	{
		case GL_BLEND:
			for (GLuint i = 0; i < ctx->jkg_caps->max_draw_buffers; ++i)
			{
				if (ctx->blend_state.attachments[i].enable != v)
				{
					ctx->blend_state.attachments[i].enable = v;
					ctx->dirty |= GL_CTX_DIRTY_BLEND_STATE;
				}
			}
			break;
		case GL_COLOR_LOGIC_OP:
			if (ctx->blend_state.logic_enable != v)
			{
				ctx->blend_state.logic_enable = v;
				ctx->dirty |= GL_CTX_DIRTY_BLEND_STATE;
			}
			break;
		case GL_CULL_FACE:
			if (ctx->rasterizer_state.cull_enable != v)
			{
				ctx->rasterizer_state.cull_enable = v;
				ctx->dirty |= GL_CTX_DIRTY_RASTERIZER_STATE;
			}
			break;
		case GL_DEPTH_CLAMP:
			if (ctx->rasterizer_state.depth_clamp_enable != v)
			{
				ctx->rasterizer_state.depth_clamp_enable = v;
				ctx->dirty |= GL_CTX_DIRTY_RASTERIZER_STATE;
			}
			break;
		case GL_DEPTH_TEST:
			if (ctx->depth_stencil_state.depth_test_enable != v)
			{
				ctx->depth_stencil_state.depth_test_enable = v;
				ctx->dirty |= GL_CTX_DIRTY_DEPTH_STENCIL_STATE;
			}
			break;
		case GL_LIGHT0:
		case GL_LIGHT1:
		case GL_LIGHT2:
		case GL_LIGHT3:
		case GL_LIGHT4:
		case GL_LIGHT5:
		case GL_LIGHT6:
		case GL_LIGHT7:
			ctx->fixed.block.lights[cap - GL_LIGHT0].enable = v;
			break;
		case GL_LIGHTING:
			ctx->fixed.block.lighting_enable = v;
			break;
		case GL_LINE_SMOOTH:
			if (ctx->rasterizer_state.line_smooth_enable != v)
			{
				ctx->rasterizer_state.line_smooth_enable = v;
				ctx->dirty |= GL_CTX_DIRTY_RASTERIZER_STATE;
			}
			break;
		case GL_POINT_SMOOTH:
			if (ctx->rasterizer_state.point_smooth_enable != v)
			{
				ctx->rasterizer_state.point_smooth_enable = v;
				ctx->dirty |= GL_CTX_DIRTY_RASTERIZER_STATE;
			}
			break;
		case GL_FOG:
			ctx->fixed.block.fog_enable = v;
			break;
		case GL_MULTISAMPLE:
			if (ctx->rasterizer_state.multisample_enable != v)
			{
				ctx->rasterizer_state.multisample_enable = v;
				ctx->dirty |= GL_CTX_DIRTY_RASTERIZER_STATE;
			}
			break;
		case GL_PRIMITIVE_RESTART:
			ctx->primitive_restart_enable = v;
			break;
		case GL_PRIMITIVE_RESTART_FIXED_INDEX:
			ctx->primitive_restart_fixed_index_enable = v;
			break;
		case GL_RASTERIZER_DISCARD:
			if (ctx->rasterizer_state.rasterizer_discard_enable != v)
			{
				ctx->rasterizer_state.rasterizer_discard_enable = v;
				ctx->dirty |= GL_CTX_DIRTY_RASTERIZER_STATE;
			}
			break;
		case GL_SCISSOR_TEST:
			if (ctx->rasterizer_state.scissor_enable != v)
			{
				ctx->rasterizer_state.scissor_enable = v;
				ctx->dirty |= GL_CTX_DIRTY_RASTERIZER_STATE;
			}
			break;
		case GL_STENCIL_TEST:
			if (ctx->depth_stencil_state.stencil_enable != v)
			{
				ctx->depth_stencil_state.stencil_enable = v;
				ctx->dirty |= GL_CTX_DIRTY_DEPTH_STENCIL_STATE;
			}
			break;
		case GL_TEXTURE_1D:
			ctx->fixed.block.texture_1d_enable = v;
			break;
		case GL_TEXTURE_2D:
			ctx->fixed.block.texture_2d_enable = v;
			break;
		case GL_TEXTURE_3D:
			ctx->fixed.block.texture_3d_enable = v;
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			break;
	}
}

void
glEnable(GLenum cap)
{
	set(g_ctx, cap, GL_TRUE);
}

void
glDisable(GLenum cap)
{
	set(g_ctx, cap, GL_FALSE);
}

GLboolean
glIsEnabled(GLenum cap)
{
	struct gl_ctx *ctx = g_ctx;

	switch (cap)
	{
		case GL_BLEND:
			return ctx->blend_state.attachments[0].enable;
		case GL_COLOR_ARRAY:
			return ctx->fixed.arrays.color.enable;
		case GL_COLOR_LOGIC_OP:
			return ctx->blend_state.logic_enable;
		case GL_CULL_FACE:
			return ctx->rasterizer_state.cull_enable;
		case GL_DEPTH_CLAMP:
			return ctx->rasterizer_state.depth_clamp_enable;
		case GL_DEPTH_TEST:
			return ctx->depth_stencil_state.depth_test_enable;
		case GL_FOG:
			return ctx->fixed.block.fog_enable;
		case GL_LIGHT0:
		case GL_LIGHT1:
		case GL_LIGHT2:
		case GL_LIGHT3:
		case GL_LIGHT4:
		case GL_LIGHT5:
		case GL_LIGHT6:
		case GL_LIGHT7:
			return ctx->fixed.block.lights[cap - GL_LIGHT0].enable;
		case GL_LIGHTING:
			return ctx->fixed.block.lighting_enable;
		case GL_LINE_SMOOTH:
			return ctx->rasterizer_state.line_smooth_enable;
		case GL_MULTISAMPLE:
			return ctx->rasterizer_state.multisample_enable;
		case GL_NORMAL_ARRAY:
			return ctx->fixed.arrays.normal.enable;
		case GL_POINT_SMOOTH:
			return ctx->rasterizer_state.point_smooth_enable;
		case GL_PRIMITIVE_RESTART:
			return ctx->primitive_restart_enable;
		case GL_PRIMITIVE_RESTART_FIXED_INDEX:
			return ctx->primitive_restart_fixed_index_enable;
		case GL_RASTERIZER_DISCARD:
			return ctx->rasterizer_state.rasterizer_discard_enable;
		case GL_SCISSOR_TEST:
			return ctx->rasterizer_state.scissor_enable;
		case GL_STENCIL_TEST:
			return ctx->depth_stencil_state.stencil_enable;
		case GL_TEXTURE_COORD_ARRAY:
			return ctx->fixed.arrays.texcoord.enable;
		case GL_TEXTURE_1D:
			return ctx->fixed.block.texture_1d_enable;
		case GL_TEXTURE_2D:
			return ctx->fixed.block.texture_2d_enable;
		case GL_TEXTURE_3D:
			return ctx->fixed.block.texture_3d_enable;
		case GL_VERTEX_ARRAY:
			return ctx->fixed.arrays.vertex.enable;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return GL_FALSE;
	}
}
