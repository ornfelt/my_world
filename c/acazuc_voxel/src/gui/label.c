#include "gui/label.h"
#include "gui/gui.h"

#include "graphics.h"
#include "textures.h"
#include "shaders.h"
#include "voxel.h"

#include <gfx/device.h>

#include <jks/array.h>
#include <jks/vec3.h>

#include <stdlib.h>
#include <string.h>

void gui_label_init(struct gui_label *label)
{
	label->attributes_state = GFX_ATTRIBUTES_STATE_INIT();
	label->vertexes_buffer = GFX_BUFFER_INIT();
	label->indices_buffer = GFX_BUFFER_INIT();
	label->uniform_buffer = GFX_BUFFER_INIT();
	label->x = 0;
	label->y = 0;
	label->text = NULL;
	label->color = 'f';
	label->shadow = true;
}

void gui_label_destroy(struct gui_label *label)
{
	gfx_delete_attributes_state(g_voxel->device, &label->attributes_state);
	gfx_delete_buffer(g_voxel->device, &label->vertexes_buffer);
	gfx_delete_buffer(g_voxel->device, &label->indices_buffer);
	gfx_delete_buffer(g_voxel->device, &label->uniform_buffer);
	free(label->text);
}

void gui_label_draw(struct gui_label *label)
{
	if (!label->text || !*label->text)
		return;
	if (!label->uniform_buffer.handle.ptr)
		gfx_create_buffer(g_voxel->device, &label->uniform_buffer, GFX_BUFFER_UNIFORM, NULL, sizeof(struct shader_gui_model_block), GFX_BUFFER_STREAM);
	if (label->dirty)
	{
		label->dirty = false;
		struct jks_array vertexes;
		struct jks_array indices;
		jks_array_init(&vertexes, sizeof(struct shader_gui_vertex), NULL, NULL);
		jks_array_init(&indices, sizeof(uint16_t), NULL, NULL);
		const char *it = label->text;
		int32_t x = 0;
		int32_t y = 0;
		int32_t size = 8;
		struct vec4f color = gui_get_text_color(label->color);
		struct vec4f shadow_color = gui_get_text_shadow_color(label->color);
		while (*it)
		{
			if (*it == '\n')
			{
				x = 0;
				y += size;
				continue;
			}
			uint8_t c = *it;
			uint8_t char_width = g_voxel->gui->char_size[c] & 0xF;
			uint8_t char_off = g_voxel->gui->char_size[c] >> 4;
			float tex_x = (c % 16) / 16.0f + char_off / 128.0f;
			float tex_y = (c / 16) / 16.0f;
			float tex_width = char_width / 128.0f;
			float tex_height = 8.0f / 128.0f;
			struct shader_gui_vertex *vertex;
			uint16_t *indice;
			uint16_t base;
			if (label->shadow)
			{
				base = vertexes.size;
				vertex = jks_array_grow(&vertexes, 4);
				indice = jks_array_grow(&indices, 6);
				vertex[0].position.x = x + 1;
				vertex[0].position.y = y + 1;
				vertex[0].color = shadow_color;
				vertex[0].uv.x = tex_x;
				vertex[0].uv.y = tex_y;
				vertex[1].position.x = x + 1 + char_width;
				vertex[1].position.y = y + 1;
				vertex[1].color = shadow_color;
				vertex[1].uv.x = tex_x + tex_width;
				vertex[1].uv.y = tex_y;
				vertex[2].position.x = x + 1 + char_width;
				vertex[2].position.y = y + 1 + size;
				vertex[2].color = shadow_color;
				vertex[2].uv.x = tex_x + tex_width;
				vertex[2].uv.y = tex_y + tex_height;
				vertex[3].position.x = x + 1;
				vertex[3].position.y = y + 1 + size;
				vertex[3].color = shadow_color;
				vertex[3].uv.x = tex_x;
				vertex[3].uv.y = tex_y + tex_height;
				indice[0] = base + 0;
				indice[1] = base + 1;
				indice[2] = base + 2;
				indice[3] = base + 0;
				indice[4] = base + 2;
				indice[5] = base + 3;
			}
			base = vertexes.size;
			vertex = jks_array_grow(&vertexes, 4);
			indice = jks_array_grow(&indices, 6);
			vertex[0].position.x = x;
			vertex[0].position.y = y;
			vertex[0].color = color;
			vertex[0].uv.x = tex_x;
			vertex[0].uv.y = tex_y;
			vertex[1].position.x = x + char_width;
			vertex[1].position.y = y;
			vertex[1].color = color;
			vertex[1].uv.x = tex_x + tex_width;
			vertex[1].uv.y = tex_y;
			vertex[2].position.x = x + char_width;
			vertex[2].position.y = y + size;
			vertex[2].color = color;
			vertex[2].uv.x = tex_x + tex_width;
			vertex[2].uv.y = tex_y + tex_height;
			vertex[3].position.x = x;
			vertex[3].position.y = y + size;
			vertex[3].color = color;
			vertex[3].uv.x = tex_x;
			vertex[3].uv.y = tex_y + tex_height;
			indice[0] = base + 0;
			indice[1] = base + 1;
			indice[2] = base + 2;
			indice[3] = base + 0;
			indice[4] = base + 2;
			indice[5] = base + 3;
			x += char_width;
			it++;
		}
		gfx_delete_buffer(g_voxel->device, &label->vertexes_buffer);
		gfx_delete_buffer(g_voxel->device, &label->indices_buffer);
		gfx_delete_attributes_state(g_voxel->device, &label->attributes_state);
		gfx_create_buffer(g_voxel->device, &label->vertexes_buffer, GFX_BUFFER_VERTEXES, vertexes.data, sizeof(struct shader_gui_vertex) * vertexes.size, GFX_BUFFER_IMMUTABLE);
		gfx_create_buffer(g_voxel->device, &label->indices_buffer, GFX_BUFFER_INDICES, indices.data, sizeof(uint16_t) * indices.size, GFX_BUFFER_IMMUTABLE);
		const struct gfx_attribute_bind binds[] =
		{
			{&label->vertexes_buffer},
		};
		gfx_create_attributes_state(g_voxel->device, &label->attributes_state, binds, sizeof(binds) / sizeof(*binds), &label->indices_buffer, GFX_INDEX_UINT16);
		label->indices = indices.size;
		jks_array_destroy(&vertexes);
		jks_array_destroy(&indices);
	}
	if (!label->indices)
		return;
	texture_bind(&g_voxel->textures->ascii);
	struct shader_gui_model_block model_block;
	struct vec3f offset = {label->x, label->y, 0};
	MAT4_TRANSLATE(model_block.mvp, g_voxel->gui->mat, offset);
	gfx_set_buffer_data(&label->uniform_buffer, &model_block, sizeof(model_block), 0);
	gfx_bind_constant(g_voxel->device, 1, &label->uniform_buffer, sizeof(model_block), 0);
	gfx_bind_attributes_state(g_voxel->device, &label->attributes_state, &g_voxel->graphics->gui.input_layout);
	gfx_draw_indexed(g_voxel->device, label->indices, 0);
}

