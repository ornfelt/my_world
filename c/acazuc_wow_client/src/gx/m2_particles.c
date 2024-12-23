#include "gx/m2_particles.h"
#include "gx/skybox.h"
#include "gx/frame.h"
#include "gx/blp.h"
#include "gx/m2.h"

#include "map/map.h"

#include "graphics.h"
#include "shaders.h"
#include "camera.h"
#include "memory.h"
#include "cache.h"
#include "log.h"
#include "wow.h"

#include <jks/mat4.h>
#include <jks/vec3.h>

#include <gfx/device.h>

#include <wow/mpq.h>
#include <wow/m2.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

MEMORY_DECL(GX);

static void destroy_emitter(void *ptr)
{
	struct gx_m2_particles_emitter *emitter = ptr;
	gx_blp_free(emitter->texture);
	for (size_t i = 0; i < RENDER_FRAMES_COUNT; ++i)
	{
		struct gx_m2_particles_emitter_frame *emitter_frame = &emitter->frames[i];
		gfx_delete_attributes_state(g_wow->device, &emitter_frame->attributes_state);
		gfx_delete_buffer(g_wow->device, &emitter_frame->vertexes_buffer);
		gfx_delete_buffer(g_wow->device, &emitter_frame->uniform_buffer);
		jks_array_destroy(&emitter_frame->vertexes);
	}
	jks_array_destroy(&emitter->particles);
}

struct gx_m2_particles *gx_m2_particles_new(struct gx_m2_instance *parent)
{
	struct gx_m2_particles *particles = mem_malloc(MEM_GX, sizeof(*particles));
	if (!particles)
	{
		LOG_ERROR("malloc failed");
		return NULL;
	}
	particles->parent = parent;
	jks_array_init(&particles->emitters, sizeof(struct gx_m2_particles_emitter), destroy_emitter, &jks_array_memory_fn_GX);
	if (!jks_array_reserve(&particles->emitters, parent->parent->particles_nb))
	{
		LOG_ERROR("failed to reserve particle emitters");
		goto err;
	}
	for (size_t i = 0; i < parent->parent->particles_nb; ++i)
	{
		struct gx_m2_particles_emitter *emitter = jks_array_grow(&particles->emitters, 1);
		if (!emitter)
		{
			LOG_ERROR("failed to grow emitter array");
			goto err;
		}
		for (size_t j = 0; j < RENDER_FRAMES_COUNT; ++j)
		{
			struct gx_m2_particles_emitter_frame *emitter_frame = &emitter->frames[j];
			emitter_frame->attributes_state = GFX_ATTRIBUTES_STATE_INIT();
			emitter_frame->vertexes_buffer = GFX_BUFFER_INIT();
			emitter_frame->uniform_buffer = GFX_BUFFER_INIT();
			jks_array_init(&emitter_frame->vertexes, sizeof(struct shader_particle_input), NULL, &jks_array_memory_fn_GX);
		}
		struct wow_m2_particle *particle = &parent->parent->particles[i];
		struct wow_m2_texture *texture = &parent->parent->textures[particle->texture];
		if (!texture->type)
		{
			char path[256];
			snprintf(path, sizeof(path), "%s", texture->filename);
			wow_mpq_normalize_blp_fn(path, sizeof(path));
			if (!cache_ref_blp(g_wow->cache, path, &emitter->texture))
			{
				LOG_ERROR("failed to get texture %s", path);
				emitter->texture = NULL;
			}
			gx_blp_ask_load(emitter->texture);
		}
		else
		{
			LOG_ERROR("unsupported texture type for particle: %d", (int)texture->type);
			emitter->texture = NULL;
		}
		emitter->emitter = particle;
		emitter->last_spawned = g_wow->frametime;
		jks_array_init(&emitter->particles, sizeof(struct gx_m2_particle), NULL, &jks_array_memory_fn_GX);
		uint8_t blending_type;
		if (parent->parent->version >= 262)
		{
			blending_type = emitter->emitter->blending_type2;
			emitter->emitter_type = emitter->emitter->emitter_type2;
		}
		else
		{
			blending_type = emitter->emitter->blending_type;
			emitter->emitter_type = emitter->emitter->emitter_type;
		}
		enum world_blend_state blend_state;
		switch (blending_type)
		{
			case 0:
				blend_state = WORLD_BLEND_OPAQUE;
				emitter->alpha_test = 0.0;
				emitter->fog_override = false;
				break;
			case 1:
				blend_state = WORLD_BLEND_OPAQUE;
				emitter->alpha_test = 224.0 / 255.0;
				emitter->fog_override = false;
				break;
			case 2:
				blend_state = WORLD_BLEND_ALPHA;
				emitter->alpha_test = 1.0 / 255.0;
				emitter->fog_override = false;
				break;
			case 3:
				blend_state = WORLD_BLEND_NO_ALPHA_ADD;
				emitter->alpha_test = 1.0 / 255.0;
				emitter->fog_override = true;
				VEC3_SETV(emitter->fog_color, 0);
				break;
			case 4:
				blend_state = WORLD_BLEND_ADD;
				emitter->alpha_test = 1.0 / 255.0;
				emitter->fog_override = true;
				VEC3_SETV(emitter->fog_color, 0);
				break;
			default:
				LOG_INFO("unsupported blending: %d", (int)emitter->emitter->blending_type);
				blend_state = WORLD_BLEND_ALPHA;
				emitter->alpha_test = 0.0;
				emitter->fog_override = false;
				break;
		}
		emitter->pipeline_state = blend_state;
	}
	particles->initialized = false;
	return particles;

err:
	gx_m2_particles_delete(particles);
	return NULL;
}

