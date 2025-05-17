#include <X11/extensions/XShm.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xlib.h>

#include <sys/shm.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

int
main(int argc, char **argv)
{
	XSetWindowAttributes swa;
	XShmSegmentInfo shminfo;
	XVisualInfo vi;
	XGCValues gc_values;
	Display *display;
	XImage *image;
	Window src_win = None;
	Window window;
	Window root;
	Bool xshm_pixmaps;
	GC gc;
	unsigned mask;
	int xshm_major_opcode;
	int xshm_first_event;
	int xshm_first_error;
	int xshm_major;
	int xshm_minor;
	int screen;

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
	if (!XShmQueryVersion(display,
	                      &xshm_major,
	                      &xshm_minor,
	                      &xshm_pixmaps))
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
	if (argc >= 2)
		src_win = strtol(argv[1], NULL, 0);
	if (!src_win)
	{
		int revert_to;
		if (!XGetInputFocus(display, &src_win, &revert_to))
		{
			fprintf(stderr, "%s: failed to get focus window\n",
			        argv[0]);
			return EXIT_FAILURE;
		}
	}
	root = DefaultRootWindow(display);
	screen = DefaultScreen(display);
	if (!XMatchVisualInfo(display, screen, 24, TrueColor, &vi))
	{
		fprintf(stderr, "%s: failed to get vi\n", argv[0]);
		return EXIT_FAILURE;
	}
	swa.event_mask = ExposureMask;
	mask = CWEventMask;
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
	                (uint8_t*)"shmget",
	                6);
	XMapWindow(display, window);
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
	shminfo.shmaddr = image->data;
	shminfo.readOnly = False;
	XShmAttach(display, &shminfo);
	while (1)
	{
		XEvent event;

		if (XNextEvent(display, &event))
			continue;
		if (event.type == Expose)
		{
			if (!XShmGetImage(display,
			                  src_win,
			                  image,
			                  0,
			                  0,
			                  AllPlanes))
			{
				fprintf(stderr, "%s: failed to get image\n",
				        argv[0]);
				return EXIT_FAILURE;
			}
			XPutImage(display,
			          window,
			          gc,
			          image,
			          0,
			          0,
			          0,
			          0,
			          128,
			          128);
		}
	}
	return EXIT_SUCCESS;
}
