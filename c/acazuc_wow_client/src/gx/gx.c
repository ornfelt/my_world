#include "gx/m2_particles.h"
#include "gx/blp.h"
#include "gx/gx.h"

#include "shaders.h"
#include "memory.h"
#include "cache.h"
#include "const.h"
#include "log.h"
#include "wow.h"

#include <wow/m2.h>

#include <gfx/device.h>

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

#ifdef WITH_DEBUG_RENDERING
static const struct gfx_input_layout_bind g_m2_bones_points_binds[] =
{
	{0, GFX_ATTR_R32G32B32_FLOAT   , sizeof(struct shader_m2_bones_input), offsetof(struct shader_m2_bones_input, position), GFX_STEP_VERTEX},
	{0, GFX_ATTR_R32G32B32A32_FLOAT, sizeof(struct shader_m2_bones_input), offsetof(struct shader_m2_bones_input, color)   , GFX_STEP_VERTEX},
	{0, GFX_ATTR_R32_UINT          , sizeof(struct shader_m2_bones_input), offsetof(struct shader_m2_bones_input, bone)    , GFX_STEP_VERTEX},
};

static const struct gfx_input_layout_bind g_m2_bones_lines_binds[] =
{
	{0, GFX_ATTR_R32G32B32_FLOAT   , sizeof(struct shader_m2_bones_input), offsetof(struct shader_m2_bones_input, position), GFX_STEP_VERTEX},
	{0, GFX_ATTR_R32G32B32A32_FLOAT, sizeof(struct shader_m2_bones_input), offsetof(struct shader_m2_bones_input, color)   , GFX_STEP_VERTEX},
	{0, GFX_ATTR_R32_UINT          , sizeof(struct shader_m2_bones_input), offsetof(struct shader_m2_bones_input, bone)    , GFX_STEP_VERTEX},
};

static const struct gfx_input_layout_bind g_wmo_collisions_binds[] =
{
	{0, GFX_ATTR_R32G32B32_FLOAT, sizeof(struct vec3f), 0, GFX_STEP_VERTEX},
};

static const struct gfx_input_layout_bind g_m2_collisions_binds[] =
{
	{0, GFX_ATTR_R32G32B32_FLOAT, sizeof(struct shader_m2_collisions_input), offsetof(struct shader_m2_collisions_input, position), GFX_STEP_VERTEX},
};

static const struct gfx_input_layout_bind g_wmo_portals_binds[] =
{
	{0, GFX_ATTR_R32G32B32_FLOAT   , sizeof(struct shader_wmo_portals_input), offsetof(struct shader_wmo_portals_input, position), GFX_STEP_VERTEX},
	{0, GFX_ATTR_R32G32B32A32_FLOAT, sizeof(struct shader_wmo_portals_input), offsetof(struct shader_wmo_portals_input, color)   , GFX_STEP_VERTEX},
};

static const struct gfx_input_layout_bind g_collisions_binds[] =
{
	{0, GFX_ATTR_R32G32B32_FLOAT, sizeof(struct vec3f), 0, GFX_STEP_VERTEX},
};

static const struct gfx_input_layout_bind g_wmo_lights_binds[] =
{
	{0, GFX_ATTR_R32G32B32_FLOAT   , sizeof(struct shader_basic_input), offsetof(struct shader_basic_input, position), GFX_STEP_VERTEX},
	{0, GFX_ATTR_R32G32B32A32_FLOAT, sizeof(struct shader_basic_input), offsetof(struct shader_basic_input, color)   , GFX_STEP_VERTEX},
};

static const struct gfx_input_layout_bind g_m2_lights_binds[] =
{
	{0, GFX_ATTR_R32G32B32_FLOAT   , sizeof(struct shader_m2_lights_input), offsetof(struct shader_m2_lights_input, position), GFX_STEP_VERTEX},
	{0, GFX_ATTR_R32G32B32A32_FLOAT, sizeof(struct shader_m2_lights_input), offsetof(struct shader_m2_lights_input, color)   , GFX_STEP_VERTEX},
	{0, GFX_ATTR_R32_UINT          , sizeof(struct shader_m2_lights_input), offsetof(struct shader_m2_lights_input, bone)    , GFX_STEP_VERTEX},
};

static const struct gfx_input_layout_bind g_aabb_binds[] =
{
	{0, GFX_ATTR_R32G32B32_FLOAT   , sizeof(struct vec3f)            , 0                                          , GFX_STEP_VERTEX},
	{1, GFX_ATTR_R32G32B32A32_FLOAT, sizeof(struct shader_aabb_input), offsetof(struct shader_aabb_input, color)  , GFX_STEP_INSTANCE},
	{1, GFX_ATTR_R32G32B32A32_FLOAT, sizeof(struct shader_aabb_input), offsetof(struct shader_aabb_input, min_pos), GFX_STEP_INSTANCE},
	{1, GFX_ATTR_R32G32B32A32_FLOAT, sizeof(struct shader_aabb_input), offsetof(struct shader_aabb_input, max_pos), GFX_STEP_INSTANCE},
	{1, GFX_ATTR_R32G32B32A32_FLOAT, sizeof(struct shader_aabb_input), offsetof(struct shader_aabb_input, mvp.x)  , GFX_STEP_INSTANCE},
	{1, GFX_ATTR_R32G32B32A32_FLOAT, sizeof(struct shader_aabb_input), offsetof(struct shader_aabb_input, mvp.y)  , GFX_STEP_INSTANCE},
	{1, GFX_ATTR_R32G32B32A32_FLOAT, sizeof(struct shader_aabb_input), offsetof(struct shader_aabb_input, mvp.z)  , GFX_STEP_INSTANCE},
	{1, GFX_ATTR_R32G32B32A32_FLOAT, sizeof(struct shader_aabb_input), offsetof(struct shader_aabb_input, mvp.w)  , GFX_STEP_INSTANCE},
};

static const struct gfx_input_layout_bind g_taxi_binds[] =
{
	{0, GFX_ATTR_R32G32B32_FLOAT, sizeof(struct vec3f), 0, GFX_STEP_VERTEX},
};
#endif

static const struct gfx_input_layout_bind g_wmo_colored_binds[] =
{
	{0, GFX_ATTR_R32G32B32_FLOAT, sizeof(struct shader_wmo_input), offsetof(struct shader_wmo_input, position), GFX_STEP_VERTEX},
	{0, GFX_ATTR_R32G32B32_FLOAT, sizeof(struct shader_wmo_input), offsetof(struct shader_wmo_input, norm)    , GFX_STEP_VERTEX},
	{0, GFX_ATTR_R32G32_FLOAT   , sizeof(struct shader_wmo_input), offsetof(struct shader_wmo_input, uv)      , GFX_STEP_VERTEX},
	{1, GFX_ATTR_R8G8B8A8_UNORM , sizeof(uint8_t) * 4            , 0                                          , GFX_STEP_VERTEX},
};

static const struct gfx_input_layout_bind g_mclq_water_binds[] =
{
	{0, GFX_ATTR_R32G32B32_FLOAT, sizeof(struct shader_mclq_input), offsetof(struct shader_mclq_input, position), GFX_STEP_VERTEX},
	{1, GFX_ATTR_R8_UNORM       , sizeof(uint8_t)                 , 0                                           , GFX_STEP_VERTEX},
	{0, GFX_ATTR_R32G32_FLOAT   , sizeof(struct shader_mclq_input), offsetof(struct shader_mclq_input, uv)      , GFX_STEP_VERTEX},
};

static const struct gfx_input_layout_bind g_mclq_magma_binds[] =
{
	{0, GFX_ATTR_R32G32B32_FLOAT, sizeof(struct shader_mclq_input), offsetof(struct shader_mclq_input, position), GFX_STEP_VERTEX},
	{0, GFX_ATTR_R32G32_FLOAT   , sizeof(struct shader_mclq_input), offsetof(struct shader_mclq_input, uv)      , GFX_STEP_VERTEX},
};

static const struct gfx_input_layout_bind g_particles_binds[] =
{
	{0, GFX_ATTR_R32G32B32A32_FLOAT, sizeof(struct shader_particle_input), offsetof(struct shader_particle_input, position), GFX_STEP_VERTEX},
	{0, GFX_ATTR_R32G32B32A32_FLOAT, sizeof(struct shader_particle_input), offsetof(struct shader_particle_input, color)   , GFX_STEP_VERTEX},
	{0, GFX_ATTR_R32G32_FLOAT      , sizeof(struct shader_particle_input), offsetof(struct shader_particle_input, uv)      , GFX_STEP_VERTEX},
};

static const struct gfx_input_layout_bind g_wmo_mliq_binds[] =
{
	{0, GFX_ATTR_R32G32B32_FLOAT   , sizeof(struct shader_mliq_input), offsetof(struct shader_mliq_input, position), GFX_STEP_VERTEX},
	{0, GFX_ATTR_R32G32B32A32_FLOAT, sizeof(struct shader_mliq_input), offsetof(struct shader_mliq_input, uv)      , GFX_STEP_VERTEX},
};

static const struct gfx_input_layout_bind g_ribbons_binds[] =
{
	{0, GFX_ATTR_R32G32B32A32_FLOAT, sizeof(struct shader_ribbon_input), offsetof(struct shader_ribbon_input, position), GFX_STEP_VERTEX},
	{0, GFX_ATTR_R32G32B32A32_FLOAT, sizeof(struct shader_ribbon_input), offsetof(struct shader_ribbon_input, color)   , GFX_STEP_VERTEX},
	{0, GFX_ATTR_R32G32_FLOAT      , sizeof(struct shader_ribbon_input), offsetof(struct shader_ribbon_input, uv)      , GFX_STEP_VERTEX},
};

