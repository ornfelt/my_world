#include "internal.h"

#include <limits.h>

void glMaterialf(GLenum face, GLenum pname, GLfloat param)
{
	switch (face)
	{
		case GL_FRONT:
		case GL_BACK:
			break;
		case GL_FRONT_AND_BACK:
			glMaterialf(GL_FRONT, pname, param);
			glMaterialf(GL_BACK, pname, param);
			return;
		default:
			g_ctx->errno = GL_INVALID_ENUM;
			return;
	}
	struct material *m = &g_ctx->materials[face == GL_FRONT];
	switch (pname)
	{
		case GL_SHININESS:
			m->shininess = param;
			break;
		default:
			g_ctx->errno = GL_INVALID_ENUM;
			return;
	}
}

void glMateriali(GLenum face, GLenum pname, GLint param)
{
	switch (face)
	{
		case GL_FRONT:
		case GL_BACK:
			break;
		case GL_FRONT_AND_BACK:
			glMateriali(GL_FRONT, pname, param);
			glMateriali(GL_BACK, pname, param);
			return;
		default:
			g_ctx->errno = GL_INVALID_ENUM;
			return;
	}
	struct material *m = &g_ctx->materials[face == GL_FRONT];
	switch (pname)
	{
		case GL_SHININESS:
			m->shininess = param;
			break;
		default:
			g_ctx->errno = GL_INVALID_ENUM;
			return;
	}
}

void glMaterialfv(GLenum face, GLenum pname, GLfloat *params)
{
	switch (face)
	{
		case GL_FRONT:
		case GL_BACK:
			break;
		case GL_FRONT_AND_BACK:
			glMaterialfv(GL_FRONT, pname, params);
			glMaterialfv(GL_BACK, pname, params);
			return;
		default:
			g_ctx->errno = GL_INVALID_ENUM;
			return;
	}
	struct material *m = &g_ctx->materials[face == GL_FRONT];
	switch (pname)
	{
		case GL_AMBIENT:
			m->ambient[0] = params[0];
			m->ambient[1] = params[1];
			m->ambient[2] = params[2];
			m->ambient[3] = params[3];
			break;
		case GL_DIFFUSE:
			m->diffuse[0] = params[0];
			m->diffuse[1] = params[1];
			m->diffuse[2] = params[2];
			m->diffuse[3] = params[3];
			break;
		case GL_SPECULAR:
			m->specular[0] = params[0];
			m->specular[1] = params[1];
			m->specular[2] = params[2];
			m->specular[3] = params[3];
			break;
		case GL_EMISSION:
			m->emission[0] = params[0];
			m->emission[1] = params[1];
			m->emission[2] = params[2];
			m->emission[3] = params[3];
			break;
		case GL_SHININESS:
			m->shininess = params[0];
			break;
		case GL_AMBIENT_AND_DIFFUSE:
			m->ambient[0] = params[0];
			m->ambient[1] = params[1];
			m->ambient[2] = params[2];
			m->ambient[3] = params[3];
			m->diffuse[0] = params[0];
			m->diffuse[1] = params[1];
			m->diffuse[2] = params[2];
			m->diffuse[3] = params[3];
			break;
		default:
			g_ctx->errno = GL_INVALID_ENUM;
			return;
	}
}

