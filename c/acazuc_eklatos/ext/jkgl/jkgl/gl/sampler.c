#include "internal.h"
#include "fnv.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

void
sampler_state_init(struct sampler_state *state)
{
	state->wrap_s = GL_REPEAT;
	state->wrap_t = GL_REPEAT;
	state->wrap_r = GL_REPEAT;
	state->min_filter = GL_NEAREST_MIPMAP_LINEAR;
	state->mag_filter = GL_LINEAR;
	state->compare_mode = GL_NONE;
	state->compare_func = GL_LEQUAL;
	state->lod_bias = 0;
	state->min_lod = -1000;
	state->max_lod = 1000;
	state->max_anisotropy = 0;
	state->border_color[0] = 0;
	state->border_color[1] = 0;
	state->border_color[2] = 0;
	state->border_color[3] = 0;
}

struct sampler *
sampler_alloc(struct gl_ctx *ctx)
{
	struct sampler *sampler;

	(void)ctx;
	sampler = calloc(1, sizeof(*sampler));
	if (!sampler)
		return NULL;
	sampler->object.type = OBJECT_TEXTURE;
	sampler->dirty = true;
	sampler_state_init(&sampler->state);
	sampler->sampler = NULL;
	return sampler;
}

void
glGenSamplers(GLsizei n, GLuint *samplers)
{
	struct gl_ctx *ctx = g_ctx;
	struct sampler *sampler;
	GLsizei i;

	if (n < 0)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	for (i = 0; i < n; ++i)
	{
		sampler = sampler_alloc(ctx);
		if (!sampler)
			goto err;
		if (!object_alloc(ctx, &sampler->object))
		{
			sampler_free(ctx, sampler);
			goto err;
		}
		samplers[i] = sampler->object.id;
	}
	return;

err:
	for (GLsizei j = 0; j < i; ++j)
	{
		sampler = object_get(ctx, samplers[j], OBJECT_SAMPLER);
		if (!sampler)
			continue;
		sampler_free(ctx, sampler);
	}
}

void
glCreateSamplers(GLsizei n, GLuint *samplers)
{
	glGenSamplers(n, samplers); /* XXX more? */
}

void
sampler_free(struct gl_ctx *ctx, struct sampler *sampler)
{
	if (sampler->sampler)
		cache_unref(ctx, &ctx->sampler_cache, &sampler->sampler->cache);
	object_free(ctx, &sampler->object);
}

void
glDeleteSamplers(GLsizei n, const GLuint *samplers)
{
	struct gl_ctx *ctx = g_ctx;

	if (n < 0)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	for (GLsizei i = 0; i < n; ++i)
	{
		GLuint id = samplers[i];
		struct sampler *sampler = object_get(ctx, id, OBJECT_SAMPLER);
		if (!sampler)
			continue;
		sampler_free(ctx, sampler);
	}
}

GLboolean
glIsSampler(GLuint id)
{
	return object_is(g_ctx, id, OBJECT_SAMPLER);
}

void
glBindSampler(GLuint unit, GLuint id)
{
	struct gl_ctx *ctx = g_ctx;
	struct sampler *sampler;

	if (unit > ctx->jkg_caps->max_texture_image_units) /* XXX another limit */
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	if (id)
	{
		sampler = object_get(ctx, id, OBJECT_SAMPLER);
		if (!sampler)
			return;
	}
	else
	{
		sampler = NULL;
	}
	if (ctx->samplers[unit] != sampler)
	{
		ctx->samplers[unit] = sampler;
		ctx->dirty |= GL_CTX_DIRTY_SAMPLERS;
	}
}

void
glBindSamplers(GLuint first, GLsizei count, const GLuint *samplers)
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
		struct sampler *sampler;

		if (samplers[i])
		{
			sampler = object_get(ctx, samplers[i], OBJECT_SAMPLER);
			if (!sampler)
			{
				GL_SET_ERR(ctx, GL_INVALID_OPERATION);
				return;
			}
		}
		else
		{
			sampler = NULL;
		}
		if (ctx->samplers[first + i] != sampler)
		{
			ctx->samplers[first + i] = sampler;
			ctx->dirty |= GL_CTX_DIRTY_SAMPLERS;
		}
	}
}

