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

static void *array_read(size_t data_size, uint32_t *elem_nb, struct wow_m2_array *array, struct wow_mpq_file *mpq)
{
	void *data = WOW_MALLOC(data_size * array->count);
	if (!data)
		return NULL;
	if (wow_mpq_seek(mpq, array->offset, SEEK_SET) == -1)
	{
		WOW_FREE(data);
		return NULL;
	}
	if (wow_mpq_read(mpq, data, data_size * array->count) != data_size * array->count)
	{
		WOW_FREE(data);
		return NULL;
	}
	if (elem_nb)
		*elem_nb = array->count;
	return data;
}

static bool read_track(struct wow_m2_track *track, size_t data_size, struct wow_m2_track_int *track_int, struct wow_mpq_file *mpq)
{
	track->interpolation_type = track_int->base.interpolation_type;
	track->global_sequence = track_int->base.global_sequence;
	track->interpolation_ranges = array_read(sizeof(struct wow_m2_range), &track->interpolation_ranges_nb, &track_int->base.interpolation_ranges, mpq);
	if (!track->interpolation_ranges)
		return false;
	track->timestamps = array_read(sizeof(uint32_t), &track->timestamps_nb, &track_int->base.timestamps, mpq);
	if (!track->timestamps)
		return false;
	track->values = array_read(data_size, &track->values_nb, &track_int->values, mpq);
	if (!track->values)
		return false;
	return true;
}

static bool read_texture_transforms(struct wow_m2_file *file, struct wow_mpq_file *mpq)
{
	bool ret = false;
	struct wow_m2_texture_transform_int *texture_transforms = array_read(sizeof(*texture_transforms), &file->texture_transforms_nb, &file->header.texture_transforms, mpq);
	if (!texture_transforms)
		goto err;
	file->texture_transforms = WOW_MALLOC(sizeof(*file->texture_transforms) * file->texture_transforms_nb);
	if (!file->texture_transforms)
		goto err;
	memset(file->texture_transforms, 0, sizeof(*file->texture_transforms) * file->texture_transforms_nb);
	for (uint32_t i = 0; i < file->texture_transforms_nb; ++i)
	{
		struct wow_m2_texture_transform *texture_transform = &file->texture_transforms[i];
		if (!read_track(&texture_transform->translation, sizeof(struct wow_vec3f), &texture_transforms[i].translation, mpq)
		 || !read_track(&texture_transform->rotation, sizeof(struct wow_quatf), &texture_transforms[i].rotation, mpq)
		 || !read_track(&texture_transform->scaling, sizeof(struct wow_vec3f), &texture_transforms[i].scaling, mpq))
			goto err;
	}
	ret = true;

err:
	WOW_FREE(texture_transforms);
	return ret;
}

static bool read_texture_weights(struct wow_m2_file *file, struct wow_mpq_file *mpq)
{
	bool ret = false;
	struct wow_m2_texture_weight_int *texture_weights = array_read(sizeof(*texture_weights), &file->texture_weights_nb, &file->header.texture_weights, mpq);
	if (!texture_weights)
		goto err;
	file->texture_weights = WOW_MALLOC(sizeof(*file->texture_weights) * file->texture_weights_nb);
	if (!file->texture_weights)
		goto err;
	memset(file->texture_weights, 0, sizeof(*file->texture_weights) * file->texture_weights_nb);
	for (uint32_t i = 0;i  < file->texture_weights_nb; ++i)
	{
		struct wow_m2_texture_weight *texture_weight = &file->texture_weights[i];
		if (!read_track(&texture_weight->weight, sizeof(int16_t), &texture_weights[i].weight, mpq))
			goto err;
	}
	ret = true;

err:
	WOW_FREE(texture_weights);
	return ret;
}

static bool read_skin_profiles(struct wow_m2_file *file, struct wow_mpq_file *mpq)
{
	bool ret = false;
	struct wow_m2_skin_profile_int *skin_profiles = array_read(sizeof(*skin_profiles), &file->skin_profiles_nb, &file->header.skin_profiles, mpq);
	if (!skin_profiles)
		goto err;
	file->skin_profiles = WOW_MALLOC(sizeof(*file->skin_profiles) * file->skin_profiles_nb);
	if (!file->skin_profiles)
		goto err;
	memset(file->skin_profiles, 0, sizeof(*file->skin_profiles) * file->skin_profiles_nb);
	for (uint32_t i = 0; i < file->skin_profiles_nb; ++i)
	{
		struct wow_m2_skin_profile *skin_profile = &file->skin_profiles[i];
		skin_profile->sections = array_read(sizeof(*skin_profile->sections), &skin_profile->sections_nb, &skin_profiles[i].sections, mpq);
		if (!skin_profile->sections)
			goto err;
		skin_profile->batches = array_read(sizeof(*skin_profile->batches), &skin_profile->batches_nb, &skin_profiles[i].batches, mpq);
		if (!skin_profile->batches)
			goto err;
		skin_profile->vertexes = array_read(sizeof(*skin_profile->vertexes), &skin_profile->vertexes_nb, &skin_profiles[i].vertexes, mpq);
		if (!skin_profile->vertexes)
			goto err;
		skin_profile->indices = array_read(sizeof(*skin_profile->indices), &skin_profile->indices_nb, &skin_profiles[i].indices, mpq);
		if (!skin_profile->indices)
			goto err;
		skin_profile->bones = array_read(sizeof(*skin_profile->bones), &skin_profile->bones_nb, &skin_profiles[i].bones, mpq);
		if (!skin_profile->bones)
			goto err;
	}
	ret = true;

err:
	WOW_FREE(skin_profiles);
	return ret;
}

static bool read_attachments(struct wow_m2_file *file, struct wow_mpq_file *mpq)
{
	bool ret = false;
	struct wow_m2_attachment_int *attachments = array_read(sizeof(*attachments), &file->attachments_nb, &file->header.attachments, mpq);
	if (!attachments)
		goto err;
	file->attachments = WOW_MALLOC(sizeof(*file->attachments) * file->attachments_nb);
	if (!file->attachments)
		goto err;
	memset(file->attachments, 0, sizeof(*file->attachments) * file->attachments_nb);
	for (uint32_t i = 0; i < file->attachments_nb; ++i)
	{
		struct wow_m2_attachment *attachment = &file->attachments[i];
		attachment->id = attachments[i].id;
		attachment->bone = attachments[i].bone;
		attachment->unknown = attachments[i].unknown;
		attachment->position = attachments[i].position;
		if (!read_track(&attachment->animate_attached, sizeof(uint8_t), &attachments[i].animate_attached, mpq))
			goto err;
	}
	ret = true;

err:
	WOW_FREE(attachments);
	return ret;
}

