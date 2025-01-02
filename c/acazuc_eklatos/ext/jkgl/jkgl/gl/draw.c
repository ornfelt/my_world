#include "internal.h"
#include "fixed.h"

#include <assert.h>
#include <stddef.h>
#include <limits.h>
#include <math.h>

static GLfloat get_pos_val(GLint off, GLint idx)
{
	const GLubyte *ptr = g_ctx->vertex_array.pointer;
	ptr = &ptr[off];
	switch (g_ctx->vertex_array.type)
	{
		case GL_SHORT:
			return ((GLshort*)ptr)[idx];
		case GL_INT:
			return ((GLint*)ptr)[idx];
		case GL_FLOAT:
			return ((GLfloat*)ptr)[idx];
		case GL_DOUBLE:
			return ((GLdouble*)ptr)[idx];
		default:
			assert(!"unknown vertex array type");
			return 0;
	}
}

static void get_pos(struct vert *vert, GLint pos)
{
	GLint off;

	off = pos * g_ctx->vertex_array.stride;
	switch (g_ctx->vertex_array.size)
	{
		case 2:
			vert->attr[VERT_ATTR_X] = get_pos_val(off, 0);
			vert->attr[VERT_ATTR_Y] = get_pos_val(off, 1);
			vert->attr[VERT_ATTR_Z] = 0;
			vert->attr[VERT_ATTR_W] = 1;
			break;
		case 3:
			vert->attr[VERT_ATTR_X] = get_pos_val(off, 0);
			vert->attr[VERT_ATTR_Y] = get_pos_val(off, 1);
			vert->attr[VERT_ATTR_Z] = get_pos_val(off, 2);
			vert->attr[VERT_ATTR_W] = 1;
			break;
		case 4:
			vert->attr[VERT_ATTR_X] = get_pos_val(off, 0);
			vert->attr[VERT_ATTR_Y] = get_pos_val(off, 1);
			vert->attr[VERT_ATTR_Z] = get_pos_val(off, 2);
			vert->attr[VERT_ATTR_W] = get_pos_val(off, 3);
			break;
		default:
			assert(!"unknown vertex array size");
			vert->attr[VERT_ATTR_X] = 0;
			vert->attr[VERT_ATTR_Y] = 0;
			vert->attr[VERT_ATTR_Z] = 0;
			vert->attr[VERT_ATTR_W] = 1;
			break;
	}
}

static GLfloat get_color_val(GLint off, GLint idx)
{
	const GLubyte *ptr = g_ctx->color_array.pointer;
	ptr = &ptr[off];
	switch (g_ctx->color_array.type)
	{
		case GL_BYTE:
			return ((GLbyte*)ptr)[idx] / (GLfloat)CHAR_MAX;
		case GL_UNSIGNED_BYTE:
			return ((GLubyte*)ptr)[idx] / (GLfloat)UCHAR_MAX;
		case GL_SHORT:
			return ((GLshort*)ptr)[idx] / (GLfloat)SHRT_MAX;
		case GL_UNSIGNED_SHORT:
			return ((GLushort*)ptr)[idx] / (GLfloat)USHRT_MAX;
		case GL_INT:
			return ((GLint*)ptr)[idx] / (GLfloat)INT_MAX;
		case GL_UNSIGNED_INT:
			return ((GLuint*)ptr)[idx] / (GLfloat)UINT_MAX;
		case GL_FLOAT:
			return ((GLfloat*)ptr)[idx];
		case GL_DOUBLE:
			return ((GLdouble*)ptr)[idx];
		default:
			assert(!"unknown vertex array type");
			return 0;
	}
}

static void get_color(struct vert *vert, GLint pos)
{
	GLint off;

	off = pos * g_ctx->color_array.stride;
	switch (g_ctx->color_array.size)
	{
		case 3:
			vert->attr[VERT_ATTR_R] = get_color_val(off, 0);
			vert->attr[VERT_ATTR_G] = get_color_val(off, 1);
			vert->attr[VERT_ATTR_B] = get_color_val(off, 2);
			vert->attr[VERT_ATTR_A] = 1;
			break;
		case 4:
			vert->attr[VERT_ATTR_R] = get_color_val(off, 0);
			vert->attr[VERT_ATTR_G] = get_color_val(off, 1);
			vert->attr[VERT_ATTR_B] = get_color_val(off, 2);
			vert->attr[VERT_ATTR_A] = get_color_val(off, 3);
			break;
		default:
			vert->attr[VERT_ATTR_R] = 0;
			vert->attr[VERT_ATTR_G] = 0;
			vert->attr[VERT_ATTR_B] = 0;
			vert->attr[VERT_ATTR_A] = 0;
			break;
	}
}

