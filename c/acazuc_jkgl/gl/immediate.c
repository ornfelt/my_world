#include "internal.h"
#include "fixed.h"

#include <limits.h>
#include <math.h>

void glBegin(GLenum mode)
{
	if (g_ctx->immediate.enabled)
	{
		g_ctx->errno = GL_INVALID_OPERATION;
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
			g_ctx->errno = GL_INVALID_ENUM;
			return;
	}
#ifdef ENABLE_GCCJIT
	if (!jit_update_depth_test(g_ctx)
	 || !jit_update_fragment_set(g_ctx))
	{
		g_ctx->errno = GL_OUT_OF_MEMORY;
		return;
	}
	if (g_ctx->texture_binding_2d > 0 && g_ctx->texture_binding_2d < g_ctx->textures_capacity)
	{
		if (!jit_update_texture_fetch(g_ctx, g_ctx->textures[g_ctx->texture_binding_2d]))
		{
			g_ctx->errno = GL_OUT_OF_MEMORY;
			return;
		}
	}
#endif
	g_ctx->immediate.mode = mode;
	g_ctx->immediate.enabled = GL_TRUE;
	g_ctx->immediate.vert_len = 0;
	g_ctx->immediate.vert_pos = 0;
}

#define IMMEDIATE_VERT(pos) (&g_ctx->immediate.verts[(pos)])
#define IMMEDIATE_VERT_CUR() IMMEDIATE_VERT(g_ctx->immediate.vert_pos)
#define IMMEDIATE_VERT_OFF(off, mod) IMMEDIATE_VERT((g_ctx->immediate.vert_pos + (off)) % (mod))

void glEnd(void)
{
	if (!g_ctx->immediate.enabled)
	{
		g_ctx->errno = GL_INVALID_OPERATION;
		return;
	}
	if (g_ctx->immediate.mode == GL_LINE_LOOP)
	{
		if (g_ctx->immediate.vert_len > 2)
			rast_line(IMMEDIATE_VERT_OFF(1, 2), IMMEDIATE_VERT(3));
	}
	g_ctx->immediate.enabled = GL_FALSE;
}

static void points(void)
{
	rast_point(IMMEDIATE_VERT_CUR());
}

static void line_strip(void)
{
	if (!g_ctx->immediate.vert_len)
	{
		g_ctx->immediate.vert_len = 1;
		g_ctx->immediate.vert_pos = 1;
		return;
	}
	rast_line(IMMEDIATE_VERT_OFF(1, 2), IMMEDIATE_VERT_CUR());
	g_ctx->immediate.vert_pos++;
	g_ctx->immediate.vert_pos %= 2;
}

static void line_loop(void)
{
	if (!g_ctx->immediate.vert_len)
	{
		*IMMEDIATE_VERT(3) = *IMMEDIATE_VERT(0);
		g_ctx->immediate.vert_len = 1;
		g_ctx->immediate.vert_pos = 1;
		return;
	}
	rast_line(IMMEDIATE_VERT_OFF(1, 2), IMMEDIATE_VERT_CUR());
	g_ctx->immediate.vert_len++;
	g_ctx->immediate.vert_pos++;
	g_ctx->immediate.vert_pos %= 2;
}

static void lines(void)
{
	if (!g_ctx->immediate.vert_pos)
	{
		g_ctx->immediate.vert_pos = 1;
		return;
	}
	rast_line(IMMEDIATE_VERT(0), IMMEDIATE_VERT(1));
	g_ctx->immediate.vert_pos = 0;
}

static void triangle_strip(void)
{
	if (g_ctx->immediate.vert_len < 2)
	{
		g_ctx->immediate.vert_len++;
		g_ctx->immediate.vert_pos++;
		return;
	}
	rast_triangle(IMMEDIATE_VERT_OFF(2, 4),
	              IMMEDIATE_VERT_OFF(3, 4),
	              IMMEDIATE_VERT_CUR());
	g_ctx->immediate.vert_pos++;
	g_ctx->immediate.vert_pos %= 4;
}

