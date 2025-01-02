#include "internal.h"

#include <string.h>
#include <assert.h>
#include <math.h>

typedef void (*mat_opf_t)(const GLfloat *m);

static void mat_opf(const GLdouble *m, mat_opf_t opf)
{
	GLfloat matf[16];
	for (GLint i = 0; i < 16; ++i)
		matf[i] = m[i];
	opf(matf);
}

static void get_curmat(struct mat4 *mat)
{
	switch (g_ctx->matrix_mode)
	{
		case GL_MODELVIEW:
			memcpy(mat, &g_ctx->modelview_matrix[g_ctx->modelview_stack_depth], sizeof(*mat));
			break;
		case GL_PROJECTION:
			memcpy(mat, &g_ctx->projection_matrix[g_ctx->projection_stack_depth], sizeof(*mat));
			break;
		default:
			g_ctx->errno = GL_INVALID_ENUM;
			return;
	}
}

void glFrustum(GLdouble left, GLdouble right, GLdouble bottom,
               GLdouble top, GLdouble near, GLdouble far)
{
	struct mat4 mat;

	if (g_ctx->immediate.enabled)
	{
		g_ctx->errno = GL_INVALID_OPERATION;
		return;
	}
	if (near < 0 || far < 0)
	{
		g_ctx->errno = GL_INVALID_ENUM;
		return;
	}
	mat4_clear(&mat);
	mat.v[0] = (2 * near) / (right - left);
	mat.v[5] = (2 * near) / (top - bottom);
	mat.v[8] = (right + left) / (right - left);
	mat.v[9] = (top + bottom) / (top - bottom);
	mat.v[10] = -(far + near) / (far - near);
	mat.v[11] = -1;
	mat.v[14] = -(2 * far * near) / (far - near);
	glMultMatrixf(mat.v);
}

void glLoadMatrixd(const GLdouble *m)
{
	mat_opf(m, glLoadMatrixf);
}

void glLoadMatrixf(const GLfloat *m)
{
	GLfloat *dst;

	if (g_ctx->immediate.enabled)
	{
		g_ctx->errno = GL_INVALID_OPERATION;
		return;
	}
	switch (g_ctx->matrix_mode)
	{
		case GL_MODELVIEW:
			dst = g_ctx->modelview_matrix[g_ctx->modelview_stack_depth].v;
			break;
		case GL_PROJECTION:
			dst = g_ctx->projection_matrix[g_ctx->projection_stack_depth].v;
			break;
		default:
			assert(!"invalid matrix type");
			return;
	}
	memcpy(dst, m, sizeof(*m) * 16);
}

void glLoadIdentity(void)
{
	struct mat4 identity;

	if (g_ctx->immediate.enabled)
	{
		g_ctx->errno = GL_INVALID_OPERATION;
		return;
	}
	mat4_init_identity(&identity);
	glLoadMatrixf(identity.v);
}

void glLoadTransposeMatrixd(const GLdouble *m)
{
	mat_opf(m, glLoadTransposeMatrixf);
}

void glLoadTransposeMatrixf(const GLfloat *m)
{
	struct mat4 *dst;

	if (g_ctx->immediate.enabled)
	{
		g_ctx->errno = GL_INVALID_OPERATION;
		return;
	}
	switch (g_ctx->matrix_mode)
	{
		case GL_MODELVIEW:
			dst = &g_ctx->modelview_matrix[g_ctx->modelview_stack_depth];
			break;
		case GL_PROJECTION:
			dst = &g_ctx->projection_matrix[g_ctx->projection_stack_depth];
			break;
		default:
			assert(!"invalid matrix type");
			return;
	}
	memcpy(dst->v, m, sizeof(*m) * 16);
	mat4_reverse(dst);
}

void glMatrixMode(GLenum mode)
{
	if (g_ctx->immediate.enabled)
	{
		g_ctx->errno = GL_INVALID_OPERATION;
		return;
	}
	switch (mode)
	{
		case GL_MODELVIEW:
		case GL_PROJECTION:
			g_ctx->matrix_mode = mode;
			break;
		default:
			g_ctx->errno = GL_INVALID_ENUM;
			return;
	}
}

void glMultMatrixd(const GLdouble *m)
{
	mat_opf(m, glMultMatrixf);
}

void glMultMatrixf(const GLfloat *m)
{
	struct mat4 cur_mat;
	struct mat4 new;

	if (g_ctx->immediate.enabled)
	{
		g_ctx->errno = GL_INVALID_OPERATION;
		return;
	}
	memcpy(new.v, m, sizeof(*m) * 16);
	get_curmat(&cur_mat);
	mat4_mult(&cur_mat, &cur_mat, &new);
	glLoadMatrixf(cur_mat.v);
}

void glMultTransposeMatrixd(const GLdouble *m)
{
	mat_opf(m, glMultTransposeMatrixf);
}

void glMultTransposeMatrixf(const GLfloat *m)
{
	struct mat4 cur_mat;
	struct mat4 new;

	if (g_ctx->immediate.enabled)
	{
		g_ctx->errno = GL_INVALID_OPERATION;
		return;
	}
	memcpy(new.v, m, sizeof(*m) * 16);
	mat4_reverse(&new);
	get_curmat(&cur_mat);
	mat4_mult(&cur_mat, &cur_mat, &new);
	glLoadMatrixf(cur_mat.v);
}