static const struct gfx_input_layout_bind g_skybox_binds[] =
{
	{0, GFX_ATTR_R32G32B32_FLOAT, sizeof(struct shader_skybox_input), offsetof(struct shader_skybox_input, position) , GFX_STEP_VERTEX},
	{0, GFX_ATTR_R32_FLOAT      , sizeof(struct shader_skybox_input), offsetof(struct shader_skybox_input, colors[0]), GFX_STEP_VERTEX},
	{0, GFX_ATTR_R32_FLOAT      , sizeof(struct shader_skybox_input), offsetof(struct shader_skybox_input, colors[1]), GFX_STEP_VERTEX},
	{0, GFX_ATTR_R32_FLOAT      , sizeof(struct shader_skybox_input), offsetof(struct shader_skybox_input, colors[2]), GFX_STEP_VERTEX},
	{0, GFX_ATTR_R32_FLOAT      , sizeof(struct shader_skybox_input), offsetof(struct shader_skybox_input, colors[3]), GFX_STEP_VERTEX},
	{0, GFX_ATTR_R32_FLOAT      , sizeof(struct shader_skybox_input), offsetof(struct shader_skybox_input, colors[4]), GFX_STEP_VERTEX},
	{0, GFX_ATTR_R32G32_FLOAT   , sizeof(struct shader_skybox_input), offsetof(struct shader_skybox_input, uv)       , GFX_STEP_VERTEX},
};

static const struct gfx_input_layout_bind g_mcnk_binds[] =
{
	{0, GFX_ATTR_R8G8B8A8_SNORM, sizeof(struct shader_mcnk_input), offsetof(struct shader_mcnk_input, norm), GFX_STEP_VERTEX},
	{1, GFX_ATTR_R32G32_FLOAT  , sizeof(struct vec2f) * 2        , sizeof(struct vec2f) * 0                , GFX_STEP_VERTEX},
	{1, GFX_ATTR_R32G32_FLOAT  , sizeof(struct vec2f) * 2        , sizeof(struct vec2f) * 1                , GFX_STEP_VERTEX},
	{0, GFX_ATTR_R32_FLOAT     , sizeof(struct shader_mcnk_input), offsetof(struct shader_mcnk_input, y)   , GFX_STEP_VERTEX},
};

static const struct gfx_input_layout_bind g_text_binds[] =
{
	{0, GFX_ATTR_R32G32_FLOAT, sizeof(struct shader_text_input), offsetof(struct shader_text_input, position), GFX_STEP_VERTEX},
	{0, GFX_ATTR_R32G32_FLOAT, sizeof(struct shader_text_input), offsetof(struct shader_text_input, uv)      , GFX_STEP_VERTEX},
};

static const struct gfx_input_layout_bind g_wdl_binds[] =
{
	{0, GFX_ATTR_R32G32B32_FLOAT, sizeof(struct vec3f), 0, GFX_STEP_VERTEX},
};

static const struct gfx_input_layout_bind g_wmo_binds[] =
{
	{0, GFX_ATTR_R32G32B32_FLOAT, sizeof(struct shader_wmo_input), offsetof(struct shader_wmo_input, position), GFX_STEP_VERTEX},
	{0, GFX_ATTR_R32G32B32_FLOAT, sizeof(struct shader_wmo_input), offsetof(struct shader_wmo_input, norm)    , GFX_STEP_VERTEX},
	{0, GFX_ATTR_R32G32_FLOAT   , sizeof(struct shader_wmo_input), offsetof(struct shader_wmo_input, uv)      , GFX_STEP_VERTEX},
	{1, GFX_ATTR_DISABLED       , 0                              , 0                                          , GFX_STEP_VERTEX},
};

static const struct gfx_input_layout_bind g_m2_binds[] =
{
	{0, GFX_ATTR_R8G8B8A8_UNORM , sizeof(struct wow_m2_vertex), offsetof(struct wow_m2_vertex, bone_weights) , GFX_STEP_VERTEX},
	{0, GFX_ATTR_R32G32B32_FLOAT, sizeof(struct wow_m2_vertex), offsetof(struct wow_m2_vertex, pos)          , GFX_STEP_VERTEX},
	{0, GFX_ATTR_R32G32B32_FLOAT, sizeof(struct wow_m2_vertex), offsetof(struct wow_m2_vertex, normal)       , GFX_STEP_VERTEX},
	{0, GFX_ATTR_R8G8B8A8_UINT  , sizeof(struct wow_m2_vertex), offsetof(struct wow_m2_vertex, bone_indices) , GFX_STEP_VERTEX},
	{0, GFX_ATTR_R32G32_FLOAT   , sizeof(struct wow_m2_vertex), offsetof(struct wow_m2_vertex, tex_coords[0]), GFX_STEP_VERTEX},
	{0, GFX_ATTR_R32G32_FLOAT   , sizeof(struct wow_m2_vertex), offsetof(struct wow_m2_vertex, tex_coords[1]), GFX_STEP_VERTEX},
};

static const struct gfx_input_layout_bind g_m2_ground_binds[] =
{
	{0, GFX_ATTR_R8G8B8A8_UNORM    , sizeof(struct wow_m2_vertex)         , offsetof(struct wow_m2_vertex, bone_weights)  , GFX_STEP_VERTEX},
	{0, GFX_ATTR_R32G32B32_FLOAT   , sizeof(struct wow_m2_vertex)         , offsetof(struct wow_m2_vertex, pos)           , GFX_STEP_VERTEX},
	{0, GFX_ATTR_R32G32B32_FLOAT   , sizeof(struct wow_m2_vertex)         , offsetof(struct wow_m2_vertex, normal)        , GFX_STEP_VERTEX},
	{0, GFX_ATTR_R8G8B8A8_UINT     , sizeof(struct wow_m2_vertex)         , offsetof(struct wow_m2_vertex, bone_indices)  , GFX_STEP_VERTEX},
	{0, GFX_ATTR_R32G32_FLOAT      , sizeof(struct wow_m2_vertex)         , offsetof(struct wow_m2_vertex, tex_coords[0]) , GFX_STEP_VERTEX},
	{0, GFX_ATTR_R32G32_FLOAT      , sizeof(struct wow_m2_vertex)         , offsetof(struct wow_m2_vertex, tex_coords[1]) , GFX_STEP_VERTEX},
	{1, GFX_ATTR_R32G32B32A32_FLOAT, sizeof(struct shader_m2_ground_input), offsetof(struct shader_m2_ground_input, mat.x), GFX_STEP_INSTANCE},
	{1, GFX_ATTR_R32G32B32A32_FLOAT, sizeof(struct shader_m2_ground_input), offsetof(struct shader_m2_ground_input, mat.y), GFX_STEP_INSTANCE},
	{1, GFX_ATTR_R32G32B32A32_FLOAT, sizeof(struct shader_m2_ground_input), offsetof(struct shader_m2_ground_input, mat.z), GFX_STEP_INSTANCE},
	{1, GFX_ATTR_R32G32B32A32_FLOAT, sizeof(struct shader_m2_ground_input), offsetof(struct shader_m2_ground_input, mat.w), GFX_STEP_INSTANCE},
};

bool
gx_build_rasterizer_states(struct gx *gx)
{
	gfx_delete_rasterizer_state(gx->device,
	                            &gx->rasterizer_states[GX_RASTERIZER_CULLED]);
	gfx_delete_rasterizer_state(gx->device,
	                            &gx->rasterizer_states[GX_RASTERIZER_UNCULLED]);
	gfx_create_rasterizer_state(gx->device,
	                            &gx->rasterizer_states[GX_RASTERIZER_CULLED],
	                            (gx->opt & GX_OPT_MESH) ? GFX_FILL_LINE : GFX_FILL_SOLID,
	                            GFX_CULL_BACK,
	                            GFX_FRONT_CCW,
	                            true);
	gfx_create_rasterizer_state(gx->device,
	                            &gx->rasterizer_states[GX_RASTERIZER_UNCULLED],
	                            (gx->opt & GX_OPT_MESH) ? GFX_FILL_LINE : GFX_FILL_SOLID,
	                            GFX_CULL_NONE,
	                            GFX_FRONT_CCW,
	                            true);
	return true;
}

/*
LOD 1: 256

 1    2    3    4    5    6    7    8    9
   10   11   12   13   14   15   16   17
 18   19   20   21   22   23   24   25   26
   27   28   29   30   31   32   33   34
 35   36   37   38   39   40   41   42   43
   44   45   46   47   48   49   50   51
 52   53   54   55   56   57   58   59   60
   61   62   63   64   65   66   67   68
 69   70   71   72   73   74   75   76   77
   78   79   80   81   82   83   84   85
 86   87   88   89   90   91   92   93   94
   95   96   97   98  99  100  101  102
103  104  105  106  107  108  109  110  111
  112  113  114  115  116  117  118  119
120  121  122  123  124  125  126  127  128
  129  130  131  132  133  134  135  136
137  138  139  140  141  142  143  144  145
 */

