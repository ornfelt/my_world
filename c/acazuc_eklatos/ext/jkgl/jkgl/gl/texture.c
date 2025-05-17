#include "internal.h"

#include <stdlib.h>
#include <assert.h>

static bool
is_valid_target(GLenum target)
{
	switch (target)
	{
		case GL_TEXTURE_1D:
		case GL_TEXTURE_1D_ARRAY:
		case GL_TEXTURE_2D:
		case GL_TEXTURE_2D_ARRAY:
		case GL_TEXTURE_3D:
			return true;
		default:
			return false;
	}
}

static void
get_texture_max_sizes(struct gl_ctx *ctx,
                      GLenum target,
                      uint32_t *max_sizes)
{
	switch (target)
	{
		case GL_TEXTURE_1D:
			max_sizes[0] = ctx->jkg_caps->max_texture_2d_size;
			max_sizes[1] = 0;
			max_sizes[2] = 0;
			break;
		case GL_TEXTURE_1D_ARRAY:
			max_sizes[0] = ctx->jkg_caps->max_texture_2d_size;
			max_sizes[1] = ctx->jkg_caps->max_texture_layers;
			max_sizes[2] = 0;
			break;
		case GL_TEXTURE_2D:
			max_sizes[0] = ctx->jkg_caps->max_texture_2d_size;
			max_sizes[1] = ctx->jkg_caps->max_texture_2d_size;
			max_sizes[2] = 0;
			break;
		case GL_TEXTURE_2D_ARRAY:
			max_sizes[0] = ctx->jkg_caps->max_texture_2d_size;
			max_sizes[1] = ctx->jkg_caps->max_texture_2d_size;
			max_sizes[2] = ctx->jkg_caps->max_texture_layers;
			break;
		case GL_TEXTURE_3D:
			max_sizes[0] = ctx->jkg_caps->max_texture_3d_size;
			max_sizes[1] = ctx->jkg_caps->max_texture_3d_size;
			max_sizes[2] = ctx->jkg_caps->max_texture_3d_size;
			break;
		default:
			assert(!"unknown texture type");
			max_sizes[0] = 0;
			max_sizes[1] = 0;
			max_sizes[2] = 0;
			break;
	}
}

static struct texture *
get_texture(struct gl_ctx *ctx, GLenum target)
{
	struct texture *texture;

	if (!is_valid_target(target))
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return NULL;
	}
	texture = ctx->textures[ctx->active_texture - GL_TEXTURE0];
	if (!texture)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return NULL;
	}
	if (texture->target != target)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return NULL;
	}
	return texture;
}

static bool
test_dimension(GLint offset,
               GLsizei size,
               uint32_t image_size)
{
	uint32_t end;

	if (!image_size)
		return true;
	if (offset < 0)
		return false;
	if ((uint32_t)offset >= image_size)
		return false;
	if (size < 0)
		return false;
	if (__builtin_add_overflow(offset, size, &end))
		return false;
	if (end > image_size)
		return false;
	return true;
}

static bool
test_bounds(GLint level,
            GLint xoffset,
            GLint yoffset,
            GLint zoffset,
            GLsizei width,
            GLsizei height,
            GLsizei depth,
            uint32_t image_levels,
            uint32_t image_width,
            uint32_t image_height,
            uint32_t image_depth)
{
	if (level < 0
	 || (uint32_t)level >= image_levels
	 || !test_dimension(xoffset, width, image_width)
	 || !test_dimension(yoffset, height, image_height)
	 || !test_dimension(zoffset, depth, image_depth))
		return false;
	return true;
}

struct texture *
texture_alloc(struct gl_ctx *ctx)
{
	struct texture *texture;

	(void)ctx;
	texture = calloc(1, sizeof(*texture));
	if (!texture)
		return NULL;
	texture->object.type = OBJECT_TEXTURE;
	texture->dirty = -1;
	texture->width = 0;
	texture->height = 0;
	texture->depth = 0;
	texture->format = 0;
	texture->target = GL_NONE;
	texture->levels = 0;
	texture->format = JKG_FORMAT_UNKNOWN;
	texture->immutable = GL_FALSE;
	sampler_state_init(&texture->sampler_state);
	texture->image_view_state.min_level = 0;
	texture->image_view_state.max_level = 1000;
	texture->image_view_state.min_layer = 0;
	texture->image_view_state.num_layers = 0; /* XXX change at creation */
	texture->image_view_state.swizzle[0] = GL_RED;
	texture->image_view_state.swizzle[1] = GL_GREEN;
	texture->image_view_state.swizzle[2] = GL_BLUE;
	texture->image_view_state.swizzle[3] = GL_ALPHA;
	texture->image_view = NULL;
	texture->sampler = NULL;
	texture->image = NULL;
	return texture;
}

void
glGenTextures(GLsizei n, GLuint *textures)
{
	struct gl_ctx *ctx = g_ctx;
	struct texture *texture;
	GLsizei i;

	if (n < 0)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	for (i = 0; i < n; ++i)
	{
		texture = texture_alloc(ctx);
		if (!texture)
			goto err;
		if (!object_alloc(ctx, &texture->object))
		{
			texture_free(ctx, texture);
			goto err;
		}
		textures[i] = texture->object.id;
	}
	return;

err:
	for (GLsizei j = 0; j < i; ++j)
	{
		texture = object_get(ctx, textures[j], OBJECT_TEXTURE);
		if (!texture)
			continue;
		texture_free(ctx, texture);
	}
}

void
glCreateTextures(GLenum target, GLsizei n, GLuint *textures)
{
	struct gl_ctx *ctx = g_ctx;

	if (n < 0)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	if (!is_valid_target(target))
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	for (GLsizei i = 0; i < n; ++i)
	{
		struct texture *texture = texture_alloc(ctx);
		if (!texture)
		{
			/* XXX */
			continue;
		}
		if (!object_alloc(ctx, &texture->object))
		{
			texture_free(ctx, texture);
			/* XXX */
			continue;
		}
		texture->target = target;
		textures[i] = texture->object.id;
	}
}

static void
release_image_view(struct gl_ctx *ctx, struct texture *texture)
{
	if (texture->image_view)
	{
		ctx->jkg_op->image_view_free(ctx->jkg_ctx, texture->image_view);
		texture->image_view = NULL;
	}
	if (texture->image)
	{
		ctx->jkg_op->image_free(ctx->jkg_ctx, texture->image);
		texture->image = NULL;
	}
}

void
texture_free(struct gl_ctx *ctx, struct texture *texture)
{
	if (texture->sampler)
		cache_unref(ctx, &ctx->sampler_cache, &texture->sampler->cache);
	release_image_view(ctx, texture);
	object_free(ctx, &texture->object);
}

void
glDeleteTextures(GLsizei n, const GLuint *textures)
{
	struct gl_ctx *ctx = g_ctx;

	if (n < 0)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	for (GLsizei i = 0; i < n; ++i)
	{
		GLuint id = textures[i];
		struct texture *texture = object_get(ctx, id, OBJECT_TEXTURE);
		if (!texture)
			continue;
		texture_free(ctx, texture);
	}
}

GLboolean
glIsTexture(GLuint id)
{
	return object_is(g_ctx, id, OBJECT_TEXTURE);
}

