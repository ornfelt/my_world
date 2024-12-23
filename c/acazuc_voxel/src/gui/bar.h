#ifndef GUI_BAR_H
#define GUI_BAR_H

#include "gui/sprite.h"

struct gui_bar
{
	struct gui_sprite background;
	struct gui_sprite selected;
};

void gui_bar_init(struct gui_bar *bar);
void gui_bar_destroy(struct gui_bar *bar);
void gui_bar_draw(struct gui_bar *bar);

#endif