/* 
LOD 2: 128

 1    2    3    4    5    6    7    8    9

 18   19   20   21   22   23   24   25   26

 35   36   37   38   39   40   41   42   43

 52   53   54   55   56   57   58   59   60

 69   70   71   72   73   74   75   76   77

 86   87   88   89   90   91   92   93   94

103  104  105  106  107  108  109  110  111

120  121  122  123  124  125  126  127  128

137  138  139  140  141  142  143  144  145
 */

/*
LOD 3: 48

 1    2    3    4    5    6    7    8    9

 18                                      26

 35        37        39        41        43

 52                                      60

 69        71        73        75        77

 86                                      94

103       105       107       109       111

120                                     128

137  138  139  140  141  142  143  144  145*/

/* Possible lod opti: if no holes, place sync points only near other chunks */
/* One more lod if no holes ? */
static void
init_mcnk_buffers(struct gx *gx)
{
	uint16_t *indices = NULL;
	uint32_t points_nb = (16 * 16 * (9 * 9 + 8 * 8));
	struct
	{
		struct vec2f xz;
		struct vec2f uv;
	} *vertexes = NULL;
	uint32_t indices_pos = 0;
	indices = mem_malloc(MEM_GX, sizeof(*indices) * (16 * 16 * ((8 * 8 * 4 * 3) + (8 * 8 * 2 * 3) + (48 * 3))));
	if (!indices)
		abort(); /* XXX */
	vertexes = mem_malloc(MEM_GX, sizeof(*vertexes) * points_nb);
	if (!vertexes)
		abort(); /* XXX */
	for (size_t cz = 0; cz < 16; ++cz)
	{
		for (size_t cx = 0; cx < 16; ++cx)
		{
			size_t base = (cz * 16 + cx) * (9 * 9 + 8 * 8);
			for (size_t i = 0; i < 9 * 9 + 8 * 8; ++i)
			{
				size_t y2 = i % 17;
				float z = i / 17 * 2;
				float x;
				if (y2 < 9)
				{
					x = y2 * 2;
				}
				else
				{
					z++;
					x = (y2 - 9) * 2 + 1;
				}
				vertexes[base + i].xz.x = (-1 - (ssize_t)cz + (16 - z) / 16.0f) * CHUNK_WIDTH;
				vertexes[base + i].xz.y = ((1 + cx - (16 - x) / 16.0f) * CHUNK_WIDTH);
				vertexes[base + i].uv.x = x / 16.0f;
				vertexes[base + i].uv.y = z / 16.0f;
			}
			for (size_t z = 0; z < 8; ++z)
			{
				for (size_t x = 0; x < 8; ++x)
				{
					uint16_t idx = base + 9 + z * 17 + x;
					uint16_t p1 = idx - 9;
					uint16_t p2 = idx - 8;
					uint16_t p3 = idx + 9;
					uint16_t p4 = idx + 8;
					indices[indices_pos++] = p2;
					indices[indices_pos++] = p1;
					indices[indices_pos++] = idx;
					indices[indices_pos++] = p3;
					indices[indices_pos++] = p2;
					indices[indices_pos++] = idx;
					indices[indices_pos++] = p4;
					indices[indices_pos++] = p3;
					indices[indices_pos++] = idx;
					indices[indices_pos++] = p1;
					indices[indices_pos++] = p4;
					indices[indices_pos++] = idx;
				}
			}
			for (size_t z = 0; z < 8; ++z)
			{
				for (size_t x = 0; x < 8; ++x)
				{
					uint16_t idx = base + 9 + z * 17 + x;
					uint16_t p1 = idx - 9;
					uint16_t p2 = idx - 8;
					uint16_t p3 = idx + 9;
					uint16_t p4 = idx + 8;
					indices[indices_pos++] = p2;
					indices[indices_pos++] = p1;
					indices[indices_pos++] = p3;
					indices[indices_pos++] = p3;
					indices[indices_pos++] = p1;
					indices[indices_pos++] = p4;
				}
			}
			static const uint16_t points[144] =
			{
				1  , 0  , 17 , 36 , 17 , 34 , 36 , 2  , 1  , 36 , 1  , 17,
				36 , 3  , 2  , 38 , 4  , 3  , 38 , 3  , 36,
				38 , 5  , 4  , 40 , 6  , 5  , 40 , 5  , 38,
				25 , 8  , 7  , 40 , 42 , 25 , 40 , 7  , 6  , 40 , 25 , 7,
				70 , 51 , 68 , 36 , 34 , 51 , 36 , 51 , 70,
				36 , 70 , 72 , 72 , 38 , 36 ,
				40 , 38 , 72 , 72 , 74 , 40 ,
				40 , 59 , 42 , 74 , 76 , 59 , 74 , 59 , 40,
				104, 85 , 102, 70 , 68 , 85 , 70 , 85 , 104,
				104, 106, 72 , 72 , 70 , 104,
				72 , 106, 108, 108, 74 , 72 ,
				74 , 93 , 76 , 108, 110, 93 , 108, 93 , 74,
				119, 136, 137, 104, 102, 119, 104, 137, 138, 104, 119, 137,
				106, 139, 140, 104, 138, 139, 104, 139, 106,
				108, 141, 142, 106, 140, 141, 106, 141, 108,
				143, 144, 127, 108, 142, 143, 108, 127, 110, 108, 143, 127
			};
			for (size_t i = 0; i < 144; ++i)
				indices[indices_pos++] = base + points[i];
		}
	}
	gx->mcnk_indices_nb = indices_pos;
	gfx_create_buffer(gx->device,
	                  &gx->mcnk_vertexes_buffer,
	                  GFX_BUFFER_VERTEXES,
	                  vertexes,
	                  points_nb * sizeof(*vertexes),
	                  GFX_BUFFER_IMMUTABLE);
	gfx_create_buffer(gx->device,
	                  &gx->mcnk_indices_buffer,
	                  GFX_BUFFER_INDICES,
	                  indices,
	                  indices_pos * sizeof(*indices),
	                  GFX_BUFFER_IMMUTABLE);
	mem_free(MEM_GX, vertexes);
	mem_free(MEM_GX, indices);
}

static void
init_particles_buffers(struct gx *gx)
{
	uint16_t *indices = mem_malloc(MEM_GX, sizeof(*indices) * MAX_PARTICLES * 6);
	if (!indices)
		abort(); /* XXX */
	for (size_t i = 0; i < MAX_PARTICLES; ++i)
	{
		uint16_t *tmp = &indices[i * 6];
		size_t n = i * 4;
		tmp[0] = n + 0;
		tmp[1] = n + 1;
		tmp[2] = n + 2;
		tmp[3] = n + 0;
		tmp[4] = n + 2;
		tmp[5] = n + 3;
	}
	gfx_create_buffer(gx->device,
	                  &gx->particles_indices_buffer,
	                  GFX_BUFFER_INDICES,
	                  indices,
	                  6 * MAX_PARTICLES * sizeof(*indices),
	                  GFX_BUFFER_IMMUTABLE);
	mem_free(MEM_GX, indices);
}

static void
init_aabb_buffers(struct gx *gx)
{
	static const struct vec3f vertexes[8] =
	{
		{0, 0, 0},
		{1, 0, 0},
		{0, 1, 0},
		{1, 1, 0},
		{0, 0, 1},
		{1, 0, 1},
		{0, 1, 1},
		{1, 1, 1},
	};
	static const uint16_t indices[24] =
	{
		0, 1, 1, 3,
		3, 2, 2, 0,
		0, 4, 1, 5,
		2, 6, 3, 7,
		4, 5, 5, 7,
		7, 6, 6, 4,
	};
	gfx_create_buffer(gx->device,
	                  &gx->aabb_vertexes_buffer,
	                  GFX_BUFFER_VERTEXES,
	                  vertexes,
	                  sizeof(vertexes),
	                  GFX_BUFFER_IMMUTABLE);
	gfx_create_buffer(gx->device,
	                  &gx->aabb_indices_buffer,
	                  GFX_BUFFER_INDICES,
	                  indices,
	                  sizeof(indices),
	                  GFX_BUFFER_IMMUTABLE);
}

