#include "internal.h"

#include <limits.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>

void
glBegin(GLenum mode)
{
	struct gl_ctx *ctx = g_ctx;

	if (ctx->immediate.enable)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	switch (mode)
	{
		case GL_POINTS:
		case GL_LINE_STRIP:
		case GL_LINE_LOOP:
		case GL_LINES:
		case GL_TRIANGLE_STRIP:
		case GL_TRIANGLE_FAN:
		case GL_TRIANGLES:
		case GL_QUADS:
		case GL_QUAD_STRIP:
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
	ctx->immediate.mode = mode;
	ctx->immediate.enable = GL_TRUE;
	ctx->immediate.vertexes_pos = 0;
}

static void
flush_immediate(struct gl_ctx *ctx)
{
	if (!ctx->immediate.vertexes_len)
		return;
	struct program *prev_program = ctx->program;
	program_bind(ctx, NULL);
	interleaved_arrays(ctx,
	                   GL_T4F_C4F_N3F_V4F,
	                   sizeof(*ctx->immediate.vertexes),
	                   ctx->immediate.vertexes);
	draw_arrays(ctx,
	            ctx->immediate.mode,
	            0,
	            ctx->immediate.vertexes_pos,
	            0,
	            0);
	program_bind(ctx, prev_program);
}

void
glEnd(void)
{
	struct gl_ctx *ctx = g_ctx;

	if (!ctx->immediate.enable)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	flush_immediate(ctx);
	ctx->immediate.enable = GL_FALSE;
}

static void
glVertex(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	struct immediate_vertex *vertex;
	struct gl_ctx *ctx = g_ctx;

	if (ctx->immediate.vertexes_pos >= ctx->immediate.vertexes_len)
	{
		size_t new_len;

		new_len = ctx->immediate.vertexes_len * 2;
		if (!new_len)
			new_len = 1024;
		vertex = realloc(ctx->immediate.vertexes, sizeof(*vertex) * new_len);
		if (!vertex)
		{
			GL_SET_ERR(ctx, GL_OUT_OF_MEMORY);
			return;
		}
		ctx->immediate.vertexes = vertex;
		ctx->immediate.vertexes_len = new_len;
	}
	vertex = &ctx->immediate.vertexes[ctx->immediate.vertexes_pos++];
	vertex->position[0] = x;
	vertex->position[1] = y;
	vertex->position[2] = z;
	vertex->position[3] = w;
	vertex->color[0] = ctx->immediate.color[0];
	vertex->color[1] = ctx->immediate.color[1];
	vertex->color[2] = ctx->immediate.color[2];
	vertex->color[3] = ctx->immediate.color[3];
	vertex->tex_coord[0] = ctx->immediate.tex_coord[0];
	vertex->tex_coord[1] = ctx->immediate.tex_coord[1];
	vertex->tex_coord[2] = ctx->immediate.tex_coord[2];
	vertex->tex_coord[3] = ctx->immediate.tex_coord[3];
	vertex->normal[0] = ctx->immediate.normal[0];
	vertex->normal[1] = ctx->immediate.normal[1];
	vertex->normal[2] = ctx->immediate.normal[2];
}

static void
glNormal(GLfloat x, GLfloat y, GLfloat z)
{
	struct gl_ctx *ctx = g_ctx;

	ctx->immediate.normal[0] = x;
	ctx->immediate.normal[1] = y;
	ctx->immediate.normal[2] = z;
}

static void
glTexCoord(GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
	struct gl_ctx *ctx = g_ctx;

	ctx->immediate.tex_coord[0] = s;
	ctx->immediate.tex_coord[1] = t;
	ctx->immediate.tex_coord[2] = r;
	ctx->immediate.tex_coord[3] = q;
}

static void
glColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
	struct gl_ctx *ctx = g_ctx;

	ctx->immediate.color[0] = r;
	ctx->immediate.color[1] = g;
	ctx->immediate.color[2] = b;
	ctx->immediate.color[3] = a;
}

void
glVertex2s(GLshort x, GLshort y)
{
	glVertex(x, y, 0, 1);
}

void
glVertex2i(GLint x, GLint y)
{
	glVertex(x, y, 0, 1);
}

void
glVertex2f(GLfloat x, GLfloat y)
{
	glVertex(x, y, 0, 1);
}