static bool read_particles(struct wow_m2_file *file, struct wow_mpq_file *mpq)
{
	bool ret = false;
	struct wow_m2_particle_int *particles = array_read(sizeof(*particles), &file->particles_nb, &file->header.particles, mpq);
	if (!particles)
		goto err;
	file->particles = WOW_MALLOC(sizeof(*file->particles) * file->particles_nb);
	if (!file->particles)
		goto err;
	memset(file->particles, 0, sizeof(*file->particles) * file->particles_nb);
	for (uint32_t i = 0; i < file->particles_nb; ++i)
	{
		struct wow_m2_particle *particle = &file->particles[i];
		particle->id = particles[i].id;
		particle->flags = particles[i].flags;
		particle->position = particles[i].position;
		particle->bone = particles[i].bone;
		particle->texture = particles[i].texture;
		particle->geometry_model_filename = array_read(sizeof(*particle->geometry_model_filename), NULL, &particles[i].geometry_model_filename, mpq);
		if (!particle->geometry_model_filename)
			goto err;
		particle->recursion_model_filename = array_read(sizeof(*particle->recursion_model_filename), NULL, &particles[i].recursion_model_filename, mpq);
		if (!particle->recursion_model_filename)
			goto err;
		particle->blending_type = particles[i].blending_type;
		particle->emitter_type = particles[i].emitter_type;
		particle->particle_type = particles[i].particle_type;
		particle->head_or_tail = particles[i].head_or_tail;
		particle->texture_tile_rotation = particles[i].texture_tile_rotation;
		particle->texture_dimensions_rows = particles[i].texture_dimensions_rows;
		particle->texture_dimensions_columns = particles[i].texture_dimensions_columns;
		if (!read_track(&particle->emission_speed, sizeof(float), &particles[i].emission_speed, mpq)
		 || !read_track(&particle->speed_variation, sizeof(float), &particles[i].speed_variation, mpq)
		 || !read_track(&particle->vertical_range, sizeof(float), &particles[i].vertical_range, mpq)
		 || !read_track(&particle->horizontal_range, sizeof(float), &particles[i].horizontal_range, mpq)
		 || !read_track(&particle->gravity, sizeof(float), &particles[i].gravity, mpq)
		 || !read_track(&particle->lifespan, sizeof(float), &particles[i].lifespan, mpq)
		 || !read_track(&particle->emission_rate, sizeof(float), &particles[i].emission_rate, mpq)
		 || !read_track(&particle->emission_area_length, sizeof(float), &particles[i].emission_area_length, mpq)
		 || !read_track(&particle->emission_area_width, sizeof(float), &particles[i].emission_area_width, mpq)
		 || !read_track(&particle->z_source, sizeof(float), &particles[i].z_source, mpq))
			goto err;
		particle->mid_point = particles[i].mid_point;
		memcpy(particle->color_values, particles[i].color_values, sizeof(particle->color_values));
		memcpy(particle->scale_values, particles[i].scale_values, sizeof(particle->scale_values));
		memcpy(particle->lifespan_uv_anim, particles[i].lifespan_uv_anim, sizeof(particle->lifespan_uv_anim));
		memcpy(particle->decay_uv_anim, particles[i].decay_uv_anim, sizeof(particle->decay_uv_anim));
		memcpy(particle->tail_uv_anim, particles[i].tail_uv_anim, sizeof(particle->tail_uv_anim));
		memcpy(particle->tail_decay_uv_anim, particles[i].tail_decay_uv_anim, sizeof(particle->tail_decay_uv_anim));
		particle->tail_length = particles[i].tail_length;
		particle->twinkle_speed = particles[i].twinkle_speed;
		particle->twinkle_percent = particles[i].twinkle_percent;
		particle->twinkle_scale_min = particles[i].twinkle_scale_min;
		particle->twinkle_scale_max = particles[i].twinkle_scale_max;
		particle->burst_multiplier = particles[i].burst_multiplier;
		particle->drag = particles[i].drag;
		particle->spin = particles[i].spin;
		particle->tumble = particles[i].tumble;
		particle->wind_vector = particles[i].wind_vector;
		particle->wind_time = particles[i].wind_time;
		particle->follow_speed1 = particles[i].follow_speed1;
		particle->follow_scale1 = particles[i].follow_scale1;
		particle->follow_speed2 = particles[i].follow_speed2;
		particle->follow_scale2 = particles[i].follow_scale2;
		particle->spline_points = array_read(sizeof(*particle->spline_points), &particle->spline_points_nb, &particles[i].spline_points, mpq);
		if (!particle->spline_points)
			goto err;
		if (!read_track(&particle->enabled_in, sizeof(uint8_t), &particles[i].enabled_in, mpq))
			goto err;
	}
	ret = true;

err:
	WOW_FREE(particles);
	return ret;
}

static bool read_textures(struct wow_m2_file *file, struct wow_mpq_file *mpq)
{
	bool ret = false;
	struct wow_m2_texture_int *textures = array_read(sizeof(*textures), &file->textures_nb, &file->header.textures, mpq);
	if (!textures)
		goto err;
	file->textures = WOW_MALLOC(sizeof(*file->textures) * file->textures_nb);
	if (!file->textures)
		goto err;
	memset(file->textures, 0, sizeof(*file->textures) * file->textures_nb);
	for (uint32_t i = 0; i < file->textures_nb; ++i)
	{
		struct wow_m2_texture *texture = &file->textures[i];
		texture->type = textures[i].type;
		texture->flags = textures[i].flags;
		texture->filename = array_read(sizeof(*texture->filename), NULL, &textures[i].filename, mpq);
		if (!texture->filename)
			goto err;
	}
	ret = true;

err:
	WOW_FREE(textures);
	return ret;
}

