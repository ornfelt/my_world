#include "internal.h"

#include <stdlib.h>

void glDeleteTextures(GLsizei n, const GLuint *textures)
{
	if (n < 0)
	{
		g_ctx->errno = GL_INVALID_VALUE;
		return;
	}
	for (GLsizei i = 0; i < n; ++i)
	{
		if (!textures[i] || textures[i] >= g_ctx->textures_capacity)
			continue;
		if (textures[i] == g_ctx->texture_binding_1d)
			g_ctx->texture_binding_1d = 0;
		if (textures[i] == g_ctx->texture_binding_2d)
			g_ctx->texture_binding_2d = 0;
		if (textures[i] == g_ctx->texture_binding_3d)
			g_ctx->texture_binding_3d = 0;
		struct texture *tex = g_ctx->textures[i];
		if (tex->data)
			free(tex->data);
		free(tex);
		g_ctx->textures[i] = NULL;
	}
}
