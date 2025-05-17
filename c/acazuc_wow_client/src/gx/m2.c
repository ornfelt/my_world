#include "gx/m2_particles.h"
#include "gx/m2_ribbons.h"
#include "gx/skybox.h"
#include "gx/frame.h"
#include "gx/blp.h"
#include "gx/m2.h"
#include "gx/gx.h"

#include "map/map.h"

#include "shaders.h"
#include "camera.h"
#include "loader.h"
#include "memory.h"
#include "cache.h"
#include "log.h"
#include "wow.h"

#include <jks/quaternion.h>

#include <gfx/device.h>

#include <wow/mpq.h>
#include <wow/m2.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>

#define GX_M2_VERTEX_DIFFUSE                   0x1
#define GX_M2_VERTEX_DIFFUSE_ENV_MAP           0x2
#define GX_M2_VERTEX_DIFFUSE_2TEX              0x3
#define GX_M2_VERTEX_DIFFUSE_ENV_MAP0_2TEX     0x4
#define GX_M2_VERTEX_DIFFUSE_ENV_MAP_2TEX      0x5
#define GX_M2_VERTEX_DIFFUSE_DUAL_ENV_MAP_2TEX 0x6

#define GX_M2_FRAGMENT_OPAQUE                 0x1
#define GX_M2_FRAGMENT_DIFFUSE                0x2
#define GX_M2_FRAGMENT_DECAL                  0x3
#define GX_M2_FRAGMENT_ADD                    0x4
#define GX_M2_FRAGMENT_DIFFUSE2X              0x5
#define GX_M2_FRAGMENT_FADE                   0x6
#define GX_M2_FRAGMENT_OPAQUE_OPAQUE          0x7
#define GX_M2_FRAGMENT_OPAQUE_MOD             0x8
#define GX_M2_FRAGMENT_OPAQUE_DECAL           0x9
#define GX_M2_FRAGMENT_OPAQUE_ADD             0xA
#define GX_M2_FRAGMENT_OPAQUE_MOD2X           0xB
#define GX_M2_FRAGMENT_OPAQUE_FADE            0xC
#define GX_M2_FRAGMENT_DIFFUSE_2TEX_OPAQUE    0xD
#define GX_M2_FRAGMENT_DIFFUSE_2TEX           0xE
#define GX_M2_FRAGMENT_DIFFUSE_2TEX_DECAL     0xF
#define GX_M2_FRAGMENT_DIFFUSE_2TEX_ADD       0x10
#define GX_M2_FRAGMENT_DIFFUSE_2TEX_2X        0x11
#define GX_M2_FRAGMENT_DIFFUSE_2TEX_FADE      0x12
#define GX_M2_FRAGMENT_DECAL_OPAQUE           0x13
#define GX_M2_FRAGMENT_DECAL_MOD              0x14
#define GX_M2_FRAGMENT_DECAL_DECAL            0x15
#define GX_M2_FRAGMENT_DECAL_ADD              0x16
#define GX_M2_FRAGMENT_DECAL_MOD2X            0x17
#define GX_M2_FRAGMENT_DECAL_FADE             0x18
#define GX_M2_FRAGMENT_ADD_OPAQUE             0x19
#define GX_M2_FRAGMENT_ADD_MOD                0x1A
#define GX_M2_FRAGMENT_ADD_DECAL              0x1B
#define GX_M2_FRAGMENT_ADD_ADD                0x1C
#define GX_M2_FRAGMENT_ADD_MOD2X              0x1D
#define GX_M2_FRAGMENT_ADD_FADE               0x1E
#define GX_M2_FRAGMENT_DIFFUSE_2TEX_OPAQUE_2X 0x1F
/* #define GX_M2_FRAGMENT_DIFFUSE_2TEX_2X      0x20 */
#define GX_M2_FRAGMENT_DIFFUSE_2TEX_DECAL_2X  0x21
#define GX_M2_FRAGMENT_DIFFUSE_2TEX_ADD_2X    0x22
#define GX_M2_FRAGMENT_DIFFUSE_2TEX_4X        0x23
#define GX_M2_FRAGMENT_DIFFUSE_2TEX_FADE_2X   0x24

MEMORY_DECL(GX);

/*
T interpolateHermite(T &v0, T &v1, T &v2, T &v3, float x)
{
	return v1 + (v1 - v0 + (v0 * 2.f - v1 * 5.f + v2 * 4.f - v3 + (-v0 + v1 * 3.f - v2 * 3.f + v3) * x) * x) * x;
}
*/

static void update_instance_uniform_buffer(struct gx_m2_instance *profile, struct gx_frame *frame);

static bool get_track_values(struct gx_m2 *m2, struct wow_m2_track *track, void **v1, void **v2, size_t size, float *a, struct wow_m2_sequence *sequence, uint32_t t)
{
	if (track->timestamps_nb == 0)
		return false;
	if (track->timestamps_nb != track->values_nb)
	{
		LOG_WARN("timestamps_nb != values_nb");
		return false;
	}
	if (track->values_nb == 1)
	{
		*v1 = track->values;
		*v2 = track->values;
		*a = 0;
		return true;
	}
	if (track->global_sequence >= 0 && (uint16_t)track->global_sequence < m2->global_sequences_nb)
	{
		uint32_t max = m2->global_sequences[track->global_sequence];
		if (max == 0)
			t = 0;
		else
			t = (g_wow->frametime / 1000000) % max;
	}
	else if (sequence)
	{
		t = sequence->start + t % (sequence->end - sequence->start);
	}
	t %= track->timestamps[track->timestamps_nb - 1] + 1;
	uint32_t range = track->timestamps_nb - 1;
	uint32_t i = 0;
	if (g_wow->wow_opt & WOW_OPT_M2_TRACK_BSEARCH)
	{
		if (t > track->timestamps[0])
		{
			while (range > 1)
			{
				uint32_t half = range / 2;
				if (t < track->timestamps[i + half])
				{
					range = half;
				}
				else
				{
					i += half;
					range -= half;
				}
			}
			i++;
		}
	}
	else
	{
		for (i = 0; i < track->timestamps_nb; ++i)
		{
			if (track->timestamps[i] >= t)
				break;
		}
	}
	if (i == 0)
	{
		*a = 0;
		*v1 = track->values;
		*v2 = track->values;
		return true;
	}
	if (track->timestamps[i] == track->timestamps[i - 1])
		*a = 1;
	else
		*a = (t - track->timestamps[i - 1]) / (float)(track->timestamps[i] - track->timestamps[i - 1]);
	*v1 = ((char*)track->values) + (i - 1) * size;
	*v2 = ((char*)track->values) +  i      * size;
	return true;
}

bool m2_get_track_value_vec4f(struct gx_m2 *m2, struct wow_m2_track *track, struct vec4f *val, struct wow_m2_sequence *sequence, uint32_t t)
{
	struct vec4f *v1;
	struct vec4f *v2;
	float a;
	if (!get_track_values(m2, track, (void**)&v1, (void**)&v2, sizeof(*v1), &a, sequence, t))
		return false;
	switch (track->interpolation_type)
	{
		case 0:
			*val = *v1;
			break;
		default:
			LOG_DEBUG("unknown interpolation type: %d", track->interpolation_type);
			/* FALLTHROUGH */
		case 1:
			VEC4_SUB(*val, *v2, *v1);
			VEC4_MULV(*val, *val, a);
			VEC4_ADD(*val, *val, *v1);
			break;
	}
	return true;
}

bool m2_get_track_value_vec3f(struct gx_m2 *m2, struct wow_m2_track *track, struct vec3f *val, struct wow_m2_sequence *sequence, uint32_t t)
{
	struct vec3f *v1;
	struct vec3f *v2;
	float a;
	if (!get_track_values(m2, track, (void**)&v1, (void**)&v2, sizeof(*v1), &a, sequence, t))
		return false;
	switch (track->interpolation_type)
	{
		case 0:
			*val = *v1;
			break;
		default:
			LOG_DEBUG("unknown interpolation type: %d", track->interpolation_type);
			/* FALLTHROUGH */
		case 1:
			VEC3_SUB(*val, *v2, *v1);
			VEC3_MULV(*val, *val, a);
			VEC3_ADD(*val, *val, *v1);
			break;
	}
	return true;
}

bool m2_get_track_value_float(struct gx_m2 *m2, struct wow_m2_track *track, float *val, struct wow_m2_sequence *sequence, uint32_t t)
{
	float *v1;
	float *v2;
	float a;
	if (!get_track_values(m2, track, (void**)&v1, (void**)&v2, sizeof(*v1), &a, sequence, t))
		return false;
	switch (track->interpolation_type)
	{
		case 0:
			*val = *v1;
			break;
		default:
			LOG_DEBUG("unknown interpolation type: %d", track->interpolation_type);
			/* FALLTHROUGH */
		case 1:
			*val = *v1 + (*v2 - *v1) * a;
			break;
	}
	return true;
}

bool m2_get_track_value_uint8(struct gx_m2 *m2, struct wow_m2_track *track, uint8_t *val, struct wow_m2_sequence *sequence, uint32_t t)
{
	uint8_t *v1;
	uint8_t *v2;
	float a;
	if (!get_track_values(m2, track, (void**)&v1, (void**)&v2, sizeof(*v1), &a, sequence, t))
		return false;
	switch (track->interpolation_type)
	{
		case 0:
			*val = *v1;
			break;
		default:
			LOG_DEBUG("unknown interpolation type: %d", track->interpolation_type);
			/* FALLTHROUGH */
		case 1:
			*val = *v1 + (*v2 - *v1) * a;
			break;
	}
	return true;
}

bool m2_get_track_value_int16(struct gx_m2 *m2, struct wow_m2_track *track, int16_t *val, struct wow_m2_sequence *sequence, uint32_t t)
{
	int16_t *v1;
	int16_t *v2;
	float a;
	if (!get_track_values(m2, track, (void**)&v1, (void**)&v2, sizeof(*v1), &a, sequence, t))
		return false;
	switch (track->interpolation_type)
	{
		case 0:
			*val = *v1;
			break;
		default:
			LOG_DEBUG("unknown interpolation type: %d", track->interpolation_type);
			/* FALLTHROUGH */
		case 1:
			*val = *v1 + (*v2 - *v1) * a;
			break;
	}
	return true;
}

bool m2_get_track_value_quat16(struct gx_m2 *m2, struct wow_m2_track *track, struct vec4f *val, struct wow_m2_sequence *sequence, uint32_t t)
{
	struct vec4s *v1;
	struct vec4s *v2;
	float a;
	if (!get_track_values(m2, track, (void**)&v1, (void**)&v2, sizeof(*v1), &a, sequence, t))
		return false;
	switch (track->interpolation_type)
	{
		case 0:
			val->x = (v1->x < 0 ? v1->x + 32768 : v1->x - 32767) / 32767.f;
			val->y = (v1->y < 0 ? v1->y + 32768 : v1->y - 32767) / 32767.f;
			val->z = (v1->z < 0 ? v1->z + 32768 : v1->z - 32767) / 32767.f;
			val->w = (v1->w < 0 ? v1->w + 32768 : v1->w - 32767) / 32767.f;
			break;
		default:
			LOG_DEBUG("unknown interpolation type: %d", track->interpolation_type);
			/* FALLTHROUGH */
		case 1:
		{
			struct vec4f tmp1;
			tmp1.x = (v1->x < 0 ? v1->x + 32768 : v1->x - 32767) / 32767.f;
			tmp1.y = (v1->y < 0 ? v1->y + 32768 : v1->y - 32767) / 32767.f;
			tmp1.z = (v1->z < 0 ? v1->z + 32768 : v1->z - 32767) / 32767.f;
			tmp1.w = (v1->w < 0 ? v1->w + 32768 : v1->w - 32767) / 32767.f;
			struct vec4f tmp2;
			tmp2.x = (v2->x < 0 ? v2->x + 32768 : v2->x - 32767) / 32767.f;
			tmp2.y = (v2->y < 0 ? v2->y + 32768 : v2->y - 32767) / 32767.f;
			tmp2.z = (v2->z < 0 ? v2->z + 32768 : v2->z - 32767) / 32767.f;
			tmp2.w = (v2->w < 0 ? v2->w + 32768 : v2->w - 32767) / 32767.f;
			VEC4_SUB(*val, tmp2, tmp1);
			VEC4_MULV(*val, *val, a);
			VEC4_ADD(*val, *val, tmp1);
			break;
		}
	}
	return true;
}

bool m2_instance_get_track_value_vec4f(struct gx_m2_instance *instance, struct wow_m2_track *track, struct vec4f *val)
{
	struct vec4f v1;
	if (!m2_get_track_value_vec4f(instance->parent, track, &v1, instance->sequence, instance->sequence_time))
		return false;
	if (g_wow->frametime - instance->sequence_started >= (instance->sequence->blend_time * 1000000)
	 || !instance->prev_sequence || instance->prev_sequence->end <= instance->prev_sequence->start)
	{
		*val = v1;
		return true;
	}
	struct vec4f v2;
	if (!m2_get_track_value_vec4f(instance->parent, track, &v2, instance->prev_sequence, instance->prev_sequence_time))
	{
		*val = v1;
		return true;
	}
	float pct = (g_wow->frametime - instance->sequence_started) / (instance->sequence->blend_time * 1000000.0);
	VEC4_MULV(v1, v1, pct);
	VEC4_MULV(v2, v2, 1 - pct);
	VEC4_ADD(*val, v1, v2);
	return true;
}

bool m2_instance_get_track_value_vec3f(struct gx_m2_instance *instance, struct wow_m2_track *track, struct vec3f *val)
{
	struct vec3f v1;
	if (!m2_get_track_value_vec3f(instance->parent, track, &v1, instance->sequence, instance->sequence_time))
		return false;
	if (g_wow->frametime - instance->sequence_started >= (instance->sequence->blend_time * 1000000)
	 || !instance->prev_sequence || instance->prev_sequence->end <= instance->prev_sequence->start)
	{
		*val = v1;
		return true;
	}
	struct vec3f v2;
	if (!m2_get_track_value_vec3f(instance->parent, track, &v2, instance->prev_sequence, instance->prev_sequence_time))
	{
		*val = v1;
		return true;
	}
	float pct = (g_wow->frametime - instance->sequence_started) / (instance->sequence->blend_time * 1000000.0);
	VEC3_MULV(v1, v1, pct);
	VEC3_MULV(v2, v2, 1 - pct);
	VEC3_ADD(*val, v1, v2);
	return true;
}

bool m2_instance_get_track_value_float(struct gx_m2_instance *instance, struct wow_m2_track *track, float *val)
{
	float v1;
	if (!m2_get_track_value_float(instance->parent, track, &v1, instance->sequence, instance->sequence_time))
		return false;
	if (g_wow->frametime - instance->sequence_started >= (instance->sequence->blend_time * 1000000)
	 || !instance->prev_sequence || instance->prev_sequence->end <= instance->prev_sequence->start)
	{
		*val = v1;
		return true;
	}
	float v2;
	if (!m2_get_track_value_float(instance->parent, track, &v2, instance->prev_sequence, instance->prev_sequence_time))
	{
		*val = v1;
		return true;
	}
	float pct = (g_wow->frametime - instance->sequence_started) / (instance->sequence->blend_time * 1000000.0);
	*val = v1 * pct + v2 * (1 - pct);
	return true;
}