void gx_m2_particles_delete(struct gx_m2_particles *particles)
{
	if (!particles)
		return;
	jks_array_destroy(&particles->emitters);
	mem_free(MEM_GX, particles);
}

static struct vec3f get_gravity(struct gx_m2_particles *particles, struct wow_m2_particle *emitter)
{
	struct vec3f gravity;
	if (emitter->flags & WOW_M2_PARTICLE_FLAG_COMP_GRAV)
	{
		/* XXX */
		VEC3_SET(gravity, 0, -((float*)emitter->gravity.values)[0], 0);
	}
	else
	{
		float g;
		if (!m2_instance_get_track_value_float(particles->parent, &emitter->gravity, &g))
			g = 0;
		VEC3_SET(gravity, 0, -g, 0);
	}
	return gravity;
}

static void update_position(struct gx_m2_particles *particles, struct wow_m2_particle *emitter, struct gx_m2_particle *particle, float dt)
{
	struct vec3f tmp;
	struct vec3f gravity = get_gravity(particles, emitter);
	VEC3_MULV(tmp, gravity, .5);
	VEC3_MULV(tmp, tmp, dt);
	VEC3_ADD(tmp, tmp, particle->velocity);
	VEC3_MULV(tmp, tmp, dt);
	VEC3_ADD(particle->position, particle->position, tmp);
	VEC3_MULV(tmp, gravity, dt);
	VEC3_ADD(particle->velocity, particle->velocity, tmp);
	float drag = 1 - (emitter->drag * dt);
	VEC3_MULV(particle->velocity, particle->velocity, drag);
	particle->position.w = 1;
}

