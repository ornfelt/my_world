#include "rast/rast.h"
#include "internal.h"
#include "fnv.h"

#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>

enum fixed_attr
{
	FIXED_ATTR_POSITION,
	FIXED_ATTR_COLOR,
	FIXED_ATTR_NORMAL,
	FIXED_ATTR_TEXCOORD,
};

enum fixed_varying
{
	FIXED_VARYING_COLOR,
	FIXED_VARYING_TEXCOORD,
	FIXED_VARYING_FOG,
};

static GLfloat
fog_factor(const struct fixed_block *block, GLfloat z)
{
	switch (block->fog_mode)
	{
		/* XXX this is all wrong, figure out why */
		case GL_LINEAR:
			if (block->fog_end - block->fog_start < 0.00001f)
				return 0;
			return (-z - block->fog_start) / (block->fog_end - block->fog_start);
		case GL_EXP:
			return 1.0f - expf(block->fog_density * z);
		case GL_EXP2:
		{
			float t = block->fog_density * z;
			return 1.0f - expf(t * t);
		}
		default:
			assert(!"unknown fog mode");
			return 0;
	}
}

static void
light_colors(const struct fixed_block *block,
             const struct rast_vert *vert,
             const struct light *light,
             const GLfloat *modelview_pos,
             const GLfloat *normal,
             GLfloat *ambient,
             GLfloat *diffuse,
             GLfloat *specular)
{
	const struct material *material = &block->materials[!!vert->front_face];
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
		d = -1.0f / d;
		dir[0] *= d;
		dir[1] *= d;
		dir[2] *= d;
	}
	else
	{
		dir[0] = light->position[0];
		dir[1] = light->position[1];
		dir[2] = light->position[2];
		GLfloat d = 1.0f / sqrtf(dir[0] * dir[0]
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
		d = 1.0f / sqrtf(reflect[0] * reflect[0]
		               + reflect[1] * reflect[1]
		               + reflect[2] * reflect[2]);
		reflect[0] *= d;
		reflect[1] *= d;
		reflect[2] *= d;
		GLfloat pos[3];
		pos[0] = modelview_pos[0];
		pos[1] = modelview_pos[1];
		pos[2] = modelview_pos[2];
		d = -1.0f / sqrtf(pos[0] * pos[0]
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

static void
lights_colors(const struct fixed_block *block,
              const struct rast_vert *vert,
              const GLfloat *modelview_pos,
              const GLfloat *normal,
              GLfloat *ambient,
              GLfloat *diffuse,
              GLfloat *specular)
{
	for (GLuint i = 0; i < sizeof(block->lights) / sizeof(*block->lights); ++i)
	{
		const struct light *light = &block->lights[i];
		if (!light->enable)
			continue;
		light_colors(block,
		             vert,
		             light,
		             modelview_pos,
		             normal,
		             ambient,
		             diffuse,
		             specular);
	}
}

static bool
fixed_fragment_shader(const struct rast_vert *vert,
                      const void **uniform_blocks,
                      float * restrict color)
{
	const struct fixed_block *block = uniform_blocks[0];

	color[0] = vert->varying[FIXED_VARYING_COLOR].x;
	color[1] = vert->varying[FIXED_VARYING_COLOR].y;
	color[2] = vert->varying[FIXED_VARYING_COLOR].z;
	color[3] = vert->varying[FIXED_VARYING_COLOR].w;
	if (block->texture_2d_enable)
	{
		GLfloat texture_color[4];
		gl_sample(0, &vert->varying[FIXED_VARYING_TEXCOORD].x, texture_color);
		color[0] *= texture_color[0];
		color[1] *= texture_color[1];
		color[2] *= texture_color[2];
		color[3] *= texture_color[3];
	}
	if (block->fog_enable)
	{
		color[0] = mixf(color[0], block->fog_color[0], vert->varying[FIXED_VARYING_FOG].x);
		color[1] = mixf(color[1], block->fog_color[1], vert->varying[FIXED_VARYING_FOG].x);
		color[2] = mixf(color[2], block->fog_color[2], vert->varying[FIXED_VARYING_FOG].x);
	}
	return false;
}

static float
dp4(const float * restrict a, const float * restrict b)
{
	return a[0] * b[0]
	     + a[1] * b[1]
	     + a[2] * b[2]
	     + a[3] * b[3];
}

static void
fixed_vertex_shader(struct rast_vert *vert,
                    const void **uniform_blocks)
{
	const struct fixed_block *block = uniform_blocks[0];
	GLfloat modelview_pos[4];
	GLfloat projection_pos[4];

	modelview_pos[0] = dp4(&vert->attribs[FIXED_ATTR_POSITION].x, &block->modelview_matrix[0x0]);
	modelview_pos[1] = dp4(&vert->attribs[FIXED_ATTR_POSITION].x, &block->modelview_matrix[0x4]);
	modelview_pos[2] = dp4(&vert->attribs[FIXED_ATTR_POSITION].x, &block->modelview_matrix[0x8]);
	modelview_pos[3] = dp4(&vert->attribs[FIXED_ATTR_POSITION].x, &block->modelview_matrix[0xC]);
	projection_pos[0] = dp4(modelview_pos, &block->projection_matrix[0x0]);
	projection_pos[1] = dp4(modelview_pos, &block->projection_matrix[0x4]);
	projection_pos[2] = dp4(modelview_pos, &block->projection_matrix[0x8]);
	projection_pos[3] = dp4(modelview_pos, &block->projection_matrix[0xC]);
	vert->x = projection_pos[0];
	vert->y = projection_pos[1];
	vert->z = projection_pos[2];
	vert->w = projection_pos[3];
	vert->varying[FIXED_VARYING_COLOR].x = vert->attribs[FIXED_ATTR_COLOR].x;
	vert->varying[FIXED_VARYING_COLOR].y = vert->attribs[FIXED_ATTR_COLOR].y;
	vert->varying[FIXED_VARYING_COLOR].z = vert->attribs[FIXED_ATTR_COLOR].z;
	vert->varying[FIXED_VARYING_COLOR].w = vert->attribs[FIXED_ATTR_COLOR].w;
	vert->varying[FIXED_VARYING_TEXCOORD].x = vert->attribs[FIXED_ATTR_TEXCOORD].x;
	vert->varying[FIXED_VARYING_TEXCOORD].y = vert->attribs[FIXED_ATTR_TEXCOORD].y;
	vert->varying[FIXED_VARYING_TEXCOORD].z = vert->attribs[FIXED_ATTR_TEXCOORD].z;
	vert->varying[FIXED_VARYING_TEXCOORD].w = vert->attribs[FIXED_ATTR_TEXCOORD].w;
	if (block->lighting_enable)
	{
		GLfloat normal[4];
		normal[0] = vert->attribs[FIXED_ATTR_NORMAL].x;
		normal[1] = vert->attribs[FIXED_ATTR_NORMAL].y;
		normal[2] = vert->attribs[FIXED_ATTR_NORMAL].z;
		normal[3] = 0;
		mat4_transform_vec4(block->modelview_matrix, normal);
		GLfloat tmp = 1.0f / sqrtf(normal[0] * normal[0]
		                         + normal[1] * normal[1]
		                         + normal[2] * normal[2]);
		normal[0] *= tmp;
		normal[1] *= tmp;
		normal[2] *= tmp;
		GLfloat ambient[3] = {0};
		GLfloat diffuse[3] = {0};
		GLfloat specular[3] = {0};
		lights_colors(block, vert, modelview_pos, normal, ambient, diffuse, specular);
		const struct material *material = &block->materials[!!vert->front_face];
		diffuse[0] = material->emission[0] + diffuse[0] * material->diffuse[0];
		diffuse[1] = material->emission[1] + diffuse[1] * material->diffuse[1];
		diffuse[2] = material->emission[2] + diffuse[2] * material->diffuse[2];
		vert->varying[FIXED_VARYING_COLOR].x *= ambient[0] * material->ambient[0] + diffuse[0];
		vert->varying[FIXED_VARYING_COLOR].x += specular[0] * material->specular[0];
		vert->varying[FIXED_VARYING_COLOR].y *= ambient[1] * material->ambient[1] + diffuse[1];
		vert->varying[FIXED_VARYING_COLOR].y += specular[1] * material->specular[1];
		vert->varying[FIXED_VARYING_COLOR].z *= ambient[2] * material->ambient[2] + diffuse[2];
		vert->varying[FIXED_VARYING_COLOR].z += specular[2] * material->specular[2];
	}
	if (block->fog_enable)
		vert->varying[FIXED_VARYING_FOG].x = clampf(fog_factor(block, modelview_pos[2]), 0, 1);
}

static GLsizei
type_size(GLenum type)
{
	switch (type)
	{
		case GL_BYTE:
			return sizeof(GLbyte);
		case GL_UNSIGNED_BYTE:
			return sizeof(GLubyte);
		case GL_SHORT:
			return sizeof(GLshort);
		case GL_UNSIGNED_SHORT:
			return sizeof(GLushort);
		case GL_INT:
			return sizeof(GLint);
		case GL_UNSIGNED_INT:
			return sizeof(GLuint);
		case GL_FLOAT:
			return sizeof(GLfloat);
		case GL_DOUBLE:
			return sizeof(GLdouble);
		default:
			return 0;
	}
}

static void
set_pointer(struct client_array *array,
            GLint size,
            GLenum type,
            GLsizei stride,
            const GLvoid *pointer,
            GLboolean interleaved)
{
	array->size = size;
	array->type = type;
	array->stride = stride;
	array->pointer = pointer;
	array->interleaved = interleaved;
}

static void
set_client_state(struct gl_ctx *ctx,
                 GLenum cap,
                 GLboolean enable)
{
	switch (cap)
	{
		case GL_COLOR_ARRAY:
			ctx->fixed.arrays.color.enable = enable;
			break;
		case GL_VERTEX_ARRAY:
			ctx->fixed.arrays.vertex.enable = enable;
			break;
		case GL_TEXTURE_COORD_ARRAY:
			ctx->fixed.arrays.texcoord.enable = enable;
			break;
		case GL_NORMAL_ARRAY:
			ctx->fixed.arrays.normal.enable = enable;
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			break;
	}
}

void
glColorPointer(GLint size,
               GLenum type,
               GLsizei stride,
               const GLvoid *pointer)
{
	struct gl_ctx *ctx = g_ctx;

	switch (size)
	{
		case 3:
		case 4:
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_VALUE);
			return;
	}
	switch (type)
	{
		case GL_BYTE:
		case GL_UNSIGNED_BYTE:
		case GL_SHORT:
		case GL_UNSIGNED_SHORT:
		case GL_INT:
		case GL_UNSIGNED_INT:
		case GL_FLOAT:
		case GL_DOUBLE:
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
	if (stride < 0)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	if (!stride)
		stride = size * type_size(type);
	set_pointer(&ctx->fixed.arrays.color,
	            size,
	            type,
	            stride,
	            pointer,
	            GL_FALSE);
}

void
glVertexPointer(GLint size,
                GLenum type,
                GLsizei stride,
                const GLvoid *pointer)
{
	struct gl_ctx *ctx = g_ctx;

	switch (size)
	{
		case 2:
		case 3:
		case 4:
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_VALUE);
			return;
	}
	switch (type)
	{
		case GL_SHORT:
		case GL_INT:
		case GL_FLOAT:
		case GL_DOUBLE:
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
	if (stride < 0)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	if (!stride)
		stride = size * type_size(type);
	set_pointer(&ctx->fixed.arrays.vertex,
	            size,
	            type,
	            stride,
	            pointer,
	            GL_FALSE);
}

void
glTexCoordPointer(GLint size,
                  GLenum type,
                  GLsizei stride,
                  const GLvoid *pointer)
{
	struct gl_ctx *ctx = g_ctx;

	switch (size)
	{
		case 1:
		case 2:
		case 3:
		case 4:
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_VALUE);
			return;
	}
	switch (type)
	{
		case GL_SHORT:
		case GL_INT:
		case GL_FLOAT:
		case GL_DOUBLE:
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
	if (stride < 0)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	if (!stride)
		stride = size * type_size(type);
	set_pointer(&ctx->fixed.arrays.texcoord,
	            size,
	            type,
	            stride,
	            pointer,
	            GL_FALSE);
}

void
glNormalPointer(GLenum type,
                GLsizei stride,
                const GLvoid *pointer)
{
	struct gl_ctx *ctx = g_ctx;

	switch (type)
	{
		case GL_BYTE:
		case GL_SHORT:
		case GL_INT:
		case GL_FLOAT:
		case GL_DOUBLE:
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
	if (stride < 0)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	if (!stride)
		stride = 3 * type_size(type);
	set_pointer(&ctx->fixed.arrays.normal,
	            3,
	            type,
	            stride,
	            pointer,
	            GL_FALSE);
}

void
interleaved_arrays(struct gl_ctx *ctx,
                   GLenum format,
                   GLsizei stride,
                   const GLvoid *pointer)
{
	const uint8_t *ptr;

	ptr = pointer;
	if (stride < 0)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	switch (format)
	{
		case GL_V2F:
			if (!stride)
				stride = 2 * sizeof(float);
			set_client_state(ctx, GL_VERTEX_ARRAY, GL_TRUE);
			set_pointer(&ctx->fixed.arrays.vertex,
			            2,
			            GL_FLOAT,
			            stride,
			            ptr,
			            GL_TRUE);
			break;
		case GL_V3F:
			if (!stride)
				stride = 3 * sizeof(float);
			set_client_state(ctx, GL_VERTEX_ARRAY, GL_TRUE);
			set_pointer(&ctx->fixed.arrays.vertex,
			            3,
			            GL_FLOAT,
			            stride,
			            ptr,
			            GL_TRUE);
			break;
		case GL_C4UB_V2F:
			if (!stride)
				stride = 4 * sizeof(uint8_t) + 2 * sizeof(float);
			set_client_state(ctx, GL_COLOR_ARRAY, GL_TRUE);
			set_pointer(&ctx->fixed.arrays.color,
			            4,
			            GL_UNSIGNED_BYTE,
			            stride,
			            ptr,
			            GL_TRUE);
			ptr += 4 * sizeof(uint8_t);
			set_client_state(ctx, GL_VERTEX_ARRAY, GL_TRUE);
			set_pointer(&ctx->fixed.arrays.vertex,
			            2,
			            GL_FLOAT,
			            stride,
			            ptr,
			            GL_TRUE);
			break;
		case GL_C4UB_V3F:
			if (!stride)
				stride = 4 * sizeof(uint8_t) + 3 * sizeof(float);
			set_client_state(ctx, GL_COLOR_ARRAY, GL_TRUE);
			set_pointer(&ctx->fixed.arrays.color,
			            4,
			            GL_UNSIGNED_BYTE,
			            stride,
			            ptr,
			            GL_TRUE);
			ptr += 4 * sizeof(uint8_t);
			set_client_state(ctx, GL_VERTEX_ARRAY, GL_TRUE);
			set_pointer(&ctx->fixed.arrays.vertex,
			            3,
			            GL_FLOAT,
			            stride,
			            ptr,
			            GL_TRUE);
			break;
		case GL_C3F_V3F:
			if (!stride)
				stride = 3 * sizeof(float) + 3 * sizeof(float);
			set_client_state(ctx, GL_COLOR_ARRAY, GL_TRUE);
			set_pointer(&ctx->fixed.arrays.color,
			            3,
			            GL_FLOAT,
			            stride,
			            ptr,
			            GL_TRUE);
			ptr += 3 * sizeof(float);
			set_client_state(ctx, GL_VERTEX_ARRAY, GL_TRUE);
			set_pointer(&ctx->fixed.arrays.vertex,
			            3,
			            GL_FLOAT,
			            stride,
			            ptr,
			            GL_TRUE);
			break;
		case GL_N3F_V3F:
			if (!stride)
				stride = 3 * sizeof(float) + 3 * sizeof(float);
			set_client_state(ctx, GL_NORMAL_ARRAY, GL_TRUE);
			set_pointer(&ctx->fixed.arrays.normal,
			            3,
			            GL_FLOAT,
			            stride,
			            ptr,
			            GL_TRUE);
			ptr += 3 * sizeof(float);
			set_client_state(ctx, GL_VERTEX_ARRAY, GL_TRUE);
			set_pointer(&ctx->fixed.arrays.vertex,
			            3,
			            GL_FLOAT,
			            stride,
			            ptr,
			            GL_TRUE);
			break;
		case GL_C4F_N3F_V3F:
			if (!stride)
				stride = 4 * sizeof(float) + 3 * sizeof(float) + 3 * sizeof(float);
			set_client_state(ctx, GL_COLOR_ARRAY, GL_TRUE);
			set_pointer(&ctx->fixed.arrays.color,
			            4,
			            GL_FLOAT,
			            stride,
			            ptr,
			            GL_TRUE);
			ptr += 4 * sizeof(float);
			set_client_state(ctx, GL_NORMAL_ARRAY, GL_TRUE);
			set_pointer(&ctx->fixed.arrays.normal,
			            3,
			            GL_FLOAT,
			            stride,
			            ptr,
			            GL_TRUE);
			ptr += 3 * sizeof(float);
			set_client_state(ctx, GL_VERTEX_ARRAY, GL_TRUE);
			set_pointer(&ctx->fixed.arrays.vertex,
			            3,
			            GL_FLOAT,
			            stride,
			            ptr,
			            GL_TRUE);
			break;
		case GL_T2F_V3F:
			if (!stride)
				stride = 2 * sizeof(float) + 3 * sizeof(float);
			set_client_state(ctx, GL_TEXTURE_COORD_ARRAY, GL_TRUE);
			set_pointer(&ctx->fixed.arrays.texcoord,
			            2,
			            GL_FLOAT,
			            stride,
			            ptr,
			            GL_TRUE);
			ptr += 2 * sizeof(float);
			set_client_state(ctx, GL_VERTEX_ARRAY, GL_TRUE);
			set_pointer(&ctx->fixed.arrays.vertex,
			            3,
			            GL_FLOAT,
			            stride,
			            ptr,
			            GL_TRUE);
			break;
		case GL_T4F_V4F:
			if (!stride)
				stride = 4 * sizeof(float) + 4 * sizeof(float);
			set_client_state(ctx, GL_TEXTURE_COORD_ARRAY, GL_TRUE);
			set_pointer(&ctx->fixed.arrays.texcoord,
			            4,
			            GL_FLOAT,
			            stride,
			            ptr,
			            GL_TRUE);
			ptr += 4 * sizeof(float);
			set_client_state(ctx, GL_VERTEX_ARRAY, GL_TRUE);
			set_pointer(&ctx->fixed.arrays.vertex,
			            4,
			            GL_FLOAT,
			            stride,
			            ptr,
			            GL_TRUE);
			break;
		case GL_T2F_C4UB_V3F:
			if (!stride)
				stride = 2 * sizeof(float) + 4 * sizeof(uint8_t) + 3 * sizeof(float);
			set_client_state(ctx, GL_TEXTURE_COORD_ARRAY, GL_TRUE);
			set_pointer(&ctx->fixed.arrays.texcoord,
			            2,
			            GL_FLOAT,
			            stride,
			            ptr,
			            GL_TRUE);
			ptr += 2 * sizeof(float);
			set_client_state(ctx, GL_COLOR_ARRAY, GL_TRUE);
			set_pointer(&ctx->fixed.arrays.color,
			            4,
			            GL_UNSIGNED_BYTE,
			            stride,
			            ptr,
			            GL_TRUE);
			ptr += 4 * sizeof(uint8_t);
			set_client_state(ctx, GL_VERTEX_ARRAY, GL_TRUE);
			set_pointer(&ctx->fixed.arrays.vertex,
			            3,
			            GL_FLOAT,
			            stride,
			            ptr,
			            GL_TRUE);
			break;
		case GL_T2F_C3F_V3F:
			if (!stride)
				stride = 2 * sizeof(float) + 3 * sizeof(float) + 3 * sizeof(float);
			set_client_state(ctx, GL_TEXTURE_COORD_ARRAY, GL_TRUE);
			set_pointer(&ctx->fixed.arrays.texcoord,
			            2,
			            GL_FLOAT,
			            stride,
			            ptr,
			            GL_TRUE);
			ptr += 2 * sizeof(float);
			set_client_state(ctx, GL_COLOR_ARRAY, GL_TRUE);
			set_pointer(&ctx->fixed.arrays.color,
			            3,
			            GL_FLOAT,
			            stride,
			            ptr,
			            GL_TRUE);
			ptr += 3 * sizeof(float);
			set_client_state(ctx, GL_VERTEX_ARRAY, GL_TRUE);
			set_pointer(&ctx->fixed.arrays.vertex,
			            3,
			            GL_FLOAT,
			            stride,
			            ptr,
			            GL_TRUE);
			break;
		case GL_T2F_N3F_V3F:
			if (!stride)
				stride = 2 * sizeof(float) + 3 * sizeof(float) + 3 * sizeof(float);
			set_client_state(ctx, GL_TEXTURE_COORD_ARRAY, GL_TRUE);
			set_pointer(&ctx->fixed.arrays.texcoord,
			            2,
			            GL_FLOAT,
			            stride,
			            ptr,
			            GL_TRUE);
			ptr += 2 * sizeof(float);
			set_client_state(ctx, GL_NORMAL_ARRAY, GL_TRUE);
			set_pointer(&ctx->fixed.arrays.normal,
			            3,
			            GL_FLOAT,
			            stride,
			            ptr,
			            GL_TRUE);
			ptr += 3 * sizeof(float);
			set_client_state(ctx, GL_VERTEX_ARRAY, GL_TRUE);
			set_pointer(&ctx->fixed.arrays.vertex,
			            3,
			            GL_FLOAT,
			            stride,
			            ptr,
			            GL_TRUE);
			break;
		case GL_T2F_C4F_N3F_V3F:
			if (!stride)
				stride = 2 * sizeof(float) + 4 * sizeof(float) + 3 * sizeof(float) + 3 * sizeof(float);
			set_client_state(ctx, GL_TEXTURE_COORD_ARRAY, GL_TRUE);
			set_pointer(&ctx->fixed.arrays.texcoord,
			            2,
			            GL_FLOAT,
			            stride,
			            ptr,
			            GL_TRUE);
			ptr += 2 * sizeof(float);
			set_client_state(ctx, GL_COLOR_ARRAY, GL_TRUE);
			set_pointer(&ctx->fixed.arrays.color,
			            4,
			            GL_FLOAT,
			            stride,
			            ptr,
			            GL_TRUE);
			ptr += 4 * sizeof(float);
			set_client_state(ctx, GL_NORMAL_ARRAY, GL_TRUE);
			set_pointer(&ctx->fixed.arrays.normal,
			            3,
			            GL_FLOAT,
			            stride,
			            ptr,
			            GL_TRUE);
			ptr += 3 * sizeof(float);
			set_client_state(ctx, GL_VERTEX_ARRAY, GL_TRUE);
			set_pointer(&ctx->fixed.arrays.vertex,
			            3,
			            GL_FLOAT,
			            stride,
			            ptr,
			            GL_TRUE);
			break;
		case GL_T4F_C4F_N3F_V4F:
			if (!stride)
				stride = 4 * sizeof(float) + 4 * sizeof(float) + 3 * sizeof(float) + 4 * sizeof(float);
			set_client_state(ctx, GL_TEXTURE_COORD_ARRAY, GL_TRUE);
			set_pointer(&ctx->fixed.arrays.texcoord,
			            4,
			            GL_FLOAT,
			            stride,
			            ptr,
			            GL_TRUE);
			ptr += 4 * sizeof(float);
			set_client_state(ctx, GL_COLOR_ARRAY, GL_TRUE);
			set_pointer(&ctx->fixed.arrays.color,
			            4,
			            GL_FLOAT,
			            stride,
			            ptr,
			            GL_TRUE);
			ptr += 4 * sizeof(float);
			set_client_state(ctx, GL_NORMAL_ARRAY, GL_TRUE);
			set_pointer(&ctx->fixed.arrays.normal,
			            3,
			            GL_FLOAT,
			            stride,
			            ptr,
			            GL_TRUE);
			ptr += 3 * sizeof(float);
			set_client_state(ctx, GL_VERTEX_ARRAY, GL_TRUE);
			set_pointer(&ctx->fixed.arrays.vertex,
			            3,
			            GL_FLOAT,
			            stride,
			            ptr,
			            GL_TRUE);
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
	set_pointer(&ctx->fixed.arrays.interleaved,
	            0,
	            format,
	            stride,
	            pointer,
	            GL_TRUE);
}

void
glInterleavedArrays(GLenum format,
                    GLsizei stride,
                    const GLvoid *pointer)
{
	interleaved_arrays(g_ctx, format, stride, pointer);
}

void
glEnableClientState(GLenum cap)
{
	set_client_state(g_ctx, cap, GL_TRUE);
}

void
glDisableClientState(GLenum cap)
{
	set_client_state(g_ctx, cap, GL_FALSE);
}

void
glPushClientAttrib(GLbitfield mask)
{
	/* XXX */
}

void
glPopClientAttrib(void)
{
	/* XXX */
}

bool
fixed_init(struct gl_ctx *ctx)
{
	ctx->fixed.vao = vertex_array_alloc(ctx);
	ctx->fixed.vertex_buffer = buffer_alloc(ctx);
	ctx->fixed.color_buffer = buffer_alloc(ctx);
	ctx->fixed.texcoord_buffer = buffer_alloc(ctx);
	ctx->fixed.normal_buffer = buffer_alloc(ctx);
	ctx->fixed.interleaved_buffer = buffer_alloc(ctx);
	ctx->fixed.indice_buffer = buffer_alloc(ctx);
	ctx->fixed.uniform_buffer = buffer_alloc(ctx);
	if (!ctx->fixed.vao
	 || !ctx->fixed.vertex_buffer
	 || !ctx->fixed.color_buffer
	 || !ctx->fixed.texcoord_buffer
	 || !ctx->fixed.normal_buffer
	 || !ctx->fixed.interleaved_buffer
	 || !ctx->fixed.indice_buffer
	 || !ctx->fixed.uniform_buffer)
		return false;
	/* XXX remove those hacks */
	ctx->fixed.vertex_buffer->target = GL_ARRAY_BUFFER;
	ctx->fixed.color_buffer->target = GL_ARRAY_BUFFER;
	ctx->fixed.texcoord_buffer->target = GL_ARRAY_BUFFER;
	ctx->fixed.normal_buffer->target = GL_ARRAY_BUFFER;
	ctx->fixed.interleaved_buffer->target = GL_ARRAY_BUFFER;
	ctx->fixed.indice_buffer->target = GL_ELEMENT_ARRAY_BUFFER;
	ctx->fixed.uniform_buffer->target = GL_UNIFORM_BUFFER;
	vertex_array_index_buffer(ctx, ctx->fixed.vao, ctx->fixed.indice_buffer);
	buffer_storage(ctx, ctx->fixed.uniform_buffer, sizeof(ctx->fixed.block), NULL, 0);
	return true;
}

static void
update_array(struct gl_ctx *ctx,
             struct client_array *array,
             enum fixed_attr attr,
             size_t *interleaved_buffer,
             size_t *buffer_index,
             size_t count,
             struct buffer *buffer)
{
	size_t offset;

	if (!array->enable)
	{
		vertex_array_attrib_enable(ctx,
		                           ctx->fixed.vao,
		                           attr,
		                           GL_FALSE);
		return;
	}
	if (array->interleaved)
	{
		if (*interleaved_buffer == SIZE_MAX)
		{
			*interleaved_buffer = (*buffer_index)++;
			vertex_array_vertex_buffer(ctx,
			                           ctx->fixed.vao,
			                           *interleaved_buffer,
			                           ctx->fixed.interleaved_buffer,
			                           0,
			                           ctx->fixed.arrays.interleaved.stride);
			buffer_data(ctx,
			            ctx->fixed.interleaved_buffer,
			            count * ctx->fixed.arrays.interleaved.stride,
			            ctx->fixed.arrays.interleaved.pointer,
			            GL_STATIC_DRAW);
		}
		vertex_array_attrib_binding(ctx,
		                            ctx->fixed.vao,
		                            attr,
		                            *interleaved_buffer);
		offset = (uint8_t*)array->pointer - (uint8_t*)ctx->fixed.arrays.interleaved.pointer;
	}
	else
	{
		vertex_array_vertex_buffer(ctx,
		                           ctx->fixed.vao,
		                           *buffer_index,
		                           buffer,
		                           0,
		                           array->stride);
		buffer_data(ctx,
		            buffer,
		            count * array->stride,
		            array->pointer,
		            GL_STATIC_DRAW);
		vertex_array_attrib_binding(ctx,
		                            ctx->fixed.vao,
		                            attr,
		                            *buffer_index);
		(*buffer_index)++;
		offset = 0;
	}
	vertex_array_attrib_format(ctx,
	                           ctx->vao,
	                           attr,
	                           array->size,
	                           array->type,
	                           (array->type == GL_FLOAT || array->type == GL_DOUBLE) ? CONV_SCALED : CONV_NORM,
	                           offset);
	vertex_array_attrib_enable(ctx,
	                           ctx->fixed.vao,
	                           attr,
	                           GL_TRUE);
}

static bool
update_state(struct gl_ctx *ctx)
{
	struct fixed_entry *entry;

	ctx->fixed.state.light_enable = 0;
	for (size_t i = 0; i < MAX_LIGHTS; ++i)
	{
		if (ctx->fixed.block.lights[i].enable)
			ctx->fixed.state.light_enable |= (1 << i);
	}
	ctx->fixed.state.client_arrays_enable = 0;
	if (ctx->fixed.arrays.vertex.enable)
		ctx->fixed.state.client_arrays_enable |= (1 << FIXED_ATTR_POSITION);
	if (ctx->fixed.arrays.color.enable)
		ctx->fixed.state.client_arrays_enable |= (1 << FIXED_ATTR_COLOR);
	if (ctx->fixed.arrays.normal.enable)
		ctx->fixed.state.client_arrays_enable |= (1 << FIXED_ATTR_NORMAL);
	if (ctx->fixed.arrays.texcoord.enable)
		ctx->fixed.state.client_arrays_enable |= (1 << FIXED_ATTR_TEXCOORD);
	ctx->fixed.state.lighting_enable = ctx->fixed.block.lighting_enable;
	ctx->fixed.state.fog_enable = ctx->fixed.block.fog_enable;
	ctx->fixed.state.fog_mode = ctx->fixed.block.fog_mode;
	ctx->fixed.state.fog_coord_src = ctx->fixed.block.fog_coord_src;
	ctx->fixed.state.texture_1d_enable = ctx->fixed.block.texture_1d_enable;
	ctx->fixed.state.texture_2d_enable = ctx->fixed.block.texture_2d_enable;
	ctx->fixed.state.texture_3d_enable = ctx->fixed.block.texture_3d_enable;
	entry = (struct fixed_entry*)cache_get(ctx, &ctx->fixed_cache, &ctx->fixed.state);
	if (!entry)
		return false;
	if (entry == ctx->fixed.entry)
	{
		cache_unref(ctx, &ctx->fixed_cache, &entry->cache);
		return true;
	}
	if (ctx->fixed.entry)
		cache_unref(ctx, &ctx->fixed_cache, &ctx->fixed.entry->cache);
	ctx->fixed.entry = entry;
	ctx->dirty |= GL_CTX_DIRTY_PROGRAM;
	return true;
}

static void
transpose_matrix(float * restrict dst, const float * restrict src)
{
	dst[0x0] = src[0x0];
	dst[0x1] = src[0x4];
	dst[0x2] = src[0x8];
	dst[0x3] = src[0xC];
	dst[0x4] = src[0x1];
	dst[0x5] = src[0x5];
	dst[0x6] = src[0x9];
	dst[0x7] = src[0xD];
	dst[0x8] = src[0x2];
	dst[0x9] = src[0x6];
	dst[0xA] = src[0xA];
	dst[0xB] = src[0xE];
	dst[0xC] = src[0x3];
	dst[0xD] = src[0x7];
	dst[0xE] = src[0xB];
	dst[0xF] = src[0xF];
}

bool
fixed_update(struct gl_ctx *ctx,
             const GLvoid *indices,
             GLenum indice_type,
             GLsizei count)
{
	size_t first_indice;
	size_t last_indice;
	size_t interleaved_buffer = SIZE_MAX;
	size_t buffer_index = 0;

	if (!ctx->fixed.arrays.vertex.enable)
		return false;
	transpose_matrix(ctx->fixed.block.projection_matrix,
	                 ctx->fixed.projection_matrix[ctx->fixed.projection_stack_depth]);
	transpose_matrix(ctx->fixed.block.modelview_matrix,
	                 ctx->fixed.modelview_matrix[ctx->fixed.modelview_stack_depth]);
	if (ctx->fixed.block.fog_enable)
	{
		switch (ctx->fixed.block.fog_mode)
		{
			case GL_LINEAR:
				if (ctx->fixed.block.fog_end - ctx->fixed.block.fog_start < 0.00001f)
				{
					ctx->fixed.block.fog_factors[0] = 0;
					ctx->fixed.block.fog_factors[1] = 0;
				}
				else
				{
					ctx->fixed.block.fog_factors[0] = -ctx->fixed.block.fog_start;
					ctx->fixed.block.fog_factors[1] = 1.0f / (ctx->fixed.block.fog_end - ctx->fixed.block.fog_start);
				}
				break;
			case GL_EXP:
				ctx->fixed.block.fog_factors[0] = ctx->fixed.block.fog_density;
				break;
			case GL_EXP2:
				ctx->fixed.block.fog_factors[0] = ctx->fixed.block.fog_density;
				break;
		}
	}
	buffer_subdata(ctx, ctx->fixed.uniform_buffer, 0, sizeof(ctx->fixed.block), &ctx->fixed.block);
	if (indices)
	{
		first_indice = SIZE_MAX;
		last_indice = 0;
		switch (indice_type)
		{
			case GL_UNSIGNED_BYTE:
			{
				const uint8_t *data = indices;
				for (GLsizei i = 0; i < count; ++i)
				{
					if (data[i] < first_indice)
						first_indice = data[i];
					if (data[i] > last_indice)
						last_indice = data[i];
				}
				buffer_data(ctx,
				            ctx->fixed.indice_buffer,
				            count * sizeof(uint8_t),
				            indices,
				            GL_STATIC_DRAW);
				break;
			}
			case GL_UNSIGNED_SHORT:
			{
				const uint16_t *data = indices;
				for (GLsizei i = 0; i < count; ++i)
				{
					if (data[i] < first_indice)
						first_indice = data[i];
					if (data[i] > last_indice)
						last_indice = data[i];
				}
				buffer_data(ctx,
				            ctx->fixed.indice_buffer,
				            count * sizeof(uint16_t),
				            indices,
				            GL_STATIC_DRAW);
				break;
			}
			case GL_UNSIGNED_INT:
			{
				const uint32_t *data = indices;
				for (GLsizei i = 0; i < count; ++i)
				{
					if (data[i] < first_indice)
						first_indice = data[i];
					if (data[i] > last_indice)
						last_indice = data[i];
				}
				buffer_data(ctx,
				            ctx->fixed.indice_buffer,
				            count * sizeof(uint32_t),
				            indices,
				            GL_STATIC_DRAW);
				break;
			}
		}
	}
	else
	{
		first_indice = 0;
		last_indice = count;
	}
	update_array(ctx,
	             &ctx->fixed.arrays.vertex,
	             FIXED_ATTR_POSITION,
	             &interleaved_buffer,
	             &buffer_index,
	             last_indice + 1,
	             ctx->fixed.vertex_buffer);
	update_array(ctx,
	             &ctx->fixed.arrays.color,
	             FIXED_ATTR_COLOR,
	             &interleaved_buffer,
	             &buffer_index,
	             last_indice + 1,
	             ctx->fixed.color_buffer);
	update_array(ctx,
	             &ctx->fixed.arrays.texcoord,
	             FIXED_ATTR_TEXCOORD,
	             &interleaved_buffer,
	             &buffer_index,
	             last_indice + 1,
	             ctx->fixed.texcoord_buffer);
	update_array(ctx,
	             &ctx->fixed.arrays.normal,
	             FIXED_ATTR_NORMAL,
	             &interleaved_buffer,
	             &buffer_index,
	             last_indice + 1,
	             ctx->fixed.normal_buffer);
	vertex_array_bind(ctx, ctx->fixed.vao); /* XXX restore */
	buffer_bind_range(ctx,
	                  ctx->fixed.uniform_buffer,
	                  0,
	                  0,
	                  sizeof(ctx->fixed.block)); /* XXX restore */
	if (!update_state(ctx))
		return false;
	return true;
}

static uint32_t
fixed_state_hash(struct gl_ctx *ctx,
                 const void *st)
{
	const struct fixed_state *state = st;
	uint32_t hash;

	(void)ctx;
	hash = FNV_BASIS32;
	hash = fnv32v(hash, &state->light_enable, sizeof(state->light_enable));
	hash = fnv32v(hash, &state->lighting_enable, sizeof(state->lighting_enable));
	hash = fnv32v(hash, &state->fog_enable, sizeof(state->fog_enable));
	hash = fnv32v(hash, &state->fog_mode, sizeof(state->fog_mode));
	hash = fnv32v(hash, &state->fog_coord_src, sizeof(state->fog_coord_src));
	hash = fnv32v(hash, &state->texture_1d_enable, sizeof(state->texture_1d_enable));
	hash = fnv32v(hash, &state->texture_2d_enable, sizeof(state->texture_2d_enable));
	hash = fnv32v(hash, &state->texture_3d_enable, sizeof(state->texture_3d_enable));
	return hash;
}

static bool
fixed_state_eq(struct gl_ctx *ctx,
               const struct cache_entry *entry,
               const void *state)
{
	const struct fixed_state *a = &((struct fixed_entry*)entry)->state;
	const struct fixed_state *b = state;

	(void)ctx;
	if (a->light_enable != b->light_enable
	 || a->lighting_enable != b->lighting_enable
	 || a->fog_enable != b->fog_enable
	 || a->fog_mode != b->fog_mode
	 || a->fog_coord_src != b->fog_coord_src
	 || a->texture_1d_enable != b->texture_1d_enable
	 || a->texture_2d_enable != b->texture_2d_enable
	 || a->texture_3d_enable != b->texture_3d_enable)
		return false;
	return true;
}

static const char *
get_texture_type(const struct fixed_state *state)
{
	if (!(state->client_arrays_enable & (1 << FIXED_ATTR_TEXCOORD)))
		return NULL;
	/* XXX cubemap */
	if (state->texture_3d_enable)
		return "3D";
	if (state->texture_2d_enable)
		return "2D";
	if (state->texture_1d_enable)
		return "1D";
	return NULL;
}

struct code
{
	char *data;
	size_t size;
	size_t len;
};

__attribute__((format(printf, 2, 3)))
static bool
append_line(struct code *code, const char *fmt, ...)
{
	va_list va_arg;
	bool ret = false;
	int len;

	va_start(va_arg, fmt);
	if (code->size - code->len < 128) /* XXX remove this per-line limit */
	{
		size_t new_size;
		char *new_data;

		new_size = code->size + 4096;
		new_data = realloc(code->data, new_size);
		if (!new_data)
			goto end;
		code->data = new_data;
		code->size = new_size;
	}
	len = vsnprintf(&code->data[code->len],
	                code->size - code->len,
	                fmt,
	                va_arg);
	if (code->len + len + 2 > code->size)
		goto end;
	code->len += len;
	code->data[code->len++] = '\n';
	code->data[code->len] = '\0';
	ret = true;

end:
	va_end(va_arg);
	return ret;
}

static char *
gen_vs_code(struct gl_ctx *ctx,
            const struct fixed_state *state)
{
	const char *texture_type;
	struct code code;

	memset(&code, 0, sizeof(code));
	texture_type = get_texture_type(state);
	if (!append_line(&code, "VERT")
	 || !append_line(&code, "DCL IN[0]")
	 || !append_line(&code, "DCL OUT[0], POSITION")
	 || !append_line(&code, "DCL OUT[1], COLOR, PERSPECTIVE")
	 || !append_line(&code,
	                 "IMM[0] FLT32 {0x%08x, 0x%08x, 0x%08x, 0x%08x}",
	                 0x3F800000, 0x3F800000, 0x3F800000, 0x3F800000))
		goto err;
	if (state->client_arrays_enable & (1 << FIXED_ATTR_COLOR))
	{
		if (!append_line(&code, "DCL IN[1]"))
			goto err;
	}
	if (texture_type)
	{
		if (!append_line(&code, "DCL IN[3]")
		 || !append_line(&code, "DCL OUT[2], TEXCOORD, PERSPECTIVE"))
			goto err;
	}
	if (state->fog_enable)
	{
		if (!append_line(&code, "DCL TEMP[1]")
		 || !append_line(&code, "DCL TEMP[2]")
		 || !append_line(&code, "DCL OUT[3], GENERIC, PERSPECTIVE"))
			goto err;
	}
	if (!append_line(&code, "DCL CONST[1][0..9]")
	 || !append_line(&code, "DCL TEMP[0]")
	 || !append_line(&code, "DP4 TEMP[0].x, IN[0], CONST[1][4]")
	 || !append_line(&code, "DP4 TEMP[0].y, IN[0], CONST[1][5]")
	 || !append_line(&code, "DP4 TEMP[0].z, IN[0], CONST[1][6]")
	 || !append_line(&code, "DP4 TEMP[0].w, IN[0], CONST[1][7]")
	 || !append_line(&code, "DP4 OUT[0].x, TEMP[0], CONST[1][0]")
	 || !append_line(&code, "DP4 OUT[0].y, TEMP[0], CONST[1][1]")
	 || !append_line(&code, "DP4 OUT[0].z, TEMP[0], CONST[1][2]")
	 || !append_line(&code, "DP4 OUT[0].w, TEMP[0], CONST[1][3]"))
		goto err;
	if (state->client_arrays_enable & (1 << FIXED_ATTR_COLOR))
	{
		if (!append_line(&code, "MOV OUT[1], IN[1]"))
			goto err;
	}
	else
	{
		if (!append_line(&code, "MOV OUT[1], IMM[0]"))
			goto err;
	}
	if (texture_type)
	{
		if (!append_line(&code, "MOV OUT[2], IN[3]"))
			goto err;
	}
	if (state->fog_enable)
	{
		switch (state->fog_mode)
		{
			case GL_LINEAR:
				if (!append_line(&code, "SUB TEMP[1], CONST[1][9].xxxx, TEMP[0].zzzz")
				 || !append_line(&code, "MUL_SAT OUT[3], TEMP[1].xxxx, CONST[1][9].yyyy"))
				break;
			case GL_EXP:
				if (!append_line(&code, "MUL TEMP[1], CONST[1][9].xxxx, TEMP[0].zzzz")
				 || !append_line(&code, "EX2 TEMP[2], TEMP[1]")
				 || !append_line(&code, "SUB_SAT OUT[3], IMM[0].xxxx, TEMP[2].xxxx"))
					goto err;
				break;
			case GL_EXP2:
				if (!append_line(&code, "MUL TEMP[1], CONST[1][9].xxxx, TEMP[0].zzzz")
				 || !append_line(&code, "MUL TEMP[2], TEMP[1], TEMP[1]")
				 || !append_line(&code, "EX2 TEMP[1], TEMP[2]")
				 || !append_line(&code, "SUB_SAT OUT[3], IMM[0].xxxx, TEMP[1].xxxx"))
					goto err;
				break;
		}
	}
	if (!append_line(&code, "END"))
		goto err;
	return code.data;

err:
	free(code.data);
	return NULL;
}

static char *
gen_fs_code(struct gl_ctx *ctx,
            const struct fixed_state *state)
{
	const char *texture_type;
	struct code code;

	memset(&code, 0, sizeof(code));
	texture_type = get_texture_type(state);
	if (!append_line(&code, "FRAG")
	 || !append_line(&code, "DCL IN[0], COLOR, PERSPECTIVE")
	 || !append_line(&code, "DCL OUT[0], COLOR")
	 || !append_line(&code, "DCL TEMP[1]")
	 || !append_line(&code, "DCL CONST[1][0..9]"))
		goto err;
	if (state->fog_enable)
	{
		if (!append_line(&code, "DCL IN[2], GENERIC, PERSPECTIVE"))
			goto err;
	}
	if (texture_type)
	{
		if (!append_line(&code, "DCL IN[1], TEXCOORD, PERSPECTIVE")
		 || !append_line(&code, "DCL SAMP[0]")
		 || !append_line(&code, "DCL SVIEW[0], %s, FLOAT, FLOAT, FLOAT, FLOAT", texture_type)
		 || !append_line(&code, "DCL TEMP[0]")
		 || !append_line(&code, "TEX TEMP[0], IN[1], SAMP[0], %s", texture_type)
		 || !append_line(&code, "MUL TEMP[1], IN[0], TEMP[0]"))
			goto err;
	}
	else
	{
		if (!append_line(&code, "MOV TEMP[1], IN[0]"))
			goto err;
	}
	if (state->fog_enable)
	{
		if (!append_line(&code, "LRP OUT[0], IN[2].xxxx, CONST[1][8], TEMP[1]"))
			goto err;
	}
	else
	{
		if (!append_line(&code, "MOV OUT[0], TEMP[1]"))
			goto err;
	}
	if (!append_line(&code, "END"))
		goto err;
	return code.data;

err:
	free(code.data);
	return NULL;
}

static struct shader *
gen_shader(struct gl_ctx *ctx,
           const struct fixed_state *state,
           int fs)
{
	struct jkg_shader_create_info create_info;
	struct shader *shader;
	char *code = NULL;

	shader = shader_alloc(ctx, fs ? GL_FRAGMENT_SHADER : GL_VERTEX_SHADER);
	if (!shader)
	{
		free(code);
		return NULL;
	}
	create_info.type = fs ? JKG_SHADER_FRAGMENT : JKG_SHADER_VERTEX;
	if (!strcmp(ctx->jkg_caps->renderer, "soft")
	 || !strcmp(ctx->jkg_caps->renderer, "gjit"))
	{
		create_info.data = fs ? (void*)fixed_fragment_shader : (void*)fixed_vertex_shader;
		create_info.size = sizeof(void*);
	}
	else
	{
		if (fs)
			code = gen_fs_code(ctx, state);
		else
			code = gen_vs_code(ctx, state);
		if (!code)
		{
			shader_free(ctx, shader);
			return NULL;
		}
		create_info.data = code;
		create_info.size = strlen(code);
	}
	if (!shader_compile(ctx, shader, &create_info))
	{
		free(code);
		shader_free(ctx, shader);
		return NULL;
	}
	free(code);
	return shader;
}

static struct program *
gen_program(struct gl_ctx *ctx,
            const struct fixed_state *state)
{
	struct program *program;
	struct shader *vs;
	struct shader *fs;

	vs = gen_shader(ctx, state, 0);
	fs = gen_shader(ctx, state, 1);
	program = program_alloc(ctx);
	if (!vs
	 || !fs
	 || !program
	 || !program_link(ctx, program, vs, fs))
		goto err;
	/* XXX free vs / fs */
	return program;

err:
	if (vs)
		shader_free(ctx, vs);
	if (fs)
		shader_free(ctx, fs);
	if (program)
		program_free(ctx, program);
	return NULL;
}

static struct cache_entry *
fixed_entry_alloc(struct gl_ctx *ctx,
                  GLuint hash,
                  const void *st)
{
	const struct fixed_state *state = st;
	struct fixed_entry *entry;

	entry = malloc(sizeof(*entry));
	if (!entry)
	{
		GL_SET_ERR(ctx, GL_OUT_OF_MEMORY);
		return NULL;
	}
	memcpy(&entry->state, state, sizeof(*state));
	entry->cache.hash = hash;
	entry->cache.ref = 0;
	entry->program = gen_program(ctx, state);
	if (!entry->program)
	{
		free(entry);
		return NULL;
	}
	return &entry->cache;
}

static void
fixed_entry_free(struct gl_ctx *ctx,
                 struct cache_entry *e)
{
	struct fixed_entry *entry = (struct fixed_entry*)e;

	program_free(ctx, entry->program);
}

const struct cache_op
fixed_cache_op =
{
	.alloc = fixed_entry_alloc,
	.free = fixed_entry_free,
	.hash = fixed_state_hash,
	.eq = fixed_state_eq,
};