static GLfloat get_tex_coord_val(GLint off, GLint idx)
{
	const GLubyte *ptr = g_ctx->texture_coord_array.pointer;
	ptr = &ptr[off];
	switch (g_ctx->color_array.type)
	{
		case GL_SHORT:
			return ((GLshort*)ptr)[idx] / (GLfloat)SHRT_MAX;
		case GL_INT:
			return ((GLint*)ptr)[idx] / (GLfloat)INT_MAX;
		case GL_FLOAT:
			return ((GLfloat*)ptr)[idx];
		case GL_DOUBLE:
			return ((GLdouble*)ptr)[idx];
		default:
			assert(!"unknown vertex array type");
			return 0;
	}
}

static void get_tex_coord(struct vert *vec, GLint pos)
{
	GLint off;

	off = pos * g_ctx->texture_coord_array.stride;
	switch (g_ctx->texture_coord_array.size)
	{
		case 1:
			vec->attr[VERT_ATTR_S] = get_tex_coord_val(off, 0);
			vec->attr[VERT_ATTR_T] = 0;
			vec->attr[VERT_ATTR_P] = 0;
			vec->attr[VERT_ATTR_Q] = 0;
			break;
		case 2:
			vec->attr[VERT_ATTR_S] = get_tex_coord_val(off, 0);
			vec->attr[VERT_ATTR_T] = get_tex_coord_val(off, 1);
			vec->attr[VERT_ATTR_P] = 0;
			vec->attr[VERT_ATTR_Q] = 0;
			break;
		case 3:
			vec->attr[VERT_ATTR_S] = get_tex_coord_val(off, 0);
			vec->attr[VERT_ATTR_T] = get_tex_coord_val(off, 1);
			vec->attr[VERT_ATTR_P] = get_tex_coord_val(off, 2);
			vec->attr[VERT_ATTR_Q] = 0;
			break;
		case 4:
			vec->attr[VERT_ATTR_S] = get_tex_coord_val(off, 0);
			vec->attr[VERT_ATTR_T] = get_tex_coord_val(off, 1);
			vec->attr[VERT_ATTR_P] = get_tex_coord_val(off, 2);
			vec->attr[VERT_ATTR_Q] = get_tex_coord_val(off, 3);
			break;
		default:
			assert(!"unknown texture coord array size");
			vec->attr[VERT_ATTR_S] = 0;
			vec->attr[VERT_ATTR_T] = 0;
			vec->attr[VERT_ATTR_P] = 0;
			vec->attr[VERT_ATTR_Q] = 0;
			break;
	}
}

static void get_normal(struct vert *vert, GLint pos)
{
	GLint off;

	off = pos * g_ctx->vertex_array.stride;
	switch (g_ctx->vertex_array.size)
	{
		case 2:
			vert->attr[VERT_ATTR_NX] = get_pos_val(off, 0);
			vert->attr[VERT_ATTR_NY] = get_pos_val(off, 1);
			vert->attr[VERT_ATTR_NZ] = 0;
			break;
		case 3:
			vert->attr[VERT_ATTR_NX] = get_pos_val(off, 0);
			vert->attr[VERT_ATTR_NY] = get_pos_val(off, 1);
			vert->attr[VERT_ATTR_NZ] = get_pos_val(off, 2);
			break;
		case 4:
			vert->attr[VERT_ATTR_NX] = get_pos_val(off, 0);
			vert->attr[VERT_ATTR_NY] = get_pos_val(off, 1);
			vert->attr[VERT_ATTR_NZ] = get_pos_val(off, 2);
			break;
		default:
			assert(!"unknown normal array size");
			vert->attr[VERT_ATTR_NX] = 0;
			vert->attr[VERT_ATTR_NY] = 0;
			vert->attr[VERT_ATTR_NZ] = 0;
			break;
	}
}

