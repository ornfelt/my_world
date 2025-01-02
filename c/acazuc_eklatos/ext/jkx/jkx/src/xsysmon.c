#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>

#include <stdbool.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

enum probe
{
	PROBE_CPU_TOTAL,
	PROBE_CPU_USER,
	PROBE_CPU_SYS,
	PROBE_MEM_TOTAL,
	PROBE_MEM_USED,
	PROBE_MEM_CACHED,
	PROBE_NET_INPUT,
	PROBE_NET_OUTPUT,
	PROBE_LAST,
};

struct graph;
struct env;

typedef float (*get_value_t)(struct env *env, struct graph *graph);

struct graph
{
	Window window;
	float *values;
	XSegment *segments;
	size_t x;
	size_t y;
	size_t width;
	size_t height;
	float per_x;
	float per_y;
	float per_width;
	float per_height;
	get_value_t get_value;
	uint32_t plain_color;
	uint32_t border_color;
};

struct env
{
	const char *progname;
	size_t width;
	size_t height;
	Display *display;
	XVisualInfo vi;
	Window window;
	GC gc;
	uint64_t probe_interval;
	uint64_t probes[PROBE_LAST];
};

static uint64_t nanotime(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000000000 + ts.tv_nsec;
}

static int probe_cpu(struct env *env)
{
	static unsigned long long last_total;
	static unsigned long long last_user;
	static unsigned long long last_sys;
	unsigned long long total = 0;
	unsigned long long user = 0;
	unsigned long long sys = 0;
	bool found = false;

#if defined(__linux__)
	FILE *fp = fopen("/proc/stat", "rb");
	if (!fp)
	{
		fprintf(stderr, "%s: failed to open /proc/stat\n",
		        env->progname);
		return 1;
	}
	char *line = NULL;
	size_t size = 0;
	while ((getline(&line, &size, fp)) > 0)
	{
		if (strncmp(line, "cpu ", 4))
			continue;
		char *endptr = line + 4;
		for (size_t i = 0; i < 4; ++i)
		{
			unsigned long long value = strtoull(endptr + 1, &endptr, 10);
			total += value;
			switch (i)
			{
				case 0: /* user */
				case 1: /* nice */
					user += value;
					break;
				case 2: /* system */
				case 5: /* irq */
				case 6: /* softirq */
					sys += value;
					break;
			}
		}
		found = true;
		break;
	}
	fclose(fp);
#elif defined(__eklat__)
	FILE *fp = fopen("/sys/cpustat", "rb");
	if (!fp)
	{
		fprintf(stderr, "%s: failed to open /sys/cpustat\n",
		        env->progname);
		return 1;
	}
	char *line = NULL;
	size_t size = 0;
	while ((getline(&line, &size, fp)) > 0)
	{
		if (strncmp(line, "cpu ", 4))
			continue;
		char *endptr = line + 3;
		for (size_t i = 0; i < 3; ++i)
		{
			unsigned long long sec = strtoull(endptr + 1, &endptr, 10);
			unsigned long long nsec = strtoull(endptr + 1, &endptr, 10);
			unsigned long long value = sec * 1000 + nsec / 1000000;
			total += value;
			switch (i)
			{
				case 0:
					user += value;
					break;
				case 1:
					sys += value;
					break;
			}
		}
		found = true;
		break;
	}
	fclose(fp);
#else
# error "unknown platform"
#endif
	if (!found)
	{
		fprintf(stderr, "%s: failed to find cpu load\n",
		        env->progname);
		return 1;
	}
	if (last_total)
	{
		env->probes[PROBE_CPU_TOTAL] = total - last_total;
		env->probes[PROBE_CPU_USER] = user - last_user;
		env->probes[PROBE_CPU_SYS] = sys - last_sys;
	}
	last_total = total;
	last_user = user;
	last_sys = sys;
	return 0;
}

