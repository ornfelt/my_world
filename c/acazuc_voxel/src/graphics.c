#include "graphics.h"
#include "shaders.h"
#include "voxel.h"
#include "log.h"

#include <gfx/device.h>

#include <stdlib.h>
#include <string.h>

static bool build_particles(struct graphics_particles *particles)
{
	static const struct gfx_input_layout_bind binds[] =
	{
		{0, GFX_ATTR_R32G32B32A32_FLOAT, sizeof(struct shader_particles_vertex), offsetof(struct shader_particles_vertex, position)},
		{0, GFX_ATTR_R32G32B32A32_FLOAT, sizeof(struct shader_particles_vertex), offsetof(struct shader_particles_vertex, color)},
		{0, GFX_ATTR_R32G32_FLOAT      , sizeof(struct shader_particles_vertex), offsetof(struct shader_particles_vertex, uv)},
	};

	gfx_create_depth_stencil_state(g_voxel->device, &particles->depth_stencil_state, false, true, GFX_CMP_LEQUAL, false, 0, GFX_CMP_ALWAYS, 0, 0, GFX_STENCIL_KEEP, GFX_STENCIL_KEEP, GFX_STENCIL_KEEP);
	gfx_create_rasterizer_state(g_voxel->device, &particles->rasterizer_state, GFX_FILL_SOLID, GFX_CULL_NONE, GFX_FRONT_CCW, false);
	gfx_create_input_layout(g_voxel->device, &particles->input_layout, binds, sizeof(binds) / sizeof(*binds), &g_voxel->shaders->particles);
	gfx_create_blend_state(g_voxel->device, &particles->blend_state, true, GFX_BLEND_SRC_ALPHA, GFX_BLEND_ONE_MINUS_SRC_ALPHA, GFX_BLEND_SRC_ALPHA, GFX_BLEND_ONE_MINUS_SRC_ALPHA, GFX_EQUATION_ADD, GFX_EQUATION_ADD, GFX_COLOR_MASK_ALL);
	gfx_create_pipeline_state(g_voxel->device, &particles->pipeline_state, &g_voxel->shaders->particles, &particles->rasterizer_state, &particles->depth_stencil_state, &particles->blend_state, &particles->input_layout, GFX_PRIMITIVE_TRIANGLES);
	return true;
}

static void clean_particles(struct graphics_particles *particles)
{
	gfx_delete_pipeline_state(g_voxel->device, &particles->pipeline_state);
	gfx_delete_blend_state(g_voxel->device, &particles->blend_state);
	gfx_delete_input_layout(g_voxel->device, &particles->input_layout);
	gfx_delete_rasterizer_state(g_voxel->device, &particles->rasterizer_state);
	gfx_delete_depth_stencil_state(g_voxel->device, &particles->depth_stencil_state);
}

static bool build_breaking(struct graphics_breaking *breaking)
{
	static const struct gfx_input_layout_bind binds[] =
	{
		{0, GFX_ATTR_R32G32B32_FLOAT, sizeof(struct shader_breaking_vertex), offsetof(struct shader_breaking_vertex, position)},
		{0, GFX_ATTR_R32G32_FLOAT   , sizeof(struct shader_breaking_vertex), offsetof(struct shader_breaking_vertex, uv)},
	};

	gfx_create_depth_stencil_state(g_voxel->device, &breaking->depth_stencil_state, false, true, GFX_CMP_LEQUAL, false, 0, GFX_CMP_ALWAYS, 0, 0, GFX_STENCIL_KEEP, GFX_STENCIL_KEEP, GFX_STENCIL_KEEP);
	gfx_create_rasterizer_state(g_voxel->device, &breaking->rasterizer_state, GFX_FILL_SOLID, GFX_CULL_BACK, GFX_FRONT_CCW, false);
	gfx_create_input_layout(g_voxel->device, &breaking->input_layout, binds, sizeof(binds) / sizeof(*binds), &g_voxel->shaders->breaking);
	gfx_create_blend_state(g_voxel->device, &breaking->blend_state, true, GFX_BLEND_DST_COLOR, GFX_BLEND_SRC_COLOR, GFX_BLEND_DST_ALPHA, GFX_BLEND_SRC_ALPHA, GFX_EQUATION_ADD, GFX_EQUATION_ADD, GFX_COLOR_MASK_ALL);
	gfx_create_pipeline_state(g_voxel->device, &breaking->pipeline_state, &g_voxel->shaders->breaking, &breaking->rasterizer_state, &breaking->depth_stencil_state, &breaking->blend_state, &breaking->input_layout, GFX_PRIMITIVE_TRIANGLES);
	return true;
}

