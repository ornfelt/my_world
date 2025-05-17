#include <X11/cursorfont.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>

#include <inttypes.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

struct button
{
	Window window;
	Cursor cursor;
	int32_t x;
	int32_t y;
	uint32_t width;
	uint32_t height;
};

static Display *display;
static Window window;
static XVisualInfo *vi;
static GC gc;
static struct button buttons[78];

static int
create_button(const char *progname,
              struct button *button,
              int32_t x,
              int32_t y,
              uint32_t width,
              uint32_t height,
              uint32_t cid)
{
	XSetWindowAttributes swa;
	char name[64];

	button->x = x;
	button->y = y;
	button->width = width;
	button->height = height;
	swa.event_mask = ExposureMask;
	swa.border_pixel = 0xFF0000;
	button->window = XCreateWindow(display,
	                               window,
	                               x,
	                               y,
	                               width,
	                               height, 2,
	                               vi->depth,
	                               InputOutput,
	                               vi->visual,
	                               CWEventMask | CWBorderPixel,
	                               &swa);
	if (!button->window)
	{
		fprintf(stderr, "%s: failed to create button window\n",
		        progname);
		return 1;
	}
	snprintf(name, sizeof(name), "xcursor button %" PRIu32, cid / 2);
	XChangeProperty(display,
	                button->window,
	                XA_WM_NAME,
	                XA_STRING,
	                8,
	                PropModeReplace,
	                (uint8_t*)name,
	                strlen(name));
	if (cid < 77 * 2)
	{
		button->cursor = XCreateFontCursor(display, cid);
		if (!button->cursor)
		{
			fprintf(stderr, "%s: failed to create button font cursor\n",
			        progname);
			return 1;
		}
	}
	else
	{
		uint8_t colors[2 * 32] =
		{
			0x00, 0x00, 0x00, 0x00,
			0x80, 0x01, 0x00, 0x00,
			0x80, 0x01, 0x00, 0x00,
			0x80, 0x01, 0x00, 0x00,
			0x80, 0x01, 0x00, 0x00,
			0x80, 0x01, 0x00, 0x00,
			0x80, 0x01, 0x00, 0x00,
			0xFE, 0x7F, 0x00, 0x00,
			0xFE, 0x7F, 0x00, 0x00,
			0x80, 0x01, 0x00, 0x00,
			0x80, 0x01, 0x00, 0x00,
			0x80, 0x01, 0x00, 0x00,
			0x80, 0x01, 0x00, 0x00,
			0x80, 0x01, 0x00, 0x00,
			0x80, 0x01, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00,
		};
		uint8_t masks[2 * 32] =
		{
			0x80, 0x01, 0x00, 0x00,
			0xC0, 0x03, 0x00, 0x00,
			0xC0, 0x03, 0x00, 0x00,
			0xC0, 0x03, 0x00, 0x00,
			0xC0, 0x03, 0x00, 0x00,
			0xC0, 0x03, 0x00, 0x00,
			0xFE, 0x7F, 0x00, 0x00,
			0xFF, 0xFF, 0x00, 0x00,
			0xFF, 0xFF, 0x00, 0x00,
			0xFE, 0x7F, 0x00, 0x00,
			0xC0, 0x03, 0x00, 0x00,
			0xC0, 0x03, 0x00, 0x00,
			0xC0, 0x03, 0x00, 0x00,
			0xC0, 0x03, 0x00, 0x00,
			0xC0, 0x03, 0x00, 0x00,
			0x80, 0x01, 0x00, 0x00,
		};
		Pixmap source = XCreateBitmapFromData(display, button->window,
		                                      (char*)colors, 32, 16);
		if (!source)
		{
			fprintf(stderr, "%s: failed to create cursor source pixmap\n",
			        progname);
			return 1;
		}
		Pixmap mask = XCreateBitmapFromData(display, button->window,
		                                    (char*)masks, 32, 16);
		if (!mask)
		{
			fprintf(stderr, "%s: failed to create cursor mask pixmap\n",
			        progname);
			return 1;
		}
		XColor background;
		background.pixel = 0xFFFFFFFF;
		background.red = 0xFFFF;
		background.green = 0xFFFF;
		background.blue = 0xFFFF;
		background.flags = 0;
		background.pad = 0;
		XColor foreground;
		foreground.pixel = 0;
		foreground.red = 0;
		foreground.green = 0;
		foreground.blue = 0;
		foreground.flags = 0;
		foreground.pad = 0;
		button->cursor = XCreatePixmapCursor(display, source, mask,
		                                     &foreground, &background,
		                                     8, 8);
		if (!button->cursor)
		{
			fprintf(stderr, "%s: failed to create pixmap cursor\n",
			        progname);
			return 1;
		}
	}
	XDefineCursor(display, button->window, button->cursor);
	return 0;
}

int
main(int argc, char **argv)
{
	XSetWindowAttributes swa;
	XSizeHints hints;
	XGCValues gc_values;
	Window root;
	unsigned mask;
	int nitems;
	long vimask = 0;

	(void)argc;
	display = XOpenDisplay(NULL);
	if (!display)
	{
		fprintf(stderr, "%s: failed to open display\n", argv[0]);
		return EXIT_FAILURE;
	}
	vi = XGetVisualInfo(display, vimask, NULL, &nitems);
	root = XRootWindow(display, 0);
	swa.event_mask = ExposureMask;
	mask = CWEventMask;
	window = XCreateWindow(display,
	                       root,
	                       0,
	                       0,
	                       320,
	                       320,
	                       0,
	                       vi->depth,
	                       InputOutput,
	                       vi->visual,
	                       mask,
	                       &swa);
	if (!window)
	{
		fprintf(stderr, "%s: failed to create window\n", argv[0]);
		return EXIT_FAILURE;
	}
	XChangeProperty(display,
	                window,
	                XA_WM_NAME,
	                XA_STRING,
	                8,
	                PropModeReplace,
	                (uint8_t*)"xcursor",
	                7);
	gc_values.foreground = 0xFFFFFFFF;
	gc_values.graphics_exposures = 0;
	gc = XCreateGC(display,
	               window,
	               GCForeground | GCGraphicsExposures,
	               &gc_values);
	if (!gc)
	{
		fprintf(stderr, "%s: failed to create GC\n", argv[0]);
		return EXIT_FAILURE;
	}
	for (size_t i = 0; i < 78; ++i)
	{
		uint32_t x = i % 9;
		uint32_t y = i / 9;
		if (create_button(argv[0], &buttons[i], 3 + 35 * x, 3 + 35 * y,
		                  30, 30, i * 2))
			return EXIT_FAILURE;
	}
	hints.flags = PMinSize | PMaxSize;
	hints.min_width = 320;
	hints.max_width = 320;
	hints.min_height = 320;
	hints.max_height = 320;
	XSetWMNormalHints(display, window, &hints);
	XMapSubwindows(display, window);
	XMapWindow(display, window);
	while (1)
	{
		XEvent event;
		XNextEvent(display, &event);
	}
	return EXIT_SUCCESS;
}