static bool read_cameras(struct wow_m2_file *file, struct wow_mpq_file *mpq)
{
	bool ret = false;
	struct wow_m2_camera_int *cameras = array_read(sizeof(*cameras), &file->cameras_nb, &file->header.cameras, mpq);
	if (!cameras)
		goto err;
	file->cameras = WOW_MALLOC(sizeof(*file->cameras) * file->cameras_nb);
	if (!file->cameras)
		goto err;
	memset(file->cameras, 0, sizeof(*file->cameras) * file->cameras_nb);
	for (uint32_t i = 0; i < file->cameras_nb; ++i)
	{
		struct wow_m2_camera *camera = &file->cameras[i];
		camera->type = cameras[i].type;
		camera->fov = cameras[i].fov;
		camera->far_clip = cameras[i].far_clip;
		camera->near_clip = cameras[i].near_clip;
		camera->position_base = cameras[i].position_base;
		camera->target_position_base = cameras[i].target_position_base;
		if (!read_track(&camera->position, sizeof(struct wow_m2_spline_vec3f), &cameras[i].position, mpq)
		 || !read_track(&camera->target_position, sizeof(struct wow_m2_spline_vec3f), &cameras[i].target_position, mpq)
		 || !read_track(&camera->roll, sizeof(struct wow_m2_spline_float), &cameras[i].roll, mpq))
			goto err;
	}
	ret = true;

err:
	WOW_FREE(cameras);
	return ret;
}

static bool read_ribbons(struct wow_m2_file *file, struct wow_mpq_file *mpq)
{
	bool ret = false;
	struct wow_m2_ribbon_int *ribbons = array_read(sizeof(*ribbons), &file->ribbons_nb, &file->header.ribbons, mpq);
	if (!ribbons)
		goto err;
	file->ribbons = WOW_MALLOC(sizeof(*file->ribbons) * file->ribbons_nb);
	if (!file->ribbons)
		goto err;
	memset(file->ribbons, 0, sizeof(*file->ribbons) * file->ribbons_nb);
	for (uint32_t i = 0; i < file->ribbons_nb; ++i)
	{
		struct wow_m2_ribbon *ribbon = &file->ribbons[i];
		ribbon->ribbon_id = ribbons[i].ribbon_id;
		ribbon->bone_index = ribbons[i].bone_index;
		ribbon->position = ribbons[i].position;
		ribbon->edges_per_second = ribbons[i].edges_per_second;
		ribbon->edge_lifetime = ribbons[i].edge_lifetime;
		ribbon->gravity = ribbons[i].gravity;
		ribbon->texture_rows = ribbons[i].texture_rows;
		ribbon->texture_cols = ribbons[i].texture_cols;
		ribbon->texture_indices = array_read(sizeof(uint16_t), &ribbon->texture_indices_nb, &ribbons[i].texture_indices, mpq);
		ribbon->material_indices = array_read(sizeof(uint16_t), &ribbon->material_indices_nb, &ribbons[i].material_indices, mpq);
		if (!ribbon->texture_indices
		 || !ribbon->material_indices
		 || !read_track(&ribbon->color, sizeof(struct wow_vec3f), &ribbons[i].color, mpq)
		 || !read_track(&ribbon->alpha, sizeof(int16_t), &ribbons[i].alpha, mpq)
		 || !read_track(&ribbon->height_above, sizeof(float), &ribbons[i].height_above, mpq)
		 || !read_track(&ribbon->height_below, sizeof(float), &ribbons[i].height_below, mpq)
		 || !read_track(&ribbon->tex_slot, sizeof(uint16_t), &ribbons[i].tex_slot, mpq)
		 || !read_track(&ribbon->visibility, sizeof(uint8_t), &ribbons[i].visibility, mpq))
			goto err;
	}
	ret = true;

err:
	WOW_FREE(ribbons);
	return ret;
}

static bool read_colors(struct wow_m2_file *file, struct wow_mpq_file *mpq)
{
	bool ret = false;
	struct wow_m2_color_int *colors = array_read(sizeof(*colors), &file->colors_nb, &file->header.colors, mpq);
	if (!colors)
		goto err;
	file->colors = WOW_MALLOC(sizeof(*file->colors) * file->colors_nb);
	if (!file->colors)
		goto err;
	memset(file->colors, 0, sizeof(*file->colors) * file->colors_nb);
	for (uint32_t i = 0; i < file->colors_nb; ++i)
	{
		struct wow_m2_color *color = &file->colors[i];
		if (!read_track(&color->color, sizeof(struct wow_vec3f), &colors[i].color, mpq)
		 || !read_track(&color->alpha, sizeof(int16_t), &colors[i].alpha, mpq))
			goto err;
	}
	ret = true;

err:
	WOW_FREE(colors);
	return ret;
}

static bool read_lights(struct wow_m2_file *file, struct wow_mpq_file *mpq)
{
	bool ret = false;
	struct wow_m2_light_int *lights = array_read(sizeof(*lights), &file->lights_nb, &file->header.lights, mpq);
	if (!lights)
		goto err;
	file->lights = WOW_MALLOC(sizeof(*file->lights) * file->lights_nb);
	if (!file->lights)
		goto err;
	memset(file->lights, 0, sizeof(*file->lights) * file->lights_nb);
	for (uint32_t i = 0; i < file->lights_nb; ++i)
	{
		struct wow_m2_light *light = &file->lights[i];
		light->type = lights[i].type;
		light->bone = lights[i].bone;
		light->position = lights[i].position;
		if (!read_track(&light->ambient_color, sizeof(struct wow_vec3f), &lights[i].ambient_color, mpq)
		 || !read_track(&light->ambient_intensity, sizeof(float), &lights[i].ambient_intensity, mpq)
		 || !read_track(&light->diffuse_color, sizeof(struct wow_vec3f), &lights[i].diffuse_color, mpq)
		 || !read_track(&light->diffuse_intensity, sizeof(float), &lights[i].diffuse_intensity, mpq)
		 || !read_track(&light->attenuation_start, sizeof(float), &lights[i].attenuation_start, mpq)
		 || !read_track(&light->attenuation_end, sizeof(float), &lights[i].attenuation_end, mpq)
		 || !read_track(&light->visibility, sizeof(uint8_t), &lights[i].visibility, mpq))
			goto err;
	}
	ret = true;

err:
	WOW_FREE(lights);
	return ret;
}

