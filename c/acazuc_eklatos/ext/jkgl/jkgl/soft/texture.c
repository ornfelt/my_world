#include "utils.h"
#include "soft.h"

#include <assert.h>
#include <math.h>

struct sample_ctx
{
	struct jkg_image_view *image_view;
	struct jkg_sampler *sampler;
	struct jkg_image *image;
	float fract[3];
	int32_t block[3];
	int32_t wrapped[3 * 4];
	int32_t fetch_coords[3];
	float color[4];
	uint32_t stride;
};

static int32_t
wrap_clamp_to_edge(int32_t max, int32_t v)
{
	if (v < 0)
		return 0;
	if (v >= max)
		return max - 1;
	return v;
}

static int32_t
wrap_clamp_to_border(int32_t max, int32_t v)
{
	if (v < 0)
		return -1;
	if (v >= max)
		return -1;
	return v;
}

static int32_t
wrap_repeat(int32_t max, int32_t v)
{
	v %= max;
	if (v < 0)
		return v + max;
	return v;
}

static int32_t
wrap_mirrored_repeat(int32_t max, int32_t v)
{
	int32_t w2 = max * 2;
	v %= w2;
	if (v <= -max)
		return -v - max;
	if (v < 0)
		return -v;
	if (v >= max)
		return max - 1 - (v - max);
	return v;
}

static int32_t
wrap_mirror_clamp_to_edge(int32_t max, int32_t v)
{
	if (v <= -max)
		return max - 1;
	if (v >= max)
		return max - 1;
	if (v < 0)
		return -v;
	return v;
}

static int32_t
wrap(enum jkg_wrap_mode wrap, int32_t max, int32_t v)
{
	switch (wrap)
	{
		case JKG_WRAP_CLAMP_TO_EDGE:
			return wrap_clamp_to_edge(max, v);
		case JKG_WRAP_CLAMP_TO_BORDER:
			return wrap_clamp_to_border(max, v);
		case JKG_WRAP_REPEAT:
			return wrap_repeat(max, v);
		case JKG_WRAP_MIRRORED_REPEAT:
			return wrap_mirrored_repeat(max, v);
		case JKG_WRAP_MIRROR_CLAMP_TO_EDGE:
			return wrap_mirror_clamp_to_edge(max, v);
		default:
			return -1;
	}
}

static int
fetch_block(struct sample_ctx *ctx,
            float *color)
{
	switch (ctx->image_view->format)
	{
		case JKG_FORMAT_BC1_RGB_UNORM_BLOCK:
			jkg_get_bc1_rgb(color,
			                ctx->fetch_coords[0],
			                ctx->fetch_coords[1],
			                ctx->image->size.x,
			                ctx->image->size.y,
			                ctx->image->data);
			return 1;
		case JKG_FORMAT_BC1_RGBA_UNORM_BLOCK:
			jkg_get_bc1_rgba(color,
			                 ctx->fetch_coords[0],
			                 ctx->fetch_coords[1],
			                 ctx->image->size.x,
			                 ctx->image->size.y,
			                 ctx->image->data);
			return 1;
		case JKG_FORMAT_BC2_UNORM_BLOCK:
			jkg_get_bc2(color,
			            ctx->fetch_coords[0],
			            ctx->fetch_coords[1],
			            ctx->image->size.x,
			            ctx->image->size.y,
			            ctx->image->data);
			return 1;
		case JKG_FORMAT_BC3_UNORM_BLOCK:
			jkg_get_bc3(color,
			            ctx->fetch_coords[0],
			            ctx->fetch_coords[1],
			            ctx->image->size.x,
			            ctx->image->size.y,
			            ctx->image->data);
			return 1;
		case JKG_FORMAT_BC4_UNORM_BLOCK:
			jkg_get_bc4_unorm(color,
			                  ctx->fetch_coords[0],
			                  ctx->fetch_coords[1],
			                  ctx->image->size.x,
			                  ctx->image->size.y,
			                  ctx->image->data);
			return 1;
		case JKG_FORMAT_BC4_SNORM_BLOCK:
			jkg_get_bc4_snorm(color,
			                  ctx->fetch_coords[0],
			                  ctx->fetch_coords[1],
			                  ctx->image->size.x,
			                  ctx->image->size.y,
			                  ctx->image->data);
			return 1;
		case JKG_FORMAT_BC5_UNORM_BLOCK:
			jkg_get_bc5_unorm(color,
			                  ctx->fetch_coords[0],
			                  ctx->fetch_coords[1],
			                  ctx->image->size.x,
			                  ctx->image->size.y,
			                  ctx->image->data);
			return 1;
		case JKG_FORMAT_BC5_SNORM_BLOCK:
			jkg_get_bc5_snorm(color,
			                  ctx->fetch_coords[0],
			                  ctx->fetch_coords[1],
			                  ctx->image->size.x,
			                  ctx->image->size.y,
			                  ctx->image->data);
			return 1;
		default:
			return 0;
	}
}