static void triangle_fan(void)
{
	if (g_ctx->immediate.vert_len < 2)
	{
		if (!g_ctx->immediate.vert_len)
			*IMMEDIATE_VERT(4) = *IMMEDIATE_VERT(0);
		g_ctx->immediate.vert_len++;
		g_ctx->immediate.vert_pos++;
		return;
	}
	rast_triangle(IMMEDIATE_VERT(4),
	              IMMEDIATE_VERT_OFF(3, 4),
	              IMMEDIATE_VERT_CUR());
	g_ctx->immediate.vert_pos++;
	g_ctx->immediate.vert_pos %= 4;
}

static void triangles(void)
{
	if (g_ctx->immediate.vert_len < 2)
	{
		g_ctx->immediate.vert_len++;
		g_ctx->immediate.vert_pos++;
		g_ctx->immediate.vert_pos %= 4;
		return;
	}
	rast_triangle(IMMEDIATE_VERT_OFF(2, 4),
	              IMMEDIATE_VERT_OFF(3, 4),
	              IMMEDIATE_VERT_CUR());
	g_ctx->immediate.vert_len = 0;
	g_ctx->immediate.vert_pos++;
	g_ctx->immediate.vert_pos %= 4;
}

static void quads(void)
{
	if (g_ctx->immediate.vert_len < 3)
	{
		g_ctx->immediate.vert_len++;
		g_ctx->immediate.vert_pos++;
		g_ctx->immediate.vert_pos %= 4;
		return;
	}
	struct vert *v1 = IMMEDIATE_VERT_OFF(1, 4);
	struct vert *v2 = IMMEDIATE_VERT_OFF(2, 4);
	struct vert *v3 = IMMEDIATE_VERT_OFF(3, 4);
	struct vert *v4 = IMMEDIATE_VERT_CUR();
	rast_triangle(v1, v2, v3);
	rast_triangle(v3, v4, v1);
	g_ctx->immediate.vert_len = 0;
	g_ctx->immediate.vert_pos++;
	g_ctx->immediate.vert_pos %= 4;
}

static void quad_strip(void)
{
	if (g_ctx->immediate.vert_len < 3
	 || !(g_ctx->immediate.vert_pos & 1))
	{
		g_ctx->immediate.vert_len++;
		g_ctx->immediate.vert_pos++;
		g_ctx->immediate.vert_pos %= 4;
		return;
	}
	struct vert *v1 = IMMEDIATE_VERT_OFF(1, 4);
	struct vert *v2 = IMMEDIATE_VERT_OFF(2, 4);
	struct vert *v3 = IMMEDIATE_VERT_OFF(3, 4);
	struct vert *v4 = IMMEDIATE_VERT_CUR();
	if (g_ctx->immediate.vert_pos & 2)
	{
		rast_triangle(v1, v2, v3);
		rast_triangle(v3, v4, v1);
	}
	else
	{
		rast_triangle(v2, v1, v3);
		rast_triangle(v3, v4, v1);
	}
	g_ctx->immediate.vert_pos++;
	g_ctx->immediate.vert_pos %= 4;
}

