#include "internal.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void alloc_new(void)
{
	struct texture **new;
	GLuint new_capacity;
	GLuint i;

	if (g_ctx->textures_capacity)
		new_capacity = g_ctx->textures_capacity * 2;
	else
		new_capacity = 16;
	new = malloc(sizeof(*new) * new_capacity);
	assert(new);
	for (i = 0; i < g_ctx->textures_capacity; ++i)
		new[i] = g_ctx->textures[i];
	for (; i < new_capacity; ++i)
		new[i] = NULL;
	free(g_ctx->textures);
	g_ctx->textures = new;
	g_ctx->textures_capacity = new_capacity;
}

static void get_tex(GLuint *tex, GLuint *index)
{
	struct texture *texture;
	while (*index < g_ctx->textures_capacity && g_ctx->textures[*index])
		++(*index);
	if (*index >= g_ctx->textures_capacity)
		alloc_new();
	texture = malloc(sizeof(**g_ctx->textures));
	assert(texture);
	memset(texture, 0, sizeof(*texture));
	texture->dirty = -1;
	texture->initialized = GL_FALSE;
	texture->min_filter = GL_NEAREST_MIPMAP_LINEAR;
	texture->mag_filter = GL_LINEAR;
	texture->wrap_s = GL_REPEAT;
	texture->wrap_t = GL_REPEAT;
	texture->wrap_r = GL_REPEAT;
	g_ctx->textures[*index] = texture;
	*tex = *index;
}

void glGenTextures(GLsizei n, GLuint *textures)
{
	GLuint index = 1;

	if (n < 0)
	{
		g_ctx->errno = GL_INVALID_VALUE;
		return;
	}
	for (GLsizei i = 0; i < n; ++i)
		get_tex(&textures[i], &index);
}