static void
fetch(struct sample_ctx *ctx,
      float *color)
{
	size_t index;

	index = 0;
	switch (ctx->image->type)
	{
		case JKG_IMAGE_1D:
			index = ctx->fetch_coords[0];
			break;
		case JKG_IMAGE_1D_ARRAY:
			index = ctx->fetch_coords[0] + ctx->fetch_coords[1] * ctx->image->size.x;
			break;
		case JKG_IMAGE_2D:
			if (fetch_block(ctx, color))
				return;
			index = ctx->fetch_coords[0] + ctx->fetch_coords[1] * ctx->image->size.x;
			break;
		case JKG_IMAGE_2D_ARRAY:
			index = ctx->fetch_coords[0] + (ctx->fetch_coords[1] + ctx->fetch_coords[2] * ctx->image->size.y) * ctx->image->size.x;
			break;
		case JKG_IMAGE_3D:
			index = ctx->fetch_coords[0] + (ctx->fetch_coords[1] + ctx->fetch_coords[2] * ctx->image->size.y) * ctx->image->size.x;
			break;
	}
	jkg_get_vec4f(color, &((uint8_t*)ctx->image->data)[index * ctx->stride], ctx->image_view->format);
}

static void
set_border_color(struct sample_ctx *ctx,
                 float *color)
{
	for (size_t i = 0; i < 4; ++i)
		color[i] = ctx->sampler->border_color[i];
}

static void
cubic_interpolate(float * restrict dst,
                  const float * restrict c0,
                  const float * restrict c1,
                  const float * restrict c2,
                  const float * restrict c3,
                  float f)
{
	for (size_t i = 0; i < 4; ++i)
	{
		float p0 = c1[i];
		float p1 = c2[i];
		float m0 = c1[i] - c0[i];
		float m1 = c3[i] - c2[i];
		float f3 = 2*p0 + m0 - 2*p1 + m1;
		float f2 = -3*p0 - 2*m0 + 3*p1 - m1 + f * f3;
		float f1 = m0 + f * f2;
		dst[i] = p0 + f * f1;
	}
}

#define FETCH_COORD(v, n, coord_id, next) \
do \
{ \
	for (size_t i = 0; i < (n); ++i) \
	{ \
		if (ctx->wrapped[(coord_id) * 4 + i] < 0) \
		{ \
			set_border_color(ctx, v[i]); \
			continue; \
		} \
		ctx->fetch_coords[coord_id] = ctx->wrapped[(coord_id) * 4 + i]; \
		if (coord_id) \
			next(ctx, (v)[i]); \
		else \
			fetch(ctx, (v)[i]); \
	} \
} while (0)

