#include "internal.h"

#include <string.h>
#include <assert.h>
#include <math.h>

static void
matrix_dtf(GLfloat *mf, const GLdouble *m)
{
	for (GLint i = 0; i < 16; ++i)
		mf[i] = m[i];
}

static void
matrix_transpose(GLfloat * restrict dst, const GLfloat * restrict src)
{
	for (int y = 0; y < 4; ++y)
	{
		for (int x = 0; x < 4; ++x)
			dst[y + x * 4] = src[y + x * 4];
	}
}

static float
mult_value(const GLfloat *m1, const GLfloat *m2, int x, int y)
{
	return (m1[y + 0x0] * m2[0 + x * 4]
	      + m1[y + 0x4] * m2[1 + x * 4]
	      + m1[y + 0x8] * m2[2 + x * 4]
	      + m1[y + 0xC] * m2[3 + x * 4]);
}

static void
matrix_multiply(GLfloat *dst, const GLfloat *m1, const GLfloat *m2)
{
	GLfloat tmp[16];

	for (int y = 0; y < 4; ++y)
	{
		for (int x = 0; x < 4; ++x)
			tmp[y + x * 4] = mult_value(m1, m2, x, y);
	}
	memcpy(dst, &tmp, sizeof(tmp));
}

static GLfloat *
get_matrix_pointer(struct gl_ctx *ctx)
{
	switch (ctx->fixed.matrix_mode)
	{
		case GL_MODELVIEW:
			return ctx->fixed.modelview_matrix[ctx->fixed.modelview_stack_depth];
		case GL_PROJECTION:
			return ctx->fixed.projection_matrix[ctx->fixed.projection_stack_depth];
		default:
			return NULL;
	}
}

static void
mult_matrix(struct gl_ctx *ctx,
            const GLfloat *m)
{
	GLfloat *dst;

	if (ctx->immediate.enable)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	dst = get_matrix_pointer(ctx);
	if (!dst)
		return;
	matrix_multiply(dst, dst, m);
}

void
glFrustum(GLdouble left,
          GLdouble right,
          GLdouble bottom,
          GLdouble top,
          GLdouble near,
          GLdouble far)
{
	struct gl_ctx *ctx = g_ctx;
	GLfloat m[16];

	if (ctx->immediate.enable)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	if (near < 0 || far < 0)
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	m[0x0] = (2 * near) / (right - left);
	m[0x1] = 0;
	m[0x2] = 0;
	m[0x3] = 0;
	m[0x4] = 0;
	m[0x5] = (2 * near) / (top - bottom);
	m[0x6] = 0;
	m[0x7] = 0;
	m[0x8] = (right + left) / (right - left);
	m[0x9] = (top + bottom) / (top - bottom);
	m[0xA] = -(far + near) / (far - near);
	m[0xB] = -1;
	m[0xC] = 0;
	m[0xD] = 0;
	m[0xE] = -(2 * far * near) / (far - near);
	m[0xF] = 0;
	mult_matrix(ctx, m);
}

