#include "internal.h"

GLuint
glGenLists(GLsizei range)
{
	struct gl_ctx *ctx = g_ctx;

	if (range < 0)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return 0;
	}
	if (ctx->immediate.enable)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return 0;
	}
}

void
glDeleteLists(GLuint list, GLsizei range)
{
	struct gl_ctx *ctx = g_ctx;

	if (ctx->immediate.enable)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
}

GLboolean
glIsList(GLuint list)
{
	struct gl_ctx *ctx = g_ctx;

	if (ctx->immediate.enable)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return GL_FALSE;
	}
	/* XXX */
	return GL_FALSE;
}

void
glListBase(GLuint base)
{
	struct gl_ctx *ctx = g_ctx;

	if (ctx->immediate.enable)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
}

void
glNewList(GLuint list, GLenum mode)
{
	struct gl_ctx *ctx = g_ctx;

	if (ctx->immediate.enable)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
}

void
glEndList(void)
{
	struct gl_ctx *ctx = g_ctx;

	if (ctx->immediate.enable)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
}

void
glCallList(GLuint list)
{
	struct gl_ctx *ctx = g_ctx;

	if (ctx->immediate.enable)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
}

void
glCallLists(GLsizei n, GLenum type, const GLvoid *lists)
{
	struct gl_ctx *ctx = g_ctx;

	if (ctx->immediate.enable)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
}