static void update_particles(struct gx_m2_particles *particles, struct gx_m2_particles_emitter *emitter, struct gx_frame *frame, struct gx_m2_render_params *params)
{
	float dt = frame->dt / 1000000000.0f;
	for (size_t i = 0; i < emitter->particles.size; ++i)
	{
		struct gx_m2_particle *particle = JKS_ARRAY_GET(&emitter->particles, i, struct gx_m2_particle);
		if (frame->time - particle->created > particle->lifespan)
		{
			jks_array_erase(&emitter->particles, i);
			i--;
			continue;
		}
		update_position(particles, emitter->emitter, particle, dt);
		if ((frame->time - particle->created) / 1000000000.0f < emitter->emitter->wind_time)
		{
			struct vec4f wind;
			VEC3_MULV(wind, emitter->emitter->wind_vector, dt);
			wind.w = 0;
			struct vec4f tmp;
#if 0
			if (emitter->emitter->bone != (uint16_t)-1 && emitter->emitter->bone < particles->parent->parent->bones_nb)
			{
				struct mat4f *bone_mat = JKS_ARRAY_GET(&particles->parent->render_frames[frame->id].bone_mats, emitter->emitter->bone, struct mat4f);
				struct vec4f tmp2;
				MAT4_VEC4_MUL(tmp2, *bone_mat, wind);
				wind = tmp2;
			}
			MAT4_VEC4_MUL(tmp, particles->parent->m, wind);
#else
			tmp = wind;
#endif
			VEC3_SUB(particle->position, particle->position, tmp);
		}
	}
	struct gx_m2_particles_emitter_frame *emitter_frame = &emitter->frames[frame->id];
	if (!jks_array_resize(&emitter_frame->vertexes, emitter->particles.size * 4))
	{
		jks_array_resize(&emitter->particles, emitter->particles.size - 1);
		LOG_ERROR("failed to grow vertexes array");
		return;
	}
	for (size_t i = 0; i < emitter->particles.size; ++i)
	{
		struct gx_m2_particle *particle = JKS_ARRAY_GET(&emitter->particles, i, struct gx_m2_particle);
		struct shader_particle_input *vertexes = JKS_ARRAY_GET(&emitter_frame->vertexes, i * 4, struct shader_particle_input);
		float lifetime = (frame->time - particle->created) / (float)particle->lifespan;
		struct vec4f color1 = {1, 1, 1, 1};
		struct vec4f color2 = {1, 1, 1, 1};
		float scale1;
		float scale2;
		float a = 1;
		float scale = 1;
		vertexes[0].position = particle->position;
		vertexes[1].position = particle->position;
		vertexes[2].position = particle->position;
		vertexes[3].position = particle->position;
		if (emitter->emitter->twinkle_percent != 0 && emitter->emitter->twinkle_scale_min != emitter->emitter->twinkle_scale_max)
		{
			float len = 1000000000.0f / emitter->emitter->twinkle_speed * 2;
			float p = fmod(frame->time - particle->created, len) / len;
			if (p <= .5)
				scale *= (2 * p) * (emitter->emitter->twinkle_scale_max - emitter->emitter->twinkle_scale_min) + emitter->emitter->twinkle_scale_min;
			else
				scale *= (2 * (p - .5)) * (emitter->emitter->twinkle_scale_min - emitter->emitter->twinkle_scale_max) + emitter->emitter->twinkle_scale_max;
		}
		if (lifetime <= emitter->emitter->mid_point)
		{
			a = lifetime / emitter->emitter->mid_point;
			VEC4_DIVV(color1, emitter->emitter->color_values[0], 255.);
			VEC4_DIVV(color2, emitter->emitter->color_values[1], 255.);
			scale1 = emitter->emitter->scale_values[0];
			scale2 = emitter->emitter->scale_values[1];
		}
		else
		{
			a = (lifetime - emitter->emitter->mid_point) / (1 - emitter->emitter->mid_point);
			VEC4_DIVV(color1, emitter->emitter->color_values[1], 255.);
			VEC4_DIVV(color2, emitter->emitter->color_values[2], 255.);
			scale1 = emitter->emitter->scale_values[1];
			scale2 = emitter->emitter->scale_values[2];
		}
		struct vec4f tmp1;
		struct vec4f tmp2;
		VEC4_SUB(tmp1, color2, color1);
		VEC4_MULV(tmp2, tmp1, a);
		VEC4_ADD(vertexes[0].color, tmp2, color1);
		if (!(emitter->emitter->flags & WOW_M2_PARTICLE_FLAG_UNLIT))
		{
			VEC3_CPY(tmp1, g_wow->map->gx_skybox->int_values[SKYBOX_INT_AMBIENT]);
			VEC3_ADD(tmp1, tmp1, g_wow->map->gx_skybox->int_values[SKYBOX_INT_DIFFUSE]);
			float t;
			t = tmp1.x;
			tmp1.x = tmp1.z;
			tmp1.z = t;
			VEC3_MUL(vertexes[0].color, vertexes[0].color, tmp1);
		}
		VEC4_CPY(vertexes[1].color, vertexes[0].color);
		VEC4_CPY(vertexes[2].color, vertexes[0].color);
		VEC4_CPY(vertexes[3].color, vertexes[0].color);
		scale *= scale1 + (scale2 - scale1) * a;
		scale *= particles->parent->scale;
		if (emitter->emitter->particle_type > 0)
		{
			struct vec4f right;
			struct vec4f bot;
			VEC3_MULV(right, params->view_right, scale);
			VEC3_MULV(bot, params->view_bottom, scale);
			struct vec4f p1 = particle->position;
			struct vec4f p2;
			VEC3_MULV(p2, particle->velocity, -emitter->emitter->tail_length);
			VEC3_ADD(p2, p2, particle->position);
			p2.w = 1;
			VEC3_SUB(vertexes[0].position, p1, right);
			VEC3_SUB(vertexes[0].position, vertexes[0].position, bot);
			VEC3_ADD(vertexes[1].position, p1, right);
			VEC3_SUB(vertexes[1].position, vertexes[1].position, bot);
			VEC3_ADD(vertexes[2].position, p2, right);
			VEC3_ADD(vertexes[2].position, vertexes[2].position, bot);
			VEC3_SUB(vertexes[3].position, p2, right);
			VEC3_ADD(vertexes[3].position, vertexes[3].position, bot);
		}
		else
		{
			struct vec4f right;
			struct vec4f bot;
			VEC3_MULV(right, params->view_right, scale);
			VEC3_MULV(bot, params->view_bottom, scale);
			if (emitter->emitter->spin != 0)
			{
				float t = emitter->emitter->spin * lifetime + particle->spin_random + M_PI * .25;
				float c = cosf(t) * 1.4142;
				float s = sinf(t) * 1.4142;
				struct vec3f cx;
				struct vec3f cy;
				struct vec3f sx;
				struct vec3f sy;

				VEC3_MULV(cx, right, c);
				VEC3_MULV(cy, bot, c);
				VEC3_MULV(sx, right, s);
				VEC3_MULV(sy, bot, s);

				VEC3_SUB(vertexes[0].position, particle->position, sx);
				VEC3_SUB(vertexes[0].position, vertexes[0].position, cy);
				VEC3_ADD(vertexes[1].position, particle->position, cx);
				VEC3_SUB(vertexes[1].position, vertexes[1].position, sy);
				VEC3_ADD(vertexes[2].position, particle->position, sx);
				VEC3_ADD(vertexes[2].position, vertexes[2].position, cy);
				VEC3_SUB(vertexes[3].position, particle->position, cx);
				VEC3_ADD(vertexes[3].position, vertexes[3].position, sy);
			}
			else
			{
				VEC3_SUB(vertexes[0].position, particle->position, right);
				VEC3_SUB(vertexes[0].position, vertexes[0].position, bot);
				VEC3_ADD(vertexes[1].position, particle->position, right);
				VEC3_SUB(vertexes[1].position, vertexes[1].position, bot);
				VEC3_ADD(vertexes[2].position, particle->position, right);
				VEC3_ADD(vertexes[2].position, vertexes[2].position, bot);
				VEC3_SUB(vertexes[3].position, particle->position, right);
				VEC3_ADD(vertexes[3].position, vertexes[3].position, bot);
			}
		}
		float u = 1.f / emitter->emitter->texture_dimensions_columns;
		float v = 1.f / emitter->emitter->texture_dimensions_rows;
#if 0
		switch (emitter->emitter->texture_tile_rotation)
		{
			case -1:
				VEC2_SET(vertexes[3].uv, 0, 0);
				VEC2_SET(vertexes[0].uv, u, 0);
				VEC2_SET(vertexes[1].uv, u, v);
				VEC2_SET(vertexes[2].uv, 0, v);
				break;
			default:
#if 0
				LOG_WARN("unknown particle tile rotation: %u", emitter->emitter->texture_tile_rotation);
#endif
				/* FALLTHROUGH */
			case 0:
#endif
				VEC2_SET(vertexes[0].uv, 0, 0);
				VEC2_SET(vertexes[1].uv, u, 0);
				VEC2_SET(vertexes[2].uv, u, v);
				VEC2_SET(vertexes[3].uv, 0, v);
#if 0
				break;
			case 1:
				VEC2_SET(vertexes[1].uv, 0, 0);
				VEC2_SET(vertexes[2].uv, u, 0);
				VEC2_SET(vertexes[3].uv, u, v);
				VEC2_SET(vertexes[0].uv, 0, v);
				break;
		}
#endif
	}
}