bool m2_instance_get_track_value_uint8(struct gx_m2_instance *instance, struct wow_m2_track *track, uint8_t *val)
{
	uint8_t v1;
	if (!m2_get_track_value_uint8(instance->parent, track, &v1, instance->sequence, instance->sequence_time))
		return false;
	if (g_wow->frametime - instance->sequence_started >= (instance->sequence->blend_time * 1000000)
	 || !instance->prev_sequence || instance->prev_sequence->end <= instance->prev_sequence->start)
	{
		*val = v1;
		return true;
	}
	uint8_t v2;
	if (!m2_get_track_value_uint8(instance->parent, track, &v2, instance->prev_sequence, instance->prev_sequence_time))
	{
		*val = v1;
		return true;
	}
	float pct = (g_wow->frametime - instance->sequence_started) / (instance->sequence->blend_time * 1000000.0);
	*val = v1 * pct + v2 * (1 - pct);
	return true;
}

bool m2_instance_get_track_value_int16(struct gx_m2_instance *instance, struct wow_m2_track *track, int16_t *val)
{
	int16_t v1;
	if (!m2_get_track_value_int16(instance->parent, track, &v1, instance->sequence, instance->sequence_time))
		return false;
	if (g_wow->frametime - instance->sequence_started >= (instance->sequence->blend_time * 1000000)
	 || !instance->prev_sequence || instance->prev_sequence->end <= instance->prev_sequence->start)
	{
		*val = v1;
		return true;
	}
	int16_t v2;
	if (!m2_get_track_value_int16(instance->parent, track, &v2, instance->prev_sequence, instance->prev_sequence_time))
	{
		*val = v1;
		return true;
	}
	float pct = (g_wow->frametime - instance->sequence_started) / (instance->sequence->blend_time * 1000000.0);
	*val = v1 * pct + v2 * (1 - pct);
	return true;
}

bool m2_instance_get_track_value_quat16(struct gx_m2_instance *instance, struct wow_m2_track *track, struct vec4f *val)
{
	struct vec4f v1;
	if (!m2_get_track_value_quat16(instance->parent, track, &v1, instance->sequence, instance->sequence_time))
		return false;
	if (g_wow->frametime - instance->sequence_started >= (instance->sequence->blend_time * 1000000)
	 || !instance->prev_sequence || instance->prev_sequence->end <= instance->prev_sequence->start)
	{
		*val = v1;
		return true;
	}
	struct vec4f v2;
	if (!m2_get_track_value_quat16(instance->parent, track, &v2, instance->prev_sequence, instance->prev_sequence_time))
	{
		*val = v1;
		return true;
	}
	float pct = (g_wow->frametime - instance->sequence_started) / (instance->sequence->blend_time * 1000000.0);
	VEC4_MULV(v1, v1, pct);
	VEC4_MULV(v2, v2, 1 - pct);
	VEC4_ADD(*val, v1, v2);
	return true;
}

static void gx_m2_texture_init(struct gx_m2_texture *texture)
{
	texture->texture = NULL;
	texture->initialized = false;
}

static void gx_m2_texture_destroy(struct gx_m2_texture *texture)
{
	gx_blp_free(texture->texture);
}

static void init_texture_transform(struct gx_m2_texture *texture, struct wow_m2_file *file, struct wow_m2_batch *wow_batch, uint16_t offset)
{
	texture->has_transform = false;
	if (wow_batch->texture_transform_combo_index == UINT16_MAX)
		return;
	if (wow_batch->texture_transform_combo_index + offset >= file->texture_transforms_lookups_nb)
	{
		LOG_WARN("invalid textureTransformComboIndex: %u / %u", wow_batch->texture_transform_combo_index + offset, file->texture_transforms_lookups_nb);
		return;
	}
	uint16_t lookup = file->texture_transforms_lookups[wow_batch->texture_transform_combo_index + offset];
	if (lookup == UINT16_MAX)
		return;
	if (lookup >= file->texture_transforms_nb)
	{
		LOG_WARN("invalid textureTransformLookup: %u / %u", lookup, file->texture_transforms_nb);
		return;
	}
	texture->transform = lookup;
	texture->has_transform = true;
}

static void gx_m2_texture_load(struct gx_m2_texture *texture, struct wow_m2_file *file, struct wow_m2_batch *wow_batch, uint16_t offset)
{
	texture->initialized = true;
	struct wow_m2_texture *wow_texture = &file->textures[file->texture_lookups[wow_batch->texture_combo_index + offset]];
	texture->type = wow_texture->type;
	if (!wow_texture->type)
	{
		if (wow_texture->filename)
		{
			char filename[512];
			snprintf(filename, sizeof(filename), "%s", wow_texture->filename);
			wow_mpq_normalize_blp_fn(filename, sizeof(filename));
			if (cache_ref_blp(g_wow->cache, filename, &texture->texture))
			{
				gx_blp_ask_load(texture->texture);
			}
			else
			{
				LOG_ERROR("failed to load texture %s", filename);
				texture->texture = NULL;
			}
		}
	}
	texture->wow_flags = wow_texture->flags;
	init_texture_transform(texture, file, wow_batch, offset);
}

const gfx_texture_t *gx_m2_texture_bind(struct gx_m2_texture *texture, struct gx_m2_instance *instance)
{
	if (!texture->initialized)
		return &g_wow->grey_texture->texture;
	struct gx_blp *blp;
	if (texture->texture)
	{
		blp = texture->texture;
	}
	else
	{
		switch (texture->type)
		{
			case 1:
				blp = instance->skin_texture;
				break;
			case 2:
				blp = instance->object_texture;
				break;
			case 6:
				blp = instance->hair_texture;
				break;
			case 8:
				blp = instance->skin_extra_texture;
				break;
			case 11:
				blp = instance->monster_textures[0];
				break;
			case 12:
				blp = instance->monster_textures[1];
				break;
			case 13:
				blp = instance->monster_textures[2];
				break;
			default:
				blp = NULL;
				break;
		}
	}
	if (!blp)
	{
		/* LOG_WARN("batch no type: %u", texture->type); */
		return &g_wow->grey_texture->texture;
	}
	if (!gx_blp_flag_get(blp, GX_BLP_FLAG_INITIALIZED))
		return &g_wow->grey_texture->texture;
	enum gfx_texture_addressing addressing_s = (texture->wow_flags & WOW_M2_TEXTURE_FLAG_CLAMP_S) ? GFX_TEXTURE_ADDRESSING_REPEAT : GFX_TEXTURE_ADDRESSING_CLAMP;
	enum gfx_texture_addressing addressing_t = (texture->wow_flags & WOW_M2_TEXTURE_FLAG_CLAMP_T) ? GFX_TEXTURE_ADDRESSING_REPEAT : GFX_TEXTURE_ADDRESSING_CLAMP;
	gfx_set_texture_addressing(&blp->texture, addressing_s, addressing_t, GFX_TEXTURE_ADDRESSING_CLAMP);
	return &blp->texture;
}

const gfx_texture_t *gx_m2_texture_bind_instanced(struct gx_m2_texture *texture)
{
	if (!texture->initialized)
		return &g_wow->grey_texture->texture;
	if (!texture->texture)
		return &g_wow->grey_texture->texture;
	if (!gx_blp_flag_get(texture->texture, GX_BLP_FLAG_INITIALIZED))
		return &g_wow->grey_texture->texture;
	enum gfx_texture_addressing addressing_s = (texture->wow_flags & WOW_M2_TEXTURE_FLAG_CLAMP_S) ? GFX_TEXTURE_ADDRESSING_REPEAT : GFX_TEXTURE_ADDRESSING_CLAMP;
	enum gfx_texture_addressing addressing_t = (texture->wow_flags & WOW_M2_TEXTURE_FLAG_CLAMP_T) ? GFX_TEXTURE_ADDRESSING_REPEAT : GFX_TEXTURE_ADDRESSING_CLAMP;
	gfx_set_texture_addressing(&texture->texture->texture, addressing_s, addressing_t, GFX_TEXTURE_ADDRESSING_CLAMP);
	return &texture->texture->texture;
}

static void gx_m2_texture_update_matrix(struct gx_m2_texture *texture, struct gx_frame *frame, struct gx_m2_batch *batch, struct mat4f *mat_ref)
{
	if (!texture->initialized || (texture->type == 0 && !texture->texture))
		return;
	struct mat4f mat;
	{
		struct vec3f tmp = {0.5, 0.5, 0.0};
		struct mat4f tmp_mat;
		MAT4_IDENTITY(tmp_mat);
		MAT4_TRANSLATE(mat, tmp_mat, tmp);
	}
	struct gx_m2 *m2 = batch->parent->parent;
	if (texture->has_transform)
	{
		{
			struct vec3f v;
			if (m2_get_track_value_vec3f(m2, &m2->texture_transforms[texture->transform].translation, &v, NULL, frame->time / 1000000))
			{
				struct mat4f tmp_mat;
				MAT4_TRANSLATE(tmp_mat, mat, v);
				mat = tmp_mat;
			}
			if (m2_get_track_value_vec3f(m2, &m2->texture_transforms[texture->transform].scaling, &v, NULL, frame->time / 1000000))
			{
				struct mat4f tmp_mat;
				MAT4_SCALE(tmp_mat, mat, v);
				mat = tmp_mat;
			}
		}
		{
			struct vec4f v;
			if (m2_get_track_value_vec4f(m2, &m2->texture_transforms[texture->transform].rotation, &v, NULL, frame->time / 1000000))
			{
				struct mat4f quat;
				QUATERNION_TO_MAT4(float, quat, v);
				struct mat4f tmp_mat;
				MAT4_MUL(tmp_mat, mat, quat);
				mat = tmp_mat;
			}
		}
	}
	*mat_ref = mat;
}

static void init_batch_color_transform(struct gx_m2_batch *batch, struct wow_m2_file *file, struct wow_m2_batch *wow_batch)
{
	batch->has_color_transform = false;
	if (wow_batch->color_index == UINT16_MAX)
		return;
	if (wow_batch->color_index >= file->colors_nb)
	{
		LOG_WARN("invalid color index: %u / %u", wow_batch->color_index, file->colors_nb);
		return;
	}
	batch->color_transform = wow_batch->color_index;
	batch->has_color_transform = true;
}

static void init_batch_texture_weight(struct gx_m2_batch *batch, struct wow_m2_file *file, struct wow_m2_batch *wow_batch)
{
	batch->has_texture_weight = false;
	if (wow_batch->texture_weight_combo_index == UINT16_MAX)
		return;
	if (wow_batch->texture_weight_combo_index >= file->texture_weights_lookups_nb)
	{
		LOG_WARN("invalid texture weight combo index: %u / %u", wow_batch->texture_weight_combo_index, file->texture_weights_lookups_nb);
		return;
	}
	uint16_t lookup = file->texture_weights_lookups[wow_batch->texture_weight_combo_index];
	if (lookup == UINT16_MAX)
		return;
	if (lookup >= file->texture_weights_nb)
	{
		LOG_WARN("invalid texture weight lookup: %u / %u", lookup, file->texture_weights_nb);
		return;
	}
	batch->texture_weight = lookup;
	batch->has_texture_weight = true;
}

static bool get_combiners(uint16_t tex_nb, uint32_t *fragment_modes, bool tex1env, bool tex2env, uint16_t *combiners)
{
	if (tex_nb == 1)
	{
		combiners[0] = tex1env ? GX_M2_VERTEX_DIFFUSE_ENV_MAP : GX_M2_VERTEX_DIFFUSE;
		static const uint32_t modes[] = {GX_M2_FRAGMENT_OPAQUE, GX_M2_FRAGMENT_DIFFUSE, GX_M2_FRAGMENT_DECAL, GX_M2_FRAGMENT_ADD, GX_M2_FRAGMENT_DIFFUSE2X, GX_M2_FRAGMENT_FADE};
		if (fragment_modes[0] >= sizeof(modes) / sizeof(*modes))
			return false;
		combiners[1] = modes[fragment_modes[0]];
		return true;
	}
	if (tex1env)
		combiners[0] = tex2env ? GX_M2_VERTEX_DIFFUSE_DUAL_ENV_MAP_2TEX : GX_M2_VERTEX_DIFFUSE_ENV_MAP0_2TEX;
	else
		combiners[0] = tex2env ? GX_M2_VERTEX_DIFFUSE_ENV_MAP_2TEX : GX_M2_VERTEX_DIFFUSE_2TEX;
	static const uint32_t modes[] =
	{
		GX_M2_FRAGMENT_OPAQUE_OPAQUE         , GX_M2_FRAGMENT_OPAQUE_MOD     , GX_M2_FRAGMENT_OPAQUE_DECAL         , GX_M2_FRAGMENT_OPAQUE_ADD         , GX_M2_FRAGMENT_OPAQUE_MOD2X   , GX_M2_FRAGMENT_OPAQUE_FADE,
		GX_M2_FRAGMENT_DIFFUSE_2TEX_OPAQUE   , GX_M2_FRAGMENT_DIFFUSE_2TEX   , GX_M2_FRAGMENT_DIFFUSE_2TEX_DECAL   , GX_M2_FRAGMENT_DIFFUSE_2TEX_ADD   , GX_M2_FRAGMENT_DIFFUSE_2TEX_2X, GX_M2_FRAGMENT_DIFFUSE_2TEX_FADE,
		GX_M2_FRAGMENT_DECAL_OPAQUE          , GX_M2_FRAGMENT_DECAL_MOD      , GX_M2_FRAGMENT_DECAL_DECAL          , GX_M2_FRAGMENT_DECAL_ADD          , GX_M2_FRAGMENT_DECAL_MOD2X    , GX_M2_FRAGMENT_DECAL_FADE,
		GX_M2_FRAGMENT_ADD_OPAQUE            , GX_M2_FRAGMENT_ADD_MOD        , GX_M2_FRAGMENT_ADD_DECAL            , GX_M2_FRAGMENT_ADD_ADD            , GX_M2_FRAGMENT_ADD_MOD2X      , GX_M2_FRAGMENT_ADD_FADE,
		GX_M2_FRAGMENT_DIFFUSE_2TEX_OPAQUE_2X, GX_M2_FRAGMENT_DIFFUSE_2TEX_2X, GX_M2_FRAGMENT_DIFFUSE_2TEX_DECAL_2X, GX_M2_FRAGMENT_DIFFUSE_2TEX_ADD_2X, GX_M2_FRAGMENT_DIFFUSE_2TEX_4X, GX_M2_FRAGMENT_DIFFUSE_2TEX_FADE_2X,
	};
	if (fragment_modes[0] >= 5 || fragment_modes[1] >= 6)
		return false;
	combiners[1] = modes[fragment_modes[0] * 6 + fragment_modes[1]];
	return true;
}