static int probe_mem(struct env *env)
{
#if defined(__linux__)
	FILE *fp = fopen("/proc/meminfo", "rb");
	if (!fp)
	{
		fprintf(stderr, "%s: failed to open /proc/meminfo\n",
		        env->progname);
		return 1;
	}
	char *line = NULL;
	size_t size = 0;
	unsigned long long total = ULLONG_MAX;
	unsigned long long available = ULLONG_MAX;
	unsigned long long cached = ULLONG_MAX;
	while ((getline(&line, &size, fp)) > 0)
	{
		if (!strncmp(line, "MemTotal:", 9))
			total = strtoull(line + 9, NULL, 10);
		else if (!strncmp(line, "MemAvailable:", 13))
			available = strtoull(line + 13, NULL, 10);
		else if (!strncmp(line, "Cached:", 7))
			cached = strtoull(line + 7, NULL, 10);
	}
	fclose(fp);
	if (total == ULLONG_MAX
	 || available == ULLONG_MAX
	 || cached == ULLONG_MAX)
	{
		fprintf(stderr, "%s: failed to find memory load\n",
		        env->progname);
		return 1;
	}
	env->probes[PROBE_MEM_TOTAL] = total * 1000;
	env->probes[PROBE_MEM_USED] = (total - available - cached) * 1000;
	env->probes[PROBE_MEM_CACHED] = cached * 1000;
	return 0;
#elif defined(__eklat__)
	FILE *fp = fopen("/sys/meminfo", "rb");
	if (!fp)
	{
		fprintf(stderr, "%s: failed to open /sys/meminfo\n",
		        env->progname);
		return 1;
	}
	char *line = NULL;
	size_t size = 0;
	unsigned long long total = ULLONG_MAX;
	unsigned long long used = ULLONG_MAX;
	while ((getline(&line, &size, fp)) > 0)
	{
		if (!strncmp(line, "PhysicalUsed:", 13))
			used = strtoull(line + 13, NULL, 0);
		else if (!strncmp(line, "PhysicalSize:", 13))
			total = strtoull(line + 13, NULL, 0);
	}
	fclose(fp);
	if (total == ULLONG_MAX
	 || used == ULLONG_MAX)
	{
		fprintf(stderr, "%s: failed to find memory load\n",
		        env->progname);
		return 1;
	}
	env->probes[PROBE_MEM_TOTAL] = total;
	env->probes[PROBE_MEM_USED] = used;
	env->probes[PROBE_MEM_CACHED] = 0;
	return 0;
#else
# error "unknown platform"
#endif
}

static int probe_net(struct env *env)
{
	static unsigned long long last_sum_in;
	static unsigned long long last_sum_out;
	unsigned long long sum_in = 0;
	unsigned long long sum_out = 0;

#if defined(__linux__)
	FILE *fp = fopen("/proc/net/dev", "rb");
	if (!fp)
	{
		fprintf(stderr, "%s: failed to open /proc/net/dev\n",
		        env->progname);
		return 1;
	}
	char *line = NULL;
	size_t size = 0;
	while ((getline(&line, &size, fp)) > 0)
	{
		char *sc = strchr(line, ':');
		if (!sc)
			continue;
		char *endptr = sc + 1;
		for (size_t i = 0; i < 16; ++i)
		{
			unsigned long long value = strtoull(endptr + 1, &endptr, 10);
			if (i == 0)
				sum_in += value;
			if (i == 8)
				sum_out += value;
		}
	}
	fclose(fp);
#elif defined(__eklat__)
#else
# error "unknown platform"
#endif

	if (last_sum_in)
	{
		env->probes[PROBE_NET_INPUT] = sum_in - last_sum_in;
		env->probes[PROBE_NET_OUTPUT] = sum_out - last_sum_out;
	}
	last_sum_in = sum_in;
	last_sum_out = sum_out;
	return 0;
}


static int probe(struct env *env)
{
	return probe_cpu(env)
	    || probe_mem(env)
	    || probe_net(env);
}

static float get_cpu(struct env *env, struct graph *graph)
{
	(void)graph;
	if (!env->probes[PROBE_CPU_TOTAL])
		return 0;
	return (env->probes[PROBE_CPU_USER] + env->probes[PROBE_CPU_SYS])
	     / (float)(env->probes[PROBE_CPU_TOTAL]);
}

static float get_mem(struct env *env, struct graph *graph)
{
	(void)graph;
	if (!env->probes[PROBE_MEM_TOTAL])
		return 0;
	return (env->probes[PROBE_MEM_USED] + env->probes[PROBE_MEM_CACHED])
	     / (float)env->probes[PROBE_MEM_TOTAL];
}

static float get_net_in(struct env *env, struct graph *graph)
{
	(void)graph;
	return env->probes[PROBE_NET_INPUT] / 10000000.;
}

static float get_net_out(struct env *env, struct graph *graph)
{
	(void)graph;
	return env->probes[PROBE_NET_INPUT] / 100000000.;
}