static bool read_bones(struct wow_m2_file *file, struct wow_mpq_file *mpq)
{
	bool ret = false;
	struct wow_m2_bone_int *bones = array_read(sizeof(*bones), &file->bones_nb, &file->header.bones, mpq);
	if (!bones)
		goto err;
	file->bones = WOW_MALLOC(sizeof(*file->bones) * file->bones_nb);
	if (!file->bones)
		goto err;
	memset(file->bones, 0, sizeof(*file->bones) * file->bones_nb);
	for (uint32_t i = 0; i < file->bones_nb; ++i)
	{
		struct wow_m2_bone *bone = &file->bones[i];
		bone->key_bone_id = bones[i].key_bone_id;
		bone->flags = bones[i].flags;
		bone->parent_bone = bones[i].parent_bone;
		bone->submesh_id = bones[i].submesh_id;
		bone->bone_name_crc = bones[i].bone_name_crc;
		bone->pivot = bones[i].pivot;
		if (!read_track(&bone->translation, sizeof(struct wow_vec3f), &bones[i].translation, mpq)
		 || !read_track(&bone->rotation, sizeof(struct wow_quats), &bones[i].rotation, mpq)
		 || !read_track(&bone->scale, sizeof(struct wow_vec3f), &bones[i].scale, mpq))
			goto err;
	}
	ret = true;

err:
	WOW_FREE(bones);
	return ret;
}

struct wow_m2_file *wow_m2_file_new(struct wow_mpq_file *mpq)
{
	struct wow_m2_file *file = WOW_MALLOC(sizeof(*file));
	if (!file)
		return NULL;
	memset(file, 0, sizeof(*file));
	if (wow_mpq_read(mpq, &file->header, sizeof(file->header)) != sizeof(file->header))
		goto err;
	file->materials = array_read(sizeof(*file->materials), &file->materials_nb, &file->header.materials, mpq);
	if (!file->materials)
		goto err;
	file->sequences = array_read(sizeof(*file->sequences), &file->sequences_nb, &file->header.sequences, mpq);
	if (!file->sequences)
		goto err;
	file->vertexes = array_read(sizeof(*file->vertexes), &file->vertexes_nb, &file->header.vertexes, mpq);
	if (!file->vertexes)
		goto err;
	file->texture_transforms_lookups = array_read(sizeof(*file->texture_transforms_lookups), &file->texture_transforms_lookups_nb, &file->header.texture_transforms_lookup_table, mpq);
	if (!file->texture_transforms_lookups)
		goto err;
	file->texture_weights_lookups = array_read(sizeof(*file->texture_weights_lookups), &file->texture_weights_lookups_nb, &file->header.texture_weights_lookup_table, mpq);
	if (!file->texture_weights_lookups)
		goto err;
	if (file->header.flags & WOW_M2_HEADER_FLAG_USE_TEXTURE_COMBINER_COMBO)
	{
		file->texture_combiner_combos = array_read(sizeof(*file->texture_combiner_combos), &file->texture_combiner_combos_nb, &file->header.texture_combiner_combos, mpq);
		if (!file->texture_combiner_combos)
			goto err;
	}
	file->texture_unit_lookups = array_read(sizeof(*file->texture_unit_lookups), &file->texture_unit_lookups_nb, &file->header.texture_unit_lookup_table, mpq);
	if (!file->texture_unit_lookups)
		goto err;
	file->attachment_lookups = array_read(sizeof(*file->attachment_lookups), &file->attachment_lookups_nb, &file->header.attachment_lookup_table, mpq);
	if (!file->attachment_lookups)
		goto err;
	file->key_bone_lookups = array_read(sizeof(*file->key_bone_lookups), &file->key_bone_lookups_nb, &file->header.key_bone_lookup_table, mpq);
	if (!file->key_bone_lookups)
		goto err;
	file->sequence_lookups = array_read(sizeof(*file->sequence_lookups), &file->sequence_lookups_nb, &file->header.sequence_lookup_table, mpq);
	if (!file->sequence_lookups)
		goto err;
	file->global_sequences = array_read(sizeof(*file->global_sequences), &file->global_sequences_nb, &file->header.global_sequences, mpq);
	if (!file->global_sequences)
		goto err;
	file->texture_lookups = array_read(sizeof(*file->texture_lookups), &file->texture_lookups_nb, &file->header.texture_lookup_table, mpq);
	if (!file->texture_lookups)
		goto err;
	file->camera_lookups = array_read(sizeof(*file->camera_lookups), &file->camera_lookups_nb, &file->header.camera_lookup_table, mpq);
	if (!file->camera_lookups)
		goto err;
	file->bone_lookups = array_read(sizeof(*file->bone_lookups), &file->bone_lookups_nb, &file->header.bone_lookup_table, mpq);
	if (!file->bone_lookups)
		goto err;
	file->playable_animations = array_read(sizeof(*file->playable_animations), &file->playable_animations_nb, &file->header.playable_animations, mpq);
	if (!file->playable_animations)
		goto err;
	file->collision_triangles = array_read(sizeof(*file->collision_triangles), &file->collision_triangles_nb, &file->header.collision_triangles, mpq);
	if (!file->collision_triangles)
		goto err;
	file->collision_vertexes = array_read(sizeof(*file->collision_vertexes), &file->collision_vertexes_nb, &file->header.collision_vertexes, mpq);
	if (!file->collision_vertexes)
		goto err;
	file->collision_normals = array_read(sizeof(*file->collision_normals), &file->collision_normals_nb, &file->header.collision_normals, mpq);
	if (!file->collision_normals)
		goto err;
	if (!read_texture_transforms(file, mpq))
		goto err;
	if (!read_texture_weights(file, mpq))
		goto err;
	if (!read_skin_profiles(file, mpq))
		goto err;
	if (!read_attachments(file, mpq))
		goto err;
	if (!read_particles(file, mpq))
		goto err;
	if (!read_textures(file, mpq))
		goto err;
	if (!read_cameras(file, mpq))
		goto err;
	if (!read_ribbons(file, mpq))
		goto err;
	if (!read_colors(file, mpq))
		goto err;
	if (!read_lights(file, mpq))
		goto err;
	if (!read_bones(file, mpq))
		goto err;
	file->name = array_read(sizeof(*file->name), NULL, &file->header.name, mpq);
	if (!file->name)
		goto err;
	return file;

err:
	wow_m2_file_delete(file);
	return NULL;
}