static void clean_breaking(struct graphics_breaking *breaking)
{
	gfx_delete_pipeline_state(g_voxel->device, &breaking->pipeline_state);
	gfx_delete_blend_state(g_voxel->device, &breaking->blend_state);
	gfx_delete_input_layout(g_voxel->device, &breaking->input_layout);
	gfx_delete_rasterizer_state(g_voxel->device, &breaking->rasterizer_state);
	gfx_delete_depth_stencil_state(g_voxel->device, &breaking->depth_stencil_state);
}

static bool build_focused(struct graphics_focused *focused)
{
	static const struct gfx_input_layout_bind binds[] =
	{
		{0, GFX_ATTR_R32G32B32_FLOAT, sizeof(struct shader_focused_vertex), offsetof(struct shader_focused_vertex, position)},
	};

	gfx_create_depth_stencil_state(g_voxel->device, &focused->depth_stencil_state, false, true, GFX_CMP_LEQUAL, false, 0, GFX_CMP_ALWAYS, 0, 0, GFX_STENCIL_KEEP, GFX_STENCIL_KEEP, GFX_STENCIL_KEEP);
	gfx_create_rasterizer_state(g_voxel->device, &focused->rasterizer_state, GFX_FILL_SOLID, GFX_CULL_NONE, GFX_FRONT_CCW, false);
	gfx_create_input_layout(g_voxel->device, &focused->input_layout, binds, sizeof(binds) / sizeof(*binds), &g_voxel->shaders->focused);
	gfx_create_blend_state(g_voxel->device, &focused->blend_state, true, GFX_BLEND_SRC_ALPHA, GFX_BLEND_ONE_MINUS_SRC_ALPHA, GFX_BLEND_SRC_ALPHA, GFX_BLEND_ONE_MINUS_SRC_ALPHA, GFX_EQUATION_ADD, GFX_EQUATION_ADD, GFX_COLOR_MASK_ALL);
	gfx_create_pipeline_state(g_voxel->device, &focused->pipeline_state, &g_voxel->shaders->focused, &focused->rasterizer_state, &focused->depth_stencil_state, &focused->blend_state, &focused->input_layout, GFX_PRIMITIVE_LINES);
	return true;
}

static void clean_focused(struct graphics_focused *focused)
{
	gfx_delete_pipeline_state(g_voxel->device, &focused->pipeline_state);
	gfx_delete_blend_state(g_voxel->device, &focused->blend_state);
	gfx_delete_input_layout(g_voxel->device, &focused->input_layout);
	gfx_delete_rasterizer_state(g_voxel->device, &focused->rasterizer_state);
	gfx_delete_depth_stencil_state(g_voxel->device, &focused->depth_stencil_state);
}

static bool build_sunmoon(struct graphics_sunmoon *sunmoon)
{
	static const struct gfx_input_layout_bind binds[] =
	{
		{0, GFX_ATTR_R32G32B32_FLOAT, sizeof(struct shader_sunmoon_vertex), offsetof(struct shader_sunmoon_vertex, position)},
		{0, GFX_ATTR_R32G32B32_FLOAT, sizeof(struct shader_sunmoon_vertex), offsetof(struct shader_sunmoon_vertex, color)},
		{0, GFX_ATTR_R32G32_FLOAT   , sizeof(struct shader_sunmoon_vertex), offsetof(struct shader_sunmoon_vertex, uv)},
	};

	gfx_create_depth_stencil_state(g_voxel->device, &sunmoon->depth_stencil_state, false, true, GFX_CMP_LEQUAL, false, 0, GFX_CMP_ALWAYS, 0, 0, GFX_STENCIL_KEEP, GFX_STENCIL_KEEP, GFX_STENCIL_KEEP);
	gfx_create_rasterizer_state(g_voxel->device, &sunmoon->rasterizer_state, GFX_FILL_SOLID, GFX_CULL_NONE, GFX_FRONT_CCW, false);
	gfx_create_input_layout(g_voxel->device, &sunmoon->input_layout, binds, sizeof(binds) / sizeof(*binds), &g_voxel->shaders->sunmoon);
	gfx_create_blend_state(g_voxel->device, &sunmoon->blend_state, true, GFX_BLEND_ONE, GFX_BLEND_ONE_MINUS_SRC_COLOR, GFX_BLEND_ONE, GFX_BLEND_ONE_MINUS_SRC_COLOR, GFX_EQUATION_ADD, GFX_EQUATION_ADD, GFX_COLOR_MASK_ALL);
	gfx_create_pipeline_state(g_voxel->device, &sunmoon->pipeline_state, &g_voxel->shaders->sunmoon, &sunmoon->rasterizer_state, &sunmoon->depth_stencil_state, &sunmoon->blend_state, &sunmoon->input_layout, GFX_PRIMITIVE_TRIANGLES);
	return true;
}

