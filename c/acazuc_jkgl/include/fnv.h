#ifndef FNV_H
#define FNV_H

#include "gl.h"

#define FNV_PRIME32 0x01000193U
#define FNV_BASIS32 0x811C9DC5U

static inline GLuint fnv32(GLuint h, GLubyte v)
{
	return (h ^ v) * FNV_PRIME32;
}

static inline GLuint fnv32v(GLuint h, const GLvoid *d, GLsizei n)
{
	for (GLsizei i = 0; i < n; ++i)
		h = fnv32(h, ((const GLubyte*)d)[i]);
	return h;
}

#endif