struct gx *
gx_new(struct gfx_device *device)
{
	struct gx *gx;

	gx = mem_zalloc(MEM_GENERIC, sizeof(*gx));
	if (!gx)
		return NULL;
	gx->opt |= GX_OPT_MCNK;
	gx->opt |= GX_OPT_MCLQ;
	gx->opt |= GX_OPT_WMO;
	gx->opt |= GX_OPT_WDL;
	gx->opt |= GX_OPT_M2;
	gx->opt |= GX_OPT_FOG;
	gx->opt |= GX_OPT_WMO_LIQUIDS;
	gx->opt |= GX_OPT_SKYBOX;
	gx->opt |= GX_OPT_M2_PARTICLES;
	gx->opt |= GX_OPT_M2_RIBBONS;
	gx->opt |= GX_OPT_GROUND_EFFECT;
	gx->opt |= GX_OPT_DYN_SHADOW;
	gx->device = device;
	for (size_t i = 0; i < GX_BLEND_LAST; ++i)
		gx->blend_states[i] = GFX_BLEND_STATE_INIT();
	gfx_create_blend_state(gx->device,
	                       &gx->blend_states[GX_BLEND_OPAQUE],
	                       false,
	                       GFX_BLEND_ONE,
	                       GFX_BLEND_ZERO,
	                       GFX_BLEND_ONE,
	                       GFX_BLEND_ZERO,
	                       GFX_EQUATION_ADD,
	                       GFX_EQUATION_ADD,
	                       GFX_COLOR_MASK_ALL);
	gfx_create_blend_state(gx->device,
	                       &gx->blend_states[GX_BLEND_ALPHA],
	                       true,
	                       GFX_BLEND_SRC_ALPHA,
	                       GFX_BLEND_ONE_MINUS_SRC_ALPHA,
	                       GFX_BLEND_SRC_ALPHA,
	                       GFX_BLEND_ONE_MINUS_SRC_ALPHA,
	                       GFX_EQUATION_ADD,
	                       GFX_EQUATION_ADD,
	                       GFX_COLOR_MASK_ALL);
	gfx_create_blend_state(gx->device,
	                       &gx->blend_states[GX_BLEND_NO_ALPHA_ADD],
	                       true,
	                       GFX_BLEND_ONE,
	                       GFX_BLEND_ONE,
	                       GFX_BLEND_ONE,
	                       GFX_BLEND_ONE,
	                       GFX_EQUATION_ADD,
	                       GFX_EQUATION_ADD,
	                       GFX_COLOR_MASK_ALL);
	gfx_create_blend_state(gx->device,
	                       &gx->blend_states[GX_BLEND_ADD],
	                       true,
	                       GFX_BLEND_SRC_ALPHA,
	                       GFX_BLEND_ONE,
	                       GFX_BLEND_SRC_ALPHA,
	                       GFX_BLEND_ONE,
	                       GFX_EQUATION_ADD,
	                       GFX_EQUATION_ADD,
	                       GFX_COLOR_MASK_ALL);
	gfx_create_blend_state(gx->device,
	                       &gx->blend_states[GX_BLEND_MOD],
	                       true,
	                       GFX_BLEND_DST_COLOR,
	                       GFX_BLEND_ZERO,
	                       GFX_BLEND_DST_ALPHA,
	                       GFX_BLEND_ZERO,
	                       GFX_EQUATION_ADD,
	                       GFX_EQUATION_ADD,
	                       GFX_COLOR_MASK_ALL);
	gfx_create_blend_state(gx->device,
	                       &gx->blend_states[GX_BLEND_MOD2X],
	                       true,
	                       GFX_BLEND_DST_COLOR,
	                       GFX_BLEND_SRC_COLOR,
	                       GFX_BLEND_DST_ALPHA,
	                       GFX_BLEND_SRC_ALPHA,
	                       GFX_EQUATION_ADD,
	                       GFX_EQUATION_ADD,
	                       GFX_COLOR_MASK_ALL);
	for (enum gx_depth_stencil_state i = 0; i < GX_DEPTH_STENCIL_LAST; ++i)
		gx->depth_stencil_states[i] = GFX_DEPTH_STENCIL_STATE_INIT();
	gfx_create_depth_stencil_state(gx->device,
	                               &gx->depth_stencil_states[GX_DEPTH_STENCIL_NO_NO],
	                               false,
	                               false,
	                               GFX_CMP_LEQUAL,
	                               false,
	                               0,
	                               GFX_CMP_NEVER,
	                               0,
	                               0,
	                               GFX_STENCIL_KEEP,
	                               GFX_STENCIL_KEEP,
	                               GFX_STENCIL_KEEP);
	gfx_create_depth_stencil_state(gx->device,
	                               &gx->depth_stencil_states[GX_DEPTH_STENCIL_NO_RO],
	                               false,
	                               false,
	                               GFX_CMP_LEQUAL,
	                               true,
	                               0,
	                               GFX_CMP_NOTEQUAL,
	                               1,
	                               -1,
	                               GFX_STENCIL_KEEP,
	                               GFX_STENCIL_KEEP,
	                               GFX_STENCIL_KEEP);
	gfx_create_depth_stencil_state(gx->device,
	                               &gx->depth_stencil_states[GX_DEPTH_STENCIL_NO_WO],
	                               false,
	                               false,
	                               GFX_CMP_LEQUAL,
	                               true,
	                               -1,
	                               GFX_CMP_ALWAYS,
	                               1,
	                               -1,
	                               GFX_STENCIL_KEEP,
	                               GFX_STENCIL_KEEP,
	                               GFX_STENCIL_REPLACE);
	gfx_create_depth_stencil_state(gx->device,
	                               &gx->depth_stencil_states[GX_DEPTH_STENCIL_NO_RW],
	                               false,
	                               false,
	                               GFX_CMP_LEQUAL,
	                               true ,
	                               -1,
	                               GFX_CMP_NOTEQUAL,
	                               1,
	                               -1,
	                               GFX_STENCIL_KEEP,
	                               GFX_STENCIL_KEEP,
	                               GFX_STENCIL_REPLACE);
	gfx_create_depth_stencil_state(gx->device,
	                               &gx->depth_stencil_states[GX_DEPTH_STENCIL_RO_NO],
	                               false,
	                               true,
	                               GFX_CMP_LEQUAL,
	                               false,
	                               0,
	                               GFX_CMP_NEVER,
	                               0,
	                               0,
	                               GFX_STENCIL_KEEP,
	                               GFX_STENCIL_KEEP,
	                               GFX_STENCIL_KEEP);
	gfx_create_depth_stencil_state(gx->device,
	                               &gx->depth_stencil_states[GX_DEPTH_STENCIL_RO_RO],
	                               false,
	                               true,
	                               GFX_CMP_LEQUAL,
	                               true,
	                               0,
	                               GFX_CMP_NOTEQUAL,
	                               1,
	                               -1,
	                               GFX_STENCIL_KEEP,
	                               GFX_STENCIL_KEEP,
	                               GFX_STENCIL_KEEP);
	gfx_create_depth_stencil_state(gx->device,
	                               &gx->depth_stencil_states[GX_DEPTH_STENCIL_RO_WO],
	                               false,
	                               true,
	                               GFX_CMP_LEQUAL,
	                               true,
	                               -1,
	                               GFX_CMP_ALWAYS,
	                               1,
	                               -1,
	                               GFX_STENCIL_KEEP,
	                               GFX_STENCIL_KEEP,
	                               GFX_STENCIL_REPLACE);
	gfx_create_depth_stencil_state(gx->device,
	                               &gx->depth_stencil_states[GX_DEPTH_STENCIL_RO_RW],
	                               false,
	                               true,
	                               GFX_CMP_LEQUAL,
	                               true,
	                               -1,
	                               GFX_CMP_NOTEQUAL,
	                               1,
	                               -1,
	                               GFX_STENCIL_KEEP,
	                               GFX_STENCIL_KEEP,
	                               GFX_STENCIL_REPLACE);
	gfx_create_depth_stencil_state(gx->device,
	                               &gx->depth_stencil_states[GX_DEPTH_STENCIL_WO_NO],
	                               true,
	                               false,
	                               GFX_CMP_LEQUAL,
	                               false,
	                               0,
	                               GFX_CMP_NEVER,
	                               0,
	                               0,
	                               GFX_STENCIL_KEEP,
	                               GFX_STENCIL_KEEP,
	                               GFX_STENCIL_KEEP);
	gfx_create_depth_stencil_state(gx->device,
	                               &gx->depth_stencil_states[GX_DEPTH_STENCIL_WO_RO],
	                               true,
	                               false,
	                               GFX_CMP_LEQUAL,
	                               true,
	                               0,
	                               GFX_CMP_NOTEQUAL,
	                               1,
	                               -1,
	                               GFX_STENCIL_KEEP,
	                               GFX_STENCIL_KEEP,
	                               GFX_STENCIL_KEEP);
	gfx_create_depth_stencil_state(gx->device,
	                               &gx->depth_stencil_states[GX_DEPTH_STENCIL_WO_WO],
	                               true,
	                               false,
	                               GFX_CMP_LEQUAL,
	                               true,
	                               -1,
	                               GFX_CMP_ALWAYS,
	                               1,
	                               -1,
	                               GFX_STENCIL_KEEP,
	                               GFX_STENCIL_KEEP,
	                               GFX_STENCIL_REPLACE);
	gfx_create_depth_stencil_state(gx->device,
	                               &gx->depth_stencil_states[GX_DEPTH_STENCIL_WO_RW],
	                               true,
	                               false,
	                               GFX_CMP_LEQUAL,
	                               true,
	                               -1,
	                               GFX_CMP_NOTEQUAL,
	                               1,
	                               -1,
	                               GFX_STENCIL_KEEP,
	                               GFX_STENCIL_KEEP,
	                               GFX_STENCIL_REPLACE);
	gfx_create_depth_stencil_state(gx->device,
	                               &gx->depth_stencil_states[GX_DEPTH_STENCIL_RW_NO],
	                               true,
	                               true,
	                               GFX_CMP_LEQUAL,
	                               false,
	                               0,
	                               GFX_CMP_NEVER,
	                               0,
	                               0,
	                               GFX_STENCIL_KEEP,
	                               GFX_STENCIL_KEEP,
	                               GFX_STENCIL_KEEP);
	gfx_create_depth_stencil_state(gx->device,
	                               &gx->depth_stencil_states[GX_DEPTH_STENCIL_RW_RO],
	                               true,
	                               true,
	                               GFX_CMP_LEQUAL,
	                               true,
	                               0,
	                               GFX_CMP_NOTEQUAL,
	                               1,
	                               -1,
	                               GFX_STENCIL_KEEP,
	                               GFX_STENCIL_KEEP,
	                               GFX_STENCIL_KEEP);
	gfx_create_depth_stencil_state(gx->device, &gx->depth_stencil_states[GX_DEPTH_STENCIL_RW_WO],
	                               true,
	                               true,
	                               GFX_CMP_LEQUAL,
	                               true,
	                               -1,
	                               GFX_CMP_ALWAYS,
	                               1,
	                               -1,
	                               GFX_STENCIL_KEEP,
	                               GFX_STENCIL_KEEP,
	                               GFX_STENCIL_REPLACE);
	gfx_create_depth_stencil_state(gx->device,
	                               &gx->depth_stencil_states[GX_DEPTH_STENCIL_RW_RW],
	                               true,
	                               true,
	                               GFX_CMP_LEQUAL,
	                               true,
	                               -1,
	                               GFX_CMP_NOTEQUAL,
	                               1,
	                               -1,
	                               GFX_STENCIL_KEEP,
	                               GFX_STENCIL_KEEP,
	                               GFX_STENCIL_REPLACE);
	for (enum gx_rasterizer_state i = 0; i < GX_RASTERIZER_LAST; ++i)
		gx->rasterizer_states[i] = GFX_RASTERIZER_STATE_INIT();
	gx_build_rasterizer_states(gx);
#ifdef WITH_DEBUG_RENDERING
	gx->collisions_triangles_rasterizer_state = GFX_RASTERIZER_STATE_INIT();
	gx->collisions_lines_rasterizer_state = GFX_RASTERIZER_STATE_INIT();
	gfx_create_rasterizer_state(gx->device,
	                            &gx->collisions_triangles_rasterizer_state,
	                            GFX_FILL_SOLID,
	                            GFX_CULL_NONE,
	                            GFX_FRONT_CCW,
	                            true);
	gfx_create_rasterizer_state(gx->device,
	                            &gx->collisions_lines_rasterizer_state,
	                            GFX_FILL_LINE,
	                            GFX_CULL_NONE,
	                            GFX_FRONT_CCW,
	                            true);
	gx->m2_bones_points_input_layout = GFX_INPUT_LAYOUT_INIT();
	gx->m2_bones_lines_input_layout = GFX_INPUT_LAYOUT_INIT();
	gx->wmo_collisions_input_layout = GFX_INPUT_LAYOUT_INIT();
	gx->m2_collisions_input_layout = GFX_INPUT_LAYOUT_INIT();
	gx->wmo_portals_input_layout = GFX_INPUT_LAYOUT_INIT();
	gx->collisions_input_layout = GFX_INPUT_LAYOUT_INIT();
	gx->wmo_lights_input_layout = GFX_INPUT_LAYOUT_INIT();
	gx->m2_lights_input_layout = GFX_INPUT_LAYOUT_INIT();
	gx->aabb_input_layout = GFX_INPUT_LAYOUT_INIT();
	gx->taxi_input_layout = GFX_INPUT_LAYOUT_INIT();
	gfx_create_input_layout(gx->device,
	                        &gx->m2_bones_points_input_layout,
	                        g_m2_bones_points_binds,
	                        sizeof(g_m2_bones_points_binds) / sizeof(*g_m2_bones_points_binds),
	                        &g_wow->shaders->m2_bones);
	gfx_create_input_layout(gx->device,
	                        &gx->m2_bones_lines_input_layout,
	                        g_m2_bones_lines_binds,
	                        sizeof(g_m2_bones_lines_binds) / sizeof(*g_m2_bones_lines_binds),
	                        &g_wow->shaders->m2_bones);
	gfx_create_input_layout(gx->device,
	                        &gx->wmo_collisions_input_layout,
	                        g_wmo_collisions_binds,
	                        sizeof(g_wmo_collisions_binds) / sizeof(*g_wmo_collisions_binds),
	                        &g_wow->shaders->basic);
	gfx_create_input_layout(gx->device,
	                        &gx->m2_collisions_input_layout,
	                        g_m2_collisions_binds,
	                        sizeof(g_m2_collisions_binds) / sizeof(*g_m2_collisions_binds),
	                        &g_wow->shaders->basic);
	gfx_create_input_layout(gx->device,
	                        &gx->wmo_portals_input_layout,
	                        g_wmo_portals_binds,
	                        sizeof(g_wmo_portals_binds) / sizeof(*g_wmo_portals_binds),
	                        &g_wow->shaders->basic);
	gfx_create_input_layout(gx->device,
	                        &gx->collisions_input_layout,
	                        g_collisions_binds,
	                        sizeof(g_collisions_binds) / sizeof(*g_collisions_binds),
	                        &g_wow->shaders->collisions);
	gfx_create_input_layout(gx->device,
	                        &gx->wmo_lights_input_layout,
	                        g_wmo_lights_binds,
	                        sizeof(g_wmo_lights_binds) / sizeof(*g_wmo_lights_binds),
	                        &g_wow->shaders->basic);
	gfx_create_input_layout(gx->device,
	                        &gx->m2_lights_input_layout,
	                        g_m2_lights_binds,
	                        sizeof(g_m2_lights_binds) / sizeof(*g_m2_lights_binds),
	                        &g_wow->shaders->basic);
	gfx_create_input_layout(gx->device,
	                        &gx->aabb_input_layout,
	                        g_aabb_binds,
	                        sizeof(g_aabb_binds) / sizeof(*g_aabb_binds),
	                        &g_wow->shaders->aabb);
	gfx_create_input_layout(gx->device,
	                        &gx->taxi_input_layout,
	                        g_taxi_binds,
	                        sizeof(g_taxi_binds) / sizeof(*g_taxi_binds),
	                        &g_wow->shaders->taxi);
	gx->wmo_collisions_triangles_pipeline_state = GFX_PIPELINE_STATE_INIT();
	gx->m2_collisions_triangles_pipeline_state = GFX_PIPELINE_STATE_INIT();
	gx->collisions_triangles_pipeline_state = GFX_PIPELINE_STATE_INIT();
	gx->wmo_collisions_lines_pipeline_state = GFX_PIPELINE_STATE_INIT();
	gx->m2_collisions_lines_pipeline_state = GFX_PIPELINE_STATE_INIT();
	gx->collisions_lines_pipeline_state = GFX_PIPELINE_STATE_INIT();
	gx->m2_bones_points_pipeline_state = GFX_PIPELINE_STATE_INIT();
	gx->m2_bones_lines_pipeline_state = GFX_PIPELINE_STATE_INIT();
	gx->wmo_portals_pipeline_state = GFX_PIPELINE_STATE_INIT();
	gx->wmo_lights_pipeline_state = GFX_PIPELINE_STATE_INIT();
	gx->m2_lights_pipeline_state = GFX_PIPELINE_STATE_INIT();
	gx->aabb_pipeline_state = GFX_PIPELINE_STATE_INIT();
	gx->taxi_pipeline_state = GFX_PIPELINE_STATE_INIT();
	gfx_create_pipeline_state(gx->device,
	                          &gx->wmo_collisions_triangles_pipeline_state,
	                          &g_wow->shaders->wmo_collisions,
	                          &gx->collisions_triangles_rasterizer_state,
	                          &gx->depth_stencil_states[GX_DEPTH_STENCIL_RO_NO],
	                          &gx->blend_states[GX_BLEND_ALPHA],
	                          &gx->wmo_collisions_input_layout,
	                          GFX_PRIMITIVE_TRIANGLES);
	gfx_create_pipeline_state(gx->device,
	                          &gx->m2_collisions_triangles_pipeline_state,
	                          &g_wow->shaders->m2_collisions,
	                          &gx->collisions_triangles_rasterizer_state,
	                          &gx->depth_stencil_states[GX_DEPTH_STENCIL_RO_NO],
	                          &gx->blend_states[GX_BLEND_ALPHA],
	                          &gx->m2_collisions_input_layout,
	                          GFX_PRIMITIVE_TRIANGLES);
	gfx_create_pipeline_state(gx->device,
	                          &gx->collisions_triangles_pipeline_state,
	                          &g_wow->shaders->collisions,
	                          &gx->collisions_triangles_rasterizer_state,
	                          &gx->depth_stencil_states[GX_DEPTH_STENCIL_NO_NO],
	                          &gx->blend_states[GX_BLEND_ALPHA],
	                          &gx->collisions_input_layout,
	                          GFX_PRIMITIVE_TRIANGLES);
	gfx_create_pipeline_state(gx->device,
	                          &gx->wmo_collisions_lines_pipeline_state,
	                          &g_wow->shaders->wmo_collisions,
	                          &gx->collisions_lines_rasterizer_state,
	                          &gx->depth_stencil_states[GX_DEPTH_STENCIL_RO_NO],
	                          &gx->blend_states[GX_BLEND_ALPHA],
	                          &gx->wmo_collisions_input_layout,
	                          GFX_PRIMITIVE_TRIANGLES);
	gfx_create_pipeline_state(gx->device,
	                          &gx->m2_collisions_lines_pipeline_state,
	                          &g_wow->shaders->m2_collisions,
	                          &gx->collisions_lines_rasterizer_state,
	                          &gx->depth_stencil_states[GX_DEPTH_STENCIL_RO_NO],
	                          &gx->blend_states[GX_BLEND_ALPHA],
	                          &gx->m2_collisions_input_layout,
	                          GFX_PRIMITIVE_TRIANGLES);
	gfx_create_pipeline_state(gx->device,
	                          &gx->collisions_lines_pipeline_state,
	                          &g_wow->shaders->collisions,
	                          &gx->collisions_lines_rasterizer_state,
	                          &gx->depth_stencil_states[GX_DEPTH_STENCIL_NO_NO],
	                          &gx->blend_states[GX_BLEND_ALPHA],
	                          &gx->collisions_input_layout,
	                          GFX_PRIMITIVE_TRIANGLES);
	gfx_create_pipeline_state(gx->device,
	                          &gx->m2_bones_points_pipeline_state,
	                          &g_wow->shaders->m2_bones,
	                          &gx->rasterizer_states[GX_RASTERIZER_UNCULLED],
	                          &gx->depth_stencil_states[GX_DEPTH_STENCIL_NO_NO],
	                          &gx->blend_states[GX_BLEND_ALPHA],
	                          &gx->m2_bones_points_input_layout,
	                          GFX_PRIMITIVE_POINTS);
	gfx_create_pipeline_state(gx->device,
	                          &gx->m2_bones_lines_pipeline_state,
	                          &g_wow->shaders->m2_bones,
	                          &gx->rasterizer_states[GX_RASTERIZER_UNCULLED],
	                          &gx->depth_stencil_states[GX_DEPTH_STENCIL_NO_NO],
	                          &gx->blend_states[GX_BLEND_ALPHA],
	                          &gx->m2_bones_lines_input_layout,
	                          GFX_PRIMITIVE_LINES);
	gfx_create_pipeline_state(gx->device,
	                          &gx->wmo_portals_pipeline_state,
	                          &g_wow->shaders->wmo_portals,
	                          &gx->rasterizer_states[GX_RASTERIZER_UNCULLED],
	                          &gx->depth_stencil_states[GX_DEPTH_STENCIL_RO_NO],
	                          &gx->blend_states[GX_BLEND_ALPHA],
	                          &gx->wmo_portals_input_layout,
	                          GFX_PRIMITIVE_TRIANGLES);
	gfx_create_pipeline_state(gx->device,
	                          &gx->wmo_lights_pipeline_state,
	                          &g_wow->shaders->basic,
	                          &gx->rasterizer_states[GX_RASTERIZER_UNCULLED],
	                          &gx->depth_stencil_states[GX_DEPTH_STENCIL_NO_NO],
	                          &gx->blend_states[GX_BLEND_ALPHA],
	                          &gx->wmo_lights_input_layout,
	                          GFX_PRIMITIVE_POINTS);
	gfx_create_pipeline_state(gx->device,
	                          &gx->m2_lights_pipeline_state,
	                          &g_wow->shaders->m2_lights,
	                          &gx->rasterizer_states[GX_RASTERIZER_UNCULLED],
	                          &gx->depth_stencil_states[GX_DEPTH_STENCIL_NO_NO],
	                          &gx->blend_states[GX_BLEND_ALPHA],
	                          &gx->m2_lights_input_layout,
	                          GFX_PRIMITIVE_POINTS);
	gfx_create_pipeline_state(gx->device,
	                          &gx->aabb_pipeline_state,
	                          &g_wow->shaders->aabb,
	                          &gx->rasterizer_states[GX_RASTERIZER_UNCULLED],
	                          &gx->depth_stencil_states[GX_DEPTH_STENCIL_RO_NO],
	                          &gx->blend_states[GX_BLEND_ALPHA],
	                          &gx->aabb_input_layout,
	                          GFX_PRIMITIVE_LINES);
	gfx_create_pipeline_state(gx->device,
	                          &gx->taxi_pipeline_state,
	                          &g_wow->shaders->taxi,
	                          &gx->rasterizer_states[GX_RASTERIZER_UNCULLED],
	                          &gx->depth_stencil_states[GX_DEPTH_STENCIL_RO_NO],
	                          &gx->blend_states[GX_BLEND_ALPHA],
	                          &gx->taxi_input_layout,
	                          GFX_PRIMITIVE_LINES);
#endif
	gx->wmo_colored_input_layout = GFX_INPUT_LAYOUT_INIT();
	gx->mclq_water_input_layout = GFX_INPUT_LAYOUT_INIT();
	gx->mclq_magma_input_layout = GFX_INPUT_LAYOUT_INIT();
	gx->particles_input_layout = GFX_INPUT_LAYOUT_INIT();
	gx->m2_ground_input_layout = GFX_INPUT_LAYOUT_INIT();
	gx->wmo_mliq_input_layout = GFX_INPUT_LAYOUT_INIT();
	gx->ribbons_input_layout = GFX_INPUT_LAYOUT_INIT();
	gx->skybox_input_layout = GFX_INPUT_LAYOUT_INIT();
	gx->mcnk_input_layout = GFX_INPUT_LAYOUT_INIT();
	gx->text_input_layout = GFX_INPUT_LAYOUT_INIT();
	gx->wdl_input_layout = GFX_INPUT_LAYOUT_INIT();
	gx->wmo_input_layout = GFX_INPUT_LAYOUT_INIT();
	gx->m2_input_layout = GFX_INPUT_LAYOUT_INIT();
	gfx_create_input_layout(gx->device,
	                        &gx->wmo_colored_input_layout,
	                        g_wmo_colored_binds,
	                        sizeof(g_wmo_colored_binds) / sizeof(*g_wmo_colored_binds),
	                        &g_wow->shaders->wmo);
	gfx_create_input_layout(gx->device,
	                        &gx->mclq_water_input_layout,
	                        g_mclq_water_binds,
	                        sizeof(g_mclq_water_binds) / sizeof(*g_mclq_water_binds),
	                        &g_wow->shaders->mclq_water);
	gfx_create_input_layout(gx->device,
	                        &gx->mclq_magma_input_layout,
	                        g_mclq_magma_binds,
	                        sizeof(g_mclq_magma_binds) / sizeof(*g_mclq_magma_binds),
	                        &g_wow->shaders->mclq_magma);
	gfx_create_input_layout(gx->device,
	                        &gx->particles_input_layout,
	                        g_particles_binds,
	                        sizeof(g_particles_binds) / sizeof(*g_particles_binds),
	                        &g_wow->shaders->particle);
	gfx_create_input_layout(gx->device,
	                        &gx->m2_ground_input_layout,
	                        g_m2_ground_binds,
	                        sizeof(g_m2_ground_binds) / sizeof(*g_m2_ground_binds),
	                        &g_wow->shaders->m2_ground);
	gfx_create_input_layout(gx->device,
	                        &gx->wmo_mliq_input_layout,
	                        g_wmo_mliq_binds,
	                        sizeof(g_wmo_mliq_binds) / sizeof(*g_wmo_mliq_binds),
	                        &g_wow->shaders->mliq);
	gfx_create_input_layout(gx->device,
	                        &gx->ribbons_input_layout,
	                        g_ribbons_binds,
	                        sizeof(g_ribbons_binds) / sizeof(*g_ribbons_binds),
	                        &g_wow->shaders->ribbon);
	gfx_create_input_layout(gx->device,
	                        &gx->skybox_input_layout,
	                        g_skybox_binds,
	                        sizeof(g_skybox_binds) / sizeof(*g_skybox_binds),
	                        &g_wow->shaders->skybox);
	gfx_create_input_layout(gx->device,
	                        &gx->mcnk_input_layout,
	                        g_mcnk_binds,
	                        sizeof(g_mcnk_binds) / sizeof(*g_mcnk_binds),
	                        &g_wow->shaders->mcnk);
	gfx_create_input_layout(gx->device,
	                        &gx->text_input_layout,
	                        g_text_binds,
	                        sizeof(g_text_binds) / sizeof(*g_text_binds),
	                        &g_wow->shaders->text);
	gfx_create_input_layout(gx->device,
	                        &gx->wdl_input_layout,
	                        g_wdl_binds,
	                        sizeof(g_wdl_binds) / sizeof(*g_wdl_binds),
	                        &g_wow->shaders->wdl);
	gfx_create_input_layout(gx->device,
	                        &gx->wmo_input_layout,
	                        g_wmo_binds,
	                        sizeof(g_wmo_binds) / sizeof(*g_wmo_binds),
	                        &g_wow->shaders->wmo);
	gfx_create_input_layout(gx->device,
	                        &gx->m2_input_layout,
	                        g_m2_binds,
	                        sizeof(g_m2_binds) / sizeof(*g_m2_binds),
	                        &g_wow->shaders->m2);
	gx->mclq_water_dyn_pipeline_state = GFX_PIPELINE_STATE_INIT();
	gx->mclq_water_pipeline_state = GFX_PIPELINE_STATE_INIT();
	gx->mclq_magma_pipeline_state = GFX_PIPELINE_STATE_INIT();
	gx->m2_ground_pipeline_state = GFX_PIPELINE_STATE_INIT();
	gx->wmo_mliq_pipeline_state = GFX_PIPELINE_STATE_INIT();
	gx->skybox_pipeline_state = GFX_PIPELINE_STATE_INIT();
	gx->mcnk_pipeline_state = GFX_PIPELINE_STATE_INIT();
	gx->text_pipeline_state = GFX_PIPELINE_STATE_INIT();
	gx->wdl_pipeline_state = GFX_PIPELINE_STATE_INIT();
	gfx_create_pipeline_state(gx->device,
	                          &gx->mclq_water_dyn_pipeline_state,
	                          &g_wow->shaders->mclq_water_dyn,
	                          &gx->rasterizer_states[GX_RASTERIZER_CULLED],
	                          &gx->depth_stencil_states[GX_DEPTH_STENCIL_RW_NO],
	                          &gx->blend_states[GX_BLEND_ALPHA],
	                          &gx->mclq_water_input_layout,
	                          GFX_PRIMITIVE_TRIANGLES);
	gfx_create_pipeline_state(gx->device,
	                          &gx->mclq_water_pipeline_state,
	                          &g_wow->shaders->mclq_water,
	                          &gx->rasterizer_states[GX_RASTERIZER_CULLED],
	                          &gx->depth_stencil_states[GX_DEPTH_STENCIL_RW_NO],
	                          &gx->blend_states[GX_BLEND_ALPHA],
	                          &gx->mclq_water_input_layout,
	                          GFX_PRIMITIVE_TRIANGLES);
	gfx_create_pipeline_state(gx->device,
	                          &gx->mclq_magma_pipeline_state,
	                          &g_wow->shaders->mclq_magma,
	                          &gx->rasterizer_states[GX_RASTERIZER_CULLED],
	                          &gx->depth_stencil_states[GX_DEPTH_STENCIL_RW_NO],
	                          &gx->blend_states[GX_BLEND_ALPHA],
	                          &gx->mclq_magma_input_layout,
	                          GFX_PRIMITIVE_TRIANGLES);
	gfx_create_pipeline_state(gx->device,
	                          &gx->m2_ground_pipeline_state,
	                          &g_wow->shaders->m2_ground,
	                          &gx->rasterizer_states[GX_RASTERIZER_UNCULLED],
	                          &gx->depth_stencil_states[GX_DEPTH_STENCIL_RW_WO],
	                          &gx->blend_states[GX_BLEND_OPAQUE],
	                          &gx->m2_ground_input_layout,
	                          GFX_PRIMITIVE_TRIANGLES);
	gfx_create_pipeline_state(gx->device,
	                          &gx->wmo_mliq_pipeline_state,
	                          &g_wow->shaders->mliq,
	                          &gx->rasterizer_states[GX_RASTERIZER_CULLED],
	                          &gx->depth_stencil_states[GX_DEPTH_STENCIL_RO_NO],
	                          &gx->blend_states[GX_BLEND_ALPHA],
	                          &gx->wmo_mliq_input_layout,
	                          GFX_PRIMITIVE_TRIANGLES);
	gfx_create_pipeline_state(gx->device,
	                          &gx->skybox_pipeline_state,
	                          &g_wow->shaders->skybox,
	                          &gx->rasterizer_states[GX_RASTERIZER_CULLED],
	                          &gx->depth_stencil_states[GX_DEPTH_STENCIL_NO_RO],
	                          &gx->blend_states[GX_BLEND_ALPHA],
	                          &gx->skybox_input_layout,
	                          GFX_PRIMITIVE_TRIANGLES);
	gfx_create_pipeline_state(gx->device,
	                          &gx->mcnk_pipeline_state,
	                          &g_wow->shaders->mcnk,
	                          &gx->rasterizer_states[GX_RASTERIZER_CULLED],
	                          &gx->depth_stencil_states[GX_DEPTH_STENCIL_RW_WO],
	                          &gx->blend_states[GX_BLEND_ALPHA],
	                          &gx->mcnk_input_layout,
	                          GFX_PRIMITIVE_TRIANGLES);
	gfx_create_pipeline_state(gx->device,
	                          &gx->text_pipeline_state,
	                          &g_wow->shaders->text,
	                          &gx->rasterizer_states[GX_RASTERIZER_UNCULLED],
	                          &gx->depth_stencil_states[GX_DEPTH_STENCIL_RO_NO],
	                          &gx->blend_states[GX_BLEND_ALPHA],
	                          &gx->text_input_layout,
	                          GFX_PRIMITIVE_TRIANGLES);
	gfx_create_pipeline_state(gx->device,
	                          &gx->wdl_pipeline_state,
	                          &g_wow->shaders->wdl,
	                          &gx->rasterizer_states[GX_RASTERIZER_CULLED],
	                          &gx->depth_stencil_states[GX_DEPTH_STENCIL_NO_RW],
	                          &gx->blend_states[GX_BLEND_ALPHA],
	                          &gx->wdl_input_layout,
	                          GFX_PRIMITIVE_TRIANGLES);
	for (enum gx_rasterizer_state rasterizer = 0; rasterizer < GX_RASTERIZER_LAST; ++rasterizer)
	{
		for (enum gx_blend_state blend = 0; blend < GX_BLEND_LAST; ++blend)
		{
			gx->wmo_colored_pipeline_states[rasterizer][blend] = GFX_PIPELINE_STATE_INIT();
			gfx_create_pipeline_state(gx->device,
			                          &gx->wmo_colored_pipeline_states[rasterizer][blend],
			                          &g_wow->shaders->wmo,
			                          &gx->rasterizer_states[rasterizer],
			                          &gx->depth_stencil_states[GX_DEPTH_STENCIL_RW_WO],
			                          &gx->blend_states[blend],
			                          &gx->wmo_colored_input_layout,
			                          GFX_PRIMITIVE_TRIANGLES);
		}
	}
	for (enum gx_blend_state blend = 0; blend < GX_BLEND_LAST; ++blend)
	{
		gx->particles_pipeline_states[blend] = GFX_PIPELINE_STATE_INIT();
		gfx_create_pipeline_state(gx->device,
		                          &gx->particles_pipeline_states[blend],
		                          &g_wow->shaders->particle,
		                          &gx->rasterizer_states[GX_RASTERIZER_UNCULLED],
		                          &gx->depth_stencil_states[GX_DEPTH_STENCIL_RO_NO],
		                          &gx->blend_states[blend],
		                          &gx->particles_input_layout,
		                          GFX_PRIMITIVE_TRIANGLES);
	}
	for (enum gx_rasterizer_state rasterizer = 0; rasterizer < GX_RASTERIZER_LAST; ++rasterizer)
	{
		for (enum gx_blend_state blend = 0; blend < GX_BLEND_LAST; ++blend)
		{
			gx->wmo_pipeline_states[rasterizer][blend] = GFX_PIPELINE_STATE_INIT();
			gfx_create_pipeline_state(gx->device,
			                          &gx->wmo_pipeline_states[rasterizer][blend],
			                          &g_wow->shaders->wmo,
			                          &gx->rasterizer_states[rasterizer],
			                          &gx->depth_stencil_states[GX_DEPTH_STENCIL_RW_WO],
			                          &gx->blend_states[blend],
			                          &gx->wmo_input_layout,
			                          GFX_PRIMITIVE_TRIANGLES);
		}
	}
	for (enum gx_rasterizer_state rasterizer = 0; rasterizer < GX_RASTERIZER_LAST; ++rasterizer)
	{
		for (enum gx_depth_stencil_state depth_stencil = 0; depth_stencil < GX_DEPTH_STENCIL_LAST; ++depth_stencil)
		{
			for (enum gx_blend_state blend = 0; blend < GX_BLEND_LAST; ++blend)
			{
				gx->m2_pipeline_states[rasterizer][depth_stencil][blend] = GFX_PIPELINE_STATE_INIT();
				gfx_create_pipeline_state(gx->device,
				                          &gx->m2_pipeline_states[rasterizer][depth_stencil][blend],
				                          &g_wow->shaders->m2,
				                          &gx->rasterizer_states[rasterizer],
				                          &gx->depth_stencil_states[depth_stencil],
				                          &gx->blend_states[blend],
				                          &gx->m2_input_layout,
				                          GFX_PRIMITIVE_TRIANGLES);
				gx->ribbons_pipeline_states[rasterizer][depth_stencil][blend] = GFX_PIPELINE_STATE_INIT();
				gfx_create_pipeline_state(gx->device,
				                          &gx->ribbons_pipeline_states[rasterizer][depth_stencil][blend],
				                          &g_wow->shaders->ribbon,
				                          &gx->rasterizer_states[rasterizer],
				                          &gx->depth_stencil_states[depth_stencil],
				                          &gx->blend_states[blend],
				                          &gx->ribbons_input_layout,
				                          GFX_PRIMITIVE_TRIANGLE_STRIP);
			}
		}
	}
	gx->particles_indices_buffer = GFX_BUFFER_INIT();
	gx->mcnk_vertexes_buffer = GFX_BUFFER_INIT();
	gx->aabb_vertexes_buffer = GFX_BUFFER_INIT();
	gx->mcnk_indices_buffer = GFX_BUFFER_INIT();
	gx->aabb_indices_buffer = GFX_BUFFER_INIT();
	init_mcnk_buffers(gx);
	init_particles_buffers(gx);
	init_aabb_buffers(gx);
	for (uint32_t i = 0; i < sizeof(gx->lavag_textures) / sizeof(*gx->lavag_textures); ++i)
	{
		char filename[256];
		snprintf(filename, sizeof(filename), "XTEXTURES\\LAVAGREEN\\LAVAGREEN.%u.BLP", i + 1);
		if (cache_ref_blp(g_wow->cache, filename, &gx->lavag_textures[i]))
			gx_blp_ask_load(gx->lavag_textures[i]);
		else
			LOG_ERROR("failed to ref %s", filename);
	}
	for (uint32_t i = 0; i < sizeof(gx->river_textures) / sizeof(*gx->river_textures); ++i)
	{
		char filename[256];
		snprintf(filename, sizeof(filename), "XTEXTURES\\RIVER\\LAKE_A.%u.BLP", i + 1);
		if (cache_ref_blp(g_wow->cache, filename, &gx->river_textures[i]))
			gx_blp_ask_load(gx->river_textures[i]);
		else
			LOG_ERROR("failed to ref %s", filename);
	}
	for (uint32_t i = 0; i < sizeof(gx->ocean_textures) / sizeof(*gx->ocean_textures); ++i)
	{
		char filename[256];
		snprintf(filename, sizeof(filename), "XTEXTURES\\OCEAN\\OCEAN_H.%u.blp", i + 1);
		if (cache_ref_blp(g_wow->cache, filename, &gx->ocean_textures[i]))
			gx_blp_ask_load(gx->ocean_textures[i]);
		else
			LOG_ERROR("failed to ref %s", filename);
	}
	for (uint32_t i = 0; i < sizeof(gx->magma_textures) / sizeof(*gx->magma_textures); ++i)
	{
		char filename[256];
		snprintf(filename, sizeof(filename), "XTEXTURES\\LAVA\\LAVA.%u.BLP", i + 1);
		if (cache_ref_blp(g_wow->cache, filename, &gx->magma_textures[i]))
			gx_blp_ask_load(gx->magma_textures[i]);
		else
			LOG_ERROR("failed to ref %s", filename);
	}
	for (uint32_t i = 0; i < sizeof(gx->slime_textures) / sizeof(*gx->slime_textures); ++i)
	{
		char filename[256];
		snprintf(filename, sizeof(filename), "XTEXTURES\\SLIME\\SLIME.%u.BLP", i + 1);
		if (cache_ref_blp(g_wow->cache, filename, &gx->slime_textures[i]))
			gx_blp_ask_load(gx->slime_textures[i]);
		else
			LOG_ERROR("failed to ref %s", filename);
	}
	return gx;
}