void
glVertex2d(GLdouble x, GLdouble y)
{
	glVertex(x, y, 0, 1);
}

void
glVertex3s(GLshort x, GLshort y, GLshort z)
{
	glVertex(x, y, z, 1);
}

void
glVertex3i(GLint x, GLint y, GLint z)
{
	glVertex(x, y, z, 1);
}

void
glVertex3f(GLfloat x, GLfloat y, GLfloat z)
{
	glVertex(x, y, z, 1);
}

void
glVertex3d(GLdouble x, GLdouble y, GLdouble z)
{
	glVertex(x, y, z, 1);
}

void
glVertex4s(GLshort x, GLshort y, GLshort z, GLshort w)
{
	glVertex(x, y, z, w);
}

void
glVertex4i(GLint x, GLint y, GLint z, GLint w)
{
	glVertex(x, y, z, w);
}

void
glVertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	glVertex(x, y, z, w);
}

void
glVertex4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	glVertex(x, y, z, w);
}

void
glVertex2sv(const GLshort *v)
{
	glVertex(v[0], v[1], 0, 1);
}

void
glVertex2iv(const GLint *v)
{
	glVertex(v[0], v[1], 0, 1);
}

void
glVertex2fv(const GLfloat *v)
{
	glVertex(v[0], v[1], 0, 1);
}

void
glVertex2dv(const GLdouble *v)
{
	glVertex(v[0], v[1], v[2], 1);
}

void
glVertex3sv(const GLshort *v)
{
	glVertex(v[0], v[1], v[2], 1);
}

void
glVertex3iv(const GLint *v)
{
	glVertex(v[0], v[1], v[2], 1);
}

void
glVertex3fv(const GLfloat *v)
{
	glVertex(v[0], v[1], v[2], 1);
}

void
glVertex3dv(const GLdouble *v)
{
	glVertex(v[0], v[1], v[2], 1);
}

void
glVertex4sv(const GLshort *v)
{
	glVertex(v[0], v[1], v[2], v[3]);
}

void
glVertex4iv(const GLint *v)
{
	glVertex(v[0], v[1], v[2], v[3]);
}

void
glVertex4fv(const GLfloat *v)
{
	glVertex(v[0], v[1], v[2], v[3]);
}

void
glVertex4dv(const GLdouble *v)
{
	glVertex(v[0], v[1], v[2], v[3]);
}

void
glNormal3b(GLbyte x, GLbyte y, GLbyte z)
{
	glNormal(x * (1.0 / INT8_MAX),
	         y * (1.0 / INT8_MAX),
	         z * (1.0 / INT8_MAX));
}

void
glNormal3s(GLshort x, GLshort y, GLshort z)
{
	glNormal(x * (1.0 / INT16_MAX),
	         y * (1.0 / INT16_MAX),
	         z * (1.0 / INT16_MAX));
}

void
glNormal3i(GLint x, GLint y, GLint z)
{
	glNormal(x * (1.0 / INT32_MAX),
	         y * (1.0 / INT32_MAX),
	         z * (1.0 / INT32_MAX));
}

void
glNormal3f(GLfloat x, GLfloat y, GLfloat z)
{
	glNormal(x, y, z);
}

void
glNormal3d(GLdouble x, GLdouble y, GLdouble z)
{
	glNormal(x, y, z);
}

void
glNormal3bv(const GLbyte *v)
{
	glNormal(v[0] * (1.0 / INT8_MAX),
	         v[1] * (1.0 / INT8_MAX),
	         v[2] * (1.0 / INT8_MAX));
}

void
glNormal3sv(const GLshort *v)
{
	glNormal(v[0] * (1.0 / INT16_MAX),
	         v[1] * (1.0 / INT16_MAX),
	         v[2] * (1.0 / INT16_MAX));
}

void
glNormal3iv(const GLint *v)
{
	glNormal(v[0] * (1.0 / INT32_MAX),
	         v[1] * (1.0 / INT32_MAX),
	         v[2] * (1.0 / INT32_MAX));
}

void
glNormal3fv(const GLfloat *v)
{
	glNormal(v[0], v[1], v[2]);
}

void
glNormal3dv(const GLdouble *v)
{
	glNormal(v[0], v[1], v[2]);
}

