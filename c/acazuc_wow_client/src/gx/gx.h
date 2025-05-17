#ifndef GX_GX_H
#define GX_GX_H

#include <gfx/objects.h>

struct gx_blp;

enum gx_blend_state
{
	GX_BLEND_OPAQUE,
	GX_BLEND_ALPHA,
	GX_BLEND_NO_ALPHA_ADD,
	GX_BLEND_ADD,
	GX_BLEND_MOD,
	GX_BLEND_MOD2X,
	GX_BLEND_LAST
};

enum gx_depth_stencil_state
{
	GX_DEPTH_STENCIL_NO_NO,
	GX_DEPTH_STENCIL_NO_RO,
	GX_DEPTH_STENCIL_NO_WO,
	GX_DEPTH_STENCIL_NO_RW,
	GX_DEPTH_STENCIL_RO_NO,
	GX_DEPTH_STENCIL_RO_RO,
	GX_DEPTH_STENCIL_RO_WO,
	GX_DEPTH_STENCIL_RO_RW,
	GX_DEPTH_STENCIL_WO_NO,
	GX_DEPTH_STENCIL_WO_RO,
	GX_DEPTH_STENCIL_WO_WO,
	GX_DEPTH_STENCIL_WO_RW,
	GX_DEPTH_STENCIL_RW_NO,
	GX_DEPTH_STENCIL_RW_RO,
	GX_DEPTH_STENCIL_RW_WO,
	GX_DEPTH_STENCIL_RW_RW,
	GX_DEPTH_STENCIL_LAST
};

enum gx_rasterizer_state
{
	GX_RASTERIZER_CULLED,
	GX_RASTERIZER_UNCULLED,
	GX_RASTERIZER_LAST
};

enum gx_opt
{
	GX_OPT_FOG            = (1 << 0),
	GX_OPT_WDL            = (1 << 1),
	GX_OPT_WDL_AABB       = (1 << 2),
	GX_OPT_MESH           = (1 << 3),
	GX_OPT_ADT_AABB       = (1 << 4),
	GX_OPT_MCNK           = (1 << 5),
	GX_OPT_MCNK_AABB      = (1 << 6),
	GX_OPT_MCLQ           = (1 << 7),
	GX_OPT_MCLQ_AABB      = (1 << 8),
	GX_OPT_M2             = (1 << 9),
	GX_OPT_M2_AABB        = (1 << 10),
	GX_OPT_M2_BONES       = (1 << 11),
	GX_OPT_M2_LIGHTS      = (1 << 12),
	GX_OPT_M2_INSTANCED   = (1 << 13),
	GX_OPT_M2_PARTICLES   = (1 << 14),
	GX_OPT_M2_COLLISIONS  = (1 << 15),
	GX_OPT_M2_RIBBONS     = (1 << 16),
	GX_OPT_WMO            = (1 << 17),
	GX_OPT_WMO_AABB       = (1 << 18),
	GX_OPT_WMO_LIGHTS     = (1 << 19),
	GX_OPT_WMO_PORTALS    = (1 << 20),
	GX_OPT_WMO_LIQUIDS    = (1 << 21),
	GX_OPT_WMO_COLLISIONS = (1 << 22),
	GX_OPT_SKYBOX         = (1 << 23),
	GX_OPT_COLLISIONS     = (1 << 24),
	GX_OPT_TAXI           = (1 << 25),
	GX_OPT_DYN_WATER      = (1 << 26),
	GX_OPT_GROUND_EFFECT  = (1 << 27),
	GX_OPT_DYN_SHADOW     = (1 << 28),
};

