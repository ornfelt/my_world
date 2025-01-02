#include "internal.h"

#include <limits.h>

void glLightf(GLenum light, GLenum pname, GLfloat param)
{
	switch (pname)
	{
		case GL_AMBIENT:
		case GL_DIFFUSE:
		case GL_SPECULAR:
		case GL_POSITION:
		case GL_SPOT_DIRECTION:
			g_ctx->errno = GL_INVALID_ENUM;
			return;
	}
	glLightfv(light, pname, &param);
}

void glLighti(GLenum light, GLenum pname, GLint param)
{
	switch (pname)
	{
		case GL_AMBIENT:
		case GL_DIFFUSE:
		case GL_SPECULAR:
		case GL_POSITION:
		case GL_SPOT_DIRECTION:
			g_ctx->errno = GL_INVALID_ENUM;
			return;
	}
	glLightiv(light, pname, &param);
}

void glLightfv(GLenum light, GLenum pname, GLfloat *params)
{
	if (light < GL_LIGHT0 || light >= GL_LIGHT0 + g_ctx->max_lights)
	{
		g_ctx->errno = GL_INVALID_ENUM;
		return;
	}
	struct light *l = &g_ctx->lights[light - GL_LIGHT0];
	switch (pname)
	{
		case GL_AMBIENT:
			l->ambient[0] = params[0];
			l->ambient[1] = params[1];
			l->ambient[2] = params[2];
			l->ambient[3] = params[3];
			break;
		case GL_DIFFUSE:
			l->diffuse[0] = params[0];
			l->diffuse[1] = params[1];
			l->diffuse[2] = params[2];
			l->diffuse[3] = params[3];
			break;
		case GL_SPECULAR:
			l->specular[0] = params[0];
			l->specular[1] = params[1];
			l->specular[2] = params[2];
			l->specular[3] = params[3];
			break;
		case GL_POSITION:
		{
			GLfloat tmp[4];
			tmp[0] = params[0];
			tmp[1] = params[1];
			tmp[2] = params[2];
			tmp[3] = params[3];
			mat4_transform_vec4(&g_ctx->modelview_matrix[g_ctx->modelview_stack_depth], tmp);
			l->position[0] = tmp[0];
			l->position[1] = tmp[1];
			l->position[2] = tmp[2];
			l->position[3] = tmp[3];
			break;
		}
		case GL_SPOT_DIRECTION:
			l->spot_direction[0] = params[0];
			l->spot_direction[1] = params[1];
			l->spot_direction[2] = params[2];
			break;
		case GL_SPOT_EXPONENT:
			l->spot_exponent = *params;
			break;
		case GL_SPOT_CUTOFF:
			l->spot_cutoff = *params;
			break;
		case GL_CONSTANT_ATTENUATION:
			l->attenuations[0] = *params;
			break;
		case GL_LINEAR_ATTENUATION:
			l->attenuations[1] = *params;
			break;
		case GL_QUADRATIC_ATTENUATION:
			l->attenuations[2] = *params;
			break;
		default:
			g_ctx->errno = GL_INVALID_ENUM;
			return;
	}
}

void glLightiv(GLenum light, GLenum pname, GLint *params)
{
	if (light < GL_LIGHT0 || light >= GL_LIGHT0 + g_ctx->max_lights)
	{
		g_ctx->errno = GL_INVALID_ENUM;
		return;
	}
	struct light *l = &g_ctx->lights[light - GL_LIGHT0];
	switch (pname)
	{
		case GL_AMBIENT:
			l->ambient[0] = params[0] / (GLfloat)INT_MAX;
			l->ambient[1] = params[1] / (GLfloat)INT_MAX;
			l->ambient[2] = params[2] / (GLfloat)INT_MAX;
			l->ambient[3] = params[3] / (GLfloat)INT_MAX;
			break;
		case GL_DIFFUSE:
			l->diffuse[0] = params[0] / (GLfloat)INT_MAX;
			l->diffuse[1] = params[1] / (GLfloat)INT_MAX;
			l->diffuse[2] = params[2] / (GLfloat)INT_MAX;
			l->diffuse[3] = params[3] / (GLfloat)INT_MAX;
			break;
		case GL_SPECULAR:
			l->specular[0] = params[0] / (GLfloat)INT_MAX;
			l->specular[1] = params[1] / (GLfloat)INT_MAX;
			l->specular[2] = params[2] / (GLfloat)INT_MAX;
			l->specular[3] = params[3] / (GLfloat)INT_MAX;
			break;
		case GL_POSITION:
		{
			GLfloat tmp[4];
			tmp[0] = params[0];
			tmp[1] = params[1];
			tmp[2] = params[2];
			tmp[3] = params[3];
			mat4_transform_vec4(&g_ctx->modelview_matrix[g_ctx->modelview_stack_depth], tmp);
			l->position[0] = tmp[0];
			l->position[1] = tmp[1];
			l->position[2] = tmp[2];
			l->position[3] = tmp[3];
			break;
		}
		case GL_SPOT_DIRECTION:
			l->spot_direction[0] = params[0];
			l->spot_direction[1] = params[1];
			l->spot_direction[2] = params[2];
			break;
		case GL_SPOT_EXPONENT:
			l->spot_exponent = *params;
			break;
		case GL_SPOT_CUTOFF:
			l->spot_cutoff = *params;
			break;
		case GL_CONSTANT_ATTENUATION:
			l->attenuations[0] = *params;
			break;
		case GL_LINEAR_ATTENUATION:
			l->attenuations[1] = *params;
			break;
		case GL_QUADRATIC_ATTENUATION:
			l->attenuations[2] = *params;
			break;
		default:
			g_ctx->errno = GL_INVALID_ENUM;
			return;
	}
}

