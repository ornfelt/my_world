#include "internal.h"

#include <stdlib.h>
#include <assert.h>

struct shader *
shader_alloc(struct gl_ctx *ctx, GLenum type)
{
	struct shader *shader;

	(void)ctx;
	shader = calloc(1, sizeof(*shader));
	if (!shader)
		return NULL;
	shader->object.type = OBJECT_SHADER;
	shader->type = type;
	shader->shader = NULL;
	return shader;
}

GLuint
glCreateShader(GLenum type)
{
	struct gl_ctx *ctx = g_ctx;
	struct shader *shader;

	switch (type)
	{
		case GL_VERTEX_SHADER:
		case GL_FRAGMENT_SHADER:
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return 0;
	}
	shader = shader_alloc(ctx, type);
	if (!shader)
		return 0;
	if (!object_alloc(ctx, &shader->object))
	{
		shader_free(ctx, shader);
		return 0;
	}
	return shader->object.id;
}

void
shader_free(struct gl_ctx *ctx, struct shader *shader)
{
	if (shader->shader)
		ctx->jkg_op->shader_free(ctx->jkg_ctx, shader->shader);
	object_free(ctx, &shader->object);
}

void
glDeleteShader(GLuint id)
{
	struct gl_ctx *ctx = g_ctx;
	struct shader *shader;

	if (!id)
		return;
	shader = object_get(ctx, id, OBJECT_SHADER);
	if (!shader)
		return;
	/* XXX refcount */
	shader_free(ctx, shader);
}

void
glShaderSource(GLuint id,
               GLsizei count,
               const GLchar **string,
               const GLuint *length)
{
	struct gl_ctx *ctx = g_ctx;
	struct shader *shader;

	shader = object_get(ctx, id, OBJECT_SHADER);
	if (!shader)
		return;
	/* XXX more real */
	if (count != 1)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	if (*length != sizeof(void*))
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	shader->data = (void*)*string;
	shader->size = *length;
}

bool
shader_compile(struct gl_ctx *ctx,
               struct shader *shader,
               const struct jkg_shader_create_info *create_info)
{
	struct jkg_shader *jkg_shader;
	int ret;

	ret = ctx->jkg_op->shader_alloc(ctx->jkg_ctx, create_info, &jkg_shader);
	if (ret)
	{
		GL_SET_ERR(ctx, get_jkg_error(ret));
		return false;
	}
	if (shader->shader)
		ctx->jkg_op->shader_free(ctx->jkg_ctx, shader->shader);
	shader->shader = jkg_shader;
	return true;
}

void
glCompileShader(GLuint id)
{
	struct jkg_shader_create_info create_info;
	struct gl_ctx *ctx = g_ctx;
	struct shader *shader;

	shader = object_get(ctx, id, OBJECT_SHADER);
	if (!shader)
		return;
	switch (shader->type)
	{
		case GL_VERTEX_SHADER:
			create_info.type = JKG_SHADER_VERTEX;
			break;
		case GL_FRAGMENT_SHADER:
			create_info.type = JKG_SHADER_FRAGMENT;
			break;
		default:
			assert(!"unknown shader");
	}
	create_info.data = shader->data;
	create_info.size = shader->size;
	/* XXX outputs */
	shader_compile(ctx, shader, &create_info);
}

void
glGetShaderiv(GLuint id, GLenum pname, GLint *params)
{
	struct gl_ctx *ctx = g_ctx;
	struct shader *shader;

	if (!id)
		return;
	shader = object_get(ctx, id, OBJECT_SHADER);
	if (!shader)
		return;
	switch (pname)
	{
		case GL_COMPILE_STATUS:
			*params = GL_TRUE; /* XXX */
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
}

GLboolean
glIsShader(GLuint id)
{
	return object_is(g_ctx, id, OBJECT_SHADER);
}
