#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>

#include <sys/queue.h>
#include <sys/param.h>

#include <inttypes.h>
#include <stdbool.h>
#include <dirent.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

#define MAXCPU 256

enum probe
{
	PROBE_CPU_TOTAL,
	PROBE_CPU_USR,
	PROBE_CPU_SYS,
	PROBE_CPU0_TOTAL,
	PROBE_CPU0_USR,
	PROBE_CPU0_SYS,
	PROBE_MEM_TOTAL = PROBE_CPU_SYS + 3 * MAXCPU,
	PROBE_MEM_USED,
	PROBE_MEM_CACHED,
	PROBE_NET_INPUT,
	PROBE_NET_OUTPUT,
	PROBE_LAST,
};

enum graph_border
{
	GRAPH_BORDER_LEFT   = (1 << 0),
	GRAPH_BORDER_RIGHT  = (1 << 1),
	GRAPH_BORDER_TOP    = (1 << 2),
	GRAPH_BORDER_BOTTOM = (1 << 3),
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
	enum graph_border borders;
	void *userdata;
	TAILQ_ENTRY(graph) chain;
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
	uint64_t prev_probes[PROBE_LAST];
	uint64_t probes[PROBE_LAST];
	TAILQ_HEAD(, graph) graphs;
};

static uint64_t
nanotime(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000000000 + ts.tv_nsec;
}

#if defined(__linux__)

static int
get_ncpu(struct env *env, size_t *ncpu)
{
	FILE *fp = NULL;
	char *line = NULL;
	size_t size = 0;
	int ret = 1;

	*ncpu = 0;
	fp = fopen("/proc/stat", "rb");
	if (!fp)
	{
		fprintf(stderr, "%s: open(%s): %s\n",
		        env->progname,
		        "/proc/stat",
		        strerror(errno));
		goto end;
	}
	while ((getline(&line, &size, fp)) > 0)
	{
		uint32_t id;

		if (!sscanf(line, "cpu%" PRIu32, &id))
		{
			id++;
			if (id > *ncpu)
				*ncpu = id;
		}
	}
	ret = 0;

end:
	fclose(fp);
	return ret;
}

static int
probe_cpu(struct env *env)
{
	uint64_t values[7];
	uint64_t *prev_probes;
	uint64_t *probes;
	uint64_t total;
	uint64_t usr;
	uint64_t sys;
	FILE *fp = NULL;
	char *line = NULL;
	size_t size = 0;
	int ret = 1;

	fp = fopen("/proc/stat", "rb");
	if (!fp)
	{
		fprintf(stderr, "%s: open(%s): %s\n",
		        env->progname,
		        "/proc/stat",
		        strerror(errno));
		goto end;
	}
	while ((getline(&line, &size, fp)) > 0)
	{
		if (!strncmp(line, "cpu ", 4))
		{
			if (sscanf(line,
			           "cpu "
			           "%" PRIu64 " "
			           "%" PRIu64 " "
			           "%" PRIu64 " "
			           "%" PRIu64 " "
			           "%" PRIu64 " "
			           "%" PRIu64 " "
			           "%" PRIu64,
			           &values[0],
			           &values[1],
			           &values[2],
			           &values[3],
			           &values[4],
			           &values[5],
			           &values[6]) == EOF)
			{
				fprintf(stderr, "%s: sscanf: %s\n",
				        env->progname,
				        strerror(errno));
				goto end;
			}
			prev_probes = &env->prev_probes[PROBE_CPU_TOTAL];
			probes = &env->probes[PROBE_CPU_TOTAL];
		}
		else if (!strncmp(line, "cpu", 3))
		{
			uint32_t id;

			if (sscanf(line,
			           "cpu%" PRIu32 " "
			           "%" PRIu64 " "
			           "%" PRIu64 " "
			           "%" PRIu64 " "
			           "%" PRIu64 " "
			           "%" PRIu64 " "
			           "%" PRIu64 " "
			           "%" PRIu64,
			           &id,
			           &values[0],
			           &values[1],
			           &values[2],
			           &values[3],
			           &values[4],
			           &values[5],
			           &values[6]) == EOF)
			{
				fprintf(stderr, "%s: sscanf: %s\n",
				        env->progname,
				        strerror(errno));
				goto end;
			}
			if (id >= MAXCPU)
				continue;
			prev_probes = &env->prev_probes[PROBE_CPU0_TOTAL + 3 * id];
			probes = &env->probes[PROBE_CPU0_TOTAL + 3 * id];
		}
		else
		{
			continue;
		}
		total = values[0] + values[1] + values[2] + values[3] + values[5] + values[6];
		usr = values[0] + values[1];
		sys = values[2] + values[5] + values[6];
		if (prev_probes[0])
		{
			probes[0] = total - prev_probes[0];
			probes[1] = usr - prev_probes[1];
			probes[2] = sys - prev_probes[2];
		}
		prev_probes[0] = total;
		prev_probes[1] = usr;
		prev_probes[2] = sys;
	}
	ret = 0;

end:
	fclose(fp);
	return ret;
}

