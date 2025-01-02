#include "internal.h"

GLboolean glIsTexture(GLuint texture)
{
	if (texture <= 0 || texture >= g_ctx->textures_capacity)
		return GL_FALSE;
	if (!g_ctx->textures[texture])
		return GL_FALSE;
	return GL_TRUE;
}