static void create_particle(struct gx_m2_particles *particles, struct gx_m2_particles_emitter *emitter, struct gx_frame *frame)
{
	struct gx_m2_particle *particle = jks_array_grow(&emitter->particles, 1);
	if (!particle)
	{
		LOG_ERROR("failed to grow particle array");
		return;
	}
	particle->spin_random = rand() / (float)RAND_MAX * M_PI * 2;
	float speed;
	if (!m2_instance_get_track_value_float(particles->parent, &emitter->emitter->emission_speed, &speed))
		speed = 0;
	float speed_variation;
	if (!m2_instance_get_track_value_float(particles->parent, &emitter->emitter->speed_variation, &speed_variation))
		speed_variation = 0;
	float vertical_range;
	if (!m2_instance_get_track_value_float(particles->parent, &emitter->emitter->vertical_range, &vertical_range))
		vertical_range = 0;
	float horizontal_range;
	if (!m2_instance_get_track_value_float(particles->parent, &emitter->emitter->horizontal_range, &horizontal_range))
		horizontal_range = 0;
	float z_source;
	if (!m2_instance_get_track_value_float(particles->parent, &emitter->emitter->z_source, &z_source))
		z_source = 0;
	float area_length;
	if (!m2_instance_get_track_value_float(particles->parent, &emitter->emitter->emission_area_length, &area_length))
		area_length = 0;
	float area_width;
	if (!m2_instance_get_track_value_float(particles->parent, &emitter->emitter->emission_area_width, &area_width))
		area_width = 0;
	struct vec4f position;
	struct vec4f velocity;
	switch (emitter->emitter_type)
	{
		default:
			LOG_INFO("unhandled emitter type: %d", emitter->emitter_type);
			return;
			/* FALLTHROUGH */
		case 3: /* spline */
			/* XXX */
			return;
		case 1: /* plane */
		{
			VEC4_SET(position, (rand() / (float)RAND_MAX - 0.5f) * area_width, 0, ((rand() / (float)RAND_MAX - 0.5f) * area_length), 1);
			VEC3_ADD(position, position, emitter->emitter->position);
			float polar = (rand() / (float)RAND_MAX - 0.5f) * 2 * vertical_range;
			float azimuth = (rand() / (float)RAND_MAX - 0.5f) * 2 * horizontal_range;
			float pc = cosf(polar);
			float ps = sinf(polar);
			float ac = cosf(azimuth);
			float as = sinf(azimuth);
			float spd = speed * (1 + speed_variation * (rand() / (float)RAND_MAX - 0.5f) * 2);
			VEC4_SET(velocity, ac * ps, pc, -as * ps, 0);
			VEC3_MULV(velocity, velocity, spd);
			break;
		}
		case 2: /* sphere */
		{
			float radius = area_length + (area_width - area_length) * (rand() / (float)RAND_MAX);
			float polar = (rand() / (float)RAND_MAX - 0.5f) * 2 * vertical_range;
			float azimuth = (rand() / (float)RAND_MAX - 0.5f) * 2 * horizontal_range;
			float pc = cosf(polar);
			float ps = sinf(polar);
			float ac = cosf(azimuth);
			float as = sinf(azimuth);
			VEC4_SET(position, as * pc, ps, ac * pc, 1);
			VEC3_CPY(velocity, position);
			float spd = speed * (1 + speed_variation * (rand() / (float)RAND_MAX - 0.5f) * 2);
			VEC3_MULV(velocity, velocity, spd);
			velocity.w = 0;
			VEC3_MULV(position, position, radius);
			VEC3_ADD(position, position, emitter->emitter->position);
			break;
		}
	}
	if (emitter->emitter->flags & WOW_M2_PARTICLE_FLAG_BURST)
		VEC3_MULV(velocity, velocity, emitter->emitter->burst_multiplier);
	if (emitter->emitter->bone != (uint16_t)-1 && emitter->emitter->bone < particles->parent->parent->bones_nb)
	{
		struct mat4f *bone_mat = JKS_ARRAY_GET(&particles->parent->frames[frame->id].bone_mats, emitter->emitter->bone, struct mat4f);
		struct vec4f tmp;
		MAT4_VEC4_MUL(tmp, *bone_mat, velocity);
		velocity = tmp;
		MAT4_VEC4_MUL(tmp, *bone_mat, position);
		position = tmp;
	}
	MAT4_VEC4_MUL(particle->position, particles->parent->m, position);
	MAT4_VEC4_MUL(particle->velocity, particles->parent->m, velocity);
	particle->created = frame->time;
	float lifespan;
	if (!m2_instance_get_track_value_float(particles->parent, &emitter->emitter->lifespan, &lifespan))
		lifespan = 0;
	particle->lifespan = lifespan * 1000000000;
}