static int
probe_mem(struct env *env)
{
	uint64_t total = UINT64_MAX;
	uint64_t available = UINT64_MAX;
	uint64_t cached = UINT64_MAX;
	FILE *fp;
	char *line = NULL;
	size_t size = 0;

	fp = fopen("/proc/meminfo", "rb");
	if (!fp)
	{
		fprintf(stderr, "%s: open(%s): %s\n",
		        env->progname,
		        "/proc/meminfo",
		        strerror(errno));
		return 1;
	}
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
	if (total == UINT64_MAX
	 || available == UINT64_MAX
	 || cached == UINT64_MAX)
	{
		fprintf(stderr, "%s: failed to find memory load\n",
		        env->progname);
		return 1;
	}
	env->probes[PROBE_MEM_TOTAL] = total * 1000;
	env->probes[PROBE_MEM_USED] = (total - available - cached) * 1000;
	env->probes[PROBE_MEM_CACHED] = cached * 1000;
	return 0;
}

static int
probe_net(struct env *env)
{
	uint64_t sum_in = 0;
	uint64_t sum_out = 0;
	FILE *fp;
	char *line = NULL;
	size_t size = 0;

	fp = fopen("/proc/net/dev", "rb");
	if (!fp)
	{
		fprintf(stderr, "%s: open(%s): %s\n",
		        env->progname,
		        "/proc/net/dev",
		        strerror(errno));
		return 1;
	}
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
	if (env->prev_probes[PROBE_NET_INPUT])
	{
		env->probes[PROBE_NET_INPUT] = sum_in - env->prev_probes[PROBE_NET_INPUT];
		env->probes[PROBE_NET_OUTPUT] = sum_out - env->prev_probes[PROBE_NET_OUTPUT];
	}
	env->prev_probes[PROBE_NET_INPUT] = sum_in;
	env->prev_probes[PROBE_NET_OUTPUT] = sum_out;
	fclose(fp);
	return 0;
}

#elif defined(__eklat__)

static int
get_ncpu(struct env *env, size_t *ncpu)
{
	FILE *fp = NULL;
	char *line = NULL;
	size_t size = 0;
	int ret = 1;

	*ncpu = 0;
	fp = fopen("/sys/cpustat", "rb");
	if (!fp)
	{
		fprintf(stderr, "%s: open(%s): %s\n",
		        env->progname,
		        "/sys/cpustat",
		        strerror(errno));
		goto end;
	}
	while ((getline(&line, &size, fp)) > 0)
	{
		uint32_t id;

		if (!sscanf(line, "cpu%" PRIu32, &id))
		{
			id++;
			if (id > *ncpu)
				*ncpu = id;
		}
	}
	ret = 0;

end:
	fclose(fp);
	return ret;
}

