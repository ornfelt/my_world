#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>

#include <sys/shm.h>

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#define THREADS_COUNT 4

enum fractal_type
{
	FRACTAL_MANDELBROT,
	FRACTAL_JULIA,
	FRACTAL_BURNINGSHIP,
};

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
	void (*on_key_down)(struct window *window, KeySym sym);
	void (*on_key_up)(struct window *window, KeySym sym);
};

struct env
{
	const char *progname;
	struct window window;
	enum fractal_type fractal_type;
	double julia_x_factor;
	double julia_y_factor;
	double x_offset;
	double y_offset;
	size_t max_iterations;
	double x_size;
	double x_min;
	double x_max;
	double y_size;
	double y_min;
	double y_max;
	double zoom;
	uint32_t colors[257];
	pthread_t threads[THREADS_COUNT];
	pthread_cond_t start_cond;
	pthread_cond_t end_cond;
	pthread_mutex_t mutex;
	size_t started_threads;
	size_t finished_threads;
};

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

static int setup_window(const char *progname, struct window *window)
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
	                8, PropModeReplace, (uint8_t*)"fractal", 7);
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

static double burningship(struct env *env, double x, double y)
{
	double cx = 0;
	double cy = 0;
	for (size_t i = 0; i < env->max_iterations; ++i)
	{
		double x2 = cx * cx;
		double y2 = cy * cy;
		if (x2 + y2 > 4)
			return env->colors[i * 256 / env->max_iterations];
		double tx = cx > 0 ? cx : -cx;
		double ty = cy > 0 ? cy : -cy;
		cx = x2 - y2 + x;
		cy = 2.0 * tx * ty + y / 1.777777;
	}
	return env->colors[256];
}

static double julia(struct env *env, double x, double y)
{
	double cx = x;
	double cy = y;
	for (size_t i = 0; i < env->max_iterations; ++i)
	{
		double x2 = cx * cx;
		double y2 = cy * cy;
		if (x2 + y2 > 4)
			return env->colors[i * 256 / env->max_iterations];
		double tx = cx;
		double ty = cy;
		cx = x2 - y2 - 0.7;
		cy = 2.0 * tx * ty + 0.27;
	}
	return env->colors[256];
}

static uint32_t mandelbrot(struct env *env, double x, double y)
{
	double cx = 0;
	double cy = 0;
	for (size_t i = 0; i < env->max_iterations; ++i)
	{
		double x2 = cx * cx;
		double y2 = cy * cy;
		if (x2 + y2 > 4)
			return env->colors[i * 256 / env->max_iterations];
		double tx = cx;
		double ty = cy;
		cx = x2 - y2 + x;
		cy = 2. * tx * ty + y / 1.777777;
	}
	return env->colors[256];
}

static uint32_t draw_pixel(struct env *env, size_t x, size_t y)
{
	double px = env->x_min + env->x_size * (x / (double)env->window.width);
	double py = env->y_min + env->y_size * (y / (double)env->window.height);
	switch (env->fractal_type)
	{
		default:
		case FRACTAL_MANDELBROT:
			return mandelbrot(env, px, py);
		case FRACTAL_JULIA:
			return julia(env, px, py);
		case FRACTAL_BURNINGSHIP:
			return burningship(env, px, py);
	}
}

static void draw(struct env *env, size_t id)
{
	uint32_t *dst = (uint32_t*)env->window.image->data;
	size_t start = (env->window.width * env->window.height) * id / THREADS_COUNT;
	size_t end = (env->window.width * env->window.height) * (id + 1) / THREADS_COUNT;
	for (size_t i = start; i < end; ++i)
	{
		size_t x = i % env->window.width;
		size_t y = i / env->window.width;
		dst[i] = draw_pixel(env, x, y);
	}
}

static void on_key_down(struct window *window, KeySym sym)
{
	struct env *env = window->userptr;
	switch (sym)
	{
		case XK_Left:
			env->x_min -= 0.05 / env->zoom;
			env->x_max -= 0.05 / env->zoom;
			env->x_size = env->x_max - env->x_min;
			break;
		case XK_Right:
			env->x_min += 0.05 / env->zoom;
			env->x_max += 0.05 / env->zoom;
			env->x_size = env->x_max - env->x_min;
			break;
		case XK_Up:
			env->y_min -= 0.05 / env->zoom;
			env->y_max -= 0.05 / env->zoom;
			env->y_size = env->y_max - env->y_min;
			break;
		case XK_Down:
			env->y_min += 0.05 / env->zoom;
			env->y_max += 0.05 / env->zoom;
			env->y_size = env->y_max - env->y_min;
			break;
		case XK_Page_Up:
		{
			env->zoom *= 1.15;
			double cx = (env->x_max + env->x_min) / 2.0;
			double cy = (env->y_max + env->y_min) / 2.0;
			env->x_min = cx - 0.5 / env->zoom;
			env->x_max = cx + 0.5 / env->zoom;
			env->y_min = cy - 0.5 / env->zoom;
			env->y_max = cy + 0.5 / env->zoom;
			env->x_size = env->x_max - env->x_min;
			env->y_size = env->y_max - env->y_min;
			break;
		}
		case XK_Page_Down:
		{
			env->zoom /= 1.15;
			if (env->zoom < 0.25)
				env->zoom = 0.25;
			double cx = (env->x_max + env->x_min) / 2.0;
			double cy = (env->y_max + env->y_min) / 2.0;
			env->x_min = cx - 0.5 / env->zoom;
			env->x_max = cx + 0.5 / env->zoom;
			env->y_min = cy - 0.5 / env->zoom;
			env->y_max = cy + 0.5 / env->zoom;
			env->x_size = env->x_max - env->x_min;
			env->y_size = env->y_max - env->y_min;
			break;
		}
		case XK_Home:
			env->max_iterations *= 2;
			break;
		case XK_End:
			env->max_iterations /= 2;
			if (env->max_iterations < 1)
				env->max_iterations = 1;
			break;
	}
}

