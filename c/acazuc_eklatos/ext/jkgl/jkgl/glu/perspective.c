#include "internal.h"

#include <glu.h>

void gluPerspective(GLdouble fovy, GLdouble aspect, GLdouble z_near,
                    GLdouble z_far)
{
	struct mat4 perspective;
	GLfloat ranges[2];

	if (g_ctx->immediate.enabled)
	{
		g_ctx->errno = GL_INVALID_OPERATION;
		return;
	}
	ranges[0] = z_near;
	ranges[1] = z_far;
	mat4_init_projection(&perspective, fovy, aspect, ranges);
	glMultMatrixf(perspective.v);
}
