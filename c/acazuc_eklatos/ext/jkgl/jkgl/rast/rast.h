#ifndef JKGL_RAST_H
#define JKGL_RAST_H

#define RAST_MAX_ATTRIBS 32

#include <stdbool.h>
#include <stdint.h>

enum rast_cull_face
{
	RAST_CULL_FRONT = (1 << 0),
	RAST_CULL_BACK  = (1 << 1),
};

enum rast_fill_mode
{
	RAST_FILL_POINT,
	RAST_FILL_LINE,
	RAST_FILL_SOLID,
};

enum rast_primitive
{
	RAST_PRIMITIVE_POINTS,
	RAST_PRIMITIVE_LINES,
	RAST_PRIMITIVE_LINE_LOOP,
	RAST_PRIMITIVE_LINE_STRIP,
	RAST_PRIMITIVE_TRIANGLES,
	RAST_PRIMITIVE_TRIANGLE_FAN,
	RAST_PRIMITIVE_TRIANGLE_STRIP,
	RAST_PRIMITIVE_QUADS,
	RAST_PRIMITIVE_QUAD_STRIP,
};

struct rast_vec
{
	float x;
	float y;
	float z;
	float w;
};

struct rast_vert
{
	bool front_face;
	float x;
	float y;
	float z;
	float w;
	struct rast_vec attribs[RAST_MAX_ATTRIBS];
	struct rast_vec varying[RAST_MAX_ATTRIBS];
};

struct rast_ctx
{
	void (*fragment)(struct rast_ctx *ctx,
                         struct rast_vert *vert);
	bool (*get_vert)(struct rast_ctx *ctx,
	                 uint32_t instance,
	                 uint32_t index,
	                 struct rast_vert *vert);
	void *userdata;
	float min_x;
	float max_x;
	float min_y;
	float max_y;
	float min_z;
	float max_z;
	float line_width;
	float point_size;
	bool line_smooth_enable;
	bool point_smooth_enable;
	uint32_t nvarying;
	uint32_t nattribs;
	bool front_ccw;
	uint32_t start;
	uint32_t count;
	uint32_t instance_start;
	uint32_t instance_count;
	enum rast_cull_face cull_face;
	enum rast_fill_mode fill_front;
	enum rast_fill_mode fill_back;
	enum rast_primitive primitive;
};

void rast_draw(struct rast_ctx *ctx);
void rast_point(struct rast_ctx *ctx,
                struct rast_vert *vert);
void rast_line(struct rast_ctx *ctx,
               struct rast_vert *v1,
               struct rast_vert *v2);
void rast_triangle(struct rast_ctx *ctx,
                   struct rast_vert *v1,
                   struct rast_vert *v2,
                   struct rast_vert *v3);

void rast_vert_mix(struct rast_ctx *ctx,
                   struct rast_vert *dst,
                   struct rast_vert *v1,
                   struct rast_vert *v2,
                   float f);
void rast_vert_normalize(struct rast_vert *vert,
                         float min_x,
                         float min_y,
                         float max_x,
                         float max_y);

#endif