static void gx_m2_batch_load(struct gx_m2_batch *batch, struct wow_m2_file *file, struct wow_m2_batch *wow_batch)
{
	uint32_t fragment_modes[2] = {0};
	uint32_t lookups[2] = {0};
	bool tex2env;
	bool tex1env;

	if (wow_batch->shader_id < 0)
	{
		switch (-wow_batch->shader_id)
		{
			default:
#if 0
				LOG_WARN("unknown shader combiner: %d", wow_batch->shader_id);
#endif
				/* FALLTHROUGH */
			case 3:
				fragment_modes[0] = 1; /* test for real values */
				break;
		}
#if 0
		LOG_WARN("using combiner combo %u for %u", fragment_modes[0], batch->id);
#endif
	}
	else
	{
		if (wow_batch->texture_count > 0)
		{
			if (file->header.flags & WOW_M2_HEADER_FLAG_USE_TEXTURE_COMBINER_COMBO)
			{
				for (uint16_t i = 0; i < wow_batch->texture_count; ++i)
				{
					assert((unsigned)wow_batch->shader_id + i < file->texture_combiner_combos_nb);
					fragment_modes[i] = file->texture_combiner_combos[wow_batch->shader_id + i];
#if 0
					LOG_WARN("using combiner combo %u for id %u tex %u", fragment_modes[i], batch->id, i);
#endif
				}
			}
			else
			{
				struct wow_m2_material *material = &file->materials[wow_batch->material_index];
				for (uint16_t i = 0; i < wow_batch->texture_count; ++i)
				{
					static const uint32_t static_fragments[] = {0, 1, 1, 1, 1, 5, 5};
					fragment_modes[i] = static_fragments[material->blend_mode];
				}
			}
		}
	}
	assert(wow_batch->texture_coord_combo_index < file->texture_unit_lookups_nb);
	lookups[0] = file->texture_unit_lookups[wow_batch->texture_coord_combo_index];
	if (wow_batch->texture_count > 1)
	{
		assert(wow_batch->texture_coord_combo_index + 1u < file->texture_unit_lookups_nb);
		lookups[1] = file->texture_unit_lookups[wow_batch->texture_coord_combo_index + 1];
	}
	if (wow_batch->texture_count > 1)
		tex2env = lookups[1] > 2;
	else
		tex2env = false;
	tex1env = lookups[0] > 2 ? 1 : 0;
	if (get_combiners(wow_batch->texture_count, fragment_modes, tex1env, tex2env, batch->combiners))
		return;
	if (wow_batch->texture_count > 1)
		tex2env = lookups[1] > 2;
	else
		tex2env = false;
	fragment_modes[0] = 1;
	fragment_modes[1] = 1;
	if (get_combiners(wow_batch->texture_count, fragment_modes, tex1env, tex2env, batch->combiners))
		return;
	LOG_WARN("undefined combiner :(");
	batch->combiners[0] = GX_M2_VERTEX_DIFFUSE;
	batch->combiners[1] = GX_M2_FRAGMENT_DIFFUSE;
}

static bool gx_m2_batch_init(struct gx_m2_batch *batch, struct gx_m2_profile *parent, struct wow_m2_file *file, struct wow_m2_skin_profile *profile, struct wow_m2_batch *wow_batch, struct jks_array *indices)
{
	batch->parent = parent;
	gx_m2_texture_init(&batch->textures[0]);
	gx_m2_texture_init(&batch->textures[1]);
	struct wow_m2_skin_section *section = &profile->sections[wow_batch->skin_section_index];
	batch->skin_section_id = section->skin_section_id;
	batch->wow_flags = wow_batch->flags;
	gx_m2_batch_load(batch, file, wow_batch);
	batch->indices_offset = indices->size;
	batch->indices_nb = section->index_count;
	batch->priority_plane = wow_batch->priority_plane;
	batch->material = wow_batch->material_index;
	uint16_t *data = jks_array_grow(indices, section->index_count);
	if (!data)
	{
		LOG_ERROR("failed to grow indices array");
		return false;
	}
	for (uint32_t i = 0; i < section->index_count; ++i)
		data[i] = profile->vertexes[profile->indices[section->index_start + i]];
	if (wow_batch->texture_count >= 1)
	{
		gx_m2_texture_load(&batch->textures[0], file, wow_batch, 0);
		if (wow_batch->texture_count >= 2)
			gx_m2_texture_load(&batch->textures[1], file, wow_batch, 1);
	}
	struct wow_m2_material *material = &file->materials[batch->material];
	enum gx_rasterizer_state rasterizer_state;
	enum gx_depth_stencil_state depth_stencil_state;
	enum gx_blend_state blend_state;
	rasterizer_state = (material->flags & WOW_M2_MATERIAL_FLAGS_UNCULLED) ? GX_RASTERIZER_UNCULLED : GX_RASTERIZER_CULLED;
	/* XXX remove this hack */
	if (0 && strstr(batch->parent->parent->filename, "COT_HOURGLASS"))
	{
		if (batch->id == 4)
			material->blend_mode = 4;
	}
	/* COT hourglass:
	 * batch 0 is solid circles
	 * batch 1 is reflection on circles
	 * batch 2 is ornaments
	 * batch 3 is static sand
	 * batch 4 is glass shininess (hourglassgleam.blp): strange thing is that it's a grey-keyed texture (used ofr example as reflection, but in an "add / mix" way), but doing add of this on global looks strange
	 * batch 5 is glass stars reflection (starsportals.blp)
	 * batch 6 is ornaments
	 * batch 7 is flowing sand
	 * batch 8 is flowing sand (more opaque ?)
	 */
	switch (material->blend_mode)
	{
		case 0: /* opaque */
			blend_state = GX_BLEND_OPAQUE;
			batch->blending = false;
			batch->alpha_test = 0.0 / 255.0;
			batch->fog_override = false;
			break;
		case 1: /* alpha key */
			blend_state = GX_BLEND_OPAQUE;
			batch->blending = false;
			batch->alpha_test = 224.0 / 255.0;
			batch->fog_override = false;
			break;
		case 2: /* alpha */
			blend_state = GX_BLEND_ALPHA;
			batch->blending = true;
			batch->alpha_test = 1.0 / 255.0;
			batch->fog_override = false;
			break;
		case 3: /* no alpha add */
			blend_state = GX_BLEND_NO_ALPHA_ADD;
			batch->blending = true;
			batch->alpha_test = 1.0 / 255.0;
			batch->fog_override = true;
			VEC3_SETV(batch->fog_color, 0);
			break;
		case 4: /* add */
			blend_state = GX_BLEND_ADD;
			batch->blending = true;
			batch->alpha_test = 1.0 / 255.0;
			batch->fog_override = true;
			VEC3_SETV(batch->fog_color, 0);
			break;
		case 5: /* mod */
			blend_state = GX_BLEND_MOD;
			batch->blending = true;
			batch->alpha_test = 1.0 / 255.0;
			batch->fog_override = true;
			VEC3_SETV(batch->fog_color, 1);
			batch->wow_flags |= WOW_M2_MATERIAL_FLAGS_UNLIT;
			break;
		case 6: /* mod2x */
			blend_state = GX_BLEND_MOD2X;
			batch->blending = true;
			batch->alpha_test = 1.0 / 255.0;
			batch->fog_override = true;
			VEC3_SETV(batch->fog_color, 0.5);
			batch->wow_flags |= WOW_M2_MATERIAL_FLAGS_UNLIT;
			break;
		default:
			blend_state = GX_BLEND_ALPHA;
			batch->blending = true;
			batch->alpha_test = 1.0 / 255.0;
			batch->fog_override = false;
			LOG_WARN("unsupported blend mode: %u", material->blend_mode);
			break;
	}
	switch ((!(material->flags & WOW_M2_MATERIAL_FLAGS_DEPTH_WRITE)) * 2 + (!(material->flags & WOW_M2_MATERIAL_FLAGS_DEPTH_TEST)) * 1)
	{
		case 0:
			if (gx_m2_flag_get(batch->parent->parent, GX_M2_FLAG_SKYBOX))
				depth_stencil_state = GX_DEPTH_STENCIL_NO_RO;
			else if (batch->blending)
				depth_stencil_state = GX_DEPTH_STENCIL_NO_NO;
			else
				depth_stencil_state = GX_DEPTH_STENCIL_NO_WO;
			break;
		case 1:
			if (gx_m2_flag_get(batch->parent->parent, GX_M2_FLAG_SKYBOX))
				depth_stencil_state = GX_DEPTH_STENCIL_RO_RO;
			else if (batch->blending)
				depth_stencil_state = GX_DEPTH_STENCIL_RO_NO;
			else
				depth_stencil_state = GX_DEPTH_STENCIL_RO_WO;
			break;
		case 2:
			if (gx_m2_flag_get(batch->parent->parent, GX_M2_FLAG_SKYBOX))
				depth_stencil_state = GX_DEPTH_STENCIL_WO_RO;
			else if (batch->blending)
				depth_stencil_state = GX_DEPTH_STENCIL_WO_NO;
			else
				depth_stencil_state = GX_DEPTH_STENCIL_WO_WO;
			break;
		case 3:
			if (gx_m2_flag_get(batch->parent->parent, GX_M2_FLAG_SKYBOX))
				depth_stencil_state = GX_DEPTH_STENCIL_RW_RO;
			else if (batch->blending)
				depth_stencil_state = GX_DEPTH_STENCIL_RW_NO;
			else
				depth_stencil_state = GX_DEPTH_STENCIL_RW_WO;
			break;
	}
	batch->pipeline_state = &g_wow->gx->m2_pipeline_states[rasterizer_state][depth_stencil_state][blend_state] - &g_wow->gx->m2_pipeline_states[0][0][0];
	init_batch_color_transform(batch, file, wow_batch);
	init_batch_texture_weight(batch, file, wow_batch);
	for (size_t i = 0; i < RENDER_FRAMES_COUNT; ++i)
		batch->uniform_buffers[i] = GFX_BUFFER_INIT();
	return true;
}

static void gx_m2_batch_destroy(struct gx_m2_batch *batch)
{
	for (size_t i = 0; i < RENDER_FRAMES_COUNT; ++i)
		gfx_delete_buffer(g_wow->device, &batch->uniform_buffers[i]);
	gx_m2_texture_destroy(&batch->textures[0]);
	gx_m2_texture_destroy(&batch->textures[1]);
}

static void gx_m2_batch_initialize(struct gx_m2_batch *batch)
{
	for (size_t i = 0; i < RENDER_FRAMES_COUNT; ++i)
		gfx_create_buffer(g_wow->device, &batch->uniform_buffers[i], GFX_BUFFER_UNIFORM, NULL, sizeof(struct shader_m2_mesh_block), GFX_BUFFER_STREAM);
}

static bool gx_m2_batch_prepare_draw(struct gx_m2_batch *batch, struct gx_frame *frame, struct gx_m2_render_params *params, bool pipeline_state)
{
	struct gx_m2 *m2 = batch->parent->parent;
	struct vec4f color = {1.0f, 1.0f, 1.0f, 1.0f};
	if (batch->has_color_transform)
	{
		{
			struct vec3f v;
			if (m2_get_track_value_vec3f(m2, &m2->colors[batch->color_transform].color, &v, NULL, frame->time / 1000000))
				VEC3_CPY(color, v);
		}
		{
			int16_t v;
			if (m2_get_track_value_int16(m2, &m2->colors[batch->color_transform].alpha, &v, NULL, frame->time / 1000000))
				color.w *= v / (float)INT16_MAX;
		}
	}
	if (batch->has_texture_weight)
	{
		int16_t v;
		if (m2_get_track_value_int16(m2, &m2->texture_weights[batch->texture_weight].weight, &v, NULL, frame->time / 1000000))
			color.w *= v / (float)INT16_MAX;
	}
	if (color.w == 0)
		return false;
	struct shader_m2_mesh_block mesh_block;
	gx_m2_texture_update_matrix(&batch->textures[0], frame, batch, &mesh_block.tex1_matrix);
	gx_m2_texture_update_matrix(&batch->textures[1], frame, batch, &mesh_block.tex2_matrix);
	struct wow_m2_material *material = &m2->materials[batch->material];
	mesh_block.settings.x = (material->flags & WOW_M2_MATERIAL_FLAGS_UNFOGGED) ? 1 : 0;
	mesh_block.settings.y = (material->flags & WOW_M2_MATERIAL_FLAGS_UNLIT) ? 1 : 0;
	mesh_block.settings.z = m2->bones_nb;
	mesh_block.settings.w = 0;
	mesh_block.combiners.x = batch->combiners[0];
	mesh_block.combiners.y = batch->combiners[1];
	mesh_block.combiners.z = 0;
	mesh_block.combiners.w = 0;
	mesh_block.color = color;
	if (batch->fog_override)
	{
		mesh_block.fog_color = batch->fog_color;
	}
	else
	{
		if (params)
		{
			mesh_block.fog_color = params->fog_color;
		}
		else
		{
			VEC3_CPY(mesh_block.fog_color, g_wow->map->gx_skybox->int_values[SKYBOX_INT_FOG]);
		}
	}
	mesh_block.alpha_test = batch->alpha_test * color.w;
	gfx_set_buffer_data(&batch->uniform_buffers[frame->id], &mesh_block, sizeof(mesh_block), 0);
	gfx_bind_constant(g_wow->device, 0, &batch->uniform_buffers[frame->id], sizeof(struct shader_m2_mesh_block), 0);
	if (pipeline_state)
		gfx_bind_pipeline_state(g_wow->device, &((gfx_pipeline_state_t*)g_wow->gx->m2_pipeline_states)[batch->pipeline_state]);
	return true;
}

static void gx_m2_batch_render(struct gx_m2_batch *batch, struct gx_frame *frame, struct gx_m2_instance *instance)
{
	struct gx_m2_instance_frame *instance_frame = &instance->frames[frame->id];
	const gfx_texture_t *textures[2];
	textures[0] = gx_m2_texture_bind(&batch->textures[0], instance);
	textures[1] = gx_m2_texture_bind(&batch->textures[1], instance);
	gfx_bind_samplers(g_wow->device, 0, 2, textures);
	gfx_bind_constant(g_wow->device, 1, &instance_frame->uniform_buffer, sizeof(struct shader_m2_model_block), 0); /* XXX really there ? */
	gfx_draw_indexed(g_wow->device, batch->indices_nb, batch->indices_offset);
}

static void gx_m2_batch_render_instanced(struct gx_m2_batch *batch, size_t instances_count)
{
	const gfx_texture_t *textures[2];
	textures[0] = gx_m2_texture_bind_instanced(&batch->textures[0]);
	textures[1] = gx_m2_texture_bind_instanced(&batch->textures[1]);
	gfx_bind_samplers(g_wow->device, 0, 2, textures);
	gfx_draw_indexed_instanced(g_wow->device, batch->indices_nb, batch->indices_offset, instances_count);
}