void gui_label_set_text(struct gui_label *label, const char *text)
{
	if (label->text && !strcmp(label->text, text))
		return;
	free(label->text);
	label->text = strdup(text);
	label->dirty = true;
}

void gui_label_set_pos(struct gui_label *label, int32_t x, int32_t y)
{
	label->x = x;
	label->y = y;
}

void gui_label_set_color(struct gui_label *label, char color)
{
	if (label->color == color)
		return;
	label->color = color;
	label->dirty = true;
}

void gui_label_set_shadow(struct gui_label *label, bool shadow)
{
	if (label->shadow == shadow)
		return;
	label->shadow = shadow;
	label->dirty = true;
}

int32_t gui_label_get_width(struct gui_label *label)
{
	if (!label->text)
		return 0;
	int32_t max_width = 0;
	int32_t x = 0;
	const char *it = label->text;
	while (*it)
	{
		if (*it == '\n')
		{
			if (x > max_width)
				max_width = x;
			x = 0;
			continue;
		}
		x += (g_voxel->gui->char_size[(uint8_t)*it] & 0xF);
		it++;
	}
	return x > max_width ? x : max_width;
}

int32_t gui_label_get_height(struct gui_label *label)
{
	if (!label->text)
		return 0;
	int32_t height = 8;
	const char *it = label->text;
	while (*it)
	{
		if (*it == '\n')
			height += 8;
		it++;
	}
	return height;
}