static bool track_dup(struct wow_m2_track *track, const struct wow_m2_track *dup, size_t data_size)
{
	track->interpolation_type = dup->interpolation_type;
	track->global_sequence = dup->global_sequence;
	track->interpolation_ranges_nb = dup->interpolation_ranges_nb;
	track->interpolation_ranges = WOW_MALLOC(sizeof(*track->interpolation_ranges) * track->interpolation_ranges_nb);
	if (!track->interpolation_ranges)
		return false;
	memcpy(track->interpolation_ranges, dup->interpolation_ranges, sizeof(*track->interpolation_ranges) * track->interpolation_ranges_nb);
	track->timestamps_nb = dup->timestamps_nb;
	track->timestamps = WOW_MALLOC(sizeof(*track->timestamps) * track->timestamps_nb);
	if (!track->timestamps)
		return false;
	memcpy(track->timestamps, dup->timestamps, sizeof(*track->timestamps) * track->timestamps_nb);
	track->values_nb = dup->values_nb;
	track->values = WOW_MALLOC(data_size * track->values_nb);
	if (!track->values)
		return false;
	memcpy(track->values, dup->values, data_size * track->values_nb);
	return true;
}

static void track_free(struct wow_m2_track *track)
{
	WOW_FREE(track->interpolation_ranges);
	WOW_FREE(track->timestamps);
	WOW_FREE(track->values);
}

void wow_m2_file_delete(struct wow_m2_file *file)
{
	if (!file)
		return;
	WOW_FREE(file->playable_animations);
	wow_m2_texture_transforms_delete(file->texture_transforms, file->texture_transforms_nb);
	wow_m2_texture_weights_delete(file->texture_weights, file->texture_weights_nb);
	for (uint32_t i = 0; i < file->skin_profiles_nb; ++i)
	{
		WOW_FREE(file->skin_profiles[i].sections);
		WOW_FREE(file->skin_profiles[i].batches);
		WOW_FREE(file->skin_profiles[i].vertexes);
		WOW_FREE(file->skin_profiles[i].indices);
		WOW_FREE(file->skin_profiles[i].bones);
	}
	WOW_FREE(file->skin_profiles);
	WOW_FREE(file->materials);
	wow_m2_attachments_delete(file->attachments, file->attachments_nb);
	wow_m2_sequences_delete(file->sequences, file->sequences_nb);
	wow_m2_particles_delete(file->particles, file->particles_nb);
	wow_m2_textures_delete(file->textures, file->textures_nb);
	WOW_FREE(file->vertexes);
	wow_m2_cameras_delete(file->cameras, file->cameras_nb);
	wow_m2_ribbons_delete(file->ribbons, file->ribbons_nb);
	wow_m2_colors_delete(file->colors, file->colors_nb);
	wow_m2_lights_delete(file->lights, file->lights_nb);
	wow_m2_bones_delete(file->bones, file->bones_nb);
	WOW_FREE(file->texture_transforms_lookups);
	WOW_FREE(file->texture_weights_lookups);
	WOW_FREE(file->texture_combiner_combos);
	WOW_FREE(file->texture_unit_lookups);
	WOW_FREE(file->collision_triangles);
	WOW_FREE(file->attachment_lookups);
	WOW_FREE(file->collision_vertexes);
	WOW_FREE(file->collision_normals);
	WOW_FREE(file->key_bone_lookups);
	WOW_FREE(file->sequence_lookups);
	WOW_FREE(file->global_sequences);
	WOW_FREE(file->texture_lookups);
	WOW_FREE(file->camera_lookups);
	WOW_FREE(file->bone_lookups);
	WOW_FREE(file->name);
	WOW_FREE(file);
}

struct wow_m2_texture_transform *wow_m2_texture_transforms_dup(const struct wow_m2_texture_transform *dup, uint32_t nb)
{
	if (!dup || !nb)
		return NULL;
	struct wow_m2_texture_transform *texture_transforms = WOW_MALLOC(sizeof(*texture_transforms) * nb);
	if (!texture_transforms)
		return NULL;
	memset(texture_transforms, 0, sizeof(*texture_transforms) * nb);
	for (uint32_t i = 0; i < nb; ++i)
	{
		struct wow_m2_texture_transform *texture_transform = &texture_transforms[i];
		if (!track_dup(&texture_transform->translation, &dup[i].translation, sizeof(struct wow_vec3f))
		 || !track_dup(&texture_transform->rotation, &dup[i].rotation, sizeof(struct wow_quatf))
		 || !track_dup(&texture_transform->scaling, &dup[i].scaling, sizeof(struct wow_vec3f)))
		{
			wow_m2_texture_transforms_delete(texture_transforms, i + 1);
			return NULL;
		}
	}
	return texture_transforms;
}

void wow_m2_texture_transforms_delete(struct wow_m2_texture_transform *val, uint32_t nb)
{
	if (!val)
		return;
	for (uint32_t i = 0; i < nb; ++i)
	{
		track_free(&val[i].translation);
		track_free(&val[i].rotation);
		track_free(&val[i].scaling);
	}
	WOW_FREE(val);
}

struct wow_m2_texture_weight *wow_m2_texture_weights_dup(const struct wow_m2_texture_weight *dup, uint32_t nb)
{
	if (!dup || !nb)
		return NULL;
	struct wow_m2_texture_weight *texture_weights = WOW_MALLOC(sizeof(*texture_weights) * nb);
	if (!texture_weights)
		return NULL;
	memset(texture_weights, 0, sizeof(*texture_weights) * nb);
	for (uint32_t i = 0; i < nb; ++i)
	{
		struct wow_m2_texture_weight *texture_weight = &texture_weights[i];
		if (!track_dup(&texture_weight->weight, &dup[i].weight, sizeof(int16_t)))
		{
			wow_m2_texture_weights_delete(texture_weights, i + 1);
			return NULL;
		}
	}
	return texture_weights;
}

