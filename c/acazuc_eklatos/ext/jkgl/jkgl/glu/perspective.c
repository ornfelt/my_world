#define __BSD_VISIBLE 1
#include <math.h>
#include "glu.h"

void
gluPerspective(GLdouble fovy,
               GLdouble aspect,
               GLdouble z_near,
               GLdouble z_far)
{
	GLfloat mat[16];
	GLfloat fov;

	fov = 1.0f / tanf((fovy / 2.0f) / 360.0f * (float)M_PI * 2.0f);
	mat[0] = fov / aspect;
	mat[1] = 0;
	mat[2] = 0;
	mat[3] = 0;
	mat[4] = 0;
	mat[5] = fov;
	mat[6] = 0;
	mat[7] = 0;
	mat[8] = 0;
	mat[9] = 0;
	mat[10] = (z_near + z_far) / (z_near - z_far);
	mat[11] = -1;
	mat[12] = 0;
	mat[13] = 0;
	mat[14] = (2 * z_near * z_far) / (z_near - z_far);
	mat[15] = 0;
	glMultMatrixf(mat);
}
