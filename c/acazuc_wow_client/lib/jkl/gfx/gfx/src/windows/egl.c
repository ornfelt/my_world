#include "windows/wl.h"

#include "window_vtable.h"
#include "window.h"
#include "config.h"

#include <wayland-egl.h>
#include <EGL/egl.h>
#include <string.h>
#include <stdlib.h>

struct gfx_egl_window
{
	struct gfx_window window;
	struct gfx_wl_window wl;
	EGLContext context;
	EGLDisplay display;
	EGLSurface surface;
	struct wl_egl_window *egl_window;
};

#ifdef EGL_WINDOW
# undef EGL_WINDOW
#endif

#define WL_WINDOW (&EGL_WINDOW->wl)
#define EGL_WINDOW ((struct gfx_egl_window*)window)

static void *get_proc_address(const char *name)
{
	return (void*)eglGetProcAddress(name);
}

static bool egl_ctr(struct gfx_window *window, struct gfx_window_properties *properties)
{
	if (!gfx_wl_ctr(WL_WINDOW, window))
		return false;
	return gfx_window_vtable.ctr(window, properties);
}

static void egl_dtr(struct gfx_window *window)
{
	eglTerminate(EGL_WINDOW->display);
	gfx_wl_dtr(WL_WINDOW);
	gfx_window_vtable.dtr(window);
}

