#include "window.h"
#include "window_vtable.h"
#include "devices.h"
#include "config.h"

#include <EGL/egl.h>
#include <string.h>
#include <stdlib.h>

struct gfx_emscripten_window
{
	struct gfx_window window;
	EGLContext context;
	EGLDisplay display;
	EGLSurface surface;
};

#define EMSCRIPTEN_WINDOW ((struct gfx_emscripten_window*)window)

static void *get_proc_address(const char *name)
{
	return (void*)eglGetProcAddress(name);
}

static bool egl_ctr(struct gfx_window *window, struct gfx_window_properties *properties)
{
	return gfx_window_vtable.ctr(window, properties);
}

static void egl_dtr(struct gfx_window *window)
{
	eglTerminate(EMSCRIPTEN_WINDOW->display);
	gfx_window_vtable.dtr(window);
}

static bool egl_create_device(struct gfx_window *window)
{
	switch (window->properties.device_backend)
	{
#ifdef GFX_ENABLE_DEVICE_GLES3
		case GFX_DEVICE_GLES3:
			window->device = gfx_gles3_device_new(window, get_proc_address);
			return true;
#endif
		default:
			break;
	}
	return false;
}

static void egl_show(struct gfx_window *window)
{
	(void)window;
}

static void egl_hide(struct gfx_window *window)
{
	(void)window;
}

static void egl_poll_events(struct gfx_window *window)
{
	//XXX
}

static void egl_wait_events(struct gfx_window *window)
{
	//XXX
}

static void egl_set_swap_interval(struct gfx_window *window, int interval)
{
	eglSwapInterval(EMSCRIPTEN_WINDOW->display, interval);
}

static void egl_swap_buffers(struct gfx_window *window)
{
	eglSwapBuffers(EMSCRIPTEN_WINDOW->display, EMSCRIPTEN_WINDOW->surface);
}

static void egl_make_current(struct gfx_window *window)
{
	eglMakeCurrent(EMSCRIPTEN_WINDOW->display, EMSCRIPTEN_WINDOW->surface, EMSCRIPTEN_WINDOW->surface, EMSCRIPTEN_WINDOW->context);
}

static void egl_set_title(struct gfx_window *window, const char *title)
{
	(void)window;
	(void)title;
}

static void egl_set_icon(struct gfx_window *window, const void *data, uint32_t width, uint32_t height)
{
	(void)window;
	(void)data;
	(void)width;
	(void)height;
}

static void egl_resize(struct gfx_window *window, uint32_t width, uint32_t height)
{
	(void)window;
	(void)width;
	(void)height;
}

static void egl_grab_cursor(struct gfx_window *window)
{
	(void)window;
}

static void egl_ungrab_cursor(struct gfx_window *window)
{
	(void)window;
}

static char *egl_get_clipboard(struct gfx_window *window)
{
	(void)window;
	return NULL;
}

static void egl_set_clipboard(struct gfx_window *window, const char *text)
{
	(void)window;
	(void)text;
}

static gfx_cursor_t egl_create_native_cursor(struct gfx_window *window, enum gfx_native_cursor native_cursor)
{
	(void)window;
	(void)native_cursor;
	return NULL;
}

static gfx_cursor_t egl_create_cursor(struct gfx_window *window, const void *data, uint32_t width, uint32_t height, uint32_t xhot, uint32_t yhot)
{
	(void)window;
	(void)data;
	(void)width;
	(void)height;
	(void)xhot;
	(void)yhot;
	return NULL;
}

static void egl_delete_cursor(struct gfx_window *window, gfx_cursor_t cursor)
{
	(void)window;
	(void)cursor;
}

static void egl_set_cursor(struct gfx_window *window, gfx_cursor_t cursor)
{
	(void)window;
	(void)cursor;
}

static void egl_set_mouse_position(struct gfx_window *window, int32_t x, int32_t y)
{
	(void)window;
	(void)x;
	(void)y;
}

static const struct gfx_window_vtable egl_vtable =
{
	GFX_WINDOW_VTABLE_DEF(egl)
};

