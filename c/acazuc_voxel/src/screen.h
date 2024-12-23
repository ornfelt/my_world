#ifndef SCREEN_H
#define SCREEN_H

struct gfx_scroll_event;
struct gfx_mouse_event;
struct gfx_char_event;
struct gfx_key_event;
struct screen;

struct screen_op
{
	void (*destroy)(struct screen *screen);
	void (*tick)(struct screen *screen);
	void (*draw)(struct screen *screen);
	void (*mouse_move)(struct screen *screen, struct gfx_pointer_event *event);
	void (*mouse_down)(struct screen *screen, struct gfx_mouse_event *event);
	void (*mouse_up)(struct screen *screen, struct gfx_mouse_event *event);
	void (*mouse_scroll)(struct screen *screen, struct gfx_scroll_event *event);
	void (*key_down)(struct screen *screen, struct gfx_key_event *event);
	void (*key_press)(struct screen *screen, struct gfx_key_event *event);
	void (*key_up)(struct screen *screen, struct gfx_key_event *event);
	void (*char_entered)(struct screen *screen, struct gfx_char_event *event);
	void (*lang_changed)(struct screen *screen);
	void (*window_resized)(struct screen *screen);
	void (*focus_in)(struct screen *screen);
	void (*focus_out)(struct screen *screen);
};

struct screen
{
	const struct screen_op *op;
};

#define SCREEN_CALL(screen, fn, ...) \
do \
{ \
	if ((screen)->op->fn) \
		(screen)->op->fn(screen, ##__VA_ARGS__); \
} while (0)

#endif
