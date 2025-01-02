#include <libh261/h261.h>

#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>

#include <sys/shm.h>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

struct window
{
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
};

struct env
{
	const char *progname;
	struct window window;
};

static uint64_t nanotime(void)
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

static void handle_events(struct window *window)
{
	while (XPending(window->display))
	{
		XEvent event;
		XNextEvent(window->display, &event);
		switch (event.type)
		{
			case ConfigureNotify:
				handle_configure(window, &event.xconfigure);
				break;
		}
	}
}

static int setup_window(const char *progname, struct window *window)
{
	memset(window, 0, sizeof(*window));
	window->progname = progname;
	window->width = 352;
	window->height = 288;
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
	                8, PropModeReplace, (uint8_t*)"h261", 4);
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
	return 0;
}

static void swap_buffers(struct window *window)
{
	XShmPutImage(window->display, window->window, window->gc,
	             window->image, 0, 0, 0, 0,
	             window->width, window->height, False);
	XSync(window->display, False);
}

static void draw(struct window *window, const uint8_t *data,
                 uint32_t width, uint32_t height)
{
	uint32_t maxy = height > window->height ? window->height : height;
	uint32_t maxx = width > window->width ? window->width : width;
	const uint8_t *src = data;
	uint8_t *dst = (uint8_t*)window->image->data;
	uint32_t src_pitch = width * 3;
	uint32_t dst_pitch = window->width * 4;
	for (uint32_t y = 0; y < maxy; ++y)
	{
		const uint8_t *row_src = src;
		uint8_t *row_dst = dst;
		for (uint32_t x = 0; x < maxx; ++x)
		{
			row_dst[x * 4 + 0] = row_src[x * 3 + 2];
			row_dst[x * 4 + 1] = row_src[x * 3 + 1];
			row_dst[x * 4 + 2] = row_src[x * 3 + 0];
			row_dst[x * 4 + 3] = 0xFF;
		}
		src += src_pitch;
		dst += dst_pitch;
	}
}

static int read_h261(struct env *env, const char *filename)
{
	FILE *fp = NULL;
	struct h261 *h261 = NULL;
	int ret = 1;

	fp = fopen(filename, "rb");
	if (!fp)
	{
		fprintf(stderr, "%s: fopen(%s): %s\n", env->progname,
		        filename, strerror(errno));
		goto end;
	}
	h261 = h261_new();
	if (!h261)
	{
		fprintf(stderr, "%s: h261_new failed\n", env->progname);
		goto end;
	}
	h261_init_io(h261, fp);
	while (1)
	{
		uint8_t data[352 * 288 * 3];
		uint32_t width;
		uint32_t height;
		uint64_t frame_start;
		uint64_t frame_end;
		uint64_t frame_diff;
		struct timespec ts;

		frame_start = nanotime();
		handle_events(&env->window);
		switch (h261_decode_frame(h261, data, &width, &height))
		{
			case -1:
				ret = 0;
				goto end;
			case 0:
				break;
			case 1:
				fprintf(stderr, "%s: h261_decode_frame: %s\n",
				        env->progname, h261_get_err(h261));
				goto end;
		}
		draw(&env->window, data, width, height);
		swap_buffers(&env->window);
		frame_end = nanotime();
		frame_diff = frame_end - frame_start;
		if (frame_diff >= 33366666)
			continue;
		ts.tv_sec = 0;
		ts.tv_nsec = 33366666 - frame_diff;
		nanosleep(&ts, NULL);
	}
	ret = 0;

end:
	if (fp)
		fclose(fp);
	h261_free(h261);
	return ret;
}

static void usage(const char *progname)
{
	printf("%s [-h] FILE\n", progname);
	printf("-h: show this help\n");
}

int main(int argc, char **argv)
{
	struct env env;
	int c;

	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	while ((c = getopt(argc, argv, "h")) != -1)
	{
		switch (c)
		{
			case 'h':
				usage(argv[0]);
				return EXIT_SUCCESS;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	if (optind == argc)
	{
		fprintf(stderr, "%s: missing operand\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (optind != argc - 1)
	{
		fprintf(stderr, "%s: extra operand\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (setup_window(argv[0], &env.window))
		return EXIT_FAILURE;
	if (read_h261(&env, argv[optind]))
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}
