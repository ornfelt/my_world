#include "internal.h"

#include <stdlib.h>
#include <assert.h>

void
glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
	struct gl_ctx *ctx = g_ctx;

	if (width < 0 || height < 0)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	if (ctx->viewport[0] != x)
	{
		ctx->viewport[0] = x;
		ctx->dirty |= GL_CTX_DIRTY_VIEWPORT;
	}
	if (ctx->viewport[1] != y)
	{
		ctx->viewport[1] = y;
		ctx->dirty |= GL_CTX_DIRTY_VIEWPORT;
	}
	if (ctx->viewport[2] != width)
	{
		ctx->viewport[2] = width;
		ctx->dirty |= GL_CTX_DIRTY_VIEWPORT;
	}
	if (ctx->viewport[3] != height)
	{
		ctx->viewport[3] = height;
		ctx->dirty |= GL_CTX_DIRTY_VIEWPORT;
	}
}

void
glShadeModel(GLenum mode)
{
	struct gl_ctx *ctx = g_ctx;

	switch (mode)
	{
		case GL_SMOOTH:
		case GL_FLAT:
			ctx->fixed.shade_model = mode;
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_VALUE);
			return;
	}
}

GLenum
glGetError(void)
{
	struct gl_ctx *ctx = g_ctx;
	GLenum error;

	error = ctx->err;
	GL_SET_ERR(ctx, GL_NO_ERROR);
	return error;
}

void
glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
	struct gl_ctx *ctx = g_ctx;

	if (width < 0 || height < 0)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	if (ctx->scissor[0] != x)
	{
		ctx->scissor[0] = x;
		ctx->dirty |= GL_CTX_DIRTY_SCISSOR;
	}
	if (ctx->scissor[1] != y)
	{
		ctx->scissor[1] = y;
		ctx->dirty |= GL_CTX_DIRTY_SCISSOR;
	}
	if (ctx->scissor[2] != width)
	{
		ctx->scissor[2] = width;
		ctx->dirty |= GL_CTX_DIRTY_SCISSOR;
	}
	if (ctx->scissor[3] != height)
	{
		ctx->scissor[3] = height;
		ctx->dirty |= GL_CTX_DIRTY_SCISSOR;
	}
}

static void
flush(struct gl_ctx *ctx)
{
	int ret;

	ret = ctx->jkg_op->flush(ctx->jkg_ctx);
	if (ret)
	{
		GL_SET_ERR(ctx, get_jkg_error(ret));
		return;
	}
}

void
glFlush(void)
{
	flush(g_ctx);
}

void
glFinish(void)
{
	struct gl_ctx *ctx = g_ctx;

	flush(ctx);
	update_default_fbo(ctx);
}