void wow_m2_texture_weights_delete(struct wow_m2_texture_weight *val, uint32_t nb)
{
	if (!val)
		return;
	for (uint32_t i = 0; i < nb; ++i)
		track_free(&val[i].weight);
	WOW_FREE(val);
}

struct wow_m2_attachment *wow_m2_attachments_dup(const struct wow_m2_attachment *dup, uint32_t nb)
{
	if (!dup || !nb)
		return NULL;
	struct wow_m2_attachment *attachments = WOW_MALLOC(sizeof(*attachments) * nb);
	if (!attachments)
		return NULL;
	memset(attachments, 0, sizeof(*attachments));
	for (uint32_t i = 0; i < nb; ++i)
	{
		struct wow_m2_attachment *attachment = &attachments[i];
		attachment->id = dup[i].id;
		attachment->bone = dup[i].bone;
		attachment->unknown = dup[i].unknown;
		attachment->position = dup[i].position;
		if (!track_dup(&attachment->animate_attached, &dup[i].animate_attached, sizeof(uint8_t)))
			goto err;
	}
	return attachments;

err:
	WOW_FREE(attachments);
	return NULL;
}

void wow_m2_attachments_delete(struct wow_m2_attachment *val, uint32_t nb)
{
	if (!val)
		return;
	for (uint32_t i = 0; i < nb; ++i)
		track_free(&val[i].animate_attached);
	WOW_FREE(val);
}

struct wow_m2_sequence *wow_m2_sequences_dup(const struct wow_m2_sequence *dup, uint32_t nb)
{
	if (!dup || !nb)
		return NULL;
	struct wow_m2_sequence *sequences = WOW_MALLOC(sizeof(*sequences) * nb);
	if (!sequences)
		return NULL;
	memcpy(sequences, dup, sizeof(*sequences) * nb);
	return sequences;
}

void wow_m2_sequences_delete(struct wow_m2_sequence *val, uint32_t nb)
{
	(void)nb;
	if (!val)
		return;
	WOW_FREE(val);
}

struct wow_m2_particle *wow_m2_particles_dup(const struct wow_m2_particle *dup, uint32_t nb)
{
	if (!dup || !nb)
		return NULL;
	struct wow_m2_particle *particles = WOW_MALLOC(sizeof(*particles) * nb);
	if (!particles)
		return NULL;
	memset(particles, 0, sizeof(*particles) * nb);
	for (uint32_t i = 0; i < nb; ++i)
	{
		struct wow_m2_particle *particle = &particles[i];
		particle->id = dup[i].id;
		particle->flags = dup[i].flags;
		particle->position = dup[i].position;
		particle->bone = dup[i].bone;
		particle->texture = dup[i].texture;
		particle->geometry_model_filename = WOW_MALLOC(strlen(dup[i].geometry_model_filename) + 1);
		if (!particle->geometry_model_filename)
			goto err;
		strcpy(particle->geometry_model_filename, dup[i].geometry_model_filename);
		particle->recursion_model_filename = WOW_MALLOC(strlen(dup[i].recursion_model_filename) + 1);
		if (!particle->recursion_model_filename)
			goto err;
		strcpy(particle->recursion_model_filename, dup[i].recursion_model_filename);
		particle->blending_type = dup[i].blending_type;
		particle->emitter_type = dup[i].emitter_type;
		particle->particle_type = dup[i].particle_type;
		particle->head_or_tail = dup[i].head_or_tail;
		particle->texture_tile_rotation = dup[i].texture_tile_rotation;
		particle->texture_dimensions_rows = dup[i].texture_dimensions_rows;
		particle->texture_dimensions_columns = dup[i].texture_dimensions_columns;
		if (!track_dup(&particle->emission_speed, &dup[i].emission_speed, sizeof(float))
		 || !track_dup(&particle->speed_variation, &dup[i].speed_variation, sizeof(float))
		 || !track_dup(&particle->vertical_range, &dup[i].vertical_range, sizeof(float))
		 || !track_dup(&particle->horizontal_range, &dup[i].horizontal_range, sizeof(float))
		 || !track_dup(&particle->gravity, &dup[i].gravity, sizeof(float))
		 || !track_dup(&particle->lifespan, &dup[i].lifespan, sizeof(float))
		 || !track_dup(&particle->emission_rate, &dup[i].emission_rate, sizeof(float))
		 || !track_dup(&particle->emission_area_length, &dup[i].emission_area_length, sizeof(float))
		 || !track_dup(&particle->emission_area_width, &dup[i].emission_area_width, sizeof(float))
		 || !track_dup(&particle->z_source, &dup[i].z_source, sizeof(float)))
			goto err;
		particle->mid_point = dup[i].mid_point;
		memcpy(particle->color_values, dup[i].color_values, sizeof(particle->color_values));
		memcpy(particle->scale_values, dup[i].scale_values, sizeof(particle->scale_values));
		memcpy(particle->lifespan_uv_anim, dup[i].lifespan_uv_anim, sizeof(particle->lifespan_uv_anim));
		memcpy(particle->decay_uv_anim, dup[i].decay_uv_anim, sizeof(particle->decay_uv_anim));
		memcpy(particle->tail_uv_anim, dup[i].tail_uv_anim, sizeof(particle->tail_uv_anim));
		memcpy(particle->tail_decay_uv_anim, dup[i].tail_decay_uv_anim, sizeof(particle->tail_decay_uv_anim));
		particle->tail_length = dup[i].tail_length;
		particle->twinkle_speed = dup[i].twinkle_speed;
		particle->twinkle_percent = dup[i].twinkle_percent;
		particle->twinkle_scale_min = dup[i].twinkle_scale_min;
		particle->twinkle_scale_max = dup[i].twinkle_scale_max;
		particle->burst_multiplier = dup[i].burst_multiplier;
		particle->drag = dup[i].drag;
		particle->spin = dup[i].spin;
		particle->tumble = dup[i].tumble;
		particle->wind_vector = dup[i].wind_vector;
		particle->wind_time = dup[i].wind_time;
		particle->follow_speed1 = dup[i].follow_speed1;
		particle->follow_scale1 = dup[i].follow_scale1;
		particle->follow_speed2 = dup[i].follow_speed2;
		particle->follow_scale2 = dup[i].follow_scale2;
		particle->spline_points_nb = dup[i].spline_points_nb;
		particle->spline_points = WOW_MALLOC(sizeof(*particle->spline_points) * particle->spline_points_nb);
		if (!particle->spline_points)
			goto err;
		memcpy(particle->spline_points, dup[i].spline_points, sizeof(*particle->spline_points) * particle->spline_points_nb);
		if (!track_dup(&particle->enabled_in, &dup[i].enabled_in, sizeof(uint8_t)))
			goto err;
	}
	return particles;

err:
	WOW_FREE(particles);
	return NULL;
}

