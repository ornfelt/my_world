#ifndef GUI_LABEL_H
#define GUI_LABEL_H

#include <gfx/objects.h>

#include <jks/vec2.h>

#include <stdint.h>

struct gui_label
{
	gfx_attributes_state_t attributes_state;
	gfx_buffer_t vertexes_buffer;
	gfx_buffer_t indices_buffer;
	gfx_buffer_t uniform_buffer;
	uint32_t indices;
	int32_t x;
	int32_t y;
	char *text;
	char color;
	bool shadow;
	bool dirty;
};

void gui_label_init(struct gui_label *label);
void gui_label_destroy(struct gui_label *label);
void gui_label_draw(struct gui_label *label);
void gui_label_set_text(struct gui_label *label, const char *text);
void gui_label_set_pos(struct gui_label *label, int32_t x, int32_t y);
void gui_label_set_color(struct gui_label *label, char color);
void gui_label_set_shadow(struct gui_label *label, bool shadow);
int32_t gui_label_get_width(struct gui_label *label);
int32_t gui_label_get_height(struct gui_label *label);

#endif
