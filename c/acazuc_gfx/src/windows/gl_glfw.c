#include "windows/glfw.h"

#include "window_vtable.h"
#include "devices.h"
#include "window.h"

#include <string.h>
#include <stdlib.h>

struct gfx_gl_glfw_window
{
	struct gfx_window window;
	struct gfx_glfw_window glfw;
};

#define GLFW_WINDOW (&GL_GLFW_WINDOW->glfw)
#define GL_GLFW_WINDOW ((struct gfx_gl_glfw_window*)window)

static bool gl_glfw_ctr(struct gfx_window *window, struct gfx_window_properties *properties)
{
	if (!gfx_glfw_ctr(GLFW_WINDOW, window))
		return false;
	return gfx_window_vtable.ctr(window, properties);
}

static void gl_glfw_dtr(struct gfx_window *window)
{
	gfx_glfw_dtr(GLFW_WINDOW);
	gfx_window_vtable.dtr(window);
}

static bool gl_glfw_create_device(struct gfx_window *window)
{
	switch (window->properties.device_backend)
	{
#ifdef GFX_ENABLE_DEVICE_GL3
		case GFX_DEVICE_GL3:
			window->device = gfx_gl3_device_new(window, gfx_glfw_get_proc_address);
			return true;
#endif
#ifdef GFX_ENABLE_DEVICE_GL4
		case GFX_DEVICE_GL4:
			window->device = gfx_gl4_device_new(window, gfx_glfw_get_proc_address);
			return true;
#endif
#ifdef GFX_ENABLE_DEVICE_GLES3
		case GFX_DEVICE_GLES3:
			window->device = gfx_gles3_device_new(window, gfx_glfw_get_proc_address);
			return true;
#endif
		default:
			break;
	}
	return false;
}

static void gl_glfw_show(struct gfx_window *window)
{
	gfx_glfw_show(GLFW_WINDOW);
}

static void gl_glfw_hide(struct gfx_window *window)
{
	gfx_glfw_hide(GLFW_WINDOW);
}

static void gl_glfw_poll_events(struct gfx_window *window)
{
	gfx_glfw_poll_events(GLFW_WINDOW);
}

static void gl_glfw_wait_events(struct gfx_window *window)
{
	gfx_glfw_wait_events(GLFW_WINDOW);
}

static void gl_glfw_grab_cursor(struct gfx_window *window)
{
	gfx_glfw_grab_cursor(GLFW_WINDOW);
}

static void gl_glfw_ungrab_cursor(struct gfx_window *window)
{
	gfx_glfw_ungrab_cursor(GLFW_WINDOW);
}

static void gl_glfw_swap_buffers(struct gfx_window *window)
{
	glfwSwapBuffers(GLFW_WINDOW->window);
}

static void gl_glfw_make_current(struct gfx_window *window)
{
	glfwMakeContextCurrent(GLFW_WINDOW->window);
}

static void gl_glfw_set_swap_interval(struct gfx_window *window, int interval)
{
	(void)window;
	glfwSwapInterval(interval);
}

static void gl_glfw_set_title(struct gfx_window *window, const char *title)
{
	gfx_glfw_set_title(GLFW_WINDOW, title);
}

static void gl_glfw_set_icon(struct gfx_window *window, const void *data, uint32_t width, uint32_t height)
{
	gfx_glfw_set_icon(GLFW_WINDOW, data, width, height);
}

static void gl_glfw_resize(struct gfx_window *window, uint32_t width, uint32_t height)
{
	gfx_glfw_resize(GLFW_WINDOW, width, height);
}

static char *gl_glfw_get_clipboard(struct gfx_window *window)
{
	return gfx_glfw_get_clipboard(GLFW_WINDOW);
}

static void gl_glfw_set_clipboard(struct gfx_window *window, const char *text)
{
	gfx_glfw_set_clipboard(GLFW_WINDOW, text);
}

static gfx_cursor_t gl_glfw_create_native_cursor(struct gfx_window *window, enum gfx_native_cursor cursor)
{
	return gfx_glfw_create_native_cursor(GLFW_WINDOW, cursor);
}

static gfx_cursor_t gl_glfw_create_cursor(struct gfx_window *window, const void *data, uint32_t width, uint32_t height, uint32_t xhot, uint32_t yhot)
{
	return gfx_glfw_create_cursor(GLFW_WINDOW, data, width, height, xhot, yhot);
}

static void gl_glfw_delete_cursor(struct gfx_window *window, gfx_cursor_t cursor)
{
	gfx_glfw_delete_cursor(GLFW_WINDOW, cursor);
}

static void gl_glfw_set_cursor(struct gfx_window *window, gfx_cursor_t cursor)
{
	gfx_glfw_set_cursor(GLFW_WINDOW, cursor);
}

static void gl_glfw_set_mouse_position(struct gfx_window *window, int32_t x, int32_t y)
{
	gfx_glfw_set_mouse_position(GLFW_WINDOW, x, y);
}

static const struct gfx_window_vtable gl_glfw_vtable =
{
	GFX_WINDOW_VTABLE_DEF(gl_glfw)
};

struct gfx_window *gfx_gl_glfw_window_new(const char *title, uint32_t width, uint32_t height, struct gfx_window_properties *properties)
{
	struct gfx_window *window = GFX_MALLOC(sizeof(struct gfx_gl_glfw_window));
	if (!window)
	{
		GFX_ERROR_CALLBACK("allocation failed");
		return NULL;
	}
	memset(window, 0, sizeof(struct gfx_gl_glfw_window));
	window->vtable = &gl_glfw_vtable;
	if (!window->vtable->ctr(window, properties))
		goto err;
	window->width = width;
	window->height = height;
	if (!gfx_glfw_create_window(GLFW_WINDOW, title, width, height, properties))
		goto err;
	return window;

err:
	window->vtable->dtr(window);
	GFX_FREE(window);
	return NULL;
}
