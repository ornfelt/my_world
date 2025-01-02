#include "internal.h"

#define __BSD_VISIBLE 1
#include <string.h>
#include <stdio.h>
#include <math.h>

void mat4_clear(struct mat4 *mat)
{
	for (int i = 0; i < 16; ++i)
		mat->v[i] = 0;
}

void mat4_dump(struct mat4 *mat)
{
	for (int y = 0; y < 4; ++y)
	{
		for (int x = 0; x  < 4; ++x)
			printf("%4.3f ", mat->v[y * 4 + x]);
		putchar('\n');
	}
}

void mat4_init_identity(struct mat4 *mat)
{
	mat4_clear(mat);
	mat->v[0] = 1;
	mat->v[5] = 1;
	mat->v[10] = 1;
	mat->v[15] = 1;
}

void mat4_init_projection(struct mat4 *mat, float sfov, float ratio,
                          float ranges[2])
{
	float fov;

	mat4_clear(mat);
	fov = 1.f / tanf((sfov / 2.f) / 360.f * (float)M_PI * 2.f);
	mat->v[0] = fov / ratio;
	mat->v[5] = fov;
	mat->v[10] = (ranges[0] + ranges[1]) / (ranges[0] - ranges[1]);
	mat->v[11] = -1;
	mat->v[14] = (2 * ranges[0] * ranges[1]) / (ranges[0] - ranges[1]);
}

void mat4_init_rotation_x(struct mat4 *mat, float angle)
{
	mat4_clear(mat);
	mat->v[0] = 1;
	mat->v[5] = cosf(angle);
	mat->v[9] = -sinf(angle);
	mat->v[6] = sinf(angle);
	mat->v[10] = cosf(angle);
	mat->v[15] = 1;
}

void mat4_init_rotation_y(struct mat4 *mat, float angle)
{
	mat4_clear(mat);
	mat->v[0] = cosf(angle);
	mat->v[8] = sinf(angle);
	mat->v[5] = 1;
	mat->v[2] = -sinf(angle);
	mat->v[10] = cosf(angle);
	mat->v[15] = 1;
}

void mat4_init_rotation_z(struct mat4 *mat, float angle)
{
	mat4_clear(mat);
	mat->v[0] = cosf(angle);
	mat->v[4] = -sinf(angle);
	mat->v[1] = sinf(angle);
	mat->v[5] = cosf(angle);
	mat->v[10] = 1;
	mat->v[15] = 1;
}

void mat4_init_scale(struct mat4 *mat, float x, float y, float z)
{
	mat4_clear(mat);
	mat->v[0] = x;
	mat->v[5] = y;
	mat->v[10] = z;
	mat->v[15] = 1;
}

void mat4_init_translation(struct mat4 *mat, float x, float y, float z)
{
	mat4_init_identity(mat);
	mat->v[12] = x;
	mat->v[13] = y;
	mat->v[14] = z;
}

static float priv_patch(struct mat4 *m1, struct mat4 *m2, int x, int y)
{
	return (m1->v[y] * m2->v[x * 4]
	      + m1->v[y + 4] * m2->v[1 + x * 4]
	      + m1->v[y + 8] * m2->v[2 + x * 4]
	      + m1->v[y + 12] * m2->v[3 + x * 4]);
}

void mat4_mult(struct mat4 *dst, struct mat4 *m1, struct mat4 *m2)
{
	struct mat4 tmp;

	for (int y = 0; y < 4; ++y)
	{
		for (int x = 0; x < 4; ++x)
			tmp.v[y + x * 4] = priv_patch(m1, m2, x, y);
	}
	memcpy(dst, &tmp, sizeof(tmp));
}

void mat4_reverse(struct mat4 *mat)
{
	struct mat4 new;

	for (int y = 0; y < 4; ++y)
	{
		for (int x = 0; x < 4; ++x)
			new.v[y + x * 4] = mat->v[y + x * 4];
	}
	*mat = new;
}

void mat4_transform_vec4(struct mat4 *mat, float *v)
{
	float x;
	float y;
	float z;
	float w;

	x = v[0] * mat->v[0]
	  + v[1] * mat->v[4]
	  + v[2] * mat->v[8]
	  + v[3] * mat->v[12];
	y = v[0] * mat->v[1]
	  + v[1] * mat->v[5]
	  + v[2] * mat->v[9]
	  + v[3] * mat->v[13];
	z = v[0] * mat->v[2]
	  + v[1] * mat->v[6]
	  + v[2] * mat->v[10]
	  + v[3] * mat->v[14];
	w = v[0] * mat->v[3]
	  + v[1] * mat->v[7]
	  + v[2] * mat->v[11]
	  + v[3] * mat->v[15];
	v[0] = x;
	v[1] = y;
	v[2] = z;
	v[3] = w;
}