static void glVertex(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	struct vert *vert = &g_ctx->immediate.verts[g_ctx->immediate.vert_pos];
	vert->attr[VERT_ATTR_X] = x;
	vert->attr[VERT_ATTR_Y] = y;
	vert->attr[VERT_ATTR_Z] = z;
	vert->attr[VERT_ATTR_W] = w;
	vert->attr[VERT_ATTR_R] = g_ctx->immediate.color[0];
	vert->attr[VERT_ATTR_G] = g_ctx->immediate.color[1];
	vert->attr[VERT_ATTR_B] = g_ctx->immediate.color[2];
	vert->attr[VERT_ATTR_A] = g_ctx->immediate.color[3];
	vert->attr[VERT_ATTR_S] = g_ctx->immediate.tex_coord[0];
	vert->attr[VERT_ATTR_T] = g_ctx->immediate.tex_coord[1];
	vert->attr[VERT_ATTR_P] = g_ctx->immediate.tex_coord[2];
	vert->attr[VERT_ATTR_Q] = g_ctx->immediate.tex_coord[3];
	vert->attr[VERT_ATTR_NX] = g_ctx->immediate.normal[0];
	vert->attr[VERT_ATTR_NY] = g_ctx->immediate.normal[1];
	vert->attr[VERT_ATTR_NZ] = g_ctx->immediate.normal[2];
	g_ctx->vs.fn(vert);
	rast_normalize_vert(vert);
	switch (g_ctx->immediate.mode)
	{
		case GL_POINTS:
			points();
			break;
		case GL_LINE_STRIP:
			line_strip();
			break;
		case GL_LINE_LOOP:
			line_loop();
			break;
		case GL_LINES:
			lines();
			break;
		case GL_TRIANGLE_STRIP:
			triangle_strip();
			break;
		case GL_TRIANGLE_FAN:
			triangle_fan();
			break;
		case GL_TRIANGLES:
			triangles();
			break;
		case GL_QUADS:
			quads();
			break;
		case GL_QUAD_STRIP:
			quad_strip();
			break;
	}
}

static void glNormal(GLfloat x, GLfloat y, GLfloat z)
{
	g_ctx->immediate.normal[0] = x;
	g_ctx->immediate.normal[1] = y;
	g_ctx->immediate.normal[2] = z;
}

static void glTexCoord(GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
	g_ctx->immediate.tex_coord[0] = s;
	g_ctx->immediate.tex_coord[1] = t;
	g_ctx->immediate.tex_coord[2] = r;
	g_ctx->immediate.tex_coord[3] = q;
}

static void glColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
	g_ctx->immediate.color[0] = r;
	g_ctx->immediate.color[1] = g;
	g_ctx->immediate.color[2] = b;
	g_ctx->immediate.color[3] = a;
}

void glVertex2s(GLshort x, GLshort y)
{
	glVertex(x, y, 0, 1);
}

void glVertex2i(GLint x, GLint y)
{
	glVertex(x, y, 0, 1);
}

void glVertex2f(GLfloat x, GLfloat y)
{
	glVertex(x, y, 0, 1);
}

void glVertex2d(GLdouble x, GLdouble y)
{
	glVertex(x, y, 0, 1);
}

void glVertex3s(GLshort x, GLshort y, GLshort z)
{
	glVertex(x, y, z, 1);
}

void glVertex3i(GLint x, GLint y, GLint z)
{
	glVertex(x, y, z, 1);
}

void glVertex3f(GLfloat x, GLfloat y, GLfloat z)
{
	glVertex(x, y, z, 1);
}

void glVertex3d(GLdouble x, GLdouble y, GLdouble z)
{
	glVertex(x, y, z, 1);
}

void glVertex4s(GLshort x, GLshort y, GLshort z, GLshort w)
{
	glVertex(x, y, z, w);
}

void glVertex4i(GLint x, GLint y, GLint z, GLint w)
{
	glVertex(x, y, z, w);
}

void glVertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	glVertex(x, y, z, w);
}

void glVertex4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	glVertex(x, y, z, w);
}

void glVertex2sv(const GLshort *v)
{
	glVertex(v[0], v[1], 0, 1);
}

void glVertex2iv(const GLint *v)
{
	glVertex(v[0], v[1], 0, 1);
}

void glVertex2fv(const GLfloat *v)
{
	glVertex(v[0], v[1], 0, 1);
}

void glVertex2dv(const GLdouble *v)
{
	glVertex(v[0], v[1], v[2], 1);
}

void glVertex3sv(const GLshort *v)
{
	glVertex(v[0], v[1], v[2], 1);
}

void glVertex3iv(const GLint *v)
{
	glVertex(v[0], v[1], v[2], 1);
}

