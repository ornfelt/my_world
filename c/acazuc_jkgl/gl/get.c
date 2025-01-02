#include "internal.h"

#include <limits.h>
#include <string.h>

GLboolean glGet(GLenum pname, void **data, GLenum *format, GLuint *length)
{
	switch (pname)
	{
		case GL_MATRIX_MODE:
			*data = &g_ctx->matrix_mode;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_MODELVIEW_MATRIX:
			*data = g_ctx->modelview_matrix[g_ctx->modelview_stack_depth].v;
			*format = GL_FLOAT;
			*length = 16;
			break;
		case GL_PROJECTION_MATRIX:
			*data = g_ctx->projection_matrix[g_ctx->projection_stack_depth].v;
			*format = GL_FLOAT;
			*length = 16;
			break;
		case GL_DEPTH_RANGE:
			*data = g_ctx->depth_range;
			*format = GL_FLOAT;
			*length = 2;
			break;
		case GL_DEPTH_FUNC:
			*data = &g_ctx->depth_func;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_DEPTH_TEST:
			*data = &g_ctx->depth_test;
			*format = GL_UNSIGNED_BYTE;
			*length = 1;
			break;
		case GL_DEPTH_WRITEMASK:
			*data = &g_ctx->depth_write;
			*format = GL_UNSIGNED_BYTE;
			*length = 1;
			break;
		case GL_BLEND_SRC_RGB:
			*data = &g_ctx->blend_src_rgb;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_BLEND_SRC_ALPHA:
			*data = &g_ctx->blend_src_alpha;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_BLEND_DST_RGB:
			*data = &g_ctx->blend_dst_rgb;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_BLEND_DST_ALPHA:
			*data = &g_ctx->blend_dst_alpha;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_BLEND_COLOR:
			*data = g_ctx->blend_color;
			*format = GL_FLOAT;
			*length = 4;
			break;
		case GL_BLEND_EQUATION_RGB:
			*data = &g_ctx->blend_equation_rgb;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_BLEND_EQUATION_ALPHA:
			*data = &g_ctx->blend_equation_alpha;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_BLEND:
			*data = &g_ctx->blend;
			*format = GL_UNSIGNED_BYTE;
			*length = 1;
			break;
		case GL_FOG:
			*data = &g_ctx->fog;
			*format = GL_UNSIGNED_BYTE;
			*length = 1;
			break;
		case GL_FOG_COLOR:
			*data = g_ctx->fog_color;
			*format = GL_FLOAT;
			*length = 4;
			break;
		case GL_FOG_DENSITY:
			*data = &g_ctx->fog_density;
			*format = GL_FLOAT;
			*length = 1;
			break;
		case GL_FOG_END:
			*data = &g_ctx->fog_end;
			*format = GL_FLOAT;
			*length = 1;
			break;
		case GL_FOG_START:
			*data = &g_ctx->fog_start;
			*format = GL_FLOAT;
			*length = 1;
			break;
		case GL_FOG_INDEX:
			*data = &g_ctx->fog_index;
			*format = GL_FLOAT;
			*length = 1;
			break;
		case GL_FOG_MODE:
			*data = &g_ctx->fog_mode;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_MODELVIEW_STACK_DEPTH:
			*data = &g_ctx->modelview_stack_depth;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_PROJECTION_STACK_DEPTH:
			*data = &g_ctx->projection_stack_depth;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_MAX_MODELVIEW_STACK_DEPTH:
			*data = &g_ctx->modelview_max_stack_depth;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_MAX_PROJECTION_STACK_DEPTH:
			*data = &g_ctx->projection_max_stack_depth;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_TEXTURE_BINDING_1D:
			*data = &g_ctx->texture_binding_1d;
			*format = GL_UNSIGNED_INT;
			*length = 1;
			break;
		case GL_TEXTURE_BINDING_2D:
			*data = &g_ctx->texture_binding_2d;
			*format = GL_UNSIGNED_INT;
			*length = 1;
			break;
		case GL_TEXTURE_BINDING_3D:
			*data = &g_ctx->texture_binding_3d;
			*format = GL_UNSIGNED_INT;
			*length = 1;
			break;
		case GL_VERTEX_ARRAY_SIZE:
			*data = &g_ctx->vertex_array.size;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_VERTEX_ARRAY_TYPE:
			*data = &g_ctx->vertex_array.type;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_VERTEX_ARRAY_STRIDE:
			*data = &g_ctx->vertex_array.stride;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_COLOR_ARRAY_SIZE:
			*data = &g_ctx->color_array.size;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_COLOR_ARRAY_TYPE:
			*data = &g_ctx->color_array.type;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_COLOR_ARRAY_STRIDE:
			*data = &g_ctx->color_array.stride;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_TEXTURE_COORD_ARRAY_SIZE:
			*data = &g_ctx->texture_coord_array.size;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_TEXTURE_COORD_ARRAY_TYPE:
			*data = &g_ctx->texture_coord_array.type;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_TEXTURE_COORD_ARRAY_STRIDE:
			*data = &g_ctx->texture_coord_array.stride;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_COLOR_WRITEMASK:
			*data = &g_ctx->color_mask[0];
			*format = GL_UNSIGNED_BYTE;
			*length = 4;
			break;
		case GL_COLOR_CLEAR_VALUE:
			*data = &g_ctx->clear_color[0];
			*format = GL_FLOAT;
			*length = 4;
			break;
		case GL_DEPTH_CLEAR_VALUE:
			*data = &g_ctx->clear_depth;
			*format = GL_FLOAT;
			*length = 1;
			break;
		case GL_STENCIL_CLEAR_VALUE:
			*data = &g_ctx->clear_stencil;
			*format = GL_UNSIGNED_BYTE;
			*length = 1;
			break;
		case GL_POINT_SIZE:
			*data = &g_ctx->point_size;
			*format = GL_FLOAT;
			*length = 1;
			break;
		case GL_LINE_WIDTH:
			*data = &g_ctx->line_width;
			*format = GL_FLOAT;
			*length = 1;
			break;
		case GL_MAX_LIGHTS:
			*data = &g_ctx->max_lights;
			*format = GL_UNSIGNED_INT;
			*length = 1;
			break;
		case GL_NORMAL_ARRAY_TYPE:
			*data = &g_ctx->normal_array.type;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_NORMAL_ARRAY_STRIDE:
			*data = &g_ctx->normal_array.stride;
			*format = GL_INT;
			*length = 1;
			break;
		default:
			g_ctx->errno = GL_INVALID_ENUM;
			return GL_FALSE;
	}
	return GL_TRUE;
}

