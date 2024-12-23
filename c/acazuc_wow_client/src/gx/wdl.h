#ifndef GX_WDL_H
#define GX_WDL_H

#ifdef WITH_DEBUG_RENDERING
# include "gx/aabb.h"
#endif

#include <jks/aabb.h>

#include <gfx/objects.h>

#include <stdbool.h>

struct wdl_init_data;
struct wow_wdl_file;

struct wdl_chunk
{
	uint32_t indices_offset;
#ifdef WITH_DEBUG_RENDERING
	struct gx_aabb gx_aabb;
#endif
	struct vec3f center;
	struct aabb aabb;
};

struct gx_wdl_frame
{
	gfx_buffer_t uniform_buffer;
	uint8_t culled[64 * 64 / 8];
};

struct gx_wdl
{
	struct wdl_init_data *init_data;
	struct gx_wdl_frame frames[RENDER_FRAMES_COUNT];
	struct wdl_chunk chunks[64 * 64];
	gfx_attributes_state_t attributes_state;
	gfx_buffer_t indices_buffer;
	gfx_buffer_t vertex_buffer;
	bool initialized;
};

struct gx_wdl *gx_wdl_new(struct wow_wdl_file *file);
void gx_wdl_delete(struct gx_wdl *wdl);
void gx_wdl_cull(struct gx_wdl *wdl, struct gx_frame *frame);
void gx_wdl_render(struct gx_wdl *wdl, struct gx_frame *frame);

#endif