void glVertex3fv(const GLfloat *v)
{
	glVertex(v[0], v[1], v[2], 1);
}

void glVertex3dv(const GLdouble *v)
{
	glVertex(v[0], v[1], v[2], 1);
}

void glVertex4sv(const GLshort *v)
{
	glVertex(v[0], v[1], v[2], v[3]);
}

void glVertex4iv(const GLint *v)
{
	glVertex(v[0], v[1], v[2], v[3]);
}

void glVertex4fv(const GLfloat *v)
{
	glVertex(v[0], v[1], v[2], v[3]);
}

void glVertex4dv(const GLdouble *v)
{
	glVertex(v[0], v[1], v[2], v[3]);
}

void glNormal3b(GLbyte x, GLbyte y, GLbyte z)
{
	glNormal(x / (GLfloat)CHAR_MAX,
	         y / (GLfloat)CHAR_MAX,
	         z / (GLfloat)CHAR_MAX);
}

void glNormal3s(GLshort x, GLshort y, GLshort z)
{
	glNormal(x / (GLfloat)SHRT_MAX,
	         y / (GLfloat)SHRT_MAX,
	         z / (GLfloat)SHRT_MAX);
}

void glNormal3i(GLint x, GLint y, GLint z)
{
	glNormal(x / (GLfloat)INT_MAX,
	         y / (GLfloat)INT_MAX,
	         z / (GLfloat)INT_MAX);
}

void glNormal3f(GLfloat x, GLfloat y, GLfloat z)
{
	glNormal(x, y, z);
}

void glNormal3d(GLdouble x, GLdouble y, GLdouble z)
{
	glNormal(x, y, z);
}

void glNormal3bv(const GLbyte *v)
{
	glNormal(v[0] / (GLfloat)CHAR_MAX,
	         v[1] / (GLfloat)CHAR_MAX,
	         v[2] / (GLfloat)CHAR_MAX);
}

void glNormal3sv(const GLshort *v)
{
	glNormal(v[0] / (GLfloat)SHRT_MAX,
	         v[1] / (GLfloat)SHRT_MAX,
	         v[2] / (GLfloat)SHRT_MAX);
}

void glNormal3iv(const GLint *v)
{
	glNormal(v[0] / (GLfloat)INT_MAX,
	         v[1] / (GLfloat)INT_MAX,
	         v[2] / (GLfloat)INT_MAX);
}

void glNormal3fv(const GLfloat *v)
{
	glNormal(v[0], v[1], v[2]);
}

void glNormal3dv(const GLdouble *v)
{
	glNormal(v[0], v[1], v[2]);
}

void glTexCoord1s(GLshort s)
{
	glTexCoord(s / (GLfloat)SHRT_MAX, 0, 0, 0);
}

void glTexCoord1i(GLint s)
{
	glTexCoord(s / (GLfloat)INT_MAX, 0, 0, 0);
}

void glTexCoord1f(GLfloat s)
{
	glTexCoord(s, 0, 0, 0);
}

void glTexCoord1d(GLdouble s)
{
	glTexCoord(s, 0, 0, 0);
}

void glTexCoord2s(GLshort s, GLshort t)
{
	glTexCoord(s / (GLfloat)SHRT_MAX,
	           t / (GLfloat)SHRT_MAX,
	           0,
	           0);
}

void glTexCoord2i(GLint s, GLint t)
{
	glTexCoord(s / (GLfloat)INT_MAX,
	           t / (GLfloat)INT_MAX,
	           0,
	           0);
}

void glTexCoord2f(GLfloat s, GLfloat t)
{
	glTexCoord(s, t, 0, 0);
}

void glTexCoord2d(GLdouble s, GLdouble t)
{
	glTexCoord(s, t, 0, 0);
}

void glTexCoord3s(GLshort s, GLshort t, GLshort r)
{
	glTexCoord(s / (GLfloat)SHRT_MAX,
	           t / (GLfloat)SHRT_MAX,
	           r / (GLfloat)SHRT_MAX,
	           0);
}