static bool get_config(struct gfx_window *window, struct gfx_window_properties *properties, EGLConfig *config)
{
	int attributes[30];
	int attributes_nb = 0;
	attributes[attributes_nb++] = EGL_COLOR_BUFFER_TYPE;
	attributes[attributes_nb++] = EGL_RGB_BUFFER;
	attributes[attributes_nb++] = EGL_NATIVE_RENDERABLE;
	attributes[attributes_nb++] = EGL_TRUE;
	attributes[attributes_nb++] = EGL_SURFACE_TYPE;
	attributes[attributes_nb++] = EGL_WINDOW_BIT;
	attributes[attributes_nb++] = EGL_CONFORMANT;
#ifdef GFX_ENABLE_DEVICE_GLES3
	if (properties->device_backend == GFX_DEVICE_GLES3)
		attributes[attributes_nb++] = EGL_OPENGL_ES2_BIT;
#endif
	attributes[attributes_nb++] = EGL_DEPTH_SIZE;
	attributes[attributes_nb++] = properties->depth_bits;
	attributes[attributes_nb++] = EGL_STENCIL_SIZE;
	attributes[attributes_nb++] = properties->stencil_bits;
	attributes[attributes_nb++] = EGL_RED_SIZE;
	attributes[attributes_nb++] = properties->red_bits;
	attributes[attributes_nb++] = EGL_GREEN_SIZE;
	attributes[attributes_nb++] = properties->green_bits;
	attributes[attributes_nb++] = EGL_BLUE_SIZE;
	attributes[attributes_nb++] = properties->blue_bits;
	attributes[attributes_nb++] = EGL_ALPHA_SIZE;
	attributes[attributes_nb++] = properties->alpha_bits;
	attributes[attributes_nb++] = EGL_NONE;
	EGLint configs_count;
	if (!eglChooseConfig(EMSCRIPTEN_WINDOW->display, attributes, config, 1, &configs_count))
		return false;
	return configs_count == 1;
}

static EGLContext create_context(struct gfx_window *window, struct gfx_window_properties *properties, EGLConfig *config)
{
	int attributes[10];
	int attributes_nb = 0;
#ifdef GFX_ENABLE_DEVICE_GLES3
	if (properties->device_backend == GFX_DEVICE_GLES3)
	{
		attributes[attributes_nb++] = EGL_CONTEXT_MAJOR_VERSION;
		attributes[attributes_nb++] = 3;
		attributes[attributes_nb++] = EGL_CONTEXT_MINOR_VERSION;
		attributes[attributes_nb++] = 2;
	}
#endif
	attributes[attributes_nb++] = EGL_NONE;
	attributes[attributes_nb++] = EGL_NONE;
	EMSCRIPTEN_WINDOW->context = eglCreateContext(EMSCRIPTEN_WINDOW->display, config, EGL_NO_CONTEXT, attributes);
}

struct gfx_window *gfx_egl_window_new(const char *title, uint32_t width, uint32_t height, struct gfx_window_properties *properties)
{
	EGLConfig config;
	struct gfx_window *window = GFX_MALLOC(sizeof(struct gfx_emscripten_window));
	if (!window)
	{
		GFX_ERROR_CALLBACK("allocation failed");
		return NULL;
	}
	memset(window, 0, sizeof(struct gfx_emscripten_window));
	window->vtable = &egl_vtable;
	if (!window->vtable->ctr(window, properties))
		goto err;
	EMSCRIPTEN_WINDOW->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (EMSCRIPTEN_WINDOW->display == EGL_NO_DISPLAY)
	{
		GFX_ERROR_CALLBACK("failed to create egl display");
		goto err;
	}
	if (!eglInitialize(EMSCRIPTEN_WINDOW->display, NULL, NULL))
	{
		GFX_ERROR_CALLBACK("failed to initialized egl");
		goto err;
	}
	if (!get_config(window, properties, &config))
	{
		GFX_ERROR_CALLBACK("failed to get egl config");
		goto err;
	}
	EMSCRIPTEN_WINDOW->surface = eglCreateWindowSurface(EMSCRIPTEN_WINDOW->display, config, 0, NULL);
	if (EMSCRIPTEN_WINDOW->surface == EGL_NO_SURFACE)
	{
		GFX_ERROR_CALLBACK("failed to create egl surface");
		goto err;
	}
	EMSCRIPTEN_WINDOW->context = create_context(window, properties, &config);
	if (EMSCRIPTEN_WINDOW->context == EGL_NO_CONTEXT)
	{
		GFX_ERROR_CALLBACK("failed to create egl context");
		goto err;
	}
	return window;

err:
	window->vtable->dtr(window);
	GFX_FREE(window);
	return NULL;
}
