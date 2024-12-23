#ifndef GUI_CROSS_H
#define GUI_CROSS_H

#include "gui/sprite.h"

struct gui_cross
{
	struct gui_sprite sprite;
};

void gui_cross_init(struct gui_cross *cross);
void gui_cross_destroy(struct gui_cross *cross);
void gui_cross_draw(struct gui_cross *cross);

#endif
