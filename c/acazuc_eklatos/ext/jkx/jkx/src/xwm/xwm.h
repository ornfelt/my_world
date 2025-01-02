#ifndef XWM_H
#define XWM_H

#include <X11/Xutil.h>
#include <X11/Xlib.h>

#include <sys/queue.h>

enum button_state
{
	BUTTON_NORMAL   = 0,
	BUTTON_HOVERED  = (1 << 0),
	BUTTON_CLICKED  = (1 << 1),
	BUTTON_DISABLED = (1 << 2),
};

enum resize_x_type
{
	RESIZE_X_NONE,
	RESIZE_X_LEFT,
	RESIZE_X_RIGHT,
};

enum resize_y_type
{
	RESIZE_Y_NONE,
	RESIZE_Y_TOP,
	RESIZE_Y_BOTTOM,
};

struct button
{
	Window window;
	enum button_state state;
};

enum window_flags
{
	WINDOW_MAPPED    = (1 << 0),
	WINDOW_MAXIMIZED = (1 << 1),
};

struct window
{
	Window parent;
	Window header;
	Window child;
	struct button resize_tl;
	struct button resize_tr;
	struct button resize_bl;
	struct button resize_br;
	struct button resize_l;
	struct button resize_r;
	struct button resize_t;
	struct button resize_b;
	struct button maximize;
	struct button iconify;
	struct button close;
	struct button taskbar_button;
	enum window_flags flags;
	int32_t x;
	int32_t y;
	uint32_t width;
	uint32_t height;
	uint32_t border_width;
	int32_t prev_x;
	int32_t prev_y;
	uint32_t prev_width;
	uint32_t prev_height;
	char *name;
	uint32_t name_len;
	char *icon_name;
	uint32_t icon_name_len;
	TAILQ_ENTRY(window) chain;
};

struct asset
{
	Pixmap pixmap;
	uint32_t width;
	uint32_t height;
};

struct assets
{
	struct asset background;
	struct asset header;
	struct asset header_resize_l;
	struct asset header_resize_r;
	struct asset iconify;
	struct asset iconify_hovered;
	struct asset iconify_clicked;
	struct asset maximize;
	struct asset maximize_hovered;
	struct asset maximize_clicked;
	struct asset close;
	struct asset close_hovered;
	struct asset close_clicked;
	struct asset taskbar;
	struct asset taskbar_l;
	struct asset taskbar_r;
	struct asset taskbar_button;
	struct asset taskbar_button_l;
	struct asset taskbar_button_r;
	struct asset taskbar_button_hovered;
	struct asset taskbar_button_disabled;
	struct asset resize_tl;
	struct asset resize_tr;
	struct asset resize_bl;
	struct asset resize_br;
	struct asset resize_l;
	struct asset resize_r;
	struct asset resize_t;
	struct asset resize_b;
};

struct cursors
{
	Cursor left_ptr;
	Cursor hand1;
	Cursor bottom_left_corner;
	Cursor bottom_right_corner;
	Cursor top_left_corner;
	Cursor top_right_corner;
	Cursor left_side;
	Cursor right_side;
	Cursor top_side;
	Cursor bottom_side;
	Cursor fleur;
};

struct xwm
{
	const char *progname;
	const char *assets_path;
	Display *display;
	int screen;
	uint32_t screen_width;
	uint32_t screen_height;
	XVisualInfo vi;
	Window root;
	Window taskbar;
	Window container;
	Font header_font;
	Font taskbar_font;
	GC header_gc;
	GC bitmap_gc;
	GC header_text_gc;
	GC taskbar_text_gc;
	GC resize_gc;
	struct window *drag;
	struct assets assets;
	struct cursors cursors;
	int drag_x;
	int drag_y;
	int enable_reframe;
	TAILQ_HEAD(, window) windows;
};

int setup_background(struct xwm *xwm);
int setup_assets(struct xwm *xwm);

#endif