void
glTexCoord1s(GLshort s)
{
	glTexCoord(s * (1.0 / INT16_MAX), 0, 0, 1);
}

void
glTexCoord1i(GLint s)
{
	glTexCoord(s * (1.0 / INT32_MAX), 0, 0, 1);
}

void
glTexCoord1f(GLfloat s)
{
	glTexCoord(s, 0, 0, 1);
}

void
glTexCoord1d(GLdouble s)
{
	glTexCoord(s, 0, 0, 1);
}

void
glTexCoord2s(GLshort s, GLshort t)
{
	glTexCoord(s * (1.0 / INT16_MAX),
	           t * (1.0 / INT16_MAX),
	           0,
	           1);
}

void
glTexCoord2i(GLint s, GLint t)
{
	glTexCoord(s * (1.0 / INT32_MAX),
	           t * (1.0 / INT32_MAX),
	           0,
	           1);
}

void
glTexCoord2f(GLfloat s, GLfloat t)
{
	glTexCoord(s, t, 0, 1);
}

void
glTexCoord2d(GLdouble s, GLdouble t)
{
	glTexCoord(s, t, 0, 1);
}

void
glTexCoord3s(GLshort s, GLshort t, GLshort r)
{
	glTexCoord(s * (1.0 / INT16_MAX),
	           t * (1.0 / INT16_MAX),
	           r * (1.0 / INT16_MAX),
	           1);
}

void
glTexCoord3i(GLint s, GLint t, GLint r)
{
	glTexCoord(s * (1.0 / INT32_MAX),
	           t * (1.0 / INT32_MAX),
	           r * (1.0 / INT32_MAX),
	           1);
}

void
glTexCoord3f(GLfloat s, GLfloat t, GLfloat r)
{
	glTexCoord(s, t, r, 1);
}

void
glTexCoord3d(GLdouble s, GLdouble t, GLdouble r)
{
	glTexCoord(s, t, r, 1);
}

void
glTexCoord4s(GLshort s, GLshort t, GLshort r, GLshort q)
{
	glTexCoord(s * (1.0 / INT16_MAX),
	           t * (1.0 / INT16_MAX),
	           r * (1.0 / INT16_MAX),
	           q * (1.0 / INT16_MAX));
}

void
glTexCoord4i(GLint s, GLint t, GLint r, GLint q)
{
	glTexCoord(s * (1.0 / INT32_MAX),
	           t * (1.0 / INT32_MAX),
	           r * (1.0 / INT32_MAX),
	           q * (1.0 / INT32_MAX));
}

void
glTexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
	glTexCoord(s, t, r, q);
}

void
glTexCoord4d(GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
	glTexCoord(s, t, r, q);
}

void
glTexCoord1sv(const GLshort *v)
{
	glTexCoord(v[0] * (1.0 / INT16_MAX), 0, 0, 1);
}

void
glTexCoord1iv(const GLint *v)
{
	glTexCoord(v[0] * (1.0 / INT32_MAX), 0, 0, 1);
}

void
glTexCoord1fv(const GLfloat *v)
{
	glTexCoord(v[0], 0, 0, 1);
}

void
glTexCoord1dv(const GLdouble *v)
{
	glTexCoord(v[0], 0, 0, 1);
}

void
glTexCoord2sv(const GLshort *v)
{
	glTexCoord(v[0] * (1.0 / INT16_MAX),
	           v[1] * (1.0 / INT16_MAX),
	           0,
	           1);
}

void
glTexCoord2iv(const GLint *v)
{
	glTexCoord(v[0] * (1.0 / INT32_MAX),
	           v[1] * (1.0 / INT32_MAX),
	           0,
	           1);
}

void
glTexCoord2fv(const GLfloat *v)
{
	glTexCoord(v[0], v[1], 0, 1);
}

void
glTexCoord2dv(const GLdouble *v)
{
	glTexCoord(v[0], v[1], 0, 1);
}

void
glTexCoord3sv(const GLshort *v)
{
	glTexCoord(v[0] * (1.0 / INT16_MAX),
	           v[1] * (1.0 / INT16_MAX),
	           v[2] * (1.0 / INT16_MAX),
	           1);
}

void
glTexCoord3iv(const GLint *v)
{
	glTexCoord(v[0] * (1.0 / INT32_MAX),
	           v[1] * (1.0 / INT32_MAX),
	           v[2] * (1.0 / INT32_MAX),
	           1);
}