static void
load_matrix(struct gl_ctx *ctx,
            const GLfloat *m)
{
	GLfloat *dst;

	if (ctx->immediate.enable)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	dst = get_matrix_pointer(ctx);
	if (!dst)
	{
		assert(!"invalid matrix type");
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	memcpy(dst, m, sizeof(*m) * 16);
}

void
glLoadMatrixd(const GLdouble *m)
{
	struct gl_ctx *ctx = g_ctx;
	GLfloat mf[16];

	matrix_dtf(mf, m);
	load_matrix(ctx, mf);
}

void
glLoadMatrixf(const GLfloat *m)
{
	struct gl_ctx *ctx = g_ctx;

	load_matrix(ctx, m);
}

void
glLoadIdentity(void)
{
	struct gl_ctx *ctx = g_ctx;
	GLfloat *dst;

	if (ctx->immediate.enable)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	dst = get_matrix_pointer(ctx);
	if (!dst)
		return;
	dst[0x0] = 1;
	dst[0x1] = 0;
	dst[0x2] = 0;
	dst[0x3] = 0;
	dst[0x4] = 0;
	dst[0x5] = 1;
	dst[0x6] = 0;
	dst[0x7] = 0;
	dst[0x8] = 0;
	dst[0x9] = 0;
	dst[0xA] = 1;
	dst[0xB] = 0;
	dst[0xC] = 0;
	dst[0xD] = 0;
	dst[0xE] = 0;
	dst[0xF] = 1;
}

static void
load_transpose_matrix(struct gl_ctx *ctx,
                      const GLfloat *m)
{
	GLfloat *dst;

	if (ctx->immediate.enable)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	dst = get_matrix_pointer(ctx);
	if (!dst)
		return;
	matrix_transpose(dst, m);
}

void
glLoadTransposeMatrixd(const GLdouble *m)
{
	struct gl_ctx *ctx = g_ctx;
	GLfloat mf[16];

	matrix_dtf(mf, m);
	load_transpose_matrix(ctx, mf);
}

void
glLoadTransposeMatrixf(const GLfloat *m)
{
	struct gl_ctx *ctx = g_ctx;

	load_transpose_matrix(ctx, m);
}

void
glMatrixMode(GLenum mode)
{
	struct gl_ctx *ctx = g_ctx;

	if (ctx->immediate.enable)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	switch (mode)
	{
		case GL_MODELVIEW:
		case GL_PROJECTION:
			ctx->fixed.matrix_mode = mode;
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
}

void
glMultMatrixd(const GLdouble *m)
{
	struct gl_ctx *ctx = g_ctx;
	GLfloat mf[16];

	matrix_dtf(mf, m);
	mult_matrix(ctx, mf);
}

void
glMultMatrixf(const GLfloat *m)
{
	struct gl_ctx *ctx = g_ctx;

	mult_matrix(ctx, m);
}

static void
mult_transpose_matrix(struct gl_ctx *ctx,
                      const GLfloat *m)
{
	GLfloat *dst;
	GLfloat new[16];

	if (ctx->immediate.enable)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	dst = get_matrix_pointer(ctx);
	if (!dst)
		return;
	matrix_transpose(new, m);
	matrix_multiply(dst, dst, new);
}

void
glMultTransposeMatrixd(const GLdouble *m)
{
	struct gl_ctx *ctx = g_ctx;
	GLfloat mf[16];

	matrix_dtf(mf, m);
	mult_transpose_matrix(ctx, mf);
}

void
glMultTransposeMatrixf(const GLfloat *m)
{
	struct gl_ctx *ctx = g_ctx;

	mult_transpose_matrix(ctx, m);
}

void
glOrtho(GLdouble left,
        GLdouble right,
        GLdouble bottom,
        GLdouble top,
        GLdouble near,
        GLdouble far)
{
	struct gl_ctx *ctx = g_ctx;
	GLfloat m[16];

	if (ctx->immediate.enable)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	if (left == right || bottom == top || near == far)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	m[0x0] = 2 / (right - left);
	m[0x1] = 0;
	m[0x2] = 0;
	m[0x3] = 0;
	m[0x4] = 0;
	m[0x5] = 2 / (top - bottom);
	m[0x6] = 0;
	m[0x7] = 0;
	m[0x8] = 0;
	m[0x9] = 0;
	m[0xA] = -2 / (far - near);
	m[0xB] = 0;
	m[0xC] = -(right + left) / (right - left);
	m[0xD] = -(top + bottom) / (top - bottom);
	m[0xE] = -(far + near) / (far - near);
	m[0xF] = 1;
	mult_matrix(ctx, m);
}

void
glPopMatrix(void)
{
	struct gl_ctx *ctx = g_ctx;

	if (ctx->immediate.enable)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	switch (ctx->fixed.matrix_mode)
	{
		case GL_MODELVIEW:
			if (ctx->fixed.modelview_stack_depth <= 0)
			{
				GL_SET_ERR(ctx, GL_STACK_UNDERFLOW);
				return;
			}
			ctx->fixed.modelview_stack_depth--;
			break;
		case GL_PROJECTION:
			if (ctx->fixed.projection_stack_depth <= 0)
			{
				GL_SET_ERR(ctx, GL_STACK_UNDERFLOW);
				return;
			}
			ctx->fixed.projection_stack_depth--;
			break;
		default:
			assert(!"invalid matrix mode");
			return;
	}
}

void
glPushMatrix(void)
{
	struct gl_ctx *ctx = g_ctx;

	if (ctx->immediate.enable)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	switch (ctx->fixed.matrix_mode)
	{
		case GL_MODELVIEW:
			if (ctx->fixed.modelview_stack_depth >= ctx->fixed.modelview_max_stack_depth - 1)
			{
				GL_SET_ERR(ctx, GL_STACK_OVERFLOW);
				return;
			}
			memcpy(&ctx->fixed.modelview_matrix[ctx->fixed.modelview_stack_depth + 1],
			       &ctx->fixed.modelview_matrix[ctx->fixed.modelview_stack_depth],
			       sizeof(*ctx->fixed.modelview_matrix));
			ctx->fixed.modelview_stack_depth++;
			break;
		case GL_PROJECTION:
			if (ctx->fixed.projection_stack_depth >= ctx->fixed.projection_max_stack_depth - 1)
			{
				GL_SET_ERR(ctx, GL_STACK_OVERFLOW);
				return;
			}
			memcpy(&ctx->fixed.projection_matrix[ctx->fixed.projection_stack_depth + 1],
			       &ctx->fixed.projection_matrix[ctx->fixed.projection_stack_depth],
			       sizeof(*ctx->fixed.projection_matrix));
			ctx->fixed.projection_stack_depth++;
			break;
		default:
			assert(!"invalid matrix mode");
			return;
	}
}

static void
rotate(struct gl_ctx *ctx,
       GLfloat angle,
       GLfloat x,
       GLfloat y,
       GLfloat z)
{
	GLfloat m[16];
	GLfloat c;
	GLfloat s;

	if (ctx->immediate.enable)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	x = clampf(x, -1, 1);
	y = clampf(y, -1, 1);
	z = clampf(z, -1, 1);
	c = cosf(angle);
	s = sinf(angle);
	m[0x0] = x * x * (1 - c) + c;
	m[0x1] = y * x * (1 - c) + z * s;
	m[0x2] = x * z * (1 - c) - y * s;
	m[0x3] = 0;
	m[0x4] = x * y * (1 - c) - z * s;
	m[0x5] = y * y * (1 - c) + c;
	m[0x6] = y * z * (1 - c) + x * s;
	m[0x7] = 0;
	m[0x8] = x * z * (1 - c) + y * s;
	m[0x9] = y * z * (1 - c) - x * s;
	m[0xA] = z * z * (1 - c) + c;
	m[0xB] = 0;
	m[0xC] = 0;
	m[0xD] = 0;
	m[0xE] = 0;
	m[0xF] = 1;
	mult_matrix(ctx, m);
}

void
glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
	rotate(g_ctx, angle, x, y, z);
}

void
glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
	rotate(g_ctx, angle, x, y, z);
}