static int batch_cmp(const void *v1, const void *v2)
{
	const struct gx_m2_batch *b1 = v1;
	const struct gx_m2_batch *b2 = v2;
	if (b1->priority_plane > b2->priority_plane)
		return 1;
	if (b2->priority_plane > b1->priority_plane)
		return -1;
	if (b1->id > b2->id)
		return 1;
	if (b2->id > b1->id)
		return -1;
	return 0;
}

static bool gx_m2_profile_init(struct gx_m2_profile *profile, struct gx_m2 *parent, struct wow_m2_file *file, struct wow_m2_skin_profile *wow_profile, struct jks_array *indices)
{
	profile->parent = parent;
	profile->initialized = false;
	jks_array_init(&profile->batches, sizeof(struct gx_m2_batch), (jks_array_destructor_t)gx_m2_batch_destroy, &jks_array_memory_fn_GX);
	jks_array_init(&profile->transparent_batches, sizeof(uint8_t), NULL, &jks_array_memory_fn_GX);
	jks_array_init(&profile->opaque_batches, sizeof(uint8_t), NULL, &jks_array_memory_fn_GX);
	if (!jks_array_reserve(&profile->batches, wow_profile->batches_nb))
		return false;
	for (uint32_t i = 0; i < wow_profile->batches_nb; ++i)
	{
		struct wow_m2_batch *batch = &wow_profile->batches[i];
		struct gx_m2_batch *batch_profile = jks_array_grow(&profile->batches, 1);
		if (!batch_profile)
			return false;
		batch_profile->id = i;
		if (!gx_m2_batch_init(batch_profile, profile, file, wow_profile, batch, indices))
			return false;
	}
	if (profile->batches.size)
		qsort(profile->batches.data, profile->batches.size, sizeof(struct gx_m2_batch), batch_cmp);
	for (uint8_t i = 0; i < profile->batches.size; ++i)
	{
		struct gx_m2_batch *batch_profile = JKS_ARRAY_GET(&profile->batches, i, struct gx_m2_batch);
		if (batch_profile->blending)
		{
			if (!jks_array_push_back(&profile->transparent_batches, &i))
				return false;
		}
		else
		{
			if (!jks_array_push_back(&profile->opaque_batches, &i))
				return false;
		}
	}
	return true;
}

static void gx_m2_profile_destroy(struct gx_m2_profile *profile)
{
	jks_array_destroy(&profile->batches);
	jks_array_destroy(&profile->transparent_batches);
	jks_array_destroy(&profile->opaque_batches);
}

static void gx_m2_profile_initialize(struct gx_m2_profile *profile)
{
	for (size_t i = 0; i < profile->batches.size; ++i)
		gx_m2_batch_initialize(JKS_ARRAY_GET(&profile->batches, i, struct gx_m2_batch));
	profile->initialized = true;
}

static bool should_render_profile(struct gx_m2_instance *instance, struct gx_m2_batch *batch)
{
	if (batch->skin_section_id == 0 || !gx_m2_instance_flag_get(instance, GX_M2_INSTANCE_FLAG_DYN_BATCHES)) /* XXX: Is it really always the case ? (I think not, biodomes have a bizarre thing) */
		return true;
	for (size_t i = 0; i < instance->enabled_batches.size; ++i)
	{
		if (batch->skin_section_id == *JKS_ARRAY_GET(&instance->enabled_batches, i, uint16_t))
			return true;
	}
	return false;
}

#if 0
#define TEST_BATCH 4
#endif

static void gx_m2_profile_render(struct gx_m2_profile *profile, struct gx_frame *frame, struct jks_array *instances, bool transparent)
{
	if (!profile->initialized)
		return;
	struct jks_array *batches = transparent ? &profile->transparent_batches : &profile->opaque_batches;
	for (size_t i = 0; i < batches->size; ++i)
	{
		uint8_t batch_id = *JKS_ARRAY_GET(batches, i, uint8_t);
		bool initialized = false;
		struct gx_m2_batch *batch = JKS_ARRAY_GET(&profile->batches, batch_id, struct gx_m2_batch);
#ifdef TEST_BATCH
		if (strstr(profile->parent->filename, "HOURGLASS"))
		{
			if (batch->id != TEST_BATCH)
				continue;
		}
#endif
		for (size_t j = 0; j < instances->size; ++j)
		{
			struct gx_m2_instance *instance = *JKS_ARRAY_GET(instances, j, struct gx_m2_instance*);
			if (!should_render_profile(instance, batch))
				continue;
			if (!initialized)
			{
				if (!gx_m2_batch_prepare_draw(batch, frame, NULL, true))
					break;
				initialized = true;
			}
			if (instance->lighting_type != frame->m2_lighting_type
			 || instance->lighting_type == GX_M2_LIGHTING_WMO_INDOOR)
			{
				frame->m2_lighting_type = instance->lighting_type;
				gfx_buffer_t *uniform_buffer;
				switch (instance->lighting_type)
				{
					case GX_M2_LIGHTING_WMO_INDOOR:
						uniform_buffer = &instance->local_lighting->uniform_buffer;
						break;
					default:
					case GX_M2_LIGHTING_WORLD:
					case GX_M2_LIGHTING_WMO_OUTDOOR:
						uniform_buffer = &frame->m2_world_uniform_buffer;
						break;
					case GX_M2_LIGHTING_GROUND_SHADOW:
						uniform_buffer = &frame->m2_ground_shadow_uniform_buffer;
						break;
					case GX_M2_LIGHTING_GROUND_LIGHT:
						uniform_buffer = &frame->m2_ground_light_uniform_buffer;
						break;
				}
				gfx_bind_constant(g_wow->device, 2, uniform_buffer, sizeof(struct shader_m2_scene_block), 0);
			}
			gx_m2_batch_render(batch, frame, instance);
		}
	}
}

static void gx_m2_profile_render_instance(struct gx_m2_profile *profile, struct gx_frame *frame, struct gx_m2_instance *instance, bool transparent, struct gx_m2_render_params *params)
{
	if (!profile->initialized)
		return;
	struct jks_array *batches = transparent ? &profile->transparent_batches : &profile->opaque_batches;
	for (size_t i = 0; i < batches->size; ++i)
	{
		uint8_t batch_id = *JKS_ARRAY_GET(batches, i, uint8_t);
		struct gx_m2_batch *batch = JKS_ARRAY_GET(&profile->batches, batch_id, struct gx_m2_batch);
#ifdef TEST_BATCH
		if (strstr(profile->parent->filename, "HOURGLASS"))
		{
			if (batch->id != TEST_BATCH)
				continue;
		}
#endif
		if (!should_render_profile(instance, batch))
			continue;
		if (!gx_m2_batch_prepare_draw(batch, frame, params, true))
			continue;
		gx_m2_batch_render(batch, frame, instance);
	}
}

struct gx_m2 *gx_m2_new(char *filename)
{
	struct gx_m2 *m2 = mem_malloc(MEM_GX, sizeof(*m2));
	if (!m2)
		return NULL;
	m2->filename = filename;
	m2->flags = 0;
	for (size_t i = 0; i < RENDER_FRAMES_COUNT; ++i)
	{
		jks_array_init(&m2->frames[i].shadow_to_render, sizeof(struct gx_m2_instance*), NULL, &jks_array_memory_fn_GX);
		jks_array_init(&m2->frames[i].to_render, sizeof(struct gx_m2_instance*), NULL, &jks_array_memory_fn_GX);
		m2->frames[i].updated = false;
		pthread_mutex_init(&m2->frames[i].mutex, NULL);
	}
	jks_array_init(&m2->instances, sizeof(struct gx_m2_instance*), NULL, &jks_array_memory_fn_GX);
	jks_array_init(&m2->profiles, sizeof(struct gx_m2_profile), (jks_array_destructor_t)gx_m2_profile_destroy, &jks_array_memory_fn_GX);
	jks_array_init(&m2->indices, sizeof(uint16_t), NULL, &jks_array_memory_fn_GX);
#ifdef WITH_DEBUG_RENDERING
	gx_m2_collisions_init(&m2->gx_collisions);
	gx_m2_lights_init(&m2->gx_lights);
	gx_m2_bones_init(&m2->gx_bones);
#endif
	m2->playable_animations = NULL;
	m2->texture_transforms = NULL;
	m2->attachment_lookups = NULL;
	m2->collision_triangles = NULL;
	m2->collision_vertexes = NULL;
	m2->collision_normals = NULL;
	m2->key_bone_lookups = NULL;
	m2->sequence_lookups = NULL;
	m2->global_sequences = NULL;
	m2->texture_weights = NULL;
	m2->bone_lookups = NULL;
	m2->attachments = NULL;
	m2->sequences = NULL;
	m2->particles = NULL;
	m2->materials = NULL;
	m2->textures = NULL;
	m2->vertexes = NULL;
	m2->cameras = NULL;
	m2->ribbons = NULL;
	m2->colors = NULL;
	m2->lights = NULL;
	m2->bones = NULL;
	m2->attributes_state = GFX_ATTRIBUTES_STATE_INIT();
	m2->vertexes_buffer = GFX_BUFFER_INIT();
	m2->indices_buffer = GFX_BUFFER_INIT();
	refcount_init(&m2->refcount, 1);
	return m2;
}

static void m2_unload_task(struct wow_mpq_compound *mpq_compound, void *userdata)
{
	(void)mpq_compound;
	struct gx_m2 *m2 = userdata;
	for (size_t i = 0; i < RENDER_FRAMES_COUNT; ++i)
	{
		jks_array_destroy(&m2->frames[i].shadow_to_render);
		jks_array_destroy(&m2->frames[i].to_render);
		pthread_mutex_destroy(&m2->frames[i].mutex);
	}
	jks_array_destroy(&m2->instances);
	jks_array_destroy(&m2->profiles);
	jks_array_destroy(&m2->indices);
#ifdef WITH_DEBUG_RENDERING
	gx_m2_collisions_destroy(&m2->gx_collisions);
	gx_m2_lights_destroy(&m2->gx_lights);
	gx_m2_bones_destroy(&m2->gx_bones);
#endif
	mem_free(MEM_GX, m2->playable_animations);
	wow_m2_texture_transforms_delete(m2->texture_transforms, m2->texture_transforms_nb);
	wow_m2_texture_weights_delete(m2->texture_weights, m2->texture_weights_nb);
	mem_free(MEM_GX, m2->collision_vertexes);
	mem_free(MEM_GX, m2->collision_normals);
	mem_free(MEM_GX, m2->collision_triangles);
	mem_free(MEM_GX, m2->attachment_lookups);
	mem_free(MEM_GX, m2->key_bone_lookups);
	mem_free(MEM_GX, m2->sequence_lookups);
	mem_free(MEM_GX, m2->global_sequences);
	mem_free(MEM_GX, m2->bone_lookups);
	wow_m2_attachments_delete(m2->attachments, m2->attachments_nb);
	mem_free(MEM_GX, m2->sequences);
	wow_m2_particles_delete(m2->particles, m2->particles_nb);
	mem_free(MEM_GX, m2->materials);
	wow_m2_textures_delete(m2->textures, m2->textures_nb);
	wow_m2_ribbons_delete(m2->ribbons, m2->ribbons_nb);
	mem_free(MEM_GX, m2->vertexes);
	wow_m2_cameras_delete(m2->cameras, m2->cameras_nb);
	wow_m2_colors_delete(m2->colors, m2->colors_nb);
	wow_m2_lights_delete(m2->lights, m2->lights_nb);
	wow_m2_bones_delete(m2->bones, m2->bones_nb);
	gfx_delete_buffer(g_wow->device, &m2->vertexes_buffer);
	gfx_delete_buffer(g_wow->device, &m2->indices_buffer);
	gfx_delete_attributes_state(g_wow->device, &m2->attributes_state);
	mem_free(MEM_GENERIC, m2->filename);
	mem_free(MEM_GX, m2);
}

void gx_m2_free(struct gx_m2 *m2)
{
	if (!m2)
		return;
	if (refcount_dec(&m2->refcount))
		return;
	cache_lock_m2(g_wow->cache);
	if (refcount_get(&m2->refcount))
	{
		cache_unlock_m2(g_wow->cache);
		return;
	}
	cache_unref_unmutexed_m2(g_wow->cache, m2->filename);
	cache_unlock_m2(g_wow->cache);
	loader_push(g_wow->loader, ASYNC_TASK_M2_UNLOAD, m2_unload_task, m2);
}

void gx_m2_ref(struct gx_m2 *m2)
{
	refcount_inc(&m2->refcount);
}

static bool initialize(void *userdata)
{
	struct gx_m2 *m2 = userdata;
	for (size_t i = 0; i < m2->profiles.size; ++i)
	{
		struct gx_m2_profile *profile = JKS_ARRAY_GET(&m2->profiles, i, struct gx_m2_profile);
		if (!profile->initialized)
		{
			gx_m2_profile_initialize(profile);
			return false;
		}
	}
	if (m2->vertexes_nb)
		gfx_create_buffer(g_wow->device, &m2->vertexes_buffer, GFX_BUFFER_VERTEXES, m2->vertexes, m2->vertexes_nb * sizeof(*m2->vertexes), GFX_BUFFER_IMMUTABLE);
	mem_free(MEM_GX, m2->vertexes);
	m2->vertexes = NULL;
	if (m2->indices.size)
		gfx_create_buffer(g_wow->device, &m2->indices_buffer, GFX_BUFFER_INDICES, m2->indices.data, m2->indices.size * sizeof(uint16_t), GFX_BUFFER_IMMUTABLE);
	jks_array_resize(&m2->indices, 0);
	jks_array_shrink(&m2->indices);
	const struct gfx_attribute_bind binds[] =
	{
		{&m2->vertexes_buffer},
	};
	gfx_create_attributes_state(g_wow->device, &m2->attributes_state, binds, sizeof(binds) / sizeof(*binds), &m2->indices_buffer, GFX_INDEX_UINT16);
#ifdef WITH_DEBUG_RENDERING
	gx_m2_collisions_initialize(&m2->gx_collisions);
	gx_m2_lights_initialize(&m2->gx_lights);
	gx_m2_bones_initialize(&m2->gx_bones);
#endif
	gx_m2_flag_set(m2, GX_M2_FLAG_INITIALIZED);
	gx_m2_free(m2);
	return true;
}

