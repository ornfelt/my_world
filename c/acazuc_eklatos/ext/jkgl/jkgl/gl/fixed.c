#include "internal.h"
#include "fixed.h"

#include <stddef.h>
#include <assert.h>
#include <math.h>

static GLfloat fog_factor(GLfloat z)
{
	switch (g_ctx->fog_mode)
	{
		/* XXX this is all wrong, figure out why */
		case GL_LINEAR:
			if (g_ctx->fog_end - g_ctx->fog_start < 0.00001f)
				return 0;
			return (-z - g_ctx->fog_start) / (g_ctx->fog_end - g_ctx->fog_start);
		case GL_EXP:
			return 1.f - expf(g_ctx->fog_density * z);
		case GL_EXP2:
		{
			float t = g_ctx->fog_density * z;
			return 1.f - expf(t * t);
		}
		default:
			assert(!"unknown fog mode");
			return 0;
	}
}

static void light_colors(const struct vert *vert, const struct light *light,
                         const GLfloat *modelview_pos, const GLfloat *normal,
                         GLfloat *ambient, GLfloat *diffuse, GLfloat *specular)
{
	struct material *material = &g_ctx->materials[!!vert->front_face];
	GLfloat att = 1;
	GLfloat dir[3];
	GLfloat diffuse_factor;
	GLfloat specular_factor;
	if (light->position[3] != 0)
	{
		dir[0] = modelview_pos[0] - light->position[0];
		dir[1] = modelview_pos[1] - light->position[1];
		dir[2] = modelview_pos[2] - light->position[2];
		GLfloat d = sqrtf(dir[0] * dir[0] + dir[1] * dir[1] + dir[2] * dir[2]);
		GLfloat tmp = light->attenuations[1] + light->attenuations[2] * d;
		tmp = light->attenuations[0] + d * tmp;
		if (tmp > 0.00001f)
		{
			att /= tmp;
			if (att < 0.00001f)
				return;
		}
		d = -1.f / d;
		dir[0] *= d;
		dir[1] *= d;
		dir[2] *= d;
	}
	else
	{
		dir[0] = light->position[0];
		dir[1] = light->position[1];
		dir[2] = light->position[2];
		GLfloat d = 1.f / sqrtf(dir[0] * dir[0]
		                      + dir[1] * dir[1]
		                      + dir[2] * dir[2]);
		dir[0] *= d;
		dir[1] *= d;
		dir[2] *= d;
	}
	diffuse_factor = clampf(normal[0] * dir[0]
	                      + normal[1] * dir[1]
	                      + normal[2] * dir[2], 0, 1);
	GLfloat reflect[3];
	GLfloat d = normal[0] * -dir[0]
	          + normal[1] * -dir[1]
	          + normal[2] * -dir[2];
	if (d > 0)
	{
		specular_factor = 0;
	}
	else
	{
		reflect[0] = -dir[0] - 2 * normal[0] * d;
		reflect[1] = -dir[1] - 2 * normal[1] * d;
		reflect[2] = -dir[2] - 2 * normal[2] * d;
		d = 1.f / sqrtf(reflect[0] * reflect[0]
		              + reflect[1] * reflect[1]
		              + reflect[2] * reflect[2]);
		reflect[0] *= d;
		reflect[1] *= d;
		reflect[2] *= d;
		GLfloat pos[3];
		pos[0] = modelview_pos[0];
		pos[1] = modelview_pos[1];
		pos[2] = modelview_pos[2];
		d = -1.f / sqrtf(pos[0] * pos[0]
		               + pos[1] * pos[1]
		               + pos[2] * pos[2]);
		pos[0] *= d;
		pos[1] *= d;
		pos[2] *= d;
		specular_factor = clampf(pos[0] * reflect[0]
		                       + pos[1] * reflect[1]
		                       + pos[2] * reflect[2], 0, 1);
		specular_factor = powf(specular_factor, material->shininess);
	}
	GLfloat diffuse_att = att * diffuse_factor;
	GLfloat specular_att = att * specular_factor;
	ambient[0] += light->ambient[0] * att;
	ambient[1] += light->ambient[1] * att;
	ambient[2] += light->ambient[2] * att;
	diffuse[0] += light->diffuse[0] * diffuse_att;
	diffuse[1] += light->diffuse[1] * diffuse_att;
	diffuse[2] += light->diffuse[2] * diffuse_att;
	specular[0] += light->specular[0] * specular_att;
	specular[1] += light->specular[1] * specular_att;
	specular[2] += light->specular[2] * specular_att;
}

static void lights_colors(const struct vert *vert, const GLfloat *modelview_pos,
                          const GLfloat *normal, GLfloat *ambient,
                          GLfloat *diffuse, GLfloat *specular)
{
	for (GLuint i = 0; i < sizeof(g_ctx->lights) / sizeof(*g_ctx->lights); ++i)
	{
		const struct light *light = &g_ctx->lights[i];
		if (!light->enabled)
			continue;
		light_colors(vert, light, modelview_pos, normal, ambient, diffuse, specular);
	}
}