static void
scale(struct gl_ctx *ctx,
      GLfloat x,
      GLfloat y,
      GLfloat z)
{
	GLfloat m[16];

	if (ctx->immediate.enable)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	m[0x0] = x;
	m[0x1] = 0;
	m[0x2] = 0;
	m[0x3] = 0;
	m[0x4] = 0;
	m[0x5] = y;
	m[0x6] = 0;
	m[0x7] = 0;
	m[0x8] = 0;
	m[0x9] = 0;
	m[0xA] = z;
	m[0xB] = 0;
	m[0xC] = 0;
	m[0xD] = 0;
	m[0xE] = 0;
	m[0xF] = 1;
	mult_matrix(ctx, m);
}

void
glScaled(GLdouble x, GLdouble y, GLdouble z)
{
	scale(g_ctx, x, y, z);
}

void
glScalef(GLfloat x, GLfloat y, GLfloat z)
{
	scale(g_ctx, x, y, z);
}

static void
translate(struct gl_ctx *ctx,
          GLfloat x,
          GLfloat y,
          GLfloat z)
{
	GLfloat m[16];

	if (ctx->immediate.enable)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	m[0x0] = 1;
	m[0x1] = 0;
	m[0x2] = 0;
	m[0x3] = 0;
	m[0x4] = 0;
	m[0x5] = 1;
	m[0x6] = 0;
	m[0x7] = 0;
	m[0x8] = 0;
	m[0x9] = 0;
	m[0xA] = 1;
	m[0xB] = 0;
	m[0xC] = x;
	m[0xD] = y;
	m[0xE] = z;
	m[0xF] = 1;
	mult_matrix(ctx, m);
}

void
glTranslated(GLdouble x, GLdouble y, GLdouble z)
{
	translate(g_ctx, x, y, z);
}

void
glTranslatef(GLfloat x, GLfloat y, GLfloat z)
{
	translate(g_ctx, x, y, z);
}
