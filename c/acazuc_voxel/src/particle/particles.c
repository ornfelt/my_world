#include "particle/particles.h"
#include "particle/particle.h"

#include "world/world.h"
#include "world/chunk.h"

#include "player/player.h"

#include "graphics.h"
#include "textures.h"
#include "shaders.h"
#include "voxel.h"
#include "log.h"

#include <gfx/device.h>

#include <math.h>

bool particles_init(struct particles *particles, struct chunk *chunk)
{
	jks_array_init(&particles->vertexes, sizeof(struct shader_particles_vertex), NULL, NULL);
	jks_array_init(&particles->indices, sizeof(uint32_t), NULL, NULL);
	particles->attributes_state = GFX_ATTRIBUTES_STATE_INIT();
	particles->vertexes_buffer = GFX_BUFFER_INIT();
	particles->indices_buffer = GFX_BUFFER_INIT();
	particles->uniform_buffer = GFX_BUFFER_INIT();
	particles->vertexes_size = 0;
	particles->indices_size = 0;
	particles->chunk = chunk;
	TAILQ_INIT(&particles->particles);
	return true;
}

void particles_destroy(struct particles *particles)
{
	struct particle *particle = TAILQ_FIRST(&particles->particles);
	while (particle)
	{
		struct particle *next = TAILQ_NEXT(particle, chain);
		entity_delete(&particle->entity);
		particle = next;
	}
	jks_array_destroy(&particles->vertexes);
	jks_array_destroy(&particles->indices);
	gfx_delete_buffer(g_voxel->device, &particles->uniform_buffer);
	gfx_delete_buffer(g_voxel->device, &particles->indices_buffer);
	gfx_delete_buffer(g_voxel->device, &particles->vertexes_buffer);
	gfx_delete_attributes_state(g_voxel->device, &particles->attributes_state);
}

void particles_tick(struct particles *particles)
{
	struct particle *particle = TAILQ_FIRST(&particles->particles);
	while (particle)
	{
		struct particle *next = TAILQ_NEXT(particle, chain);
		entity_tick(&particle->entity);
		if (particle->entity.deleted)
		{
			TAILQ_REMOVE(&particles->particles, particle, chain);
			entity_delete(&particle->entity);
		}
		particle = next;
	}
}

void particles_draw(struct particles *particles)
{
	if (TAILQ_EMPTY(&particles->particles))
		return;
	jks_array_resize(&particles->vertexes, 0);
	jks_array_resize(&particles->indices, 0);
	struct mat4f tmp1;
	struct mat4f tmp2;
	MAT4_IDENTITY(tmp1);
	MAT4_ROTATEZ(float, tmp2, tmp1, -particles->chunk->world->player->entity.rot.z / 180. * M_PI);
	MAT4_ROTATEY(float, tmp1, tmp2, -particles->chunk->world->player->entity.rot.y / 180. * M_PI);
	MAT4_ROTATEX(float, tmp2, tmp1, -particles->chunk->world->player->entity.rot.x / 180. * M_PI);
	struct particle *particle;
	TAILQ_FOREACH(particle, &particles->particles, chain)
		particle_draw(particle, &particles->vertexes, &particles->indices, &tmp2);
	if (!particles->indices.size)
		return;
	bool rebuilt = false;
	if (particles->vertexes.size > particles->vertexes_size)
	{
		particles->vertexes_size = npot32(particles->vertexes.size);
		gfx_delete_buffer(g_voxel->device, &particles->vertexes_buffer);
		gfx_create_buffer(g_voxel->device, &particles->vertexes_buffer, GFX_BUFFER_VERTEXES, NULL, particles->vertexes_size * sizeof(struct shader_particles_vertex), GFX_BUFFER_STREAM);
		rebuilt = true;
	}
	gfx_set_buffer_data(&particles->vertexes_buffer, particles->vertexes.data, particles->vertexes.size * sizeof(struct shader_particles_vertex), 0);
	if (particles->indices.size > particles->indices_size)
	{
		particles->indices_size = npot32(particles->indices.size);
		gfx_delete_buffer(g_voxel->device, &particles->indices_buffer);
		gfx_create_buffer(g_voxel->device, &particles->indices_buffer, GFX_BUFFER_INDICES, NULL, particles->indices_size * sizeof(uint32_t), GFX_BUFFER_STREAM);
		rebuilt = true;
	}
	gfx_set_buffer_data(&particles->indices_buffer, particles->indices.data, particles->indices.size * sizeof(uint32_t), 0);
	if (rebuilt)
	{
		const struct gfx_attribute_bind binds[] =
		{
			{&particles->vertexes_buffer},
		};
		gfx_delete_attributes_state(g_voxel->device, &particles->attributes_state);
		particles->attributes_state = GFX_ATTRIBUTES_STATE_INIT();
		gfx_create_attributes_state(g_voxel->device, &particles->attributes_state, binds, sizeof(binds) / sizeof(*binds), &particles->indices_buffer, GFX_INDEX_UINT32);
	}
	struct shader_particles_model_block model_block;
	model_block.mvp = particles->chunk->world->player->mat_vp;
	model_block.mv = particles->chunk->world->player->mat_v;
	model_block.fog_color = particles->chunk->world->fog_color;
	model_block.fog_distance = particles->chunk->world->fog_distance;
	model_block.fog_density = particles->chunk->world->fog_density;
	if (!particles->uniform_buffer.handle.ptr)
		gfx_create_buffer(g_voxel->device, &particles->uniform_buffer, GFX_BUFFER_UNIFORM, NULL, sizeof(model_block), GFX_BUFFER_STREAM);
	gfx_set_buffer_data(&particles->uniform_buffer, &model_block, sizeof(model_block), 0);
	gfx_bind_pipeline_state(g_voxel->device, &g_voxel->graphics->particles.pipeline_state);
	gfx_bind_attributes_state(g_voxel->device, &particles->attributes_state, &g_voxel->graphics->particles.input_layout);
	gfx_bind_constant(g_voxel->device, 1, &particles->uniform_buffer, sizeof(model_block), 0);
	texture_bind(&g_voxel->textures->terrain);
	gfx_draw_indexed(g_voxel->device, particles->indices.size, 0);
}

void particles_add(struct particles *particles, struct particle *particle)
{
	TAILQ_INSERT_TAIL(&particles->particles, particle, chain);
}

void particles_remove(struct particles *particles, struct particle *particle)
{
	TAILQ_REMOVE(&particles->particles, particle, chain);
}
