#include "windows/x11.h"

#include "window_vtable.h"
#include "devices.h"
#include "window.h"
#include "config.h"

#include <vulkan/vulkan_xlib.h>
#include <string.h>
#include <stdlib.h>

struct gfx_vk_x11_window
{
	struct gfx_window window;
	struct gfx_x11_window x11;
};

#define X11_WINDOW (&VK_X11_WINDOW->x11)
#define VK_X11_WINDOW ((struct gfx_vk_x11_window*)window)

static bool vk_x11_ctr(struct gfx_window *window, struct gfx_window_properties *properties)
{
	if (!gfx_x11_ctr(X11_WINDOW, window))
		return false;
	return gfx_window_vtable.ctr(window, properties);
}

static void vk_x11_dtr(struct gfx_window *window)
{
	gfx_x11_dtr(X11_WINDOW);
	gfx_window_vtable.dtr(window);
}

static bool vk_x11_create_device(struct gfx_window *window)
{
	switch (window->properties.device_backend)
	{
#ifdef GFX_ENABLE_DEVICE_VK
		case GFX_DEVICE_VK:
		{
			VkResult result;
			VkInstance instance;
			VkSurfaceKHR surface;
			{
				VkApplicationInfo application_info;
				application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
				application_info.pNext = NULL;
				application_info.pApplicationName = "gfx";
				application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
				application_info.pEngineName = "gfx";
				application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
				application_info.apiVersion = VK_API_VERSION_1_1;
				static const char *extensions[] =
				{
					"VK_KHR_surface",
					"VK_KHR_xlib_surface",
				};
				const char *layers[] =
				{
				};
				VkInstanceCreateInfo create_info;
				create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
				create_info.pNext = NULL;
				create_info.flags = 0;
				create_info.pApplicationInfo = &application_info;
				create_info.enabledLayerCount = sizeof(layers) / sizeof(*layers);
				create_info.ppEnabledLayerNames = layers;
				create_info.enabledExtensionCount = sizeof(extensions) / sizeof(*extensions);
				create_info.ppEnabledExtensionNames = extensions;
				result = vkCreateInstance(&create_info, NULL, &instance);
				if (result != VK_SUCCESS)
				{
					GFX_ERROR_CALLBACK("can't create x11 vulkan instance");
					return false;
				}
			}
			{
				VkXlibSurfaceCreateInfoKHR create_info;
				create_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
				create_info.pNext = NULL;
				create_info.flags = 0;
				create_info.dpy = X11_WINDOW->display;
				create_info.window = X11_WINDOW->window;
				result = vkCreateXlibSurfaceKHR(instance, &create_info, NULL, &surface);
				if (result != VK_SUCCESS)
				{
					GFX_ERROR_CALLBACK("can't create x11 vulkan surface");
					vkDestroyInstance(instance, NULL);
					return false;
				}
			}
			window->device = gfx_vk_device_new(window, instance, surface);
			return true;
		}
#endif
		default:
			break;
	}
	return false;
}

static void vk_x11_show(struct gfx_window *window)
{
	gfx_x11_show(X11_WINDOW);
}

static void vk_x11_hide(struct gfx_window *window)
{
	gfx_x11_hide(X11_WINDOW);
}

static void vk_x11_poll_events(struct gfx_window *window)
{
	gfx_x11_poll_events(X11_WINDOW);
}

static void vk_x11_wait_events(struct gfx_window *window)
{
	gfx_x11_wait_events(X11_WINDOW);
}

static void vk_x11_set_swap_interval(struct gfx_window *window, int interval)
{
	gfx_vk_set_swap_interval(window->device, interval);
}

static void vk_x11_swap_buffers(struct gfx_window *window)
{
	gfx_vk_swap_buffers(window->device);
}

static void vk_x11_make_current(struct gfx_window *window)
{
	(void)window;
}

static void vk_x11_set_title(struct gfx_window *window, const char *title)
{
	gfx_x11_set_title(X11_WINDOW, title);
}

static void vk_x11_set_icon(struct gfx_window *window, const void *data, uint32_t width, uint32_t height)
{
	gfx_x11_set_icon(X11_WINDOW, data, width, height);
}

static void vk_x11_resize(struct gfx_window *window, uint32_t width, uint32_t height)
{
	gfx_x11_resize(X11_WINDOW, width, height);
}

static void vk_x11_grab_cursor(struct gfx_window *window)
{
	gfx_x11_grab_cursor(X11_WINDOW);
}

static void vk_x11_ungrab_cursor(struct gfx_window *window)
{
	gfx_x11_ungrab_cursor(X11_WINDOW);
}

static char *vk_x11_get_clipboard(struct gfx_window *window)
{
	return gfx_x11_get_clipboard(X11_WINDOW);
}

static void vk_x11_set_clipboard(struct gfx_window *window, const char *text)
{
	gfx_x11_set_clipboard(X11_WINDOW, text);
}

static gfx_cursor_t vk_x11_create_native_cursor(struct gfx_window *window, enum gfx_native_cursor native_cursor)
{
	return gfx_x11_create_native_cursor(X11_WINDOW, native_cursor);
}

static gfx_cursor_t vk_x11_create_cursor(struct gfx_window *window, const void *data, uint32_t width, uint32_t height, uint32_t xhot, uint32_t yhot)
{
	return gfx_x11_create_cursor(X11_WINDOW, data, width, height, xhot, yhot);
}

static void vk_x11_delete_cursor(struct gfx_window *window, gfx_cursor_t cursor)
{
	gfx_x11_delete_cursor(X11_WINDOW, cursor);
}

static void vk_x11_set_cursor(struct gfx_window *window, gfx_cursor_t cursor)
{
	gfx_x11_set_cursor(X11_WINDOW, cursor);
}

static void vk_x11_set_mouse_position(struct gfx_window *window, int32_t x, int32_t y)
{
	gfx_x11_set_mouse_position(X11_WINDOW, x, y);
}

static const struct gfx_window_vtable vk_x11_vtable =
{
	GFX_WINDOW_VTABLE_DEF(vk_x11)
};

struct gfx_window *gfx_vk_x11_window_new(const char *title, uint32_t width, uint32_t height, struct gfx_window_properties *properties)
{
	XVisualInfo *vi;
	struct gfx_window *window = GFX_MALLOC(sizeof(struct gfx_vk_x11_window));
	if (!window)
	{
		GFX_ERROR_CALLBACK("allocation failed");
		return NULL;
	}
	memset(window, 0, sizeof(struct gfx_vk_x11_window));
	window->vtable = &vk_x11_vtable;
	if (!window->vtable->ctr(window, properties))
		goto err;
	int nitems;
	vi = XGetVisualInfo(X11_WINDOW->display, VisualNoMask, NULL, &nitems);
	if (nitems == 0)
	{
		GFX_ERROR_CALLBACK("can't get visual info");
		goto err;
	}
	if (!gfx_x11_create_window(X11_WINDOW, title, width, height, vi))
	{
		GFX_ERROR_CALLBACK("failed to create window");
		goto err;
	}
	XFree(vi);
	return window;

err:
	window->vtable->dtr(window);
	XFree(vi);
	GFX_FREE(window);
	return NULL;
}