void glTexCoord3i(GLint s, GLint t, GLint r)
{
	glTexCoord(s / (GLfloat)INT_MAX,
	           t / (GLfloat)INT_MAX,
	           r / (GLfloat)INT_MAX,
	           0);
}

void glTexCoord3f(GLfloat s, GLfloat t, GLfloat r)
{
	glTexCoord(s, t, r, 0);
}

void glTexCoord3d(GLdouble s, GLdouble t, GLdouble r)
{
	glTexCoord(s, t, r, 0);
}

void glTexCoord4s(GLshort s, GLshort t, GLshort r, GLshort q)
{
	glTexCoord(s / (GLfloat)SHRT_MAX,
	           t / (GLfloat)SHRT_MAX,
	           r / (GLfloat)SHRT_MAX,
	           q / (GLfloat)SHRT_MAX);
}

void glTexCoord4i(GLint s, GLint t, GLint r, GLint q)
{
	glTexCoord(s / (GLfloat)INT_MAX,
	           t / (GLfloat)INT_MAX,
	           r / (GLfloat)INT_MAX,
	           q / (GLfloat)INT_MAX);
}

void glTexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
	glTexCoord(s, t, r, q);
}

void glTexCoord4d(GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
	glTexCoord(s, t, r, q);
}

void glTexCoord1sv(const GLshort *v)
{
	glTexCoord(v[0] / (GLfloat)SHRT_MAX, 0, 0, 0);
}

void glTexCoord1iv(const GLint *v)
{
	glTexCoord(v[0] / (GLfloat)INT_MAX, 0, 0, 0);
}

void glTexCoord1fv(const GLfloat *v)
{
	glTexCoord(v[0], 0, 0, 0);
}

void glTexCoord1dv(const GLdouble *v)
{
	glTexCoord(v[0], 0, 0, 0);
}

void glTexCoord2sv(const GLshort *v)
{
	glTexCoord(v[0] / (GLfloat)SHRT_MAX,
	           v[1] / (GLfloat)SHRT_MAX,
	           0,
	           0);
}

void glTexCoord2iv(const GLint *v)
{
	glTexCoord(v[0] / (GLfloat)INT_MAX,
	           v[1] / (GLfloat)INT_MAX,
	           0,
	           0);
}

void glTexCoord2fv(const GLfloat *v)
{
	glTexCoord(v[0], v[1], 0, 0);
}

void glTexCoord2dv(const GLdouble *v)
{
	glTexCoord(v[0], v[1], 0, 0);
}

void glTexCoord3sv(const GLshort *v)
{
	glTexCoord(v[0] / (GLfloat)SHRT_MAX,
	           v[1] / (GLfloat)SHRT_MAX,
	           v[2] / (GLfloat)SHRT_MAX,
	           0);
}

void glTexCoord3iv(const GLint *v)
{
	glTexCoord(v[0] / (GLfloat)INT_MAX,
	           v[1] / (GLfloat)INT_MAX,
	           v[2] / (GLfloat)INT_MAX,
	           0);
}

void glTexCoord3fv(const GLfloat *v)
{
	glTexCoord(v[0], v[1], v[2], 0);
}

void glTexCoord3dv(const GLdouble *v)
{
	glTexCoord(v[0], v[1], v[2], 0);
}

void glTexCoord4sv(const GLshort *v)
{
	glTexCoord(v[0] / (GLfloat)SHRT_MAX,
	           v[1] / (GLfloat)SHRT_MAX,
	           v[2] / (GLfloat)SHRT_MAX,
	           v[3] / (GLfloat)SHRT_MAX);
}

void glTexCoord4iv(const GLint *v)
{
	glTexCoord(v[0] / (GLfloat)INT_MAX,
	           v[1] / (GLfloat)INT_MAX,
	           v[2] / (GLfloat)INT_MAX,
	           v[3] / (GLfloat)INT_MAX);
}

