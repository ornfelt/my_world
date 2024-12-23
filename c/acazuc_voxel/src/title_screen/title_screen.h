#ifndef TITLE_SCREEN_H
#define TITLE_SCREEN_H

#include "title_screen/title_screen_mode.h"

#include "gui/singleplayer.h"
#include "gui/multiplayer.h"
#include "gui/options.h"
#include "gui/title.h"

#include "screen.h"

#include <gfx/events.h>

struct title_screen
{
	struct screen screen;
	enum title_screen_mode mode;
	struct gui_singleplayer singleplayer;
	struct gui_multiplayer multiplayer;
	struct gui_options options;
	struct gui_sprite background;
	struct gui_title title;
};

struct screen *title_screen_new(void);

#endif
