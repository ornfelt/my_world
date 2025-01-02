#ifndef MAT4_H
#define MAT4_H

struct vert;

struct mat4
{
	float v[16];
};

void mat4_clear(struct mat4 *mat);
void mat4_init_scale(struct mat4 *mat, float x, float y, float z);
void mat4_init_identity(struct mat4 *mat);
void mat4_init_projection(struct mat4 *mat, float sfov, float ratio, float ranges[2]);
void mat4_init_translation(struct mat4 *mat, float x, float y, float z);
void mat4_init_rotation_x(struct mat4 *mat, float angle);
void mat4_init_rotation_y(struct mat4 *mat, float angle);
void mat4_init_rotation_z(struct mat4 *mat, float angle);
void mat4_dump(struct mat4 *mat);
void mat4_mult(struct mat4 *dst, struct mat4 *m1, struct mat4 *m2);
void mat4_transform_vec4(struct mat4 *mat, float *v);
void mat4_reverse(struct mat4 *mat);

#endif
