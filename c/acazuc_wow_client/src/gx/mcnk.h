#ifndef GX_MCNK_H
#define GX_MCNK_H

#ifdef WITH_DEBUG_RENDERING
# include "gx/aabb.h"
#endif

#include <jks/frustum.h>
#include <jks/array.h>
#include <jks/aabb.h>
#include <jks/vec3.h>
#include <jks/vec2.h>

#include <gfx/objects.h>

#include <stdbool.h>

#define GX_MCNK_CHUNKS_PER_TILE    256
#define GX_MCNK_MAX_BATCH_TEXTURES 7

enum gx_mcnk_flag
{
	GX_MCNK_FLAG_INITIALIZED    = (1 << 0),
	GX_MCNK_FLAG_HOLES          = (1 << 1),
	GX_MCNK_FLAG_IN_RENDER_LIST = (1 << 2),
};

struct gx_m2_instance;
struct wow_adt_file;
struct map_tile;
struct gx_blp;
struct gx_m2;

struct gx_mcnk_chunk_frame
{
	float distance_to_camera;
	bool culled;
};

struct gx_mcnk_chunk
{
	struct gx_mcnk_chunk_frame frames[RENDER_FRAMES_COUNT];
	uint32_t indices_offsets[3];
	uint16_t indices_nbs[3];
	enum frustum_result frustum_result;
};

struct gx_mcnk_batch
{
	uint8_t chunks[GX_MCNK_CHUNKS_PER_TILE];
	uint16_t textures[GX_MCNK_MAX_BATCH_TEXTURES];
	uint16_t chunks_nb;
	uint8_t textures_nb;
};

struct gx_mcnk_frame
{
	gfx_buffer_t indirect_buffer;
	gfx_buffer_t uniform_buffer;
	struct gfx_draw_indexed_indirect_cmd draw_cmds[GX_MCNK_CHUNKS_PER_TILE];
	uint16_t batches_draw_cmds[GX_MCNK_CHUNKS_PER_TILE];
	size_t draw_cmd_nb;
	struct mat4f mvp;
	struct mat4f mv;
};

struct gx_mcnk
{
	struct map_tile *parent;
	struct gx_mcnk_frame frames[RENDER_FRAMES_COUNT];
	struct mcnk_init_data *init_data;
	struct gx_mcnk_chunk chunks[GX_MCNK_CHUNKS_PER_TILE];
	struct gx_blp **specular_textures;
	struct gx_blp **diffuse_textures;
	uint16_t textures_nb;
	gfx_attributes_state_t attributes_state;
	gfx_buffer_t chunks_uniform_buffer;
	gfx_buffer_t vertexes_buffer;
	gfx_buffer_t indices_buffer;
	gfx_texture_t alpha_texture;
	struct jks_array batches; /* gx_mcnk_batch */
	struct mat4f m;
	enum gx_mcnk_flag flags;
	uint8_t layers;
	float distance_to_camera;
};

struct gx_mcnk *gx_mcnk_new(struct map_tile *parent, struct wow_adt_file *file);
void gx_mcnk_delete(struct gx_mcnk *mcnk);
int gx_mcnk_initialize(struct gx_mcnk *mcnk);
void gx_mcnk_cull(struct gx_mcnk *mcnk, struct gx_frame *frame);
void gx_mcnk_render(struct gx_mcnk *mcnk, struct gx_frame *frame);

static inline bool gx_mcnk_flag_set(struct gx_mcnk *mcnk, enum gx_mcnk_flag flag)
{
	return (__atomic_fetch_or(&mcnk->flags, flag, __ATOMIC_RELAXED) & flag) != 0;
}

static inline bool gx_mcnk_flag_clear(struct gx_mcnk *mcnk, enum gx_mcnk_flag flag)
{
	return (__atomic_fetch_and(&mcnk->flags, ~flag, __ATOMIC_RELAXED) & flag) != 0;
}

#endif
