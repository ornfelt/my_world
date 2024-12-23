#ifndef GFX_WINDOW_VTABLE_H
#define GFX_WINDOW_VTABLE_H

struct gfx_window_vtable
{
	bool (*ctr)(struct gfx_window *window, struct gfx_window_properties *properties);
	void (*dtr)(struct gfx_window *window);
	bool (*create_device)(struct gfx_window *window);
	void (*show)(struct gfx_window *window);
	void (*hide)(struct gfx_window *window);
	void (*poll_events)(struct gfx_window *window);
	void (*wait_events)(struct gfx_window *window);
	void (*grab_cursor)(struct gfx_window *window);
	void (*ungrab_cursor)(struct gfx_window *window);
	void (*swap_buffers)(struct gfx_window *window);
	void (*make_current)(struct gfx_window *window);
	void (*set_swap_interval)(struct gfx_window *window, int interval);
	void (*set_title)(struct gfx_window *window, const char *title);
	void (*set_icon)(struct gfx_window *window, const void *data, uint32_t width, uint32_t height);
	void (*resize)(struct gfx_window *window, uint32_t width, uint32_t height);
	char *(*get_clipboard)(struct gfx_window *window);
	void (*set_clipboard)(struct gfx_window *window, const char *clipboard);
	gfx_cursor_t (*create_native_cursor)(struct gfx_window *window, enum gfx_native_cursor cursor);
	gfx_cursor_t (*create_cursor)(struct gfx_window *window, const void *data, uint32_t width, uint32_t height, uint32_t xhot, uint32_t yhot);
	void (*delete_cursor)(struct gfx_window *window, gfx_cursor_t cursor);
	void (*set_cursor)(struct gfx_window *window, gfx_cursor_t cursor);
	void (*set_mouse_position)(struct gfx_window *window, int32_t x, int32_t y);
};

extern const struct gfx_window_vtable gfx_window_vtable;

#define GFX_WINDOW_VTABLE_DEF(prefix) \
	.ctr                  = prefix##_ctr, \
	.dtr                  = prefix##_dtr, \
	.create_device        = prefix##_create_device, \
	.show                 = prefix##_show, \
	.hide                 = prefix##_hide, \
	.poll_events          = prefix##_poll_events, \
	.wait_events          = prefix##_wait_events, \
	.grab_cursor          = prefix##_grab_cursor, \
	.ungrab_cursor        = prefix##_ungrab_cursor, \
	.swap_buffers         = prefix##_swap_buffers, \
	.make_current         = prefix##_make_current, \
	.set_swap_interval    = prefix##_set_swap_interval, \
	.set_title            = prefix##_set_title, \
	.set_icon             = prefix##_set_icon, \
	.resize               = prefix##_resize, \
	.get_clipboard        = prefix##_get_clipboard, \
	.set_clipboard        = prefix##_set_clipboard, \
	.create_native_cursor = prefix##_create_native_cursor, \
	.create_cursor        = prefix##_create_cursor, \
	.delete_cursor        = prefix##_delete_cursor, \
	.set_cursor           = prefix##_set_cursor, \
	.set_mouse_position   = prefix##_set_mouse_position,

#endif