static int
probe_cpu(struct env *env)
{
	uint64_t sec[3];
	uint64_t nsec[3];
	uint64_t *prev_probes;
	uint64_t *probes;
	uint64_t total;
	uint64_t usr;
	uint64_t sys;
	FILE *fp = NULL;
	char *line = NULL;
	size_t size = 0;
	int ret = 1;

	fp = fopen("/sys/cpustat", "rb");
	if (!fp)
	{
		fprintf(stderr, "%s: open(%s): %s\n",
		        env->progname,
		        "/sys/cpustat",
		        strerror(errno));
		goto end;
	}
	while ((getline(&line, &size, fp)) > 0)
	{
		if (!strncmp(line, "cpu ", 4))
		{
			if (sscanf(line,
			           "cpu "
			           "%" PRIu64 "."
			           "%" PRIu64 " "
			           "%" PRIu64 "."
			           "%" PRIu64 " "
			           "%" PRIu64 "."
			           "%" PRIu64 " ",
			           &sec[0],
			           &nsec[0],
			           &sec[1],
			           &nsec[1],
			           &sec[2],
			           &nsec[2]) == EOF)
			{
				fprintf(stderr, "%s: sscanf: %s\n",
				        env->progname,
				        strerror(errno));
				goto end;
			}
			prev_probes = &env->prev_probes[PROBE_CPU_TOTAL];
			probes = &env->probes[PROBE_CPU_TOTAL];
		}
		else if (!strncmp(line, "cpu", 3))
		{
			uint32_t id;

			if (sscanf(line,
			           "cpu%" PRIu32 " "
			           "%" PRIu64 "."
			           "%" PRIu64 " "
			           "%" PRIu64 "."
			           "%" PRIu64 " "
			           "%" PRIu64 "."
			           "%" PRIu64 " ",
			           &id,
			           &sec[0],
			           &nsec[0],
			           &sec[1],
			           &nsec[1],
			           &sec[2],
			           &nsec[2]) == EOF)
			{
				fprintf(stderr, "%s: sscanf: %s\n",
				        env->progname,
				        strerror(errno));
				goto end;
			}
			if (id >= MAXCPU)
				continue;
			prev_probes = &env->prev_probes[PROBE_CPU0_TOTAL + 3 * id];
			probes = &env->probes[PROBE_CPU0_TOTAL + 3 * id];
		}
		else
		{
			continue;
		}
		usr = sec[0] * 1000 + nsec[0] / 1000000;
		sys = sec[1] * 1000 + nsec[1] / 1000000;
		total = usr + sys + sec[2] * 1000 + nsec[2] / 1000000;
		if (prev_probes[0])
		{
			probes[0] = total - prev_probes[0];
			probes[1] = usr - prev_probes[1];
			probes[2] = sys - prev_probes[2];
		}
		prev_probes[0] = total;
		prev_probes[1] = usr;
		prev_probes[2] = sys;
	}
	ret = 0;

end:
	fclose(fp);
	return ret;
}

static int
probe_mem(struct env *env)
{
	uint64_t total = UINT64_MAX;
	uint64_t used = UINT64_MAX;
	FILE *fp;
	char *line = NULL;
	size_t size = 0;

	fp = fopen("/sys/meminfo", "rb");
	if (!fp)
	{
		fprintf(stderr, "%s: open(%s): %s\n",
		        env->progname,
		        "/sys/meminfo",
		        strerror(errno));
		return 1;
	}
	while ((getline(&line, &size, fp)) > 0)
	{
		if (!strncmp(line, "PhysicalUsed:", 13))
			used = strtoull(line + 13, NULL, 0);
		else if (!strncmp(line, "PhysicalSize:", 13))
			total = strtoull(line + 13, NULL, 0);
	}
	fclose(fp);
	if (total == UINT64_MAX
	 || used == UINT64_MAX)
	{
		fprintf(stderr, "%s: failed to find memory load\n",
		        env->progname);
		return 1;
	}
	env->probes[PROBE_MEM_TOTAL] = total;
	env->probes[PROBE_MEM_USED] = used;
	env->probes[PROBE_MEM_CACHED] = 0;
	return 0;
}

static int
probe_net_if(struct env *env,
             const char *name,
             uint64_t *sum_in,
             uint64_t *sum_out)
{
	char path[MAXPATHLEN];
	FILE *fp = NULL;
	char *line = NULL;
	size_t size = 0;
	int ret = 1;

	snprintf(path, sizeof(path), "/sys/net/%s", name);
	fp = fopen(path, "rb");
	if (!fp)
	{
		fprintf(stderr, "%s: open(%s): %s\n",
		        env->progname,
		        path,
		        strerror(errno));
		goto end;
	}
	while ((getline(&line, &size, fp)) > 0)
	{
		if (!strncmp(line, "rx_bytes:", 9))
			*sum_in += strtoull(&line[9], NULL, 10);
		if (!strncmp(line, "tx_bytes:", 9))
			*sum_out += strtoull(&line[9], NULL, 10);
	}
	ret = 0;

end:
	fclose(fp);
	return ret;
}

