#include "voxel.h"

#include "world_screen/world_screen.h"

#include "title_screen/title_screen.h"

#include "block/blocks.h"

#include "biome/biomes.h"

#include "world/world.h"

#include "entity/def.h"

#include "gui/gui.h"

#include "lagometer.h"
#include "textures.h"
#include "graphics.h"
#include "shaders.h"
#include "screen.h"
#include "const.h"
#include "voxel.h"
#include "log.h"

#include <gfx/window.h>
#include <gfx/device.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

struct voxel *g_voxel;

static const struct vec4f clear_color = {0.71, 0.82, 1, 1};

static void on_mouse_scroll(struct gfx_window *window, struct gfx_scroll_event *event);
static void on_mouse_move(struct gfx_window *window, struct gfx_pointer_event *event);
static void on_mouse_down(struct gfx_window *window, struct gfx_mouse_event *event);
static void on_mouse_up(struct gfx_window *window, struct gfx_mouse_event *event);
static void on_key_down(struct gfx_window *window, struct gfx_key_event *event);
static void on_focus_in(struct gfx_window *window);
static void on_focus_out(struct gfx_window *window);

static bool init_window(struct voxel *voxel)
{
	struct gfx_window_properties properties;
	gfx_window_properties_init(&properties);
	properties.device_backend = GFX_DEVICE_GL4;
	properties.window_backend = GFX_WINDOW_X11;
	properties.depth_bits = 24;
	properties.stencil_bits = 8;
	properties.red_bits = 8;
	properties.green_bits = 8;
	properties.blue_bits = 8;
	properties.alpha_bits = 8;
	voxel->window = gfx_create_window("voxel", 1280, 720, &properties);
	if (!voxel->window)
	{
		LOG_ERROR("window creation failed");
		return false;
	}
	gfx_window_make_current(voxel->window);
	gfx_window_show(voxel->window);
	if (!gfx_create_device(voxel->window))
	{
		LOG_ERROR("device creation failed");
		return false;
	}
	voxel->window->userdata = voxel;
	voxel->device = voxel->window->device;
	gfx_window_set_swap_interval(voxel->window, -1);
	voxel->vsync = true;
	voxel->window->scroll_callback = on_mouse_scroll;
	voxel->window->mouse_move_callback = on_mouse_move;
	voxel->window->mouse_down_callback = on_mouse_down;
	voxel->window->mouse_up_callback = on_mouse_up;
	voxel->window->key_down_callback = on_key_down;
	voxel->window->focus_in_callback = on_focus_in;
	voxel->window->focus_out_callback = on_focus_out;
	return true;
}