void glMaterialiv(GLenum face, GLenum pname, GLint *params)
{
	switch (face)
	{
		case GL_FRONT:
		case GL_BACK:
			break;
		case GL_FRONT_AND_BACK:
			glMaterialiv(GL_FRONT, pname, params);
			glMaterialiv(GL_BACK, pname, params);
			return;
		default:
			g_ctx->errno = GL_INVALID_ENUM;
			return;
	}
	struct material *m = &g_ctx->materials[face == GL_FRONT];
	switch (pname)
	{
		case GL_AMBIENT:
			m->ambient[0] = params[0] / (GLfloat)INT_MAX;
			m->ambient[1] = params[1] / (GLfloat)INT_MAX;
			m->ambient[2] = params[2] / (GLfloat)INT_MAX;
			m->ambient[3] = params[3] / (GLfloat)INT_MAX;
			break;
		case GL_DIFFUSE:
			m->diffuse[0] = params[0] / (GLfloat)INT_MAX;
			m->diffuse[1] = params[1] / (GLfloat)INT_MAX;
			m->diffuse[2] = params[2] / (GLfloat)INT_MAX;
			m->diffuse[3] = params[3] / (GLfloat)INT_MAX;
			break;
		case GL_SPECULAR:
			m->specular[0] = params[0] / (GLfloat)INT_MAX;
			m->specular[1] = params[1] / (GLfloat)INT_MAX;
			m->specular[2] = params[2] / (GLfloat)INT_MAX;
			m->specular[3] = params[3] / (GLfloat)INT_MAX;
			break;
		case GL_EMISSION:
			m->emission[0] = params[0] / (GLfloat)INT_MAX;
			m->emission[1] = params[1] / (GLfloat)INT_MAX;
			m->emission[2] = params[2] / (GLfloat)INT_MAX;
			m->emission[3] = params[3] / (GLfloat)INT_MAX;
			break;
		case GL_SHININESS:
			m->shininess = params[0];
			break;
		case GL_AMBIENT_AND_DIFFUSE:
			m->ambient[0] = params[0] / (GLfloat)INT_MAX;
			m->ambient[1] = params[1] / (GLfloat)INT_MAX;
			m->ambient[2] = params[2] / (GLfloat)INT_MAX;
			m->ambient[3] = params[3] / (GLfloat)INT_MAX;
			m->diffuse[0] = params[0] / (GLfloat)INT_MAX;
			m->diffuse[1] = params[1] / (GLfloat)INT_MAX;
			m->diffuse[2] = params[2] / (GLfloat)INT_MAX;
			m->diffuse[3] = params[3] / (GLfloat)INT_MAX;
			break;
		default:
			g_ctx->errno = GL_INVALID_ENUM;
			return;
	}
}

void glGetMaterialfv(GLenum face, GLenum pname, GLfloat *params)
{
	switch (face)
	{
		case GL_FRONT:
		case GL_BACK:
			break;
		default:
			g_ctx->errno = GL_INVALID_ENUM;
			return;
	}
	struct material *m = &g_ctx->materials[face == GL_FRONT];
	switch (pname)
	{
		case GL_AMBIENT:
			params[0] = m->ambient[0];
			params[1] = m->ambient[1];
			params[2] = m->ambient[2];
			params[3] = m->ambient[3];
			break;
		case GL_DIFFUSE:
			params[0] = m->diffuse[0];
			params[1] = m->diffuse[1];
			params[2] = m->diffuse[2];
			params[3] = m->diffuse[3];
			break;
		case GL_SPECULAR:
			params[0] = m->specular[0];
			params[1] = m->specular[1];
			params[2] = m->specular[2];
			params[3] = m->specular[3];
			break;
		case GL_EMISSION:
			params[0] = m->emission[0];
			params[1] = m->emission[1];
			params[2] = m->emission[2];
			params[3] = m->emission[3];
			break;
		case GL_SHININESS:
			params[0] = m->shininess;
			break;
		default:
			g_ctx->errno = GL_INVALID_ENUM;
			return;
	}
}

void glGetMaterialiv(GLenum face, GLenum pname, GLint *params)
{
	switch (face)
	{
		case GL_FRONT:
		case GL_BACK:
			break;
		default:
			g_ctx->errno = GL_INVALID_ENUM;
			return;
	}
	struct material *m = &g_ctx->materials[face == GL_FRONT];
	switch (pname)
	{
		case GL_AMBIENT:
			params[0] = m->ambient[0] * INT_MAX;
			params[1] = m->ambient[1] * INT_MAX;
			params[2] = m->ambient[2] * INT_MAX;
			params[3] = m->ambient[3] * INT_MAX;
			break;
		case GL_DIFFUSE:
			params[0] = m->diffuse[0] * INT_MAX;
			params[1] = m->diffuse[1] * INT_MAX;
			params[2] = m->diffuse[2] * INT_MAX;
			params[3] = m->diffuse[3] * INT_MAX;
			break;
		case GL_SPECULAR:
			params[0] = m->specular[0] * INT_MAX;
			params[1] = m->specular[1] * INT_MAX;
			params[2] = m->specular[2] * INT_MAX;
			params[3] = m->specular[3] * INT_MAX;
			break;
		case GL_EMISSION:
			params[0] = m->emission[0] * INT_MAX;
			params[1] = m->emission[1] * INT_MAX;
			params[2] = m->emission[2] * INT_MAX;
			params[3] = m->emission[3] * INT_MAX;
			break;
		case GL_SHININESS:
			params[0] = m->shininess;
			break;
		default:
			g_ctx->errno = GL_INVALID_ENUM;
			return;
	}
}
