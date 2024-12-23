#ifndef GX_M2_RIBBONS_H
#define GX_M2_RIBBONS_H

#include <jks/array.h>
#include <jks/vec4.h>
#include <jks/vec3.h>
#include <jks/vec2.h>

#include <gfx/objects.h>

#define MAX_RIBBONS 512

struct gx_m2_render_params;
struct gx_m2_instance;
struct wow_m2_ribbon;
struct gx_frame;
struct gx_blp;

struct gx_m2_ribbon_point
{
	struct vec4f position;
	struct vec4f color;
	struct vec2f size;
	struct vec3f dir;
	uint64_t created;
};

struct gx_m2_ribbons_emitter_frame
{
	gfx_attributes_state_t attributes_state;
	gfx_buffer_t vertexes_buffer;
	gfx_buffer_t uniform_buffer;
	struct jks_array vertexes; /* struct shader_ribbon_input */
};

struct gx_m2_ribbons_emitter
{
	struct wow_m2_ribbon *emitter;
	struct gx_blp *texture;
	struct gx_m2_ribbons_emitter_frame frames[RENDER_FRAMES_COUNT];
	struct jks_array points; /* struct gx_m2_ribbon_point */
	uint32_t pipeline_state;
	uint64_t last_spawned;
	float alpha_test;
	struct vec3f fog_color;
	bool fog_override;
};

struct gx_m2_ribbons
{
	struct gx_m2_instance *parent;
	struct jks_array emitters; /* struct gx_m2_ribbons_emitter */
	bool initialized;
};

struct gx_m2_ribbons *gx_m2_ribbons_new(struct gx_m2_instance *parent);
void gx_m2_ribbons_delete(struct gx_m2_ribbons *ribbons);
void gx_m2_ribbons_update(struct gx_m2_ribbons *ribbons, struct gx_frame *frame);
void gx_m2_ribbons_render(struct gx_m2_ribbons *ribbons, struct gx_frame *frame, struct gx_m2_render_params *params);

#endif
