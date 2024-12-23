#ifndef SHADERS_H
#define SHADERS_H

#include <gfx/objects.h>

#include <jks/mat4.h>
#include <jks/vec3.h>
#include <jks/vec2.h>
#include <jks/vec1.h>

struct shader_particles_model_block
{
	struct mat4f mvp;
	struct mat4f mv;
	struct vec4f fog_color;
	float fog_distance;
	float fog_density;
};

struct shader_particles_vertex
{
	struct vec4f position;
	struct vec4f color;
	struct vec2f uv;
};

struct shader_breaking_model_block
{
	struct mat4f mvp;
	struct mat4f mv;
	struct vec4f fog_color;
	float fog_distance;
	float fog_density;
};

struct shader_breaking_vertex
{
	struct vec3f position;
	struct vec2f uv;
};

struct shader_focused_model_block
{
	struct mat4f mvp;
};

struct shader_focused_vertex
{
	struct vec3f position;
};

struct shader_sunmoon_model_block
{
	struct mat4f mvp;
};

struct shader_sunmoon_vertex
{
	struct vec3f position;
	struct vec3f color;
	struct vec2f uv;
};

struct shader_entity_mesh_block
{
	struct mat4f mvp;
	struct mat4f mv;
};

struct shader_entity_model_block
{
	struct vec4f color;
};

struct shader_entity_scene_block
{
	struct vec4f fog_color;
	float fog_distance;
	float fog_density;
};

struct shader_entity_vertex
{
	struct vec3f position;
	struct vec2f uv;
};

struct shader_skybox_model_block
{
	struct mat4f mvp;
};

struct shader_skybox_vertex
{
	struct vec3f position;
	struct vec3f color;
};

struct shader_blocks_model_block
{
	struct mat4f mvp;
	struct mat4f mv;
	struct vec4f fog_color;
	float fog_distance;
	float fog_density;
	float time_factor;
	int32_t disable_tex;
};

struct shader_blocks_vertex
{
	struct vec3f position;
	struct vec3f color;
	struct vec2f uv;
};

struct shader_clouds_model_block
{
	struct mat4f mvp;
	struct mat4f mv;
	struct vec4f fog_color;
	float fog_distance;
	float fog_density;
};

struct shader_clouds_vertex
{
	struct vec3f position;
	struct vec3f color;
};

struct shader_gui_model_block
{
	struct mat4f mvp;
};

struct shader_gui_vertex
{
	struct vec2f position;
	struct vec4f color;
	struct vec2f uv;
};

struct shaders
{
	gfx_shader_state_t particles;
	gfx_shader_state_t breaking;
	gfx_shader_state_t dropped;
	gfx_shader_state_t focused;
	gfx_shader_state_t sunmoon;
	gfx_shader_state_t blocks;
	gfx_shader_state_t clouds;
	gfx_shader_state_t entity;
	gfx_shader_state_t skybox;
	gfx_shader_state_t gui;
};

struct shaders *shaders_new(void);
void shaders_delete(struct shaders *shaders);

#endif
