#include "internal.h"

void
glClear(GLbitfield mask)
{
	struct jkg_clear_cmd cmd;
	struct gl_ctx *ctx = g_ctx;
	int ret;

	if (mask & ~(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT))
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	if (!mask)
		return;
	if (ctx->rasterizer_state.rasterizer_discard_enable)
		return;
	if (gl_ctx_commit(ctx, GL_CTX_DIRTY_VIEWPORT
	                     | GL_CTX_DIRTY_SCISSOR
	                     | GL_CTX_DIRTY_DEPTH_STENCIL_STATE
	                     | GL_CTX_DIRTY_RASTERIZER_STATE
	                     | GL_CTX_DIRTY_DRAW_FBO))
		return;
	cmd.mask = 0;
	if (mask & GL_COLOR_BUFFER_BIT)
	{
		for (size_t i = 0; i < 4; ++i)
			cmd.color[i] = ctx->clear_color[i];
		cmd.mask |= JKG_CLEAR_COLOR;
	}
	if (mask & GL_DEPTH_BUFFER_BIT)
	{
		cmd.depth = ctx->clear_depth;
		cmd.mask |= JKG_CLEAR_DEPTH;
	}
	if (mask & GL_STENCIL_BUFFER_BIT)
	{
		cmd.stencil = ctx->clear_stencil;
		cmd.mask |= JKG_CLEAR_STENCIL;
	}
	ret = ctx->jkg_op->clear(ctx->jkg_ctx, &cmd);
	if (ret)
	{
		GL_SET_ERR(ctx, get_jkg_error(ret));
		return;
	}
}

void
glClearColor(GLfloat red,
             GLfloat green,
             GLfloat blue,
             GLfloat alpha)
{
	struct gl_ctx *ctx = g_ctx;

	ctx->clear_color[0] = clampf(red, 0, 1);
	ctx->clear_color[1] = clampf(green, 0, 1);
	ctx->clear_color[2] = clampf(blue, 0, 1);
	ctx->clear_color[3] = clampf(alpha, 0, 1);
}

void
glClearDepth(GLdouble depth)
{
	struct gl_ctx *ctx = g_ctx;

	ctx->clear_depth = clampf(depth, 0, 1);
}

void
glClearDepthf(GLfloat depth)
{
	struct gl_ctx *ctx = g_ctx;

	ctx->clear_depth = clampf(depth, 0, 1);
}

void
glClearStencil(GLint stencil)
{
	struct gl_ctx *ctx = g_ctx;

	ctx->clear_stencil = stencil;
}