static bool
is_valid_wrap(GLenum v)
{
	switch (v)
	{
		case GL_CLAMP:
		case GL_CLAMP_TO_BORDER:
		case GL_CLAMP_TO_EDGE:
		case GL_MIRRORED_REPEAT:
		case GL_REPEAT:
			return true;
		default:
			return false;
	}
}

static bool
sampler_state_set_wrap(struct gl_ctx *ctx,
                       GLenum *wrapp,
                       GLenum value)
{
	if (!is_valid_wrap(value))
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return false;
	}
	if (*wrapp == value)
		return false;
	*wrapp = value;
	return true;
}

bool
sampler_state_set_wrap_s(struct gl_ctx *ctx,
                         struct sampler_state *state,
                         GLenum value)
{
	return sampler_state_set_wrap(ctx, &state->wrap_s, value);
}

bool
sampler_state_set_wrap_t(struct gl_ctx *ctx,
                         struct sampler_state *state,
                         GLenum value)
{
	return sampler_state_set_wrap(ctx, &state->wrap_t, value);
}

bool
sampler_state_set_wrap_r(struct gl_ctx *ctx,
                         struct sampler_state *state,
                         GLenum value)
{
	return sampler_state_set_wrap(ctx, &state->wrap_r, value);
}

static bool
is_valid_min_filter(GLenum v)
{
	switch (v)
	{
		case GL_NEAREST:
		case GL_LINEAR:
		case GL_CUBIC:
		case GL_NEAREST_MIPMAP_NEAREST:
		case GL_LINEAR_MIPMAP_NEAREST:
		case GL_CUBIC_MIPMAP_NEAREST:
		case GL_NEAREST_MIPMAP_LINEAR:
		case GL_LINEAR_MIPMAP_LINEAR:
		case GL_CUBIC_MIPMAP_LINEAR:
			return true;
		default:
			return false;
	}
}

bool
sampler_state_set_min_filter(struct gl_ctx *ctx,
                             struct sampler_state *state,
                             GLenum value)
{
	if (!is_valid_min_filter(value))
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return false;
	}
	if (state->min_filter == value)
		return false;
	state->min_filter = value;
	return true;
}

static bool
is_valid_mag_filter(GLenum v)
{
	switch (v)
	{
		case GL_NEAREST:
		case GL_LINEAR:
		case GL_CUBIC:
			return true;
		default:
			return false;
	}
}

bool
sampler_state_set_mag_filter(struct gl_ctx *ctx,
                             struct sampler_state *state,
                             GLenum value)
{
	if (!is_valid_mag_filter(value))
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return false;
	}
	if (state->mag_filter == value)
		return false;
	state->mag_filter = value;
	return true;
}

bool
sampler_state_set_lod_bias(struct gl_ctx *ctx,
                           struct sampler_state *state,
                           GLfloat value)
{
	(void)ctx;
	if (state->lod_bias == value)
		return false;
	state->lod_bias = value;
	return true;
}

bool
sampler_state_set_min_lod(struct gl_ctx *ctx,
                          struct sampler_state *state,
                          GLint value)
{
	(void)ctx;
	if (state->min_lod == value)
		return false;
	state->min_lod = value;
	return true;
}

bool
sampler_state_set_max_lod(struct gl_ctx *ctx,
                          struct sampler_state *state,
                          GLint value)
{
	(void)ctx;
	if (state->max_lod == value)
		return false;
	state->max_lod = value;
	return true;
}

static bool
is_valid_compare_mode(GLenum v)
{
	switch (v)
	{
		case GL_NONE:
		case GL_COMPARE_REF_TO_TEXTURE:
			return true;
		default:
			return false;
	}
}

bool
sampler_state_set_compare_mode(struct gl_ctx *ctx,
                               struct sampler_state *state,
                               GLenum value)
{
	if (!is_valid_compare_mode(value))
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return false;
	}
	if (state->compare_mode == value)
		return false;
	state->compare_mode = value;
	return true;
}

