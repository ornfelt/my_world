#include "m2.h"

#include "common.h"
#include "mpq.h"

#include <stdlib.h>
#include <string.h>

struct wow_m2_track_base_int
{
	/* 0x00 */ uint16_t interpolation_type;
	/* 0x02 */ int16_t global_sequence;
	/* 0x04 */ struct wow_m2_array interpolation_ranges;
	/* 0x0C */ struct wow_m2_array timestamps;
};

struct wow_m2_track_int
{
	/* 0x00 */ struct wow_m2_track_base_int base;
	/* 0x14 */ struct wow_m2_array values;
};

struct wow_m2_camera_int
{
	/* 0x00 */ uint32_t type;
	/* 0x04 */ float fov;
	/* 0x08 */ float far_clip;
	/* 0x0C */ float near_clip;
	/* 0x10 */ struct wow_m2_track_int position;
	/* 0x2C */ struct wow_vec3f position_base;
	/* 0x38 */ struct wow_m2_track_int target_position;
	/* 0x54 */ struct wow_vec3f target_position_base;
	/* 0x60 */ struct wow_m2_track_int roll;
};

struct wow_m2_event_int
{
	/* 0x00 */ uint32_t identifier;
	/* 0x04 */ uint32_t data;
	/* 0x08 */ uint32_t bone;
	/* 0x0C */ struct wow_vec3f position;
	/* 0x18 */ struct wow_m2_track_base_int enabled;
};

struct wow_m2_skin_profile_int
{
	/* 0x00 */ struct wow_m2_array vertexes;  /* uint16_t */
	/* 0x08 */ struct wow_m2_array indices;   /* uint16_t */
	/* 0x10 */ struct wow_m2_array bones;     /* uint32_t */
	/* 0x18 */ struct wow_m2_array sections;  /* struct wow_m2_skin_section */
	/* 0x20 */ struct wow_m2_array batches;   /* struct wow_m2_batch */
	/* 0x28 */ uint32_t bone_count_max;
};

struct wow_m2_ribbon_int
{
	/* 0x00 */ uint32_t ribbon_id;
	/* 0x04 */ uint32_t bone_index;
	/* 0x08 */ struct wow_vec3f position;
	/* 0x14 */ struct wow_m2_array texture_indices;   /* uint16_t */
	/* 0x1C */ struct wow_m2_array material_indices;  /* uint16_t */
	/* 0x24 */ struct wow_m2_track_int color;         /* struct wow_vec3f */
	/* 0x40 */ struct wow_m2_track_int alpha;         /* int16_t */
	/* 0x5C */ struct wow_m2_track_int height_above;  /* float */
	/* 0x78 */ struct wow_m2_track_int height_below;  /* float */
	/* 0x94 */ float edges_per_second;
	/* 0x98 */ float edge_lifetime;
	/* 0x9C */ float gravity;
	/* 0xA0 */ uint16_t texture_rows;
	/* 0xA2 */ uint16_t texture_cols;
	/* 0xA4 */ struct wow_m2_track_int tex_slot;      /* uint16_t */
	/* 0xC0 */ struct wow_m2_track_int visibility;    /* uint8_t */
};

struct wow_m2_bone_int
{
	/* 0x00 */ int32_t key_bone_id;
	/* 0x04 */ uint32_t flags;
	/* 0x08 */ int16_t parent_bone;
	/* 0x0A */ uint16_t submesh_id;
	/* 0x0C */ uint32_t bone_name_crc;
	/* 0x10 */ struct wow_m2_track_int translation; /* struct wow_vec3f */
	/* 0x2C */ struct wow_m2_track_int rotation;    /* struct wow_quats */
	/* 0x48 */ struct wow_m2_track_int scale;       /* struct wow_vec3f */
	/* 0x64 */ struct wow_vec3f pivot;
};

struct wow_m2_texture_weight_int
{
	/* 0x0 */ struct wow_m2_track_int weight; /* int16_t */
};

struct wow_m2_color_int
{
	/* 0x00 */ struct wow_m2_track_int color; /* struct wow_vec3f */
	/* 0x1C */ struct wow_m2_track_int alpha; /* int16_t */
};

struct wow_m2_light_int
{
	/* 0x00 */ uint16_t type;
	/* 0x02 */ int16_t bone;
	/* 0x04 */ struct wow_vec3f position;
	/* 0x10 */ struct wow_m2_track_int ambient_color;     /* struct wow_vec3f */
	/* 0x2C */ struct wow_m2_track_int ambient_intensity; /* float */
	/* 0x48 */ struct wow_m2_track_int diffuse_color;     /* struct wow_vec3f */
	/* 0x64 */ struct wow_m2_track_int diffuse_intensity; /* float */
	/* 0x80 */ struct wow_m2_track_int attenuation_start; /* float */
	/* 0x9C */ struct wow_m2_track_int attenuation_end;   /* float */
	/* 0xB8 */ struct wow_m2_track_int visibility;        /* uint8_t */
};

struct wow_m2_texture_int
{
	/* 0x0 */ uint32_t type;
	/* 0x4 */ uint32_t flags;
	/* 0x8 */ struct wow_m2_array filename;
};

struct wow_m2_texture_transform_int
{
	/* 0x00 */ struct wow_m2_track_int translation; /* struct wow_vec3f */
	/* 0x1C */ struct wow_m2_track_int rotation;    /* struct wow_quatf */
	/* 0x38 */ struct wow_m2_track_int scaling;     /* struct wow_vec3f */
};

struct wow_m2_attachment_int
{
	/* 0x00 */ uint32_t id;
	/* 0x04 */ uint16_t bone;
	/* 0x06 */ uint16_t unknown;
	/* 0x08 */ struct wow_vec3f position;
	/* 0x04 */ struct wow_m2_track_int animate_attached; /* uint8_t */
};