static void clean_sunmoon(struct graphics_sunmoon *sunmoon)
{
	gfx_delete_pipeline_state(g_voxel->device, &sunmoon->pipeline_state);
	gfx_delete_blend_state(g_voxel->device, &sunmoon->blend_state);
	gfx_delete_input_layout(g_voxel->device, &sunmoon->input_layout);
	gfx_delete_rasterizer_state(g_voxel->device, &sunmoon->rasterizer_state);
	gfx_delete_depth_stencil_state(g_voxel->device, &sunmoon->depth_stencil_state);
}

static bool build_clouds(struct graphics_clouds *clouds)
{
	static const struct gfx_input_layout_bind binds[] =
	{
		{0, GFX_ATTR_R32G32B32_FLOAT, sizeof(struct shader_clouds_vertex), offsetof(struct shader_clouds_vertex, position)},
		{0, GFX_ATTR_R32G32B32_FLOAT, sizeof(struct shader_clouds_vertex), offsetof(struct shader_clouds_vertex, color)},
	};

	gfx_create_depth_stencil_state(g_voxel->device, &clouds->depth_stencil_state, true, true, GFX_CMP_LEQUAL, false, 0, GFX_CMP_ALWAYS, 0, 0, GFX_STENCIL_KEEP, GFX_STENCIL_KEEP, GFX_STENCIL_KEEP);
	gfx_create_rasterizer_state(g_voxel->device, &clouds->rasterizer_state, GFX_FILL_SOLID, GFX_CULL_NONE, GFX_FRONT_CCW, false);
	gfx_create_input_layout(g_voxel->device, &clouds->input_layout, binds, sizeof(binds) / sizeof(*binds), &g_voxel->shaders->clouds);
	gfx_create_blend_state(g_voxel->device, &clouds->first_blend_state, false, GFX_BLEND_SRC_ALPHA, GFX_BLEND_ONE_MINUS_SRC_ALPHA, GFX_BLEND_SRC_ALPHA, GFX_BLEND_ONE_MINUS_SRC_ALPHA, GFX_EQUATION_ADD, GFX_EQUATION_ADD, GFX_COLOR_MASK_NONE);
	gfx_create_blend_state(g_voxel->device, &clouds->second_blend_state, false, GFX_BLEND_SRC_ALPHA, GFX_BLEND_ONE_MINUS_SRC_ALPHA, GFX_BLEND_SRC_ALPHA, GFX_BLEND_ONE_MINUS_SRC_ALPHA, GFX_EQUATION_ADD, GFX_EQUATION_ADD, GFX_COLOR_MASK_ALL);
	gfx_create_pipeline_state(g_voxel->device, &clouds->first_pipeline_state, &g_voxel->shaders->clouds, &clouds->rasterizer_state, &clouds->depth_stencil_state, &clouds->first_blend_state, &clouds->input_layout, GFX_PRIMITIVE_TRIANGLES);
	gfx_create_pipeline_state(g_voxel->device, &clouds->second_pipeline_state, &g_voxel->shaders->clouds, &clouds->rasterizer_state, &clouds->depth_stencil_state, &clouds->second_blend_state, &clouds->input_layout, GFX_PRIMITIVE_TRIANGLES);
	return true;
}

static void clean_clouds(struct graphics_clouds *clouds)
{
	gfx_delete_pipeline_state(g_voxel->device, &clouds->first_pipeline_state);
	gfx_delete_pipeline_state(g_voxel->device, &clouds->second_pipeline_state);
	gfx_delete_blend_state(g_voxel->device, &clouds->first_blend_state);
	gfx_delete_blend_state(g_voxel->device, &clouds->second_blend_state);
	gfx_delete_input_layout(g_voxel->device, &clouds->input_layout);
	gfx_delete_rasterizer_state(g_voxel->device, &clouds->rasterizer_state);
	gfx_delete_depth_stencil_state(g_voxel->device, &clouds->depth_stencil_state);
}