static void load(struct gx_m2 *m2, struct wow_m2_file *file)
{
	struct vec3f p0 = {file->header.aabb0.x, file->header.aabb0.z, -file->header.aabb0.y};
	struct vec3f p1 = {file->header.aabb1.x, file->header.aabb1.z, -file->header.aabb1.y};
	VEC3_MIN(m2->aabb.p0, p0, p1);
	VEC3_MAX(m2->aabb.p1, p0, p1);
	VEC3_SET(p0, file->header.caabb0.x, file->header.caabb0.z, -file->header.caabb0.y);
	VEC3_SET(p1, file->header.caabb1.x, file->header.caabb1.z, -file->header.caabb1.y);
	VEC3_MIN(m2->caabb.p0, p0, p1);
	VEC3_MAX(m2->caabb.p1, p0, p1);
	m2->collision_sphere_radius = file->header.collision_sphere_radius;
	m2->version = file->header.version;
	m2->wow_flags = file->header.flags;
	m2->render_distance = file->header.bounding_sphere_radius;
	m2->playable_animations_nb = file->playable_animations_nb;
	m2->playable_animations = mem_malloc(MEM_GX, sizeof(*m2->playable_animations) * m2->playable_animations_nb);
	if (!m2->playable_animations)
	{
		LOG_ERROR("failed to allocate m2 attachment lookups");
		return;
	}
	memcpy(m2->playable_animations, file->playable_animations, sizeof(*m2->playable_animations) * m2->playable_animations_nb);
	m2->texture_transforms_nb = file->texture_transforms_nb;
	m2->texture_transforms = wow_m2_texture_transforms_dup(file->texture_transforms, file->texture_transforms_nb);
	if (m2->texture_transforms_nb && !m2->texture_transforms)
	{
		LOG_ERROR("failed to allocate m2 m2 texture transforms");
		return;
	}
	m2->texture_weights_nb = file->texture_weights_nb;
	m2->texture_weights = wow_m2_texture_weights_dup(file->texture_weights, file->texture_weights_nb);
	if (m2->texture_weights_nb && !m2->texture_weights)
	{
		LOG_ERROR("failed to allocate m2 m2 texture weights");
		return;
	}
	m2->attachment_lookups_nb = file->attachment_lookups_nb;
	m2->attachment_lookups = mem_malloc(MEM_GX, sizeof(*m2->attachment_lookups) * m2->attachment_lookups_nb);
	if (!m2->attachment_lookups)
	{
		LOG_ERROR("failed to allocate m2 attachment lookups");
		return;
	}
	memcpy(m2->attachment_lookups, file->attachment_lookups, sizeof(*m2->attachment_lookups) * m2->attachment_lookups_nb);
	m2->attachments_nb = file->attachments_nb;
	m2->attachments = wow_m2_attachments_dup(file->attachments, file->attachments_nb);
	if (m2->attachments_nb && !m2->attachments)
	{
		LOG_ERROR("failed to allocate m2 attachments");
		return;
	}
	m2->collision_triangles_nb = file->collision_triangles_nb;
	m2->collision_triangles = mem_malloc(MEM_GX, sizeof(*m2->collision_triangles) * m2->collision_triangles_nb);
	if (!m2->collision_triangles)
	{
		LOG_ERROR("failed to allocate m2 collision triangles");
		return;
	}
	memcpy(m2->collision_triangles, file->collision_triangles, sizeof(*m2->collision_triangles) * m2->collision_triangles_nb);
	m2->collision_vertexes_nb = file->collision_vertexes_nb;
	m2->collision_vertexes = mem_malloc(MEM_GX, sizeof(*m2->collision_vertexes) * m2->collision_vertexes_nb);
	if (!m2->collision_vertexes)
	{
		LOG_ERROR("failed to allocate m2 collision vertexes");
		return;
	}
	memcpy(m2->collision_vertexes, file->collision_vertexes, sizeof(*m2->collision_vertexes) * m2->collision_vertexes_nb);
	m2->collision_normals_nb = file->collision_normals_nb;
	m2->collision_normals = mem_malloc(MEM_GX, sizeof(*m2->collision_normals) * m2->collision_normals_nb);
	if (!m2->collision_normals)
	{
		LOG_ERROR("failed to allocate m2 collision normals");
		return;
	}
	memcpy(m2->collision_normals, file->collision_normals, sizeof(*m2->collision_normals) * m2->collision_normals_nb);
	m2->sequences_nb = file->sequences_nb;
	m2->sequences = mem_malloc(MEM_GX, sizeof(*m2->sequences) * m2->sequences_nb);
	if (m2->sequences_nb && !m2->sequences)
	{
		LOG_ERROR("failed to allocate m2 sequences");
		return;
	}
	memcpy(m2->sequences, file->sequences, sizeof(*m2->sequences) * m2->sequences_nb);
	m2->particles_nb = file->particles_nb;
	m2->particles = wow_m2_particles_dup(file->particles, file->particles_nb);
	if (m2->particles_nb && !m2->particles)
	{
		LOG_ERROR("failed to allocate m2 particles");
		return;
	}
	m2->ribbons_nb = file->ribbons_nb;
	m2->ribbons = wow_m2_ribbons_dup(file->ribbons, file->ribbons_nb);
	if (m2->ribbons_nb && !m2->ribbons)
	{
		LOG_ERROR("failed to allocate m2 ribbons");
		return;
	}
	m2->key_bone_lookups_nb = file->key_bone_lookups_nb;
	m2->key_bone_lookups = mem_malloc(MEM_GX, sizeof(*m2->key_bone_lookups) * m2->key_bone_lookups_nb);
	if (m2->key_bone_lookups_nb && !m2->key_bone_lookups)
	{
		LOG_ERROR("failed to allocate m2 key bone lookups");
		return;
	}
	memcpy(m2->key_bone_lookups, file->key_bone_lookups, sizeof(*m2->key_bone_lookups) * m2->key_bone_lookups_nb);
	m2->sequence_lookups_nb = file->sequence_lookups_nb;
	m2->sequence_lookups = mem_malloc(MEM_GX, sizeof(*m2->sequence_lookups) * m2->sequence_lookups_nb);
	if (m2->sequence_lookups_nb && !m2->sequence_lookups)
	{
		LOG_ERROR("failed to allocate m2 sequence lookups");
		return;
	}
	memcpy(m2->sequence_lookups, file->sequence_lookups, sizeof(*m2->sequence_lookups) * m2->sequence_lookups_nb);
	m2->global_sequences_nb = file->global_sequences_nb;
	m2->global_sequences = mem_malloc(MEM_GX, sizeof(*m2->global_sequences) * m2->global_sequences_nb);
	if (m2->global_sequences_nb && !m2->global_sequences)
	{
		LOG_ERROR("failed to allocate m2 global sequences");
		return;
	}
	memcpy(m2->global_sequences, file->global_sequences, sizeof(*m2->global_sequences) * m2->global_sequences_nb);
	m2->bone_lookups_nb = file->bone_lookups_nb;
	m2->bone_lookups = mem_malloc(MEM_GX, sizeof(*m2->bone_lookups) * m2->bone_lookups_nb);
	if (m2->bone_lookups_nb && !m2->bone_lookups)
	{
		LOG_ERROR("failed to allocate m2 bone lookups");
		return;
	}
	memcpy(m2->bone_lookups, file->bone_lookups, sizeof(*m2->bone_lookups) * m2->bone_lookups_nb);
	m2->materials_nb = file->materials_nb;
	m2->materials = mem_malloc(MEM_GX, sizeof(*m2->materials) * m2->materials_nb);
	if (m2->materials_nb && !m2->materials)
	{
		LOG_ERROR("failed to allocate m2 materials");
		return;
	}
	memcpy(m2->materials, file->materials, sizeof(*m2->materials) * m2->materials_nb);
	m2->textures_nb = file->textures_nb;
	m2->textures = wow_m2_textures_dup(file->textures, file->textures_nb);
	if (m2->textures_nb && !m2->textures)
	{
		LOG_ERROR("failed to allocate m2 textures");
		return;
	}
	m2->vertexes_nb = file->vertexes_nb;
	m2->vertexes = mem_malloc(MEM_GX, sizeof(*m2->vertexes) * m2->vertexes_nb);
	if (m2->vertexes_nb && !m2->vertexes)
	{
		LOG_ERROR("failed to allocate m2 vertexes");
		return;
	}
	memcpy(m2->vertexes, file->vertexes, sizeof(*m2->vertexes) * m2->vertexes_nb);
	m2->cameras_nb = file->cameras_nb;
	m2->cameras = wow_m2_cameras_dup(file->cameras, file->cameras_nb);
	if (m2->cameras_nb && !m2->cameras)
	{
		LOG_ERROR("failed to allocate m2 cameras");
		return;
	}
	m2->colors_nb = file->colors_nb;
	m2->colors = wow_m2_colors_dup(file->colors, file->colors_nb);
	if (m2->colors_nb && !m2->colors)
	{
		LOG_ERROR("failed to allocate m2 colors");
		return;
	}
	m2->lights_nb = file->lights_nb;
	m2->lights = wow_m2_lights_dup(file->lights, file->lights_nb);
	if (m2->lights_nb && !m2->lights)
	{
		LOG_ERROR("failed to allocate m2 lights");
		return;
	}
	m2->bones_nb = file->bones_nb;
	m2->bones = wow_m2_bones_dup(file->bones, file->bones_nb);
	if (m2->bones_nb && !m2->bones)
	{
		LOG_ERROR("failed to allocate m2 bones");
		return;
	}
	for (uint32_t i = 0; i < m2->particles_nb; ++i)
	{
		struct wow_m2_particle *particle = &m2->particles[i];
		particle->position = (struct wow_vec3f){particle->position.x, particle->position.z, -particle->position.y};
		particle->wind_vector = (struct wow_vec3f){particle->wind_vector.x, particle->wind_vector.z, -particle->wind_vector.y};
	}
	for (uint32_t i = 0; i < m2->ribbons_nb; ++i)
	{
		struct wow_m2_ribbon *ribbon = &m2->ribbons[i];
		ribbon->position = (struct wow_vec3f){ribbon->position.x, ribbon->position.z, -ribbon->position.y};
	}
	for (uint32_t i = 0; i < m2->attachments_nb; ++i)
	{
		struct wow_m2_attachment *attachment = &m2->attachments[i];
		attachment->position = (struct wow_vec3f){attachment->position.x, attachment->position.z, -attachment->position.y};
	}
	for (uint32_t i = 0; i < m2->lights_nb; ++i)
	{
		struct wow_m2_light *light = &m2->lights[i];
		light->position = (struct wow_vec3f){light->position.x, light->position.z, -light->position.y};
	}
	for (uint32_t i = 0; i < m2->bones_nb; ++i)
	{
		struct wow_m2_bone *bone = &m2->bones[i];
		bone->pivot = (struct wow_vec3f){bone->pivot.x, bone->pivot.z, -bone->pivot.y};
	}
	for (uint32_t i = 0; i < m2->collision_vertexes_nb; ++i)
	{
		struct wow_vec3f *v = &m2->collision_vertexes[i];
		*v = (struct wow_vec3f){v->x, v->z, -v->y};
	}
	for (uint32_t i = 0; i < m2->collision_normals_nb; ++i)
	{
		struct wow_vec3f *v = &m2->collision_normals[i];
		*v = (struct wow_vec3f){v->x, v->z, -v->y};
	}
	for (uint32_t i = 0; i < m2->vertexes_nb; ++i)
	{
		struct wow_vec3f *v = &m2->vertexes[i].pos;
		*v = (struct wow_vec3f){v->x, v->z, -v->y};
		v = &m2->vertexes[i].normal;
		*v = (struct wow_vec3f){v->x, v->z, -v->y};
	}
	for (uint32_t i = 0; i < m2->bones_nb; ++i)
	{
		struct wow_m2_bone *bone = &m2->bones[i];
		if (bone->flags & WOW_M2_BONE_BILLBOARD)
		{
			gx_m2_flag_set(m2, GX_M2_FLAG_HAS_BILLBOARD_BONES);
			break;
		}
	}
	for (size_t i = file->skin_profiles_nb - 1; i < file->skin_profiles_nb; ++i)
	{
		struct gx_m2_profile *profile = jks_array_grow(&m2->profiles, 1);
		if (!profile)
		{
			LOG_ERROR("failed to grow skin profiles array");
			continue;
		}
		if (!gx_m2_profile_init(profile, m2, file, &file->skin_profiles[i], &m2->indices))
		{
			LOG_ERROR("failed to initialize m2 skin profile");
			jks_array_resize(&m2->profiles, m2->profiles.size - 1);
			continue;
		}
		for (size_t j = 0; j < profile->batches.size; ++j)
		{
			struct gx_m2_batch *batch = JKS_ARRAY_GET(&profile->batches, j, struct gx_m2_batch);
			if (batch->blending)
				gx_m2_flag_set(m2, GX_M2_FLAG_HAS_TRANS_BATCHES);
			else
				gx_m2_flag_set(m2, GX_M2_FLAG_HAS_OPAQUE_BATCHES);
		}
	}
#ifdef WITH_DEBUG_RENDERING
	if (!gx_m2_collisions_load(&m2->gx_collisions, m2))
		LOG_ERROR("failed to load m2 collisions m2");
	if (!gx_m2_lights_load(&m2->gx_lights, m2->lights, m2->lights_nb))
		LOG_ERROR("failed to load m2 lights m2");
	if (!gx_m2_bones_load(&m2->gx_bones, m2->bones, m2->bones_nb))
		LOG_ERROR("failed to load m2 bones m2");
#endif
	gx_m2_flag_set(m2, GX_M2_FLAG_LOADED);
	cache_lock_m2(g_wow->cache);
	for (size_t i = 0; i < m2->instances.size; ++i)
	{
		struct gx_m2_instance *instance = *JKS_ARRAY_GET(&m2->instances, i, struct gx_m2_instance*);
		gx_m2_instance_on_parent_loaded(instance);
	}
	cache_unlock_m2(g_wow->cache);
	gx_m2_ref(m2);
	loader_init_object(g_wow->loader, LOADER_M2, initialize, m2);
}

static void m2_load_task(struct wow_mpq_compound *mpq_compound, void *userdata)
{
	struct gx_m2 *m2 = userdata;
	struct wow_mpq_file *mpq_file = NULL;
	struct wow_m2_file *m2_file = NULL;

	mpq_file = wow_mpq_get_file(mpq_compound, m2->filename);
	if (!mpq_file)
	{
		LOG_WARN("M2 file not found: %s", m2->filename);
		goto end;
	}
	m2_file = wow_m2_file_new(mpq_file);
	if (!m2_file)
	{
		LOG_ERROR("failed to create m2 from file %s", m2->filename);
		goto end;
	}
	load(m2, m2_file);

end:
	wow_mpq_file_delete(mpq_file);
	wow_m2_file_delete(m2_file);
	gx_m2_free(m2);
}

void gx_m2_ask_load(struct gx_m2 *m2)
{
	if (gx_m2_flag_set(m2, GX_M2_FLAG_LOAD_ASKED))
		return;
	gx_m2_ref(m2);
	loader_push(g_wow->loader, ASYNC_TASK_M2_LOAD, m2_load_task, m2);
}

void gx_m2_render(struct gx_m2 *m2, struct gx_frame *frame, bool transparent)
{
	if (!gx_m2_flag_get(m2, GX_M2_FLAG_INITIALIZED))
		return;
	gfx_bind_attributes_state(g_wow->device, &m2->attributes_state, &g_wow->gx->m2_input_layout);
	struct gx_m2_frame *m2_frame = &m2->frames[frame->id];
	if (!m2_frame->updated)
	{
		for (size_t i = 0; i < m2_frame->to_render.size; ++i)
		{
			struct gx_m2_instance *instance = *JKS_ARRAY_GET(&m2_frame->to_render, i, struct gx_m2_instance*);
			update_instance_uniform_buffer(instance, frame);
		}
		m2_frame->updated = true;
	}
	int profile_id = 0;
	struct gx_m2_profile *profile = JKS_ARRAY_GET(&m2->profiles, profile_id, struct gx_m2_profile);
	gx_m2_profile_render(profile, frame, &m2_frame->to_render, transparent);
}