struct gx
{
	gfx_depth_stencil_state_t depth_stencil_states[GX_DEPTH_STENCIL_LAST];
	gfx_rasterizer_state_t rasterizer_states[GX_RASTERIZER_LAST];
	gfx_blend_state_t blend_states[GX_BLEND_LAST];
#ifdef WITH_DEBUG_RENDERING
	gfx_rasterizer_state_t collisions_triangles_rasterizer_state;
	gfx_rasterizer_state_t collisions_lines_rasterizer_state;
	gfx_pipeline_state_t wmo_collisions_triangles_pipeline_state;
	gfx_pipeline_state_t m2_collisions_triangles_pipeline_state;
	gfx_pipeline_state_t collisions_triangles_pipeline_state;
	gfx_pipeline_state_t wmo_collisions_lines_pipeline_state;
	gfx_pipeline_state_t m2_collisions_lines_pipeline_state;
	gfx_pipeline_state_t collisions_lines_pipeline_state;
	gfx_pipeline_state_t m2_bones_points_pipeline_state;
	gfx_pipeline_state_t m2_bones_lines_pipeline_state;
	gfx_pipeline_state_t wmo_portals_pipeline_state;
	gfx_pipeline_state_t wmo_lights_pipeline_state;
	gfx_pipeline_state_t m2_lights_pipeline_state;
	gfx_pipeline_state_t aabb_pipeline_state;
	gfx_pipeline_state_t taxi_pipeline_state;
	gfx_input_layout_t m2_bones_points_input_layout;
	gfx_input_layout_t m2_bones_lines_input_layout;
	gfx_input_layout_t wmo_collisions_input_layout;
	gfx_input_layout_t m2_collisions_input_layout;
	gfx_input_layout_t wmo_portals_input_layout;
	gfx_input_layout_t collisions_input_layout;
	gfx_input_layout_t wmo_lights_input_layout;
	gfx_input_layout_t m2_lights_input_layout;
	gfx_input_layout_t aabb_input_layout;
	gfx_input_layout_t taxi_input_layout;
#endif
	gfx_pipeline_state_t wmo_colored_pipeline_states[GX_RASTERIZER_LAST][GX_BLEND_LAST];
	gfx_pipeline_state_t mclq_water_dyn_pipeline_state;
	gfx_pipeline_state_t mclq_water_pipeline_state;
	gfx_pipeline_state_t mclq_magma_pipeline_state;
	gfx_pipeline_state_t particles_pipeline_states[GX_BLEND_LAST];
	gfx_pipeline_state_t m2_ground_pipeline_state;
	gfx_pipeline_state_t wmo_mliq_pipeline_state;
	gfx_pipeline_state_t ribbons_pipeline_states[GX_RASTERIZER_LAST][GX_DEPTH_STENCIL_LAST][GX_BLEND_LAST];
	gfx_pipeline_state_t skybox_pipeline_state;
	gfx_pipeline_state_t mcnk_pipeline_state;
	gfx_pipeline_state_t text_pipeline_state;
	gfx_pipeline_state_t wdl_pipeline_state;
	gfx_pipeline_state_t wmo_pipeline_states[GX_RASTERIZER_LAST][GX_BLEND_LAST];
	gfx_pipeline_state_t m2_pipeline_states[GX_RASTERIZER_LAST][GX_DEPTH_STENCIL_LAST][GX_BLEND_LAST];
	gfx_input_layout_t wmo_colored_input_layout;
	gfx_input_layout_t mclq_water_input_layout;
	gfx_input_layout_t mclq_magma_input_layout;
	gfx_input_layout_t particles_input_layout;
	gfx_input_layout_t m2_ground_input_layout;
	gfx_input_layout_t wmo_mliq_input_layout;
	gfx_input_layout_t ribbons_input_layout;
	gfx_input_layout_t skybox_input_layout;
	gfx_input_layout_t mcnk_input_layout;
	gfx_input_layout_t text_input_layout;
	gfx_input_layout_t wdl_input_layout;
	gfx_input_layout_t wmo_input_layout;
	gfx_input_layout_t m2_input_layout;
	gfx_buffer_t particles_indices_buffer;
	gfx_buffer_t mcnk_vertexes_buffer;
	gfx_buffer_t aabb_vertexes_buffer;
	gfx_buffer_t mcnk_indices_buffer;
	gfx_buffer_t aabb_indices_buffer;
	struct gx_blp *lavag_textures[30];
	struct gx_blp *river_textures[30];
	struct gx_blp *ocean_textures[30];
	struct gx_blp *magma_textures[30];
	struct gx_blp *slime_textures[30];
	struct gfx_device *device;
	uint32_t mcnk_indices_nb;
	enum gx_opt opt;
};

struct gx *gx_new(struct gfx_device *device);
void gx_delete(struct gx *gx);
bool gx_build_rasterizer_states(struct gx *gx);

#endif