static int configure(struct env *env, struct graph *graph)
{
	uint32_t x = graph->per_x * env->width;
	uint32_t y = graph->per_y * env->height;
	uint32_t width = graph->per_width * env->width;
	uint32_t height = graph->per_height * env->height;
	float *values = malloc(sizeof(*values) * width + 1);
	if (!values)
	{
		fprintf(stderr, "%s: values allocation failed\n", env->progname);
		return 1;
	}
	XSegment *segments = malloc(sizeof(*segments) * width + 1);
	if (!segments)
	{
		fprintf(stderr, "%s: segments allocation failed\n", env->progname);
		free(values);
		return 1;
	}
	size_t ncpy = width;
	if (ncpy > graph->width)
		ncpy = graph->width;
	for (size_t i = 0; i < ncpy; ++i)
		values[width - 1 - i] = graph->values[graph->width - 1 - i];
	for (size_t i = 0; i < width - ncpy; ++i)
		values[i] = 0;
	for (size_t i = 0; i < width; ++i)
	{
		segments[i].x1 = i;
		segments[i].x2 = i;
	}
	free(graph->values);
	free(graph->segments);
	graph->segments = segments;
	graph->values = values;
	graph->x = x;
	graph->y = y;
	graph->width = width;
	graph->height = height;
	XMoveResizeWindow(env->display, graph->window, x, y, width, height);
	return 0;
}

static void draw(struct env *env, struct graph *graph)
{
	XSetForeground(env->display, env->gc, 0xFFFFFFFF);
	XFillRectangle(env->display, graph->window, env->gc,
	               0, 0, graph->width, graph->height);
	XSegment scales[9];
	for (size_t i = 0; i < 9; ++i)
	{
		scales[i].x1 = 0;
		scales[i].y1 = graph->height * (i + 1) / 10;
		scales[i].x2 = graph->width;
		scales[i].y2 = scales[i].y1;
	}
	XSetForeground(env->display, env->gc, 0xFFEEEEEE);
	XDrawSegments(env->display, graph->window, env->gc,
	              scales, sizeof(scales) / sizeof(*scales));
	for (size_t i = 0; i < graph->width; ++i)
	{
		graph->segments[i].y1 = graph->height;
		graph->segments[i].y2 = graph->height - graph->height * graph->values[i];
	}
	XSetForeground(env->display, env->gc, graph->plain_color);
	XDrawSegments(env->display, graph->window, env->gc,
	              graph->segments, graph->width);
	float prev_min = 0;
	float prev_max = 0;
	for (size_t i = 0; i < graph->width; ++i)
	{
		float min = graph->height - graph->height * graph->values[i];
		float max = min;
		if (i)
		{
			if (prev_max < min)
				min = prev_max;
			if (prev_min > max)
				max = prev_min;
		}
		graph->segments[i].y1 = min;
		graph->segments[i].y2 = max;
		prev_min = min;
		prev_max = max;
	}
	XSetForeground(env->display, env->gc, graph->border_color);
	XDrawSegments(env->display, graph->window, env->gc,
	              graph->segments, graph->width);
	XFlush(env->display);
}

static void tick(struct env *env, struct graph *graph)
{
	for (size_t i = 0; i < graph->width; ++i)
	{
		if (i < graph->width - 1)
			graph->values[i] = graph->values[i + 1];
		else
			graph->values[i] = graph->get_value(env, graph);
	}
}

static int init(struct env *env, struct graph *graph,
                float per_x, float per_y,
                float per_width, float per_height,
                get_value_t get_value,
                uint32_t plain_color, uint32_t border_color)
{
	memset(graph, 0, sizeof(*graph));
	graph->per_x = per_x;
	graph->per_y = per_y;
	graph->per_width = per_width;
	graph->per_height = per_height;
	uint32_t x = per_x * env->width;
	uint32_t y = per_y * env->height;
	uint32_t width = per_width * env->width;
	uint32_t height = per_height * env->height;
	XSetWindowAttributes swa;
	unsigned mask = 0;
	graph->window = XCreateWindow(env->display, env->window,
	                              x, y, width, height,
	                              0, env->vi.depth,
	                              InputOutput, env->vi.visual,
	                              mask, &swa);
	if (!graph->window)
	{
		fprintf(stderr, "%s: failed to create window\n", env->progname);
		return 1;
	}
	XMapWindow(env->display, graph->window);
	graph->get_value = get_value;
	graph->plain_color = plain_color;
	graph->border_color = border_color;
	if (configure(env, graph))
		return 1;
	return 0;
}