static void loop(struct voxel *voxel)
{
	gfx_depth_stencil_state_t depth_stencil_state = GFX_DEPTH_STENCIL_STATE_INIT();
	gfx_rasterizer_state_t rasterizer_state = GFX_RASTERIZER_STATE_INIT();
	gfx_pipeline_state_t pipeline_state = GFX_PIPELINE_STATE_INIT();
	gfx_input_layout_t input_layout = GFX_INPUT_LAYOUT_INIT();
	gfx_blend_state_t blend_state = GFX_BLEND_STATE_INIT();
	gfx_create_depth_stencil_state(voxel->device, &depth_stencil_state, true, true, GFX_CMP_LEQUAL, true, -1, GFX_CMP_NOTEQUAL, 1, -1, GFX_STENCIL_KEEP, GFX_STENCIL_KEEP, GFX_STENCIL_KEEP);
	gfx_create_rasterizer_state(voxel->device, &rasterizer_state, GFX_FILL_SOLID, GFX_CULL_NONE, GFX_FRONT_CCW, false);
	gfx_create_blend_state(voxel->device, &blend_state, true, GFX_BLEND_SRC_ALPHA, GFX_BLEND_ONE_MINUS_SRC_ALPHA, GFX_BLEND_SRC_ALPHA, GFX_BLEND_ONE_MINUS_SRC_ALPHA, GFX_EQUATION_ADD, GFX_EQUATION_ADD, GFX_COLOR_MASK_ALL);
	gfx_create_input_layout(voxel->device, &input_layout, NULL, 0, &voxel->shaders->gui);
	gfx_create_pipeline_state(voxel->device, &pipeline_state, &voxel->shaders->gui, &rasterizer_state, &depth_stencil_state, &blend_state, &input_layout, GFX_PRIMITIVE_TRIANGLES);

	voxel->frametime = nanotime();
	int64_t fps_count = 0;
	int64_t last_fps = voxel->frametime / 1000000000 * 1000000000;
	int64_t last_tick = voxel->frametime;
	while (!voxel->window->close_requested)
	{
		voxel->frametime = nanotime();
		++fps_count;
		if (voxel->frametime - last_fps >= 1000000000)
		{
			last_fps += 1000000000;
			voxel->fps = fps_count;
			fps_count = 0;
		}
		gfx_bind_render_target(voxel->device, NULL);
		gfx_set_viewport(voxel->device, 0, 0, voxel->window->width, voxel->window->height);
		gfx_bind_pipeline_state(voxel->device, &pipeline_state);
		gfx_clear_color(voxel->device, NULL, GFX_RENDERTARGET_ATTACHMENT_COLOR0, clear_color);
		gfx_clear_depth_stencil(voxel->device, NULL, 1, 0);
		if (voxel->next_screen)
		{
			SCREEN_CALL(voxel->screen, destroy);
			free(voxel->screen);
			voxel->screen = voxel->next_screen;
			voxel->next_screen = NULL;
		}
		gui_update(voxel->gui);
		while (voxel->frametime - last_tick >= 1000000000 / TPS)
		{
			SCREEN_CALL(voxel->screen, tick);
			last_tick += 1000000000 / TPS;
		}
		voxel->delta = (voxel->frametime - last_tick) / (1000000000. / TPS);
		SCREEN_CALL(voxel->screen, draw);
		lagometer_draw(voxel->lagometer);
		uint64_t draw_end = nanotime();
		voxel->last_frame_draw_duration = draw_end - voxel->frametime;
		gfx_window_swap_buffers(voxel->window);
		gfx_window_poll_events(voxel->window);
		gfx_device_tick(voxel->device);
		if (voxel->grabbed)
			gfx_set_mouse_position(voxel->window, voxel->window->width / 2, voxel->window->height / 2);
		voxel->last_frame_update_duration = nanotime() - draw_end;
	}
	gfx_delete_depth_stencil_state(voxel->device, &depth_stencil_state);
	gfx_delete_rasterizer_state(voxel->device, &rasterizer_state);
	gfx_delete_blend_state(voxel->device, &blend_state);
	gfx_delete_input_layout(voxel->device, &input_layout);
	gfx_delete_pipeline_state(voxel->device, &pipeline_state);
}

int voxel_main(struct voxel *voxel)
{
	voxel->frametime = nanotime();
	if (!init_window(voxel))
		return EXIT_FAILURE;
	voxel->textures = textures_new();
	if (!voxel->textures)
		return EXIT_FAILURE;
	voxel->shaders = shaders_new();
	if (!voxel->shaders)
		return EXIT_FAILURE;
	voxel->graphics = graphics_new();
	if (!voxel->graphics)
		return EXIT_FAILURE;
	voxel->blocks = blocks_def_new();
	if (!voxel->blocks)
		return EXIT_FAILURE;
	voxel->biomes = biomes_new();
	if (!voxel->biomes)
		return EXIT_FAILURE;
	voxel->entities = entities_def_new();
	if (!voxel->entities)
		return EXIT_FAILURE;
	voxel->lagometer = lagometer_new();
	if (!voxel->lagometer)
		return EXIT_FAILURE;
	voxel->gui = gui_new();
	if (!voxel->gui)
		return EXIT_FAILURE;
	voxel->screen = title_screen_new();
	loop(voxel);
	SCREEN_CALL(voxel->screen, destroy);
	free(voxel->screen);
	world_delete(voxel->world);
	gui_delete(voxel->gui);
	lagometer_delete(voxel->lagometer);
	entities_def_delete(voxel->entities);
	biomes_delete(voxel->biomes);
	blocks_def_delete(voxel->blocks);
	graphics_delete(voxel->graphics);
	shaders_delete(voxel->shaders);
	textures_delete(voxel->textures);
	gfx_delete_window(voxel->window);
	return EXIT_SUCCESS;
}

static void on_mouse_scroll(struct gfx_window *window, struct gfx_scroll_event *event)
{
	struct voxel *voxel = window->userdata;
	SCREEN_CALL(voxel->screen, mouse_scroll, event);
}

static void on_mouse_move(struct gfx_window *window, struct gfx_pointer_event *event)
{
	struct voxel *voxel = window->userdata;
	int32_t old_x = event->x;
	int32_t old_y = event->y;
	event->x /= voxel->gui->scale;
	event->y /= voxel->gui->scale;
	SCREEN_CALL(voxel->screen, mouse_move, event);
	event->x = old_x;
	event->y = old_y;
}