struct wow_m2_particle_int
{
	/* 0x000 */ uint32_t id;
	/* 0x004 */ uint32_t flags;
	/* 0x008 */ struct wow_vec3f position;
	/* 0x014 */ uint16_t bone;
	/* 0x016 */ uint16_t texture;
	/* 0x018 */ struct wow_m2_array geometry_model_filename;  /* char */
	/* 0x020 */ struct wow_m2_array recursion_model_filename; /* char */
	union
	{
		struct
		{
			/* 0x028 */ uint8_t blending_type2;
			/* 0x029 */ uint8_t emitter_type2;
			/* 0x02A */ uint16_t particle_color_index;
		};
		struct
		{
			/* 0x028 */ uint16_t blending_type;
			/* 0x02A */ uint16_t emitter_type;
		};
	};
	/* 0x02C */ uint8_t particle_type;
	/* 0x02D */ uint8_t head_or_tail;
	/* 0x02E */ int16_t texture_tile_rotation;
	/* 0x030 */ uint16_t texture_dimensions_rows;
	/* 0x032 */ uint16_t texture_dimensions_columns;
	/* 0x034 */ struct wow_m2_track_int emission_speed;       /* float */
	/* 0x050 */ struct wow_m2_track_int speed_variation;      /* float */
	/* 0x06C */ struct wow_m2_track_int vertical_range;       /* float */
	/* 0x088 */ struct wow_m2_track_int horizontal_range;     /* float */
	/* 0x0A4 */ struct wow_m2_track_int gravity;              /* float */
	/* 0x0C0 */ struct wow_m2_track_int lifespan;             /* float */
	/* 0x0DC */ struct wow_m2_track_int emission_rate;        /* float */
	/* 0x0F8 */ struct wow_m2_track_int emission_area_length; /* float */
	/* 0x114 */ struct wow_m2_track_int emission_area_width;  /* float */
	/* 0x130 */ struct wow_m2_track_int z_source;             /* float */
	/* 0x14C */ float mid_point;
	/* 0x150 */ struct wow_vec4b color_values[3];
	/* 0x15C */ float scale_values[3];
	/* 0x168 */ uint16_t lifespan_uv_anim[3];
	/* 0x16E */ uint16_t decay_uv_anim[3];
	/* 0x174 */ int16_t tail_uv_anim[2];
	/* 0x178 */ int16_t tail_decay_uv_anim[2];
	/* 0x17C */ float tail_length;
	/* 0x180 */ float twinkle_speed;
	/* 0x184 */ float twinkle_percent;
	/* 0x188 */ float twinkle_scale_min;
	/* 0x18C */ float twinkle_scale_max;
	/* 0x190 */ float burst_multiplier;
	/* 0x194 */ float drag;
	/* 0x198 */ float spin;
	/* 0x19C */ struct wow_m2_box tumble;
	/* 0x1B4 */ struct wow_vec3f wind_vector;
	/* 0x1C0 */ float wind_time;
	/* 0x1C4 */ float follow_speed1;
	/* 0x1C8 */ float follow_scale1;
	/* 0x1CC */ float follow_speed2;
	/* 0x1D0 */ float follow_scale2;
	/* 0x1D4 */ struct wow_m2_array spline_points;  /* struct wow_vec3f */
	/* 0x1DC */ struct wow_m2_track_int enabled_in; /* uint8_t */
};