static int
probe_net(struct env *env)
{
	uint64_t sum_in = 0;
	uint64_t sum_out = 0;
	struct dirent *dirent;
	DIR *dir = NULL;
	int ret = 1;

	dir = opendir("/sys/net");
	if (!dir)
	{
		fprintf(stderr, "%s: opendir(%s): %s\n",
		        env->progname,
		        "/sys/net",
		        strerror(errno));
		goto end;
	}
	while ((dirent = readdir(dir)))
	{
		if (!strcmp(dirent->d_name, ".")
		 || !strcmp(dirent->d_name, ".."))
			continue;
		if (probe_net_if(env, dirent->d_name, &sum_in, &sum_out))
			goto end;
	}
	if (env->prev_probes[PROBE_NET_INPUT])
	{
		env->probes[PROBE_NET_INPUT] = sum_in - env->prev_probes[PROBE_NET_INPUT];
		env->probes[PROBE_NET_OUTPUT] = sum_out - env->prev_probes[PROBE_NET_OUTPUT];
	}
	env->prev_probes[PROBE_NET_INPUT] = sum_in;
	env->prev_probes[PROBE_NET_OUTPUT] = sum_out;
	ret = 0;

end:
	closedir(dir);
	return ret;
}

#else
# error "unknown platform"
#endif

static int
probe(struct env *env)
{
	return probe_cpu(env)
	    || probe_mem(env)
	    || probe_net(env);
}

static float
get_cpu(struct env *env, struct graph *graph)
{
	(void)graph;
	if (!env->probes[PROBE_CPU_TOTAL])
		return 0;
	return (env->probes[PROBE_CPU_USR] + env->probes[PROBE_CPU_SYS])
	     / (float)(env->probes[PROBE_CPU_TOTAL]);
}

static float
get_cpun(struct env *env, struct graph *graph)
{
	(void)graph;
	if (!env->probes[PROBE_CPU0_TOTAL + 3 * (uintptr_t)graph->userdata])
		return 0;
	return (env->probes[PROBE_CPU_USR + 3 * (uintptr_t)graph->userdata]
	      + env->probes[PROBE_CPU_SYS + 3 * (uintptr_t)graph->userdata])
	     / (float)(env->probes[PROBE_CPU_TOTAL + 3 * (uintptr_t)graph->userdata]);
}

static float
get_mem(struct env *env, struct graph *graph)
{
	(void)graph;
	if (!env->probes[PROBE_MEM_TOTAL])
		return 0;
	return (env->probes[PROBE_MEM_USED] + env->probes[PROBE_MEM_CACHED])
	     / (float)env->probes[PROBE_MEM_TOTAL];
}

static float
get_net_in(struct env *env, struct graph *graph)
{
	(void)graph;
	return env->probes[PROBE_NET_INPUT] / 10000000.;
}

static float
get_net_out(struct env *env, struct graph *graph)
{
	(void)graph;
	return env->probes[PROBE_NET_INPUT] / 100000000.;
}