void glTexCoord4fv(const GLfloat *v)
{
	glTexCoord(v[0], v[1], v[2], v[3]);
}

void glTexCoord4dv(const GLdouble *v)
{
	glTexCoord(v[0], v[1], v[2], v[3]);
}

void glColor3b(GLbyte r, GLbyte g, GLbyte b)
{
	glColor(r / (GLfloat)CHAR_MAX,
	        g / (GLfloat)CHAR_MAX,
	        b / (GLfloat)CHAR_MAX,
	        1);
}

void glColor3ub(GLubyte r, GLubyte g, GLubyte b)
{
	glColor(r / (GLfloat)UCHAR_MAX,
	        g / (GLfloat)UCHAR_MAX,
	        b / (GLfloat)UCHAR_MAX,
	        1);
}

void glColor3s(GLshort r, GLshort g, GLshort b)
{
	glColor(r / (GLfloat)SHRT_MAX,
	        g / (GLfloat)SHRT_MAX,
	        b / (GLfloat)SHRT_MAX,
	        1);
}

void glColor3us(GLushort r, GLushort g, GLushort b)
{
	glColor(r / (GLfloat)USHRT_MAX,
	        g / (GLfloat)USHRT_MAX,
	        b / (GLfloat)USHRT_MAX,
	        1);
}

void glColor3i(GLint r, GLint g, GLint b)
{
	glColor(r / (GLfloat)INT_MAX,
	        g / (GLfloat)INT_MAX,
	        b / (GLfloat)INT_MAX,
	        1);
}

void glColor3ui(GLuint r, GLuint g, GLuint b)
{
	glColor(r / (GLfloat)UINT_MAX,
	        g / (GLfloat)UINT_MAX,
	        b / (GLfloat)UINT_MAX,
	        1);
}

void glColor3f(GLfloat r, GLfloat g, GLfloat b)
{
	glColor(r, g, b, 1);
}

void glColor3d(GLdouble r, GLdouble g, GLdouble b)
{
	glColor(r, g, b, 1);
}

void glColor4b(GLbyte r, GLbyte g, GLbyte b, GLbyte a)
{
	glColor(r / (GLfloat)CHAR_MAX,
	        g / (GLfloat)CHAR_MAX,
	        b / (GLfloat)CHAR_MAX,
	        a / (GLfloat)CHAR_MAX);
}

void glColor4ub(GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{
	glColor(r / (GLfloat)UCHAR_MAX,
	        g / (GLfloat)UCHAR_MAX,
	        b / (GLfloat)UCHAR_MAX,
	        a / (GLfloat)UCHAR_MAX);
}

void glColor4s(GLshort r, GLshort g, GLshort b, GLshort a)
{
	glColor(r / (GLfloat)SHRT_MAX,
	        g / (GLfloat)SHRT_MAX,
	        b / (GLfloat)SHRT_MAX,
	        a / (GLfloat)SHRT_MAX);
}

void glColor4us(GLushort r, GLushort g, GLushort b, GLushort a)
{
	glColor(r / (GLfloat)USHRT_MAX,
	        g / (GLfloat)USHRT_MAX,
	        b / (GLfloat)USHRT_MAX,
	        a / (GLfloat)USHRT_MAX);
}

void glColor4i(GLint r, GLint g, GLint b, GLint a)
{
	glColor(r / (GLfloat)INT_MAX,
	        g / (GLfloat)INT_MAX,
	        b / (GLfloat)INT_MAX,
	        a / (GLfloat)INT_MAX);
}

void glColor4ui(GLuint r, GLuint g, GLuint b, GLuint a)
{
	glColor(r / (GLfloat)UINT_MAX,
	        g / (GLfloat)UINT_MAX,
	        b / (GLfloat)UINT_MAX,
	        a / (GLfloat)UINT_MAX);
}

void glColor4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
	glColor(r, g, b, a);
}