void
glTexCoord3fv(const GLfloat *v)
{
	glTexCoord(v[0], v[1], v[2], 1);
}

void
glTexCoord3dv(const GLdouble *v)
{
	glTexCoord(v[0], v[1], v[2], 1);
}

void
glTexCoord4sv(const GLshort *v)
{
	glTexCoord(v[0] * (1.0 / INT16_MAX),
	           v[1] * (1.0 / INT16_MAX),
	           v[2] * (1.0 / INT16_MAX),
	           v[3] * (1.0 / INT16_MAX));
}

void
glTexCoord4iv(const GLint *v)
{
	glTexCoord(v[0] * (1.0 / INT32_MAX),
	           v[1] * (1.0 / INT32_MAX),
	           v[2] * (1.0 / INT32_MAX),
	           v[3] * (1.0 / INT32_MAX));
}

void
glTexCoord4fv(const GLfloat *v)
{
	glTexCoord(v[0], v[1], v[2], v[3]);
}

void
glTexCoord4dv(const GLdouble *v)
{
	glTexCoord(v[0], v[1], v[2], v[3]);
}

void
glColor3b(GLbyte r, GLbyte g, GLbyte b)
{
	glColor(r * (1.0 / INT8_MAX),
	        g * (1.0 / INT8_MAX),
	        b * (1.0 / INT8_MAX),
	        1);
}

void
glColor3ub(GLubyte r, GLubyte g, GLubyte b)
{
	glColor(r * (1.0 / UINT8_MAX),
	        g * (1.0 / UINT8_MAX),
	        b * (1.0 / UINT8_MAX),
	        1);
}

void
glColor3s(GLshort r, GLshort g, GLshort b)
{
	glColor(r * (1.0 / INT16_MAX),
	        g * (1.0 / INT16_MAX),
	        b * (1.0 / INT16_MAX),
	        1);
}

void
glColor3us(GLushort r, GLushort g, GLushort b)
{
	glColor(r * (1.0 / UINT16_MAX),
	        g * (1.0 / UINT16_MAX),
	        b * (1.0 / UINT16_MAX),
	        1);
}

void
glColor3i(GLint r, GLint g, GLint b)
{
	glColor(r * (1.0 / INT32_MAX),
	        g * (1.0 / INT32_MAX),
	        b * (1.0 / INT32_MAX),
	        1);
}

void
glColor3ui(GLuint r, GLuint g, GLuint b)
{
	glColor(r * (1.0 / UINT32_MAX),
	        g * (1.0 / UINT32_MAX),
	        b * (1.0 / UINT32_MAX),
	        1);
}

void
glColor3f(GLfloat r, GLfloat g, GLfloat b)
{
	glColor(r, g, b, 1);
}

void
glColor3d(GLdouble r, GLdouble g, GLdouble b)
{
	glColor(r, g, b, 1);
}

void
glColor4b(GLbyte r, GLbyte g, GLbyte b, GLbyte a)
{
	glColor(r * (1.0 / INT8_MAX),
	        g * (1.0 / INT8_MAX),
	        b * (1.0 / INT8_MAX),
	        a * (1.0 / INT8_MAX));
}

void
glColor4ub(GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{
	glColor(r * (1.0 / UINT8_MAX),
	        g * (1.0 / UINT8_MAX),
	        b * (1.0 / UINT8_MAX),
	        a * (1.0 / UINT8_MAX));
}

void
glColor4s(GLshort r, GLshort g, GLshort b, GLshort a)
{
	glColor(r * (1.0 / INT16_MAX),
	        g * (1.0 / INT16_MAX),
	        b * (1.0 / INT16_MAX),
	        a * (1.0 / INT16_MAX));
}

void
glColor4us(GLushort r, GLushort g, GLushort b, GLushort a)
{
	glColor(r * (1.0 / UINT16_MAX),
	        g * (1.0 / UINT16_MAX),
	        b * (1.0 / UINT16_MAX),
	        a * (1.0 / UINT16_MAX));
}

void
glColor4i(GLint r, GLint g, GLint b, GLint a)
{
	glColor(r * (1.0 / INT32_MAX),
	        g * (1.0 / INT32_MAX),
	        b * (1.0 / INT32_MAX),
	        a * (1.0 / INT32_MAX));
}

void
glColor4ui(GLuint r, GLuint g, GLuint b, GLuint a)
{
	glColor(r * (1.0 / UINT32_MAX),
	        g * (1.0 / UINT32_MAX),
	        b * (1.0 / UINT32_MAX),
	        a * (1.0 / UINT32_MAX));
}

void
glColor4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
	glColor(r, g, b, a);
}