static float hue2rgb(float p, float q, float t)
{
	if (t < 0)
		t += 1;
	else if (t > 1)
		t -= 1;
	if (t < 1.0 / 6.0)
		return p + (q - p) * 6 * t;
	if (t < 1.0 / 2.0)
		return q;
	if (t < 2.0 / 3.0)
		return p + (q - p) * (2.0 / 3.0 - t) * 6;
	return p;
}

static void hsl2rgb(float *rgb, float *hsl)
{
	if (!hsl[1])
	{
		rgb[0] = hsl[2];
		rgb[1] = hsl[2];
		rgb[2] = hsl[2];
		return;
	}
	float q = hsl[2] < 0.5 ? hsl[2] * (1 + hsl[1]) : hsl[2] + hsl[1] - hsl[2] * hsl[1];
	float p = 2.0 * hsl[2] - q;
	rgb[0] = hue2rgb(p, q, hsl[0] + 1.0 / 3.0);
	rgb[1] = hue2rgb(p, q, hsl[0]);
	rgb[2] = hue2rgb(p, q, hsl[0] - 1.0 / 3.0);
}

static void *thread_run(void *ptr)
{
	struct env *env = ptr;
	while (1)
	{
		pthread_mutex_lock(&env->mutex);
		while (!env->started_threads)
			pthread_cond_wait(&env->start_cond, &env->mutex);
		size_t id = --env->started_threads;
		pthread_mutex_unlock(&env->mutex);

		draw(env, id);

		pthread_mutex_lock(&env->mutex);
		env->finished_threads++;
		pthread_cond_signal(&env->end_cond);
		pthread_mutex_unlock(&env->mutex);
	}
	return NULL;
}

static void usage(const char *progname)
{
	printf("%s [-h] [mandelbrot | julia | burningship]\n", progname);
	printf("-h: show this help\n");
}

int main(int argc, char **argv)
{
	struct env env;
	int c;

	memset(&env, 0, sizeof(env));
	for (size_t i = 0; i < 256; ++i)
	{
		float hsl[3];
		float rgb[3];
		hsl[0] = i / 256.0;
		hsl[1] = 0.5;
		hsl[2] = 0.5;
		hsl2rgb(rgb, hsl);
		env.colors[i] = 0xFF000000
		              | ((uint8_t)(rgb[0] * 0xFF) << 16)
		              | ((uint8_t)(rgb[1] * 0xFF) << 8)
		              | ((uint8_t)(rgb[2] * 0xFF) << 0);
	}
	env.colors[256] = 0xFF000000;
	env.x_min = -2;
	env.x_max = 2;
	env.y_min = -2;
	env.y_max = 2;
	env.x_size = env.x_max - env.x_min;
	env.y_size = env.y_max - env.y_min;
	env.zoom = 0.25;
	env.progname = argv[0];
	env.max_iterations = 100;
	env.fractal_type = FRACTAL_MANDELBROT;
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
	if (argc - optind > 1)
	{
		fprintf(stderr, "%s: extra operand\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (optind != argc)
	{
		if (!strcmp(argv[optind], "mandelbrot"))
		{
			env.fractal_type = FRACTAL_MANDELBROT;
		}
		else if (!strcmp(argv[optind], "julia"))
		{
			env.fractal_type = FRACTAL_JULIA;
		}
		else if (!strcmp(argv[optind], "burningship"))
		{
			env.fractal_type = FRACTAL_BURNINGSHIP;
		}
		else
		{
			fprintf(stderr, "%s: unknown fractal\n",
			        argv[0]);
			return EXIT_FAILURE;
		}
	}
	if (setup_window(argv[0], &env.window))
		return EXIT_FAILURE;
	env.window.userptr = &env;
	env.window.on_key_down = on_key_down;
	if (pthread_cond_init(&env.start_cond, NULL)
	 || pthread_cond_init(&env.end_cond, NULL))
	{
		fprintf(stderr, "%s: failed to setup condvar\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (pthread_mutex_init(&env.mutex, NULL))
	{
		fprintf(stderr, "%s: failed to create mutex\n", argv[0]);
		return EXIT_FAILURE;
	}
	for (size_t i = 0; i < THREADS_COUNT; ++i)
	{
		if (pthread_create(&env.threads[i], NULL, thread_run, &env))
		{
			fprintf(stderr, "%s: failed to create thread\n",
			        argv[0]);
			return EXIT_FAILURE;
		}
	}
	while (1)
	{
		handle_events(&env.window);
		pthread_mutex_lock(&env.mutex);
		env.started_threads = THREADS_COUNT;
		env.finished_threads = 0;
		pthread_cond_broadcast(&env.start_cond);
		while (env.finished_threads != THREADS_COUNT)
			pthread_cond_wait(&env.end_cond, &env.mutex);
		pthread_mutex_unlock(&env.mutex);
		swap_buffers(&env.window);
	}
	return EXIT_SUCCESS;
}