static void get_vert(struct vert *vert, GLint pos)
{
	get_pos(vert, pos);
	if (g_ctx->color_array.enabled)
	{
		get_color(vert, pos);
	}
	else
	{
		vert->attr[VERT_ATTR_R] = 1;
		vert->attr[VERT_ATTR_G] = 1;
		vert->attr[VERT_ATTR_B] = 1;
		vert->attr[VERT_ATTR_A] = 1;
	}
	if (g_ctx->texture_coord_array.enabled)
	{
		get_tex_coord(vert, pos);
	}
	else
	{
		vert->attr[VERT_ATTR_S] = 0;
		vert->attr[VERT_ATTR_T] = 0;
		vert->attr[VERT_ATTR_P] = 0;
		vert->attr[VERT_ATTR_Q] = 0;
	}
	if (g_ctx->normal_array.enabled)
	{
		get_normal(vert, pos);
	}
	else
	{
		vert->attr[VERT_ATTR_NX] = 0;
		vert->attr[VERT_ATTR_NY] = 0;
		vert->attr[VERT_ATTR_NZ] = 0;
	}
	g_ctx->vs.fn(vert);
	rast_normalize_vert(vert);
}

static GLint get_indice(GLsizei n)
{
	if (!g_ctx->indices)
		return n;
	switch (g_ctx->indice_type)
	{
		case GL_UNSIGNED_BYTE:
			return ((const GLubyte*)g_ctx->indices)[n];
		case GL_UNSIGNED_SHORT:
			return ((const GLushort*)g_ctx->indices)[n];
		case GL_UNSIGNED_INT:
			return ((const GLuint*)g_ctx->indices)[n];
		default:
			assert(!"unknown indice type");
			return 0;
	}
}

static void points(GLsizei first, GLsizei count)
{
	struct vert vert;
	GLsizei n = first;

	for (GLsizei i = 0; i < count; i++)
	{
		get_vert(&vert, get_indice(n++));
		rast_point(&vert);
	}
}

static void line_strip(GLsizei first, GLsizei count)
{
	struct vert v1;
	struct vert v2;
	GLsizei n = first;

	get_vert(&v1, n++);
	for (GLsizei i = 1; i < count; i++)
	{
		get_vert(&v2, get_indice(i++));
		rast_line(&v1, &v2);
		v1 = v2;
	}
}

static void line_loop(GLsizei first, GLsizei count)
{
	struct vert v0;
	struct vert v1;
	struct vert v2;
	GLsizei n = first;

	if (count < 2)
		return;
	get_vert(&v1, get_indice(n++));
	v0 = v1;
	for (GLsizei i = 1; i < count; i++)
	{
		get_vert(&v2, get_indice(n++));
		rast_line(&v1, &v2);
		v1 = v2;
	}
	if (count > 2)
		rast_line(&v1, &v0);
}

static void lines(GLsizei first, GLsizei count)
{
	struct vert v1;
	struct vert v2;
	GLsizei n = first;

	count /= 2;
	for (GLsizei i = 0; i < count; i++)
	{
		get_vert(&v1, get_indice(n++));
		get_vert(&v2, get_indice(n++));
		rast_line(&v1, &v2);
	}
}

static void triangle_strip(GLsizei first, GLsizei count)
{
	struct vert v1;
	struct vert v2;
	struct vert v3;
	GLsizei n = first;

	if (count < 3)
		return;
	get_vert(&v1, get_indice(n++));
	get_vert(&v2, get_indice(n++));
	for (GLsizei i = 2; i < count; i++)
	{
		get_vert(&v3, get_indice(n++));
		rast_triangle(&v1, &v2, &v3);
		v1 = v2;
		v2 = v3;
	}
}

static void triangle_fan(GLsizei first, GLsizei count)
{
	struct vert v1;
	struct vert v2;
	struct vert v3;
	GLsizei n = first;

	if (count < 3)
		return;
	get_vert(&v1, get_indice(n++));
	get_vert(&v2, get_indice(n++));
	for (GLsizei i = 2; i < count; i++)
	{
		get_vert(&v3, get_indice(n++));
		rast_triangle(&v1, &v2, &v3);
		v2 = v3;
	}
}

static void triangles(GLsizei first, GLsizei count)
{
	struct vert v1;
	struct vert v2;
	struct vert v3;
	GLsizei n = first;

	count /= 3;
	for (GLsizei i = 0; i < count; i++)
	{
		get_vert(&v1, get_indice(n++));
		get_vert(&v2, get_indice(n++));
		get_vert(&v3, get_indice(n++));
		rast_triangle(&v1, &v2, &v3);
	}
}