static void gx_m2_render_instance(struct gx_m2 *m2, struct gx_frame *frame, struct gx_m2_instance *instance, bool transparent, struct gx_m2_render_params *params)
{
	if (!gx_m2_flag_get(m2, GX_M2_FLAG_INITIALIZED))
		return;
	gfx_bind_attributes_state(g_wow->device, &m2->attributes_state, &g_wow->gx->m2_input_layout);
	update_instance_uniform_buffer(instance, frame);
	int profile_id = 0;
	struct gx_m2_profile *profile = JKS_ARRAY_GET(&m2->profiles, profile_id, struct gx_m2_profile);
	gx_m2_profile_render_instance(profile, frame, instance, transparent, params);
}

#ifdef WITH_DEBUG_RENDERING
void gx_m2_render_bones_points(struct gx_m2 *m2, struct gx_frame *frame)
{
	if (!gx_m2_flag_get(m2, GX_M2_FLAG_INITIALIZED))
		return;
	struct gx_m2_frame *m2_frame = &m2->frames[frame->id];
	gx_m2_bones_render_points(&m2->gx_bones, frame, m2_frame->to_render.data, m2_frame->to_render.size);
}

void gx_m2_render_bones_lines(struct gx_m2 *m2, struct gx_frame *frame)
{
	if (!gx_m2_flag_get(m2, GX_M2_FLAG_INITIALIZED))
		return;
	struct gx_m2_frame *m2_frame = &m2->frames[frame->id];
	gx_m2_bones_render_lines(&m2->gx_bones, frame, m2_frame->to_render.data, m2_frame->to_render.size);
}

void gx_m2_render_lights(struct gx_m2 *m2, struct gx_frame *frame)
{
	if (!gx_m2_flag_get(m2, GX_M2_FLAG_INITIALIZED))
		return;
	struct gx_m2_frame *m2_frame = &m2->frames[frame->id];
	gx_m2_lights_render(&m2->gx_lights, frame, m2_frame->to_render.data, m2_frame->to_render.size);
}

void gx_m2_render_collisions(struct gx_m2 *m2, struct gx_frame *frame, bool triangles)
{
	if (!gx_m2_flag_get(m2, GX_M2_FLAG_INITIALIZED))
		return;
	struct gx_m2_frame *m2_frame = &m2->frames[frame->id];
	gx_m2_collisions_render(&m2->gx_collisions, frame, m2_frame->to_render.data, m2_frame->to_render.size, triangles);
}
#endif

void gx_m2_clear_update(struct gx_m2 *m2, struct gx_frame *frame)
{
	struct gx_m2_frame *m2_frame = &m2->frames[frame->id];
	for (size_t i = 0; i < m2_frame->to_render.size; ++i)
	{
		struct gx_m2_instance *instance = *JKS_ARRAY_GET(&m2_frame->to_render, i, struct gx_m2_instance*);
		gx_m2_instance_clear_update(instance, frame);
	}
	jks_array_resize(&m2_frame->shadow_to_render, 0);
	jks_array_resize(&m2_frame->to_render, 0);
	m2_frame->updated = false;
}

static bool instance_init(struct gx_m2_instance *instance)
{
	if (!jks_array_push_back(&instance->parent->instances, &instance))
	{
		LOG_ERROR("failed to add m2 instance to parent");
		return false;
	}
	MAT4_IDENTITY(instance->m_inv);
	MAT4_IDENTITY(instance->m);
	instance->camera = -1;
	instance->sequence_speed = 1;
	instance->render_distance_max = -1;
	instance->scale = 1;
	instance->lighting_type = GX_M2_LIGHTING_WORLD;
	for (size_t i = 0; i < RENDER_FRAMES_COUNT; ++i)
	{
		struct gx_m2_instance_frame *instance_frame = &instance->frames[i];
		jks_array_init(&instance_frame->bone_mats, sizeof(struct mat4f), NULL, &jks_array_memory_fn_GX);
		instance_frame->culled = true;
		instance_frame->uniform_buffer = GFX_BUFFER_INIT();
	}
	jks_array_init(&instance->lights, sizeof(struct gx_m2_light), NULL, &jks_array_memory_fn_GX);
	jks_array_init(&instance->enabled_batches, sizeof(uint16_t), NULL, &jks_array_memory_fn_GX);
	jks_array_init(&instance->bone_calc, sizeof(uint8_t), NULL, &jks_array_memory_fn_GX);
#ifdef WITH_DEBUG_RENDERING
	gx_aabb_init(&instance->gx_aabb, (struct vec4f){1, 1, 1, 1}, 1);
	gx_aabb_init(&instance->gx_caabb, (struct vec4f){0.5, 0.5, 0.5, 1}, 1);
#endif
	if (gx_m2_flag_get(instance->parent, GX_M2_FLAG_LOADED))
		gx_m2_instance_on_parent_loaded(instance);
	return true;
}

struct gx_m2_instance *gx_m2_instance_new(struct gx_m2 *parent)
{
	struct gx_m2_instance *instance = mem_zalloc(MEM_GX, sizeof(*instance));
	if (!instance)
		return NULL;
	refcount_init(&instance->refcount, 1);
	instance->parent = parent;
	gx_m2_ref(parent);
	cache_lock_m2(g_wow->cache);
	if (!instance_init(instance))
	{
		cache_unlock_m2(g_wow->cache);
		gx_m2_free(parent);
		mem_free(MEM_GX, instance);
		return NULL;
	}
	cache_unlock_m2(g_wow->cache);
	return instance;
}

struct gx_m2_instance *gx_m2_instance_new_filename(const char *filename)
{
	struct gx_m2_instance *instance = mem_zalloc(MEM_GX, sizeof(*instance));
	if (!instance)
		return NULL;
	refcount_init(&instance->refcount, 1);
	cache_lock_m2(g_wow->cache);
	if (!cache_ref_unmutexed_m2(g_wow->cache, filename, &instance->parent))
	{
		cache_unlock_m2(g_wow->cache);
		LOG_ERROR("failed to get m2 instance ref: %s", filename);
		mem_free(MEM_GX, instance);
		return NULL;
	}
	if (!instance_init(instance))
	{
		cache_unlock_m2(g_wow->cache);
		gx_m2_free(instance->parent);
		mem_free(MEM_GX, instance);
		return NULL;
	}
	cache_unlock_m2(g_wow->cache);
	return instance;
}

static void instance_delete_task(struct wow_mpq_compound *mpq_compound, void *userdata)
{
	(void)mpq_compound;
	struct gx_m2_instance *instance = userdata;
	gx_m2_particles_delete(instance->gx_particles);
	gx_m2_ribbons_delete(instance->gx_ribbons);
	for (size_t i = 0; i < RENDER_FRAMES_COUNT; ++i)
	{
		struct gx_m2_instance_frame *instance_frame = &instance->frames[i];
		gfx_delete_buffer(g_wow->device, &instance_frame->uniform_buffer);
		jks_array_destroy(&instance_frame->bone_mats);
	}
	jks_array_destroy(&instance->lights);
	jks_array_destroy(&instance->enabled_batches);
	jks_array_destroy(&instance->bone_calc);
#ifdef WITH_DEBUG_RENDERING
	gx_aabb_destroy(&instance->gx_aabb);
	gx_aabb_destroy(&instance->gx_caabb);
#endif
	if (instance->local_lighting)
	{
		gfx_delete_buffer(g_wow->device, &instance->local_lighting->uniform_buffer);
		mem_free(MEM_GX, instance->local_lighting);
	}
	gx_blp_free(instance->skin_extra_texture);
	gx_blp_free(instance->skin_texture);
	gx_blp_free(instance->hair_texture);
	gx_blp_free(instance->monster_textures[0]);
	gx_blp_free(instance->monster_textures[1]);
	gx_blp_free(instance->monster_textures[2]);
	gx_blp_free(instance->object_texture);
	mem_free(MEM_GX, instance);
}

void gx_m2_instance_free(struct gx_m2_instance *instance)
{
	if (!instance)
		return;
	if (refcount_dec(&instance->refcount))
		return;
	{
		cache_lock_m2(g_wow->cache);
		for (size_t i = 0; i < instance->parent->instances.size; ++i)
		{
			struct gx_m2_instance *other = *JKS_ARRAY_GET(&instance->parent->instances, i, struct gx_m2_instance*);
			if (instance != other)
				continue;
			jks_array_erase(&instance->parent->instances, i);
			break;
		}
		cache_unlock_m2(g_wow->cache);
	}
	gx_m2_free(instance->parent);
	loader_push(g_wow->loader, ASYNC_TASK_M2_INSTANCE_DELETE, instance_delete_task, instance);
}

void gx_m2_instance_ref(struct gx_m2_instance *instance)
{
	refcount_inc(&instance->refcount);
}

static void update_local_lighting(struct gx_m2_instance *instance, struct gx_frame *frame)
{
	struct gx_m2_lighting *loc = instance->local_lighting;
	if (!loc)
		return;
	if (!loc->uniform_buffer.handle.u64)
		gfx_create_buffer(g_wow->device, &loc->uniform_buffer, GFX_BUFFER_UNIFORM, NULL, sizeof(struct shader_m2_scene_block), GFX_BUFFER_STREAM);
	struct shader_m2_scene_block data;
	VEC4_CPY(data.light_direction, loc->light_direction);
	VEC4_CPY(data.specular_color, loc->diffuse_color);
	VEC4_CPY(data.diffuse_color, loc->diffuse_color);
	VEC4_CPY(data.ambient_color, loc->ambient_color);
	data.fog_range.y = frame->view_distance * g_wow->map->gx_skybox->float_values[SKYBOX_FLOAT_FOG_END] / 36 / g_wow->map->fog_divisor;
	data.fog_range.x = data.fog_range.y * g_wow->map->gx_skybox->float_values[SKYBOX_FLOAT_FOG_START];
	gfx_set_buffer_data(&loc->uniform_buffer, &data, sizeof(data), 0);
}

static void update_instance_uniform_buffer(struct gx_m2_instance *instance, struct gx_frame *frame)
{
	size_t bones_off = offsetof(struct shader_m2_model_block, bone_mats);
	size_t lights_off = offsetof(struct shader_m2_model_block, lights);
	struct gx_m2_instance_frame *instance_frame = &instance->frames[frame->id];
	if (!instance_frame->uniform_buffer.handle.u64)
		gfx_create_buffer(g_wow->device, &instance_frame->uniform_buffer, GFX_BUFFER_UNIFORM, NULL, bones_off + instance_frame->bone_mats.size * sizeof(struct mat4f), GFX_BUFFER_STREAM);
	update_local_lighting(instance, frame);
	struct shader_m2_model_block data;
	data.v = frame->view_v;
	data.mv = instance_frame->mv;
	data.mvp = instance_frame->mvp;
	data.shadow_v = frame->view_shadow_v;
	data.shadow_mv = instance_frame->shadow_mv;
	data.shadow_mvp = instance_frame->shadow_mvp;
	if (instance->parent->bones_nb)
	{
		if (instance->parent->bones_nb <= sizeof(data.bone_mats) / sizeof(*data.bone_mats))
			gfx_set_buffer_data(&instance_frame->uniform_buffer, instance_frame->bone_mats.data, instance_frame->bone_mats.size * sizeof(struct mat4f), bones_off);
		else
			LOG_ERROR("too much bones (%u, %s)", instance->parent->bones_nb, instance->parent->filename);
	}
	if (gx_m2_instance_flag_get(instance, GX_M2_INSTANCE_FLAG_ENABLE_LIGHTS))
	{
		data.light_count.x = instance->parent->lights_nb;
		if (data.light_count.x > 4)
			data.light_count.x = 4;
		for (int i = 0; i < data.light_count.x; ++i)
		{
			struct wow_m2_light *light = &instance->parent->lights[i];
			struct vec3f ambient_rgb;
			if (!m2_instance_get_track_value_vec3f(instance, &light->ambient_color, &ambient_rgb))
				VEC3_SETV(ambient_rgb, 0);
			float ambient_alpha;
			if (!m2_instance_get_track_value_float(instance, &light->ambient_intensity, &ambient_alpha))
				ambient_alpha = 0;
			VEC3_CPY(data.lights[i].ambient, ambient_rgb);
			data.lights[i].ambient.w = ambient_alpha;
			struct vec3f diffuse_rgb;
			if (!m2_instance_get_track_value_vec3f(instance, &light->diffuse_color, &diffuse_rgb))
				VEC3_SETV(diffuse_rgb, 0);
			float diffuse_alpha;
			if (!m2_instance_get_track_value_float(instance, &light->diffuse_intensity, &diffuse_alpha))
				diffuse_alpha = 0;
			VEC3_CPY(data.lights[i].diffuse, diffuse_rgb);
			data.lights[i].diffuse.w = diffuse_alpha;
			if (!m2_instance_get_track_value_float(instance, &light->attenuation_start, &data.lights[i].attenuation.x))
				data.lights[i].attenuation.x = 0;
			if (!m2_instance_get_track_value_float(instance, &light->attenuation_end, &data.lights[i].attenuation.y))
				data.lights[i].attenuation.y = 0;
			VEC3_CPY(data.lights[i].position, light->position);
			data.lights[i].position.w = light->type;
			if (light->type && light->bone != -1)
			{
				struct mat4f *bone_mat = JKS_ARRAY_GET(&instance_frame->bone_mats, light->bone, struct mat4f);
				struct vec4f tmp;
				MAT4_VEC4_MUL(tmp, *bone_mat, data.lights[i].position);
				data.lights[i].position = tmp;
			}
			uint8_t enabled;
			if (!m2_instance_get_track_value_uint8(instance, &light->visibility, &enabled))
				enabled = 0;
			data.lights[i].data.x = enabled;
			data.lights[i].data.y = light->type;
#if 0
			LOG_INFO("light idx: %d", (int)i);
			LOG_INFO("light position: {%f, %f, %f, %f}", data.lights[i].position.x, data.lights[i].position.y, data.lights[i].position.z, data.lights[i].position.w);
			LOG_INFO("light attenuation: {x: %f, y: %f}", data.lights[i].attenuation.x, data.lights[i].attenuation.y);
			LOG_INFO("light enable: %d", (int)enabled);
			LOG_INFO("light bone: %d / %d", light->bone, (int)instance->parent->bones_nb);
			LOG_INFO("light type: %d", light->type);
			LOG_INFO("light diffuse: {%f, %f, %f, %f}", data.lights[i].diffuse.x, data.lights[i].diffuse.y, data.lights[i].diffuse.z, data.lights[i].diffuse.w);
			LOG_INFO("light ambient: {%f, %f, %f, %f}", data.lights[i].ambient.x, data.lights[i].ambient.y, data.lights[i].ambient.z, data.lights[i].ambient.w);
			LOG_INFO(" ");
#endif
		}
	}
	else
	{
		data.light_count.x = 0;
	}
	gfx_set_buffer_data(&instance_frame->uniform_buffer, &data, lights_off + data.light_count.x * sizeof(struct shader_m2_light_block), 0);
}

