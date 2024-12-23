#include "windows/win32.h"

#include "window_vtable.h"
#include "devices.h"
#include "window.h"
#include "config.h"

#include <windows.h>
#include <string.h>

#include <GL/glcorearb.h>
#include <GL/wglext.h>

struct gfx_wgl_window
{
	struct gfx_window window;
	struct gfx_win32_window win32;
	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
	PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB;
	PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
	HGLRC context;
	HDC device;
	int32_t prev_mouse_x;
	int32_t prev_mouse_y;
	bool adaptive_vsync;
};

#define WIN32_WINDOW (&WGL_WINDOW->win32)
#define GL_WINDOW (&window->gl)
#define WGL_WINDOW ((struct gfx_wgl_window*)window)

static void *get_proc_address(const char *name)
{
	void *ret = (void*)wglGetProcAddress(name);
	if (ret == NULL || ret  == (void*)0x1 || ret == (void*)0x2 || ret == (void*)0x3 || ret == (void*)-1)
	{
		HMODULE module = LoadLibraryA("opengl32.dll");
		ret = (void*)GetProcAddress(module, name);
	}
	return ret;
}

static bool wgl_ctr(struct gfx_window *window, struct gfx_window_properties *properties)
{
	return gfx_window_vtable.ctr(window, properties);
}

static void wgl_dtr(struct gfx_window *window)
{
	gfx_win32_dtr(WIN32_WINDOW);
	gfx_window_vtable.dtr(window);
}

static bool wgl_create_device(struct gfx_window *window)
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

static void wgl_show(struct gfx_window *window)
{
	gfx_win32_show(WIN32_WINDOW);
}

static void wgl_hide(struct gfx_window *window)
{
	gfx_win32_hide(WIN32_WINDOW);
}

static void wgl_set_title(struct gfx_window *window, const char *title)
{
	gfx_win32_set_title(WIN32_WINDOW, title);
}

static void wgl_set_icon(struct gfx_window *window, const void *data, uint32_t width, uint32_t height)
{
	gfx_win32_set_icon(WIN32_WINDOW, data, width, height);
}

static void wgl_poll_events(struct gfx_window *window)
{
	gfx_win32_poll_events(WIN32_WINDOW);
}

static void wgl_wait_events(struct gfx_window *window)
{
	gfx_win32_wait_events(WIN32_WINDOW);
}

static void wgl_set_swap_interval(struct gfx_window *window, int interval)
{
	WGL_WINDOW->wglSwapIntervalEXT(interval);
}

static void wgl_swap_buffers(struct gfx_window *window)
{
	SwapBuffers(WGL_WINDOW->device);
}

static void wgl_make_current(struct gfx_window *window)
{
	wglMakeCurrent(WGL_WINDOW->device, WGL_WINDOW->context);
}

static void wgl_resize(struct gfx_window *window, uint32_t width, uint32_t height)
{
	gfx_win32_resize(WIN32_WINDOW, width, height);
}

static void wgl_grab_cursor(struct gfx_window *window)
{
	gfx_win32_grab_cursor(WIN32_WINDOW);
}

static void wgl_ungrab_cursor(struct gfx_window *window)
{
	gfx_win32_ungrab_cursor(WIN32_WINDOW);
}

static char *wgl_get_clipboard(struct gfx_window *window)
{
	return gfx_win32_get_clipboard(WIN32_WINDOW);
}

static void wgl_set_clipboard(struct gfx_window *window, const char *text)
{
	gfx_win32_set_clipboard(WIN32_WINDOW, text);
}

static gfx_cursor_t wgl_create_native_cursor(struct gfx_window *window, enum gfx_native_cursor cursor)
{
	return gfx_win32_create_native_cursor(WIN32_WINDOW, cursor);
}

static gfx_cursor_t wgl_create_cursor(struct gfx_window *window, const void *data, uint32_t width, uint32_t height, uint32_t xhot, uint32_t yhot)
{
	return gfx_win32_create_cursor(WIN32_WINDOW, data, width, height, xhot, yhot);
}

static void wgl_delete_cursor(struct gfx_window *window, gfx_cursor_t cursor)
{
	gfx_win32_delete_cursor(WIN32_WINDOW, cursor);
}

static void wgl_set_cursor(struct gfx_window *window, gfx_cursor_t cursor)
{
	gfx_win32_set_cursor(WIN32_WINDOW, cursor);
}

static void wgl_set_mouse_position(struct gfx_window *window, int32_t x, int32_t y)
{
	gfx_win32_set_mouse_position(WIN32_WINDOW, x, y);
}

static const struct gfx_window_vtable wgl_vtable =
{
	GFX_WINDOW_VTABLE_DEF(wgl)
};

