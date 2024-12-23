#include "world_screen/world_screen.h"

#include "world/world.h"

#include "voxel.h"
#include "log.h"

#include <gfx/window.h>

#include <stdlib.h>

static void destroy(struct screen *screen)
{
	struct world_screen *world_screen = (struct world_screen*)screen;
	world_screen_gui_destroy(&world_screen->gui);
	voxel_ungrab_cursor(g_voxel);
}

static void tick(struct screen *screen)
{
	(void)screen;
	world_tick(g_voxel->world);
}

static void draw(struct screen *screen)
{
	struct world_screen *world_screen = (struct world_screen*)screen;
	world_draw(g_voxel->world);
	world_screen_gui_draw(&world_screen->gui);
}

static void key_down(struct screen *screen, struct gfx_key_event *event)
{
	struct world_screen *world_screen = (struct world_screen*)screen;
	world_screen_gui_key_down(&world_screen->gui, event);
	if (event->used)
		return;
	if (event->key == GFX_KEY_ESCAPE)
	{
		voxel_ungrab_cursor(g_voxel);
		event->used = true;
	}
}

static void mouse_move(struct screen *screen, struct gfx_pointer_event *event)
{
	struct world_screen *world_screen = (struct world_screen*)screen;
	if (g_voxel->grabbed)
		return;
	world_screen_gui_mouse_move(&world_screen->gui, event);
}

static void mouse_down(struct screen *screen, struct gfx_mouse_event *event)
{
	struct world_screen *world_screen = (struct world_screen*)screen;
	if (world_screen->gui.state == WORLD_SCREEN_GUI_NONE)
		voxel_grab_cursor(g_voxel);
}

static void focus_in(struct screen *screen)
{
	struct world_screen *world_screen = (struct world_screen*)screen;
	if (world_screen->gui.state == WORLD_SCREEN_GUI_NONE)
		voxel_grab_cursor(g_voxel);
}

static void focus_out(struct screen *screen)
{
	(void)screen;
	voxel_ungrab_cursor(g_voxel);
}

static const struct screen_op screen_op =
{
	.destroy = destroy,
	.tick = tick,
	.draw = draw,
	.key_down = key_down,
	.mouse_move = mouse_move,
	.mouse_down = mouse_down,
	.focus_in = focus_in,
	.focus_out = focus_out,
};

struct screen *world_screen_new(void)
{
	struct world_screen *world_screen = calloc(sizeof(*world_screen), 1);
	if (!world_screen)
	{
		LOG_ERROR("world screen allocation failed");
		return NULL;
	}
	world_screen->screen.op = &screen_op;
	world_screen_gui_init(&world_screen->gui, world_screen);
	voxel_grab_cursor(g_voxel);
	return &world_screen->screen;
}