static void calc_bone_billboard(struct gx_m2_instance *instance, struct gx_frame *frame, struct wow_m2_bone *bone, struct mat4f *mat)
{
	if (bone->flags & WOW_M2_BONE_SPHERICAL_BILLBOARD)
	{
		struct mat4f tmp;
		struct mat4f t;
		MAT4_IDENTITY(t);
		VEC3_CPY(t.x, instance->m_inv.x);
		VEC3_CPY(t.y, instance->m_inv.y);
		VEC3_CPY(t.z, instance->m_inv.z);
		MAT4_ROTATEZ(float, tmp, t, -frame->cull_rot.z);
		MAT4_ROTATEY(float, t, tmp, -frame->cull_rot.y);
		MAT4_ROTATEX(float, tmp, t, -frame->cull_rot.x);
		MAT4_ROTATEY(float, t, tmp, -M_PI / 2.0);
		tmp = t;
		float n;
		n = instance->scale * VEC3_NORM(mat->x);
		VEC3_MULV(tmp.x, tmp.x, n);
		n = instance->scale * VEC3_NORM(mat->y);
		VEC3_MULV(tmp.y, tmp.y, n);
		n = instance->scale * VEC3_NORM(mat->z);
		VEC3_MULV(tmp.z, tmp.z, n);
		VEC3_CPY(mat->x, tmp.x);
		VEC3_CPY(mat->y, tmp.y);
		VEC3_CPY(mat->z, tmp.z);
	}
	else if (bone->flags & WOW_M2_BONE_CYLINDRICAL_X_BILLBOARD)
	{
		struct mat4f tmp;
		struct mat4f t;
		MAT4_IDENTITY(t);
		VEC3_CPY(t.x, instance->m_inv.x);
		VEC3_CPY(t.y, instance->m_inv.y);
		VEC3_CPY(t.z, instance->m_inv.z);
		MAT4_ROTATEX(float, tmp, t, -frame->cull_rot.x);
		float n;
		n = instance->scale * VEC3_NORM(mat->y);
		VEC3_MULV(tmp.y, tmp.y, n);
		n = instance->scale * VEC3_NORM(mat->z);
		VEC3_MULV(tmp.z, tmp.z, n);
		VEC3_CPY(mat->y, tmp.y);
		VEC3_CPY(mat->z, tmp.z);
	}
	else if (bone->flags & WOW_M2_BONE_CYLINDRICAL_Y_BILLBOARD)
	{
		struct mat4f tmp;
		struct mat4f t;
		MAT4_IDENTITY(t);
		VEC3_CPY(t.x, instance->m_inv.x);
		VEC3_CPY(t.y, instance->m_inv.y);
		VEC3_CPY(t.z, instance->m_inv.z);
		MAT4_ROTATEZ(float, tmp, t, -frame->cull_rot.z);
		float n;
		n = instance->scale * VEC3_NORM(mat->x);
		VEC3_MULV(tmp.x, tmp.x, n);
		n = instance->scale * VEC3_NORM(mat->y);
		VEC3_MULV(tmp.y, tmp.y, n);
		VEC3_CPY(mat->x, tmp.x);
		VEC3_CPY(mat->y, tmp.y);
	}
	else if (bone->flags & WOW_M2_BONE_CYLINDRICAL_Z_BILLBOARD)
	{
		struct mat4f tmp;
		struct mat4f t;
		MAT4_IDENTITY(t);
		VEC3_CPY(t.x, instance->m_inv.x);
		VEC3_CPY(t.y, instance->m_inv.y);
		VEC3_CPY(t.z, instance->m_inv.z);
		MAT4_ROTATEY(float, tmp, t, -frame->cull_rot.y - M_PI / 2.0);
		float n;
		n = instance->scale * VEC3_NORM(mat->x);
		VEC3_MULV(tmp.x, tmp.x, n);
		n = instance->scale * VEC3_NORM(mat->z);
		VEC3_MULV(tmp.z, tmp.z, n);
		VEC3_CPY(mat->x, tmp.x);
		VEC3_CPY(mat->z, tmp.z);
	}
}

static void calc_bone_sequence(struct gx_m2_instance *instance, struct gx_frame *frame, struct mat4f *mat, struct wow_m2_bone *bone)
{
	{
		struct mat4f tmp_mat;
		MAT4_TRANSLATE(tmp_mat, *mat, bone->pivot);
		*mat = tmp_mat;
	}
	{
		struct vec3f v;
		if (m2_instance_get_track_value_vec3f(instance, &bone->translation, &v))
		{
			struct vec3f tmp = {v.x, v.z, -v.y};
			struct mat4f tmp_mat;
			MAT4_TRANSLATE(tmp_mat, *mat, tmp);
			*mat = tmp_mat;
		}
	}
	if (bone->flags & WOW_M2_BONE_BILLBOARD)
	{
		calc_bone_billboard(instance, frame, bone, mat);
	}
	else
	{
		struct vec4f v;
		if (m2_instance_get_track_value_quat16(instance, &bone->rotation, &v))
		{
			VEC4_NORMALIZE(float, v, v);
			struct vec4f tmp = {v.x, v.z, -v.y, v.w};
			struct mat4f quat;
			QUATERNION_TO_MAT4(float, quat, tmp);
			struct mat4f tmp_mat;
			MAT4_MUL(tmp_mat, *mat, quat);
			*mat = tmp_mat;
		}
	}
	{
		struct vec3f v;
		if (m2_instance_get_track_value_vec3f(instance, &bone->scale, &v))
		{
			struct vec3f tmp = {v.x, v.z, v.y};
			MAT4_SCALE(*mat, *mat, tmp);
		}
	}
	{
		struct vec3f tmp = {-bone->pivot.x, -bone->pivot.y, -bone->pivot.z};
		struct mat4f tmp_mat;
		MAT4_TRANSLATE(tmp_mat, *mat, tmp);
		*mat = tmp_mat;
	}
}

static void update_bone(struct gx_m2_instance *instance, struct gx_frame *frame, uint16_t bone_id, const struct mat4f *init_mat)
{
	if (bone_id > instance->bone_calc.size * 8)
		return;
	if ((*JKS_ARRAY_GET(&instance->bone_calc, bone_id / 8, uint8_t)) & (1 << (bone_id % 8)))
		return;
	struct gx_m2_instance_frame *instance_frame = &instance->frames[frame->id];
	struct wow_m2_bone *bone = &instance->parent->bones[bone_id];
	struct mat4f mat;
	struct mat4f *matp;
	if (bone->flags & (WOW_M2_BONE_TRANSFORMED | WOW_M2_BONE_BILLBOARD))
		matp = &mat;
	else
		matp = JKS_ARRAY_GET(&instance_frame->bone_mats, bone_id, struct mat4f);
	if (bone->parent_bone != -1)
	{
		update_bone(instance, frame, bone->parent_bone, NULL);
		*matp = *JKS_ARRAY_GET(&instance_frame->bone_mats, bone->parent_bone, struct mat4f);
		if (init_mat)
		{
			struct mat4f tmp;
			MAT4_MUL(tmp, *matp, *init_mat);
			*matp = tmp;
		}
	}
	else if (init_mat)
	{
		*matp = *init_mat;
	}
	else
	{
		MAT4_IDENTITY(*matp);
	}
	*JKS_ARRAY_GET(&instance->bone_calc, bone_id / 8, uint8_t) |= 1 << (bone_id % 8);
	if (!(bone->flags & (WOW_M2_BONE_TRANSFORMED | WOW_M2_BONE_BILLBOARD)))
		return;
	if (instance->sequence && instance->sequence->end > instance->sequence->start)
		calc_bone_sequence(instance, frame, &mat, bone);
	*JKS_ARRAY_GET(&instance_frame->bone_mats, bone_id, struct mat4f) = mat;
	*JKS_ARRAY_GET(&instance->bone_calc, bone_id / 8, uint8_t) |= 1 << (bone_id % 8);
}

void gx_m2_instance_clear_bones(struct gx_m2_instance *instance)
{
	gx_m2_instance_flag_clear(instance, GX_M2_INSTANCE_FLAG_BONES_UPDATED);
	memset(instance->bone_calc.data, 0, instance->bone_calc.size);
}

void gx_m2_instance_update_bones(struct gx_m2_instance *instance, struct gx_frame *frame)
{
	if (gx_m2_instance_flag_set(instance, GX_M2_INSTANCE_FLAG_BONES_UPDATED))
		return;
	if (!instance->bone_calc.size)
		return;
	for (size_t i = 0; i < instance->parent->bones_nb; ++i)
		update_bone(instance, frame, i, NULL);
}

void gx_m2_instance_update_bone(struct gx_m2_instance *instance, struct gx_frame *frame, uint16_t bone, const struct mat4f *mat)
{
	update_bone(instance, frame, bone, mat);
}

void gx_m2_instance_clear_update(struct gx_m2_instance *instance, struct gx_frame *frame)
{
	struct gx_m2_instance_frame *instance_frame = &instance->frames[frame->id];
	instance_frame->culled = true;
	memset(instance->bone_calc.data, 0, instance->bone_calc.size);
}

static void update_matrix(struct gx_m2_instance *instance, struct gx_frame *frame, struct gx_m2_render_params *params)
{
	struct gx_m2_instance_frame *instance_frame = &instance->frames[frame->id];
	MAT4_MUL(instance_frame->mv, params->v, instance->m);
	MAT4_MUL(instance_frame->mvp, params->p, instance_frame->mv);
	MAT4_MUL(instance_frame->shadow_mv, params->shadow_v, instance->m);
	MAT4_MUL(instance_frame->shadow_mvp, params->shadow_p, instance_frame->shadow_mv);
}

static void update_sequences_times(struct gx_m2_instance *instance, struct gx_frame *frame)
{
	if (instance->sequence && instance->sequence->end > instance->sequence->start)
		instance->sequence_time = instance->sequence->start + (uint32_t)((frame->time - instance->sequence_started) * instance->sequence_speed / 1000000);
	if (instance->prev_sequence && instance->prev_sequence->end > instance->prev_sequence->start)
		instance->prev_sequence_time = instance->prev_sequence->start + (uint32_t)((frame->time - instance->prev_sequence_started) * instance->sequence_speed / 1000000);
}

void gx_m2_instance_force_update(struct gx_m2_instance *instance, struct gx_frame *frame, struct gx_m2_render_params *params)
{
	struct gx_m2_instance_frame *instance_frame = &instance->frames[frame->id];
	if (!gx_m2_flag_get(instance->parent, GX_M2_FLAG_LOADED))
	{
		instance_frame->culled = true;
		return;
	}
	update_matrix(instance, frame, params);
	gx_m2_instance_update_bones(instance, frame);
	update_sequences_times(instance, frame);
	instance_frame->culled = false;
}

void gx_m2_instance_render(struct gx_m2_instance *instance, struct gx_frame *frame, bool transparent, struct gx_m2_render_params *params)
{
	if (instance->frames[frame->id].culled)
		return;
	gx_m2_render_instance(instance->parent, frame, instance, transparent, params);
}

void gx_m2_instance_render_particles(struct gx_m2_instance *instance, struct gx_frame *frame, struct gx_m2_render_params *params)
{
	if (instance->frames[frame->id].culled) /* XXX still create particles ? */
		return;
	gx_m2_particles_render(instance->gx_particles, frame, params);
}

void gx_m2_instance_render_ribbons(struct gx_m2_instance *instance, struct gx_frame *frame, struct gx_m2_render_params *params)
{
	if (instance->frames[frame->id].culled) /* XXX still create ribbons ? */
		return;
	gx_m2_ribbons_render(instance->gx_ribbons, frame, params);
}

void gx_m2_instance_add_to_render(struct gx_m2_instance *instance, struct gx_frame *frame, bool bypass_frustum, struct gx_m2_render_params *params)
{
	if (gx_m2_instance_flag_set(instance, GX_M2_INSTANCE_FLAG_IN_RENDER_LIST))
		return;
	gx_m2_instance_ref(instance);
	gx_frame_add_m2_backref(frame, instance);
	struct gx_m2_instance_frame *instance_frame = &instance->frames[frame->id];
	if (!gx_m2_flag_get(instance->parent, GX_M2_FLAG_LOADED))
	{
		instance_frame->shadow_culled = true;
		instance_frame->culled = true;
		return;
	}
	struct vec3f tmp;
	VEC3_SUB(tmp, frame->cull_pos, instance->pos);
	instance_frame->distance_to_camera = VEC3_NORM(tmp);
	float max_distance = frame->view_distance * instance->parent->render_distance / 8.0f * instance->scale;
	if (instance->render_distance_max != -1 && instance->render_distance_max < max_distance)
		max_distance = instance->render_distance_max;
	if (instance_frame->distance_to_camera > max_distance)
	{
		instance_frame->shadow_culled = true;
		instance_frame->culled = true;
		return;
	}
	instance_frame->culled = !bypass_frustum && !frustum_check_fast(&frame->frustum, &instance->aabb);
	instance_frame->shadow_culled = !(g_wow->gx->opt & GX_OPT_DYN_SHADOW)
	                             || !(instance->flags & GX_M2_INSTANCE_FLAG_DYN_SHADOW)
	                             || !frustum_check_fast(&frame->shadow_frustum, &instance->aabb);
	if (instance_frame->culled && instance_frame->shadow_culled)
		return;
	update_matrix(instance, frame, params);
	gx_m2_instance_update_bones(instance, frame);
	update_sequences_times(instance, frame);
#ifdef WITH_DEBUG_RENDERING
	if (g_wow->gx->opt & GX_OPT_M2_AABB)
	{
		gx_aabb_add_to_render(&instance->gx_aabb, frame, &frame->view_vp);
		gx_aabb_add_to_render(&instance->gx_caabb, frame, &frame->view_vp);
	}
#endif
	if (!instance_frame->culled)
	{
		if (!gx_m2_flag_set(instance->parent, GX_M2_FLAG_IN_RENDER_LIST))
		{
			gx_frame_add_m2(frame, instance->parent);
			if (gx_m2_flag_get(instance->parent, GX_M2_FLAG_HAS_OPAQUE_BATCHES))
				gx_frame_add_m2_opaque(frame, instance->parent);
		}
		if (gx_m2_flag_get(instance->parent, GX_M2_FLAG_HAS_TRANS_BATCHES))
			gx_frame_add_m2_transparent(frame, instance);
		if (instance->gx_particles)
		{
			gx_m2_particles_update(instance->gx_particles, frame, params);
			gx_frame_add_m2_particles(frame, instance);
		}
		if (instance->gx_ribbons)
		{
			gx_m2_ribbons_update(instance->gx_ribbons, frame);
			gx_frame_add_m2_ribbons(frame, instance);
		}
		struct gx_m2_frame *m2_frame = &instance->parent->frames[frame->id];
		pthread_mutex_lock(&m2_frame->mutex);
		if (!jks_array_push_back(&m2_frame->to_render, &instance))
			LOG_ERROR("failed to add m2 instance to parent render list");
		pthread_mutex_unlock(&m2_frame->mutex);
	}
	if (!instance_frame->shadow_culled)
	{
		if (!gx_m2_flag_set(instance->parent, GX_M2_FLAG_IN_SHADOW_RENDER_LIST))
			gx_frame_add_m2_shadow(frame, instance->parent);
		struct gx_m2_frame *m2_frame = &instance->parent->frames[frame->id];
		pthread_mutex_lock(&m2_frame->mutex);
		if (!jks_array_push_back(&m2_frame->shadow_to_render, &instance))
			LOG_ERROR("failed to add m2 instance to parent shadow render list");
		pthread_mutex_unlock(&m2_frame->mutex);
	}
}

