#include "internal.h"

#include <limits.h>

static void
lightfv(struct gl_ctx *ctx,
        GLenum id,
        GLenum pname,
        GLfloat *params)
{
	struct light *light;

	if (id < GL_LIGHT0
	 || id >= GL_LIGHT0 + ctx->fixed.max_lights)
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	light = &ctx->fixed.block.lights[id - GL_LIGHT0];
	switch (pname)
	{
		case GL_AMBIENT:
			light->ambient[0] = params[0];
			light->ambient[1] = params[1];
			light->ambient[2] = params[2];
			light->ambient[3] = params[3];
			break;
		case GL_DIFFUSE:
			light->diffuse[0] = params[0];
			light->diffuse[1] = params[1];
			light->diffuse[2] = params[2];
			light->diffuse[3] = params[3];
			break;
		case GL_SPECULAR:
			light->specular[0] = params[0];
			light->specular[1] = params[1];
			light->specular[2] = params[2];
			light->specular[3] = params[3];
			break;
		case GL_POSITION:
		{
			GLfloat tmp[4];
			tmp[0] = params[0];
			tmp[1] = params[1];
			tmp[2] = params[2];
			tmp[3] = params[3];
			mat4_transform_vec4(ctx->fixed.modelview_matrix[ctx->fixed.modelview_stack_depth], tmp);
			light->position[0] = tmp[0];
			light->position[1] = tmp[1];
			light->position[2] = tmp[2];
			light->position[3] = tmp[3];
			break;
		}
		case GL_SPOT_DIRECTION:
			light->spot_direction[0] = params[0];
			light->spot_direction[1] = params[1];
			light->spot_direction[2] = params[2];
			break;
		case GL_SPOT_EXPONENT:
			light->spot_exponent = *params;
			break;
		case GL_SPOT_CUTOFF:
			light->spot_cutoff = *params;
			break;
		case GL_CONSTANT_ATTENUATION:
			light->attenuations[0] = *params;
			break;
		case GL_LINEAR_ATTENUATION:
			light->attenuations[1] = *params;
			break;
		case GL_QUADRATIC_ATTENUATION:
			light->attenuations[2] = *params;
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
}

static void
lightiv(struct gl_ctx *ctx,
        GLenum id,
        GLenum pname,
        GLint *params)
{
	struct light *light;

	if (id < GL_LIGHT0
	 || id >= GL_LIGHT0 + ctx->fixed.max_lights)
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	light = &ctx->fixed.block.lights[id - GL_LIGHT0];
	switch (pname)
	{
		case GL_AMBIENT:
			light->ambient[0] = params[0] * (1.0 / INT32_MAX);
			light->ambient[1] = params[1] * (1.0 / INT32_MAX);
			light->ambient[2] = params[2] * (1.0 / INT32_MAX);
			light->ambient[3] = params[3] * (1.0 / INT32_MAX);
			break;
		case GL_DIFFUSE:
			light->diffuse[0] = params[0] * (1.0 / INT32_MAX);
			light->diffuse[1] = params[1] * (1.0 / INT32_MAX);
			light->diffuse[2] = params[2] * (1.0 / INT32_MAX);
			light->diffuse[3] = params[3] * (1.0 / INT32_MAX);
			break;
		case GL_SPECULAR:
			light->specular[0] = params[0] * (1.0 / INT32_MAX);
			light->specular[1] = params[1] * (1.0 / INT32_MAX);
			light->specular[2] = params[2] * (1.0 / INT32_MAX);
			light->specular[3] = params[3] * (1.0 / INT32_MAX);
			break;
		case GL_POSITION:
		{
			GLfloat tmp[4];
			tmp[0] = params[0];
			tmp[1] = params[1];
			tmp[2] = params[2];
			tmp[3] = params[3];
			mat4_transform_vec4(ctx->fixed.modelview_matrix[ctx->fixed.modelview_stack_depth], tmp);
			light->position[0] = tmp[0];
			light->position[1] = tmp[1];
			light->position[2] = tmp[2];
			light->position[3] = tmp[3];
			break;
		}
		case GL_SPOT_DIRECTION:
			light->spot_direction[0] = params[0];
			light->spot_direction[1] = params[1];
			light->spot_direction[2] = params[2];
			break;
		case GL_SPOT_EXPONENT:
			light->spot_exponent = *params;
			break;
		case GL_SPOT_CUTOFF:
			light->spot_cutoff = *params;
			break;
		case GL_CONSTANT_ATTENUATION:
			light->attenuations[0] = *params;
			break;
		case GL_LINEAR_ATTENUATION:
			light->attenuations[1] = *params;
			break;
		case GL_QUADRATIC_ATTENUATION:
			light->attenuations[2] = *params;
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
}

void
glLightf(GLenum light, GLenum pname, GLfloat param)
{
	struct gl_ctx *ctx = g_ctx;

	switch (pname)
	{
		case GL_AMBIENT:
		case GL_DIFFUSE:
		case GL_SPECULAR:
		case GL_POSITION:
		case GL_SPOT_DIRECTION:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
	lightfv(ctx, light, pname, &param);
}

void
glLighti(GLenum light, GLenum pname, GLint param)
{
	struct gl_ctx *ctx = g_ctx;

	switch (pname)
	{
		case GL_AMBIENT:
		case GL_DIFFUSE:
		case GL_SPECULAR:
		case GL_POSITION:
		case GL_SPOT_DIRECTION:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
	lightiv(ctx, light, pname, &param);
}

void
glLightfv(GLenum id, GLenum pname, GLfloat *params)
{
	struct gl_ctx *ctx = g_ctx;

	lightfv(ctx, id, pname, params);
}

void
glLightiv(GLenum id, GLenum pname, GLint *params)
{
	struct gl_ctx *ctx = g_ctx;

	lightiv(ctx, id, pname, params);
}

void
glGetLightfv(GLenum id, GLenum pname, GLfloat *params)
{
	struct gl_ctx *ctx = g_ctx;
	struct light *light;

	if (id < GL_LIGHT0
	 || id >= GL_LIGHT0 + ctx->fixed.max_lights)
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	light = &ctx->fixed.block.lights[id - GL_LIGHT0];
	switch (pname)
	{
		case GL_AMBIENT:
			params[0] = light->ambient[0];
			params[1] = light->ambient[1];
			params[2] = light->ambient[2];
			params[3] = light->ambient[3];
			break;
		case GL_DIFFUSE:
			params[0] = light->diffuse[0];
			params[1] = light->diffuse[1];
			params[2] = light->diffuse[2];
			params[3] = light->diffuse[3];
			break;
		case GL_SPECULAR:
			params[0] = light->specular[0];
			params[1] = light->specular[1];
			params[2] = light->specular[2];
			params[3] = light->specular[3];
			break;
		case GL_POSITION:
			params[0] = light->position[0];
			params[1] = light->position[1];
			params[2] = light->position[2];
			params[3] = light->position[3];
			break;
		case GL_SPOT_DIRECTION:
			params[0] = light->spot_direction[0];
			params[1] = light->spot_direction[1];
			params[2] = light->spot_direction[2];
			break;
		case GL_SPOT_EXPONENT:
			params[0] = light->spot_exponent;
			break;
		case GL_SPOT_CUTOFF:
			params[0] = light->spot_cutoff;
			break;
		case GL_CONSTANT_ATTENUATION:
			params[0] = light->attenuations[0];
			break;
		case GL_LINEAR_ATTENUATION:
			params[0] = light->attenuations[1];
			break;
		case GL_QUADRATIC_ATTENUATION:
			params[0] = light->attenuations[2];
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
}

void
glGetLightiv(GLenum id, GLenum pname, GLint *params)
{
	struct gl_ctx *ctx = g_ctx;
	struct light *light;

	if (id < GL_LIGHT0
	 || id >= GL_LIGHT0 + ctx->fixed.max_lights)
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	light = &ctx->fixed.block.lights[id - GL_LIGHT0];
	switch (pname)
	{
		case GL_AMBIENT:
			params[0] = light->ambient[0] * INT32_MAX;
			params[1] = light->ambient[1] * INT32_MAX;
			params[2] = light->ambient[2] * INT32_MAX;
			params[3] = light->ambient[3] * INT32_MAX;
			break;
		case GL_DIFFUSE:
			params[0] = light->diffuse[0] * INT32_MAX;
			params[1] = light->diffuse[1] * INT32_MAX;
			params[2] = light->diffuse[2] * INT32_MAX;
			params[3] = light->diffuse[3] * INT32_MAX;
			break;
		case GL_SPECULAR:
			params[0] = light->specular[0] * INT32_MAX;
			params[1] = light->specular[1] * INT32_MAX;
			params[2] = light->specular[2] * INT32_MAX;
			params[3] = light->specular[3] * INT32_MAX;
			break;
		case GL_POSITION:
			params[0] = light->position[0];
			params[1] = light->position[1];
			params[2] = light->position[2];
			params[3] = light->position[3];
			break;
		case GL_SPOT_DIRECTION:
			params[0] = light->spot_direction[0];
			params[1] = light->spot_direction[1];
			params[2] = light->spot_direction[2];
			break;
		case GL_SPOT_EXPONENT:
			params[0] = light->spot_exponent;
			break;
		case GL_SPOT_CUTOFF:
			params[0] = light->spot_cutoff;
			break;
		case GL_CONSTANT_ATTENUATION:
			params[0] = light->attenuations[0];
			break;
		case GL_LINEAR_ATTENUATION:
			params[0] = light->attenuations[1];
			break;
		case GL_QUADRATIC_ATTENUATION:
			params[0] = light->attenuations[2];
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
}
