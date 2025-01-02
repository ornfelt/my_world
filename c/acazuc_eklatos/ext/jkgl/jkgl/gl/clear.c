#include "internal.h"

static void clear_colors(void)
{
	for (GLsizei i = 0; i < g_ctx->width * g_ctx->height; ++i)
	{
		g_ctx->color_buffer[i * 4 + 0] = g_ctx->clear_color[0];
		g_ctx->color_buffer[i * 4 + 1] = g_ctx->clear_color[1];
		g_ctx->color_buffer[i * 4 + 2] = g_ctx->clear_color[2];
		g_ctx->color_buffer[i * 4 + 3] = g_ctx->clear_color[3];
	}
}

static void clear_depth(void)
{
	for (GLsizei i = 0; i < g_ctx->width * g_ctx->height; ++i)
		g_ctx->depth_buffer[i] = g_ctx->clear_depth;
}

static void clear_stencil(void)
{
	for (GLsizei i = 0; i < g_ctx->width * g_ctx->height; ++i)
		g_ctx->stencil_buffer[i] = g_ctx->clear_stencil;
}

void glClear(GLbitmask mask)
{
	if (mask & ~(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT))
	{
		g_ctx->errno = GL_INVALID_VALUE;
		return;
	}
	if (mask & GL_COLOR_BUFFER_BIT)
		clear_colors();
	if (mask & GL_DEPTH_BUFFER_BIT)
		clear_depth();
	if (mask & GL_STENCIL_BUFFER_BIT)
		clear_stencil();
}

void glClearColor(GLclampf red, GLclampf green, GLclampf blue,
                  GLclampf alpha)
{
	g_ctx->clear_color[0] = clampf(red, 0, 1);
	g_ctx->clear_color[1] = clampf(green, 0, 1);
	g_ctx->clear_color[2] = clampf(blue, 0, 1);
	g_ctx->clear_color[3] = clampf(alpha, 0, 1);
}

void glClearDepth(GLclampf depth)
{
	g_ctx->clear_depth = clampf(depth, 0, 1);
}

void glClearStencil(GLubyte stencil)
{
	g_ctx->clear_stencil = stencil;
}
