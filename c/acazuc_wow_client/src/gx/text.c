#include "font/font.h"

#include "gx/frame.h"
#include "gx/text.h"

#include "graphics.h"
#include "shaders.h"
#include "memory.h"
#include "loader.h"
#include "cache.h"
#include "log.h"
#include "wow.h"

#include <gfx/device.h>

#include <jks/utf8.h>

#include <inttypes.h>
#include <string.h>
#include <math.h>

#define SCALE_FACTOR 0.004

MEMORY_DECL(GX);

struct gx_text *gx_text_new(void)
{
	struct gx_text *text = mem_malloc(MEM_GX, sizeof(*text));
	if (!text)
	{
		LOG_ERROR("allocation failed");
		return NULL;
	}
	text->str = NULL;
	text->attributes_state = GFX_ATTRIBUTES_STATE_INIT();
	for (size_t i = 0; i < RENDER_FRAMES_COUNT; ++i)
		text->frames[i].uniform_buffer = GFX_BUFFER_INIT();
	text->vertexes_buffer = GFX_BUFFER_INIT();
	text->indices_buffer = GFX_BUFFER_INIT();
	text->font_revision = 0;
	text->indices = 0;
	text->flags = 0;
	VEC3_SETV(text->scale, SCALE_FACTOR);
	refcount_init(&text->refcount, 1);
	return text;
}

static void unload_task(struct wow_mpq_compound *mpq_compound, void *userdata)
{
	(void)mpq_compound;
	struct gx_text *text = userdata;
	gfx_delete_attributes_state(g_wow->device, &text->attributes_state);
	for (size_t i = 0; i < RENDER_FRAMES_COUNT; ++i)
		gfx_delete_buffer(g_wow->device, &text->frames[i].uniform_buffer);
	gfx_delete_buffer(g_wow->device, &text->vertexes_buffer);
	gfx_delete_buffer(g_wow->device, &text->indices_buffer);
	mem_free(MEM_GX, text->str);
	mem_free(MEM_GX, text);
}

void gx_text_free(struct gx_text *text)
{
	if (!text)
		return;
	if (refcount_dec(&text->refcount))
		return;
	loader_push(g_wow->loader, ASYNC_TASK_TEXT_DELETE, unload_task, text);
}

void gx_text_ref(struct gx_text *text)
{
	refcount_inc(&text->refcount);
}

static void rebuild_buffers(struct gx_text *text)
{
	struct jks_array vertexes; /* shader_text_input_t */
	struct jks_array indices; /* uint16_t */
	jks_array_init(&vertexes, sizeof(struct shader_text_input), NULL, &jks_array_memory_fn_GX);
	jks_array_init(&indices, sizeof(uint16_t), NULL, &jks_array_memory_fn_GX);
	const char *iter = text->str;
	const char *end = iter + strlen(text->str);
	int32_t x = 0;
	int32_t y = 0;
	uint32_t line_start = 0;
	int32_t line_width = 0;
	while (iter < end)
	{
		uint32_t character;
		if (!utf8_next(&iter, end, &character))
			goto err;
		if (character == '\n')
		{
			for (size_t i = line_start; i < vertexes.size; ++i)
			{
				struct shader_text_input *tmp = JKS_ARRAY_GET(&vertexes, i, struct shader_text_input);
				tmp->position.x -= line_width * .5;
				tmp->position.y = -tmp->position.y;
			}
			line_start = vertexes.size;
			line_width = 0;
			x = 0;
			y += g_wow->font_3d->height;
			continue;
		}
		struct font_glyph *glyph = font_get_glyph(g_wow->font_3d, character);
		if (!glyph)
			continue;
		float char_render_left = x + glyph->offset_x;
		float char_render_top = y + glyph->offset_y;
		float char_render_right = char_render_left + glyph->width;
		float char_render_bottom = char_render_top + glyph->height;
		{
			uint16_t *tmp = jks_array_grow(&indices, 6);
			if (!tmp)
			{
				LOG_ERROR("failed to grow indices");
				goto err;
			}
			tmp[0] = vertexes.size + 0;
			tmp[1] = vertexes.size + 1;
			tmp[2] = vertexes.size + 2;
			tmp[3] = vertexes.size + 0;
			tmp[4] = vertexes.size + 2;
			tmp[5] = vertexes.size + 3;
		}
		{
			struct shader_text_input *vertex = jks_array_grow(&vertexes, 4);
			if (!vertex)
			{
				LOG_ERROR("failed to grow vertexes");
				goto err;
			}
			struct vec2f tmp[4];
			font_glyph_tex_coords(g_wow->font_3d, glyph, &tmp[0].x);
			VEC2_CPY(vertex[0].uv, tmp[0]);
			VEC2_CPY(vertex[1].uv, tmp[1]);
			VEC2_CPY(vertex[2].uv, tmp[2]);
			VEC2_CPY(vertex[3].uv, tmp[3]);
			VEC2_SET(vertex[0].position, char_render_left , char_render_top);
			VEC2_SET(vertex[1].position, char_render_right, char_render_top);
			VEC2_SET(vertex[2].position, char_render_right, char_render_bottom);
			VEC2_SET(vertex[3].position, char_render_left , char_render_bottom);
		}
		x += glyph->advance;
		line_width += glyph->advance;
	}
	for (size_t i = line_start; i < vertexes.size; ++i)
	{
		struct shader_text_input *tmp = JKS_ARRAY_GET(&vertexes, i, struct shader_text_input);
		tmp->position.x -= line_width * .5;
		tmp->position.y = -tmp->position.y;
	}
	if (x != 0)
		y += g_wow->font_3d->height;
	for (size_t i = 0; i < vertexes.size; ++i)
	{
		struct shader_text_input *tmp = JKS_ARRAY_GET(&vertexes, i, struct shader_text_input);
		tmp->position.y += y;
	}
	text->indices = indices.size;
	gfx_delete_attributes_state(g_wow->device, &text->attributes_state);
	gfx_delete_buffer(g_wow->device, &text->vertexes_buffer);
	gfx_delete_buffer(g_wow->device, &text->indices_buffer);
	gfx_create_buffer(g_wow->device, &text->vertexes_buffer, GFX_BUFFER_VERTEXES, vertexes.data, vertexes.size * sizeof(struct shader_text_input), GFX_BUFFER_STATIC);
	gfx_create_buffer(g_wow->device, &text->indices_buffer, GFX_BUFFER_INDICES, indices.data, indices.size * sizeof(uint16_t), GFX_BUFFER_STATIC);
	const struct gfx_attribute_bind binds[] =
	{
		{&text->vertexes_buffer},
	};
	gfx_create_attributes_state(g_wow->device, &text->attributes_state, binds, sizeof(binds) / sizeof(*binds), &text->indices_buffer, GFX_INDEX_UINT16);
err:
	jks_array_destroy(&vertexes);
	jks_array_destroy(&indices);
}

