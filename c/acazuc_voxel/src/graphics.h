#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <gfx/objects.h>

struct graphics_particles
{
	gfx_depth_stencil_state_t depth_stencil_state;
	gfx_rasterizer_state_t rasterizer_state;
	gfx_pipeline_state_t pipeline_state;
	gfx_input_layout_t input_layout;
	gfx_blend_state_t blend_state;
};

struct graphics_breaking
{
	gfx_depth_stencil_state_t depth_stencil_state;
	gfx_rasterizer_state_t rasterizer_state;
	gfx_pipeline_state_t pipeline_state;
	gfx_input_layout_t input_layout;
	gfx_blend_state_t blend_state;
};

struct graphics_focused
{
	gfx_depth_stencil_state_t depth_stencil_state;
	gfx_rasterizer_state_t rasterizer_state;
	gfx_pipeline_state_t pipeline_state;
	gfx_input_layout_t input_layout;
	gfx_blend_state_t blend_state;
};

struct graphics_sunmoon
{
	gfx_depth_stencil_state_t depth_stencil_state;
	gfx_rasterizer_state_t rasterizer_state;
	gfx_pipeline_state_t pipeline_state;
	gfx_input_layout_t input_layout;
	gfx_blend_state_t blend_state;
};

struct graphics_clouds
{
	gfx_depth_stencil_state_t depth_stencil_state;
	gfx_rasterizer_state_t rasterizer_state;
	gfx_input_layout_t input_layout;
	gfx_blend_state_t first_blend_state;
	gfx_blend_state_t second_blend_state;
	gfx_pipeline_state_t first_pipeline_state;
	gfx_pipeline_state_t second_pipeline_state;
};

struct graphics_skybox
{
	gfx_depth_stencil_state_t depth_stencil_state;
	gfx_rasterizer_state_t rasterizer_state;
	gfx_pipeline_state_t pipeline_state;
	gfx_input_layout_t input_layout;
	gfx_blend_state_t blend_state;
};

struct graphics_blocks
{
	gfx_depth_stencil_state_t depth_stencil_state;
	gfx_rasterizer_state_t first_rasterizer_state;
	gfx_rasterizer_state_t second_rasterizer_state;
	gfx_input_layout_t input_layout;
	gfx_blend_state_t blend_state;
	gfx_pipeline_state_t first_pipeline_state;
	gfx_pipeline_state_t second_pipeline_state;
};

struct graphics_entity
{
	gfx_depth_stencil_state_t depth_stencil_state;
	gfx_rasterizer_state_t rasterizer_state;
	gfx_pipeline_state_t pipeline_state;
	gfx_input_layout_t input_layout;
	gfx_blend_state_t blend_state;
};

struct graphics_gui
{
	gfx_depth_stencil_state_t depth_stencil_state;
	gfx_rasterizer_state_t rasterizer_state;
	gfx_pipeline_state_t cross_pipeline_state;
	gfx_pipeline_state_t pipeline_state;
	gfx_input_layout_t input_layout;
	gfx_blend_state_t cross_blend_state;
	gfx_blend_state_t blend_state;
};

struct graphics
{
	struct graphics_particles particles;
	struct graphics_breaking breaking;
	struct graphics_focused focused;
	struct graphics_sunmoon sunmoon;
	struct graphics_clouds clouds;
	struct graphics_skybox skybox;
	struct graphics_blocks blocks;
	struct graphics_entity entity;
	struct graphics_gui gui;
};

struct graphics *graphics_new(void);
void graphics_delete(struct graphics *graphics);

#endif
