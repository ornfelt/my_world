#ifndef GFX_SDL_WINDOW_H
#define GFX_SDL_WINDOW_H

#include "window.h"

#include <SDL2/SDL.h>

struct gfx_sdl_window
{
	struct gfx_window *winref;
	SDL_Window *window;
};

void *gfx_sdl_get_proc_address(const char *name);
bool gfx_sdl_create_window(struct gfx_sdl_window *window, const char *title, uint32_t width, uint32_t height, struct gfx_window_properties *properties);
bool gfx_sdl_ctr(struct gfx_sdl_window *window, struct gfx_window *winref);
void gfx_sdl_dtr(struct gfx_sdl_window *window);
void gfx_sdl_show(struct gfx_sdl_window *window);
void gfx_sdl_hide(struct gfx_sdl_window *window);
void gfx_sdl_poll_events(struct gfx_sdl_window *window);
void gfx_sdl_wait_events(struct gfx_sdl_window *window);
void gfx_sdl_set_title(struct gfx_sdl_window *window, const char *title);
void gfx_sdl_set_icon(struct gfx_sdl_window *window, const void *data, uint32_t width, uint32_t height);
void gfx_sdl_resize(struct gfx_sdl_window *window, uint32_t width, uint32_t height);
void gfx_sdl_grab_cursor(struct gfx_sdl_window *window);
void gfx_sdl_ungrab_cursor(struct gfx_sdl_window *window);
char *gfx_sdl_get_clipboard(struct gfx_sdl_window *window);
void gfx_sdl_set_clipboard(struct gfx_sdl_window *window, const char *text);
gfx_cursor_t gfx_sdl_create_native_cursor(struct gfx_sdl_window *window, enum gfx_native_cursor cursor);
gfx_cursor_t gfx_sdl_create_cursor(struct gfx_sdl_window *window, const void *data, uint32_t width, uint32_t height, uint32_t xhot, uint32_t yhot);
void gfx_sdl_delete_cursor(struct gfx_sdl_window *window, gfx_cursor_t cursor);
void gfx_sdl_set_cursor(struct gfx_sdl_window *window, gfx_cursor_t cursor);
void gfx_sdl_set_mouse_position(struct gfx_sdl_window *window, int32_t x, int32_t y);

#endif