static void on_mouse_down(struct gfx_window *window, struct gfx_mouse_event *event)
{
	struct voxel *voxel = window->userdata;
	int32_t old_x = event->x;
	int32_t old_y = event->y;
	event->x /= voxel->gui->scale;
	event->y /= voxel->gui->scale;
	SCREEN_CALL(voxel->screen, mouse_down, event);
	event->x = old_x;
	event->y = old_y;
}

static void on_mouse_up(struct gfx_window *window, struct gfx_mouse_event *event)
{
	struct voxel *voxel = window->userdata;
	int32_t old_x = event->x;
	int32_t old_y = event->y;
	event->x /= voxel->gui->scale;
	event->y /= voxel->gui->scale;
	SCREEN_CALL(voxel->screen, mouse_up, event);
	event->x = old_x;
	event->y = old_y;
}

static void on_key_down(struct gfx_window *window, struct gfx_key_event *event)
{
	struct voxel *voxel = window->userdata;
	if (event->key == GFX_KEY_P)
	{
		voxel->ssao = !voxel->ssao;
		world_regenerate_buffers(voxel->world);
		return;
	}
	if (event->key == GFX_KEY_O)
	{
		voxel->smooth = !voxel->smooth;
		world_regenerate_buffers(voxel->world);
		return;
	}
	if (event->key == GFX_KEY_I)
	{
		voxel->disable_tex = !voxel->disable_tex;
		return;
	}
	if (event->key == GFX_KEY_B)
	{
		voxel->vsync = !voxel->vsync;
		gfx_window_set_swap_interval(voxel->window, voxel->vsync ? -1 : 0);
		return;
	}
	SCREEN_CALL(voxel->screen, key_down, event);
}

static void on_focus_in(struct gfx_window *window)
{
	struct voxel *voxel = window->userdata;
	SCREEN_CALL(voxel->screen, focus_in);
}

static void on_focus_out(struct gfx_window *window)
{
	struct voxel *voxel = window->userdata;
	SCREEN_CALL(voxel->screen, focus_out);
}

uint64_t nanotime(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_nsec + ts.tv_sec * 1000000000ULL;
}

uint32_t npot32(uint32_t v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}

#define LIGHT_MIN .125
#define LIGHT_RANGE .7

float voxel_light_value(int8_t light)
{
	static const float lights_levels_values[] =
	{
		0.03518,
		0.04398,
		0.05497,
		0.06871,
		0.08589,
		0.10737,
		0.13421,
		0.16777,
		0.20971,
		0.26214,
		0.32768,
		0.4096,
		0.512,
		0.64,
		0.8,
		1,
	};
	//static float lights_levels_values[] = {.0625, .125, .1875, .25, .3125, .375, .4375, .5, .5625, .625, .6875, .75, .8125, .875, .9375, 1};
	if (light < 0)
		light = 0;
	if (light > 15)
		light = 15;
	return lights_levels_values[light] * LIGHT_RANGE + LIGHT_MIN;
}

void voxel_grab_cursor(struct voxel *voxel)
{
	if (voxel->grabbed)
		return;
	gfx_window_grab_cursor(voxel->window);
	voxel->grabbed = true;
}

void voxel_ungrab_cursor(struct voxel *voxel)
{
	if (!voxel->grabbed)
		return;
	gfx_window_ungrab_cursor(voxel->window);
	voxel->grabbed = false;
}

int voxel_asprintf(char **str, const char *fmt, ...)
{
	int ret;
	va_list args;
	va_start(args, fmt);
	ret = voxel_vasprintf(str, fmt, args);
	va_end(args);
	return ret;
}

int voxel_vasprintf(char **str, const char *fmt, va_list args)
{
	int size;
	va_list tmp;
	va_copy(tmp, args);
	size = vsnprintf(NULL, 0, fmt, tmp);
	va_end(tmp);
	if (size < 0)
		return size;
	*str = malloc(size + 1);
	if (*str == NULL)
		return -1;
	return vsnprintf(*str, size + 1, fmt, args);
}

static void error_callback(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	LOG_ERRORV(fmt, ap);
	va_end(ap);
}

int main(int argc, char **argv)
{
	(void)argc;
	(void)argv;
	gfx_error_callback = error_callback;
	struct voxel voxel;
	memset(&voxel, 0, sizeof(voxel));
	voxel.smooth = true;
	voxel.ssao = true;
	g_voxel = &voxel;
	int ret = voxel_main(&voxel);
	return ret;
}