static void quads(GLsizei first, GLsizei count)
{
	struct vert v1;
	struct vert v2;
	struct vert v3;
	struct vert v4;
	GLsizei n = first;

	count /= 4;
	for (GLsizei i = 0; i < count; i++)
	{
		get_vert(&v1, get_indice(n++));
		get_vert(&v2, get_indice(n++));
		get_vert(&v3, get_indice(n++));
		get_vert(&v4, get_indice(n++));
		rast_triangle(&v1, &v2, &v3);
		rast_triangle(&v3, &v4, &v1);
	}
}

static void quad_strip(GLsizei first, GLsizei count)
{
	struct vert v1;
	struct vert v2;
	struct vert v3;
	struct vert v4;
	GLsizei n = first;

	if (count < 4)
		return;
	get_vert(&v1, get_indice(n++));
	get_vert(&v2, get_indice(n++));
	count /= 2;
	for (GLsizei i = 1; i < count; i++)
	{
		get_vert(&v3, get_indice(n++));
		get_vert(&v4, get_indice(n++));
		rast_triangle(&v1, &v2, &v4);
		rast_triangle(&v4, &v3, &v1);
		v1 = v3;
		v2 = v4;
	}
}

static void primitive(GLenum mode, GLsizei first, GLsizei count)
{
	switch (mode)
	{
		case GL_POINTS:
			points(first, count);
			break;
		case GL_LINE_STRIP:
			line_strip(first, count);
			break;
		case GL_LINE_LOOP:
			line_loop(first, count);
			break;
		case GL_LINES:
			lines(first, count);
			break;
		case GL_TRIANGLE_STRIP:
			triangle_strip(first, count);
			break;
		case GL_TRIANGLE_FAN:
			triangle_fan(first, count);
			break;
		case GL_TRIANGLES:
			triangles(first, count);
			break;
		case GL_QUADS:
			quads(first, count);
			break;
		case GL_QUAD_STRIP:
			quad_strip(first, count);
			break;
		default:
			g_ctx->errno = GL_INVALID_ENUM;
			return;
	}
}

static GLboolean update_jit(void)
{
#ifdef ENABLE_GCCJIT
	if (!jit_update_depth_test(g_ctx)
	 || !jit_update_fragment_set(g_ctx))
	{
		g_ctx->errno = GL_OUT_OF_MEMORY;
		return GL_FALSE;
	}
	if (g_ctx->texture_binding_2d > 0 && g_ctx->texture_binding_2d < g_ctx->textures_capacity)
	{
		if (!jit_update_texture_fetch(g_ctx, g_ctx->textures[g_ctx->texture_binding_2d]))
		{
			g_ctx->errno = GL_OUT_OF_MEMORY;
			return GL_FALSE;
		}
	}
#endif
	return GL_TRUE;
}

void glDrawElements(GLenum mode, GLsizei count, GLenum type,
                    const GLvoid *indices)
{
	if (count < 0)
	{
		g_ctx->errno = GL_INVALID_VALUE;
		return;
	}
	if (!g_ctx->vertex_array.enabled)
		return;
	if (!count)
		return;
	if (!update_jit())
		return;
	g_ctx->indices = indices;
	g_ctx->indice_type = type;
	primitive(mode, 0, count);
}

void glMultiDrawElements(GLenum mode, const GLsizei *count, GLenum type,
                         const GLvoid **indices, GLsizei primcount)
{
	for (GLsizei i = 0; i < primcount; ++i)
		glDrawElements(mode, count[i], type, indices[i]);
}

void glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
	if (count < 0 || first < 0)
	{
		g_ctx->errno = GL_INVALID_VALUE;
		return;
	}
	if (!g_ctx->vertex_array.enabled)
		return;
	if (!count)
		return;
	if (!update_jit())
		return;
	g_ctx->indices = NULL;
	primitive(mode, 0, count);
}

void glMultiDrawArrays(GLenum mode, GLint *first, GLsizei *count,
                       GLsizei primcount)
{
	for (GLsizei i = 0; i < primcount; ++i)
		glDrawArrays(mode, first[i], count[i]);
}
