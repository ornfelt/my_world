#include "internal.h"

void
mat4_transform_vec4(const float * restrict mat, float * restrict v)
{
	float x;
	float y;
	float z;
	float w;

	x = v[0] * mat[0]
	  + v[1] * mat[4]
	  + v[2] * mat[8]
	  + v[3] * mat[12];
	y = v[0] * mat[1]
	  + v[1] * mat[5]
	  + v[2] * mat[9]
	  + v[3] * mat[13];
	z = v[0] * mat[2]
	  + v[1] * mat[6]
	  + v[2] * mat[10]
	  + v[3] * mat[14];
	w = v[0] * mat[3]
	  + v[1] * mat[7]
	  + v[2] * mat[11]
	  + v[3] * mat[15];
	v[0] = x;
	v[1] = y;
	v[2] = z;
	v[3] = w;
}