static bool
is_valid_compare_func(GLenum v)
{
	switch (v)
	{
		case GL_NEVER:
		case GL_LESS:
		case GL_LEQUAL:
		case GL_EQUAL:
		case GL_GEQUAL:
		case GL_GREATER:
		case GL_NOTEQUAL:
		case GL_ALWAYS:
			return true;
		default:
			return false;
	}
}

bool
sampler_state_set_compare_func(struct gl_ctx *ctx,
                               struct sampler_state *state,
                               GLenum value)
{
	if (!is_valid_compare_func(value))
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return false;
	}
	if (state->compare_func == value)
		return false;
	state->compare_func = value;
	return true;
}

bool
sampler_state_set_max_anisotropy(struct gl_ctx *ctx,
                                 struct sampler_state *state,
                                 GLuint value)
{
	if (value < 1
	 || value > ctx->jkg_caps->max_anisotropy)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return false;
	}
	if (state->max_anisotropy == value)
		return false;
	state->max_anisotropy = value;
	return true;
}

bool
sampler_state_set_border_color(struct gl_ctx *ctx,
                               struct sampler_state *state,
                               const GLfloat *color)
{
	bool ret = false;

	(void)ctx;
	for (GLint i = 0; i < 4; ++i)
	{
		GLfloat v = clampf(color[i], 0, 1);
		if (state->border_color[i] == v)
			continue;
		state->border_color[i] = v;
		ret = true;
	}
	return ret;
}

