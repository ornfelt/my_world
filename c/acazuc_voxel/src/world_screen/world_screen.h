#ifndef WORLD_SCREEN_H
#define WORLD_SCREEN_H

#include "world_screen/gui.h"

#include "screen.h"

struct world_screen
{
	struct screen screen;
	struct world_screen_gui gui;
};

struct screen *world_screen_new(void);

#endif
