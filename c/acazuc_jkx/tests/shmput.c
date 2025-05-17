#include <X11/extensions/XShm.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xlib.h>

#include <sys/shm.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

int
main(int argc, char **argv)
{
	XSetWindowAttributes swa;
	XShmSegmentInfo shminfo;
	XVisualInfo vi;
	XGCValues gc_values;
	Display *display;
	XImage *image;
	Pixmap pixmap;
	Window window;
	Window root;
	Bool xshm_pixmaps;
	GC gc;
	int xshm_major_opcode;
	int xshm_first_event;
	int xshm_first_error;
	int xshm_major;
	int xshm_minor;
	int screen;

	(void)argc;
	display = XOpenDisplay(NULL);
	if (!display)
	{
		fprintf(stderr, "%s: failed to open display\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (!XQueryExtension(display,
	                     "MIT-SHM",
	                     &xshm_major_opcode,
	                     &xshm_first_event,
	                     &xshm_first_error))
	{
		fprintf(stderr, "%s: no xshm\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (!XShmQueryVersion(display, &xshm_major, &xshm_minor, &xshm_pixmaps))
	{
		fprintf(stderr, "%s: failed to get xshm version\n", argv[0]);
		return EXIT_FAILURE;
	}
	printf("major opcode: %d, "
	       "first event: %d, "
	       "first error: %d, "
	       "major version: %d, "
	       "minor_version: %d, "
	       "shared pixmaps: %d\n",
	       xshm_major_opcode,
	       xshm_first_event,
	       xshm_first_error,
	       xshm_major,
	       xshm_minor,
	       xshm_pixmaps);
	root = XRootWindow(display, 0);
	screen = DefaultScreen(display);
	if (!XMatchVisualInfo(display, screen, 24, TrueColor, &vi))
	{
		fprintf(stderr, "%s: failed to get vi\n", argv[0]);
		return EXIT_FAILURE;
	}
	swa.event_mask = ExposureMask;
	window = XCreateWindow(display,
	                       root,
	                       0,
	                       0,
	                       640,
	                       480,
	                       0,
	                       vi.depth,
	                       InputOutput,
	                       vi.visual,
	                       CWEventMask,
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
	                (uint8_t*)"shmput",
	                6);
	pixmap = XCreatePixmap(display, window, 640, 480, 24);
	if (!pixmap)
	{
		fprintf(stderr, "%s: failed to create pixmap\n", argv[0]);
		return EXIT_FAILURE;
	}
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
	image = XShmCreateImage(display,
	                        vi.visual,
	                        24,
	                        ZPixmap,
	                        NULL,
	                        &shminfo,
	                        128,
	                        128);
	if (!image)
	{
		fprintf(stderr, "%s: failed to create image\n", argv[0]);
		return EXIT_FAILURE;
	}
	shminfo.shmid = shmget(IPC_PRIVATE,
	                       image->bytes_per_line * image->height,
	                       IPC_CREAT | 0777);
	if (shminfo.shmid == -1)
	{
		fprintf(stderr, "%s: shmget: %s\n", argv[0], strerror(errno));
		return EXIT_FAILURE;
	}
	image->data = shmat(shminfo.shmid, 0, 0);
	if (shmctl(shminfo.shmid, IPC_RMID, NULL) == -1)
	{
		fprintf(stderr, "%s: shmctl: %s\n", argv[0], strerror(errno));
		return EXIT_FAILURE;
	}
	if (image->data == (void*)-1)
	{
		fprintf(stderr, "%s: shmat: %s\n", argv[0], strerror(errno));
		return EXIT_FAILURE;
	}
	for (int y = 0; y < 128; ++y)
	{
		for (int x = 0; x < 128; ++x)
		{
			uint8_t *dst = &((uint8_t*)image->data)[4 * (x + y * 128)];
			dst[0] = 0xFF;
			dst[1] = x << 1;
			dst[2] = y << 1;
			dst[3] = 0xFF;
		}
	}
	shminfo.shmaddr = image->data;
	shminfo.readOnly = False;
	XShmAttach(display, &shminfo);
	XShmPutImage(display, pixmap, gc, image, 0, 0, 0, 0, 128, 128, False);
	XMapWindow(display, window);
	XFlush(display);
	while (1)
	{
		XEvent event;

		if (XNextEvent(display, &event))
			continue;
		if (event.type == Expose)
		{
			XCopyArea(display, pixmap, window, gc, 0, 0, 128, 128, 0, 0);
		}
	}
	return EXIT_SUCCESS;
}
