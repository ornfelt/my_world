#include "windows/x11.h"

#include "window_vtable.h"
#include "devices.h"
#include "window.h"
#include "config.h"

#include <GL/glx.h>

#include <string.h>
#include <stdlib.h>

struct gfx_glx_window
{
	struct gfx_window window;
	struct gfx_x11_window x11;
	PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB;
	PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT;
	GLXContext context;
	GLXWindow glx_window;
	bool adaptive_vsync;
};

#ifdef GLX_WINDOW
# undef GLX_WINDOW
#endif

#define X11_WINDOW (&GLX_WINDOW->x11)
#define GLX_WINDOW ((struct gfx_glx_window*)window)

static void *get_proc_address(const char *name)
{
	return (void*)glXGetProcAddress((const GLubyte*)name);
}

static bool glx_ctr(struct gfx_window *window, struct gfx_window_properties *properties)
{
	if (!gfx_x11_ctr(X11_WINDOW, window))
		return false;
	return gfx_window_vtable.ctr(window, properties);
}

static void glx_dtr(struct gfx_window *window)
{
	gfx_x11_dtr(X11_WINDOW);
	gfx_window_vtable.dtr(window);
}

static bool glx_create_device(struct gfx_window *window)
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

static void glx_show(struct gfx_window *window)
{
	gfx_x11_show(X11_WINDOW);
}

static void glx_hide(struct gfx_window *window)
{
	gfx_x11_hide(X11_WINDOW);
}

static void glx_poll_events(struct gfx_window *window)
{
	gfx_x11_poll_events(X11_WINDOW);
}

static void glx_wait_events(struct gfx_window *window)
{
	gfx_x11_wait_events(X11_WINDOW);
}

static void glx_set_swap_interval(struct gfx_window *window, int interval)
{
	if (GLX_WINDOW->glXSwapIntervalEXT)
	{
		if (interval < 0 && !GLX_WINDOW->adaptive_vsync)
			interval = 1;
		GLX_WINDOW->glXSwapIntervalEXT(X11_WINDOW->display, GLX_WINDOW->glx_window, interval);
	}
}

static void glx_swap_buffers(struct gfx_window *window)
{
	glXSwapBuffers(X11_WINDOW->display, GLX_WINDOW->glx_window);
}

static void glx_make_current(struct gfx_window *window)
{
	glXMakeContextCurrent(X11_WINDOW->display, GLX_WINDOW->glx_window, GLX_WINDOW->glx_window, GLX_WINDOW->context);
}

static void glx_set_title(struct gfx_window *window, const char *title)
{
	gfx_x11_set_title(X11_WINDOW, title);
}

static void glx_set_icon(struct gfx_window *window, const void *data, uint32_t width, uint32_t height)
{
	gfx_x11_set_icon(X11_WINDOW, data, width, height);
}

static void glx_resize(struct gfx_window *window, uint32_t width, uint32_t height)
{
	gfx_x11_resize(X11_WINDOW, width, height);
}

static void glx_grab_cursor(struct gfx_window *window)
{
	gfx_x11_grab_cursor(X11_WINDOW);
}

static void glx_ungrab_cursor(struct gfx_window *window)
{
	gfx_x11_ungrab_cursor(X11_WINDOW);
}

static char *glx_get_clipboard(struct gfx_window *window)
{
	return gfx_x11_get_clipboard(X11_WINDOW);
}

static void glx_set_clipboard(struct gfx_window *window, const char *text)
{
	gfx_x11_set_clipboard(X11_WINDOW, text);
}

static gfx_cursor_t glx_create_native_cursor(struct gfx_window *window, enum gfx_native_cursor native_cursor)
{
	return gfx_x11_create_native_cursor(X11_WINDOW, native_cursor);
}

static gfx_cursor_t glx_create_cursor(struct gfx_window *window, const void *data, uint32_t width, uint32_t height, uint32_t xhot, uint32_t yhot)
{
	return gfx_x11_create_cursor(X11_WINDOW, data, width, height, xhot, yhot);
}

static void glx_delete_cursor(struct gfx_window *window, gfx_cursor_t cursor)
{
	gfx_x11_delete_cursor(X11_WINDOW, cursor);
}

static void glx_set_cursor(struct gfx_window *window, gfx_cursor_t cursor)
{
	gfx_x11_set_cursor(X11_WINDOW, cursor);
}

static void glx_set_mouse_position(struct gfx_window *window, int32_t x, int32_t y)
{
	gfx_x11_set_mouse_position(X11_WINDOW, x, y);
}

static const struct gfx_window_vtable glx_vtable =
{
	GFX_WINDOW_VTABLE_DEF(glx)
};

static void load_extensions(struct gfx_window *window)
{
	const char *extensions = glXQueryExtensionsString(X11_WINDOW->display, 0);
	if (strstr(extensions, "GLX_EXT_swap_control"))
		GLX_WINDOW->glXSwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)glXGetProcAddressARB((const GLubyte*)"glXSwapIntervalEXT");
	if (strstr(extensions, "GLX_ARB_create_context"))
		GLX_WINDOW->glXCreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC)glXGetProcAddressARB((const GLubyte*)"glXCreateContextAttribsARB");
	GLX_WINDOW->adaptive_vsync = (strstr(extensions, "EXT_swap_control_tear") != NULL);
}

