#include "internal.h"
#include "fixed.h"

#include <assert.h>
#include <math.h>

static GLint wrap_clamp_to_border(GLint v, GLuint max)
{
	if (v < 0)
		return 0;
	if ((GLuint)v >= max)
		return max - 1;
	return v;
}

static GLint wrap_clamp_to_edge(GLint v, GLuint max)
{
	if (v < 0)
		return 0;
	if ((GLuint)v >= max)
		return max - 1;
	return v;
}

static GLint wrap_repeat(GLint v, GLuint max)
{
	v %= (GLint)max;
	if (v < 0)
		return v + max;
	return v;
}

static GLint wrap_mirrored_repeat(GLint v, GLuint max)
{
	GLint w2 = max * 2;
	v %= w2;
	if (v < 0)
		v += w2;
	if ((GLuint)v >= max)
		return max - 1 - (v - max);
	return v;
}

static GLint texture_wrap(GLenum wrap, GLint v, GLuint max)
{
	switch (wrap)
	{
		case GL_CLAMP:
		case GL_CLAMP_TO_BORDER:
			return wrap_clamp_to_border(v, max);
		case GL_CLAMP_TO_EDGE:
			return wrap_clamp_to_edge(v, max);
		case GL_REPEAT:
			return wrap_repeat(v, max);
		case GL_MIRRORED_REPEAT:
			return wrap_mirrored_repeat(v, max);
		default:
			return 0;
	}
}

static void fetch_2d(const struct texture *texture, GLint x, GLint y, GLfloat *color)
{
	x = texture_wrap(texture->wrap_s, x, texture->width);
	y = texture_wrap(texture->wrap_t, y, texture->height);
	switch (texture->format)
	{
		case GL_ALPHA4:
		{
			GLubyte *data = texture->data;
			data = &data[(x + y * texture->width) / 2];
			color[0] = 0;
			color[1] = 0;
			color[2] = 0;
			color[3] = ((x & 1) ? (data[0] >> 4) : (data[0] & 0xf)) / 15.f;
			break;
		}
		case GL_ALPHA8:
		{
			GLubyte *data = texture->data;
			data = &data[x + y * texture->width];
			color[0] = 0;
			color[1] = 0;
			color[2] = 0;
			color[3] = data[0] / 255.f;
			break;
		}
		case GL_ALPHA16:
			break;
		case GL_R3_G3_B2:
			break;
		case GL_RGB4:
			break;
		case GL_RGB5:
			break;
		case GL_RGB8:
		{
			GLubyte *data = texture->data;
			data = &data[(x + y * texture->width) * 4];
			color[0] = data[0] / 255.f;
			color[1] = data[1] / 255.f;
			color[2] = data[2] / 255.f;
			color[3] = 1;
			break;
		}
		case GL_RGB10:
			break;
		case GL_RGB12:
			break;
		case GL_RGB16:
			break;
		case GL_RGBA2:
			break;
		case GL_RGBA4:
			break;
		case GL_RGB5_A1:
			break;
		case GL_RGBA8:
		{
			GLubyte *data = texture->data;
			data = &data[(x + y * texture->width) * 4];
			color[0] = data[0] / 255.f;
			color[1] = data[1] / 255.f;
			color[2] = data[2] / 255.f;
			color[3] = data[3] / 255.f;
			break;
		}
		case GL_RGB10_A2:
			break;
		case GL_RGBA12:
			break;
		case GL_RGBA16:
			break;
		case GL_SRGB8:
			break;
		case GL_SRGB8_ALPHA8:
			break;
		default:
			assert(!"unknown texture format");
			return;
	}
}

static void fetch(const struct texture *texture, GLint x, GLint y, GLfloat *color)
{
#ifdef ENABLE_GCCJIT
	return texture->texture_fetch_state->fn(color, texture->data, x, y, 0, 0);
#else
	switch (texture->target)
	{
		case GL_TEXTURE_2D:
			fetch_2d(texture, x, y, color);
			break;
		default:
			assert(!"unknown texture target");
			return;
	}
#endif
}