static bool build_skybox(struct graphics_skybox *skybox)
{
	static const struct gfx_input_layout_bind binds[] =
	{
		{0, GFX_ATTR_R32G32B32_FLOAT, sizeof(struct shader_skybox_vertex), offsetof(struct shader_skybox_vertex, position)},
		{0, GFX_ATTR_R32G32B32_FLOAT, sizeof(struct shader_skybox_vertex), offsetof(struct shader_skybox_vertex, color)},
	};

	gfx_create_depth_stencil_state(g_voxel->device, &skybox->depth_stencil_state, false, true, GFX_CMP_LEQUAL, false, 0, GFX_CMP_ALWAYS, 0, 0, GFX_STENCIL_KEEP, GFX_STENCIL_KEEP, GFX_STENCIL_KEEP);
	gfx_create_rasterizer_state(g_voxel->device, &skybox->rasterizer_state, GFX_FILL_SOLID, GFX_CULL_NONE, GFX_FRONT_CCW, false);
	gfx_create_input_layout(g_voxel->device, &skybox->input_layout, binds, sizeof(binds) / sizeof(*binds), &g_voxel->shaders->skybox);
	gfx_create_blend_state(g_voxel->device, &skybox->blend_state, false, GFX_BLEND_ONE, GFX_BLEND_ZERO, GFX_BLEND_ONE, GFX_BLEND_ZERO, GFX_EQUATION_ADD, GFX_EQUATION_ADD, GFX_COLOR_MASK_ALL);
	gfx_create_pipeline_state(g_voxel->device, &skybox->pipeline_state, &g_voxel->shaders->skybox, &skybox->rasterizer_state, &skybox->depth_stencil_state, &skybox->blend_state, &skybox->input_layout, GFX_PRIMITIVE_TRIANGLES);
	return true;
}

static void clean_skybox(struct graphics_skybox *skybox)
{
	gfx_delete_pipeline_state(g_voxel->device, &skybox->pipeline_state);
	gfx_delete_blend_state(g_voxel->device, &skybox->blend_state);
	gfx_delete_input_layout(g_voxel->device, &skybox->input_layout);
	gfx_delete_rasterizer_state(g_voxel->device, &skybox->rasterizer_state);
	gfx_delete_depth_stencil_state(g_voxel->device, &skybox->depth_stencil_state);
}

static bool build_blocks(struct graphics_blocks *blocks)
{
	static const struct gfx_input_layout_bind binds[] =
	{
		{0, GFX_ATTR_R32G32B32_FLOAT, sizeof(struct shader_blocks_vertex), offsetof(struct shader_blocks_vertex, position)},
		{0, GFX_ATTR_R32G32B32_FLOAT, sizeof(struct shader_blocks_vertex), offsetof(struct shader_blocks_vertex, color)},
		{0, GFX_ATTR_R32G32_FLOAT   , sizeof(struct shader_blocks_vertex), offsetof(struct shader_blocks_vertex, uv)},
	};

	gfx_create_depth_stencil_state(g_voxel->device, &blocks->depth_stencil_state, true, true, GFX_CMP_LEQUAL, false, 0, GFX_CMP_ALWAYS, 0, 0, GFX_STENCIL_KEEP, GFX_STENCIL_KEEP, GFX_STENCIL_KEEP);
	gfx_create_rasterizer_state(g_voxel->device, &blocks->first_rasterizer_state, GFX_FILL_SOLID, GFX_CULL_BACK, GFX_FRONT_CCW, false);
	gfx_create_rasterizer_state(g_voxel->device, &blocks->second_rasterizer_state, GFX_FILL_SOLID, GFX_CULL_NONE, GFX_FRONT_CCW, false);
	gfx_create_input_layout(g_voxel->device, &blocks->input_layout, binds, sizeof(binds) / sizeof(*binds), &g_voxel->shaders->blocks);
	gfx_create_blend_state(g_voxel->device, &blocks->blend_state, false, GFX_BLEND_SRC_ALPHA, GFX_BLEND_ONE_MINUS_SRC_ALPHA, GFX_BLEND_SRC_ALPHA, GFX_BLEND_ONE_MINUS_SRC_ALPHA, GFX_EQUATION_ADD, GFX_EQUATION_ADD, GFX_COLOR_MASK_ALL);
	gfx_create_pipeline_state(g_voxel->device, &blocks->first_pipeline_state, &g_voxel->shaders->blocks, &blocks->first_rasterizer_state, &blocks->depth_stencil_state, &blocks->blend_state, &blocks->input_layout, GFX_PRIMITIVE_TRIANGLES);
	gfx_create_pipeline_state(g_voxel->device, &blocks->second_pipeline_state, &g_voxel->shaders->blocks, &blocks->second_rasterizer_state, &blocks->depth_stencil_state, &blocks->blend_state, &blocks->input_layout, GFX_PRIMITIVE_TRIANGLES);
	return true;
}

