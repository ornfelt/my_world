#ifndef GX_COLLISIONS_H
#define GX_COLLISIONS_H

#include <gfx/objects.h>

#include <stddef.h>

struct collision_triangle;
struct gx_frame;

struct gx_collisions_frame
{
	gfx_attributes_state_t attributes_state;
	gfx_buffer_t triangles_uniform_buffer;
	gfx_buffer_t lines_uniform_buffer;
	gfx_buffer_t positions_buffer;
	struct vec3f *points;
	size_t points_nb;
	size_t buffer_size;
};

struct gx_collisions
{
	struct gx_collisions_frame frames[RENDER_FRAMES_COUNT];
	bool initialized;
};

void gx_collisions_init(struct gx_collisions *collisions);
void gx_collisions_destroy(struct gx_collisions *collisions);
void gx_collisions_update(struct gx_collisions *collisions, struct gx_frame *frame, struct collision_triangle *triangles, size_t triangles_nb);
void gx_collisions_render(struct gx_collisions *collisions, struct gx_frame *frame, bool triangles);

#endif