static void cubic_interpolate(GLfloat *dst, GLfloat *c1, GLfloat *c2, GLfloat *c3, GLfloat *c4, GLfloat f)
{
	for (GLint i = 0; i < 4; ++i)
		dst[i] = c2[i] + (c3[i] - c1[i] + (c1[i] * 2 - c2[i] * 5 + c3[i] * 4 - c4[i] + ((c2[i] - c3[i]) * 3 + c4[i] - c1[i]) * f) * f) * f * .5f;
}

static void filter_cubic(const struct texture *texture, GLfloat x, GLfloat y, GLfloat *color)
{
	x -= .5f;
	y -= .5f;
	GLfloat fx = fmodf(x, 1);
	GLfloat fy = fmodf(y, 1);
	GLint bx = x;
	GLint by = y;
	if (fx < 0)
	{
		bx--;
		fx++;
	}
	if (fy < 0)
	{
		by--;
		fy++;
	}
	GLfloat t[16];
	for (GLint i = 0; i < 4; ++i)
	{
		GLfloat c1[4];
		GLfloat c2[4];
		GLfloat c3[4];
		GLfloat c4[4];
		GLint ty = by - 1 + i;
		fetch(texture, bx - 1, ty, c1);
		fetch(texture, bx + 0, ty, c2);
		fetch(texture, bx + 1, ty, c3);
		fetch(texture, bx + 2, ty, c4);
		cubic_interpolate(&t[i * 4], c1, c2, c3, c4, fx);
	}
	cubic_interpolate(color, &t[0], &t[4], &t[8], &t[12], fy);
}

static void linear_interpolate(GLfloat *dst, GLfloat *c1, GLfloat *c2, GLfloat f)
{
	for (GLint i = 0; i < 4; ++i)
		dst[i] = mixf(c1[i], c2[i], f);
}

static void filter_linear(const struct texture *texture, GLfloat x, GLfloat y, GLfloat *color)
{
	x -= .5f;
	y -= .5f;
	GLfloat fx = fmodf(x, 1);
	GLfloat fy = fmodf(y, 1);
	GLint bx = x;
	GLint by = y;
	if (fx < 0)
	{
		bx--;
		fx++;
	}
	if (fy < 0)
	{
		by--;
		fy++;
	}
	GLfloat c1[4];
	GLfloat c2[4];
	GLfloat c3[4];
	GLfloat c4[4];
	GLfloat t1[4];
	GLfloat t2[4];
	fetch(texture, bx + 0, by + 0, c1);
	fetch(texture, bx + 1, by + 0, c2);
	fetch(texture, bx + 0, by + 1, c3);
	fetch(texture, bx + 1, by + 1, c4);
	linear_interpolate(t1, c1, c2, fx);
	linear_interpolate(t2, c3, c4, fx);
	linear_interpolate(color, t1, t2, fy);
}

static void filter_nearest(const struct texture *texture, GLfloat x, GLfloat y, GLfloat *color)
{
	GLint bx = floorf(x);
	GLint by = floorf(y);
	fetch(texture, bx, by, color);
}

static void texture_filter(const struct texture *texture, GLfloat x, GLfloat y, GLfloat *color)
{
	switch (texture->mag_filter)
	{
		case GL_CUBIC:
			filter_cubic(texture, x, y, color);
			break;
		case GL_LINEAR:
			filter_linear(texture, x, y, color);
			break;
		case GL_NEAREST:
			filter_nearest(texture, x, y, color);
			break;
		default:
			assert(!"unknown texture mag filter");
			return;
	}
}

void rast_texture_sample(const struct texture *texture, const GLfloat *coord,
                        GLfloat *color)
{
	if (!texture)
	{
		color[0] = 1;
		color[1] = 1;
		color[2] = 1;
		color[3] = 1;
		return;
	}
	texture_filter(texture, coord[0] * texture->width,
	                coord[1] * texture->height, color);
}
