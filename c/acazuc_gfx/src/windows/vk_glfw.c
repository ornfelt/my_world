#define GLFW_INCLUDE_VULKAN
#include "windows/glfw.h"

#include "window_vtable.h"
#include "devices.h"
#include "window.h"
#include "config.h"

#include <string.h>
#include <stdlib.h>

struct gfx_vk_glfw_window
{
	struct gfx_window window;
	struct gfx_glfw_window glfw;
};

#define GLFW_WINDOW (&VK_GLFW_WINDOW->glfw)
#define VK_GLFW_WINDOW ((struct gfx_vk_glfw_window*)window)

static bool vk_glfw_ctr(struct gfx_window *window, struct gfx_window_properties *properties)
{
	if (!gfx_glfw_ctr(GLFW_WINDOW, window))
		return false;
	return gfx_window_vtable.ctr(window, properties);
}

static void vk_glfw_dtr(struct gfx_window *window)
{
	gfx_glfw_dtr(GLFW_WINDOW);
	gfx_window_vtable.dtr(window);
}

static bool vk_glfw_create_device(struct gfx_window *window)
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
				uint32_t count;
				const char **extensions = glfwGetRequiredInstanceExtensions(&count);
				VkApplicationInfo application_info;
				application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
				application_info.pNext = NULL;
				application_info.pApplicationName = "gfx";
				application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
				application_info.pEngineName = "gfx";
				application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
				application_info.apiVersion = VK_API_VERSION_1_1;
				VkInstanceCreateInfo create_info;
				create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
				create_info.pNext = NULL;
				create_info.flags = 0;
				create_info.pApplicationInfo = &application_info;
				const char *layers[] =
				{
				};
				create_info.enabledLayerCount = sizeof(layers) / sizeof(*layers);
				create_info.ppEnabledLayerNames = layers;
				create_info.enabledExtensionCount = count;
				create_info.ppEnabledExtensionNames = extensions;
				result = vkCreateInstance(&create_info, NULL, &instance);
				if (result != VK_SUCCESS)
				{
					GFX_ERROR_CALLBACK("can't create glfw vulkan instance");
					return false;
				}
			}
			{
				result = glfwCreateWindowSurface(instance, GLFW_WINDOW->window, NULL, &surface);
				if (result != VK_SUCCESS)
				{
					GFX_ERROR_CALLBACK("can't create glfw vulkan surface");
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

static void vk_glfw_show(struct gfx_window *window)
{
	gfx_glfw_show(GLFW_WINDOW);
}

static void vk_glfw_hide(struct gfx_window *window)
{
	gfx_glfw_hide(GLFW_WINDOW);
}

static void vk_glfw_poll_events(struct gfx_window *window)
{
	gfx_glfw_poll_events(GLFW_WINDOW);
}

static void vk_glfw_wait_events(struct gfx_window *window)
{
	gfx_glfw_wait_events(GLFW_WINDOW);
}

static void vk_glfw_grab_cursor(struct gfx_window *window)
{
	gfx_glfw_grab_cursor(GLFW_WINDOW);
}

static void vk_glfw_ungrab_cursor(struct gfx_window *window)
{
	gfx_glfw_ungrab_cursor(GLFW_WINDOW);
}

static void vk_glfw_set_swap_interval(struct gfx_window *window, int interval)
{
	gfx_vk_set_swap_interval(window->device, interval);
}

static void vk_glfw_swap_buffers(struct gfx_window *window)
{
	gfx_vk_swap_buffers(window->device);
}

static void vk_glfw_make_current(struct gfx_window *window)
{
	(void)window;
}

static void vk_glfw_set_title(struct gfx_window *window, const char *title)
{
	gfx_glfw_set_title(GLFW_WINDOW, title);
}

static void vk_glfw_set_icon(struct gfx_window *window, const void *data, uint32_t width, uint32_t height)
{
	gfx_glfw_set_icon(GLFW_WINDOW, data, width, height);
}

static void vk_glfw_resize(struct gfx_window *window, uint32_t width, uint32_t height)
{
	gfx_glfw_resize(GLFW_WINDOW, width, height);
}

static char *vk_glfw_get_clipboard(struct gfx_window *window)
{
	return gfx_glfw_get_clipboard(GLFW_WINDOW);
}

static void vk_glfw_set_clipboard(struct gfx_window *window, const char *text)
{
	gfx_glfw_set_clipboard(GLFW_WINDOW, text);
}

static gfx_cursor_t vk_glfw_create_native_cursor(struct gfx_window *window, enum gfx_native_cursor cursor)
{
	return gfx_glfw_create_native_cursor(GLFW_WINDOW, cursor);
}

static gfx_cursor_t vk_glfw_create_cursor(struct gfx_window *window, const void *data, uint32_t width, uint32_t height, uint32_t xhot, uint32_t yhot)
{
	return gfx_glfw_create_cursor(GLFW_WINDOW, data, width, height, xhot, yhot);
}

static void vk_glfw_delete_cursor(struct gfx_window *window, gfx_cursor_t cursor)
{
	gfx_glfw_delete_cursor(GLFW_WINDOW, cursor);
}

static void vk_glfw_set_cursor(struct gfx_window *window, gfx_cursor_t cursor)
{
	gfx_glfw_set_cursor(GLFW_WINDOW, cursor);
}

static void vk_glfw_set_mouse_position(struct gfx_window *window, int32_t x, int32_t y)
{
	gfx_glfw_set_mouse_position(GLFW_WINDOW, x, y);
}

static const struct gfx_window_vtable vk_glfw_vtable =
{
	GFX_WINDOW_VTABLE_DEF(vk_glfw)
};

struct gfx_window *gfx_vk_glfw_window_new(const char *title, uint32_t width, uint32_t height, struct gfx_window_properties *properties)
{
	struct gfx_window *window = GFX_MALLOC(sizeof(struct gfx_vk_glfw_window));
	if (!window)
	{
		GFX_ERROR_CALLBACK("allocation failed");
		return NULL;
	}
	memset(window, 0, sizeof(struct gfx_vk_glfw_window));
	window->vtable = &vk_glfw_vtable;
	if (!window->vtable->ctr(window, properties))
		goto err;
	if (!glfwVulkanSupported())
	{
		GFX_ERROR_CALLBACK("vulkan not supported");
		goto err;
	}
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