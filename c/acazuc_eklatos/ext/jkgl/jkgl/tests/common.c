#include "common.h"

#define __BSD_VISIBLE 1
#include <sys/mman.h>
#include <sys/shm.h>

#include <X11/keysym.h>
#include <X11/Xatom.h>

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

#include "gl.h"

uint64_t
nanotime(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000000000 + ts.tv_nsec;
}

static float
hue2rgb(float p, float q, float t)
{
	if (t < 0)
		t += 1;
	else if (t > 1)
		t -= 1;
	if (t < 1 / 6.0)
		return p + (q - p) * 6 * t;
	if (t < 1 / 2.0)
		return q;
	if (t < 2 / 3.0)
		return p + (q - p) * (2 / 3.0 - t) * 6;
	return p;
}

void
hsl2rgb(float *rgb, float *hsl)
{
	if (!hsl[1])
	{
		rgb[0] = hsl[2];
		rgb[1] = hsl[2];
		rgb[2] = hsl[2];
		return;
	}
	float q = hsl[2] < 0.5 ? hsl[2] * (1 + hsl[1]) : hsl[2] + hsl[1] - hsl[2] * hsl[1];
	float p = 2 * hsl[2] - q;
	rgb[0] = hue2rgb(p, q, hsl[0] + 1 / 3.0);
	rgb[1] = hue2rgb(p, q, hsl[0]);
	rgb[2] = hue2rgb(p, q, hsl[0] - 1 / 3.0);
}

static int
create_shmimg(struct window *window)
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

static void
handle_configure(struct window *window, XConfigureEvent *event)
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
	GL_CALL(gl_ctx_resize, window->ctx, window->width, window->height);
}

void
handle_events(struct window *window)
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

int
setup_window(const char *progname, struct window *window)
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
	GL_CALL(gl_ctx_resize, window->ctx, window->width, window->height);
	window->last_fps = nanotime();
	return 0;
}

void
swap_buffers(struct window *window)
{
	GL_CALL(glReadPixels, 0, 0, window->width, window->height, GL_RGBA,
	        GL_UNSIGNED_BYTE, window->image->data);
	for (size_t y = 0; y < window->height / 2; ++y)
	{
		uint32_t *src = (uint32_t*)&window->image->data[4 * window->width * y];
		uint32_t *dst = (uint32_t*)&window->image->data[4 * window->width * (window->height - 1 - y)];
		uint32_t tmp;
		for (size_t x = 0; x < window->width; ++x)
		{
			tmp = src[x];
			src[x] = dst[x];
			dst[x] = tmp;
		}
	}
	gl_ctx_swap_buffers(window->ctx);
	XShmPutImage(window->display, window->window, window->gc,
	             window->image, 0, 0, 0, 0,
	             window->width, window->height, False);
	XSync(window->display, False);

	uint64_t current = nanotime();
	window->fps++;
	if (current - window->last_fps >= 1000000000)
	{
#if 1
		printf("fps: %" PRIu64 "\n", window->fps);
#endif
		if (current - window->last_fps >= 2000000000)
			window->last_fps = current;
		else
			window->last_fps += 1000000000;
		window->fps = 0;
	}
}