void gx_m2_particles_update(struct gx_m2_particles *particles, struct gx_frame *frame, struct gx_m2_render_params *params)
{
	for (size_t i = 0; i < particles->emitters.size; ++i)
	{
		struct gx_m2_particles_emitter *emitter = JKS_ARRAY_GET(&particles->emitters, i, struct gx_m2_particles_emitter);
		if (emitter->emitter_type == 3)
			continue;
		float rate;
		if (!m2_instance_get_track_value_float(particles->parent, &emitter->emitter->emission_rate, &rate))
			rate = 0;
		if (rate > 0)
		{
			uint64_t interval = 1000000000.0f / rate;
			size_t j = 0;
			while (frame->time - emitter->last_spawned > interval)
			{
				if (j == 1 || emitter->particles.size >= MAX_PARTICLES)
				{
					/* skip if too much particles must be created (avoiding freeze) */
					emitter->last_spawned = frame->time;
					break;
				}
				create_particle(particles, emitter, frame);
				emitter->last_spawned += interval;
				++j;
			}
		}
		update_particles(particles, emitter, frame, params);
	}
}

static void render_emitter(struct gx_m2_particles_emitter *emitter, struct gx_frame *frame, struct gx_m2_render_params *params)
{
	if (!emitter->particles.size)
		return;
	struct gx_m2_particles_emitter_frame *emitter_frame = &emitter->frames[frame->id];
	gfx_set_buffer_data(&emitter_frame->vertexes_buffer, emitter_frame->vertexes.data, sizeof(struct shader_particle_input) * emitter_frame->vertexes.size, 0);
	gfx_bind_attributes_state(g_wow->device, &emitter_frame->attributes_state, &g_wow->graphics->particles_input_layout);
	gfx_bind_pipeline_state(g_wow->device, &g_wow->graphics->particles_pipeline_states[emitter->pipeline_state]);
	struct shader_particle_model_block model_block;
	model_block.alpha_test = emitter->alpha_test;
	model_block.mvp = params->vp;
	model_block.mv = params->v;
	if (emitter->fog_override)
		VEC3_CPY(model_block.fog_color, emitter->fog_color);
	else
		VEC3_CPY(model_block.fog_color, g_wow->map->gx_skybox->int_values[SKYBOX_INT_FOG]);
	gfx_set_buffer_data(&emitter_frame->uniform_buffer, &model_block, sizeof(model_block), 0);
	gfx_bind_constant(g_wow->device, 1, &emitter_frame->uniform_buffer, sizeof(model_block), 0);
	gx_blp_bind(emitter->texture, 0);
	gfx_draw_indexed(g_wow->device, emitter->particles.size * 6, 0);
}