void glOrtho(GLdouble left, GLdouble right, GLdouble bottom,
             GLdouble top, GLdouble near, GLdouble far)
{
	struct mat4 mat;

	if (g_ctx->immediate.enabled)
	{
		g_ctx->errno = GL_INVALID_OPERATION;
		return;
	}
	if (left == right || bottom == top || near == far)
	{
		g_ctx->errno = GL_INVALID_VALUE;
		return;
	}
	mat4_clear(&mat);
	mat.v[0] = 2 / (right - left);
	mat.v[5] = 2 / (top - bottom);
	mat.v[10] = -2 / (far - near);
	mat.v[12] = -(right + left) / (right - left);
	mat.v[13] = -(top + bottom) / (top - bottom);
	mat.v[14] = -(far + near) / (far - near);
	mat.v[15] = 1;
	glMultMatrixf(mat.v);
}

void glPopMatrix(void)
{
	if (g_ctx->immediate.enabled)
	{
		g_ctx->errno = GL_INVALID_OPERATION;
		return;
	}
	switch (g_ctx->matrix_mode)
	{
		case GL_MODELVIEW:
			if (g_ctx->modelview_stack_depth <= 0)
			{
				g_ctx->errno = GL_STACK_UNDERFLOW;
				return;
			}
			g_ctx->modelview_stack_depth--;
			break;
		case GL_PROJECTION:
			if (g_ctx->projection_stack_depth <= 0)
			{
				g_ctx->errno = GL_STACK_UNDERFLOW;
				return;
			}
			g_ctx->projection_stack_depth--;
			break;
		default:
			assert(!"invalid matrix mode");
			return;
	}
}

void glPushMatrix(void)
{
	if (g_ctx->immediate.enabled)
	{
		g_ctx->errno = GL_INVALID_OPERATION;
		return;
	}
	switch (g_ctx->matrix_mode)
	{
		case GL_MODELVIEW:
			if (g_ctx->modelview_stack_depth >= g_ctx->modelview_max_stack_depth - 1)
			{
				g_ctx->errno = GL_STACK_OVERFLOW;
				return;
			}
			memcpy(&g_ctx->modelview_matrix[g_ctx->modelview_stack_depth + 1],
			       &g_ctx->modelview_matrix[g_ctx->modelview_stack_depth],
			       sizeof(*g_ctx->modelview_matrix));
			g_ctx->modelview_stack_depth++;
			break;
		case GL_PROJECTION:
			if (g_ctx->projection_stack_depth >= g_ctx->projection_max_stack_depth - 1)
			{
				g_ctx->errno = GL_STACK_OVERFLOW;
				return;
			}
			memcpy(&g_ctx->projection_matrix[g_ctx->projection_stack_depth + 1],
			       &g_ctx->projection_matrix[g_ctx->projection_stack_depth],
			       sizeof(*g_ctx->projection_matrix));
			g_ctx->projection_stack_depth++;
			break;
		default:
			assert(!"invalid matrix mode");
			return;
	}
}

void glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
	glRotatef(angle, x, y, z);
}

void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
	struct mat4 rotation;
	GLfloat c;
	GLfloat s;

	if (g_ctx->immediate.enabled)
	{
		g_ctx->errno = GL_INVALID_OPERATION;
		return;
	}
	x = clampf(x, -1, 1);
	y = clampf(y, -1, 1);
	z = clampf(z, -1, 1);
	c = cosf(angle);
	s = sinf(angle);
	mat4_clear(&rotation);
	rotation.v[0] = x * x * (1 - c) + c;
	rotation.v[1] = y * x * (1 - c) + z * s;
	rotation.v[2] = x * z * (1 - c) - y * s;
	rotation.v[4] = x * y * (1 - c) - z * s;
	rotation.v[5] = y * y * (1 - c) + c;
	rotation.v[6] = y * z * (1 - c) + x * s;
	rotation.v[8] = x * z * (1 - c) + y * s;
	rotation.v[9] = y * z * (1 - c) - x * s;
	rotation.v[10] = z * z * (1 - c) + c;
	rotation.v[15] = 1;
	glMultMatrixf(rotation.v);
}

void glScaled(GLdouble x, GLdouble y, GLdouble z)
{
	glScalef(x, y, z);
}

void glScalef(GLfloat x, GLfloat y, GLfloat z)
{
	struct mat4 scale;

	if (g_ctx->immediate.enabled)
	{
		g_ctx->errno = GL_INVALID_OPERATION;
		return;
	}
	mat4_init_scale(&scale, x, y, z);
	glMultMatrixf(scale.v);
}

void glTranslated(GLdouble x, GLdouble y, GLdouble z)
{
	glTranslatef(x, y, z);
}

void glTranslatef(GLfloat x, GLfloat y, GLfloat z)
{
	struct mat4 translate;

	if (g_ctx->immediate.enabled)
	{
		g_ctx->errno = GL_INVALID_OPERATION;
		return;
	}
	mat4_init_translation(&translate, x, y, z);
	glMultMatrixf(translate.v);
}
