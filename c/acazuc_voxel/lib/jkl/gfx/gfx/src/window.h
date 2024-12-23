#ifndef GFX_WINDOW_H
#define GFX_WINDOW_H

#ifdef __cplusplus
extern "C" {
#endif

#include "events.h"
#include <stdint.h>
#include <stddef.h>

struct gfx_device;
struct gfx_window;

enum gfx_device_backend
{
	GFX_DEVICE_GL3,
	GFX_DEVICE_GL4,
	GFX_DEVICE_D3D9,
	GFX_DEVICE_D3D11,
	GFX_DEVICE_VK,
	GFX_DEVICE_GLES3,
};

enum gfx_window_backend
{
	GFX_WINDOW_X11,
	GFX_WINDOW_WIN32,
	GFX_WINDOW_WAYLAND,
	GFX_WINDOW_GLFW,
	GFX_WINDOW_SDL,
};

enum gfx_native_cursor
{
	GFX_CURSOR_ARROW,
	GFX_CURSOR_CROSS,
	GFX_CURSOR_HAND,
	GFX_CURSOR_IBEAM,
	GFX_CURSOR_NO,
	GFX_CURSOR_SIZEALL,
	GFX_CURSOR_VRESIZE,
	GFX_CURSOR_HRESIZE,
	GFX_CURSOR_WAIT,
	GFX_CURSOR_BLANK,
	GFX_CURSOR_LAST
};

typedef void *gfx_cursor_t;

typedef void (*gfx_key_down_callback_t)(struct gfx_window *window, struct gfx_key_event *event);
typedef void (*gfx_key_press_callback_t)(struct gfx_window *window, struct gfx_key_event *event);
typedef void (*gfx_key_up_callback_t)(struct gfx_window *window, struct gfx_key_event *event);
typedef void (*gfx_char_callback_t)(struct gfx_window *window, struct gfx_char_event *event);
typedef void (*gfx_mouse_down_callback_t)(struct gfx_window *window, struct gfx_mouse_event *event);
typedef void (*gfx_mouse_up_callback_t)(struct gfx_window *window, struct gfx_mouse_event *event);
typedef void (*gfx_scroll_callback_t)(struct gfx_window *window, struct gfx_scroll_event *event);
typedef void (*gfx_mouse_move_callback_t)(struct gfx_window *window, struct gfx_pointer_event *event);
typedef void (*gfx_cursor_enter_callback_t)(struct gfx_window *window);
typedef void (*gfx_cursor_leave_callback_t)(struct gfx_window *window);
typedef void (*gfx_focus_in_callback_t)(struct gfx_window *window);
typedef void (*gfx_focus_out_callback_t)(struct gfx_window *window);
typedef void (*gfx_resize_callback_t)(struct gfx_window *window, struct gfx_resize_event *event);
typedef void (*gfx_move_callback_t)(struct gfx_window *window, struct gfx_move_event *event);
typedef void (*gfx_expose_callback_t)(struct gfx_window *window);
typedef void (*gfx_close_callback_t)(struct gfx_window *window);
typedef void (*gfx_error_callback_t)(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

struct gfx_memory
{
	void *(*malloc)(size_t size);
	void *(*realloc)(void *ptr, size_t size);
	void (*free)(void *ptr);
};

#define GFX_MALLOC(size) (gfx_memory.malloc ? gfx_memory.malloc(size) : malloc(size))
#define GFX_REALLOC(ptr, size) (gfx_memory.realloc ? gfx_memory.realloc(ptr, size) : realloc(ptr, size))
#define GFX_FREE(ptr) (gfx_memory.free ? gfx_memory.free(ptr) : free(ptr))

#define GFX_ERROR_CALLBACK(...) \
do \
{ \
	if (gfx_error_callback) \
		gfx_error_callback(__VA_ARGS__); \
} while (0)

extern gfx_error_callback_t gfx_error_callback;
extern struct gfx_memory gfx_memory;

struct gfx_window_properties
{
	enum gfx_window_backend window_backend;
	enum gfx_device_backend device_backend;
	uint8_t stencil_bits;
	uint8_t depth_bits;
	uint8_t red_bits;
	uint8_t green_bits;
	uint8_t blue_bits;
	uint8_t alpha_bits;
};

struct gfx_window_vtable;

struct gfx_window
{
	const struct gfx_window_vtable *vtable;
	gfx_key_down_callback_t key_down_callback;
	gfx_key_press_callback_t key_press_callback;
	gfx_key_up_callback_t key_up_callback;
	gfx_char_callback_t char_callback;
	gfx_mouse_down_callback_t mouse_down_callback;
	gfx_mouse_up_callback_t mouse_up_callback;
	gfx_scroll_callback_t scroll_callback;
	gfx_mouse_move_callback_t mouse_move_callback;
	gfx_cursor_enter_callback_t cursor_enter_callback;
	gfx_cursor_leave_callback_t cursor_leave_callback;
	gfx_focus_in_callback_t focus_in_callback;
	gfx_focus_out_callback_t focus_out_callback;
	gfx_resize_callback_t resize_callback;
	gfx_move_callback_t move_callback;
	gfx_expose_callback_t expose_callback;
	gfx_close_callback_t close_callback;
	struct gfx_window_properties properties;
	struct gfx_device *device;
	uint8_t keys[(GFX_KEY_LAST + 7) / 8];
	uint8_t mouse_buttons;
	double virtual_x;
	double virtual_y;
	int32_t mouse_x;
	int32_t mouse_y;
	int32_t height;
	int32_t width;
	int32_t x;
	int32_t y;
	bool close_requested;
	bool grabbed;
	void *userdata;
};

bool gfx_has_window_backend(enum gfx_window_backend backend);
bool gfx_has_device_backend(enum gfx_device_backend backend);

struct gfx_window *gfx_create_window(const char *title, uint32_t width, uint32_t height, struct gfx_window_properties *properties);
void gfx_delete_window(struct gfx_window *window);
bool gfx_create_device(struct gfx_window *window);
void gfx_window_show(struct gfx_window *window);
void gfx_window_hide(struct gfx_window *window);
void gfx_window_poll_events(struct gfx_window *window);
void gfx_window_wait_events(struct gfx_window *window);
void gfx_window_grab_cursor(struct gfx_window *window);
void gfx_window_ungrab_cursor(struct gfx_window *window);
void gfx_window_swap_buffers(struct gfx_window *window);
void gfx_window_make_current(struct gfx_window *window);
void gfx_window_set_swap_interval(struct gfx_window *window, int interval);
void gfx_window_set_title(struct gfx_window *window, const char *title);
void gfx_window_set_icon(struct gfx_window *window, const void *data, uint32_t width, uint32_t height);
void gfx_window_resize(struct gfx_window *window, uint32_t width, uint32_t height);
char *gfx_window_get_clipboard(struct gfx_window *window);
void gfx_window_set_clipboard(struct gfx_window *window, const char *clipboard);
gfx_cursor_t gfx_create_native_cursor(struct gfx_window *window, enum gfx_native_cursor cursor);
gfx_cursor_t gfx_create_cursor(struct gfx_window *window, const void *data, uint32_t width, uint32_t height, uint32_t xhot, uint32_t yhot);
void gfx_delete_cursor(struct gfx_window *window, gfx_cursor_t cursor);
void gfx_set_cursor(struct gfx_window *window, gfx_cursor_t cursor);
void gfx_set_mouse_position(struct gfx_window *window, int32_t x, int32_t y);
int32_t gfx_get_mouse_x(struct gfx_window *window);
int32_t gfx_get_mouse_y(struct gfx_window *window);
bool gfx_is_key_down(struct gfx_window *window, enum gfx_key_code key);
bool gfx_is_mouse_button_down(struct gfx_window *window, enum gfx_mouse_button mouse_button);

void gfx_window_properties_init(struct gfx_window_properties *properties);

#ifdef __cplusplus
}
#endif

#endif
