#ifndef GUI_SPRITE_H
#define GUI_SPRITE_H

#include <gfx/objects.h>

#include <jks/vec2.h>
#include <jks/vec4.h>

struct texture;

struct gui_sprite
{
	gfx_attributes_state_t attributes_state;
	gfx_buffer_t vertexes_buffer;
	gfx_buffer_t indices_buffer;
	gfx_buffer_t uniform_buffer;
	struct texture *texture;
	struct vec2f texture_position;
	struct vec2f texture_size;
	struct vec4f colors[4];
	int32_t width;
	int32_t height;
	int32_t x;
	int32_t y;
	bool dirty;
};

void gui_sprite_init(struct gui_sprite *sprite);
void gui_sprite_destroy(struct gui_sprite *sprite);
void gui_sprite_draw(struct gui_sprite *sprite);
void gui_sprite_set_texture(struct gui_sprite *sprite, struct texture *texture);
void gui_sprite_set_pos(struct gui_sprite *sprite, float x, float y);
void gui_sprite_set_size(struct gui_sprite *sprite, float width, float height);
void gui_sprite_set_tex_pos(struct gui_sprite *sprite, float x, float y);
void gui_sprite_set_tex_size(struct gui_sprite *sprite, float width, float height);
void gui_sprite_set_color(struct gui_sprite *sprite, float r, float g, float b, float a);
void gui_sprite_set_top_color(struct gui_sprite *sprite, float r, float g, float b, float a);
void gui_sprite_set_bot_color(struct gui_sprite *sprite, float r, float g, float b, float a);

#endif