static void clean_blocks(struct graphics_blocks *blocks)
{
	gfx_delete_pipeline_state(g_voxel->device, &blocks->first_pipeline_state);
	gfx_delete_pipeline_state(g_voxel->device, &blocks->second_pipeline_state);
	gfx_delete_blend_state(g_voxel->device, &blocks->blend_state);
	gfx_delete_input_layout(g_voxel->device, &blocks->input_layout);
	gfx_delete_rasterizer_state(g_voxel->device, &blocks->first_rasterizer_state);
	gfx_delete_rasterizer_state(g_voxel->device, &blocks->second_rasterizer_state);
	gfx_delete_depth_stencil_state(g_voxel->device, &blocks->depth_stencil_state);
}

static bool build_entity(struct graphics_entity *entity)
{
	static const struct gfx_input_layout_bind binds[] =
	{
		{0, GFX_ATTR_R32G32B32_FLOAT, sizeof(struct shader_entity_vertex), offsetof(struct shader_entity_vertex, position)},
		{0, GFX_ATTR_R32G32_FLOAT   , sizeof(struct shader_entity_vertex), offsetof(struct shader_entity_vertex, uv)},
	};

	gfx_create_depth_stencil_state(g_voxel->device, &entity->depth_stencil_state, true, true, GFX_CMP_LEQUAL, false, 0, GFX_CMP_ALWAYS, 0, 0, GFX_STENCIL_KEEP, GFX_STENCIL_KEEP, GFX_STENCIL_KEEP);
	gfx_create_rasterizer_state(g_voxel->device, &entity->rasterizer_state, GFX_FILL_SOLID, GFX_CULL_NONE, GFX_FRONT_CCW, false);
	gfx_create_input_layout(g_voxel->device, &entity->input_layout, binds, sizeof(binds) / sizeof(*binds), &g_voxel->shaders->entity);
	gfx_create_blend_state(g_voxel->device, &entity->blend_state, false, GFX_BLEND_SRC_ALPHA, GFX_BLEND_ONE_MINUS_SRC_ALPHA, GFX_BLEND_SRC_ALPHA, GFX_BLEND_ONE_MINUS_SRC_ALPHA, GFX_EQUATION_ADD, GFX_EQUATION_ADD, GFX_COLOR_MASK_ALL);
	gfx_create_pipeline_state(g_voxel->device, &entity->pipeline_state, &g_voxel->shaders->entity, &entity->rasterizer_state, &entity->depth_stencil_state, &entity->blend_state, &entity->input_layout, GFX_PRIMITIVE_TRIANGLES);
	return true;
}

static void clean_entity(struct graphics_entity *entity)
{
	gfx_delete_pipeline_state(g_voxel->device, &entity->pipeline_state);
	gfx_delete_blend_state(g_voxel->device, &entity->blend_state);
	gfx_delete_input_layout(g_voxel->device, &entity->input_layout);
	gfx_delete_rasterizer_state(g_voxel->device, &entity->rasterizer_state);
	gfx_delete_depth_stencil_state(g_voxel->device, &entity->depth_stencil_state);
}