#define READ_ARRAY_INT(name) \
static bool \
name##s_read(struct wow_m2_file *file, struct wow_mpq_file *mpq) \
{ \
	struct wow_m2_##name##_int *name##s; \
	bool ret = false; \
	if (!array_read((void**)&name##s, \
	                sizeof(*name##s), \
	                &file->name##s_nb, \
	                &file->header.name##s, \
	                mpq)) \
		goto end; \
	file->name##s = WOW_MALLOC(sizeof(*file->name##s) * file->name##s_nb); \
	if (!file->name##s) \
		goto end; \
	memset(file->name##s, 0, sizeof(*file->name##s) * file->name##s_nb); \
	for (uint32_t i = 0; i < file->name##s_nb; ++i) \
	{ \
		if (!name##_read(&file->name##s[i], \
		                 &name##s[i], \
		                 mpq)) \
			goto end; \
	} \
	ret = true; \
end: \
	WOW_FREE(name##s); \
	return ret; \
}

#define DUP_ARRAY_INT(name) \
struct wow_m2_##name * \
wow_m2_##name##s_dup(const struct wow_m2_##name *dup, uint32_t nb) \
{ \
	struct wow_m2_##name *name##s; \
	if (!dup || !nb) \
		return NULL; \
	name##s = WOW_MALLOC(sizeof(*name##s) * nb); \
	if (!name##s) \
		return NULL; \
	memset(name##s, 0, sizeof(*name##s) * nb); \
	for (uint32_t i = 0; i < nb; ++i) \
	{ \
		if (!name##_dup(&name##s[i], &dup[i])) \
		{ \
			wow_m2_##name##s_delete(name##s, i + 1); \
			return NULL; \
		} \
	} \
	return name##s; \
}

#define FREE_ARRAY_INT(name) \
void \
wow_m2_##name##s_delete(struct wow_m2_##name *val, uint32_t nb) \
{ \
	if (!val) \
		return; \
	for (uint32_t i = 0; i < nb; ++i) \
		name##_free(&val[i]); \
	WOW_FREE(val); \
}

#define ARRAY_FUNCTIONS(name) \
	READ_ARRAY_INT(name) \
	DUP_ARRAY_INT(name) \
	FREE_ARRAY_INT(name)

static bool
array_read(void **dst,
           size_t data_size,
           uint32_t *elem_nb,
           struct wow_m2_array *array,
           struct wow_mpq_file *mpq)
{
	void *data;

	data = WOW_MALLOC(data_size * array->count);
	if (!data
	 || wow_mpq_seek(mpq, array->offset, SEEK_SET) == -1
	 || wow_mpq_read(mpq, data, data_size * array->count) != data_size * array->count)
	{
		WOW_FREE(data);
		return false;
	}
	if (elem_nb)
		*elem_nb = array->count;
	*dst = data;
	return true;
}

static bool
array_dup(void **dst_data,
          uint32_t *dst_size,
          const void *data,
          uint32_t size,
          size_t data_size)
{
	void *dup;

	dup = WOW_MALLOC(data_size * size);
	if (!dup)
		return false;
	memcpy(dup, data, data_size * size);
	*dst_data = dup;
	if (dst_size)
		*dst_size = size;
	return true;
}

static void
array_free(void *data)
{
	WOW_FREE(data);
}

static bool
track_read(struct wow_m2_track *track,
           size_t data_size,
           struct wow_m2_track_int *track_int,
           struct wow_mpq_file *mpq)
{
	track->interpolation_type = track_int->base.interpolation_type;
	track->global_sequence = track_int->base.global_sequence;
	if (!array_read((void**)&track->interpolation_ranges,
	                sizeof(struct wow_m2_range),
	                &track->interpolation_ranges_nb,
	                &track_int->base.interpolation_ranges,
	                mpq)
	 || !array_read((void**)&track->timestamps,
	                sizeof(uint32_t),
	                &track->timestamps_nb,
	                &track_int->base.timestamps,
	                mpq)
	 || !array_read((void**)&track->values,
	                data_size,
	                &track->values_nb,
	                &track_int->values,
	                mpq))
		return false;
	return true;
}

static bool
track_dup(struct wow_m2_track *dst,
          const struct wow_m2_track *src,
          size_t data_size)
{
	dst->interpolation_type = src->interpolation_type;
	dst->global_sequence = src->global_sequence;
	if (!array_dup((void**)&dst->interpolation_ranges,
	               &dst->interpolation_ranges_nb,
	               src->interpolation_ranges,
	               src->interpolation_ranges_nb,
	               sizeof(*dst->interpolation_ranges))
	 || !array_dup((void**)&dst->timestamps,
	               &dst->timestamps_nb,
	               src->timestamps,
	               src->timestamps_nb,
	               sizeof(*dst->timestamps))
	 || !array_dup((void**)&dst->values,
	               &dst->values_nb,
	               src->values,
	               src->values_nb,
	               data_size))
		return false;
	return true;
}

static void
track_free(struct wow_m2_track *track)
{
	array_free(track->interpolation_ranges);
	array_free(track->timestamps);
	array_free(track->values);
}

static bool
texture_transform_read(struct wow_m2_texture_transform *texture_transform,
                       struct wow_m2_texture_transform_int *texture_transform_int,
                       struct wow_mpq_file *mpq)
{
	if (!track_read(&texture_transform->translation,
	                sizeof(struct wow_vec3f),
	                &texture_transform_int->translation,
	                mpq)
	 || !track_read(&texture_transform->rotation,
	                sizeof(struct wow_quatf),
	                &texture_transform_int->rotation,
	                mpq)
	 || !track_read(&texture_transform->scaling,
	                sizeof(struct wow_vec3f),
	                &texture_transform_int->scaling,
	                mpq))
		return false;
	return true;
}

static bool
texture_transform_dup(struct wow_m2_texture_transform *dst,
                      const struct wow_m2_texture_transform *src)
{
	if (!track_dup(&dst->translation,
	               &src->translation,
	               sizeof(struct wow_vec3f))
	 || !track_dup(&dst->rotation,
	               &src->rotation,
	               sizeof(struct wow_quatf))
	 || !track_dup(&dst->scaling,
	               &src->scaling,
	               sizeof(struct wow_vec3f)))
		return false;
	return true;
}

static void
texture_transform_free(struct wow_m2_texture_transform *val)
{
	track_free(&val->translation);
	track_free(&val->rotation);
	track_free(&val->scaling);
}

ARRAY_FUNCTIONS(texture_transform);

static bool
texture_weight_read(struct wow_m2_texture_weight *texture_weight,
                    struct wow_m2_texture_weight_int *texture_weight_int,
                    struct wow_mpq_file *mpq)
{
	if (!track_read(&texture_weight->weight,
	                sizeof(int16_t),
	                &texture_weight_int->weight,
	                mpq))
		return false;
	return true;
}

static bool
texture_weight_dup(struct wow_m2_texture_weight *dst,
                   const struct wow_m2_texture_weight *src)
{
	if (!track_dup(&dst->weight, &src->weight, sizeof(int16_t)))
		return false;
	return true;
}

static void
texture_weight_free(struct wow_m2_texture_weight *val)
{
	track_free(&val->weight);
}

ARRAY_FUNCTIONS(texture_weight);

static bool
skin_profile_read(struct wow_m2_skin_profile *skin_profile,
                  struct wow_m2_skin_profile_int *skin_profile_int,
                  struct wow_mpq_file *mpq)
{
	if (!array_read((void**)&skin_profile->sections,
	                sizeof(*skin_profile->sections),
	                &skin_profile->sections_nb,
	                &skin_profile_int->sections,
	                mpq)
	 || !array_read((void**)&skin_profile->batches,
	                sizeof(*skin_profile->batches),
	                &skin_profile->batches_nb,
	                &skin_profile_int->batches,
	                mpq)
	 || !array_read((void**)&skin_profile->vertexes,
	                sizeof(*skin_profile->vertexes),
	                &skin_profile->vertexes_nb,
	                &skin_profile_int->vertexes,
	                mpq)
	 || !array_read((void**)&skin_profile->indices,
	                sizeof(*skin_profile->indices),
	                &skin_profile->indices_nb,
	                &skin_profile_int->indices,
	                mpq)
	 || !array_read((void**)&skin_profile->bones,
	                sizeof(*skin_profile->bones),
	                &skin_profile->bones_nb,
	                &skin_profile_int->bones,
	                mpq))
		return false;
	return true;
}

static bool
skin_profile_dup(struct wow_m2_skin_profile *dst,
                 const struct wow_m2_skin_profile *src)
{
	if (!array_dup((void**)&dst->sections,
	               &dst->sections_nb,
	               src->sections,
	               src->sections_nb,
	               sizeof(*dst->sections))
	 || !array_dup((void**)&dst->batches,
	               &dst->batches_nb,
	               src->batches,
	               src->batches_nb,
	               sizeof(*dst->batches))
	 || !array_dup((void**)&dst->vertexes,
	               &dst->vertexes_nb,
	               src->vertexes,
	               src->vertexes_nb,
	               sizeof(*dst->vertexes))
	 || !array_dup((void**)&dst->indices,
	               &dst->indices_nb,
	               src->indices,
	               src->indices_nb,
	               sizeof(*dst->indices))
	 || !array_dup((void**)&dst->bones,
	               &dst->bones_nb,
	               src->bones,
	               src->bones_nb,
	               sizeof(*dst->bones)))
		return false;
	return true;
}

static void
skin_profile_free(struct wow_m2_skin_profile *val)
{
	array_free(val->sections);
	array_free(val->batches);
	array_free(val->vertexes);
	array_free(val->indices);
	array_free(val->bones);
}

ARRAY_FUNCTIONS(skin_profile);

static bool
attachment_read(struct wow_m2_attachment *attachment,
                struct wow_m2_attachment_int *attachment_int,
                struct wow_mpq_file *mpq)
{
	attachment->id = attachment_int->id;
	attachment->bone = attachment_int->bone;
	attachment->unknown = attachment_int->unknown;
	attachment->position = attachment_int->position;
	if (!track_read(&attachment->animate_attached,
	                sizeof(uint8_t),
	                &attachment_int->animate_attached,
	                mpq))
		return false;
	return true;
}

static bool
attachment_dup(struct wow_m2_attachment *dst,
               const struct wow_m2_attachment *src)
{
	dst->id = src->id;
	dst->bone = src->bone;
	dst->unknown = src->unknown;
	dst->position = src->position;
	if (!track_dup(&dst->animate_attached,
	               &src->animate_attached,
	               sizeof(uint8_t)))
		return false;
	return true;
}

static void
attachment_free(struct wow_m2_attachment *val)
{
	track_free(&val->animate_attached);
}

ARRAY_FUNCTIONS(attachment);

static bool
particle_read(struct wow_m2_particle *particle,
              struct wow_m2_particle_int *particle_int,
              struct wow_mpq_file *mpq)
{
	particle->id = particle_int->id;
	particle->flags = particle_int->flags;
	particle->position = particle_int->position;
	particle->bone = particle_int->bone;
	particle->texture = particle_int->texture;
	particle->blending_type = particle_int->blending_type;
	particle->emitter_type = particle_int->emitter_type;
	particle->particle_type = particle_int->particle_type;
	particle->head_or_tail = particle_int->head_or_tail;
	particle->texture_tile_rotation = particle_int->texture_tile_rotation;
	particle->texture_dimensions_rows = particle_int->texture_dimensions_rows;
	particle->texture_dimensions_columns = particle_int->texture_dimensions_columns;
	particle->mid_point = particle_int->mid_point;
	memcpy(particle->color_values,
	       particle_int->color_values,
	       sizeof(particle->color_values));
	memcpy(particle->scale_values,
	       particle_int->scale_values,
	       sizeof(particle->scale_values));
	memcpy(particle->lifespan_uv_anim,
	       particle_int->lifespan_uv_anim,
	       sizeof(particle->lifespan_uv_anim));
	memcpy(particle->decay_uv_anim,
	       particle_int->decay_uv_anim,
	       sizeof(particle->decay_uv_anim));
	memcpy(particle->tail_uv_anim,
	       particle_int->tail_uv_anim,
	       sizeof(particle->tail_uv_anim));
	memcpy(particle->tail_decay_uv_anim,
	       particle_int->tail_decay_uv_anim,
	       sizeof(particle->tail_decay_uv_anim));
	particle->tail_length = particle_int->tail_length;
	particle->twinkle_speed = particle_int->twinkle_speed;
	particle->twinkle_percent = particle_int->twinkle_percent;
	particle->twinkle_scale_min = particle_int->twinkle_scale_min;
	particle->twinkle_scale_max = particle_int->twinkle_scale_max;
	particle->burst_multiplier = particle_int->burst_multiplier;
	particle->drag = particle_int->drag;
	particle->spin = particle_int->spin;
	particle->tumble = particle_int->tumble;
	particle->wind_vector = particle_int->wind_vector;
	particle->wind_time = particle_int->wind_time;
	particle->follow_speed1 = particle_int->follow_speed1;
	particle->follow_scale1 = particle_int->follow_scale1;
	particle->follow_speed2 = particle_int->follow_speed2;
	particle->follow_scale2 = particle_int->follow_scale2;
	if (!array_read((void**)&particle->geometry_model_filename,
	                sizeof(*particle->geometry_model_filename),
	                NULL,
	                &particle_int->geometry_model_filename,
	                mpq)
	 || !array_read((void**)&particle->recursion_model_filename,
	                sizeof(*particle->recursion_model_filename),
	                NULL,
	                &particle_int->recursion_model_filename,
	                mpq)
	 || !track_read(&particle->emission_speed,
	                sizeof(float),
	                &particle_int->emission_speed,
	                mpq)
	 || !track_read(&particle->speed_variation,
	                sizeof(float),
	                &particle_int->speed_variation,
	                mpq)
	 || !track_read(&particle->vertical_range,
	                sizeof(float),
	                &particle_int->vertical_range,
	                mpq)
	 || !track_read(&particle->horizontal_range,
	                sizeof(float),
	                &particle_int->horizontal_range,
	                mpq)
	 || !track_read(&particle->gravity,
	                sizeof(float),
	                &particle_int->gravity,
	                mpq)
	 || !track_read(&particle->lifespan,
	                sizeof(float),
	                &particle_int->lifespan,
	                mpq)
	 || !track_read(&particle->emission_rate,
	                sizeof(float),
	                &particle_int->emission_rate,
	                mpq)
	 || !track_read(&particle->emission_area_length,
	                sizeof(float),
	                &particle_int->emission_area_length,
	                mpq)
	 || !track_read(&particle->emission_area_width,
	                sizeof(float),
	                &particle_int->emission_area_width,
	                mpq)
	 || !track_read(&particle->z_source,
	                sizeof(float),
	                &particle_int->z_source,
	                mpq)
	 || !array_read((void**)&particle->spline_points,
	                sizeof(*particle->spline_points),
	                &particle->spline_points_nb,
	                &particle_int->spline_points,
	                mpq)
	 || !track_read(&particle->enabled_in,
	                sizeof(uint8_t),
	                &particle_int->enabled_in,
	                mpq))
		return false;
	return true;
}

static bool
particle_dup(struct wow_m2_particle *dst,
             const struct wow_m2_particle *src)
{
	dst->id = src->id;
	dst->flags = src->flags;
	dst->position = src->position;
	dst->bone = src->bone;
	dst->texture = src->texture;
	dst->blending_type = src->blending_type;
	dst->emitter_type = src->emitter_type;
	dst->particle_type = src->particle_type;
	dst->head_or_tail = src->head_or_tail;
	dst->texture_tile_rotation = src->texture_tile_rotation;
	dst->texture_dimensions_rows = src->texture_dimensions_rows;
	dst->texture_dimensions_columns = src->texture_dimensions_columns;
	dst->mid_point = src->mid_point;
	memcpy(dst->color_values,
	       src->color_values,
	       sizeof(src->color_values));
	memcpy(dst->scale_values,
	       src->scale_values,
	       sizeof(src->scale_values));
	memcpy(dst->lifespan_uv_anim,
	       src->lifespan_uv_anim,
	       sizeof(src->lifespan_uv_anim));
	memcpy(dst->decay_uv_anim,
	       src->decay_uv_anim,
	       sizeof(src->decay_uv_anim));
	memcpy(dst->tail_uv_anim,
	       src->tail_uv_anim,
	       sizeof(src->tail_uv_anim));
	memcpy(dst->tail_decay_uv_anim,
	       src->tail_decay_uv_anim,
	       sizeof(src->tail_decay_uv_anim));
	dst->tail_length = src->tail_length;
	dst->twinkle_speed = src->twinkle_speed;
	dst->twinkle_percent = src->twinkle_percent;
	dst->twinkle_scale_min = src->twinkle_scale_min;
	dst->twinkle_scale_max = src->twinkle_scale_max;
	dst->burst_multiplier = src->burst_multiplier;
	dst->drag = src->drag;
	dst->spin = src->spin;
	dst->tumble = src->tumble;
	dst->wind_vector = src->wind_vector;
	dst->wind_time = src->wind_time;
	dst->follow_speed1 = src->follow_speed1;
	dst->follow_scale1 = src->follow_scale1;
	dst->follow_speed2 = src->follow_speed2;
	dst->follow_scale2 = src->follow_scale2;
	dst->spline_points_nb = src->spline_points_nb;
	if (!array_dup((void**)&dst->geometry_model_filename,
	               NULL,
	               src->geometry_model_filename,
	               strlen(src->geometry_model_filename) + 1,
	               sizeof(char))
	 || !array_dup((void**)&dst->recursion_model_filename,
	               NULL,
	               src->recursion_model_filename,
	               strlen(src->recursion_model_filename) + 1,
	               sizeof(char))
	 || !track_dup(&dst->emission_speed,
	               &src->emission_speed,
	               sizeof(float))
	 || !track_dup(&dst->speed_variation,
	               &src->speed_variation,
	               sizeof(float))
	 || !track_dup(&dst->vertical_range,
	               &src->vertical_range,
	               sizeof(float))
	 || !track_dup(&dst->horizontal_range,
	               &src->horizontal_range,
	               sizeof(float))
	 || !track_dup(&dst->gravity,
	               &src->gravity,
	               sizeof(float))
	 || !track_dup(&dst->lifespan,
	               &src->lifespan,
	               sizeof(float))
	 || !track_dup(&dst->emission_rate,
	               &src->emission_rate,
	               sizeof(float))
	 || !track_dup(&dst->emission_area_length,
	               &src->emission_area_length,
	               sizeof(float))
	 || !track_dup(&dst->emission_area_width,
	               &src->emission_area_width,
	               sizeof(float))
	 || !track_dup(&dst->z_source,
	               &src->z_source,
	               sizeof(float))
	 || !array_dup((void**)&dst->spline_points,
	               &dst->spline_points_nb,
	               src->spline_points,
	               src->spline_points_nb,
	               sizeof(*dst->spline_points))
	 || !track_dup(&dst->enabled_in,
	               &src->enabled_in,
	               sizeof(uint8_t)))
		return false;
	return true;
}

static void
particle_free(struct wow_m2_particle *val)
{
	array_free(val->geometry_model_filename);
	array_free(val->recursion_model_filename);
	track_free(&val->emission_speed);
	track_free(&val->speed_variation);
	track_free(&val->vertical_range);
	track_free(&val->horizontal_range);
	track_free(&val->gravity);
	track_free(&val->lifespan);
	track_free(&val->emission_rate);
	track_free(&val->emission_area_length);
	track_free(&val->emission_area_width);
	track_free(&val->z_source);
	array_free(val->spline_points);
	track_free(&val->enabled_in);
}

ARRAY_FUNCTIONS(particle);

static bool
texture_read(struct wow_m2_texture *texture,
             struct wow_m2_texture_int *texture_int,
             struct wow_mpq_file *mpq)
{
	texture->type = texture_int->type;
	texture->flags = texture_int->flags;
	if (!array_read((void**)&texture->filename,
	                sizeof(*texture->filename),
	                NULL,
	                &texture_int->filename,
	                mpq))
		return false;
	return true;
}

static bool
texture_dup(struct wow_m2_texture *dst,
            const struct wow_m2_texture *src)
{
	dst->type = src->type;
	dst->flags = src->flags;
	if (src->filename)
	{
		if (!array_dup((void**)&dst->filename,
		               NULL,
		               src->filename,
		               strlen(src->filename) + 1,
		               sizeof(char)))
			return false;
	}
	return true;
}

static void
texture_free(struct wow_m2_texture *val)
{
	array_free(val->filename);
}

ARRAY_FUNCTIONS(texture);

static bool
camera_read(struct wow_m2_camera *camera,
            struct wow_m2_camera_int *camera_int,
            struct wow_mpq_file *mpq)
{
	camera->type = camera_int->type;
	camera->fov = camera_int->fov;
	camera->far_clip = camera_int->far_clip;
	camera->near_clip = camera_int->near_clip;
	camera->position_base = camera_int->position_base;
	camera->target_position_base = camera_int->target_position_base;
	if (!track_read(&camera->position,
	                sizeof(struct wow_m2_spline_vec3f),
	                &camera_int->position,
	                mpq)
	 || !track_read(&camera->target_position,
	                sizeof(struct wow_m2_spline_vec3f),
	                &camera_int->target_position,
	                mpq)
	 || !track_read(&camera->roll,
	                sizeof(struct wow_m2_spline_float),
	                &camera_int->roll,
	                mpq))
		return false;
	return true;
}

static bool
camera_dup(struct wow_m2_camera *dst,
           const struct wow_m2_camera *src)
{
	dst->type = src->type;
	dst->fov = src->fov;
	dst->far_clip = src->far_clip;
	dst->near_clip = src->near_clip;
	dst->position_base = src->position_base;
	dst->target_position_base = src->target_position_base;
	if (!track_dup(&dst->position,
	               &src->position,
	               sizeof(struct wow_m2_spline_vec3f))
	 || !track_dup(&dst->target_position,
	               &src->target_position,
	               sizeof(struct wow_m2_spline_vec3f))
	 || !track_dup(&dst->roll,
	               &src->roll,
	               sizeof(struct wow_m2_spline_float)))
		return false;
	return true;
}

static void
camera_free(struct wow_m2_camera *val)
{
	track_free(&val->position);
	track_free(&val->target_position);
	track_free(&val->roll);
}

ARRAY_FUNCTIONS(camera);

static bool
ribbon_read(struct wow_m2_ribbon *ribbon,
            struct wow_m2_ribbon_int *ribbon_int,
            struct wow_mpq_file *mpq)
{
	ribbon->ribbon_id = ribbon_int->ribbon_id;
	ribbon->bone_index = ribbon_int->bone_index;
	ribbon->position = ribbon_int->position;
	ribbon->edges_per_second = ribbon_int->edges_per_second;
	ribbon->edge_lifetime = ribbon_int->edge_lifetime;
	ribbon->gravity = ribbon_int->gravity;
	ribbon->texture_rows = ribbon_int->texture_rows;
	ribbon->texture_cols = ribbon_int->texture_cols;
	if (!array_read((void**)&ribbon->texture_indices,
	                sizeof(uint16_t),
	                &ribbon->texture_indices_nb,
	                &ribbon_int->texture_indices,
	                mpq)
	 || !array_read((void**)&ribbon->material_indices,
	                sizeof(uint16_t),
	                &ribbon->material_indices_nb,
	                &ribbon_int->material_indices,
	                mpq)
	 || !track_read(&ribbon->color,
	                sizeof(struct wow_vec3f),
	                &ribbon_int->color,
	                mpq)
	 || !track_read(&ribbon->alpha,
	                sizeof(int16_t),
	                &ribbon_int->alpha, mpq)
	 || !track_read(&ribbon->height_above,
	                sizeof(float),
	                &ribbon_int->height_above,
	                mpq)
	 || !track_read(&ribbon->height_below,
	                sizeof(float),
	                &ribbon_int->height_below,
	                mpq)
	 || !track_read(&ribbon->tex_slot,
	                sizeof(uint16_t),
	                &ribbon_int->tex_slot,
	                mpq)
	 || !track_read(&ribbon->visibility,
	                sizeof(uint8_t),
	                &ribbon_int->visibility,
	                mpq))
		return false;
	return true;
}

static bool
ribbon_dup(struct wow_m2_ribbon *dst,
           const struct wow_m2_ribbon *src)
{
	dst->ribbon_id = src->ribbon_id;
	dst->bone_index = src->bone_index;
	dst->position = src->position;
	dst->edges_per_second = src->edges_per_second;
	dst->edge_lifetime = src->edge_lifetime;
	dst->gravity = src->gravity;
	dst->texture_rows = src->texture_rows;
	dst->texture_cols = src->texture_cols;
	if (!array_dup((void**)&dst->texture_indices,
	               &dst->texture_indices_nb,
	               src->texture_indices,
	               src->texture_indices_nb,
	               sizeof(*dst->texture_indices))
	 || !array_dup((void**)&dst->material_indices,
	               &dst->material_indices_nb,
	               src->material_indices,
	               src->material_indices_nb,
	               sizeof(*dst->material_indices))
	 || !track_dup(&dst->color,
	               &src->color,
	               sizeof(struct wow_vec3f))
	 || !track_dup(&dst->alpha,
	               &src->alpha,
	               sizeof(int16_t))
	 || !track_dup(&dst->height_above,
	               &src->height_above,
	               sizeof(float))
	 || !track_dup(&dst->height_below,
	               &src->height_below,
	               sizeof(float))
	 || !track_dup(&dst->tex_slot,
	               &src->tex_slot,
	               sizeof(uint16_t))
	 || !track_dup(&dst->visibility,
	               &src->visibility,
	               sizeof(uint8_t)))
		return false;
	return true;
}

static void
ribbon_free(struct wow_m2_ribbon *val)
{
	array_free(val->texture_indices);
	array_free(val->material_indices);
	track_free(&val->color);
	track_free(&val->alpha);
	track_free(&val->height_above);
	track_free(&val->height_below);
	track_free(&val->tex_slot);
	track_free(&val->visibility);
}

ARRAY_FUNCTIONS(ribbon);

static bool
color_read(struct wow_m2_color *color,
           struct wow_m2_color_int *color_int,
           struct wow_mpq_file *mpq)
{
	if (!track_read(&color->color,
	                sizeof(struct wow_vec3f),
	                &color_int->color,
	                mpq)
	 || !track_read(&color->alpha,
	                sizeof(int16_t),
	                &color_int->alpha,
	                mpq))
		return false;
	return true;
}

static bool
color_dup(struct wow_m2_color *dst,
          const struct wow_m2_color *src)
{
	if (!track_dup(&dst->color,
	               &src->color,
	               sizeof(struct wow_vec3f))
	 || !track_dup(&dst->alpha,
	               &src->alpha,
	               sizeof(int16_t)))
		return false;
	return true;
}

static void
color_free(struct wow_m2_color *val)
{
	track_free(&val->color);
	track_free(&val->alpha);
}

ARRAY_FUNCTIONS(color);

static bool
light_read(struct wow_m2_light *light,
           struct wow_m2_light_int *light_int,
           struct wow_mpq_file *mpq)
{
	light->type = light_int->type;
	light->bone = light_int->bone;
	light->position = light_int->position;
	if (!track_read(&light->ambient_color,
	                sizeof(struct wow_vec3f),
	                &light_int->ambient_color,
	                mpq)
	 || !track_read(&light->ambient_intensity,
	                sizeof(float),
	                &light_int->ambient_intensity,
	                mpq)
	 || !track_read(&light->diffuse_color,
	                sizeof(struct wow_vec3f),
	                &light_int->diffuse_color,
	                mpq)
	 || !track_read(&light->diffuse_intensity,
	                sizeof(float),
	                &light_int->diffuse_intensity,
	                mpq)
	 || !track_read(&light->attenuation_start,
	                sizeof(float),
	                &light_int->attenuation_start,
	                mpq)
	 || !track_read(&light->attenuation_end,
	                sizeof(float),
	                &light_int->attenuation_end,
	                mpq)
	 || !track_read(&light->visibility,
	                sizeof(uint8_t),
	                &light_int->visibility,
	                mpq))
		return false;
	return true;
}

static bool
light_dup(struct wow_m2_light *dst,
          const struct wow_m2_light *src)
{
	dst->type = src->type;
	dst->bone = src->bone;
	dst->position = src->position;
	if (!track_dup(&dst->ambient_color,
	               &src->ambient_color,
	               sizeof(struct wow_vec3f))
	 || !track_dup(&dst->ambient_intensity,
	               &src->ambient_intensity,
	               sizeof(float))
	 || !track_dup(&dst->diffuse_color,
	               &src->diffuse_color,
	               sizeof(struct wow_vec3f))
	 || !track_dup(&dst->diffuse_intensity,
	               &src->diffuse_intensity,
	               sizeof(float))
	 || !track_dup(&dst->attenuation_start,
	               &src->attenuation_start,
	               sizeof(float))
	 || !track_dup(&dst->attenuation_end,
	               &src->attenuation_end,
	               sizeof(float))
	 || !track_dup(&dst->visibility,
	               &src->visibility,
	               sizeof(uint8_t)))
		return false;
	return true;
}

static void
light_free(struct wow_m2_light *val)
{
	track_free(&val->ambient_color);
	track_free(&val->ambient_intensity);
	track_free(&val->diffuse_color);
	track_free(&val->diffuse_intensity);
	track_free(&val->attenuation_start);
	track_free(&val->attenuation_end);
	track_free(&val->visibility);
}

ARRAY_FUNCTIONS(light);

static bool
bone_read(struct wow_m2_bone *bone,
          struct wow_m2_bone_int *bone_int,
          struct wow_mpq_file *mpq)
{
	bone->key_bone_id = bone_int->key_bone_id;
	bone->flags = bone_int->flags;
	bone->parent_bone = bone_int->parent_bone;
	bone->submesh_id = bone_int->submesh_id;
	bone->bone_name_crc = bone_int->bone_name_crc;
	bone->pivot = bone_int->pivot;
	if (!track_read(&bone->translation,
	                sizeof(struct wow_vec3f),
	                &bone_int->translation,
	                mpq)
	 || !track_read(&bone->rotation,
	                sizeof(struct wow_quats),
	                &bone_int->rotation,
	                mpq)
	 || !track_read(&bone->scale,
	                sizeof(struct wow_vec3f),
	                &bone_int->scale,
	                mpq))
		return false;
	return true;
}

static bool
bone_dup(struct wow_m2_bone *dst,
         const struct wow_m2_bone *src)
{
	dst->key_bone_id = src->key_bone_id;
	dst->flags = src->flags;
	dst->parent_bone = src->parent_bone;
	dst->submesh_id = src->submesh_id;
	dst->bone_name_crc = src->bone_name_crc;
	dst->pivot = src->pivot;
	if (!track_dup(&dst->translation,
	               &src->translation,
	               sizeof(struct wow_vec3f))
	 || !track_dup(&dst->rotation,
	               &src->rotation,
	               sizeof(struct wow_quats))
	 || !track_dup(&dst->scale,
	               &src->scale,
	               sizeof(struct wow_vec3f)))
		return false;
	return true;
}

static void
bone_free(struct wow_m2_bone *val)
{
	track_free(&val->translation);
	track_free(&val->rotation);
	track_free(&val->scale);
}

ARRAY_FUNCTIONS(bone);

struct wow_m2_file *
wow_m2_file_new(struct wow_mpq_file *mpq)
{
	struct wow_m2_file *file;

	file = WOW_MALLOC(sizeof(*file));
	if (!file)
		return NULL;
	memset(file, 0, sizeof(*file));
	if (wow_mpq_read(mpq, &file->header, sizeof(file->header)) != sizeof(file->header)
	 || !array_read((void**)&file->materials,
	                sizeof(*file->materials),
	                &file->materials_nb,
	                &file->header.materials,
	                mpq)
	 || !array_read((void**)&file->sequences,
	                sizeof(*file->sequences),
	                &file->sequences_nb,
	                &file->header.sequences,
	                mpq)
	 || !array_read((void**)&file->vertexes,
	                sizeof(*file->vertexes),
	                &file->vertexes_nb,
	                &file->header.vertexes,
	                mpq)
	 || !array_read((void**)&file->texture_transforms_lookups,
	                sizeof(*file->texture_transforms_lookups),
	                &file->texture_transforms_lookups_nb,
	                &file->header.texture_transforms_lookup_table,
	                mpq)
	 || !array_read((void**)&file->texture_weights_lookups,
	                sizeof(*file->texture_weights_lookups),
	                &file->texture_weights_lookups_nb,
	                &file->header.texture_weights_lookup_table,
	                mpq))
		goto err;
	if (file->header.flags & WOW_M2_HEADER_FLAG_USE_TEXTURE_COMBINER_COMBO)
	{
		if (!array_read((void**)&file->texture_combiner_combos,
		                sizeof(*file->texture_combiner_combos),
		                &file->texture_combiner_combos_nb,
		                &file->header.texture_combiner_combos,
		                mpq))
			goto err;
	}
	if (!array_read((void**)&file->texture_unit_lookups,
	                sizeof(*file->texture_unit_lookups),
	                &file->texture_unit_lookups_nb,
	                &file->header.texture_unit_lookup_table,
	                mpq)
	 || !array_read((void**)&file->attachment_lookups,
	                sizeof(*file->attachment_lookups),
	                &file->attachment_lookups_nb,
	                &file->header.attachment_lookup_table,
	                mpq)
	 || !array_read((void**)&file->key_bone_lookups,
	                sizeof(*file->key_bone_lookups),
	                &file->key_bone_lookups_nb,
	                &file->header.key_bone_lookup_table,
	                mpq)
	 || !array_read((void**)&file->sequence_lookups,
	                sizeof(*file->sequence_lookups),
	                &file->sequence_lookups_nb,
	                &file->header.sequence_lookup_table,
	                mpq)
	 || !array_read((void**)&file->global_sequences,
	                sizeof(*file->global_sequences),
	                &file->global_sequences_nb,
	                &file->header.global_sequences,
	                mpq)
	 || !array_read((void**)&file->texture_lookups,
	                sizeof(*file->texture_lookups),
	                &file->texture_lookups_nb,
	                &file->header.texture_lookup_table,
	                mpq)
	 || !array_read((void**)&file->camera_lookups,
	                sizeof(*file->camera_lookups),
	                &file->camera_lookups_nb,
	                &file->header.camera_lookup_table,
	                mpq)
	 || !array_read((void**)&file->bone_lookups,
	                sizeof(*file->bone_lookups),
	                &file->bone_lookups_nb,
	                &file->header.bone_lookup_table,
	                mpq)
	 || !array_read((void**)&file->playable_animations,
	                sizeof(*file->playable_animations),
	                &file->playable_animations_nb,
	                &file->header.playable_animations,
	                mpq)
	 || !array_read((void**)&file->collision_triangles,
	                sizeof(*file->collision_triangles),
	                &file->collision_triangles_nb,
	                &file->header.collision_triangles,
	                mpq)
	 || !array_read((void**)&file->collision_vertexes,
	                sizeof(*file->collision_vertexes),
	                &file->collision_vertexes_nb,
	                &file->header.collision_vertexes,
	                mpq)
	 || !array_read((void**)&file->collision_normals,
	                sizeof(*file->collision_normals),
	                &file->collision_normals_nb,
	                &file->header.collision_normals,
	                mpq)
	 || !texture_transforms_read(file, mpq)
	 || !texture_weights_read(file, mpq)
	 || !skin_profiles_read(file, mpq)
	 || !attachments_read(file, mpq)
	 || !particles_read(file, mpq)
	 || !textures_read(file, mpq)
	 || !cameras_read(file, mpq)
	 || !ribbons_read(file, mpq)
	 || !colors_read(file, mpq)
	 || !lights_read(file, mpq)
	 || !bones_read(file, mpq)
	 || !array_read((void**)&file->name,
	                sizeof(*file->name),
	                NULL,
	                &file->header.name,
	                mpq))
		goto err;
	return file;

err:
	wow_m2_file_delete(file);
	return NULL;
}

void
wow_m2_file_delete(struct wow_m2_file *file)
{
	if (!file)
		return;
	array_free(file->playable_animations);
	wow_m2_texture_transforms_delete(file->texture_transforms,
	                                 file->texture_transforms_nb);
	wow_m2_texture_weights_delete(file->texture_weights,
	                              file->texture_weights_nb);
	wow_m2_skin_profiles_delete(file->skin_profiles,
	                            file->skin_profiles_nb);
	array_free(file->materials);
	wow_m2_attachments_delete(file->attachments, file->attachments_nb);
	array_free(file->sequences);
	wow_m2_particles_delete(file->particles, file->particles_nb);
	wow_m2_textures_delete(file->textures, file->textures_nb);
	array_free(file->vertexes);
	wow_m2_cameras_delete(file->cameras, file->cameras_nb);
	wow_m2_ribbons_delete(file->ribbons, file->ribbons_nb);
	wow_m2_colors_delete(file->colors, file->colors_nb);
	wow_m2_lights_delete(file->lights, file->lights_nb);
	wow_m2_bones_delete(file->bones, file->bones_nb);
	array_free(file->texture_transforms_lookups);
	array_free(file->texture_weights_lookups);
	array_free(file->texture_combiner_combos);
	array_free(file->texture_unit_lookups);
	array_free(file->collision_triangles);
	array_free(file->attachment_lookups);
	array_free(file->collision_vertexes);
	array_free(file->collision_normals);
	array_free(file->key_bone_lookups);
	array_free(file->sequence_lookups);
	array_free(file->global_sequences);
	array_free(file->texture_lookups);
	array_free(file->camera_lookups);
	array_free(file->bone_lookups);
	array_free(file->name);
	array_free(file);
}
