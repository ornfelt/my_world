#ifndef GUI_H
#define GUI_H

#include <jks/mat4.h>

struct gui
{
	struct mat4f mat;
	int32_t scale;
	int32_t width;
	int32_t height;
	uint8_t char_size[256];
};

struct gui *gui_new(void);
void gui_delete(struct gui *gui);
void gui_update(struct gui *gui);
struct vec4f gui_get_text_color(char i);
struct vec4f gui_get_text_shadow_color(char i);

#endif
