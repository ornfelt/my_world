#ifndef COMMON_H
#define COMMON_H

#include <X11/Xutil.h>
#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>

#include <stdint.h>

#define TO_RADIANS(x) (x / 180. * M_PI)
#define TO_DEGREES(x) (x * 180. / M_PI)

#define GL_CALL(fn, ...) \
do \
{ \
	fn(__VA_ARGS__); \
	GLenum err = glGetError(); \
	if (err) \
		printf(#fn "(" #__VA_ARGS__  "): %d\n", err); \
} while (0)

struct window
{
	struct gl_ctx *ctx;
	const char *progname;
	Display *display;
	Window window;
	int screen;
	uint32_t width;
	uint32_t height;
	XVisualInfo vi;
	Window root;
	GC gc;
	XImage *image;
	XShmSegmentInfo shminfo;
	void *userptr;
	void (*on_key_down)(struct window *window, KeySym sym);
	void (*on_key_up)(struct window *window, KeySym sym);
};

uint64_t nanotime(void);
int setup_window(const char *progname, struct window *window);
void handle_events(struct window *window);
void swap_buffers(struct window *window);

#endif