#define CUBIC_COORD(coord_name, coord_id, coord_next) \
static void \
cubic_##coord_name(struct sample_ctx *ctx, \
                   float *color) \
{ \
	float v[4][4]; \
 \
	FETCH_COORD(v, 4, coord_id, cubic_##coord_next); \
	cubic_interpolate(color, v[0], v[1], v[2], v[3], ctx->fract[coord_id]); \
}

CUBIC_COORD(x, 0, x);
CUBIC_COORD(y, 1, x);
CUBIC_COORD(z, 2, y);

static void
linear_interpolate(float * restrict dst,
                   const float * restrict c1,
                   const float * restrict c2,
                   float f)
{
	for (int32_t i = 0; i < 4; ++i)
		dst[i] = mixf(c1[i], c2[i], f);
}

#define LINEAR_COORD(coord_name, coord_id, coord_next) \
static void \
linear_##coord_name(struct sample_ctx *ctx, \
                    float *color) \
{ \
	float v[2][4]; \
 \
	FETCH_COORD(v, 2, coord_id, linear_##coord_next); \
	linear_interpolate(color, v[0], v[1], ctx->fract[coord_id]); \
}

LINEAR_COORD(x, 0, x);
LINEAR_COORD(y, 1, x);
LINEAR_COORD(z, 2, y);

#define NEAREST_COORD(coord_name, coord_id, coord_next) \
static void \
nearest_##coord_name(struct sample_ctx *ctx, \
                     float *color) \
{ \
	FETCH_COORD(&color, 1, coord_id, nearest_##coord_next); \
}

NEAREST_COORD(x, 0, x);
NEAREST_COORD(y, 1, x);
NEAREST_COORD(z, 2, y);

static void
wrap_cubic(int32_t *wrapped,
           int32_t block,
           int32_t max,
           enum jkg_wrap_mode mode)
{
	for (size_t i = 0; i < 4; ++i)
		wrapped[i] = wrap(mode, max, block - 1 + i);
}

static void
wrap_linear(int32_t *wrapped,
            int32_t block,
            int32_t max,
            enum jkg_wrap_mode mode)
{
	for (size_t i = 0; i < 2; ++i)
		wrapped[i] = wrap(mode, max, block + i);
}

static void
wrap_nearest(int32_t *wrapped,
             int32_t block,
             int32_t max,
             enum jkg_wrap_mode mode)
{
	for (size_t i = 0; i < 1; ++i)
		wrapped[i] = wrap(mode, max, block);
}

static void
get_filter_coord(float value,
                 int32_t *block,
                 float *fract)
{
	value -= 0.5f;
	*block = value;
	*fract = fmodf(value, 1.0f);
	if (*fract < 0)
	{
		(*block)--;
		(*fract)++;
	}
}

static void
texture_filter_1d(struct sample_ctx *ctx,
                  const float *coords)
{
	get_filter_coord(coords[0] * ctx->image->size.x, &ctx->block[0], &ctx->fract[0]);
	switch (ctx->sampler->mag_filter)
	{
		case JKG_FILTER_CUBIC:
			wrap_cubic(&ctx->wrapped[0 * 4], ctx->block[0], ctx->image->size.x, ctx->sampler->wrap[0]);
			cubic_x(ctx, ctx->color);
			break;
		case JKG_FILTER_LINEAR:
			wrap_linear(&ctx->wrapped[0 * 4], ctx->block[0], ctx->image->size.x, ctx->sampler->wrap[0]);
			linear_x(ctx, ctx->color);
			break;
		case JKG_FILTER_NEAREST:
			wrap_nearest(&ctx->wrapped[0 * 4], ctx->block[0], ctx->image->size.x, ctx->sampler->wrap[0]);
			nearest_x(ctx, ctx->color);
			break;
		default:
			assert(!"unknown texture mag filter");
			return;
	}
}

static void
texture_filter_1d_array(struct sample_ctx *ctx,
                        const float *coords)
{
	get_filter_coord(coords[0] * ctx->image->size.x, &ctx->block[0], &ctx->fract[0]);
	ctx->fetch_coords[1] = minui(maxui((uint32_t)roundf(coords[1]), 0), ctx->image->layers - 1); /* XXX? */
	switch (ctx->sampler->mag_filter)
	{
		case JKG_FILTER_CUBIC:
			wrap_cubic(&ctx->wrapped[0 * 4], ctx->block[0], ctx->image->size.x, ctx->sampler->wrap[0]);
			cubic_x(ctx, ctx->color);
			break;
		case JKG_FILTER_LINEAR:
			wrap_linear(&ctx->wrapped[0 * 4], ctx->block[0], ctx->image->size.x, ctx->sampler->wrap[0]);
			linear_x(ctx, ctx->color);
			break;
		case JKG_FILTER_NEAREST:
			wrap_nearest(&ctx->wrapped[0 * 4], ctx->block[0], ctx->image->size.x, ctx->sampler->wrap[0]);
			nearest_x(ctx, ctx->color);
			break;
		default:
			assert(!"unknown texture mag filter");
			return;
	}
}

static void
texture_filter_2d(struct sample_ctx *ctx,
                  const float *coords)
{
	get_filter_coord(coords[0] * ctx->image->size.x, &ctx->block[0], &ctx->fract[0]);
	get_filter_coord(coords[1] * ctx->image->size.y, &ctx->block[1], &ctx->fract[1]);
	switch (ctx->sampler->mag_filter)
	{
		case JKG_FILTER_CUBIC:
			wrap_cubic(&ctx->wrapped[0 * 4], ctx->block[0], ctx->image->size.x, ctx->sampler->wrap[0]);
			wrap_cubic(&ctx->wrapped[1 * 4], ctx->block[1], ctx->image->size.y, ctx->sampler->wrap[1]);
			cubic_y(ctx, ctx->color);
			break;
		case JKG_FILTER_LINEAR:
			wrap_linear(&ctx->wrapped[0 * 4], ctx->block[0], ctx->image->size.x, ctx->sampler->wrap[0]);
			wrap_linear(&ctx->wrapped[1 * 4], ctx->block[1], ctx->image->size.y, ctx->sampler->wrap[1]);
			linear_y(ctx, ctx->color);
			break;
		case JKG_FILTER_NEAREST:
			wrap_nearest(&ctx->wrapped[0 * 4], ctx->block[0], ctx->image->size.x, ctx->sampler->wrap[0]);
			wrap_nearest(&ctx->wrapped[1 * 4], ctx->block[1], ctx->image->size.y, ctx->sampler->wrap[1]);
			nearest_y(ctx, ctx->color);
			break;
		default:
			assert(!"unknown texture mag filter");
			return;
	}
}

static void
texture_filter_2d_array(struct sample_ctx *ctx,
                        const float *coords)
{
	get_filter_coord(coords[0] * ctx->image->size.x, &ctx->block[0], &ctx->fract[0]);
	get_filter_coord(coords[1] * ctx->image->size.y, &ctx->block[1], &ctx->fract[1]);
	ctx->fetch_coords[2] = minui(maxui((uint32_t)roundf(coords[2]), 0), ctx->image->layers - 1); /* XXX? */
	switch (ctx->sampler->mag_filter)
	{
		case JKG_FILTER_CUBIC:
			wrap_cubic(&ctx->wrapped[0 * 4], ctx->block[0], ctx->image->size.x, ctx->sampler->wrap[0]);
			wrap_cubic(&ctx->wrapped[1 * 4], ctx->block[1], ctx->image->size.y, ctx->sampler->wrap[1]);
			cubic_y(ctx, ctx->color);
			break;
		case JKG_FILTER_LINEAR:
			wrap_linear(&ctx->wrapped[0 * 4], ctx->block[0], ctx->image->size.x, ctx->sampler->wrap[0]);
			wrap_linear(&ctx->wrapped[1 * 4], ctx->block[1], ctx->image->size.y, ctx->sampler->wrap[1]);
			linear_y(ctx, ctx->color);
			break;
		case JKG_FILTER_NEAREST:
			wrap_nearest(&ctx->wrapped[0 * 4], ctx->block[0], ctx->image->size.x, ctx->sampler->wrap[0]);
			wrap_nearest(&ctx->wrapped[1 * 4], ctx->block[1], ctx->image->size.y, ctx->sampler->wrap[1]);
			nearest_y(ctx, ctx->color);
			break;
		default:
			assert(!"unknown texture mag filter");
			return;
	}
}

static void
texture_filter_3d(struct sample_ctx *ctx,
                  const float *coords)
{
	get_filter_coord(coords[0] * ctx->image->size.x, &ctx->block[0], &ctx->fract[0]);
	get_filter_coord(coords[1] * ctx->image->size.y, &ctx->block[1], &ctx->fract[1]);
	get_filter_coord(coords[2] * ctx->image->size.z, &ctx->block[2], &ctx->fract[2]);
	switch (ctx->sampler->mag_filter)
	{
		case JKG_FILTER_CUBIC:
			wrap_cubic(&ctx->wrapped[0 * 4], ctx->block[0], ctx->image->size.x, ctx->sampler->wrap[0]);
			wrap_cubic(&ctx->wrapped[1 * 4], ctx->block[1], ctx->image->size.y, ctx->sampler->wrap[1]);
			wrap_cubic(&ctx->wrapped[2 * 4], ctx->block[2], ctx->image->size.z, ctx->sampler->wrap[2]);
			cubic_z(ctx, ctx->color);
			break;
		case JKG_FILTER_LINEAR:
			wrap_linear(&ctx->wrapped[0 * 4], ctx->block[0], ctx->image->size.x, ctx->sampler->wrap[0]);
			wrap_linear(&ctx->wrapped[1 * 4], ctx->block[1], ctx->image->size.y, ctx->sampler->wrap[1]);
			wrap_linear(&ctx->wrapped[2 * 4], ctx->block[2], ctx->image->size.z, ctx->sampler->wrap[2]);
			linear_z(ctx, ctx->color);
			break;
		case JKG_FILTER_NEAREST:
			wrap_nearest(&ctx->wrapped[0 * 4], ctx->block[0], ctx->image->size.x, ctx->sampler->wrap[0]);
			wrap_nearest(&ctx->wrapped[1 * 4], ctx->block[1], ctx->image->size.y, ctx->sampler->wrap[1]);
			wrap_nearest(&ctx->wrapped[2 * 4], ctx->block[2], ctx->image->size.z, ctx->sampler->wrap[2]);
			nearest_z(ctx, ctx->color);
			break;
		default:
			assert(!"unknown texture mag filter");
			return;
	}
}

void
soft_sample(struct jkg_ctx *ctx,
            uint32_t id,
            const float coords[3],
            float color[4])
{
	struct sample_ctx sample_ctx;

	sample_ctx.image_view = ctx->image_views[id];
	if (!sample_ctx.image_view)
	{
		color[0] = 0;
		color[1] = 0;
		color[2] = 0;
		color[3] = 1;
		return;
	}
	sample_ctx.image = sample_ctx.image_view->image;
	sample_ctx.sampler = ctx->samplers[id];
	sample_ctx.stride = jkg_get_stride(sample_ctx.image_view->format);
	switch (sample_ctx.image_view->image->type)
	{
		case JKG_IMAGE_1D:
			texture_filter_1d(&sample_ctx, coords);
			break;
		case JKG_IMAGE_1D_ARRAY:
			texture_filter_1d_array(&sample_ctx, coords);
			break;
		case JKG_IMAGE_2D:
			texture_filter_2d(&sample_ctx, coords);
			break;
		case JKG_IMAGE_2D_ARRAY:
			texture_filter_2d_array(&sample_ctx, coords);
			break;
		case JKG_IMAGE_3D:
			texture_filter_3d(&sample_ctx, coords);
			break;
	}
	for (size_t i = 0; i < 4; ++i)
	{
		switch (sample_ctx.image_view->swizzle[i])
		{
			case JKG_SWIZZLE_R:
				color[i] = clampf(sample_ctx.color[0], 0, 1);
				break;
			case JKG_SWIZZLE_G:
				color[i] = clampf(sample_ctx.color[1], 0, 1);
				break;
			case JKG_SWIZZLE_B:
				color[i] = clampf(sample_ctx.color[2], 0, 1);
				break;
			case JKG_SWIZZLE_A:
				color[i] = clampf(sample_ctx.color[3], 0, 1);
				break;
			case JKG_SWIZZLE_ZERO:
				color[i] = 0;
				break;
			case JKG_SWIZZLE_ONE:
				color[i] = 1;
				break;
		}
	}
}
