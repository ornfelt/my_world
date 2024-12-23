#ifndef GUI_HEARTS_H
#define GUI_HEARTS_H

#include "gui/sprite.h"

struct gui_hearts
{
	struct gui_sprite backgrounds[10];
	struct gui_sprite hearts[10];
};

void gui_hearts_init(struct gui_hearts *hearts);
void gui_hearts_destroy(struct gui_hearts *hearts);
void gui_hearts_draw(struct gui_hearts *hearts);

#endif
