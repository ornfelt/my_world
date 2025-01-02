#include "common.h"

#define __BSD_VISIBLE 1
#include <sys/mman.h>
#include <sys/shm.h>

#include <X11/keysym.h>
#include <X11/Xatom.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

#include "gl.h"

uint64_t nanotime(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000000000 + ts.tv_nsec;
}

static int create_shmimg(struct window *window)
{
	window->image = XShmCreateImage(window->display, window->vi.visual, 24,
	                                ZPixmap, NULL, &window->shminfo,
	                                window->width, window->height);
	if (!window->image)
	{
		fprintf(stderr, "%s: failed to create image\n",
		        window->progname);
		return 1;
	}
	window->shminfo.shmid = shmget(IPC_PRIVATE,
	                               window->image->bytes_per_line
	                             * window->image->height,
	                               IPC_CREAT | 0777);
	if (window->shminfo.shmid == -1)
	{
		fprintf(stderr, "%s: shmget: %s\n", window->progname,
		        strerror(errno));
		return 1;
	}
	window->image->data = shmat(window->shminfo.shmid, 0, 0);
	if (!window->image->data)
	{
		fprintf(stderr, "%s: shmat: %s\n", window->progname,
		        strerror(errno));
		return 1;
	}
	window->shminfo.shmaddr = window->image->data;
	window->shminfo.readOnly = False;
	XShmAttach(window->display, &window->shminfo);
	XSync(window->display, False);
	if (shmctl(window->shminfo.shmid, IPC_RMID, NULL) == -1)
	{
		fprintf(stderr, "%s: shmctl: %s\n", window->progname,
		        strerror(errno));
		return 1;
	}
	return 0;
}

static void handle_configure(struct window *window, XConfigureEvent *event)
{
	if ((uint32_t)event->width == window->width
	 && (uint32_t)event->height == window->height)
		return;
	XFillRectangle(window->display, window->window, window->gc, 0, 0,
	               event->width, event->height);
	window->width = event->width;
	window->height = event->height;
	XShmDetach(window->display, &window->shminfo);
	shmdt(window->image->data);
	window->image->data = NULL;
	XDestroyImage(window->image);
	if (create_shmimg(window))
		exit(EXIT_FAILURE);
}

void handle_events(struct window *window)
{
	while (XPending(window->display))
	{
		XEvent event;
		XNextEvent(window->display, &event);
		switch (event.type)
		{
			case KeyPress:
				if (window->on_key_down)
				{
					KeySym sym = XLookupKeysym(&event.xkey, 0);
					window->on_key_down(window, sym);
				}
				break;
			case KeyRelease:
				if (window->on_key_up)
				{
					KeySym sym = XLookupKeysym(&event.xkey, 0);
					window->on_key_up(window, sym);
				}
				break;
			case ConfigureNotify:
				handle_configure(window, &event.xconfigure);
				break;
		}
	}
}

int setup_window(const char *progname, struct window *window)
{
	memset(window, 0, sizeof(*window));
	window->progname = progname;
	window->width = 640;
	window->height = 480;
	window->display = XOpenDisplay(NULL);
	if (!window->display)
	{
		fprintf(stderr, "%s: failed to open display\n", progname);
		return 1;
	}
	window->root = XRootWindow(window->display, 0);
	window->screen = DefaultScreen(window->display);
	if (!XMatchVisualInfo(window->display, window->screen, 24, TrueColor,
	                      &window->vi))
	{
		fprintf(stderr, "%s: failed to find visual\n", progname);
		return 1;
	}
	XSetWindowAttributes swa;
	swa.event_mask = KeyPressMask | KeyReleaseMask | StructureNotifyMask;
	swa.bit_gravity = CenterGravity;
	window->window = XCreateWindow(window->display, window->root, 0, 0,
	                               window->width, window->height, 0,
	                               window->vi.depth,
	                               InputOutput, window->vi.visual,
	                               CWEventMask | CWBitGravity, &swa);
	XChangeProperty(window->display, window->window, XA_WM_NAME, XA_STRING,
	                8, PropModeReplace, (uint8_t*)"gl test", 7);
	XGCValues gc_values;
	gc_values.foreground = 0;
	window->gc = XCreateGC(window->display, window->window,
	                       GCForeground, &gc_values);
	if (!window->gc)
	{
		fprintf(stderr, "%s: failed to create GC\n", progname);
		return 1;
	}
	if (create_shmimg(window))
		return 1;
	XMapWindow(window->display, window->window);
	XFlush(window->display);
	XSynchronize(window->display, False);
	window->ctx = gl_ctx_new();
	if (!window->ctx)
	{
		fprintf(stderr, "%s: failed to create ctx\n", window->progname);
		return 1;
	}
	GL_CALL(gl_ctx_set, window->ctx);
	GL_CALL(glViewport, window->width, window->height);
	return 0;
}

void swap_buffers(struct window *window)
{
	glReadPixels(0, 0, window->width, window->height, GL_RGBA,
	             GL_UNSIGNED_BYTE, window->image->data);
	XShmPutImage(window->display, window->window, window->gc,
	             window->image, 0, 0, 0, 0,
	             window->width, window->height, False);
	XSync(window->display, False);
}