static bool egl_create_device(struct gfx_window *window)
{
	switch (window->properties.device_backend)
	{
#ifdef GFX_ENABLE_DEVICE_GL3
		case GFX_DEVICE_GL3:
			window->device = gfx_gl3_device_new(window, get_proc_address);
			return true;
#endif
#ifdef GFX_ENABLE_DEVICE_GL4
		case GFX_DEVICE_GL4:
			window->device = gfx_gl4_device_new(window, get_proc_address);
			return true;
#endif
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
	gfx_wl_show(WL_WINDOW);
}

static void egl_hide(struct gfx_window *window)
{
	gfx_wl_hide(WL_WINDOW);
}

static void egl_poll_events(struct gfx_window *window)
{
	gfx_wl_poll_events(WL_WINDOW);
}

static void egl_wait_events(struct gfx_window *window)
{
	gfx_wl_wait_events(WL_WINDOW);
}

static void egl_set_swap_interval(struct gfx_window *window, int interval)
{
	eglSwapInterval(EGL_WINDOW->display, interval);
}

static void egl_swap_buffers(struct gfx_window *window)
{
	eglSwapBuffers(EGL_WINDOW->display, EGL_WINDOW->surface);
}

static void egl_make_current(struct gfx_window *window)
{
	eglMakeCurrent(EGL_WINDOW->display, EGL_WINDOW->surface, EGL_WINDOW->surface, EGL_WINDOW->context);
}

static void egl_set_title(struct gfx_window *window, const char *title)
{
	gfx_wl_set_title(WL_WINDOW, title);
}

static void egl_set_icon(struct gfx_window *window, const void *data, uint32_t width, uint32_t height)
{
	gfx_wl_set_icon(WL_WINDOW, data, width, height);
}

static void egl_resize(struct gfx_window *window, uint32_t width, uint32_t height)
{
	gfx_wl_resize(WL_WINDOW, width, height);
}

static void egl_grab_cursor(struct gfx_window *window)
{
	gfx_wl_grab_cursor(WL_WINDOW);
}

static void egl_ungrab_cursor(struct gfx_window *window)
{
	gfx_wl_ungrab_cursor(WL_WINDOW);
}

static char *egl_get_clipboard(struct gfx_window *window)
{
	return gfx_wl_get_clipboard(WL_WINDOW);
}

static void egl_set_clipboard(struct gfx_window *window, const char *text)
{
	gfx_wl_set_clipboard(WL_WINDOW, text);
}

static gfx_cursor_t egl_create_native_cursor(struct gfx_window *window, enum gfx_native_cursor native_cursor)
{
	return gfx_wl_create_native_cursor(WL_WINDOW, native_cursor);
}

static gfx_cursor_t egl_create_cursor(struct gfx_window *window, const void *data, uint32_t width, uint32_t height, uint32_t xhot, uint32_t yhot)
{
	return gfx_wl_create_cursor(WL_WINDOW, data, width, height, xhot, yhot);
}

static void egl_delete_cursor(struct gfx_window *window, gfx_cursor_t cursor)
{
	gfx_wl_delete_cursor(WL_WINDOW, cursor);
}

static void egl_set_cursor(struct gfx_window *window, gfx_cursor_t cursor)
{
	gfx_wl_set_cursor(WL_WINDOW, cursor);
}

static void egl_set_mouse_position(struct gfx_window *window, int32_t x, int32_t y)
{
	gfx_wl_set_mouse_position(WL_WINDOW, x, y);
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
#ifdef GFX_ENABLE_DEVICE_GL3
	if (properties->device_backend == GFX_DEVICE_GL3)
		attributes[attributes_nb++] = EGL_OPENGL_BIT;
#endif
#ifdef GFX_ENABLE_DEVICE_GL4
	if (properties->device_backend == GFX_DEVICE_GL4)
		attributes[attributes_nb++] = EGL_OPENGL_BIT;
#endif
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
	if (!eglChooseConfig(EGL_WINDOW->display, attributes, config, 1, &configs_count))
		return false;
	return configs_count == 1;
}

static EGLContext create_context(struct gfx_window *window, struct gfx_window_properties *properties, EGLConfig *config)
{
	int attributes[10];
	int attributes_nb = 0;
#ifdef GFX_ENABLE_DEVICE_GL3
	if (properties->device_backend == GFX_DEVICE_GL3)
	{
		attributes[attributes_nb++] = EGL_CONTEXT_MAJOR_VERSION;
		attributes[attributes_nb++] = 3;
		attributes[attributes_nb++] = EGL_CONTEXT_MINOR_VERSION;
		attributes[attributes_nb++] = 3;
		attributes[attributes_nb++] = EGL_CONTEXT_OPENGL_PROFILE_MASK;
		attributes[attributes_nb++] = EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT;
	}
#endif
#ifdef GFX_ENABLE_DEVICE_GL4
	if (properties->device_backend == GFX_DEVICE_GL4)
	{
		attributes[attributes_nb++] = EGL_CONTEXT_MAJOR_VERSION;
		attributes[attributes_nb++] = 4;
		attributes[attributes_nb++] = EGL_CONTEXT_MINOR_VERSION;
		attributes[attributes_nb++] = 5;
		attributes[attributes_nb++] = EGL_CONTEXT_OPENGL_PROFILE_MASK;
		attributes[attributes_nb++] = EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT;
	}
#endif
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
	EGL_WINDOW->context = eglCreateContext(EGL_WINDOW->display, config, EGL_NO_CONTEXT, attributes);
}

struct gfx_window *gfx_egl_window_new(const char *title, uint32_t width, uint32_t height, struct gfx_window_properties *properties)
{
	EGLConfig config;
	struct gfx_window *window = GFX_MALLOC(sizeof(struct gfx_egl_window));
	if (!window)
	{
		GFX_ERROR_CALLBACK("allocation failed");
		return NULL;
	}
	memset(window, 0, sizeof(struct gfx_egl_window));
	window->vtable = &egl_vtable;
	if (!window->vtable->ctr(window, properties))
		goto err;
	if (!gfx_wl_create_window(WL_WINDOW, title, width, height))
	{
		GFX_ERROR_CALLBACK("failed to create window");
		goto err;
	}
	EGL_WINDOW->display = eglGetDisplay((EGLNativeDisplayType)WL_WINDOW->display);
	if (EGL_WINDOW->display == EGL_NO_DISPLAY)
	{
		GFX_ERROR_CALLBACK("failed to create egl display");
		goto err;
	}
	if (!eglInitialize(EGL_WINDOW->display, NULL, NULL))
	{
		GFX_ERROR_CALLBACK("failed to initialized egl");
		goto err;
	}
	if (!get_config(window, properties, &config))
	{
		GFX_ERROR_CALLBACK("failed to get egl config");
		goto err;
	}
	EGL_WINDOW->window = wl_egl_window_create(WL_WINDOW->surface, width, height);
	if (!EGL_WINDOW->window)
	{
		GFX_ERROR_CALLBACK("failed to create egl window");
		goto err;
	}
	EGL_WINDOW->surface = eglCreateWindowSurface(EGL_WINDOW->display, config, EGL_WINDOW->window, NULL);
	if (EGL_WINDOW->surface == EGL_NO_SURFACE)
	{
		GFX_ERROR_CALLBACK("failed to create egl surface");
		goto err;
	}
	EGL_WINDOW->context = create_context(window, properties, &config);
	if (EGL_WINDOW->context == EGL_NO_CONTEXT)
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