static bool build_gui(struct graphics_gui *gui)
{
	static const struct gfx_input_layout_bind binds[] =
	{
		{0, GFX_ATTR_R32G32_FLOAT      , sizeof(struct shader_gui_vertex), offsetof(struct shader_gui_vertex, position)},
		{0, GFX_ATTR_R32G32B32A32_FLOAT, sizeof(struct shader_gui_vertex), offsetof(struct shader_gui_vertex, color)},
		{0, GFX_ATTR_R32G32_FLOAT      , sizeof(struct shader_gui_vertex), offsetof(struct shader_gui_vertex, uv)},
	};

	gfx_create_depth_stencil_state(g_voxel->device, &gui->depth_stencil_state, false, false, GFX_CMP_ALWAYS, false, 0, GFX_CMP_ALWAYS, 0, 0, GFX_STENCIL_KEEP, GFX_STENCIL_KEEP, GFX_STENCIL_KEEP);
	gfx_create_rasterizer_state(g_voxel->device, &gui->rasterizer_state, GFX_FILL_SOLID, GFX_CULL_NONE, GFX_FRONT_CCW, false);
	gfx_create_input_layout(g_voxel->device, &gui->input_layout, binds, sizeof(binds) / sizeof(*binds), &g_voxel->shaders->gui);
	gfx_create_blend_state(g_voxel->device, &gui->blend_state, true, GFX_BLEND_SRC_ALPHA, GFX_BLEND_ONE_MINUS_SRC_ALPHA, GFX_BLEND_SRC_ALPHA, GFX_BLEND_ONE_MINUS_SRC_ALPHA, GFX_EQUATION_ADD, GFX_EQUATION_ADD, GFX_COLOR_MASK_ALL);
	gfx_create_blend_state(g_voxel->device, &gui->cross_blend_state, true, GFX_BLEND_ONE_MINUS_DST_COLOR, GFX_BLEND_ONE_MINUS_SRC_ALPHA, GFX_BLEND_ONE_MINUS_DST_COLOR, GFX_BLEND_ONE_MINUS_SRC_ALPHA, GFX_EQUATION_ADD, GFX_EQUATION_ADD, GFX_COLOR_MASK_ALL);
	gfx_create_pipeline_state(g_voxel->device, &gui->pipeline_state, &g_voxel->shaders->gui, &gui->rasterizer_state, &gui->depth_stencil_state, &gui->blend_state, &gui->input_layout, GFX_PRIMITIVE_TRIANGLES);
	gfx_create_pipeline_state(g_voxel->device, &gui->cross_pipeline_state, &g_voxel->shaders->gui, &gui->rasterizer_state, &gui->depth_stencil_state, &gui->cross_blend_state, &gui->input_layout, GFX_PRIMITIVE_TRIANGLES);
	return true;
}

static void clean_gui(struct graphics_gui *gui)
{
	gfx_delete_pipeline_state(g_voxel->device, &gui->pipeline_state);
	gfx_delete_pipeline_state(g_voxel->device, &gui->cross_pipeline_state);
	gfx_delete_blend_state(g_voxel->device, &gui->blend_state);
	gfx_delete_blend_state(g_voxel->device, &gui->cross_blend_state);
	gfx_delete_input_layout(g_voxel->device, &gui->input_layout);
	gfx_delete_rasterizer_state(g_voxel->device, &gui->rasterizer_state);
	gfx_delete_depth_stencil_state(g_voxel->device, &gui->depth_stencil_state);
}

struct graphics *graphics_new(void)
{
	struct graphics *graphics = calloc(sizeof(*graphics), 1);
	if (!graphics)
	{
		LOG_ERROR("graphics allocation failed");
		return NULL;
	}

#define BUILD_GRAPHICS(name) \
	do \
	{ \
		LOG_INFO("building graphics " #name); \
		if (!build_##name(&graphics->name)) \
			goto err; \
	} while (0)

	BUILD_GRAPHICS(particles);
	BUILD_GRAPHICS(breaking);
	BUILD_GRAPHICS(focused);
	BUILD_GRAPHICS(sunmoon);
	BUILD_GRAPHICS(clouds);
	BUILD_GRAPHICS(skybox);
	BUILD_GRAPHICS(blocks);
	BUILD_GRAPHICS(entity);
	BUILD_GRAPHICS(gui);
	return graphics;

#undef BUILD_GRAPHICS

err:
	graphics_delete(graphics);
	return NULL;
}

void graphics_delete(struct graphics *graphics)
{
	if (!graphics)
		return;

#define CLEAN_GRAPHICS(name) \
	do \
	{ \
		LOG_INFO("cleaning graphics " #name); \
		clean_##name(&graphics->name); \
	} while (0)

	CLEAN_GRAPHICS(particles);
	CLEAN_GRAPHICS(breaking);
	CLEAN_GRAPHICS(focused);
	CLEAN_GRAPHICS(sunmoon);
	CLEAN_GRAPHICS(clouds);
	CLEAN_GRAPHICS(skybox);
	CLEAN_GRAPHICS(blocks);
	CLEAN_GRAPHICS(entity);
	CLEAN_GRAPHICS(gui);

#undef CLEAN_GRAPHICS

	free(graphics);
}
