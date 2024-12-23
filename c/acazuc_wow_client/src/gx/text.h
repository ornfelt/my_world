#ifndef GX_TEXT_H
#define GX_TEXT_H

#include "refcount.h"

#include <jks/vec3.h>
#include <jks/vec4.h>

#include <gfx/objects.h>

struct gx_frame;

enum gx_text_flag
{
	GX_TEXT_FLAG_IN_RENDER_LIST = (1 << 0),
	GX_TEXT_FLAG_DIRTY          = (1 << 1),
	GX_TEXT_FLAG_INITIALIZED    = (1 << 2),
};

struct gx_text_frame
{
	gfx_buffer_t uniform_buffer;
	struct vec3f position;
	struct vec4f color;
	struct vec3f scale;
};

struct gx_text
{
	struct gx_text_frame frames[RENDER_FRAMES_COUNT];
	gfx_attributes_state_t attributes_state;
	gfx_buffer_t vertexes_buffer;
	gfx_buffer_t indices_buffer;
	struct vec3f position;
	struct vec4f color;
	struct vec3f scale;
	char *str;
	uint32_t font_revision;
	uint32_t indices;
	enum gx_text_flag flags;
	refcount_t refcount;
};

struct gx_text *gx_text_new(void);
void gx_text_free(struct gx_text *text);
void gx_text_ref(struct gx_text *text);
void gx_text_render(struct gx_text *text, struct gx_frame *frame);
void gx_text_add_to_render(struct gx_text *text, struct gx_frame *frame);
void gx_text_set_text(struct gx_text *text, const char *str);
void gx_text_set_color(struct gx_text *text, struct vec4f color);
void gx_text_set_pos(struct gx_text *text, struct vec3f pos);
void gx_text_set_scale(struct gx_text *text, struct vec3f scale);

static inline bool gx_text_flag_set(struct gx_text *text, enum gx_text_flag flag)
{
	return (__atomic_fetch_or(&text->flags, flag, __ATOMIC_RELAXED) & flag) != 0;
}

static inline bool gx_text_flag_clear(struct gx_text *text, enum gx_text_flag flag)
{
	return (__atomic_fetch_and(&text->flags, ~flag, __ATOMIC_RELAXED) & flag) != 0;
}

#endif