void
gx_delete(struct gx *gx)
{
	if (!gx)
		return;
	for (enum gx_depth_stencil_state i = 0; i < GX_DEPTH_STENCIL_LAST; ++i)
		gfx_delete_depth_stencil_state(gx->device, &gx->depth_stencil_states[i]);
	for (enum gx_rasterizer_state i = 0; i < GX_RASTERIZER_LAST; ++i)
		gfx_delete_rasterizer_state(gx->device, &gx->rasterizer_states[i]);
	for (enum gx_blend_state i = 0; i < GX_BLEND_LAST; ++i)
		gfx_delete_blend_state(gx->device, &gx->blend_states[i]);
#ifdef WITH_DEBUG_RENDERING
	gfx_delete_rasterizer_state(gx->device, &gx->collisions_triangles_rasterizer_state);
	gfx_delete_rasterizer_state(gx->device, &gx->collisions_lines_rasterizer_state);
	gfx_delete_input_layout(gx->device, &gx->m2_bones_points_input_layout);
	gfx_delete_input_layout(gx->device, &gx->m2_bones_lines_input_layout);
	gfx_delete_input_layout(gx->device, &gx->wmo_collisions_input_layout);
	gfx_delete_input_layout(gx->device, &gx->m2_collisions_input_layout);
	gfx_delete_input_layout(gx->device, &gx->wmo_portals_input_layout);
	gfx_delete_input_layout(gx->device, &gx->collisions_input_layout);
	gfx_delete_input_layout(gx->device, &gx->wmo_lights_input_layout);
	gfx_delete_input_layout(gx->device, &gx->m2_lights_input_layout);
	gfx_delete_input_layout(gx->device, &gx->aabb_input_layout);
	gfx_delete_pipeline_state(gx->device, &gx->wmo_collisions_triangles_pipeline_state);
	gfx_delete_pipeline_state(gx->device, &gx->m2_collisions_triangles_pipeline_state);
	gfx_delete_pipeline_state(gx->device, &gx->collisions_triangles_pipeline_state);
	gfx_delete_pipeline_state(gx->device, &gx->wmo_collisions_lines_pipeline_state);
	gfx_delete_pipeline_state(gx->device, &gx->m2_collisions_lines_pipeline_state);
	gfx_delete_pipeline_state(gx->device, &gx->collisions_lines_pipeline_state);
	gfx_delete_pipeline_state(gx->device, &gx->m2_bones_points_pipeline_state);
	gfx_delete_pipeline_state(gx->device, &gx->m2_bones_lines_pipeline_state);
	gfx_delete_pipeline_state(gx->device, &gx->wmo_portals_pipeline_state);
	gfx_delete_pipeline_state(gx->device, &gx->wmo_lights_pipeline_state);
	gfx_delete_pipeline_state(gx->device, &gx->m2_lights_pipeline_state);
	gfx_delete_pipeline_state(gx->device, &gx->aabb_pipeline_state);
#endif
	gfx_delete_input_layout(gx->device, &gx->wmo_colored_input_layout);
	gfx_delete_input_layout(gx->device, &gx->mclq_water_input_layout);
	gfx_delete_input_layout(gx->device, &gx->mclq_magma_input_layout);
	gfx_delete_input_layout(gx->device, &gx->particles_input_layout);
	gfx_delete_input_layout(gx->device, &gx->m2_ground_input_layout);
	gfx_delete_input_layout(gx->device, &gx->wmo_mliq_input_layout);
	gfx_delete_input_layout(gx->device, &gx->ribbons_input_layout);
	gfx_delete_input_layout(gx->device, &gx->skybox_input_layout);
	gfx_delete_input_layout(gx->device, &gx->mcnk_input_layout);
	gfx_delete_input_layout(gx->device, &gx->wdl_input_layout);
	gfx_delete_input_layout(gx->device, &gx->wmo_input_layout);
	gfx_delete_input_layout(gx->device, &gx->m2_input_layout);
	gfx_delete_pipeline_state(gx->device, &gx->mclq_water_dyn_pipeline_state);
	gfx_delete_pipeline_state(gx->device, &gx->mclq_water_pipeline_state);
	gfx_delete_pipeline_state(gx->device, &gx->mclq_magma_pipeline_state);
	gfx_delete_pipeline_state(gx->device, &gx->m2_ground_pipeline_state);
	gfx_delete_pipeline_state(gx->device, &gx->wmo_mliq_pipeline_state);
	gfx_delete_pipeline_state(gx->device, &gx->skybox_pipeline_state);
	gfx_delete_pipeline_state(gx->device, &gx->mcnk_pipeline_state);
	gfx_delete_pipeline_state(gx->device, &gx->wdl_pipeline_state);
	for (enum gx_rasterizer_state rasterizer = 0; rasterizer < GX_RASTERIZER_LAST; ++rasterizer)
	{
		for (enum gx_blend_state blend = 0; blend < GX_BLEND_LAST; ++blend)
		{
			gfx_delete_pipeline_state(gx->device, &gx->wmo_colored_pipeline_states[rasterizer][blend]);
		}
	}
	for (enum gx_blend_state blend = 0; blend < GX_BLEND_LAST; ++blend)
	{
		gfx_delete_pipeline_state(gx->device, &gx->particles_pipeline_states[blend]);
	}
	for (enum gx_rasterizer_state rasterizer = 0; rasterizer < GX_RASTERIZER_LAST; ++rasterizer)
	{
		for (enum gx_blend_state blend = 0; blend < GX_BLEND_LAST; ++blend)
		{
			gfx_delete_pipeline_state(gx->device, &gx->wmo_pipeline_states[rasterizer][blend]);
		}
	}
	for (enum gx_rasterizer_state rasterizer = 0; rasterizer < GX_RASTERIZER_LAST; ++rasterizer)
	{
		for (enum gx_depth_stencil_state depth_stencil = 0; depth_stencil < GX_DEPTH_STENCIL_LAST; ++depth_stencil)
		{
			for (enum gx_blend_state blend = 0; blend < GX_BLEND_LAST; ++blend)
			{
				gfx_delete_pipeline_state(gx->device, &gx->m2_pipeline_states[rasterizer][depth_stencil][blend]);
				gfx_delete_pipeline_state(gx->device, &gx->ribbons_pipeline_states[rasterizer][depth_stencil][blend]);
			}
		}
	}
	gfx_delete_buffer(g_wow->device, &gx->particles_indices_buffer);
	gfx_delete_buffer(g_wow->device, &gx->mcnk_vertexes_buffer);
	gfx_delete_buffer(g_wow->device, &gx->aabb_vertexes_buffer);
	gfx_delete_buffer(g_wow->device, &gx->mcnk_indices_buffer);
	gfx_delete_buffer(g_wow->device, &gx->aabb_indices_buffer);
	for (uint32_t i = 0; i < sizeof(gx->lavag_textures) / sizeof(*gx->lavag_textures); ++i)
		gx_blp_free(gx->lavag_textures[i]);
	for (uint32_t i = 0; i < sizeof(gx->ocean_textures) / sizeof(*gx->ocean_textures); ++i)
		gx_blp_free(gx->ocean_textures[i]);
	for (uint32_t i = 0; i < sizeof(gx->river_textures) / sizeof(*gx->river_textures); ++i)
		gx_blp_free(gx->river_textures[i]);
	for (uint32_t i = 0; i < sizeof(gx->magma_textures) / sizeof(*gx->magma_textures); ++i)
		gx_blp_free(gx->magma_textures[i]);
	for (uint32_t i = 0; i < sizeof(gx->slime_textures) / sizeof(*gx->slime_textures); ++i)
		gx_blp_free(gx->slime_textures[i]);
	mem_free(MEM_GENERIC, gx);
}