static void initialize(struct gx_m2_particles *particles)
{
	for (size_t i = 0; i < particles->emitters.size; ++i)
	{
		struct gx_m2_particles_emitter *emitter = JKS_ARRAY_GET(&particles->emitters, i, struct gx_m2_particles_emitter);
		for (size_t j = 0; j < RENDER_FRAMES_COUNT; ++j)
		{
			struct gx_m2_particles_emitter_frame *emitter_frame = &emitter->frames[j];
			const struct gfx_attribute_bind binds[] =
			{
				{&emitter_frame->vertexes_buffer},
			};
			gfx_create_buffer(g_wow->device, &emitter_frame->vertexes_buffer, GFX_BUFFER_VERTEXES, NULL, sizeof(struct shader_particle_input) * MAX_PARTICLES * 4, GFX_BUFFER_STREAM);
			gfx_create_buffer(g_wow->device, &emitter_frame->uniform_buffer, GFX_BUFFER_UNIFORM, NULL, sizeof(struct shader_particle_model_block), GFX_BUFFER_STREAM);
			gfx_create_attributes_state(g_wow->device, &emitter_frame->attributes_state, binds, sizeof(binds) / sizeof(*binds), &g_wow->map->particles_indices_buffer, GFX_INDEX_UINT16);
		}
	}
	particles->initialized = true;
}

void gx_m2_particles_render(struct gx_m2_particles *particles, struct gx_frame *frame, struct gx_m2_render_params *params)
{
	if (!particles->initialized)
		initialize(particles);
	for (size_t i = 0; i < particles->emitters.size; ++i)
		render_emitter(JKS_ARRAY_GET(&particles->emitters, i, struct gx_m2_particles_emitter), frame, params);
}