static void
sampler_parameterf(struct gl_ctx *ctx,
                   struct sampler *sampler,
                   GLenum pname,
                   GLfloat param)
{
	switch (pname)
	{
		case GL_TEXTURE_LOD_BIAS:
			if (sampler_state_set_lod_bias(ctx,
			                               &sampler->state,
			                               param))
				sampler->dirty = true;
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
}

static void
sampler_parameteri(struct gl_ctx *ctx,
                   struct sampler *sampler,
                   GLenum pname,
                   GLint param)
{
	switch (pname)
	{
		case GL_TEXTURE_WRAP_S:
			if (sampler_state_set_wrap_s(ctx,
			                             &sampler->state,
			                             param))
				sampler->dirty = true;
			break;
		case GL_TEXTURE_WRAP_T:
			if (sampler_state_set_wrap_t(ctx,
			                             &sampler->state,
			                             param))
				sampler->dirty = true;
			break;
		case GL_TEXTURE_WRAP_R:
			if (sampler_state_set_wrap_r(ctx,
			                             &sampler->state,
			                             param))
				sampler->dirty = true;
			break;
		case GL_TEXTURE_MIN_FILTER:
			if (sampler_state_set_min_filter(ctx,
			                                &sampler->state,
			                                param))
				sampler->dirty = true;
			break;
		case GL_TEXTURE_MAG_FILTER:
			if (sampler_state_set_mag_filter(ctx,
			                                &sampler->state,
			                                param))
				sampler->dirty = true;
			break;
		case GL_TEXTURE_LOD_BIAS:
			if (sampler_state_set_lod_bias(ctx,
			                               &sampler->state,
			                               param))
				sampler->dirty = true;
			break;
		case GL_TEXTURE_MIN_LOD:
			if (sampler_state_set_min_lod(ctx,
			                              &sampler->state,
			                              param))
				sampler->dirty = true;
			break;
		case GL_TEXTURE_MAX_LOD:
			if (sampler_state_set_max_lod(ctx,
			                              &sampler->state,
			                              param))
				sampler->dirty = true;
			break;
		case GL_TEXTURE_COMPARE_MODE:
			if (sampler_state_set_compare_mode(ctx,
			                                   &sampler->state,
			                                   param))
				sampler->dirty = true;
			break;
		case GL_TEXTURE_COMPARE_FUNC:
			if (sampler_state_set_compare_func(ctx,
			                                   &sampler->state,
			                                   param))
				sampler->dirty = true;
			break;
		case GL_TEXTURE_MAX_ANISOTROPY:
			if (sampler_state_set_max_anisotropy(ctx,
			                                     &sampler->state,
			                                     param))
				sampler->dirty = true;
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
}

void
glSamplerParameterf(GLuint id, GLenum pname, GLfloat param)
{
	struct gl_ctx *ctx = g_ctx;
	struct sampler *sampler;

	sampler = object_get(ctx, id, OBJECT_SAMPLER);
	if (!sampler)
		return;
	sampler_parameterf(ctx, sampler, pname, param);
}

void
glSamplerParameteri(GLuint id, GLenum pname, GLint param)
{
	struct gl_ctx *ctx = g_ctx;
	struct sampler *sampler;

	sampler = object_get(ctx, id, OBJECT_SAMPLER);
	if (!sampler)
		return;
	sampler_parameteri(ctx, sampler, pname, param);
}

void
glSamplerParameterfv(GLuint id, GLenum pname, const GLfloat *params)
{
	struct gl_ctx *ctx = g_ctx;
	struct sampler *sampler;

	sampler = object_get(ctx, id, OBJECT_SAMPLER);
	if (!sampler)
		return;
	switch (pname)
	{
		case GL_TEXTURE_BORDER_COLOR:
			if (sampler_state_set_border_color(ctx,
			                                   &sampler->state,
			                                   params))
				sampler->dirty = true;
			break;
		default:
			sampler_parameterf(ctx, sampler, pname, params[0]);
			return;
	}
}

void
glSamplerParameteriv(GLuint id, GLenum pname, const GLint *params)
{
	struct gl_ctx *ctx = g_ctx;
	struct sampler *sampler;

	sampler = object_get(ctx, id, OBJECT_SAMPLER);
	if (!sampler)
		return;
	switch (pname)
	{
		default:
			sampler_parameteri(ctx, sampler, pname, params[0]);
			return;
	}
}

void
glGetSamplerParameterfv(GLuint id, GLenum pname, GLfloat *params)
{
	struct gl_ctx *ctx = g_ctx;
	struct sampler *sampler;

	sampler = object_get(ctx, id, OBJECT_SAMPLER);
	if (!sampler)
		return;
	switch (pname)
	{
		case GL_TEXTURE_BORDER_COLOR:
			params[0] = sampler->state.border_color[0];
			params[1] = sampler->state.border_color[1];
			params[2] = sampler->state.border_color[2];
			params[3] = sampler->state.border_color[3];
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
}

void
glGetSamplerParameteriv(GLuint id, GLenum pname, GLint *params)
{
	struct gl_ctx *ctx = g_ctx;
	struct sampler *sampler;

	sampler = object_get(ctx, id, OBJECT_SAMPLER);
	if (!sampler)
		return;
	switch (pname)
	{
		case GL_TEXTURE_MAG_FILTER:
			*params = sampler->state.mag_filter;
			break;
		case GL_TEXTURE_MIN_FILTER:
			*params = sampler->state.min_filter;
			break;
		case GL_TEXTURE_WRAP_S:
			*params = sampler->state.wrap_s;
			break;
		case GL_TEXTURE_WRAP_T:
			*params = sampler->state.wrap_t;
			break;
		case GL_TEXTURE_WRAP_R:
			*params = sampler->state.wrap_r;
			break;
		case GL_TEXTURE_COMPARE_MODE:
			*params = sampler->state.compare_mode;
			break;
		case GL_TEXTURE_COMPARE_FUNC:
			*params = sampler->state.compare_func;
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
}

static enum jkg_wrap_mode
get_wrap_mode(GLenum mode)
{
	switch (mode)
	{
		default:
			assert(!"unknown wrap mode");
			/* FALLTHROUGH */
		case GL_REPEAT:
			return JKG_WRAP_REPEAT;
		case GL_MIRRORED_REPEAT:
			return JKG_WRAP_MIRRORED_REPEAT;
		case GL_CLAMP_TO_EDGE:
			return JKG_WRAP_CLAMP_TO_EDGE;
		case GL_CLAMP_TO_BORDER:
			return JKG_WRAP_CLAMP_TO_BORDER;
		case GL_MIRROR_CLAMP_TO_EDGE:
			return JKG_WRAP_MIRROR_CLAMP_TO_EDGE;
	}
}

static enum jkg_filter_mode
get_min_filter(GLenum min_filter)
{
	switch (min_filter)
	{
		default:
			assert(!"unknown min filter");
			/* FALLTHROUGH */
		case GL_NEAREST_MIPMAP_NEAREST:
		case GL_NEAREST_MIPMAP_LINEAR:
		case GL_NEAREST:
			return JKG_FILTER_NEAREST;
		case GL_LINEAR_MIPMAP_NEAREST:
		case GL_LINEAR_MIPMAP_LINEAR:
		case GL_LINEAR:
			return JKG_FILTER_LINEAR;
		case GL_CUBIC_MIPMAP_NEAREST:
		case GL_CUBIC_MIPMAP_LINEAR:
		case GL_CUBIC:
			return JKG_FILTER_CUBIC;
	}
}

static enum jkg_filter_mode
get_mag_filter(GLenum mag_filter)
{
	switch (mag_filter)
	{
		default:
			assert(!"unknown mag filter");
			/* FALLTHROUGH */
		case GL_NEAREST:
			return JKG_FILTER_NEAREST;
		case GL_LINEAR:
			return JKG_FILTER_LINEAR;
		case GL_CUBIC:
			return JKG_FILTER_CUBIC;
	}
}

static enum jkg_filter_mode
get_mip_filter(GLenum min_filter)
{
	switch (min_filter)
	{
		default:
			assert(!"unknown mip filter");
			/* FALLTHROUGH */
		case GL_NEAREST:
		case GL_LINEAR:
		case GL_CUBIC:
			return JKG_FILTER_NONE;
		case GL_NEAREST_MIPMAP_NEAREST:
		case GL_LINEAR_MIPMAP_NEAREST:
		case GL_CUBIC_MIPMAP_NEAREST:
			return JKG_FILTER_NEAREST;
		case GL_NEAREST_MIPMAP_LINEAR:
		case GL_LINEAR_MIPMAP_LINEAR:
		case GL_CUBIC_MIPMAP_LINEAR:
			return JKG_FILTER_LINEAR;
	}
}

static bool
get_compare_enable(GLenum mode)
{
	switch (mode)
	{
		default:
			assert(!"unknown compare mode");
			/* FALLTHROUGH */
		case GL_NONE:
			return false;
		case GL_COMPARE_REF_TO_TEXTURE:
			return true;
	}
}

static enum jkg_compare_op
get_compare_func(GLenum func)
{
	switch (func)
	{
		default:
			assert(!"unknown compare func");
			/* FALLTHROUGH */
		case GL_NEVER:
			return JKG_COMPARE_NEVER;
		case GL_LESS:
			return JKG_COMPARE_LOWER;
		case GL_LEQUAL:
			return JKG_COMPARE_LEQUAL;
		case GL_EQUAL:
			return JKG_COMPARE_EQUAL;
		case GL_GEQUAL:
			return JKG_COMPARE_GEQUAL;
		case GL_GREATER:
			return JKG_COMPARE_GREATER;
		case GL_NOTEQUAL:
			return JKG_COMPARE_NOTEQUAL;
		case GL_ALWAYS:
			return JKG_COMPARE_ALWAYS;
	}
}

static uint32_t
sampler_state_hash(struct gl_ctx *ctx,
                   const void *st)
{
	const struct sampler_state *state = st;
	uint32_t hash;

	(void)ctx;
	hash = FNV_BASIS32;
	hash = fnv32v(hash, &state->wrap_s, sizeof(state->wrap_s));
	hash = fnv32v(hash, &state->wrap_t, sizeof(state->wrap_t));
	hash = fnv32v(hash, &state->wrap_r, sizeof(state->wrap_r));
	hash = fnv32v(hash, &state->min_filter, sizeof(state->min_filter));
	hash = fnv32v(hash, &state->mag_filter, sizeof(state->mag_filter));
	hash = fnv32v(hash, &state->compare_mode, sizeof(state->compare_mode));
	hash = fnv32v(hash, &state->compare_func, sizeof(state->compare_func));
	hash = fnv32v(hash, &state->lod_bias, sizeof(state->lod_bias));
	hash = fnv32v(hash, &state->min_lod, sizeof(state->min_lod));
	hash = fnv32v(hash, &state->max_lod, sizeof(state->max_lod));
	hash = fnv32v(hash, &state->max_anisotropy, sizeof(state->max_anisotropy));
	hash = fnv32v(hash, &state->border_color, sizeof(state->border_color));
	return hash;
}

static bool
sampler_state_eq(struct gl_ctx *ctx,
                 const struct cache_entry *entry,
                 const void *state)
{
	const struct sampler_state *a = &((struct sampler_entry*)entry)->state;
	const struct sampler_state *b = state;

	(void)ctx;
	if (a->wrap_s != b->wrap_s
	 || a->wrap_t != b->wrap_t
	 || a->wrap_r != b->wrap_r
	 || a->min_filter != b->min_filter
	 || a->mag_filter != b->mag_filter
	 || a->compare_mode != b->compare_mode
	 || a->compare_func != b->compare_func
	 || a->lod_bias != b->lod_bias
	 || a->min_lod != b->min_lod
	 || a->max_lod != b->max_lod
	 || a->max_anisotropy != b->max_anisotropy
	 || a->border_color[0] != b->border_color[0]
	 || a->border_color[1] != b->border_color[1]
	 || a->border_color[2] != b->border_color[2]
	 || a->border_color[3] != b->border_color[3])
		return false;
	return true;
}

static struct cache_entry *
sampler_entry_alloc(struct gl_ctx *ctx,
                    GLuint hash,
                    const void *st)
{
	struct jkg_sampler_create_info create_info;
	const struct sampler_state *state = st;
	struct sampler_entry *entry;
	int ret;

	entry = malloc(sizeof(*entry));
	if (!entry)
	{
		GL_SET_ERR(ctx, GL_OUT_OF_MEMORY);
		return NULL;
	}
	memcpy(&entry->state, state, sizeof(*state));
	entry->cache.hash = hash;
	entry->cache.ref = 0;
	create_info.wrap[0] = get_wrap_mode(state->wrap_s);
	create_info.wrap[1] = get_wrap_mode(state->wrap_t);
	create_info.wrap[2] = get_wrap_mode(state->wrap_r);
	create_info.min_filter = get_min_filter(state->min_filter);
	create_info.mag_filter = get_mag_filter(state->mag_filter);
	create_info.mip_filter = get_mip_filter(state->min_filter);
	create_info.compare_enable = get_compare_enable(state->compare_mode);
	create_info.compare_func = get_compare_func(state->compare_func);
	create_info.lod_bias = state->lod_bias;
	create_info.min_lod = state->min_lod;
	create_info.max_lod = state->max_lod;
	create_info.max_anisotropy = state->max_anisotropy;
	create_info.border_color[0] = state->border_color[0];
	create_info.border_color[1] = state->border_color[1];
	create_info.border_color[2] = state->border_color[2];
	create_info.border_color[3] = state->border_color[3];
	ret = ctx->jkg_op->sampler_alloc(ctx->jkg_ctx,
	                                 &create_info,
	                                 &entry->sampler);
	if (ret)
	{
		GL_SET_ERR(ctx, get_jkg_error(ret));
		free(entry);
		return NULL;
	}
	return &entry->cache;
}

static void
sampler_entry_free(struct gl_ctx *ctx,
                   struct cache_entry *entry)
{
	ctx->jkg_op->sampler_free(ctx->jkg_ctx,
	                          ((struct sampler_entry*)entry)->sampler);
}

const struct cache_op
sampler_cache_op =
{
	.alloc = sampler_entry_alloc,
	.free = sampler_entry_free,
	.hash = sampler_state_hash,
	.eq = sampler_state_eq,
};
