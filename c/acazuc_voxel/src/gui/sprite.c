#include "gui/sprite.h"
#include "gui/gui.h"

#include "textures.h"
#include "graphics.h"
#include "shaders.h"
#include "voxel.h"
#include "log.h"

#include <gfx/device.h>

void gui_sprite_init(struct gui_sprite *sprite)
{
	sprite->attributes_state = GFX_ATTRIBUTES_STATE_INIT();
	sprite->vertexes_buffer = GFX_BUFFER_INIT();
	sprite->indices_buffer = GFX_BUFFER_INIT();
	sprite->uniform_buffer = GFX_BUFFER_INIT();
	sprite->texture = NULL;
	sprite->texture_position.x = 0;
	sprite->texture_position.y = 0;
	sprite->texture_size.x = 1;
	sprite->texture_size.y = 1;
	sprite->width = 0;
	sprite->height = 0;
	sprite->x = 0;
	sprite->y = 0;
	for (int i = 0; i < 4; ++i)
	{
		sprite->colors[i].x = 1;
		sprite->colors[i].y = 1;
		sprite->colors[i].z = 1;
		sprite->colors[i].w = 1;
	}
	sprite->dirty = false;
}

void gui_sprite_destroy(struct gui_sprite *sprite)
{
	gfx_delete_attributes_state(g_voxel->device, &sprite->attributes_state);
	gfx_delete_buffer(g_voxel->device, &sprite->vertexes_buffer);
	gfx_delete_buffer(g_voxel->device, &sprite->indices_buffer);
	gfx_delete_buffer(g_voxel->device, &sprite->uniform_buffer);
}

void gui_sprite_draw(struct gui_sprite *sprite)
{
	if (!sprite->texture)
		return;
	if (!sprite->uniform_buffer.handle.ptr)
		gfx_create_buffer(g_voxel->device, &sprite->uniform_buffer, GFX_BUFFER_UNIFORM, NULL, sizeof(struct shader_gui_model_block), GFX_BUFFER_STREAM);
	if (sprite->dirty)
	{
		sprite->dirty = false;
		if (!sprite->indices_buffer.handle.ptr)
		{
			static const uint16_t indices[] = {0, 1, 2, 0, 2, 3};
			gfx_create_buffer(g_voxel->device, &sprite->indices_buffer, GFX_BUFFER_INDICES, indices, sizeof(indices), GFX_BUFFER_IMMUTABLE);
		}
		if (!sprite->attributes_state.handle.ptr)
		{
			gfx_create_buffer(g_voxel->device, &sprite->vertexes_buffer, GFX_BUFFER_VERTEXES, NULL, sizeof(struct shader_gui_vertex) * 4, GFX_BUFFER_STATIC);
			const struct gfx_attribute_bind binds[] =
			{
				{&sprite->vertexes_buffer},
			};
			gfx_create_attributes_state(g_voxel->device, &sprite->attributes_state, binds, sizeof(binds) / sizeof(*binds), &sprite->indices_buffer, GFX_INDEX_UINT16);
		}
		struct shader_gui_vertex vertexes[] =
		{
			{
				{
					sprite->x,
					sprite->y,
				},
				sprite->colors[0],
				{
					sprite->texture_position.x,
					sprite->texture_position.y,
				},
			},
			{
				{
					sprite->x + sprite->width,
					sprite->y,
				},
				sprite->colors[1],
				{
					sprite->texture_position.x + sprite->texture_size.x,
					sprite->texture_position.y,
				},
			},
			{
				{
					sprite->x + sprite->width,
					sprite->y + sprite->height,
				},
				sprite->colors[2],
				{
					sprite->texture_position.x + sprite->texture_size.x,
					sprite->texture_position.y + sprite->texture_size.y,
				},
			},
			{
				{
					sprite->x,
					sprite->y + sprite->height,
				},
				sprite->colors[3],
				{
					sprite->texture_position.x,
					sprite->texture_position.y + sprite->texture_size.y,
				},
			},
		};
		gfx_set_buffer_data(&sprite->vertexes_buffer, vertexes, sizeof(vertexes), 0);
	}
	texture_bind(sprite->texture);
	struct shader_gui_model_block model_block;
	model_block.mvp = g_voxel->gui->mat;
	gfx_set_buffer_data(&sprite->uniform_buffer, &model_block, sizeof(model_block), 0);
	gfx_bind_constant(g_voxel->device, 1, &sprite->uniform_buffer, sizeof(model_block), 0);
	gfx_bind_attributes_state(g_voxel->device, &sprite->attributes_state, &g_voxel->graphics->gui.input_layout);
	gfx_draw_indexed(g_voxel->device, 6, 0);
}

void gui_sprite_set_texture(struct gui_sprite *sprite, struct texture *texture)
{
	sprite->texture = texture;
}

void gui_sprite_set_pos(struct gui_sprite *sprite, float x, float y)
{
	if (sprite->x == x
	 && sprite->y == y)
		return;
	sprite->x = x;
	sprite->y = y;
	sprite->dirty = true;
}

void gui_sprite_set_size(struct gui_sprite *sprite, float width, float height)
{
	if (sprite->width == width
	 && sprite->height == height)
		return;
	sprite->width = width;
	sprite->height = height;
	sprite->dirty = true;
}

void gui_sprite_set_tex_pos(struct gui_sprite *sprite, float x, float y)
{
	if (sprite->texture_position.x == x
	 && sprite->texture_position.y == y)
		return;
	sprite->texture_position.x = x;
	sprite->texture_position.y = y;
	sprite->dirty = true;
}

void gui_sprite_set_tex_size(struct gui_sprite *sprite, float width, float height)
{
	if (sprite->texture_size.x == width
	 && sprite->texture_size.y == height)
		return;
	sprite->texture_size.x = width;
	sprite->texture_size.y = height;
	sprite->dirty = true;
}

static void set_color(struct gui_sprite *sprite, struct vec4f *color, float r, float g, float b, float a)
{
	if (color->x == r
	 && color->y == g
	 && color->z == b
	 && color->w == a)
		return;
	color->x = r;
	color->y = g;
	color->z = b;
	color->w = a;
	sprite->dirty = true;
}

void gui_sprite_set_color(struct gui_sprite *sprite, float r, float g, float b, float a)
{
	set_color(sprite, &sprite->colors[0], r, g, b, a);
	set_color(sprite, &sprite->colors[1], r, g, b, a);
	set_color(sprite, &sprite->colors[2], r, g, b, a);
	set_color(sprite, &sprite->colors[3], r, g, b, a);
}

void gui_sprite_set_top_color(struct gui_sprite *sprite, float r, float g, float b, float a)
{
	set_color(sprite, &sprite->colors[0], r, g, b, a);
	set_color(sprite, &sprite->colors[1], r, g, b, a);
}

void gui_sprite_set_bot_color(struct gui_sprite *sprite, float r, float g, float b, float a)
{
	set_color(sprite, &sprite->colors[2], r, g, b, a);
	set_color(sprite, &sprite->colors[3], r, g, b, a);
}