void
glBindTexture(GLenum target, GLuint id)
{
	struct gl_ctx *ctx = g_ctx;
	struct texture *texture;
	uint32_t active;

	if (!is_valid_target(target))
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	active = ctx->active_texture - GL_TEXTURE0;
	if (!id)
	{
		if (ctx->textures[active])
		{
			ctx->textures[active] = NULL;
			ctx->dirty |= GL_CTX_DIRTY_IMAGE_VIEWS;
			if (!ctx->samplers[active])
				ctx->dirty |= GL_CTX_DIRTY_SAMPLERS;
		}
		return;
	}
	texture = object_get(ctx, id, OBJECT_TEXTURE);
	if (!texture)
		return;
	if (texture->target && texture->target != target)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	if (ctx->textures[active] != texture)
	{
		ctx->textures[active] = texture;
		ctx->dirty |= GL_CTX_DIRTY_IMAGE_VIEWS;
		if (!ctx->samplers[active])
			ctx->dirty |= GL_CTX_DIRTY_SAMPLERS;
	}
	if (!texture->target)
	{
		texture->width = 0;
		texture->height = 0;
		texture->depth = 0;
		texture->levels = 0;
		texture->format = 0;
		texture->target = target;
	}
}

void
glBindTextureUnit(GLuint unit,
                  GLuint id)
{
	struct gl_ctx *ctx = g_ctx;
	struct texture *texture;

	if (unit > ctx->jkg_caps->max_texture_image_units) /* XXX another limit */
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	if (id)
	{
		texture = object_get(ctx, id, OBJECT_TEXTURE);
		if (!texture
		 || !texture->target)
		{
			GL_SET_ERR(ctx, GL_INVALID_OPERATION);
			return;
		}
	}
	else
	{
		texture = NULL;
	}
	if (ctx->textures[unit] != texture)
	{
		ctx->textures[unit] = texture;
		ctx->dirty |= GL_CTX_DIRTY_IMAGE_VIEWS;
		if (!ctx->samplers[unit])
			ctx->dirty |= GL_CTX_DIRTY_SAMPLERS;
	}
}

void
glBindTextures(GLuint first,
               GLsizei count,
               const GLuint *textures)
{
	struct gl_ctx *ctx = g_ctx;
	GLuint last;

	if (count < 0)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	if (__builtin_add_overflow(first, count, &last)
	 || last > ctx->jkg_caps->max_texture_image_units) /* XXX another limit */
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	for (GLsizei i = 0; i < count; ++i)
	{
		struct texture *texture;

		if (textures[i])
		{
			texture = object_get(ctx, textures[i], OBJECT_TEXTURE);
			if (!texture
			 || !texture->target)
			{
				GL_SET_ERR(ctx, GL_INVALID_OPERATION);
				return;
			}
		}
		else
		{
			texture = NULL;
		}
		if (ctx->textures[first + i] != texture)
		{
			ctx->textures[first + i] = texture;
			ctx->dirty |= GL_CTX_DIRTY_IMAGE_VIEWS;
			if (!ctx->samplers[first + i])
				ctx->dirty |= GL_CTX_DIRTY_SAMPLERS;
		}
	}
}

static GLboolean
is_valid_swizzle(GLenum v)
{
	switch (v)
	{
		case GL_RED:
		case GL_GREEN:
		case GL_BLUE:
		case GL_ALPHA:
		case GL_ZERO:
		case GL_ONE:
			return GL_TRUE;
		default:
			return GL_FALSE;
	}
}