void gx_m2_instance_set_mat(struct gx_m2_instance *instance, struct mat4f *mat)
{
	instance->m = *mat;
	MAT4_INVERSE(float, instance->m_inv, *mat);
	if (gx_m2_flag_get(instance->parent, GX_M2_FLAG_LOADED))
		gx_m2_instance_update_aabb(instance);
}

void gx_m2_instance_update_aabb(struct gx_m2_instance *instance)
{
	if (!gx_m2_flag_get(instance->parent, GX_M2_FLAG_LOADED))
		return;
	instance->aabb = instance->parent->aabb;
	aabb_transform(&instance->aabb, &instance->m);
#ifdef WITH_DEBUG_RENDERING
	gx_aabb_set_aabb(&instance->gx_aabb, &instance->aabb);
#endif
	instance->caabb = instance->parent->caabb;
	aabb_transform(&instance->caabb, &instance->m);
#ifdef WITH_DEBUG_RENDERING
	gx_aabb_set_aabb(&instance->gx_caabb, &instance->caabb);
#endif
}

static void resolve_sequence(struct gx_m2_instance *instance)
{
	uint32_t seq_id;
	if (instance->sequence_id < instance->parent->playable_animations_nb)
		seq_id = instance->parent->playable_animations[instance->sequence_id].id;
	else
		seq_id = 0;
	size_t i = seq_id % instance->parent->sequence_lookups_nb;
	uint16_t lookup;
	for (size_t stride = 1;; ++stride)
	{
		lookup = instance->parent->sequence_lookups[i];
		if (lookup == (uint16_t)-1)
		{
			LOG_WARN("sequence not found: %u", seq_id);
			lookup = 0;
			break;
		}
		if (instance->parent->sequences[lookup].id == seq_id)
			break;
		i = (i + stride * stride) % instance->parent->sequence_lookups_nb;
	}
	instance->sequence = &instance->parent->sequences[lookup];
}

void gx_m2_instance_on_parent_loaded(struct gx_m2_instance *instance)
{
	gx_m2_instance_update_aabb(instance);
	for (size_t i = 0; i < RENDER_FRAMES_COUNT; ++i)
	{
		if (!jks_array_resize(&instance->frames[i].bone_mats, instance->parent->bones_nb))
		{
			LOG_ERROR("failed to alloc bones matrixes array");
			return;
		}
	}
	if (!jks_array_resize(&instance->bone_calc, instance->parent->bones_nb))
	{
		LOG_ERROR("failed to alloc bones calc array");
		return;
	}
	memset(instance->bone_calc.data, 0, instance->bone_calc.size);
	if (instance->parent->particles_nb)
	{
		instance->gx_particles = gx_m2_particles_new(instance);
		if (!instance->gx_particles)
			LOG_ERROR("failed to load particles renderer");
	}
	if (instance->parent->ribbons_nb)
	{
		instance->gx_ribbons = gx_m2_ribbons_new(instance);
		if (!instance->gx_ribbons)
			LOG_ERROR("failed to load ribbons renderer");
	}
	resolve_sequence(instance);
}

void gx_m2_instance_set_sequence(struct gx_m2_instance *instance, uint32_t sequence)
{
	if (sequence == instance->sequence_id)
		return;
	instance->prev_sequence = instance->sequence;
	instance->prev_sequence_started = instance->sequence_started;
	instance->prev_sequence_id = instance->sequence_id;
	instance->prev_sequence_time = instance->sequence_time;
	instance->sequence_id = sequence;
	if (gx_m2_flag_get(instance->parent, GX_M2_FLAG_LOADED))
		resolve_sequence(instance);
	instance->sequence_started = g_wow->frametime;
}

void gx_m2_instance_set_skin_extra_texture(struct gx_m2_instance *instance, struct gx_blp *texture)
{
	gx_blp_free(instance->skin_extra_texture);
	if (texture)
		gx_blp_ref(texture);
	instance->skin_extra_texture = texture;
}

void gx_m2_instance_set_skin_texture(struct gx_m2_instance *instance, struct gx_blp *texture)
{
	gx_blp_free(instance->skin_texture);
	if (texture)
		gx_blp_ref(texture);
	instance->skin_texture = texture;
}

void gx_m2_instance_set_hair_texture(struct gx_m2_instance *instance, struct gx_blp *texture)
{
	gx_blp_free(instance->hair_texture);
	if (texture)
		gx_blp_ref(texture);
	instance->hair_texture = texture;
}

void gx_m2_instance_set_monster_texture(struct gx_m2_instance *instance, int id, struct gx_blp *texture)
{
	gx_blp_free(instance->monster_textures[id]);
	if (texture)
		gx_blp_ref(texture);
	instance->monster_textures[id] = texture;
}

void gx_m2_instance_set_object_texture(struct gx_m2_instance *instance, struct gx_blp *texture)
{
	gx_blp_free(instance->object_texture);
	if (texture)
		gx_blp_ref(texture);
	instance->object_texture = texture;
}

void gx_m2_instance_enable_batch(struct gx_m2_instance *instance, uint16_t batch)
{
	if (!jks_array_push_back(&instance->enabled_batches, &batch))
		LOG_ERROR("failed to add enabled batch");
}

void gx_m2_instance_disable_batch(struct gx_m2_instance *instance, uint16_t batch)
{
	for (size_t i = 0; i < instance->enabled_batches.size; ++i)
	{
		if (*JKS_ARRAY_GET(&instance->enabled_batches, i, uint16_t) == batch)
		{
			jks_array_erase(&instance->enabled_batches, i);
			return;
		}
	}
}

void gx_m2_instance_enable_batches(struct gx_m2_instance *instance, uint16_t start, uint16_t end)
{
	if (end <= start)
		return;
	uint16_t diff = end - start;
	uint16_t *data = jks_array_grow(&instance->enabled_batches, diff);
	if (data)
	{
		for (uint16_t i = 0; i < diff; ++i)
			data[i] = start + i;
	}
	else
	{
		LOG_ERROR("failed to add enabled batches");
	}
}

void gx_m2_instance_disable_batches(struct gx_m2_instance *instance, uint16_t start, uint16_t end)
{
	for (size_t i = 0; i < instance->enabled_batches.size; ++i)
	{
		uint16_t batch = *JKS_ARRAY_GET(&instance->enabled_batches, i, uint16_t);
		if (batch >= start && batch <= end)
		{
			jks_array_erase(&instance->enabled_batches, i);
			i--;
		}
	}
}

void gx_m2_instance_clear_batches(struct gx_m2_instance *instance)
{
	jks_array_resize(&instance->enabled_batches, 0);
}

struct gx_m2_ground_batch *gx_m2_ground_batch_new(struct gx_m2 *m2, bool light)
{
	struct gx_m2_ground_batch *batch = mem_zalloc(MEM_GX, sizeof(*batch));
	if (!batch)
		return NULL;
	gx_m2_ref(m2);
	refcount_init(&batch->refcount, 1);
	batch->m2 = m2;
	batch->light = light;
	for (size_t i = 0; i < RENDER_FRAMES_COUNT; ++i)
	{
		struct gx_m2_ground_batch_frame *frame = &batch->frames[i];
		frame->attributes_state = GFX_ATTRIBUTES_STATE_INIT();
		frame->instanced_buffer = GFX_BUFFER_INIT();
		frame->uniform_buffer = GFX_BUFFER_INIT();
		jks_array_init(&frame->instances, sizeof(struct mat4f), NULL, &jks_array_memory_fn_GX);
	}
	return batch;
}

void gx_m2_ground_batch_free(struct gx_m2_ground_batch *batch)
{
	if (!batch)
		return;
	if (refcount_dec(&batch->refcount))
		return;
	for (size_t i = 0; i < RENDER_FRAMES_COUNT; ++i)
	{
		struct gx_m2_ground_batch_frame *frame = &batch->frames[i];
		gfx_delete_attributes_state(g_wow->device, &frame->attributes_state);
		gfx_delete_buffer(g_wow->device, &frame->instanced_buffer);
		gfx_delete_buffer(g_wow->device, &frame->uniform_buffer);
		jks_array_destroy(&frame->instances);
	}
	gx_m2_free(batch->m2);
	mem_free(MEM_GX, batch);
}

void gx_m2_ground_batch_ref(struct gx_m2_ground_batch *batch)
{
	refcount_inc(&batch->refcount);
}

void gx_m2_ground_batch_cull_start(struct gx_m2_ground_batch *batch, struct gx_frame *frame)
{
	if (!gx_m2_flag_get(batch->m2, GX_M2_FLAG_INITIALIZED))
		return;
	struct gx_m2_ground_batch_frame *batch_frame = &batch->frames[frame->id];
	jks_array_resize(&batch_frame->instances, 0);
}

void gx_m2_ground_batch_cull(struct gx_m2_ground_batch *batch, struct gx_frame *frame, const struct gx_m2_ground *entries, size_t entries_count)
{
	if (!gx_m2_flag_get(batch->m2, GX_M2_FLAG_INITIALIZED))
		return;
	struct gx_m2_ground_batch_frame *batch_frame = &batch->frames[frame->id];
	for (size_t i = 0; i < entries_count; ++i)
	{
		const struct gx_m2_ground *entry = &entries[i];
		struct vec3f delta;
		VEC3_SUB(delta, entry->position, frame->cull_pos);
		float distance = VEC3_NORM(delta);
		if (distance > entry->render_distance)
			continue;
		struct mat4f *mat = jks_array_grow(&batch_frame->instances, 1);
		assert(mat);
		MAT4_MUL(*mat, frame->view_v, entry->mat);
	}
}

void gx_m2_ground_batch_cull_end(struct gx_m2_ground_batch *batch, struct gx_frame *frame)
{
	if (!gx_m2_flag_get(batch->m2, GX_M2_FLAG_INITIALIZED))
		return;
	struct gx_m2_ground_batch_frame *batch_frame = &batch->frames[frame->id];
	if (batch_frame->instances.size)
	{
		gx_m2_ground_batch_ref(batch);
		gx_frame_add_m2_ground(frame, batch);
	}
}

static void gx_m2_ground_batch_initialize(struct gx_m2_ground_batch *batch)
{
	for (size_t i = 0; i < RENDER_FRAMES_COUNT; ++i)
		gfx_create_buffer(g_wow->device, &batch->frames[i].uniform_buffer, GFX_BUFFER_UNIFORM, NULL, sizeof(struct shader_m2_ground_model_block), GFX_BUFFER_STREAM);
	batch->initialized = true;
}

void gx_m2_ground_batch_render(struct gx_m2_ground_batch *batch, struct gx_frame *frame)
{
	struct gx_m2_ground_batch_frame *batch_frame = &batch->frames[frame->id];
	if (!batch->initialized)
		gx_m2_ground_batch_initialize(batch);
	struct shader_m2_ground_model_block model_block;
	model_block.v = frame->view_v;
	model_block.p = frame->view_p;
	gfx_set_buffer_data(&batch_frame->uniform_buffer, &model_block, sizeof(struct shader_m2_ground_model_block), 0);
	gfx_bind_constant(g_wow->device, 1, &batch_frame->uniform_buffer, sizeof(struct shader_m2_ground_model_block), 0);
	if (batch_frame->instances.size > batch_frame->buffer_size
	 || batch_frame->instances.size < batch_frame->buffer_size / 2)
	{
		gfx_delete_buffer(g_wow->device, &batch_frame->instanced_buffer);
		gfx_delete_attributes_state(g_wow->device, &batch_frame->attributes_state);
		batch_frame->instanced_buffer = GFX_BUFFER_INIT();
		batch_frame->attributes_state = GFX_ATTRIBUTES_STATE_INIT();
		batch_frame->buffer_size = npot32(batch_frame->instances.size + 1);
		gfx_create_buffer(g_wow->device, &batch_frame->instanced_buffer, GFX_BUFFER_VERTEXES, NULL, sizeof(struct shader_m2_ground_input) * batch_frame->buffer_size, GFX_BUFFER_STREAM);
		const struct gfx_attribute_bind binds[] =
		{
			{&batch->m2->vertexes_buffer},
			{&batch_frame->instanced_buffer},
		};
		gfx_create_attributes_state(g_wow->device, &batch_frame->attributes_state, binds, sizeof(binds) / sizeof(*binds), &batch->m2->indices_buffer, GFX_INDEX_UINT16);
	}
	gfx_set_buffer_data(&batch_frame->instanced_buffer, batch_frame->instances.data, sizeof(struct shader_m2_ground_input) * batch_frame->instances.size, 0);
	gfx_bind_attributes_state(g_wow->device, &batch_frame->attributes_state, &g_wow->gx->m2_ground_input_layout);
	if (batch->light)
	{
		if (frame->m2_lighting_type != GX_M2_LIGHTING_GROUND_LIGHT)
		{
			frame->m2_lighting_type = GX_M2_LIGHTING_GROUND_LIGHT;
			gfx_bind_constant(g_wow->device, 2, &frame->m2_ground_light_uniform_buffer, sizeof(struct shader_m2_scene_block), 0);
		}
	}
	else
	{
		if (frame->m2_lighting_type != GX_M2_LIGHTING_GROUND_SHADOW)
		{
			frame->m2_lighting_type = GX_M2_LIGHTING_GROUND_SHADOW;
			gfx_bind_constant(g_wow->device, 2, &frame->m2_ground_shadow_uniform_buffer, sizeof(struct shader_m2_scene_block), 0);
		}
	}
	struct gx_m2_profile *m2_profile = JKS_ARRAY_GET(&batch->m2->profiles, 0, struct gx_m2_profile);
	struct gx_m2_batch *m2_batch = JKS_ARRAY_GET(&m2_profile->batches, 0, struct gx_m2_batch);
	if (!gx_m2_batch_prepare_draw(m2_batch, frame, NULL, false))
		return;
	gfx_bind_pipeline_state(g_wow->device, &g_wow->gx->m2_ground_pipeline_state);
	gx_m2_batch_render_instanced(m2_batch, batch_frame->instances.size);
}
