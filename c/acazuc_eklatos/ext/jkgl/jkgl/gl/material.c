#include "internal.h"

#include <limits.h>

static void
materialf(struct gl_ctx *ctx,
          GLenum face,
          GLenum pname,
          GLfloat param)
{
	struct material *material;

	switch (face)
	{
		case GL_FRONT:
		case GL_BACK:
			break;
		case GL_FRONT_AND_BACK:
			materialf(ctx, GL_FRONT, pname, param);
			materialf(ctx, GL_BACK, pname, param);
			return;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
	material = &ctx->fixed.block.materials[face == GL_FRONT];
	switch (pname)
	{
		case GL_SHININESS:
			material->shininess = param;
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
}

void
glMaterialf(GLenum face, GLenum pname, GLfloat param)
{
	materialf(g_ctx, face, pname, param);
}

static void
materiali(struct gl_ctx *ctx,
          GLenum face,
          GLenum pname,
          GLint param)
{
	struct material *material;

	switch (face)
	{
		case GL_FRONT:
		case GL_BACK:
			break;
		case GL_FRONT_AND_BACK:
			materiali(ctx, GL_FRONT, pname, param);
			materiali(ctx, GL_BACK, pname, param);
			return;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
	material = &ctx->fixed.block.materials[face == GL_FRONT];
	switch (pname)
	{
		case GL_SHININESS:
			material->shininess = param;
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
}

void
glMateriali(GLenum face, GLenum pname, GLint param)
{
	materiali(g_ctx, face, pname, param);
}

static void
materialfv(struct gl_ctx *ctx,
           GLenum face,
           GLenum pname,
           GLfloat *params)
{
	struct material *material;

	switch (face)
	{
		case GL_FRONT:
		case GL_BACK:
			break;
		case GL_FRONT_AND_BACK:
			materialfv(ctx, GL_FRONT, pname, params);
			materialfv(ctx, GL_BACK, pname, params);
			return;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
	material = &ctx->fixed.block.materials[face == GL_FRONT];
	switch (pname)
	{
		case GL_AMBIENT:
			material->ambient[0] = params[0];
			material->ambient[1] = params[1];
			material->ambient[2] = params[2];
			material->ambient[3] = params[3];
			break;
		case GL_DIFFUSE:
			material->diffuse[0] = params[0];
			material->diffuse[1] = params[1];
			material->diffuse[2] = params[2];
			material->diffuse[3] = params[3];
			break;
		case GL_SPECULAR:
			material->specular[0] = params[0];
			material->specular[1] = params[1];
			material->specular[2] = params[2];
			material->specular[3] = params[3];
			break;
		case GL_EMISSION:
			material->emission[0] = params[0];
			material->emission[1] = params[1];
			material->emission[2] = params[2];
			material->emission[3] = params[3];
			break;
		case GL_SHININESS:
			material->shininess = params[0];
			break;
		case GL_AMBIENT_AND_DIFFUSE:
			material->ambient[0] = params[0];
			material->ambient[1] = params[1];
			material->ambient[2] = params[2];
			material->ambient[3] = params[3];
			material->diffuse[0] = params[0];
			material->diffuse[1] = params[1];
			material->diffuse[2] = params[2];
			material->diffuse[3] = params[3];
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
}

void
glMaterialfv(GLenum face, GLenum pname, GLfloat *params)
{
	materialfv(g_ctx, face, pname, params);
}

static void
materialiv(struct gl_ctx *ctx,
           GLenum face,
           GLenum pname,
           GLint *params)
{
	struct material *material;

	switch (face)
	{
		case GL_FRONT:
		case GL_BACK:
			break;
		case GL_FRONT_AND_BACK:
			materialiv(ctx, GL_FRONT, pname, params);
			materialiv(ctx, GL_BACK, pname, params);
			return;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
	material = &ctx->fixed.block.materials[face == GL_FRONT];
	switch (pname)
	{
		case GL_AMBIENT:
			material->ambient[0] = params[0] * (1.0 / INT32_MAX);
			material->ambient[1] = params[1] * (1.0 / INT32_MAX);
			material->ambient[2] = params[2] * (1.0 / INT32_MAX);
			material->ambient[3] = params[3] * (1.0 / INT32_MAX);
			break;
		case GL_DIFFUSE:
			material->diffuse[0] = params[0] * (1.0 / INT32_MAX);
			material->diffuse[1] = params[1] * (1.0 / INT32_MAX);
			material->diffuse[2] = params[2] * (1.0 / INT32_MAX);
			material->diffuse[3] = params[3] * (1.0 / INT32_MAX);
			break;
		case GL_SPECULAR:
			material->specular[0] = params[0] * (1.0 / INT32_MAX);
			material->specular[1] = params[1] * (1.0 / INT32_MAX);
			material->specular[2] = params[2] * (1.0 / INT32_MAX);
			material->specular[3] = params[3] * (1.0 / INT32_MAX);
			break;
		case GL_EMISSION:
			material->emission[0] = params[0] * (1.0 / INT32_MAX);
			material->emission[1] = params[1] * (1.0 / INT32_MAX);
			material->emission[2] = params[2] * (1.0 / INT32_MAX);
			material->emission[3] = params[3] * (1.0 / INT32_MAX);
			break;
		case GL_SHININESS:
			material->shininess = params[0];
			break;
		case GL_AMBIENT_AND_DIFFUSE:
			material->ambient[0] = params[0] * (1.0 / INT32_MAX);
			material->ambient[1] = params[1] * (1.0 / INT32_MAX);
			material->ambient[2] = params[2] * (1.0 / INT32_MAX);
			material->ambient[3] = params[3] * (1.0 / INT32_MAX);
			material->diffuse[0] = params[0] * (1.0 / INT32_MAX);
			material->diffuse[1] = params[1] * (1.0 / INT32_MAX);
			material->diffuse[2] = params[2] * (1.0 / INT32_MAX);
			material->diffuse[3] = params[3] * (1.0 / INT32_MAX);
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
}

void
glMaterialiv(GLenum face, GLenum pname, GLint *params)
{
	materialiv(g_ctx, face, pname, params);
}

void
glGetMaterialfv(GLenum face, GLenum pname, GLfloat *params)
{
	struct gl_ctx *ctx = g_ctx;
	struct material *material;

	switch (face)
	{
		case GL_FRONT:
		case GL_BACK:
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
	material = &ctx->fixed.block.materials[face == GL_FRONT];
	switch (pname)
	{
		case GL_AMBIENT:
			params[0] = material->ambient[0];
			params[1] = material->ambient[1];
			params[2] = material->ambient[2];
			params[3] = material->ambient[3];
			break;
		case GL_DIFFUSE:
			params[0] = material->diffuse[0];
			params[1] = material->diffuse[1];
			params[2] = material->diffuse[2];
			params[3] = material->diffuse[3];
			break;
		case GL_SPECULAR:
			params[0] = material->specular[0];
			params[1] = material->specular[1];
			params[2] = material->specular[2];
			params[3] = material->specular[3];
			break;
		case GL_EMISSION:
			params[0] = material->emission[0];
			params[1] = material->emission[1];
			params[2] = material->emission[2];
			params[3] = material->emission[3];
			break;
		case GL_SHININESS:
			params[0] = material->shininess;
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
}

void
glGetMaterialiv(GLenum face, GLenum pname, GLint *params)
{
	struct gl_ctx *ctx = g_ctx;
	struct material *material;

	switch (face)
	{
		case GL_FRONT:
		case GL_BACK:
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
	material = &ctx->fixed.block.materials[face == GL_FRONT];
	switch (pname)
	{
		case GL_AMBIENT:
			params[0] = material->ambient[0] * INT32_MAX;
			params[1] = material->ambient[1] * INT32_MAX;
			params[2] = material->ambient[2] * INT32_MAX;
			params[3] = material->ambient[3] * INT32_MAX;
			break;
		case GL_DIFFUSE:
			params[0] = material->diffuse[0] * INT32_MAX;
			params[1] = material->diffuse[1] * INT32_MAX;
			params[2] = material->diffuse[2] * INT32_MAX;
			params[3] = material->diffuse[3] * INT32_MAX;
			break;
		case GL_SPECULAR:
			params[0] = material->specular[0] * INT32_MAX;
			params[1] = material->specular[1] * INT32_MAX;
			params[2] = material->specular[2] * INT32_MAX;
			params[3] = material->specular[3] * INT32_MAX;
			break;
		case GL_EMISSION:
			params[0] = material->emission[0] * INT32_MAX;
			params[1] = material->emission[1] * INT32_MAX;
			params[2] = material->emission[2] * INT32_MAX;
			params[3] = material->emission[3] * INT32_MAX;
			break;
		case GL_SHININESS:
			params[0] = material->shininess;
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
}
