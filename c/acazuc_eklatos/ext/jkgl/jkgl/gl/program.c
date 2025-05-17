#include "internal.h"

#include <stdlib.h>
#include <assert.h>

struct program *
program_alloc(struct gl_ctx *ctx)
{
	struct program *program;

	(void)ctx;
	program = calloc(1, sizeof(*program));
	if (!program)
		return NULL;
	program->object.type = OBJECT_PROGRAM;
	program->vs = NULL;
	program->fs = NULL;
	program->shader_state = NULL;
	return program;
}

GLuint
glCreateProgram(void)
{
	struct gl_ctx *ctx = g_ctx;
	struct program *program;

	program = program_alloc(ctx);
	if (!program)
		return 0;
	if (!object_alloc(ctx, &program->object))
	{
		program_free(ctx, program);
		return 0;
	}
	return program->object.id;
}

void
program_free(struct gl_ctx *ctx,
             struct program *program)
{
	if (ctx->program == program)
		program_bind(ctx, NULL);
	if (program->shader_state)
		ctx->jkg_op->shader_state_free(ctx->jkg_ctx, program->shader_state);
	object_free(ctx, &program->object);
}

void
glDeleteProgram(GLuint id)
{
	struct gl_ctx *ctx = g_ctx;
	struct program *program;

	program = object_get(ctx, id, OBJECT_PROGRAM);
	if (!program)
		return;
	program_free(ctx, program);
}

void
glAttachShader(GLuint program_id, GLuint shader_id)
{
	struct gl_ctx *ctx = g_ctx;
	struct program *program;
	struct shader *shader;

	program = object_get(ctx, program_id, OBJECT_PROGRAM);
	if (!program)
		return;
	shader = object_get(ctx, shader_id, OBJECT_SHADER);
	if (!shader)
		return;
	switch (shader->type)
	{
		case GL_VERTEX_SHADER:
			program->vs = shader;
			break;
		case GL_FRAGMENT_SHADER:
			program->fs = shader;
			break;
		default:
			assert(!"unknown shader type");
	}
}

bool
program_link(struct gl_ctx *ctx,
             struct program *program,
             struct shader *vs,
             struct shader *fs)
{
	struct jkg_shader_state_create_info create_info;
	struct jkg_shader_state *shader_state;
	int ret;

	create_info.vs = vs->shader;
	create_info.fs = fs->shader;
	ret = ctx->jkg_op->shader_state_alloc(ctx->jkg_ctx,
	                                      &create_info,
	                                      &shader_state);
	if (ret)
	{
		GL_SET_ERR(ctx, get_jkg_error(ret));
		return false;
	}
	if (program->shader_state)
		ctx->jkg_op->shader_state_free(ctx->jkg_ctx, program->shader_state);
	program->shader_state = shader_state;
	return true;
}

void
glLinkProgram(GLuint id)
{
	struct gl_ctx *ctx = g_ctx;
	struct program *program;

	program = object_get(ctx, id, OBJECT_PROGRAM);
	if (!program)
		return;
	if (!program->vs
	 || !program->fs)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	program_link(ctx, program, program->vs, program->fs);
}

void
program_bind(struct gl_ctx *ctx,
             struct program *program)
{
	if (ctx->program != program)
	{
		ctx->program = program;
		ctx->dirty |= GL_CTX_DIRTY_PROGRAM;
	}
}

void
glUseProgram(GLuint id)
{
	struct gl_ctx *ctx = g_ctx;
	struct program *program;

	if (id)
	{
		program = object_get(ctx, id, OBJECT_PROGRAM);
		if (!program)
			return;
		if (!program->shader_state)
		{
			GL_SET_ERR(ctx, GL_INVALID_OPERATION);
			return;
		}
	}
	else
	{
		program = NULL;
	}
	program_bind(ctx, program);
}

void
glGetProgramiv(GLuint id, GLenum pname, GLint *params)
{
	struct gl_ctx *ctx = g_ctx;
	struct program *program;

	program = object_get(ctx, id, OBJECT_PROGRAM);
	if (!program)
		return;
	switch (pname)
	{
		case GL_LINK_STATUS:
			*params = GL_TRUE; /* XXX */
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
}

GLboolean
glIsProgram(GLuint id)
{
	return object_is(g_ctx, id, OBJECT_PROGRAM);
}