void
glColor4d(GLdouble r, GLdouble g, GLdouble b, GLdouble a)
{
	glColor(r, g, b, a);
}

void
glColor3bv(const GLbyte *v)
{
	glColor(v[0] * (1.0 / INT8_MAX),
	        v[1] * (1.0 / INT8_MAX),
	        v[2] * (1.0 / INT8_MAX),
	        1);
}

void
glColor3ubv(const GLubyte *v)
{
	glColor(v[0] * (1.0 / UINT8_MAX),
	        v[1] * (1.0 / UINT8_MAX),
	        v[2] * (1.0 / UINT8_MAX),
	        1);
}

void
glColor3sv(const GLshort *v)
{
	glColor(v[0] * (1.0 / INT16_MAX),
	        v[1] * (1.0 / INT16_MAX),
	        v[2] * (1.0 / INT16_MAX),
	        1);
}

void
glColor3usv(const GLushort *v)
{
	glColor(v[0] * (1.0 / UINT16_MAX),
	        v[1] * (1.0 / UINT16_MAX),
	        v[2] * (1.0 / UINT16_MAX),
	        1);
}

void
glColor3iv(const GLint *v)
{
	glColor(v[0] * (1.0 / INT32_MAX),
	        v[1] * (1.0 / INT32_MAX),
	        v[2] * (1.0 / INT32_MAX),
	        1);
}

void
glColor3uiv(const GLuint *v)
{
	glColor(v[0] * (1.0 / UINT32_MAX),
	        v[1] * (1.0 / UINT32_MAX),
	        v[2] * (1.0 / UINT32_MAX),
	        1);
}

void
glColor3fv(const GLfloat *v)
{
	glColor(v[0], v[1], v[2], 1);
}

void
glColor3dv(const GLdouble *v)
{
	glColor(v[0], v[1], v[2], 1);
}

void
glColor4bv(const GLbyte *v)
{
	glColor(v[0] * (1.0 / INT8_MAX),
	        v[1] * (1.0 / INT8_MAX),
	        v[2] * (1.0 / INT8_MAX),
	        v[3] * (1.0 / INT8_MAX));
}

void
glColor4ubv(const GLubyte *v)
{
	glColor(v[0] * (1.0 / UINT8_MAX),
	        v[1] * (1.0 / UINT8_MAX),
	        v[2] * (1.0 / UINT8_MAX),
	        v[3] * (1.0 / UINT8_MAX));
}

void
glColor4sv(const GLshort *v)
{
	glColor(v[0] * (1.0 / INT16_MAX),
	        v[1] * (1.0 / INT16_MAX),
	        v[2] * (1.0 / INT16_MAX),
	        v[3] * (1.0 / INT16_MAX));
}

void
glColor4usv(const GLushort *v)
{
	glColor(v[0] * (1.0 / UINT16_MAX),
	        v[1] * (1.0 / UINT16_MAX),
	        v[2] * (1.0 / UINT16_MAX),
	        v[3] * (1.0 / UINT16_MAX));
}

void
glColor4iv(const GLint *v)
{
	glColor(v[0] * (1.0 / INT32_MAX),
	        v[1] * (1.0 / INT32_MAX),
	        v[2] * (1.0 / INT32_MAX),
	        v[3] * (1.0 / INT32_MAX));
}

void
glColor4uiv(const GLuint *v)
{
	glColor(v[0] * (1.0 / UINT32_MAX),
	        v[1] * (1.0 / UINT32_MAX),
	        v[2] * (1.0 / UINT32_MAX),
	        v[3] * (1.0 / UINT32_MAX));
}

void
glColor4fv(const GLfloat *v)
{
	glColor(v[0], v[1], v[2], v[3]);
}

void
glColor4dv(const GLdouble *v)
{
	glColor(v[0], v[1], v[2], v[3]);
}