void wow_m2_particles_delete(struct wow_m2_particle *val, uint32_t nb)
{
	if (!val)
		return;
	for (uint32_t i = 0; i < nb; ++i)
	{
		WOW_FREE(val[i].geometry_model_filename);
		WOW_FREE(val[i].recursion_model_filename);
		track_free(&val[i].emission_speed);
		track_free(&val[i].speed_variation);
		track_free(&val[i].vertical_range);
		track_free(&val[i].horizontal_range);
		track_free(&val[i].gravity);
		track_free(&val[i].lifespan);
		track_free(&val[i].emission_rate);
		track_free(&val[i].emission_area_length);
		track_free(&val[i].emission_area_width);
		track_free(&val[i].z_source);
		WOW_FREE(val[i].spline_points);
		track_free(&val[i].enabled_in);
	}
	WOW_FREE(val);
}

struct wow_m2_texture *wow_m2_textures_dup(const struct wow_m2_texture *dup, uint32_t nb)
{
	if (!dup || !nb)
		return NULL;
	struct wow_m2_texture *textures = WOW_MALLOC(sizeof(*textures) * nb);
	if (!textures)
		return NULL;
	memset(textures, 0, sizeof(*textures) * nb);
	for (uint32_t i = 0; i < nb; ++i)
	{
		struct wow_m2_texture *texture = &textures[i];
		texture->type = dup[i].type;
		texture->flags = dup[i].flags;
		if (dup[i].filename)
		{
			size_t len = strlen(dup[i].filename);
			texture->filename = WOW_MALLOC(len + 1);
			if (!texture->filename)
				goto err;
			memcpy(texture->filename, dup[i].filename, len);
			texture->filename[len] = '\0';
		}
	}
	return textures;

err:
	WOW_FREE(textures);
	return NULL;
}

void wow_m2_textures_delete(struct wow_m2_texture *val, uint32_t nb)
{
	if (!val)
		return;
	for (uint32_t i = 0; i < nb; ++i)
	{
		WOW_FREE(val[i].filename);
	}
	WOW_FREE(val);
}

struct wow_m2_camera *wow_m2_cameras_dup(const struct wow_m2_camera *dup, uint32_t nb)
{
	if (!dup || !nb)
		return NULL;
	struct wow_m2_camera *cameras = WOW_MALLOC(sizeof(*cameras) * nb);
	if (!cameras)
		return NULL;
	memset(cameras, 0, sizeof(*cameras) * nb);
	for (uint32_t i = 0; i < nb; ++i)
	{
		struct wow_m2_camera *camera = &cameras[i];
		camera->type = dup[i].type;
		camera->fov = dup[i].fov;
		camera->far_clip = dup[i].far_clip;
		camera->near_clip = dup[i].near_clip;
		camera->position_base = dup[i].position_base;
		camera->target_position_base = dup[i].target_position_base;
		if (!track_dup(&camera->position, &dup[i].position, sizeof(struct wow_m2_spline_vec3f))
		 || !track_dup(&camera->target_position, &dup[i].target_position, sizeof(struct wow_m2_spline_vec3f))
		 || !track_dup(&camera->roll, &dup[i].roll, sizeof(struct wow_m2_spline_float)))
		{
			wow_m2_cameras_delete(cameras, i + 1);
			return NULL;
		}
	}
	return cameras;
}

void wow_m2_cameras_delete(struct wow_m2_camera *val, uint32_t nb)
{
	if (!val)
		return;
	for (uint32_t i = 0; i < nb; ++i)
	{
		track_free(&val[i].position);
		track_free(&val[i].target_position);
		track_free(&val[i].roll);
	}
	WOW_FREE(val);
}

struct wow_m2_ribbon *wow_m2_ribbons_dup(const struct wow_m2_ribbon *dup, uint32_t nb)
{
	if (!dup || !nb)
		return NULL;
	struct wow_m2_ribbon *ribbons = WOW_MALLOC(sizeof(*ribbons) * nb);
	if (!ribbons)
		return NULL;
	memset(ribbons, 0, sizeof(*ribbons) * nb);
	for (uint32_t i = 0; i < nb; ++i)
	{
		struct wow_m2_ribbon *ribbon = &ribbons[i];
		ribbon->ribbon_id = dup[i].ribbon_id;
		ribbon->bone_index = dup[i].bone_index;
		ribbon->position = dup[i].position;
		ribbon->edges_per_second = dup[i].edges_per_second;
		ribbon->edge_lifetime = dup[i].edge_lifetime;
		ribbon->gravity = dup[i].gravity;
		ribbon->texture_rows = dup[i].texture_rows;
		ribbon->texture_cols = dup[i].texture_cols;
		ribbon->texture_indices_nb = dup[i].texture_indices_nb;
		ribbon->texture_indices = WOW_MALLOC(sizeof(*ribbon->texture_indices) * ribbon->texture_indices_nb);
		if (!ribbon->texture_indices)
		{
			wow_m2_ribbons_delete(ribbons, i + 1);
			return NULL;
		}
		memcpy(ribbon->texture_indices, dup[i].texture_indices, sizeof(*ribbon->texture_indices) * ribbon->texture_indices_nb);
		ribbon->material_indices_nb = dup[i].material_indices_nb;
		ribbon->material_indices = WOW_MALLOC(sizeof(*ribbon->material_indices) * ribbon->material_indices_nb);
		if (!ribbon->material_indices)
		{
			wow_m2_ribbons_delete(ribbons, i + 1);
			return NULL;
		}
		memcpy(ribbon->material_indices, dup[i].material_indices, sizeof(*ribbon->material_indices) * ribbon->material_indices_nb);
		if (!track_dup(&ribbon->color, &dup[i].color, sizeof(struct wow_vec3f))
		 || !track_dup(&ribbon->alpha, &dup[i].alpha, sizeof(int16_t))
		 || !track_dup(&ribbon->height_above, &dup[i].height_above, sizeof(float))
		 || !track_dup(&ribbon->height_below, &dup[i].height_below, sizeof(float))
		 || !track_dup(&ribbon->tex_slot, &dup[i].tex_slot, sizeof(uint16_t))
		 || !track_dup(&ribbon->visibility, &dup[i].visibility, sizeof(uint8_t)))
		{
			wow_m2_ribbons_delete(ribbons, i + 1);
			return NULL;
		}
	}
	return ribbons;
}