static void initialize(struct gx_text *text)
{
	for (size_t i = 0; i < RENDER_FRAMES_COUNT; ++i)
		gfx_create_buffer(g_wow->device, &text->frames[i].uniform_buffer, GFX_BUFFER_UNIFORM, NULL, sizeof(struct shader_text_model_block), GFX_BUFFER_STREAM);
}

void gx_text_render(struct gx_text *text, struct gx_frame *frame)
{
	if (!g_wow->font_3d || !text->str || !text->str[0])
		return;
	if (!gx_text_flag_set(text, GX_TEXT_FLAG_INITIALIZED))
		initialize(text);
	font_atlas_update(g_wow->font_3d->atlas);
	if (text->font_revision != g_wow->font_3d->atlas->revision)
	{
		text->font_revision = g_wow->font_3d->atlas->revision;
		gx_text_flag_set(text, GX_TEXT_FLAG_DIRTY);
	}
	if (gx_text_flag_clear(text, GX_TEXT_FLAG_DIRTY))
		rebuild_buffers(text);
	if (!text->indices)
		return;
	struct gx_text_frame *text_frame = &text->frames[frame->id];
	struct shader_text_model_block model_block;
	model_block.color = text_frame->color;
	struct mat4f tmp;
	MAT4_IDENTITY(tmp);
	MAT4_TRANSLATE(model_block.mvp, tmp, text_frame->position);
	MAT4_ROTATEY(float, tmp, model_block.mvp, -frame->view_rot.y);
	MAT4_ROTATEX(float, model_block.mvp, tmp, -frame->view_rot.x);
	MAT4_SCALE(tmp, model_block.mvp, text_frame->scale);
	MAT4_MUL(model_block.mvp, frame->view_vp, tmp);
	gfx_set_buffer_data(&text_frame->uniform_buffer, &model_block, sizeof(model_block), 0);
	gfx_bind_attributes_state(g_wow->device, &text->attributes_state, &g_wow->graphics->text_input_layout);
	font_atlas_bind(g_wow->font_3d->atlas, 0);
	gfx_bind_constant(g_wow->device, 1, &text_frame->uniform_buffer, sizeof(struct shader_text_model_block), 0);
	gfx_draw_indexed(g_wow->device, text->indices, 0);
}

void gx_text_add_to_render(struct gx_text *text, struct gx_frame *frame)
{
	if (gx_text_flag_set(text, GX_TEXT_FLAG_IN_RENDER_LIST))
		return;
	struct gx_text_frame *text_frame = &text->frames[frame->id];
	text_frame->position = text->position;
	text_frame->color = text->color;
	text_frame->scale = text->scale;
	gx_text_ref(text);
	gx_frame_add_text(frame, text);
}

void gx_text_set_text(struct gx_text *text, const char *str)
{
	mem_free(MEM_GX, text->str);
	if (str)
		text->str = mem_strdup(MEM_GX, str);
	else
		text->str = NULL;
	__atomic_fetch_or(&text->flags, GX_TEXT_FLAG_DIRTY, __ATOMIC_RELAXED);
}

void gx_text_set_color(struct gx_text *text, struct vec4f color)
{
	text->color = color;
}

void gx_text_set_pos(struct gx_text *text, struct vec3f pos)
{
	text->position = pos;
}

void gx_text_set_scale(struct gx_text *text, struct vec3f scale)
{
	VEC3_MULV(text->scale, scale, SCALE_FACTOR);
}