void glGetBooleanv(GLenum pname, GLboolean *params)
{
	void *data;
	GLenum format;
	GLuint length;

	if (!glGet(pname, &data, &format, &length))
		return;
	for (GLuint i = 0; i < length; ++i)
	{
		switch (format)
		{
			case GL_BYTE:
				params[i] = ((GLbyte*)data)[i] != 0;
				break;
			case GL_UNSIGNED_BYTE:
				params[i] = ((GLubyte*)data)[i] != 0;
				break;
			case GL_SHORT:
				params[i] = ((GLshort*)data)[i] != 0;
				break;
			case GL_UNSIGNED_SHORT:
				params[i] = ((GLushort*)data)[i] != 0;
				break;
			case GL_INT:
				params[i] = ((GLint*)data)[i] != 0;
				break;
			case GL_UNSIGNED_INT:
				params[i] = ((GLuint*)data)[i] != 0;
				break;
			case GL_FLOAT:
				params[i] = ((GLfloat*)data)[i] != 0;
				break;
			case GL_DOUBLE:
				params[i] = ((GLdouble*)data)[i] != 0;
				break;
		}
	}
}

void glGetDoublev(GLenum pname, GLdouble *params)
{
	void *data;
	GLenum format;
	GLuint length;

	if (!glGet(pname, &data, &format, &length))
		return;
	for (GLuint i = 0; i < length; ++i)
	{
		switch (format)
		{
			case GL_BYTE:
				params[i] = ((GLbyte*)data)[i];
				break;
			case GL_UNSIGNED_BYTE:
				params[i] = ((GLubyte*)data)[i];
				break;
			case GL_SHORT:
				params[i] = ((GLshort*)data)[i];
				break;
			case GL_UNSIGNED_SHORT:
				params[i] = ((GLushort*)data)[i];
				break;
			case GL_INT:
				params[i] = ((GLint*)data)[i];
				break;
			case GL_UNSIGNED_INT:
				params[i] = ((GLuint*)data)[i];
				break;
			case GL_FLOAT:
				params[i] = ((GLfloat*)data)[i];
				break;
			case GL_DOUBLE:
				params[i] = ((GLdouble*)data)[i];
				break;
		}
	}
}