GLboolean fixed_fragment_shader(const struct vert *vert, GLfloat *color)
{
	color[0] = vert->varying[VERT_VARYING_R];
	color[1] = vert->varying[VERT_VARYING_G];
	color[2] = vert->varying[VERT_VARYING_B];
	color[3] = vert->varying[VERT_VARYING_A];
	if (g_ctx->texture2d)
	{
		const struct texture *texture;
		if (g_ctx->texture_binding_2d > 0 && g_ctx->texture_binding_2d < g_ctx->textures_capacity)
			texture = g_ctx->textures[g_ctx->texture_binding_2d];
		else
			texture = NULL;
		if (texture)
		{
			GLfloat texture_color[4];
			rast_texture_sample(texture, &vert->varying[VERT_VARYING_S], texture_color);
			color[0] *= texture_color[0];
			color[1] *= texture_color[1];
			color[2] *= texture_color[2];
			color[3] *= texture_color[3];
		}
	}
	if (g_ctx->fog)
	{
		color[0] += vert->varying[VERT_VARYING_FOG] * (g_ctx->fog_color[0] - color[0]);
		color[1] += vert->varying[VERT_VARYING_FOG] * (g_ctx->fog_color[1] - color[1]);
		color[2] += vert->varying[VERT_VARYING_FOG] * (g_ctx->fog_color[2] - color[2]);
	}
	return GL_FALSE;
}

void fixed_vertex_shader(struct vert *vert)
{
	vert->x = vert->attr[VERT_ATTR_X];
	vert->y = vert->attr[VERT_ATTR_Y];
	vert->z = vert->attr[VERT_ATTR_Z];
	vert->w = vert->attr[VERT_ATTR_W];
	mat4_transform_vec4(&g_ctx->modelview_matrix[g_ctx->modelview_stack_depth],
	                    &vert->x);
	GLfloat modelview_pos[3] = {vert->x, vert->y, vert->z};
	mat4_transform_vec4(&g_ctx->projection_matrix[g_ctx->projection_stack_depth],
	                    &vert->x);
	vert->varying[VERT_VARYING_R] = vert->attr[VERT_ATTR_R];
	vert->varying[VERT_VARYING_G] = vert->attr[VERT_ATTR_G];
	vert->varying[VERT_VARYING_B] = vert->attr[VERT_ATTR_B];
	vert->varying[VERT_VARYING_A] = vert->attr[VERT_ATTR_A];
	vert->varying[VERT_VARYING_S] = vert->attr[VERT_ATTR_S];
	vert->varying[VERT_VARYING_T] = vert->attr[VERT_ATTR_T];
	vert->varying[VERT_VARYING_P] = vert->attr[VERT_ATTR_P];
	vert->varying[VERT_VARYING_Q] = vert->attr[VERT_ATTR_Q];
	if (g_ctx->lighting)
	{
		GLfloat normal[4];
		normal[0] = vert->attr[VERT_ATTR_NX];
		normal[1] = vert->attr[VERT_ATTR_NY];
		normal[2] = vert->attr[VERT_ATTR_NZ];
		normal[3] = 0;
		mat4_transform_vec4(&g_ctx->modelview_matrix[g_ctx->modelview_stack_depth],
		                    normal);
		GLfloat tmp = 1.f / sqrtf(normal[0] * normal[0]
		                        + normal[1] * normal[1]
		                        + normal[2] * normal[2]);
		normal[0] *= tmp;
		normal[1] *= tmp;
		normal[2] *= tmp;
		GLfloat ambient[3] = {0};
		GLfloat diffuse[3] = {0};
		GLfloat specular[3] = {0};
		lights_colors(vert, modelview_pos, normal, ambient, diffuse, specular);
		const struct material *material = &g_ctx->materials[!!vert->front_face];
		diffuse[0] = material->emission[0] + diffuse[0] * material->diffuse[0];
		diffuse[1] = material->emission[1] + diffuse[1] * material->diffuse[1];
		diffuse[2] = material->emission[2] + diffuse[2] * material->diffuse[2];
		vert->varying[VERT_VARYING_R] *= ambient[0] * material->ambient[0] + diffuse[0];
		vert->varying[VERT_VARYING_R] += specular[0] * material->specular[0];
		vert->varying[VERT_VARYING_G] *= ambient[1] * material->ambient[1] + diffuse[1];
		vert->varying[VERT_VARYING_G] += specular[1] * material->specular[1];
		vert->varying[VERT_VARYING_B] *= ambient[2] * material->ambient[2] + diffuse[2];
		vert->varying[VERT_VARYING_B] += specular[2] * material->specular[2];
	}
	if (g_ctx->fog)
		vert->varying[VERT_VARYING_FOG] = clampf(fog_factor(modelview_pos[2]), 0, 1);
}
