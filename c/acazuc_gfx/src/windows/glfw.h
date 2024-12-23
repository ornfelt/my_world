#ifndef GFX_GLFW_WINDOW_H
#define GFX_GLFW_WINDOW_H

#include "window.h"

#include <GLFW/glfw3.h>

struct gfx_glfw_window
{
	struct gfx_window *winref;
	GLFWwindow *window;
};

void *gfx_glfw_get_proc_address(const char *name);
bool gfx_glfw_create_window(struct gfx_glfw_window *window, const char *title, uint32_t width, uint32_t height, struct gfx_window_properties *properties);
bool gfx_glfw_ctr(struct gfx_glfw_window *window, struct gfx_window *winref);
void gfx_glfw_dtr(struct gfx_glfw_window *window);
void gfx_glfw_show(struct gfx_glfw_window *window);
void gfx_glfw_hide(struct gfx_glfw_window *window);
void gfx_glfw_poll_events(struct gfx_glfw_window *window);
void gfx_glfw_wait_events(struct gfx_glfw_window *window);
void gfx_glfw_set_title(struct gfx_glfw_window *window, const char *title);
void gfx_glfw_set_icon(struct gfx_glfw_window *window, const void *data, uint32_t width, uint32_t height);
void gfx_glfw_resize(struct gfx_glfw_window *window, uint32_t width, uint32_t height);
void gfx_glfw_grab_cursor(struct gfx_glfw_window *window);
void gfx_glfw_ungrab_cursor(struct gfx_glfw_window *window);
char *gfx_glfw_get_clipboard(struct gfx_glfw_window *window);
void gfx_glfw_set_clipboard(struct gfx_glfw_window *window, const char *text);
gfx_cursor_t gfx_glfw_create_native_cursor(struct gfx_glfw_window *window, enum gfx_native_cursor cursor);
gfx_cursor_t gfx_glfw_create_cursor(struct gfx_glfw_window *window, const void *data, uint32_t width, uint32_t height, uint32_t xhot, uint32_t yhot);
void gfx_glfw_delete_cursor(struct gfx_glfw_window *window, gfx_cursor_t cursor);
void gfx_glfw_set_cursor(struct gfx_glfw_window *window, gfx_cursor_t cursor);
void gfx_glfw_set_mouse_position(struct gfx_glfw_window *window, int32_t x, int32_t y);

#endif