void wow_m2_ribbons_delete(struct wow_m2_ribbon *val, uint32_t nb)
{
	if (!val)
		return;
	for (uint32_t i = 0; i < nb; ++i)
	{
		WOW_FREE(val[i].texture_indices);
		WOW_FREE(val[i].material_indices);
		track_free(&val[i].color);
		track_free(&val[i].alpha);
		track_free(&val[i].height_above);
		track_free(&val[i].height_below);
		track_free(&val[i].tex_slot);
		track_free(&val[i].visibility);
	}
	WOW_FREE(val);
}

struct wow_m2_color *wow_m2_colors_dup(const struct wow_m2_color *dup, uint32_t nb)
{
	if (!dup || !nb)
		return NULL;
	struct wow_m2_color *colors = WOW_MALLOC(sizeof(*colors) * nb);
	if (!colors)
		return NULL;
	memset(colors, 0, sizeof(*colors) * nb);
	for (uint32_t i = 0; i < nb; ++i)
	{
		struct wow_m2_color *color = &colors[i];
		if (!track_dup(&color->color, &dup[i].color, sizeof(struct wow_vec3f))
		 || !track_dup(&color->alpha, &dup[i].alpha, sizeof(int16_t)))
		{
			wow_m2_colors_delete(colors, i + 1);
			return NULL;
		}
	}
	return colors;
}

void wow_m2_colors_delete(struct wow_m2_color *val, uint32_t nb)
{
	if (!val)
		return;
	for (uint32_t i = 0; i < nb; ++i)
	{
		track_free(&val[i].color);
		track_free(&val[i].alpha);
	}
	WOW_FREE(val);
}

struct wow_m2_light *wow_m2_lights_dup(const struct wow_m2_light *dup, uint32_t nb)
{
	if (!dup || !nb)
		return NULL;
	struct wow_m2_light *lights = WOW_MALLOC(sizeof(*lights) * nb);
	if (!lights)
		return NULL;
	memset(lights, 0, sizeof(*lights) * nb);
	for (uint32_t i = 0; i < nb; ++i)
	{
		struct wow_m2_light *light = &lights[i];
		light->type = dup[i].type;
		light->bone = dup[i].bone;
		light->position = dup[i].position;
		if (!track_dup(&light->ambient_color, &dup[i].ambient_color, sizeof(struct wow_vec3f))
		 || !track_dup(&light->ambient_intensity, &dup[i].ambient_intensity, sizeof(float))
		 || !track_dup(&light->diffuse_color, &dup[i].diffuse_color, sizeof(struct wow_vec3f))
		 || !track_dup(&light->diffuse_intensity, &dup[i].diffuse_intensity, sizeof(float))
		 || !track_dup(&light->attenuation_start, &dup[i].attenuation_start, sizeof(float))
		 || !track_dup(&light->attenuation_end, &dup[i].attenuation_end, sizeof(float))
		 || !track_dup(&light->visibility, &dup[i].visibility, sizeof(uint8_t)))
		{
			wow_m2_lights_delete(lights, i + 1);
			return NULL;
		}
	}
	return lights;

}

void wow_m2_lights_delete(struct wow_m2_light *val, uint32_t nb)
{
	if (!val)
		return;
	for (uint32_t i = 0; i < nb; ++i)
	{
		track_free(&val[i].ambient_color);
		track_free(&val[i].ambient_intensity);
		track_free(&val[i].diffuse_color);
		track_free(&val[i].diffuse_intensity);
		track_free(&val[i].attenuation_start);
		track_free(&val[i].attenuation_end);
		track_free(&val[i].visibility);
	}
	WOW_FREE(val);
}

struct wow_m2_bone *wow_m2_bones_dup(const struct wow_m2_bone *dup, uint32_t nb)
{
	if (!dup || !nb)
		return NULL;
	struct wow_m2_bone *bones = WOW_MALLOC(sizeof(*bones) * nb);
	if (!bones)
		return NULL;
	memset(bones, 0, sizeof(*bones) * nb);
	for (uint32_t i = 0; i < nb; ++i)
	{
		struct wow_m2_bone *bone = &bones[i];
		bone->key_bone_id = dup[i].key_bone_id;
		bone->flags = dup[i].flags;
		bone->parent_bone = dup[i].parent_bone;
		bone->submesh_id = dup[i].submesh_id;
		bone->bone_name_crc = dup[i].bone_name_crc;
		bone->pivot = dup[i].pivot;
		if (!track_dup(&bone->translation, &dup[i].translation, sizeof(struct wow_vec3f))
		 || !track_dup(&bone->rotation, &dup[i].rotation, sizeof(struct wow_quats))
		 || !track_dup(&bone->scale, &dup[i].scale, sizeof(struct wow_vec3f)))
		{
			wow_m2_bones_delete(bones, i + 1);
			return NULL;
		}
	}
	return bones;
}

void wow_m2_bones_delete(struct wow_m2_bone *val, uint32_t nb)
{
	if (!val)
		return;
	for (uint32_t i = 0; i < nb; ++i)
	{
		track_free(&val[i].translation);
		track_free(&val[i].rotation);
		track_free(&val[i].scale);
	}
	WOW_FREE(val);
}