void glColor4d(GLdouble r, GLdouble g, GLdouble b, GLdouble a)
{
	glColor(r, g, b, a);
}

void glColor3bv(const GLbyte *v)
{
	glColor(v[0] / (GLfloat)CHAR_MAX,
	        v[1] / (GLfloat)CHAR_MAX,
	        v[2] / (GLfloat)CHAR_MAX,
	        1);
}

void glColor3ubv(const GLubyte *v)
{
	glColor(v[0] / (GLfloat)UCHAR_MAX,
	        v[1] / (GLfloat)UCHAR_MAX,
	        v[2] / (GLfloat)UCHAR_MAX,
	        1);
}

void glColor3sv(const GLshort *v)
{
	glColor(v[0] / (GLfloat)SHRT_MAX,
	        v[1] / (GLfloat)SHRT_MAX,
	        v[2] / (GLfloat)SHRT_MAX,
	        1);
}

void glColor3usv(const GLushort *v)
{
	glColor(v[0] / (GLfloat)USHRT_MAX,
	        v[1] / (GLfloat)USHRT_MAX,
	        v[2] / (GLfloat)USHRT_MAX,
	        1);
}

void glColor3iv(const GLint *v)
{
	glColor(v[0] / (GLfloat)INT_MAX,
	        v[1] / (GLfloat)INT_MAX,
	        v[2] / (GLfloat)INT_MAX,
	        1);
}

void glColor3uiv(const GLuint *v)
{
	glColor(v[0] / (GLfloat)UINT_MAX,
	        v[1] / (GLfloat)UINT_MAX,
	        v[2] / (GLfloat)UINT_MAX,
	        1);
}

void glColor3fv(const GLfloat *v)
{
	glColor(v[0], v[1], v[2], 1);
}

void glColor3dv(const GLdouble *v)
{
	glColor(v[0], v[1], v[2], 1);
}

void glColor4bv(const GLbyte *v)
{
	glColor(v[0] / (GLfloat)CHAR_MAX,
	        v[1] / (GLfloat)CHAR_MAX,
	        v[2] / (GLfloat)CHAR_MAX,
	        v[3] / (GLfloat)CHAR_MAX);
}

void glColor4ubv(const GLubyte *v)
{
	glColor(v[0] / (GLfloat)UCHAR_MAX,
	        v[1] / (GLfloat)UCHAR_MAX,
	        v[2] / (GLfloat)UCHAR_MAX,
	        v[3] / (GLfloat)UCHAR_MAX);
}

void glColor4sv(const GLshort *v)
{
	glColor(v[0] / (GLfloat)SHRT_MAX,
	        v[1] / (GLfloat)SHRT_MAX,
	        v[2] / (GLfloat)SHRT_MAX,
	        v[3] / (GLfloat)SHRT_MAX);
}

void glColor4usv(const GLushort *v)
{
	glColor(v[0] / (GLfloat)USHRT_MAX,
	        v[1] / (GLfloat)USHRT_MAX,
	        v[2] / (GLfloat)USHRT_MAX,
	        v[3] / (GLfloat)USHRT_MAX);
}

void glColor4iv(const GLint *v)
{
	glColor(v[0] / (GLfloat)INT_MAX,
	        v[1] / (GLfloat)INT_MAX,
	        v[2] / (GLfloat)INT_MAX,
	        v[3] / (GLfloat)INT_MAX);
}

void glColor4uiv(const GLuint *v)
{
	glColor(v[0] / (GLfloat)UINT_MAX,
	        v[1] / (GLfloat)UINT_MAX,
	        v[2] / (GLfloat)UINT_MAX,
	        v[3] / (GLfloat)UINT_MAX);
}

void glColor4fv(const GLfloat *v)
{
	glColor(v[0], v[1], v[2], v[3]);
}

void glColor4dv(const GLdouble *v)
{
	glColor(v[0], v[1], v[2], v[3]);
}