static int
graph_resize(struct env *env, struct graph *graph)
{
	uint32_t x = graph->per_x * env->width;
	uint32_t y = graph->per_y * env->height;
	uint32_t width = graph->per_width * env->width;
	uint32_t height = graph->per_height * env->height;
	XSegment *segments;
	float *values;

	values = malloc(sizeof(*values) * width + 1);
	if (!values)
	{
		fprintf(stderr, "%s: malloc: %s\n",
		        env->progname,
		        strerror(errno));
		return 1;
	}
	segments = malloc(sizeof(*segments) * width + 1);
	if (!segments)
	{
		fprintf(stderr, "%s: malloc: %s\n",
		        env->progname,
		        strerror(errno));
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

static void
draw(struct env *env, struct graph *graph)
{
	XSegment scales[9];

	XSetForeground(env->display, env->gc, 0xFFFFFFFF);
	XFillRectangle(env->display,
	               graph->window,
	               env->gc,
	               0,
	               0,
	               graph->width,
	               graph->height);
	for (size_t i = 0; i < 9; ++i)
	{
		scales[i].x1 = 0;
		scales[i].y1 = graph->height * (i + 1) / 10;
		scales[i].x2 = graph->width;
		scales[i].y2 = scales[i].y1;
	}
	XSetForeground(env->display, env->gc, 0xFFEEEEEE);
	XDrawSegments(env->display,
	              graph->window,
	              env->gc,
	              scales,
	              sizeof(scales) / sizeof(*scales));
	for (size_t i = 0; i < graph->width; ++i)
	{
		graph->segments[i].y1 = graph->height;
		graph->segments[i].y2 = graph->height - graph->height * graph->values[i];
	}
	XSetForeground(env->display, env->gc, graph->plain_color);
	XDrawSegments(env->display,
	              graph->window,
	              env->gc,
	              graph->segments,
	              graph->width);
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
	XDrawSegments(env->display,
	              graph->window,
	              env->gc,
	              graph->segments,
	              graph->width);
	XFlush(env->display);
}

static void
tick(struct env *env, struct graph *graph)
{
	for (size_t i = 0; i < graph->width; ++i)
	{
		if (i < graph->width - 1)
			graph->values[i] = graph->values[i + 1];
		else
			graph->values[i] = graph->get_value(env, graph);
	}
}

static void
graph_free(struct env *env, struct graph *graph)
{
	if (graph->window)
		XDestroyWindow(env->display, graph->window);
	free(graph->values);
	free(graph->segments);
	free(graph);
}

static struct graph *
graph_alloc(struct env *env,
            float per_x,
            float per_y,
            float per_width,
            float per_height,
            get_value_t get_value,
            uint32_t plain_color,
            uint32_t border_color,
            enum graph_border borders,
            void *userdata)
{
	struct graph *graph;

	graph = calloc(1, sizeof(*graph));
	if (!graph)
	{
		fprintf(stderr, "%s: malloc: %s\n",
		        env->progname,
		        strerror(errno));
		goto err;
	}
	memset(graph, 0, sizeof(*graph));
	graph->per_x = per_x;
	graph->per_y = per_y;
	graph->per_width = per_width;
	graph->per_height = per_height;
	graph->borders = borders;
	graph->userdata = userdata;
	graph->window = XCreateWindow(env->display,
	                              env->window,
	                              per_x * env->width,
	                              per_y * env->height,
	                              per_width * env->width,
	                              per_height * env->height,
	                              0,
	                              env->vi.depth,
	                              InputOutput,
	                              env->vi.visual,
	                              0,
	                              NULL);
	if (!graph->window)
	{
		fprintf(stderr, "%s: failed to create window\n", env->progname);
		goto err;
	}
	XMapWindow(env->display, graph->window);
	graph->get_value = get_value;
	graph->plain_color = plain_color;
	graph->border_color = border_color;
	if (graph_resize(env, graph))
		goto err;
	return graph;

err:
	graph_free(env, graph);
	return NULL;
}

static int
setup_window(struct env *env)
{
	XSetWindowAttributes swa;
	XVisualInfo vi;
	XGCValues gc_values;
	Window root;
	unsigned mask;
	int screen;

	env->display = XOpenDisplay(NULL);
	if (!env->display)
	{
		fprintf(stderr, "%s: failed to open display\n", env->progname);
		return 1;
	}
	root = XRootWindow(env->display, 0);
	screen = DefaultScreen(env->display);
	if (!XMatchVisualInfo(env->display, screen, 24, TrueColor, &vi))
	{
		fprintf(stderr, "%s: failed to get vi\n", env->progname);
		return 1;
	}
	swa.event_mask = ExposureMask | StructureNotifyMask;
	mask = CWEventMask;
	env->window = XCreateWindow(env->display,
	                            root,
	                            0,
	                            0,
	                            env->width,
	                            env->height,
	                            0,
	                            vi.depth,
	                            InputOutput,
	                            vi.visual,
	                            mask,
	                            &swa);
	if (!env->window)
	{
		fprintf(stderr, "%s: failed to create window\n", env->progname);
		return 1;
	}
	XChangeProperty(env->display,
	                env->window,
	                XA_WM_NAME,
	                XA_STRING,
	                8,
	                PropModeReplace,
	                (uint8_t*)"xsysmon",
	                7);
	gc_values.graphics_exposures = 0;
	env->gc = XCreateGC(env->display,
	                    env->window,
	                    GCGraphicsExposures,
	                    &gc_values);
	if (!env->gc)
	{
		fprintf(stderr, "%s: failed to create GC\n", env->progname);
		return 1;
	}
	return 0;
}

static int
setup_graphs(struct env *env)
{
	struct graph *graph;
	size_t ncpu;

	if (get_ncpu(env, &ncpu))
		return 1;
	for (size_t i = 0; i < ncpu; ++i)
	{
		graph = graph_alloc(env,
		                    1.0 / ncpu * i,
		                    0.0,
		                    1.0 / ncpu,
		                    0.33,
		                    get_cpun,
		                    0xFFBBEEFF,
		                    0xFF44AAFF,
		                    GRAPH_BORDER_BOTTOM,
		                    (void*)i);
		if (!graph)
			return 1;
		TAILQ_INSERT_TAIL(&env->graphs, graph, chain);
	}
	graph = graph_alloc(env,
	                    0.0,
	                    0.33,
	                    0.5,
	                    0.33,
	                    get_cpu,
	                    0xFFBBEEFF,
	                    0xFF44AAFF,
	                    GRAPH_BORDER_RIGHT | GRAPH_BORDER_TOP | GRAPH_BORDER_BOTTOM,
	                    NULL);
	if (!graph)
		return 1;
	TAILQ_INSERT_TAIL(&env->graphs, graph, chain);
	graph = graph_alloc(env,
	                    0.5,
	                    0.33,
	                    0.5,
	                    0.33,
	                    get_mem,
	                    0xFFCCFFCC,
	                    0xFF00CC00,
	                    GRAPH_BORDER_LEFT | GRAPH_BORDER_TOP | GRAPH_BORDER_BOTTOM,
	                    NULL);
	if (!graph)
		return 1;
	TAILQ_INSERT_TAIL(&env->graphs, graph, chain);
	graph = graph_alloc(env,
	                    0.0,
	                    0.66,
	                    0.5,
	                    0.34,
	                    get_net_in,
	                    0xFFDDCCFF,
	                    0xFF8800FF,
	                    GRAPH_BORDER_RIGHT | GRAPH_BORDER_TOP,
	                    NULL);
	if (!graph)
		return 1;
	TAILQ_INSERT_TAIL(&env->graphs, graph, chain);
	graph = graph_alloc(env,
	                    0.5,
	                    0.66,
	                    0.5,
	                    0.34,
	                    get_net_out,
	                    0xFFFFDDCC,
	                    0xFFEE8800,
	                    GRAPH_BORDER_LEFT | GRAPH_BORDER_TOP,
	                    NULL);
	if (!graph)
		return 1;
	TAILQ_INSERT_TAIL(&env->graphs, graph, chain);
	return 0;
}

static void
usage(const char *progname)
{
	printf("%s [-h] [-i interval]\n", progname);
	printf("-h: show this help\n");
	printf("-i interval: set the refresh interval (in ms)\n");
}

int
main(int argc, char **argv)
{
	struct env env;
	int c;

	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	env.width = 640;
	env.height = 480;
	env.probe_interval = 500;
	TAILQ_INIT(&env.graphs);
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
	if (setup_graphs(&env))
		return EXIT_FAILURE;
	XMapWindow(env.display, env.window);
	XSynchronize(env.display, False);
	uint64_t sleep_time = env.probe_interval;
	if (sleep_time < 10)
		sleep_time = 10;
	uint64_t last_probe = 0;
	if (probe(&env))
		return EXIT_FAILURE;
	while (1)
	{
		XEvent event;
		bool has_exposure = false;
		struct graph *graph;

		while (XPending(env.display))
		{
			XNextEvent(env.display, &event);
			switch (event.type)
			{
				case ConfigureNotify:
					if (event.xconfigure.width == (int)env.width
					 && event.xconfigure.height == (int)env.height)
						break;
					env.width = event.xconfigure.width;
					env.height = event.xconfigure.height;
					TAILQ_FOREACH(graph, &env.graphs, chain)
						graph_resize(&env, graph);
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
			TAILQ_FOREACH(graph, &env.graphs, chain)
				tick(&env, graph);
			last_probe = curtime;
			TAILQ_FOREACH(graph, &env.graphs, chain)
				draw(&env, graph);
		}
		else if (has_exposure)
		{
			TAILQ_FOREACH(graph, &env.graphs, chain)
				draw(&env, graph);
		}
		usleep(1000 * sleep_time);
	}
	return EXIT_SUCCESS;
}
