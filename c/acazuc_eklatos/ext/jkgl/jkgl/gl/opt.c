#include "internal.h"

static void set(GLenum cap, GLboolean v)
{
	switch (cap)
	{
		case GL_DEPTH_TEST:
			if (g_ctx->depth_test != v)
			{
				g_ctx->depth_test = v;
				g_ctx->dirty |= GL_CTX_DIRTY_DEPTH_TEST;
			}
			break;
		case GL_BLEND:
			if (g_ctx->blend != v)
			{
				g_ctx->blend = v;
				g_ctx->dirty |= GL_CTX_DIRTY_BLENDING;
			}
			break;
		case GL_FOG:
			g_ctx->fog = v;
			break;
		case GL_TEXTURE_1D:
			g_ctx->texture1d = v;
			break;
		case GL_TEXTURE_2D:
			g_ctx->texture2d = v;
			break;
		case GL_TEXTURE_3D:
			g_ctx->texture3d = v;
			break;
		case GL_POINT_SMOOTH:
			g_ctx->point_smooth = v;
			break;
		case GL_LINE_SMOOTH:
			g_ctx->line_smooth = v;
			break;
		case GL_CULL_FACE:
			g_ctx->enable_cull = v;
			break;
		case GL_LIGHTING:
			g_ctx->lighting = v;
			break;
		case GL_LIGHT0:
		case GL_LIGHT1:
		case GL_LIGHT2:
		case GL_LIGHT3:
		case GL_LIGHT4:
		case GL_LIGHT5:
		case GL_LIGHT6:
		case GL_LIGHT7:
			g_ctx->lights[cap - GL_LIGHT0].enabled = v;
			break;
		case GL_SCISSOR_TEST:
			g_ctx->scissor_test = v;
			break;
		default:
			g_ctx->errno = GL_INVALID_ENUM;
			break;
	}
}

void glEnable(GLenum cap)
{
	set(cap, GL_TRUE);
}

void glDisable(GLenum cap)
{
	set(cap, GL_FALSE);
}

GLboolean glIsEnabled(GLenum cap)
{
	switch (cap)
	{
		case GL_DEPTH_TEST:
			return g_ctx->depth_test;
		case GL_BLEND:
			return g_ctx->blend;
		case GL_FOG:
			return g_ctx->fog;
		case GL_COLOR_ARRAY:
			return g_ctx->color_array.enabled;
		case GL_VERTEX_ARRAY:
			return g_ctx->vertex_array.enabled;
		case GL_TEXTURE_COORD_ARRAY:
			return g_ctx->texture_coord_array.enabled;
		case GL_TEXTURE_1D:
			return g_ctx->texture1d;
		case GL_TEXTURE_2D:
			return g_ctx->texture2d;
		case GL_TEXTURE_3D:
			return g_ctx->texture3d;
		case GL_POINT_SMOOTH:
			return g_ctx->point_smooth;
		case GL_LINE_SMOOTH:
			return g_ctx->line_smooth;
		case GL_CULL_FACE:
			return g_ctx->enable_cull;
		case GL_LIGHTING:
			return g_ctx->lighting;
		case GL_LIGHT0:
		case GL_LIGHT1:
		case GL_LIGHT2:
		case GL_LIGHT3:
		case GL_LIGHT4:
		case GL_LIGHT5:
		case GL_LIGHT6:
		case GL_LIGHT7:
			return g_ctx->lights[cap - GL_LIGHT0].enabled;
		case GL_SCISSOR_TEST:
			return g_ctx->scissor_test;
		default:
			g_ctx->errno = GL_INVALID_ENUM;
			return GL_FALSE;
	}
}