void glGetFloatv(GLenum pname, GLfloat *params)
{
	void *data;
	GLenum format;
	GLuint length;

	if (!glGet(pname, &data, &format, &length))
		return;
	for (GLuint i = 0; i < length; ++i)
	{
		switch (format)
		{
			case GL_BYTE:
				params[i] = ((GLbyte*)data)[i];
				break;
			case GL_UNSIGNED_BYTE:
				params[i] = ((GLubyte*)data)[i];
				break;
			case GL_SHORT:
				params[i] = ((GLshort*)data)[i];
				break;
			case GL_UNSIGNED_SHORT:
				params[i] = ((GLushort*)data)[i];
				break;
			case GL_INT:
				params[i] = ((GLint*)data)[i];
				break;
			case GL_UNSIGNED_INT:
				params[i] = ((GLuint*)data)[i];
				break;
			case GL_FLOAT:
				params[i] = ((GLfloat*)data)[i];
				break;
			case GL_DOUBLE:
				params[i] = ((GLdouble*)data)[i];
				break;
		}
	}
}

void glGetIntegerv(GLenum pname, GLint *params)
{
	void *data;
	GLenum format;
	GLuint length;

	if (!glGet(pname, &data, &format, &length))
		return;
	for (GLuint i = 0; i < length; ++i)
	{
		switch (format)
		{
			case GL_BYTE:
				params[i] = ((GLbyte*)data)[i];
				break;
			case GL_UNSIGNED_BYTE:
				params[i] = ((GLubyte*)data)[i];
				break;
			case GL_SHORT:
				params[i] = ((GLshort*)data)[i];
				break;
			case GL_UNSIGNED_SHORT:
				params[i] = ((GLushort*)data)[i];
				break;
			case GL_INT:
				params[i] = ((GLint*)data)[i];
				break;
			case GL_UNSIGNED_INT:
				params[i] = ((GLuint*)data)[i];
				break;
			case GL_FLOAT:
				params[i] = ((GLfloat*)data)[i];
				break;
			case GL_DOUBLE:
				params[i] = ((GLdouble*)data)[i];
				break;
		}
	}
}

void glGetPointerv(GLenum pname, GLvoid **params)
{
	switch (pname)
	{
		case GL_COLOR_ARRAY_POINTER:
			*params = (GLvoid*)g_ctx->color_array.pointer;
			break;
		case GL_TEXTURE_COORD_ARRAY_POINTER:
			*params = (GLvoid*)g_ctx->texture_coord_array.pointer;
			break;
		case GL_VERTEX_ARRAY_POINTER:
			*params = (GLvoid*)g_ctx->vertex_array.pointer;
			break;
		case GL_NORMAL_ARRAY_POINTER:
			*params = (GLvoid*)g_ctx->normal_array.pointer;
			break;
		default:
			g_ctx->errno = GL_INVALID_ENUM;
			break;
	}
}