static void
texture_parameterf(struct gl_ctx *ctx,
                   struct texture *texture,
                   GLenum pname,
                   GLfloat param)
{
	switch (pname)
	{
		case GL_TEXTURE_LOD_BIAS:
			if (sampler_state_set_lod_bias(ctx,
			                               &texture->sampler_state,
			                               param))
				texture->dirty |= GL_TEXTURE_DIRTY_SAMPLER;
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
}

static void
texture_parameteri(struct gl_ctx *ctx,
                   struct texture *texture,
                   GLenum pname,
                   GLint param)
{
	switch (pname)
	{
		case GL_TEXTURE_WRAP_S:
			if (sampler_state_set_wrap_s(ctx,
			                             &texture->sampler_state,
			                             param))
				texture->dirty |= GL_TEXTURE_DIRTY_SAMPLER;
			break;
		case GL_TEXTURE_WRAP_T:
			if (sampler_state_set_wrap_t(ctx,
			                             &texture->sampler_state,
			                             param))
				texture->dirty |= GL_TEXTURE_DIRTY_SAMPLER;
			break;
		case GL_TEXTURE_WRAP_R:
			if (sampler_state_set_wrap_r(ctx,
			                             &texture->sampler_state,
			                             param))
				texture->dirty |= GL_TEXTURE_DIRTY_SAMPLER;
			break;
		case GL_TEXTURE_MIN_FILTER:
			if (sampler_state_set_min_filter(ctx,
			                                 &texture->sampler_state,
			                                 param))
				texture->dirty |= GL_TEXTURE_DIRTY_SAMPLER;
			break;
		case GL_TEXTURE_MAG_FILTER:
			if (sampler_state_set_mag_filter(ctx,
			                                 &texture->sampler_state,
			                                 param))
				texture->dirty |= GL_TEXTURE_DIRTY_SAMPLER;
			break;
		case GL_TEXTURE_LOD_BIAS:
			if (sampler_state_set_lod_bias(ctx,
			                               &texture->sampler_state,
			                               param))
				texture->dirty |= GL_TEXTURE_DIRTY_SAMPLER;
			break;
		case GL_TEXTURE_MIN_LOD:
			if (sampler_state_set_min_lod(ctx,
			                              &texture->sampler_state,
			                              param))
				texture->dirty |= GL_TEXTURE_DIRTY_SAMPLER;
			break;
		case GL_TEXTURE_MAX_LOD:
			if (sampler_state_set_max_lod(ctx,
			                              &texture->sampler_state,
			                              param))
				texture->dirty |= GL_TEXTURE_DIRTY_SAMPLER;
			break;
		case GL_TEXTURE_COMPARE_MODE:
			if (sampler_state_set_compare_mode(ctx,
			                                   &texture->sampler_state,
			                                   param))
				texture->dirty |= GL_TEXTURE_DIRTY_SAMPLER;
			break;
		case GL_TEXTURE_COMPARE_FUNC:
			if (sampler_state_set_compare_func(ctx,
			                                   &texture->sampler_state,
			                                   param))
				texture->dirty |= GL_TEXTURE_DIRTY_SAMPLER;
			break;
		case GL_TEXTURE_BASE_LEVEL:
			if (texture->image_view_state.min_level != (GLuint)param)
			{
				texture->image_view_state.min_level = param;
				texture->dirty |= GL_TEXTURE_DIRTY_IMAGE_VIEW
				                | GL_TEXTURE_DIRTY_SURFACE;
			}
			break;
		case GL_TEXTURE_MAX_LEVEL:
			if (texture->image_view_state.max_level != (GLuint)param)
			{
				texture->image_view_state.max_level = param;
				texture->dirty |= GL_TEXTURE_DIRTY_IMAGE_VIEW
				                | GL_TEXTURE_DIRTY_SURFACE;
			}
			break;
		case GL_TEXTURE_SWIZZLE_R:
			if (!is_valid_swizzle(param))
			{
				GL_SET_ERR(ctx, GL_INVALID_ENUM);
				return;
			}
			if (texture->image_view_state.swizzle[0] != (GLenum)param)
			{
				texture->image_view_state.swizzle[0] = param;
				texture->dirty |= GL_TEXTURE_DIRTY_IMAGE_VIEW;
			}
			break;
		case GL_TEXTURE_SWIZZLE_G:
			if (!is_valid_swizzle(param))
			{
				GL_SET_ERR(ctx, GL_INVALID_ENUM);
				return;
			}
			if (texture->image_view_state.swizzle[1] != (GLenum)param)
			{
				texture->image_view_state.swizzle[1] = param;
				texture->dirty |= GL_TEXTURE_DIRTY_IMAGE_VIEW;
			}
			break;
		case GL_TEXTURE_SWIZZLE_B:
			if (!is_valid_swizzle(param))
			{
				GL_SET_ERR(ctx, GL_INVALID_ENUM);
				return;
			}
			if (texture->image_view_state.swizzle[2] != (GLenum)param)
			{
				texture->image_view_state.swizzle[2] = param;
				texture->dirty |= GL_TEXTURE_DIRTY_IMAGE_VIEW;
			}
			break;
		case GL_TEXTURE_SWIZZLE_A:
			if (!is_valid_swizzle(param))
			{
				GL_SET_ERR(ctx, GL_INVALID_ENUM);
				return;
			}
			if (texture->image_view_state.swizzle[3] != (GLenum)param)
			{
				texture->image_view_state.swizzle[3] = param;
				texture->dirty |= GL_TEXTURE_DIRTY_IMAGE_VIEW;
			}
			break;
		case GL_TEXTURE_SWIZZLE_RGBA:
			if (!is_valid_swizzle(param))
			{
				GL_SET_ERR(ctx, GL_INVALID_ENUM);
				return;
			}
			if (texture->image_view_state.swizzle[0] != (GLenum)param)
			{
				texture->image_view_state.swizzle[0] = param;
				texture->dirty |= GL_TEXTURE_DIRTY_IMAGE_VIEW;
			}
			if (texture->image_view_state.swizzle[1] != (GLenum)param)
			{
				texture->image_view_state.swizzle[1] = param;
				texture->dirty |= GL_TEXTURE_DIRTY_IMAGE_VIEW;
			}
			if (texture->image_view_state.swizzle[2] != (GLenum)param)
			{
				texture->image_view_state.swizzle[2] = param;
				texture->dirty |= GL_TEXTURE_DIRTY_IMAGE_VIEW;
			}
			if (texture->image_view_state.swizzle[3] != (GLenum)param)
			{
				texture->image_view_state.swizzle[3] = param;
				texture->dirty |= GL_TEXTURE_DIRTY_IMAGE_VIEW;
			}
			break;
		case GL_TEXTURE_MAX_ANISOTROPY:
			if (sampler_state_set_max_anisotropy(ctx,
			                                     &texture->sampler_state,
			                                     param))
				texture->dirty |= GL_TEXTURE_DIRTY_SAMPLER;
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
}

void
glTexParameteri(GLenum target, GLenum pname, GLint param)
{
	struct gl_ctx *ctx = g_ctx;
	struct texture *texture;

	texture = get_texture(ctx, target);
	if (!texture)
		return;
	texture_parameteri(ctx, texture, pname, param);
}

void
glTextureParameteri(GLuint id,
                    GLenum pname,
                    GLint param)
{
	struct gl_ctx *ctx = g_ctx;
	struct texture *texture;

	texture = object_get(ctx, id, OBJECT_TEXTURE);
	if (!texture)
		return;
	texture_parameteri(ctx, texture, pname, param);
}

void
glTexParameterf(GLenum target,
                GLenum pname,
                GLfloat param)
{
	struct gl_ctx *ctx = g_ctx;
	struct texture *texture;

	texture = get_texture(ctx, target);
	if (!texture)
		return;
	texture_parameterf(ctx, texture, pname, param);
}

void
glTextureParameterf(GLuint id,
                    GLenum pname,
                    GLfloat param)
{
	struct gl_ctx *ctx = g_ctx;
	struct texture *texture;

	texture = object_get(ctx, id, OBJECT_TEXTURE);
	if (!texture)
		return;
	texture_parameterf(ctx, texture, pname, param);
}

static void
texture_parameteriv(struct gl_ctx *ctx,
                    struct texture *texture,
                    GLenum pname,
                    const GLint *params)
{
	switch (pname)
	{
		case GL_TEXTURE_SWIZZLE_RGBA:
			texture->image_view_state.swizzle[0] = params[0];
			texture->image_view_state.swizzle[1] = params[1];
			texture->image_view_state.swizzle[2] = params[2];
			texture->image_view_state.swizzle[3] = params[3];
			break;
		default:
			texture_parameteri(ctx, texture, pname, params[0]);
			return;
	}
}

void
glTexParameteriv(GLuint target,
                 GLenum pname,
                 const GLint *params)
{
	struct gl_ctx *ctx = g_ctx;
	struct texture *texture;

	texture = get_texture(ctx, target);
	if (!texture)
		return;
	texture_parameteriv(ctx, texture, pname, params);
}

void
glTextureParameteriv(GLuint id,
                     GLenum pname,
                     const GLint *params)
{
	struct gl_ctx *ctx = g_ctx;
	struct texture *texture;

	texture = object_get(ctx, id, OBJECT_TEXTURE);
	if (!texture)
		return;
	texture_parameteriv(ctx, texture, pname, params);
}

static void
texture_parameterfv(struct gl_ctx *ctx,
                    struct texture *texture,
                    GLenum pname,
                    const GLfloat *params)
{
	switch (pname)
	{
		case GL_TEXTURE_BORDER_COLOR:
			if (sampler_state_set_border_color(ctx,
			                                   &texture->sampler_state,
			                                   params))
				texture->dirty |= GL_TEXTURE_DIRTY_SAMPLER;
			break;
		default:
			texture_parameterf(ctx, texture, pname, params[0]);
			return;
	}
}

void
glTexParameterfv(GLuint target,
                 GLenum pname,
                 const GLfloat *params)
{
	struct gl_ctx *ctx = g_ctx;
	struct texture *texture;

	texture = get_texture(ctx, target);
	if (!texture)
		return;
	texture_parameterfv(ctx, texture, pname, params);
}

void
glTextureParameterfv(GLuint id,
                     GLenum pname,
                     const GLfloat *params)
{
	struct gl_ctx *ctx = g_ctx;
	struct texture *texture;

	texture = object_get(ctx, id, OBJECT_TEXTURE);
	if (!texture)
		return;
	texture_parameterfv(ctx, texture, pname, params);
}

static void
get_texture_parameteriv(struct gl_ctx *ctx,
                        struct texture *texture,
                        GLenum pname,
                        GLint *params)
{
	switch (pname)
	{
		case GL_TEXTURE_MAG_FILTER:
			*params = texture->sampler_state.mag_filter;
			break;
		case GL_TEXTURE_MIN_FILTER:
			*params = texture->sampler_state.min_filter;
			break;
		case GL_TEXTURE_BASE_LEVEL:
			*params = texture->image_view_state.min_level;
			break;
		case GL_TEXTURE_MAX_LEVEL:
			*params = texture->image_view_state.max_level;
			break;
		case GL_TEXTURE_SWIZZLE_R:
			*params = texture->image_view_state.swizzle[0];
			break;
		case GL_TEXTURE_SWIZZLE_G:
			*params = texture->image_view_state.swizzle[1];
			break;
		case GL_TEXTURE_SWIZZLE_B:
			*params = texture->image_view_state.swizzle[2];
			break;
		case GL_TEXTURE_SWIZZLE_A:
			*params = texture->image_view_state.swizzle[3];
			break;
		case GL_TEXTURE_SWIZZLE_RGBA:
			params[0] = texture->image_view_state.swizzle[0];
			params[1] = texture->image_view_state.swizzle[1];
			params[2] = texture->image_view_state.swizzle[2];
			params[3] = texture->image_view_state.swizzle[3];
			break;
		case GL_TEXTURE_WRAP_S:
			*params = texture->sampler_state.wrap_s;
			break;
		case GL_TEXTURE_WRAP_T:
			*params = texture->sampler_state.wrap_t;
			break;
		case GL_TEXTURE_WRAP_R:
			*params = texture->sampler_state.wrap_r;
			break;
		case GL_TEXTURE_COMPARE_MODE:
			*params = texture->sampler_state.compare_mode;
			break;
		case GL_TEXTURE_COMPARE_FUNC:
			*params = texture->sampler_state.compare_func;
			break;
		case GL_TEXTURE_TARGET:
			*params = texture->target;
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
}

void
glGetTexParameteriv(GLenum target,
                    GLenum pname,
                    GLint *params)
{
	struct gl_ctx *ctx = g_ctx;
	struct texture *texture;

	texture = get_texture(ctx, target);
	if (!texture)
		return;
	get_texture_parameteriv(ctx, texture, pname, params);
}

void
glGetTextureParameteriv(GLuint id,
                        GLenum pname,
                        GLint *params)
{
	struct gl_ctx *ctx = g_ctx;
	struct texture *texture;

	texture = object_get(ctx, id, OBJECT_TEXTURE);
	if (!texture)
		return;
	get_texture_parameteriv(ctx, texture, pname, params);
}

static void
get_texture_parameterfv(struct gl_ctx *ctx,
                        struct texture *texture,
                        GLenum pname,
                        GLfloat *params)
{
	switch (pname)
	{
		case GL_TEXTURE_BORDER_COLOR:
			params[0] = texture->sampler_state.border_color[0];
			params[1] = texture->sampler_state.border_color[1];
			params[2] = texture->sampler_state.border_color[2];
			params[3] = texture->sampler_state.border_color[3];
			break;
		case GL_TEXTURE_MIN_LOD:
			*params = texture->sampler_state.min_lod;
			break;
		case GL_TEXTURE_MAX_LOD:
			*params = texture->sampler_state.max_lod;
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
}

void
glGetTexParameterfv(GLenum target,
                    GLenum pname,
                    GLfloat *params)
{
	struct gl_ctx *ctx = g_ctx;
	struct texture *texture;

	texture = get_texture(ctx, target);
	if (!texture)
		return;
	get_texture_parameterfv(ctx, texture, pname, params);
}

void
glGetTextureParameterfv(GLuint id,
                        GLenum pname,
                        GLfloat *params)
{
	struct gl_ctx *ctx = g_ctx;
	struct texture *texture;

	texture = object_get(ctx, id, OBJECT_TEXTURE);
	if (!texture)
		return;
	get_texture_parameterfv(ctx, texture, pname, params);
}

static GLboolean
get_image_format(GLenum format, GLenum type, enum jkg_format *formatp)
{
	switch (format)
	{
		case GL_RED:
			switch (type)
			{
				case GL_UNSIGNED_BYTE:
					*formatp = JKG_FORMAT_R8_UNORM;
					return true;
				case GL_BYTE:
					*formatp = JKG_FORMAT_R8_SNORM;
					return true;
				case GL_UNSIGNED_SHORT:
					*formatp = JKG_FORMAT_R16_UNORM;
					return true;
				case GL_SHORT:
					*formatp = JKG_FORMAT_R16_SNORM;
					return true;
				case GL_HALF_FLOAT:
					*formatp = JKG_FORMAT_R16_SFLOAT;
					return true;
				case GL_FLOAT:
					*formatp = JKG_FORMAT_R32_SFLOAT;
					return true;
				default:
					return false;
			}
			break;
		case GL_RG:
			switch (type)
			{
				case GL_UNSIGNED_BYTE:
					*formatp = JKG_FORMAT_R8G8_UNORM;
					return true;
				case GL_BYTE:
					*formatp = JKG_FORMAT_R8G8_SNORM;
					return true;
				case GL_UNSIGNED_SHORT:
					*formatp = JKG_FORMAT_R16G16_UNORM;
					return true;
				case GL_SHORT:
					*formatp = JKG_FORMAT_R16G16_SNORM;
					return true;
				case GL_HALF_FLOAT:
					*formatp = JKG_FORMAT_R16G16_SFLOAT;
					return true;
				case GL_FLOAT:
					*formatp = JKG_FORMAT_R32G32_SFLOAT;
					return true;
				default:
					return false;
			}
			break;
		case GL_RGB:
			switch (type)
			{
				case GL_UNSIGNED_BYTE:
					*formatp = JKG_FORMAT_R8G8B8_UNORM;
					return true;
				case GL_BYTE:
					*formatp = JKG_FORMAT_R8G8B8_SNORM;
					return true;
				case GL_UNSIGNED_SHORT:
					*formatp = JKG_FORMAT_R16G16B16_UNORM;
					return true;
				case GL_SHORT:
					*formatp = JKG_FORMAT_R16G16B16_SNORM;
					return true;
				case GL_HALF_FLOAT:
					*formatp = JKG_FORMAT_R16G16B16_SFLOAT;
					return true;
				case GL_FLOAT:
					*formatp = JKG_FORMAT_R32G32B32_SFLOAT;
					return true;
				case GL_UNSIGNED_SHORT_5_6_5:
					*formatp = JKG_FORMAT_R5G6B5_UNORM_PACK16;
					return true;
				case GL_UNSIGNED_SHORT_5_6_5_REV:
					*formatp = JKG_FORMAT_B5G6R5_UNORM_PACK16;
					return true;
				default:
					return false;
			}
			break;
		case GL_BGR:
			switch (type)
			{
				case GL_UNSIGNED_BYTE:
					*formatp = JKG_FORMAT_B8G8R8_UNORM;
					return true;
				case GL_BYTE:
					*formatp = JKG_FORMAT_B8G8R8_SNORM;
					return true;
				default:
					return false;
			}
			break;
		case GL_RGBA:
			switch (type)
			{
				case GL_UNSIGNED_BYTE:
					*formatp = JKG_FORMAT_R8G8B8A8_UNORM;
					return true;
				case GL_BYTE:
					*formatp = JKG_FORMAT_R8G8B8A8_SNORM;
					return true;
				case GL_UNSIGNED_SHORT:
					*formatp = JKG_FORMAT_R16G16B16A16_UNORM;
					return true;
				case GL_SHORT:
					*formatp = JKG_FORMAT_R16G16B16A16_SNORM;
					return true;
				case GL_HALF_FLOAT:
					*formatp = JKG_FORMAT_R16G16B16A16_SFLOAT;
					return true;
				case GL_FLOAT:
					*formatp = JKG_FORMAT_R32G32B32A32_SFLOAT;
					return true;
				case GL_UNSIGNED_SHORT_4_4_4_4:
					*formatp = JKG_FORMAT_R4G4B4A4_UNORM_PACK16;
					return true;
				case GL_UNSIGNED_SHORT_4_4_4_4_REV:
					*formatp = JKG_FORMAT_B4G4R4A4_UNORM_PACK16;
					return true;
				case GL_UNSIGNED_SHORT_5_5_5_1:
					*formatp = JKG_FORMAT_R5G5B5A1_UNORM_PACK16;
					return true;
				case GL_UNSIGNED_SHORT_1_5_5_5_REV:
					*formatp = JKG_FORMAT_A1B5G5R5_UNORM_PACK16;
					return true;
				default:
					return false;
			}
			break;
		case GL_BGRA:
			switch (type)
			{
				case GL_UNSIGNED_BYTE:
					*formatp = JKG_FORMAT_B8G8R8A8_UNORM;
					return true;
				case GL_BYTE:
					*formatp = JKG_FORMAT_B8G8R8A8_SNORM;
					return true;
				default:
					return false;
			}
			break;
		case GL_RED_INTEGER:
			switch (type)
			{
				case GL_UNSIGNED_BYTE:
					*formatp = JKG_FORMAT_R8_UINT;
					return true;
				case GL_BYTE:
					*formatp = JKG_FORMAT_R8_SINT;
					return true;
				case GL_UNSIGNED_SHORT:
					*formatp = JKG_FORMAT_R16_UINT;
					return true;
				case GL_SHORT:
					*formatp = JKG_FORMAT_R16_SINT;
					return true;
				case GL_UNSIGNED_INT:
					*formatp = JKG_FORMAT_R32_UINT;
					return true;
				case GL_INT:
					*formatp = JKG_FORMAT_R32_SINT;
					return true;
				default:
					return false;
			}
			break;
		case GL_RG_INTEGER:
			switch (type)
			{
				case GL_UNSIGNED_BYTE:
					*formatp = JKG_FORMAT_R8G8_UINT;
					return true;
				case GL_BYTE:
					*formatp = JKG_FORMAT_R8G8_SINT;
					return true;
				case GL_UNSIGNED_SHORT:
					*formatp = JKG_FORMAT_R16G16_UINT;
					return true;
				case GL_SHORT:
					*formatp = JKG_FORMAT_R16G16_SINT;
					return true;
				case GL_UNSIGNED_INT:
					*formatp = JKG_FORMAT_R32G32_UINT;
					return true;
				case GL_INT:
					*formatp = JKG_FORMAT_R32G32_SINT;
					return true;
				default:
					return false;
			}
			break;
		case GL_RGB_INTEGER:
			switch (type)
			{
				case GL_UNSIGNED_BYTE:
					*formatp = JKG_FORMAT_R8G8B8_UINT;
					return true;
				case GL_BYTE:
					*formatp = JKG_FORMAT_R8G8B8_SINT;
					return true;
				case GL_UNSIGNED_SHORT:
					*formatp = JKG_FORMAT_R16G16B16_UINT;
					return true;
				case GL_SHORT:
					*formatp = JKG_FORMAT_R16G16B16_SINT;
					return true;
				case GL_UNSIGNED_INT:
					*formatp = JKG_FORMAT_R32G32B32_UINT;
					return true;
				case GL_INT:
					*formatp = JKG_FORMAT_R32G32B32_SINT;
					return true;
				default:
					return false;
			}
			break;
		case GL_BGR_INTEGER:
			switch (type)
			{
				case GL_UNSIGNED_BYTE:
					*formatp = JKG_FORMAT_B8G8R8_UINT;
					return true;
				case GL_BYTE:
					*formatp = JKG_FORMAT_B8G8R8_SINT;
					return true;
				default:
					return false;
			}
			break;
		case GL_RGBA_INTEGER:
			switch (type)
			{
				case GL_UNSIGNED_BYTE:
					*formatp = JKG_FORMAT_R8G8B8A8_UINT;
					return true;
				case GL_BYTE:
					*formatp = JKG_FORMAT_R8G8B8A8_SINT;
					return true;
				case GL_UNSIGNED_SHORT:
					*formatp = JKG_FORMAT_R16G16B16A16_UINT;
					return true;
				case GL_SHORT:
					*formatp = JKG_FORMAT_R16G16B16A16_SINT;
					return true;
				case GL_UNSIGNED_INT:
					*formatp = JKG_FORMAT_R32G32B32A32_UINT;
					return true;
				case GL_INT:
					*formatp = JKG_FORMAT_R32G32B32A32_SINT;
					return true;
				default:
					return false;
			}
			break;
		case GL_BGRA_INTEGER:
			switch (type)
			{
				case GL_UNSIGNED_BYTE:
					*formatp = JKG_FORMAT_B8G8R8A8_UINT;
					return true;
				case GL_BYTE:
					*formatp = JKG_FORMAT_B8G8R8A8_SINT;
					return true;
				default:
					return false;
			}
			break;
		case GL_STENCIL_INDEX:
			switch (type)
			{
				default:
					return false;
			}
			break;
		case GL_DEPTH_COMPONENT:
			switch (type)
			{
				default:
					return false;
			}
			break;
		case GL_DEPTH_STENCIL:
			switch (type)
			{
				case GL_UNSIGNED_INT_24_8:
					*formatp = JKG_FORMAT_D24_UNORM_S8_UINT;
					return true;
				default:
					return false;
			}
			break;
		default:
			return false;
	}
}

static bool
create_image(struct gl_ctx *ctx,
             GLenum target,
             GLuint width,
             GLuint height,
             GLuint depth,
             GLuint levels,
             GLuint layers,
             enum jkg_format format,
             struct jkg_image **imagep)
{
	struct jkg_image_create_info create_info;
	int ret;

	switch (target)
	{
		case GL_TEXTURE_1D:
			create_info.type = JKG_IMAGE_1D;
			create_info.size.x = width;
			create_info.size.y = 0;
			create_info.size.z = 0;
			create_info.levels = levels;
			create_info.layers = 0;
			create_info.format = format;
			break;
		case GL_TEXTURE_1D_ARRAY:
			create_info.type = JKG_IMAGE_1D_ARRAY;
			create_info.size.x = width;
			create_info.size.y = 0;
			create_info.size.z = 0;
			create_info.levels = levels;
			create_info.layers = height;
			create_info.format = format;
			break;
		case GL_TEXTURE_2D:
			create_info.type = JKG_IMAGE_2D;
			create_info.size.x = width;
			create_info.size.y = height;
			create_info.size.z = 0;
			create_info.levels = levels;
			create_info.layers = 0;
			create_info.format = format;
			break;
		case GL_TEXTURE_2D_ARRAY:
			create_info.type = JKG_IMAGE_2D_ARRAY;
			create_info.size.x = width;
			create_info.size.y = height;
			create_info.size.z = 0;
			create_info.levels = levels;
			create_info.layers = depth;
			create_info.format = format;
			break;
		case GL_TEXTURE_3D:
			create_info.type = JKG_IMAGE_3D;
			create_info.size.x = width;
			create_info.size.y = height;
			create_info.size.z = depth;
			create_info.levels = levels;
			create_info.layers = 0;
			create_info.format = format;
			break;
		default:
			assert(!"unknown texture type");
			return false;
	}
	ret = ctx->jkg_op->image_alloc(ctx->jkg_ctx,
	                               &create_info,
	                               imagep);
	if (ret)
	{
		GL_SET_ERR(ctx, get_jkg_error(ret));
		return false;
	}
	return true;
}

static void
texture_image(struct gl_ctx *ctx,
              struct texture *texture,
              GLenum target,
              GLint level,
              GLint internalformat,
              GLsizei width,
              GLsizei height,
              GLsizei depth,
              GLint border,
              GLenum format,
              GLenum type,
              const GLvoid *data)
{
	struct jkg_image *image;
	enum jkg_format jkg_internal_format;
	enum jkg_format jkg_format;
	uint32_t max_sizes[3];
	int ret;

	/* XXX
	 * this function should, instead of allocating an image,
	 * add the storage for a single lod layer
	 * on context sync, if there has been a call to glTexImage2D,
	 * the jkg image must be recreated
	 */
	if (texture->immutable)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	get_texture_max_sizes(ctx, texture->target, max_sizes);
	if (border
	 || !test_bounds(level,
	                 0,
	                 0,
	                 0,
	                 width,
	                 height,
	                 depth,
	                 UINT32_MAX,
	                 max_sizes[0],
	                 max_sizes[1],
	                 max_sizes[2]))
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	if (!get_image_internal_format(internalformat, &jkg_internal_format))
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	if (!get_image_format(format, type, &jkg_format))
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	/* XXX assert that we're not doing color / depth / stencil conversion */
	if (!create_image(ctx,
	                  target,
	                  width,
	                  height,
	                  depth,
	                  level + 1,
	                  0,
	                  jkg_internal_format,
	                  &image))
		return;
	if (data)
	{
		struct jkg_extent offset;
		struct jkg_extent size;

		offset.x = 0;
		offset.y = 0;
		offset.z = 0;
		size.x = width;
		size.y = height;
		size.z = depth;
		ret = ctx->jkg_op->image_write(ctx->jkg_ctx,
		                               image,
		                               level,
		                               data,
		                               jkg_format,
		                               &size,
		                               &offset);
		if (ret)
		{
			ctx->jkg_op->image_free(ctx->jkg_ctx,
			                        image);
			GL_SET_ERR(ctx, get_jkg_error(ret));
			return;
		}
	}
	release_image_view(ctx, texture);
	texture->image = image;
	texture->image_view = NULL;
	texture->dirty |= GL_TEXTURE_DIRTY_IMAGE_VIEW
	                | GL_TEXTURE_DIRTY_SURFACE;
	texture->width = width;
	texture->height = height;
	texture->depth = depth;
	texture->levels = level + 1;
	texture->format = internalformat;
	texture->jkg_format = jkg_internal_format;
}

static void
texture_storage(struct gl_ctx *ctx,
                struct texture *texture,
                GLsizei levels,
                GLenum internalformat,
                GLsizei width,
                GLsizei height,
                GLsizei depth)
{
	enum jkg_format jkg_internal_format;
	uint32_t max_sizes[3];
	struct jkg_image *image;

	get_texture_max_sizes(ctx, texture->target, max_sizes);
	if (!test_bounds(levels,
	                 0,
	                 0,
	                 0,
	                 width,
	                 height,
	                 depth,
	                 UINT32_MAX,
	                 max_sizes[0],
	                 max_sizes[1],
	                 max_sizes[2]))
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	if (!get_image_internal_format(internalformat, &jkg_internal_format))
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	if (!create_image(ctx,
	                  texture->target,
	                  width,
	                  height,
	                  depth,
	                  levels,
	                  0,
	                  jkg_internal_format,
	                  &image))
		return;
	release_image_view(ctx, texture);
	texture->image = image;
	texture->image_view = NULL;
	texture->dirty |= GL_TEXTURE_DIRTY_IMAGE_VIEW
	                | GL_TEXTURE_DIRTY_SURFACE;
	texture->width = width;
	texture->height = height;
	texture->depth = depth;
	texture->levels = levels;
	texture->format = internalformat;
	texture->jkg_format = jkg_internal_format;
	texture->immutable = GL_TRUE;
}

static void
texture_sub_image(struct gl_ctx *ctx,
                  struct texture *texture,
                  GLint level,
                  GLint xoffset,
                  GLint yoffset,
                  GLint zoffset,
                  GLsizei width,
                  GLsizei height,
                  GLsizei depth,
                  GLenum format,
                  GLenum type,
                  const GLvoid *pixels)
{
	struct jkg_extent size;
	struct jkg_extent offset;
	enum jkg_format jkg_format;
	int ret;

	if (!test_bounds(level,
	                 xoffset,
	                 yoffset,
	                 zoffset,
	                 width,
	                 height,
	                 depth,
	                 texture->levels,
	                 texture->width,
	                 texture->height,
	                 texture->depth))
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	if (!get_image_format(format, type, &jkg_format))
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	/* XXX assert that we're not doing color / depth / stencil conversion */
	if (!texture->image)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	offset.x = xoffset;
	offset.y = yoffset;
	offset.z = zoffset;
	size.x = width;
	size.y = height;
	size.z = depth;
	ret = ctx->jkg_op->image_write(ctx->jkg_ctx,
	                               texture->image,
	                               level,
	                               pixels,
	                               jkg_format,
	                               &size,
	                               &offset);
	if (ret)
	{
		GL_SET_ERR(ctx, get_jkg_error(ret));
		return;
	}
}

void
glTexImage1D(GLenum target,
             GLint level,
             GLint internalformat,
             GLsizei width,
             GLint border,
             GLenum format,
             GLenum type,
             const GLvoid *data)
{
	struct gl_ctx *ctx = g_ctx;
	struct texture *texture;

	if (target != GL_TEXTURE_1D)
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	if (!width) /* XXX really ? */
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	texture = get_texture(ctx, target);
	if (!texture)
		return;
	texture_image(ctx,
	              texture,
	              target,
	              level,
	              internalformat,
	              width,
	              0,
	              0,
	              border,
	              format,
	              type,
	              data);
}

void
glTexStorage1D(GLenum target,
               GLsizei levels,
               GLenum internalformat,
               GLsizei width)
{
	struct gl_ctx *ctx = g_ctx;
	struct texture *texture;

	if (target != GL_TEXTURE_1D)
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	if (!width) /* XXX really ? */
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	texture = get_texture(ctx, target);
	if (!texture)
		return;
	texture_storage(ctx,
	                texture,
	                levels,
	                internalformat,
	                width,
	                0,
	                0);
}

void
glTextureStorage1D(GLuint id,
                   GLsizei levels,
                   GLenum internalformat,
                   GLsizei width)
{
	struct gl_ctx *ctx = g_ctx;
	struct texture *texture;

	if (!width) /* XXX really ? */
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	texture = object_get(ctx, id, OBJECT_TEXTURE);
	if (!texture)
		return;
	if (texture->target != GL_TEXTURE_1D)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	texture_storage(ctx,
	                texture,
	                levels,
	                internalformat,
	                width,
	                0,
	                0);
}

void
glTexSubImage1D(GLenum target,
                GLint level,
                GLint xoffset,
                GLsizei width,
                GLenum format,
                GLenum type,
                const GLvoid *pixels)
{
	struct gl_ctx *ctx = g_ctx;
	struct texture *texture;

	if (target != GL_TEXTURE_1D)
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	texture = get_texture(ctx, target);
	if (!texture)
		return;
	texture_sub_image(ctx,
	                  texture,
	                  level,
	                  xoffset,
	                  0,
	                  0,
	                  width,
	                  0,
	                  0,
	                  format,
	                  type,
	                  pixels);
}

void
glTextureSubImage1D(GLuint id,
                    GLint level,
                    GLint xoffset,
                    GLsizei width,
                    GLenum format,
                    GLenum type,
                    const GLvoid *pixels)
{
	struct gl_ctx *ctx = g_ctx;
	struct texture *texture;

	texture = object_get(ctx, id, OBJECT_TEXTURE);
	if (!texture)
		return;
	if (texture->target != GL_TEXTURE_1D)
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	texture_sub_image(ctx,
	                  texture,
	                  level,
	                  xoffset,
	                  0,
	                  0,
	                  width,
	                  0,
	                  0,
	                  format,
	                  type,
	                  pixels);
}

void
glTexImage2D(GLenum target,
             GLint level,
             GLint internalformat,
             GLsizei width,
             GLsizei height,
             GLint border,
             GLenum format,
             GLenum type,
             const GLvoid *data)
{
	struct gl_ctx *ctx = g_ctx;
	struct texture *texture;

	texture = get_texture(ctx, target);
	if (!texture)
		return;
	if (target != GL_TEXTURE_2D
	 && target != GL_TEXTURE_1D_ARRAY)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	if (!width
	 || !height) /* XXX really ? */
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	texture_image(ctx,
	              texture,
	              target,
	              level,
	              internalformat,
	              width,
	              height,
	              0,
	              border,
	              format,
	              type,
	              data);
}

void
glTexStorage2D(GLenum target,
               GLsizei levels,
               GLenum internalformat,
               GLsizei width,
               GLsizei height)
{
	struct gl_ctx *ctx = g_ctx;
	struct texture *texture;

	if (target != GL_TEXTURE_2D
	 && target != GL_TEXTURE_1D_ARRAY)
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	if (!width
	 || !height) /* XXX really ? */
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	texture = get_texture(ctx, target);
	if (!texture)
		return;
	texture_storage(ctx,
	                texture,
	                levels,
	                internalformat,
	                width,
	                height,
	                0);
}

void
glTextureStorage2D(GLuint id,
                   GLsizei levels,
                   GLenum internalformat,
                   GLsizei width,
                   GLsizei height)
{
	struct gl_ctx *ctx = g_ctx;
	struct texture *texture;

	if (!width
	 || !height) /* XXX really ? */
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	texture = object_get(ctx, id, OBJECT_TEXTURE);
	if (!texture)
		return;
	if (texture->target != GL_TEXTURE_2D
	 && texture->target != GL_TEXTURE_1D_ARRAY)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	texture_storage(ctx,
	                texture,
	                levels,
	                internalformat,
	                width,
	                height,
	                0);
}

void
glTexSubImage2D(GLenum target,
                GLint level,
                GLint xoffset,
                GLint yoffset,
                GLsizei width,
                GLsizei height,
                GLenum format,
                GLenum type,
                const GLvoid *pixels)
{
	struct gl_ctx *ctx = g_ctx;
	struct texture *texture;

	if (target != GL_TEXTURE_2D
	 && target != GL_TEXTURE_1D_ARRAY)
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	texture = get_texture(ctx, target);
	if (!texture)
		return;
	texture_sub_image(ctx,
	                  texture,
	                  level,
	                  xoffset,
	                  yoffset,
	                  0,
	                  width,
	                  height,
	                  0,
	                  format,
	                  type,
	                  pixels);
}

void
glTextureSubImage2D(GLuint id,
                    GLint level,
                    GLint xoffset,
                    GLint yoffset,
                    GLsizei width,
                    GLsizei height,
                    GLenum format,
                    GLenum type,
                    const GLvoid *pixels)
{
	struct gl_ctx *ctx = g_ctx;
	struct texture *texture;

	texture = object_get(ctx, id, OBJECT_TEXTURE);
	if (!texture)
		return;
	if (texture->target != GL_TEXTURE_2D
	 && texture->target != GL_TEXTURE_1D_ARRAY)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	texture_sub_image(ctx,
	                  texture,
	                  level,
	                  xoffset,
	                  yoffset,
	                  0,
	                  width,
	                  height,
	                  0,
	                  format,
	                  type,
	                  pixels);
}

void
glTexImage3D(GLenum target,
             GLint level,
             GLint internalformat,
             GLsizei width,
             GLsizei height,
             GLsizei depth,
             GLint border,
             GLenum format,
             GLenum type,
             const GLvoid *data)
{
	struct gl_ctx *ctx = g_ctx;
	struct texture *texture;

	if (target != GL_TEXTURE_3D
	 && target != GL_TEXTURE_2D_ARRAY)
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	if (!width
	 || !height
	 || !depth) /* XXX really ? */
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	texture = get_texture(ctx, target);
	if (!texture)
		return;
	texture_image(ctx,
	              texture,
	              target,
	              level,
	              internalformat,
	              width,
	              height,
	              depth,
	              border,
	              format,
	              type,
	              data);
}

void
glTexStorage3D(GLenum target,
               GLsizei levels,
               GLenum internalformat,
               GLsizei width,
               GLsizei height,
               GLsizei depth)
{
	struct gl_ctx *ctx = g_ctx;
	struct texture *texture;

	if (target != GL_TEXTURE_3D
	 && target != GL_TEXTURE_2D_ARRAY)
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	if (!width
	 || !height
	 || !depth) /* XXX really ? */
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	texture = get_texture(ctx, target);
	if (!texture)
		return;
	texture_storage(ctx,
	                texture,
	                levels,
	                internalformat,
	                width,
	                height,
	                depth);
}

void
glTextureStorage3D(GLuint id,
                   GLsizei levels,
                   GLenum internalformat,
                   GLsizei width,
                   GLsizei height,
                   GLsizei depth)
{
	struct gl_ctx *ctx = g_ctx;
	struct texture *texture;

	if (!width
	 || !height
	 || !depth) /* XXX really ? */
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	texture = object_get(ctx, id, OBJECT_TEXTURE);
	if (!texture)
		return;
	if (texture->target != GL_TEXTURE_3D
	 && texture->target != GL_TEXTURE_2D_ARRAY)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	texture_storage(ctx,
	                texture,
	                levels,
	                internalformat,
	                width,
	                height,
	                depth);
}

void
glTexSubImage3D(GLenum target,
                GLint level,
                GLint xoffset,
                GLint yoffset,
                GLint zoffset,
                GLsizei width,
                GLsizei height,
                GLsizei depth,
                GLenum format,
                GLenum type,
                const GLvoid *pixels)
{
	struct gl_ctx *ctx = g_ctx;
	struct texture *texture;

	if (target != GL_TEXTURE_3D
	 && target != GL_TEXTURE_2D_ARRAY)
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	texture = get_texture(ctx, target);
	if (!texture)
		return;
	texture_sub_image(ctx,
	                  texture,
	                  level,
	                  xoffset,
	                  yoffset,
	                  zoffset,
	                  width,
	                  height,
	                  depth,
	                  format,
	                  type,
	                  pixels);
}

void
glTextureSubImage3D(GLuint id,
                    GLint level,
                    GLint xoffset,
                    GLint yoffset,
                    GLint zoffset,
                    GLsizei width,
                    GLsizei height,
                    GLsizei depth,
                    GLenum format,
                    GLenum type,
                    const GLvoid *pixels)
{
	struct gl_ctx *ctx = g_ctx;
	struct texture *texture;

	texture = object_get(ctx, id, OBJECT_TEXTURE);
	if (!texture)
		return;
	if (texture->target != GL_TEXTURE_3D
	 && texture->target != GL_TEXTURE_2D_ARRAY)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	texture_sub_image(ctx,
	                  texture,
	                  level,
	                  xoffset,
	                  yoffset,
	                  zoffset,
	                  width,
	                  height,
	                  depth,
	                  format,
	                  type,
	                  pixels);
}

void
glActiveTexture(GLenum texture)
{
	struct gl_ctx *ctx = g_ctx;

	if (texture < GL_TEXTURE0
	 || texture >= GL_TEXTURE0 + ctx->jkg_caps->max_texture_image_units)
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	ctx->active_texture = texture;
}

static bool
get_copy_image(struct gl_ctx *ctx,
               GLuint name,
               GLenum target,
               GLint level,
               GLint xoffset,
               GLint yoffset,
               GLint zoffset,
               GLsizei width,
               GLsizei height,
               GLsizei depth,
               struct jkg_image **imagep)
{
	struct renderbuffer *renderbuffer;
	struct texture *texture;
	uint32_t image_levels;
	uint32_t image_width;
	uint32_t image_height;
	uint32_t image_depth;

	switch (target)
	{
		case GL_TEXTURE_1D:
		case GL_TEXTURE_1D_ARRAY:
		case GL_TEXTURE_2D:
		case GL_TEXTURE_2D_ARRAY:
		case GL_TEXTURE_3D:
			texture = object_get(ctx, name, OBJECT_TEXTURE);
			if (!texture)
				return false;
			*imagep = texture->image;
			image_levels = 1; /* XXX */
			image_width = texture->width;
			image_height = texture->height;
			image_depth = texture->depth;
			break;
		case GL_RENDERBUFFER:
			renderbuffer = object_get(ctx, name, OBJECT_RENDERBUFFER);
			if (!renderbuffer)
				return false;
			*imagep = renderbuffer->image;
			image_levels = 1; /* XXX */
			image_width = renderbuffer->width;
			image_height = renderbuffer->height;
			image_depth = 0;
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return false;
	}
	if (!test_bounds(level,
	                 xoffset,
	                 yoffset,
	                 zoffset,
	                 width,
	                 height,
	                 depth,
	                 image_levels,
	                 image_width,
	                 image_height,
	                 image_depth))
		return false;
	return true;
}

void
glCopyImageSubData(GLuint srcName,
                   GLenum srcTarget,
                   GLint srcLevel,
                   GLint srcX,
                   GLint srcY,
                   GLint srcZ,
                   GLuint dstName,
                   GLenum dstTarget,
                   GLint dstLevel,
                   GLint dstX,
                   GLint dstY,
                   GLint dstZ,
                   GLsizei srcWidth,
                   GLsizei srcHeight,
                   GLsizei srcDepth)
{
	struct gl_ctx *ctx = g_ctx;
	struct jkg_image *src;
	struct jkg_image *dst;
	struct jkg_extent src_offset;
	struct jkg_extent dst_offset;
	struct jkg_extent size;
	int ret;

	if (!get_copy_image(ctx,
	                    srcName,
	                    srcTarget,
	                    srcLevel,
	                    srcX,
	                    srcY,
	                    srcZ,
	                    srcWidth,
	                    srcHeight,
	                    srcDepth,
	                    &src)
	 || !get_copy_image(ctx,
	                    dstName,
	                    dstTarget,
	                    dstLevel,
	                    dstX,
	                    dstY,
	                    dstZ,
	                    srcWidth,
	                    srcHeight,
	                    srcDepth,
	                    &dst))
		return;
	if (!src || !dst)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	src_offset.x = srcX;
	src_offset.y = srcY;
	src_offset.z = srcZ;
	dst_offset.x = dstX;
	dst_offset.y = dstY;
	dst_offset.z = dstZ;
	size.x = srcWidth;
	size.y = srcHeight;
	size.z = srcDepth;
	ret = ctx->jkg_op->image_copy(ctx->jkg_ctx,
	                              dst,
	                              src,
	                              dstLevel,
	                              srcLevel,
	                              &dst_offset,
	                              &src_offset,
	                              &size);
	if (ret)
	{
		GL_SET_ERR(ctx, get_jkg_error(ret));
		return;
	}
}

static void
get_texture_sub_image(struct gl_ctx *ctx,
                      struct texture *texture,
                      GLint level,
                      GLint xoffset,
                      GLint yoffset,
                      GLint zoffset,
                      GLsizei width,
                      GLsizei height,
                      GLsizei depth,
                      GLenum format,
                      GLenum type,
                      GLsizei bufSize,
                      GLvoid *pixels)
{
	struct jkg_extent size;
	struct jkg_extent offset;
	enum jkg_format jkg_format;
	int ret;

	if (!test_bounds(level,
	                 xoffset,
	                 yoffset,
	                 zoffset,
	                 width,
	                 height,
	                 depth,
	                 texture->levels,
	                 texture->width,
	                 texture->height,
	                 texture->depth))
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	if (!get_image_format(format, type, &jkg_format))
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	if (!texture->image)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	if (bufSize > 0)
	{
		GLsizei bytes;

		bytes = texture->width;
		if (texture->height)
			bytes *= texture->height;
		if (texture->depth)
			bytes *= texture->depth;
		bytes *= jkg_get_stride(texture->format);
		if (bufSize < bytes)
		{
			GL_SET_ERR(ctx, GL_INVALID_OPERATION);
			return;
		}
	}
	offset.x = xoffset;
	offset.y = yoffset;
	offset.z = zoffset;
	size.x = width;
	size.y = height;
	size.z = depth;
	ret = ctx->jkg_op->image_read(ctx->jkg_ctx,
	                              texture->image,
	                              level,
	                              pixels,
	                              jkg_format,
	                              &size,
	                              &offset);
	if (ret)
	{
		GL_SET_ERR(ctx, get_jkg_error(ret));
		return;
	}
}

void
glGetTextureSubImage(GLuint id,
                     GLint level,
                     GLint xoffset,
                     GLint yoffset,
                     GLint zoffset,
                     GLsizei width,
                     GLsizei height,
                     GLsizei depth,
                     GLenum format,
                     GLenum type,
                     GLsizei bufSize,
                     GLvoid *pixels)
{
	struct gl_ctx *ctx = g_ctx;
	struct texture *texture;

	if (bufSize < 0)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	texture = object_get(ctx, id, OBJECT_TEXTURE);
	if (!texture)
		return;
	get_texture_sub_image(ctx,
	                      texture,
	                      level,
	                      xoffset,
	                      yoffset,
	                      zoffset,
	                      width,
	                      height,
	                      depth,
	                      format,
	                      type,
	                      bufSize,
	                      pixels);
}

void
glGetTexImage(GLenum target,
              GLint level,
              GLenum format,
              GLenum type,
              GLvoid *pixels)
{
	struct gl_ctx *ctx = g_ctx;
	struct texture *texture;

	texture = get_texture(ctx, target);
	if (!texture)
		return;
	get_texture_sub_image(ctx,
	                      texture,
	                      level,
	                      0,
	                      0,
	                      0,
	                      texture->width,
	                      texture->height,
	                      texture->depth,
	                      format,
	                      type,
	                      -1,
	                      pixels);
}

void
glGetnTexImage(GLenum target,
               GLint level,
               GLenum format,
               GLenum type,
               GLsizei bufSize,
               GLvoid *pixels)
{
	struct gl_ctx *ctx = g_ctx;
	struct texture *texture;

	if (bufSize < 0)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	texture = get_texture(ctx, target);
	if (!texture)
		return;
	get_texture_sub_image(ctx,
	                      texture,
	                      level,
	                      0,
	                      0,
	                      0,
	                      texture->width,
	                      texture->height,
	                      texture->depth,
	                      format,
	                      type,
	                      bufSize,
	                      pixels);
}

void
glGetTextureImage(GLuint id,
                  GLint level,
                  GLenum format,
                  GLenum type,
                  GLsizei bufSize,
                  GLvoid *pixels)
{
	struct gl_ctx *ctx = g_ctx;
	struct texture *texture;

	if (bufSize < 0)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	texture = object_get(ctx, id, OBJECT_TEXTURE);
	if (!texture)
		return;
	get_texture_sub_image(ctx,
	                      texture,
	                      level,
	                      0,
	                      0,
	                      0,
	                      texture->width,
	                      texture->height,
	                      texture->depth,
	                      format,
	                      type,
	                      bufSize,
	                      pixels);
}
