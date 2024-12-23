#include "windows/sdl.h"

#include "window_vtable.h"
#include "devices.h"
#include "window.h"

#include <string.h>
#include <stdlib.h>

struct gfx_gl_sdl_window
{
	struct gfx_window window;
	struct gfx_sdl_window sdl;
	SDL_GLContext context;
};

#define SDL_WINDOW (&GL_SDL_WINDOW->sdl)
#define GL_SDL_WINDOW ((struct gfx_gl_sdl_window*)window)

static bool gl_sdl_ctr(struct gfx_window *window, struct gfx_window_properties *properties)
{
	if (!gfx_sdl_ctr(SDL_WINDOW, window))
		return false;
	return gfx_window_vtable.ctr(window, properties);
}

static void gl_sdl_dtr(struct gfx_window *window)
{
	gfx_sdl_dtr(SDL_WINDOW);
	gfx_window_vtable.dtr(window);
}

static bool gl_sdl_create_device(struct gfx_window *window)
{
	switch (window->properties.device_backend)
	{
#ifdef GFX_ENABLE_DEVICE_GL3
		case GFX_DEVICE_GL3:
			window->device = gfx_gl3_device_new(window, gfx_sdl_get_proc_address);
			return true;
#endif
#ifdef GFX_ENABLE_DEVICE_GL4
		case GFX_DEVICE_GL4:
			window->device = gfx_gl4_device_new(window, gfx_sdl_get_proc_address);
			return true;
#endif
#ifdef GFX_ENABLE_DEVICE_GLES3
		case GFX_DEVICE_GLES3:
			window->device = gfx_gles3_device_new(window, gfx_sdl_get_proc_address);
			return true;
#endif
		default:
			break;
	}
	return false;
}

static void gl_sdl_show(struct gfx_window *window)
{
	gfx_sdl_show(SDL_WINDOW);
}

static void gl_sdl_hide(struct gfx_window *window)
{
	gfx_sdl_hide(SDL_WINDOW);
}

static void gl_sdl_poll_events(struct gfx_window *window)
{
	gfx_sdl_poll_events(SDL_WINDOW);
}

static void gl_sdl_wait_events(struct gfx_window *window)
{
	gfx_sdl_wait_events(SDL_WINDOW);
}

static void gl_sdl_grab_cursor(struct gfx_window *window)
{
	gfx_sdl_grab_cursor(SDL_WINDOW);
}

static void gl_sdl_ungrab_cursor(struct gfx_window *window)
{
	gfx_sdl_ungrab_cursor(SDL_WINDOW);
}

static void gl_sdl_swap_buffers(struct gfx_window *window)
{
	SDL_GL_SwapWindow(SDL_WINDOW->window);
}

static void gl_sdl_make_current(struct gfx_window *window)
{
	SDL_GL_MakeCurrent(SDL_WINDOW->window, GL_SDL_WINDOW->context);
}

static void gl_sdl_set_swap_interval(struct gfx_window *window, int interval)
{
	(void)window;
	SDL_GL_SetSwapInterval(interval);
}

static void gl_sdl_set_title(struct gfx_window *window, const char *title)
{
	gfx_sdl_set_title(SDL_WINDOW, title);
}

static void gl_sdl_set_icon(struct gfx_window *window, const void *data, uint32_t width, uint32_t height)
{
	gfx_sdl_set_icon(SDL_WINDOW, data, width, height);
}

static void gl_sdl_resize(struct gfx_window *window, uint32_t width, uint32_t height)
{
	gfx_sdl_resize(SDL_WINDOW, width, height);
}

static char *gl_sdl_get_clipboard(struct gfx_window *window)
{
	return gfx_sdl_get_clipboard(SDL_WINDOW);
}

static void gl_sdl_set_clipboard(struct gfx_window *window, const char *text)
{
	gfx_sdl_set_clipboard(SDL_WINDOW, text);
}

static gfx_cursor_t gl_sdl_create_native_cursor(struct gfx_window *window, enum gfx_native_cursor cursor)
{
	return gfx_sdl_create_native_cursor(SDL_WINDOW, cursor);
}

static gfx_cursor_t gl_sdl_create_cursor(struct gfx_window *window, const void *data, uint32_t width, uint32_t height, uint32_t xhot, uint32_t yhot)
{
	return gfx_sdl_create_cursor(SDL_WINDOW, data, width, height, xhot, yhot);
}

static void gl_sdl_delete_cursor(struct gfx_window *window, gfx_cursor_t cursor)
{
	gfx_sdl_delete_cursor(SDL_WINDOW, cursor);
}

static void gl_sdl_set_cursor(struct gfx_window *window, gfx_cursor_t cursor)
{
	gfx_sdl_set_cursor(SDL_WINDOW, cursor);
}

static void gl_sdl_set_mouse_position(struct gfx_window *window, int32_t x, int32_t y)
{
	gfx_sdl_set_mouse_position(SDL_WINDOW, x, y);
}

static const struct gfx_window_vtable gl_sdl_vtable =
{
	GFX_WINDOW_VTABLE_DEF(gl_sdl)
};

struct gfx_window *gfx_gl_sdl_window_new(const char *title, uint32_t width, uint32_t height, struct gfx_window_properties *properties)
{
	struct gfx_window *window = GFX_MALLOC(sizeof(struct gfx_gl_sdl_window));
	if (!window)
	{
		GFX_ERROR_CALLBACK("allocation failed");
		return NULL;
	}
	memset(window, 0, sizeof(struct gfx_gl_sdl_window));
	window->vtable = &gl_sdl_vtable;
	if (!window->vtable->ctr(window, properties))
		goto err;
	window->width = width;
	window->height = height;
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, properties->depth_bits);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, properties->stencil_bits);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, properties->red_bits);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, properties->green_bits);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, properties->blue_bits);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, properties->alpha_bits);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	switch (properties->device_backend)
	{
#ifdef GFX_ENABLE_DEVICE_GL3
		case GFX_DEVICE_GL3:
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
			break;
#endif
#ifdef GFX_ENABLE_DEVICE_GL4
		case GFX_DEVICE_GL4:
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
			break;
#endif
#ifdef GFX_ENABLE_DEVICE_GLES3
		case GFX_DEVICE_GLES3:
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
			break;
#endif
		default:
			GFX_ERROR_CALLBACK("unknown sdl backend");
			return false;
	}
	if (!gfx_sdl_create_window(SDL_WINDOW, title, width, height, properties))
		goto err;
	SDL_GL_CreateContext(SDL_WINDOW->window);
	GL_SDL_WINDOW->context = SDL_GL_GetCurrentContext();
	return window;

err:
	window->vtable->dtr(window);
	GFX_FREE(window);
	return NULL;
}
