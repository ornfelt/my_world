#include <sys/queue.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>

#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xlib.h>

struct button
{
	uint32_t width;
	uint32_t height;
	int32_t x;
	int32_t y;
	Display *display;
	Window window;
	char *text;
	TAILQ_ENTRY(button) chain;
};

struct texture
{
	Pixmap pixmap;
	uint16_t width;
	uint16_t height;
};

struct window
{
	uint32_t width;
	uint32_t height;
	int32_t x;
	int32_t y;
	Display *display;
	Window window;
	Font font;
	XVisualInfo vi;
	GC gc;
	GC bg;
	TAILQ_HEAD(, button) buttons;
};

struct window g_window;
struct texture *g_texture;

static void button_draw(struct window *window, struct button *button)
{
	XDrawRectangle(button->display, button->window, window->gc, 0, 0,
	               button->width - 1, button->height - 1);
	/*int direction;
	int ascent;
	int descent;
	XCharStruct overall;
	XQueryTextExtents(button->display, window->font, button->text, strlen(button->text), &direction, &ascent, &descent, &overall);*/
	XDrawString(button->display, button->window, window->gc, 5, 15,
	            button->text, strlen(button->text));
}

static struct button *create_button(const char *progname, struct window *window,
                                    int32_t x, int32_t y,
                                    uint32_t width, uint32_t height,
                                    const char *text)
{
	struct button *button = malloc(sizeof(*button));
	if (!button)
		return NULL;
	button->display = window->display;
	button->text = strdup(text);
	button->width = width;
	button->height = height;
	button->y = x;
	button->y = y;
	XSetWindowAttributes swa;
	swa.event_mask = ExposureMask;
	unsigned mask = CWEventMask;
	button->window = XCreateWindow(button->display, window->window, x, y,
	                               width, height, 0, window->vi.depth,
	                               InputOutput, window->vi.visual, mask, &swa);
	if (!button->window)
	{
		fprintf(stderr, "%s: failed to create button window\n", progname);
		free(button->text);
		free(button);
		return NULL;
	}
	XMapWindow(button->display, button->window);
	return button;
}

static struct texture *create_texture(const char *progname,
                                      struct window *window,
                                      const uint8_t *data,
                                      size_t width, size_t height)
{
	struct texture *texture = malloc(sizeof(*texture));
	if (!texture)
	{
		fprintf(stderr, "%s: malloc: %s\n", progname, strerror(errno));
		return NULL;
	}
	texture->width = width;
	texture->height = height;
	texture->pixmap = XCreatePixmap(window->display, window->window, width,
	                                height, 24);
	if (!texture->pixmap)
	{
		fprintf(stderr, "%s: failed to create pixmap\n", progname);
		free(texture);
		return NULL;
	}
	XImage *image = XCreateImage(window->display, window->vi.visual, 24,
	                             ZPixmap, 0, (char*)data, width, height, 32,
	                             width * 4);
	XPutImage(window->display, texture->pixmap, window->gc, image, 0, 0, 0, 0,
	          width, height);
	image->data = NULL;
	XDestroyImage(image);
	return texture;
}

static void draw(struct window *window, int x, int y)
{
	XFillRectangle(window->display, window->window, window->bg, 0, 0,
	               window->width, window->height);
	XDrawPoint(window->display, window->window, window->gc, x + 24, y + 24);
	XDrawLine(window->display, window->window, window->gc, x + 16, y + 16,
	          x + 32, y + 16);
	XDrawLine(window->display, window->window, window->gc, x + 32, y + 16,
	          x + 32, y + 32);
	XDrawLine(window->display, window->window, window->gc, x + 32, y + 32,
	          x + 16, y + 32);
	XDrawLine(window->display, window->window, window->gc, x + 16, y + 32,
	          x + 16, y + 16);
	XSegment segments[] =
	{
		{x + 24, y + 32, x + 32, y + 24},
		{x + 16, y + 24, x + 24, y + 32},
		{x + 24, y + 16, x + 16, y + 24},
		{x + 24, y + 16, x + 32, y + 24},
	};
	XDrawSegments(window->display, window->window, window->gc, segments, 4);
	XDrawRectangle(window->display, window->window, window->gc, x + 12, y + 12,
	               24, 24);
	XDrawArc(window->display, window->window, window->gc, x + 8, y + 8, 32, 32,
	         0, 360 * 64);
	XPoint poly_points[] =
	{
		{x + 100, y + 30},
		{x + 100, y + 60},
		{x + 75, y + 40},
		{x + 50, y + 60},
		{x + 50, y + 30},
	};
	XFillPolygon(window->display, window->window, window->gc, poly_points, 5,
	             Nonconvex, CoordModeOrigin);
	XFillRectangle(window->display, window->window, window->gc, x + 120, y + 30,
	               20, 30);
	XFillArc(window->display, window->window, window->gc, x + 160, y + 30, 30,
	         30, 0, 360 * 64);
	XCopyArea(window->display, g_texture->pixmap, window->window, window->gc,
	          0, 0, g_texture->width, g_texture->height, x + 200, y + 100);
	struct button *button;
	TAILQ_FOREACH(button, &window->buttons, chain)
		button_draw(window, button);
	XFlush(window->display);
}