static int setup_window(struct env *env)
{
	env->display = XOpenDisplay(NULL);
	if (!env->display)
	{
		fprintf(stderr, "%s: failed to open display\n", env->progname);
		return 1;
	}
	Window root = XRootWindow(env->display, 0);
	int screen = DefaultScreen(env->display);
	XVisualInfo vi;
	if (!XMatchVisualInfo(env->display, screen, 24, TrueColor, &vi))
	{
		fprintf(stderr, "%s: failed to get vi\n", env->progname);
		return 1;
	}
	XSetWindowAttributes swa;
	swa.event_mask = ExposureMask | StructureNotifyMask;
	unsigned mask = CWEventMask;
	env->window = XCreateWindow(env->display, root, 0, 0,
	                            env->width, env->height, 0, vi.depth,
	                            InputOutput, vi.visual, mask, &swa);
	if (!env->window)
	{
		fprintf(stderr, "%s: failed to create window\n", env->progname);
		return 1;
	}
	XChangeProperty(env->display, env->window, XA_WM_NAME, XA_STRING, 8,
	                PropModeReplace, (uint8_t*)"xsysmon", 7);
	XGCValues gc_values;
	gc_values.graphics_exposures = 0;
	env->gc = XCreateGC(env->display, env->window,
	                    GCGraphicsExposures, &gc_values);
	if (!env->gc)
	{
		fprintf(stderr, "%s: failed to create GC\n", env->progname);
		return 1;
	}
	return 0;
}

static void usage(const char *progname)
{
	printf("%s [-h] [-i interval]\n", progname);
	printf("-h: show this help\n");
	printf("-i interval: set the refresh interval (in ms)\n");
}

int main(int argc, char **argv)
{
	struct env env;
	int c;

	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	env.width = 640;
	env.height = 480;
	env.probe_interval = 500;
	while ((c = getopt(argc, argv, "hi:")) != -1)
	{
		switch (c)
		{
			case 'i':
			{
				char *endptr;
				errno = 0;
				env.probe_interval = strtoull(optarg, &endptr, 0);
				if (errno || *endptr)
				{
					fprintf(stderr, "%s: invalid interval\n", argv[0]);
					return EXIT_FAILURE;
				}
				break;
			}
			case 'h':
				usage(argv[0]);
				return EXIT_SUCCESS;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	if (optind != argc)
	{
		fprintf(stderr, "%s: extra operand\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (setup_window(&env))
		return EXIT_FAILURE;
	struct graph cpu_graph;
	struct graph mem_graph;
	struct graph net_in_graph;
	struct graph net_out_graph;
	init(&env, &cpu_graph    , 0.0, 0.0, 0.5, 0.5, get_cpu    , 0xFFBBEEFF, 0xFF44AAFF);
	init(&env, &mem_graph    , 0.5, 0.0, 0.5, 0.5, get_mem    , 0xFFCCFFCC, 0xFF00CC00);
	init(&env, &net_in_graph , 0.0, 0.5, 0.5, 0.5, get_net_in , 0xFFDDCCFF, 0xFF8800FF);
	init(&env, &net_out_graph, 0.5, 0.5, 0.5, 0.5, get_net_out, 0xFFFFDDCC, 0xFFEE8800);
	XMapWindow(env.display, env.window);
	XSynchronize(env.display, False);
	struct graph *graphs[] =
	{
		&cpu_graph,
		&mem_graph,
		&net_in_graph,
		&net_out_graph,
	};
	size_t graphs_count = sizeof(graphs) / sizeof(*graphs);
	uint64_t sleep_time = env.probe_interval;
	if (sleep_time < 10)
		sleep_time = 10;
	uint64_t last_probe = nanotime();
	if (probe(&env))
		return EXIT_FAILURE;
	while (1)
	{
		XEvent event;
		bool has_exposure = false;
		while (XPending(env.display))
		{
			XNextEvent(env.display, &event);
			switch (event.type)
			{
				case ConfigureNotify:
					if (event.xconfigure.width == env.width
					 && event.xconfigure.height == env.height)
						break;
					env.width = event.xconfigure.width;
					env.height = event.xconfigure.height;
					for (size_t i = 0; i < graphs_count; ++i)
						configure(&env, graphs[i]);
					break;
				case Expose:
					has_exposure = true;
					break;
			}
		}
		uint64_t curtime = nanotime();
		if ((curtime - last_probe) / 1000000 >= env.probe_interval)
		{
			if (probe(&env))
				return 1;
			for (size_t i = 0; i < graphs_count; ++i)
				tick(&env, graphs[i]);
			last_probe = curtime;
			for (size_t i = 0; i < graphs_count; ++i)
				draw(&env, graphs[i]);
		}
		else if (has_exposure)
		{
			for (size_t i = 0; i < graphs_count; ++i)
				draw(&env, graphs[i]);
		}
		usleep(1000 * sleep_time);
	}
	return EXIT_SUCCESS;
}