struct gfx_window *gfx_wgl_window_new(const char *title, uint32_t width, uint32_t height, struct gfx_window_properties *properties)
{
	struct gfx_window *window = GFX_MALLOC(sizeof(struct gfx_wgl_window));
	if (!window)
	{
		GFX_ERROR_CALLBACK("malloc failed");
		return NULL;
	}
	memset(window, 0, sizeof(struct gfx_wgl_window));
	window->vtable = &wgl_vtable;
	if (!window->vtable->ctr(window, properties))
		goto err;
	gfx_win32_ctr(WIN32_WINDOW, window);
	if (!gfx_win32_create_window(WIN32_WINDOW, title, width, height))
	{
		GFX_ERROR_CALLBACK("failed to create window");
		goto err;
	}
	WGL_WINDOW->device = GetDC(WIN32_WINDOW->window);
	if (!WGL_WINDOW->device)
	{
		GFX_ERROR_CALLBACK("GetDC failed");
		goto err;
	}
	HGLRC tmp_ctx;
	{
		PIXELFORMATDESCRIPTOR pfd =
		{
			sizeof(PIXELFORMATDESCRIPTOR),
			1,
			PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
			PFD_TYPE_RGBA,
			32,
			0, 0, 0, 0, 0, 0,
			0,
			0,
			0,
			0, 0, 0, 0,
			24,
			8,
			0,
			PFD_MAIN_PLANE,
			0,
			0, 0, 0
		};
		int pformat = ChoosePixelFormat(WGL_WINDOW->device, &pfd);
		SetPixelFormat(WGL_WINDOW->device, pformat, &pfd);
		tmp_ctx = wglCreateContext(WGL_WINDOW->device);
		if (!tmp_ctx)
		{
			GFX_ERROR_CALLBACK("wglCreateContext failed: %u", (unsigned)GetLastError());
			goto err;
		}
		wglMakeCurrent(WGL_WINDOW->device, tmp_ctx);
	}
	WGL_WINDOW->wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
	WGL_WINDOW->wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
	WGL_WINDOW->wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
	wglDeleteContext(tmp_ctx);
	if (!WGL_WINDOW->wglGetExtensionsStringARB)
	{
		GFX_ERROR_CALLBACK("no wglGetExtensionsStringARB");
		goto err;
	}
	if (!WGL_WINDOW->wglSwapIntervalEXT)
	{
		GFX_ERROR_CALLBACK("no wglSwapIntervalEXT");
		goto err;
	}
	{
		PIXELFORMATDESCRIPTOR pfd;
		memset(&pfd, 0, sizeof(pfd));
		pfd.nSize = sizeof(pfd);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = properties->red_bits + properties->green_bits + properties->blue_bits + properties->alpha_bits;
		pfd.cRedBits = properties->red_bits;
		pfd.cGreenBits = properties->green_bits;
		pfd.cBlueBits = properties->blue_bits;
		pfd.cAlphaBits = properties->alpha_bits;
		pfd.cDepthBits = properties->depth_bits;
		pfd.cStencilBits = properties->stencil_bits;
		pfd.iLayerType = PFD_MAIN_PLANE;
		int pformat = ChoosePixelFormat(WGL_WINDOW->device, &pfd);
		SetPixelFormat(WGL_WINDOW->device, pformat, &pfd);
	}
	if (WGL_WINDOW->wglCreateContextAttribsARB)
	{
		int attributes[20];
		size_t attributes_nb = 0;
#ifdef GFX_ENABLE_DEVICE_GL3
		if (properties->device_backend == GFX_DEVICE_GL3)
		{
			attributes[attributes_nb++] = WGL_CONTEXT_MAJOR_VERSION_ARB;
			attributes[attributes_nb++] = 3;
			attributes[attributes_nb++] = WGL_CONTEXT_MINOR_VERSION_ARB;
			attributes[attributes_nb++] = 3;
			attributes[attributes_nb++] = WGL_CONTEXT_PROFILE_MASK_ARB;
			attributes[attributes_nb++] = WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
		}
#endif
#ifdef GFX_ENABLE_DEVICE_GL4
		if (properties->device_backend == GFX_DEVICE_GL4)
		{
			attributes[attributes_nb++] = WGL_CONTEXT_MAJOR_VERSION_ARB;
			attributes[attributes_nb++] = 4;
			attributes[attributes_nb++] = WGL_CONTEXT_MINOR_VERSION_ARB;
			attributes[attributes_nb++] = 5;
			attributes[attributes_nb++] = WGL_CONTEXT_PROFILE_MASK_ARB;
			attributes[attributes_nb++] = WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
		}
#endif
#ifdef GFX_ENABLE_DEVICE_GLES3
		if (properties->device_backend == GFX_DEVICE_GLES3)
		{
			attributes[attributes_nb++] = WGL_CONTEXT_MAJOR_VERSION_ARB;
			attributes[attributes_nb++] = 3;
			attributes[attributes_nb++] = WGL_CONTEXT_MINOR_VERSION_ARB;
			attributes[attributes_nb++] = 2;
			attributes[attributes_nb++] = WGL_CONTEXT_PROFILE_MASK_ARB;
			attributes[attributes_nb++] = WGL_CONTEXT_ES2_PROFILE_BIT_EXT;
		}
#endif
		attributes[attributes_nb++] = 0;
		attributes[attributes_nb++] = 0;
		WGL_WINDOW->context = WGL_WINDOW->wglCreateContextAttribsARB(WGL_WINDOW->device, NULL, attributes);
		if (!WGL_WINDOW->context)
		{
			GFX_ERROR_CALLBACK("wglCreateContextAttribsARB failed: %u", (unsigned)GetLastError());
			goto err;
		}
	}
	else
	{
		WGL_WINDOW->context = wglCreateContext(WGL_WINDOW->device);
		if (!WGL_WINDOW->context)
		{
			GFX_ERROR_CALLBACK("wglCreateContext failed: %u", (unsigned)GetLastError());
			goto err;
		}
	}
	return window;

err:
	window->vtable->dtr(window);
	GFX_FREE(window);
	return NULL;
}
