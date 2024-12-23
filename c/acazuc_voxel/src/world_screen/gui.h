#ifndef WORLD_SCREEN_GUI_H
#define WORLD_SCREEN_GUI_H

#include "gui/inventory.h"
#include "gui/crafting.h"
#include "gui/hearts.h"
#include "gui/cross.h"
#include "gui/label.h"
#include "gui/bar.h"

#include <stdint.h>

struct gfx_pointer_event;
struct gfx_scroll_event;
struct gfx_mouse_event;
struct gfx_key_event;
struct world_screen;

enum world_screen_gui_state
{
	WORLD_SCREEN_GUI_NONE,
	WORLD_SCREEN_GUI_INVENTORY,
	WORLD_SCREEN_GUI_CRAFTING,
};

struct world_screen_gui
{
	struct world_screen *world_screen;
	struct gui_inventory inventory;
	struct gui_crafting crafting;
	struct gui_hearts hearts;
	struct gui_sprite water;
	struct gui_label focused_label;
	struct gui_label light_label;
	struct gui_label fps_label;
	struct gui_label pos_label;
	struct gui_cross cross;
	struct gui_bar bar;
	uint64_t last_chunk_updates;
	uint64_t last_fps;
	enum world_screen_gui_state state;
};

void world_screen_gui_init(struct world_screen_gui *gui,
                           struct world_screen *world_screen);
void world_screen_gui_destroy(struct world_screen_gui *gui);
void world_screen_gui_draw(struct world_screen_gui *gui);
void world_screen_gui_mouse_move(struct world_screen_gui *gui,
                                 struct gfx_pointer_event *event);
void world_screen_gui_mouse_scroll(struct world_screen_gui *gui,
                                   struct gfx_scroll_event *event);
void world_screen_gui_key_down(struct world_screen_gui *gui,
                               struct gfx_key_event *event);
void world_screen_gui_set_state(struct world_screen_gui *gui,
                                enum world_screen_gui_state state);

#endif
