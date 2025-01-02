#include "internal.h"
#include "fixed.h"

GLuint glGenLists(GLsizei range)
{
	if (range < 0)
	{
		g_ctx->errno = GL_INVALID_VALUE;
		return 0;
	}
	if (g_ctx->immediate.enabled)
	{
		g_ctx->errno = GL_INVALID_OPERATION;
		return 0;
	}
}

void glDeleteLists(GLuint list, GLsizei range)
{
	if (g_ctx->immediate.enabled)
	{
		g_ctx->errno = GL_INVALID_OPERATION;
		return;
	}
}

GLboolean glIsList(GLuint list)
{
	if (g_ctx->immediate.enabled)
	{
		g_ctx->errno = GL_INVALID_OPERATION;
		return GL_FALSE;
	}
	/* XXX */
	return GL_FALSE;
}

void glListBase(GLuint base)
{
	if (g_ctx->immediate.enabled)
	{
		g_ctx->errno = GL_INVALID_OPERATION;
		return;
	}
}

void glNewList(GLuint list, GLenum mode)
{
	if (g_ctx->immediate.enabled)
	{
		g_ctx->errno = GL_INVALID_OPERATION;
		return;
	}
}

void glEndList(void)
{
	if (g_ctx->immediate.enabled)
	{
		g_ctx->errno = GL_INVALID_OPERATION;
		return;
	}
}

void glCallList(GLuint list)
{
	if (g_ctx->immediate.enabled)
	{
		g_ctx->errno = GL_INVALID_OPERATION;
		return;
	}
}

void glCallLists(GLsizei n, GLenum type, const GLvoid *lists)
{
	if (g_ctx->immediate.enabled)
	{
		g_ctx->errno = GL_INVALID_OPERATION;
		return;
	}
}