void glGetLightfv(GLenum light, GLenum pname, GLfloat *params)
{
	if (light < GL_LIGHT0 || light >= GL_LIGHT0 + g_ctx->max_lights)
	{
		g_ctx->errno = GL_INVALID_ENUM;
		return;
	}
	struct light *l = &g_ctx->lights[light - GL_LIGHT0];
	switch (pname)
	{
		case GL_AMBIENT:
			params[0] = l->ambient[0];
			params[1] = l->ambient[1];
			params[2] = l->ambient[2];
			params[3] = l->ambient[3];
			break;
		case GL_DIFFUSE:
			params[0] = l->diffuse[0];
			params[1] = l->diffuse[1];
			params[2] = l->diffuse[2];
			params[3] = l->diffuse[3];
			break;
		case GL_SPECULAR:
			params[0] = l->specular[0];
			params[1] = l->specular[1];
			params[2] = l->specular[2];
			params[3] = l->specular[3];
			break;
		case GL_POSITION:
			params[0] = l->position[0];
			params[1] = l->position[1];
			params[2] = l->position[2];
			params[3] = l->position[3];
			break;
		case GL_SPOT_DIRECTION:
			params[0] = l->spot_direction[0];
			params[1] = l->spot_direction[1];
			params[2] = l->spot_direction[2];
			break;
		case GL_SPOT_EXPONENT:
			params[0] = l->spot_exponent;
			break;
		case GL_SPOT_CUTOFF:
			params[0] = l->spot_cutoff;
			break;
		case GL_CONSTANT_ATTENUATION:
			params[0] = l->attenuations[0];
			break;
		case GL_LINEAR_ATTENUATION:
			params[0] = l->attenuations[1];
			break;
		case GL_QUADRATIC_ATTENUATION:
			params[0] = l->attenuations[2];
			break;
		default:
			g_ctx->errno = GL_INVALID_ENUM;
			return;
	}
}

void glGetLightiv(GLenum light, GLenum pname, GLint *params)
{
	if (light < GL_LIGHT0 || light >= GL_LIGHT0 + g_ctx->max_lights)
	{
		g_ctx->errno = GL_INVALID_ENUM;
		return;
	}
	struct light *l = &g_ctx->lights[light - GL_LIGHT0];
	switch (pname)
	{
		case GL_AMBIENT:
			params[0] = l->ambient[0] * INT_MAX;
			params[1] = l->ambient[1] * INT_MAX;
			params[2] = l->ambient[2] * INT_MAX;
			params[3] = l->ambient[3] * INT_MAX;
			break;
		case GL_DIFFUSE:
			params[0] = l->diffuse[0] * INT_MAX;
			params[1] = l->diffuse[1] * INT_MAX;
			params[2] = l->diffuse[2] * INT_MAX;
			params[3] = l->diffuse[3] * INT_MAX;
			break;
		case GL_SPECULAR:
			params[0] = l->specular[0] * INT_MAX;
			params[1] = l->specular[1] * INT_MAX;
			params[2] = l->specular[2] * INT_MAX;
			params[3] = l->specular[3] * INT_MAX;
			break;
		case GL_POSITION:
			params[0] = l->position[0];
			params[1] = l->position[1];
			params[2] = l->position[2];
			params[3] = l->position[3];
			break;
		case GL_SPOT_DIRECTION:
			params[0] = l->spot_direction[0];
			params[1] = l->spot_direction[1];
			params[2] = l->spot_direction[2];
			break;
		case GL_SPOT_EXPONENT:
			params[0] = l->spot_exponent;
			break;
		case GL_SPOT_CUTOFF:
			params[0] = l->spot_cutoff;
			break;
		case GL_CONSTANT_ATTENUATION:
			params[0] = l->attenuations[0];
			break;
		case GL_LINEAR_ATTENUATION:
			params[0] = l->attenuations[1];
			break;
		case GL_QUADRATIC_ATTENUATION:
			params[0] = l->attenuations[2];
			break;
		default:
			g_ctx->errno = GL_INVALID_ENUM;
			return;
	}
}