int main(int argc, char **argv)
{
	(void)argc;
	TAILQ_INIT(&g_window.buttons);
	g_window.display = XOpenDisplay(NULL);
	if (!g_window.display)
	{
		fprintf(stderr, "%s: failed to open display\n", argv[0]);
		return EXIT_FAILURE;
	}
	Window root = DefaultRootWindow(g_window.display);
	int screen = DefaultScreen(g_window.display);
	if (!XMatchVisualInfo(g_window.display, screen, 24,
	                      TrueColor, &g_window.vi))
	{
		fprintf(stderr, "%s: failed to get vi\n", argv[0]);
		return EXIT_FAILURE;
	}
	XSetWindowAttributes swa;
	swa.event_mask = ExposureMask
	               | KeyPressMask
	               | KeyReleaseMask
	               | StructureNotifyMask;
	unsigned mask = CWEventMask;
	g_window.window = XCreateWindow(g_window.display, root, 0, 0, 640, 480, 0,
	                                g_window.vi.depth, InputOutput,
	                                g_window.vi.visual, mask, &swa);
	if (!g_window.window)
	{
		fprintf(stderr, "%s: failed to create window\n", argv[0]);
		return EXIT_FAILURE;
	}
	XChangeProperty(g_window.display, g_window.window, XA_WM_NAME,
	                XA_STRING, 8, PropModeReplace, (uint8_t*)"draw", 4);
	XResizeWindow(g_window.display, g_window.window, 840, 680);
	g_window.width = 840;
	g_window.height = 680;
	g_window.font = XLoadFont(g_window.display, "8x16");
	if (!g_window.font)
	{
		fprintf(stderr, "%s: failed to open font\n", argv[0]);
		return EXIT_FAILURE;
	}
	XGCValues gc_values;
	gc_values.foreground = 0xFFFFFFFF;
	gc_values.graphics_exposures = 0;
	gc_values.font = g_window.font;
	g_window.gc = XCreateGC(g_window.display, g_window.window,
	                        GCForeground | GCGraphicsExposures | GCFont,
	                        &gc_values);
	if (!g_window.gc)
	{
		fprintf(stderr, "%s: failed to create GC\n", argv[0]);
		return EXIT_FAILURE;
	}
	gc_values.foreground = 0;
	gc_values.graphics_exposures = 0;
	g_window.bg = XCreateGC(g_window.display, g_window.window,
	                        GCForeground | GCGraphicsExposures, &gc_values);
	if (!g_window.bg)
	{
		fprintf(stderr, "%s: failed to create GC\n", argv[0]);
		return EXIT_FAILURE;
	}
	int draw_x = 0;
	int draw_y = 0;
	struct button *file_button = create_button(argv[0], &g_window, 20, 120,
	                                           50, 20, "file");
	if (!file_button)
		return EXIT_FAILURE;
	TAILQ_INSERT_TAIL(&g_window.buttons, file_button, chain);
	Cursor hand_cursor = XCreateFontCursor(g_window.display, XC_hand1);
	if (!hand_cursor)
	{
		fprintf(stderr, "%s: failed to create hand cursor\n", argv[0]);
		return EXIT_FAILURE;
	}
	XDefineCursor(file_button->display, file_button->window, hand_cursor);
	XColor red;
	red.pixel = 0xFFFF0000;
	red.red = 0xFFFF;
	red.green = 0;
	red.blue = 0;
	red.flags = 0;
	red.pad = 0;
	XColor green;
	green.pixel = 0xFF00FF00;
	green.red = 0;
	green.green = 0xFFFF;
	green.blue = 0;
	green.flags = 0;
	green.pad = 0;
	XRecolorCursor(file_button->display, hand_cursor, &red, &green);
	uint8_t data[64 * 64 * 4];
	for (size_t y = 0; y < 64; ++y)
	{
		for (size_t x = 0; x < 64; ++x)
		{
			uint8_t *dst = &data[4 * (x + y * 64)];
			dst[0] = 0xFF;
			dst[1] = x << 2;
			dst[2] = y << 2;
			dst[3] = 0xFF;
		}
	}
	g_texture = create_texture(argv[0], &g_window, data, 64, 64);
	draw(&g_window, draw_x, draw_y);
	XMapWindow(g_window.display, g_window.window);
	XSynchronize(g_window.display, False);
	while (1)
	{
		XEvent event;
		if (XNextEvent(g_window.display, &event))
			continue;
		switch (event.type)
		{
			case Expose:
			{
				if (event.xexpose.window == g_window.window)
					draw(&g_window, draw_x, draw_y);
				break;
			}
			case KeyPress:
			{
				KeySym sym = XLookupKeysym(&event.xkey, 0);
				switch (sym)
				{
					case XK_Left:
						draw_x--;
						draw(&g_window, draw_x, draw_y);
						break;
					case XK_Right:
						draw_x++;
						draw(&g_window, draw_x, draw_y);
						break;
					case XK_Up:
						draw_y--;
						draw(&g_window, draw_x, draw_y);
						break;
					case XK_Down:
						draw_y++;
						draw(&g_window, draw_x, draw_y);
						break;
				}
				break;
			}
			case ConfigureNotify:
				if (event.xany.window == g_window.window)
				{
					g_window.width = event.xconfigure.width;
					g_window.height = event.xconfigure.height;
				}
				break;
		}
		XFlush(g_window.display);
	}
	return EXIT_SUCCESS;
}
