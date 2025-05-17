#include <X11/extensions/Xrender.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

int 
main(int argc, char **argv)
{
	XSetWindowAttributes swa;
	XRenderPictFormat *last = NULL;
	XRenderPictFormat *xrender_fmt;
	XVisualInfo *vi;
	XGCValues gc_values;
	Picture window_picture;
	Picture pixmap_picture;
	Display *display;
	Pixmap pixmap;
	Window window;
	Window root;
	GC gc;
	unsigned mask;
	int xrender_major_opcode;
	int xrender_first_event;
	int xrender_first_error;
	int xrender_major;
	int xrender_minor;
	int nitems;

	(void)argc;
	display = XOpenDisplay(NULL);
	if (!display)
	{
		fprintf(stderr, "%s: failed to open display\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (!XQueryExtension(display,
	                     "RENDER",
	                     &xrender_major_opcode,
	                     &xrender_first_event,
	                     &xrender_first_error))
	{
		fprintf(stderr, "%s: no xrender\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (!XRenderQueryVersion(display, &xrender_major, &xrender_minor))
	{
		fprintf(stderr, "%s: failed to get xrender version\n", argv[0]);
		return EXIT_FAILURE;
	}
	printf("major opcode: %d,"
	       "first event: %d, "
	       "first error: %d, "
	       "major version: %d, "
	       "minor_version: %d\n",
	       xrender_major_opcode,
	       xrender_first_event,
	       xrender_first_error,
	       xrender_major,
	       xrender_minor);
	printf("formats:\n");
	for (int i = 0; i < INT_MAX - 1; ++i)
	{
		XRenderPictFormat *format;

		format = XRenderFindFormat(display, 0, NULL, i);
		if (!format || format == last)
			break;
		last = format;
		printf("id: %u\n", format->id);
		printf("\ttype: %u\n", format->type);
		printf("\tdepth: %u\n", format->depth);
		printf("\tdirect: %u/%x %u/%x %u/%x %u/%x\n",
		       format->direct.red,
		       format->direct.redMask,
		       format->direct.green,
		       format->direct.greenMask,
		       format->direct.blue,
		       format->direct.blueMask,
		       format->direct.alpha,
		       format->direct.alphaMask);
		printf("\tcolormap: %u\n", format->colormap);
	}
	vi = XGetVisualInfo(display, 0, NULL, &nitems);
	if (!vi)
	{
		fprintf(stderr, "%s: failed to get vi\n", argv[0]);
		return EXIT_FAILURE;
	}
	root = XRootWindow(display, 0);
	swa.event_mask = ExposureMask;
	mask = CWEventMask;
	window = XCreateWindow(display,
	                       root,
	                       0,
	                       0,
	                       640,
	                       480,
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
	                (uint8_t*)"xrender",
	                7);
	xrender_fmt = XRenderFindStandardFormat(display, PictStandardRGB24);
	if (!xrender_fmt)
	{
		fprintf(stderr, "%s: failed to find matching format\n", argv[0]);
		return EXIT_FAILURE;
	}
	window_picture = XRenderCreatePicture(display,
	                                      window,
	                                      xrender_fmt,
	                                      0,
	                                      NULL);
	if (!window_picture)
	{
		fprintf(stderr, "%s: failed to create window picture\n", argv[0]);
		return EXIT_FAILURE;
	}
	pixmap = XCreatePixmap(display, root, 640, 480, 24);
	if (!pixmap)
	{
		fprintf(stderr, "%s: failed to create pixmap\n", argv[0]);
		return EXIT_FAILURE;
	}
	pixmap_picture = XRenderCreatePicture(display,
	                                      pixmap,
	                                      xrender_fmt,
	                                      0,
	                                      NULL);
	if (!pixmap_picture)
	{
		fprintf(stderr, "%s: failed to create pixmap picture\n", argv[0]);
		return EXIT_FAILURE;
	}
	gc_values.foreground = 0xFF104010;
	gc = XCreateGC(display, window, GCForeground, &gc_values);
	if (!gc)
	{
		fprintf(stderr, "%s: failed to create GC\n", argv[0]);
		return EXIT_FAILURE;
	}
	XMapWindow(display, window);
	while (1)
	{
		XRenderColor color;
		XEvent event;

		if (XNextEvent(display, &event))
			continue;
		if (event.type != Expose)
			continue;
		XFillRectangle(display, pixmap, gc, 0, 0, 640, 480);
		color.red = 0x8000;
		color.green = 0x0000;
		color.blue = 0x0000;
		color.alpha = 0x8000;
		XRenderFillRectangle(display,
		                     PictOpOver,
		                     pixmap_picture,
		                     &color,
		                     0,
		                     0,
		                     100,
		                     100);
		color.red = 0x0000;
		color.green = 0x0000;
		color.blue = 0x8000;
		color.alpha = 0x8000;
		XRenderFillRectangle(display,
		                     PictOpOver,
		                     pixmap_picture,
		                     &color,
		                     50,
		                     50,
		                     100,
		                     100);
		XRenderComposite(display,
		                 PictOpSrc,
		                 pixmap_picture,
		                 None,
		                 window_picture,
		                 0,
		                 0,
		                 0,
		                 0,
		                 0,
		                 0,
		                 640,
		                 480);
	}
	return EXIT_SUCCESS;
}