static GLXFBConfig *get_configs(struct gfx_window *window, struct gfx_window_properties *properties)
{
	int attributes[30];
	int attributes_nb = 0;
	attributes[attributes_nb++] = GLX_RENDER_TYPE;
	attributes[attributes_nb++] = GLX_RGBA_BIT;
	attributes[attributes_nb++] = GLX_X_RENDERABLE;
	attributes[attributes_nb++] = True;
	attributes[attributes_nb++] = GLX_DRAWABLE_TYPE;
	attributes[attributes_nb++] = GLX_WINDOW_BIT;
	attributes[attributes_nb++] = GLX_DEPTH_SIZE;
	attributes[attributes_nb++] = properties->depth_bits;
	attributes[attributes_nb++] = GLX_STENCIL_SIZE;
	attributes[attributes_nb++] = properties->stencil_bits;
	attributes[attributes_nb++] = GLX_RED_SIZE;
	attributes[attributes_nb++] = properties->red_bits;
	attributes[attributes_nb++] = GLX_GREEN_SIZE;
	attributes[attributes_nb++] = properties->green_bits;
	attributes[attributes_nb++] = GLX_BLUE_SIZE;
	attributes[attributes_nb++] = properties->blue_bits;
	attributes[attributes_nb++] = GLX_ALPHA_SIZE;
	attributes[attributes_nb++] = properties->alpha_bits;
	attributes[attributes_nb++] = GLX_DOUBLEBUFFER;
	attributes[attributes_nb++] = True;
	attributes[attributes_nb++] = None;
	int configs_count;
	return glXChooseFBConfig(X11_WINDOW->display, 0, attributes, &configs_count);
}

static GLXContext create_context(struct gfx_window *window, struct gfx_window_properties *properties, XVisualInfo *vi, GLXFBConfig *configs)
{
	if (!GLX_WINDOW->glXCreateContextAttribsARB)
		return glXCreateContext(X11_WINDOW->display, vi, NULL, true);

	int attributes[10];
	int attributes_nb = 0;
#ifdef GFX_ENABLE_DEVICE_GL3
	if (properties->device_backend == GFX_DEVICE_GL3)
	{
		attributes[attributes_nb++] = GLX_CONTEXT_MAJOR_VERSION_ARB;
		attributes[attributes_nb++] = 3;
		attributes[attributes_nb++] = GLX_CONTEXT_MINOR_VERSION_ARB;
		attributes[attributes_nb++] = 3;
		attributes[attributes_nb++] = GLX_CONTEXT_PROFILE_MASK_ARB;
		attributes[attributes_nb++] = GLX_CONTEXT_CORE_PROFILE_BIT_ARB;
	}
#endif
#ifdef GFX_ENABLE_DEVICE_GL4
	if (properties->device_backend == GFX_DEVICE_GL4)
	{
		attributes[attributes_nb++] = GLX_CONTEXT_MAJOR_VERSION_ARB;
		attributes[attributes_nb++] = 4;
		attributes[attributes_nb++] = GLX_CONTEXT_MINOR_VERSION_ARB;
		attributes[attributes_nb++] = 5;
		attributes[attributes_nb++] = GLX_CONTEXT_PROFILE_MASK_ARB;
		attributes[attributes_nb++] = GLX_CONTEXT_CORE_PROFILE_BIT_ARB;
	}
#endif
#ifdef GFX_ENABLE_DEVICE_GLES3
	if (properties->device_backend == GFX_DEVICE_GLES3)
	{
		attributes[attributes_nb++] = GLX_CONTEXT_MAJOR_VERSION_ARB;
		attributes[attributes_nb++] = 3;
		attributes[attributes_nb++] = GLX_CONTEXT_MINOR_VERSION_ARB;
		attributes[attributes_nb++] = 2;
		attributes[attributes_nb++] = GLX_CONTEXT_PROFILE_MASK_ARB;
		attributes[attributes_nb++] = GLX_CONTEXT_ES2_PROFILE_BIT_EXT;
	}
#endif
	attributes[attributes_nb++] = None;
	attributes[attributes_nb++] = None;
	return GLX_WINDOW->glXCreateContextAttribsARB(X11_WINDOW->display, configs[0], NULL, true, attributes);
}

struct gfx_window *gfx_glx_window_new(const char *title, uint32_t width, uint32_t height, struct gfx_window_properties *properties)
{
	XVisualInfo *vi = NULL;
	GLXFBConfig *configs = NULL;
	struct gfx_window *window = GFX_MALLOC(sizeof(struct gfx_glx_window));
	if (!window)
	{
		GFX_ERROR_CALLBACK("allocation failed");
		return NULL;
	}
	memset(window, 0, sizeof(struct gfx_glx_window));
	window->vtable = &glx_vtable;
	if (!window->vtable->ctr(window, properties))
		goto err;
	load_extensions(window);
	configs = get_configs(window, properties);
	if (!configs)
	{
		GFX_ERROR_CALLBACK("failed to get glx configs");
		goto err;
	}
	vi = glXGetVisualFromFBConfig(X11_WINDOW->display, configs[0]);
	if (!vi)
	{
		GFX_ERROR_CALLBACK("failed to get glx vi");
		goto err;
	}
	if (!gfx_x11_create_window(X11_WINDOW, title, width, height, vi))
	{
		GFX_ERROR_CALLBACK("failed to create x11 window");
		goto err;
	}
	GLX_WINDOW->glx_window = glXCreateWindow(X11_WINDOW->display, configs[0], X11_WINDOW->window, NULL);
	if (!GLX_WINDOW->glx_window)
	{
		GFX_ERROR_CALLBACK("failed to create glx window");
		goto err;
	}
	GLX_WINDOW->context = create_context(window, properties, vi, configs);
	if (!GLX_WINDOW->context)
	{
		GFX_ERROR_CALLBACK("failed to create glx context");
		goto err;
	}
	XFree(configs);
	XFree(vi);
	return window;

err:
	window->vtable->dtr(window);
	GFX_FREE(window);
	XFree(configs);
	XFree(vi);
	return NULL;
}
