#include <sys/param.h>
#include <sys/mman.h>
#include <sys/shm.h>

#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>

#include <inttypes.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <dlfcn.h>
#include <time.h>
#include <zlib.h>

#include "libretro.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef AUDIO_PULSEAUDIO
# include <pulse/simple.h>
# include <pulse/error.h>
#endif

typedef void (*retro_init_t)(void);
typedef void (*retro_deinit_t)(void);
typedef void (*retro_reset_t)(void);
typedef unsigned (*retro_api_version_t)(void);
typedef void (*retro_get_system_info_t)(struct retro_system_info *info);
typedef void (*retro_get_system_av_info_t)(struct retro_system_av_info *info);
typedef void (*retro_set_environment_t)(retro_environment_t);
typedef void (*retro_set_video_refresh_t)(retro_video_refresh_t);
typedef void (*retro_set_audio_sample_t)(retro_audio_sample_t);
typedef void (*retro_set_audio_sample_batch_t)(retro_audio_sample_batch_t);
typedef void (*retro_set_input_poll_t)(retro_input_poll_t);
typedef void (*retro_set_input_state_t)(retro_input_state_t);
typedef void (*retro_run_t)(void);
typedef bool (*retro_load_game_t)(const struct retro_game_info *info);
typedef void (*retro_unload_game_t)(void);
typedef unsigned (*retro_get_region_t)(void);
typedef bool (*retro_load_game_special_t)(unsigned type,
                                          const struct retro_game_info *info,
                                          size_t num);
typedef size_t (*retro_serialize_size_t)(void);
typedef bool (*retro_serialize_t)(void *data, size_t size);
typedef bool (*retro_unserialize_t)(const void *data, size_t size);
typedef void *(*retro_get_memory_data_t)(unsigned id);
typedef size_t (*retro_get_memory_size_t)(unsigned id);
typedef void (*retro_cheat_reset_t)(void);
typedef void (*retro_cheat_set_t)(unsigned index, bool enabled,
                                  const char *code);

static void load_memory(const char *progname, const char *path,
                        void *data, size_t size);

static char sysdir[MAXPATHLEN];

struct rom
{
	uint8_t *data;
	size_t size;
	char path[MAXPATHLEN];
	char sram[MAXPATHLEN];
	char rtc[MAXPATHLEN];
};

struct core
{
	void *handle;
	uint8_t *video_buf;
	int16_t *audio_buf;
	void *sram_data;
	size_t sram_size;
	uint32_t sram_crc;
	void *rtc_data;
	size_t rtc_size;
	uint32_t rtc_crc;
	size_t audio_pos;
	struct retro_system_info system_info;
	struct retro_system_av_info system_av_info;
	struct retro_game_info game_info;
	uint8_t keys[(RETRO_DEVICE_ID_JOYPAD_MASK + 7) / 8];
	int16_t cursor_x;
	int16_t cursor_y;
	int16_t cursor_pressed;
	int16_t cursor_oos;
	retro_init_t init;
	retro_deinit_t deinit;
	retro_reset_t reset;
	retro_api_version_t api_version;
	retro_get_system_info_t get_system_info;
	retro_get_system_av_info_t get_system_av_info;
	retro_set_environment_t set_environment;
	retro_set_video_refresh_t set_video_refresh;
	retro_set_audio_sample_t set_audio_sample;
	retro_set_audio_sample_batch_t set_audio_sample_batch;
	retro_set_input_poll_t set_input_poll;
	retro_set_input_state_t set_input_state;
	retro_run_t run;
	retro_load_game_t load_game;
	retro_unload_game_t unload_game;
	retro_get_region_t get_region;
	retro_load_game_special_t load_game_special;
	retro_serialize_size_t serialize_size;
	retro_serialize_t serialize;
	retro_unserialize_t unserialize;
	retro_get_memory_data_t get_memory_data;
	retro_get_memory_size_t get_memory_size;
	retro_cheat_reset_t cheat_reset;
	retro_cheat_set_t cheat_set;
};

struct window
{
	const char *progname;
	Display *display;
	Window window;
	int screen;
	int vsync;
	uint32_t width;
	uint32_t height;
	uint32_t scale;
	XVisualInfo vi;
	Window root;
	GC gc;
	XImage *image;
	XShmSegmentInfo shminfo;
#ifdef AUDIO_EKLAT
	int snd_fd;
#elif defined(AUDIO_PULSEAUDIO)
	pa_simple *pa;
#endif
};

static struct core *g_core;

static uint64_t nanotime(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000000000 + ts.tv_nsec;
}

static void retro_log(enum retro_log_level level, const char *fmt, ...)
{
	switch (level)
	{
		case RETRO_LOG_DEBUG:
			fprintf(stderr, "[DEBUG] ");
			break;
		case RETRO_LOG_INFO:
			fprintf(stderr, "[INFO ] ");
			break;
		case RETRO_LOG_WARN:
			fprintf(stderr, "[WARN ] ");
			break;
		case RETRO_LOG_ERROR:
			fprintf(stderr, "[ERROR] ");
			break;
		default:
			fprintf(stderr, "[UNK  ] ");
			break;
	}
	va_list va;
	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	va_end(va);
}

static bool environment(unsigned cmd, void *data)
{
	switch (cmd)
	{
		case RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS:
		{
			struct retro_input_descriptor *desc = data;
			(void)desc;
			/* XXX */
			return true;
		}
		case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT:
			/* XXX */
			return true;
		case RETRO_ENVIRONMENT_GET_LOG_INTERFACE:
		{
			struct retro_log_callback *logging = data;
			logging->log = retro_log;
			return true;
		}
		case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY:
			*(const char**)data = sysdir;
			return true;
		default:
			return false;
	}
}

static void video_refresh(const void *data, unsigned width, unsigned height,
                          size_t src_pitch)
{
	if (width != g_core->system_av_info.geometry.base_width
	 || height != g_core->system_av_info.geometry.base_height)
		return;
	uint8_t *src = (uint8_t*)data;
	uint8_t *dst = g_core->video_buf;
	size_t dst_pitch = g_core->system_av_info.geometry.base_width * 4;
	size_t bytes = g_core->system_av_info.geometry.base_width * 4;
	for (size_t y = 0; y < height; ++y)
	{
		memcpy(dst, src, bytes);
		src += src_pitch;
		dst += dst_pitch;
	}
}

static void audio_sample(int16_t left, int16_t right)
{
	if (g_core->audio_pos >= g_core->system_av_info.timing.sample_rate)
		return;
	g_core->audio_buf[g_core->audio_pos * 2 + 0] = left;
	g_core->audio_buf[g_core->audio_pos * 2 + 1] = right;
	g_core->audio_pos++;
}

static size_t audio_sample_batch(const int16_t *data, size_t frames)
{
	if (g_core->audio_pos + frames >= g_core->system_av_info.timing.sample_rate)
	{
		/* XXX don't truncate ? */
		frames = g_core->system_av_info.timing.sample_rate - g_core->audio_pos;
		if (!frames)
			return 0;
	}
	memcpy(&g_core->audio_buf[g_core->audio_pos * 2], data, frames * 4);
	g_core->audio_pos += frames;
	return frames;
}

static void input_poll(void)
{
	/* XXX */
}

static int16_t input_state(unsigned port, unsigned device, unsigned index,
                           unsigned id)
{
	(void)port;
	(void)index;
	switch (device)
	{
		case RETRO_DEVICE_JOYPAD:
			if (id >= RETRO_DEVICE_ID_JOYPAD_MASK)
				return 0;
			return !!(g_core->keys[id / 8] & (1 << (id % 8)));
		case RETRO_DEVICE_POINTER:
			switch (id)
			{
				case RETRO_DEVICE_ID_POINTER_X:
					return g_core->cursor_x;
				case RETRO_DEVICE_ID_POINTER_Y:
					return g_core->cursor_y;
				case RETRO_DEVICE_ID_POINTER_PRESSED:
					return g_core->cursor_pressed && !g_core->cursor_oos;
			}
			break;
	}
	return 0;
}

static int read_gz_rom(const char *progname, struct rom *rom)
{
	gzFile gzfile;

	gzfile = gzopen(rom->path, "rb");
	if (!gzfile)
	{
		fprintf(stderr, "%s: gzopen: %s\n", progname, strerror(errno));
		return 1;
	}
	while (1)
	{
		uint8_t *newdata = realloc(rom->data, rom->size + 1024 * 1024);
		if (!newdata)
		{
			fprintf(stderr, "%s: malloc: %s\n", progname,
			        strerror(errno));
			goto err;
		}
		rom->data = newdata;
		int rd = gzread(gzfile, &rom->data[rom->size], 1024 * 1024);
		if (rd < 0)
		{
			fprintf(stderr, "%s: gzread failed\n", progname);
			goto err;
		}
		if (!rd)
			break;
		rom->size += rd;
	}
	gzclose(gzfile);
	return 0;

err:
	gzclose(gzfile);
	free(rom->data);
	return 1;
}

static int read_rom(const char *progname, const char *file,
                    struct rom *rom)
{
	snprintf(rom->path, sizeof(rom->path), "%s", file);
	snprintf(rom->sram, sizeof(rom->sram), "%s.sram.bin", file);
	snprintf(rom->rtc, sizeof(rom->rtc), "%s.rtc.bin", file);
	rom->data = NULL;
	rom->size = 0;
	if (strlen(file) >= 3 && !strcmp(&file[strlen(file) - 3], ".gz"))
		return read_gz_rom(progname, rom);
	FILE *fp = fopen(file, "r");
	if (!fp)
	{
		fprintf(stderr, "%s: open: %s\n", progname, strerror(errno));
		return 1;
	}
	while (1)
	{
		uint8_t *newdata = realloc(rom->data, rom->size + 1024 * 1024);
		if (!newdata)
		{
			fprintf(stderr, "%s: malloc: %s\n", progname, strerror(errno));
			goto err;
		}
		rom->data = newdata;
		size_t ret = fread(&rom->data[rom->size], 1, 1024 * 1024, fp);
		if (ferror(fp))
		{
			fprintf(stderr, "%s: read: %s\n", progname, strerror(errno));
			goto err;
		}
		rom->size += ret;
		if (feof(fp))
			break;
	}
	fclose(fp);
	return 0;

err:
	fclose(fp);
	free(rom->data);
	return 1;
}

static int load_core(const char *progname, struct core *core,
                     const char *file, const struct rom *rom)
{
	memset(core, 0, sizeof(*core));
	core->handle = dlopen(file, RTLD_LAZY);
	if (!core->handle)
	{
		fprintf(stderr, "%s: failed to open core: %s\n", progname, dlerror());
		return 1;
	}
#define LOAD_SYM(name) \
do \
{ \
	core->name = dlsym(core->handle, "retro_" #name); \
	if (!core->name) \
	{ \
		fprintf(stderr, "%s: dlsym(" #name "): %s\n", progname, dlerror()); \
		return 1; \
	} \
} \
while (0)

	LOAD_SYM(api_version);
	if (core->api_version() != RETRO_API_VERSION)
	{
		fprintf(stderr, "%s: invalid libretro API version\n", progname);
		return 1;
	}
	LOAD_SYM(init);
	LOAD_SYM(deinit);
	LOAD_SYM(reset);
	LOAD_SYM(get_system_info);
	LOAD_SYM(get_system_av_info);
	LOAD_SYM(set_environment);
	LOAD_SYM(set_video_refresh);
	LOAD_SYM(set_audio_sample);
	LOAD_SYM(set_audio_sample_batch);
	LOAD_SYM(set_input_poll);
	LOAD_SYM(set_input_state);
	LOAD_SYM(run);
	LOAD_SYM(load_game);
	LOAD_SYM(unload_game);
	LOAD_SYM(get_region);
	LOAD_SYM(load_game_special);
	LOAD_SYM(serialize_size);
	LOAD_SYM(serialize);
	LOAD_SYM(unserialize);
	LOAD_SYM(get_memory_data);
	LOAD_SYM(get_memory_size);
	LOAD_SYM(cheat_reset);
	LOAD_SYM(cheat_set);

#undef LOAD_SYM

	core->set_environment(&environment);
	core->init();

	core->set_video_refresh(&video_refresh);
	core->set_audio_sample(&audio_sample);
	core->set_audio_sample_batch(&audio_sample_batch);
	core->set_input_poll(&input_poll);
	core->set_input_state(&input_state);

	core->game_info.path = rom->path;
	core->game_info.data = rom->data;
	core->game_info.size = rom->size;
	core->game_info.meta = NULL;
	if (!core->load_game(&core->game_info))
	{
		fprintf(stderr, "%s: failed to load game\n", progname);
		return EXIT_FAILURE;
	}

	core->get_system_info(&core->system_info);
	core->get_system_av_info(&core->system_av_info);

	core->video_buf = malloc(core->system_av_info.geometry.base_width
	                       * core->system_av_info.geometry.base_height
	                       * 4);
	if (!core->video_buf)
	{
		fprintf(stderr, "%s: malloc: %s\n", progname, strerror(errno));
		return EXIT_FAILURE;
	}

	core->audio_buf = malloc(core->system_av_info.timing.sample_rate * sizeof(*core->audio_buf) * 2);
	if (!core->audio_buf)
	{
		fprintf(stderr, "%s: malloc: %s\n", progname, strerror(errno));
		return EXIT_FAILURE;
	}

	core->sram_data = core->get_memory_data(RETRO_MEMORY_SAVE_RAM);
	core->sram_size = core->get_memory_size(RETRO_MEMORY_SAVE_RAM);
	core->rtc_data = core->get_memory_data(RETRO_MEMORY_RTC);
	core->rtc_size = core->get_memory_size(RETRO_MEMORY_RTC);
	return 0;
}

static int get_key_id(KeySym sym)
{
	switch (sym)
	{
		case XK_Left:
			return RETRO_DEVICE_ID_JOYPAD_LEFT;
		case XK_Right:
			return RETRO_DEVICE_ID_JOYPAD_RIGHT;
		case XK_Up:
			return RETRO_DEVICE_ID_JOYPAD_UP;
		case XK_Down:
			return RETRO_DEVICE_ID_JOYPAD_DOWN;
		case XK_x:
			return RETRO_DEVICE_ID_JOYPAD_A;
		case XK_z:
			return RETRO_DEVICE_ID_JOYPAD_B;
		case XK_s:
			return RETRO_DEVICE_ID_JOYPAD_X;
		case XK_a:
			return RETRO_DEVICE_ID_JOYPAD_Y;
		case XK_w:
			return RETRO_DEVICE_ID_JOYPAD_R;
		case XK_q:
			return RETRO_DEVICE_ID_JOYPAD_L;
		case XK_Return:
			return RETRO_DEVICE_ID_JOYPAD_START;
		case XK_Shift_R:
			return RETRO_DEVICE_ID_JOYPAD_SELECT;
	}
	return -1;
}

static int create_shmimg(struct window *window)
{
	uint32_t width = g_core->system_av_info.geometry.base_width * window->scale;
	uint32_t height = g_core->system_av_info.geometry.base_height * window->scale;
	window->image = XShmCreateImage(window->display, window->vi.visual, 24,
	                                ZPixmap, NULL, &window->shminfo,
	                                (width + 3) & ~3, height);
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
	if (shmctl(window->shminfo.shmid, IPC_RMID, NULL) == -1)
	{
		fprintf(stderr, "%s: shmctl: %s\n", window->progname,
		        strerror(errno));
		return 1;
	}
	return 0;
}

static void handle_key_press(struct window *window, XKeyEvent *event)
{
	KeySym sym = XLookupKeysym(event, 0);
	if (sym == XK_space)
	{
		window->vsync = !window->vsync;
		return;
	}
	int id = get_key_id(sym);
	if (id < 0)
		return;
	g_core->keys[id / 8] |= 1 << (id % 8);
}

static void handle_key_release(struct window *window, XKeyEvent *event)
{
	(void)window;
	KeySym sym = XLookupKeysym(event, 0);
	int id = get_key_id(sym);
	if (id < 0)
		return;
	g_core->keys[id / 8] &= ~(1 << (id % 8));
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
	uint32_t width_scale = window->width / g_core->system_av_info.geometry.base_width;
	uint32_t height_scale = window->height / g_core->system_av_info.geometry.base_height;
	uint32_t scale;
	if (!width_scale || !height_scale)
		scale = 1;
	else if (width_scale > height_scale)
		scale = height_scale;
	else
		scale = width_scale;
	if (scale == window->scale)
		return;
	window->scale = scale;
	XShmDetach(window->display, &window->shminfo);
	shmdt(window->image->data);
	window->image->data = NULL;
	XDestroyImage(window->image);
	if (create_shmimg(window))
		exit(EXIT_FAILURE);
}

static void handle_pointer_motion(struct window *window, XMotionEvent *event)
{
	(void)window;
	uint32_t width = g_core->system_av_info.geometry.base_width * window->scale;
	uint32_t height = g_core->system_av_info.geometry.base_height * window->scale;
	uint32_t left = (window->width - width) / 2;
	uint32_t top = (window->height - height) / 2;
	uint32_t x = event->x < 0 ? 0 : event->x;
	uint32_t y = event->y < 0 ? 0 : event->y;
	g_core->cursor_oos = false;
	if (x < left)
	{
		g_core->cursor_x = -0x7FFF;
		g_core->cursor_oos = true;
	}
	else if (x >= left + width)
	{
		g_core->cursor_x = 0x7FFF;
		g_core->cursor_oos = true;
	}
	else
	{
		g_core->cursor_x = (((x - left) / (float)width) - .5) * 2 * 0x7FFF;
	}
	if (y < top)
	{
		g_core->cursor_y = -0x7FFF;
		g_core->cursor_oos = true;
	}
	else if (y >= top + height)
	{
		g_core->cursor_y = 0x7FFF;
		g_core->cursor_oos = true;
	}
	else
	{
		g_core->cursor_y = (((y - top) / (float)height) - .5) * 2 * 0x7FFF;
	}
}

static void handle_button_press(struct window *window, XButtonEvent *event)
{
	(void)window;
	if (event->button == Button1)
		g_core->cursor_pressed = 1;
}

static void handle_button_release(struct window *window, XButtonEvent *event)
{
	(void)window;
	if (event->button == Button1)
		g_core->cursor_pressed = 0;
}

static void handle_events(struct window *window)
{
	while (XPending(window->display))
	{
		XEvent event;
		XNextEvent(window->display, &event);
		switch (event.type)
		{
			case KeyPress:
				handle_key_press(window, &event.xkey);
				break;
			case KeyRelease:
				handle_key_release(window, &event.xkey);
				break;
			case MotionNotify:
				handle_pointer_motion(window, &event.xmotion);
				break;
			case ButtonPress:
				handle_button_press(window, &event.xbutton);
				break;
			case ButtonRelease:
				handle_button_release(window, &event.xbutton);
				break;
			case ConfigureNotify:
				handle_configure(window, &event.xconfigure);
				break;
		}
	}
}

static int setup_window(const char *progname, struct window *window)
{
	window->vsync = 1;
	window->progname = progname;
	window->width = g_core->system_av_info.geometry.base_width;
	window->height = g_core->system_av_info.geometry.base_height;
	window->scale = 1;
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
	swa.event_mask = KeyPressMask
	               | KeyReleaseMask
	               | StructureNotifyMask
	               | PointerMotionMask
	               | ButtonPressMask
	               | ButtonReleaseMask;
	swa.bit_gravity = CenterGravity;
	window->window = XCreateWindow(window->display, window->root, 0, 0,
	                               window->width, window->height, 0,
	                               window->vi.depth,
	                               InputOutput, window->vi.visual,
	                               CWEventMask | CWBitGravity, &swa);
	char name[256];
	snprintf(name, sizeof(name), "retrosef - %s %s",
	         g_core->system_info.library_name,
	         g_core->system_info.library_version);
	XChangeProperty(window->display, window->window, XA_WM_NAME, XA_STRING,
	                8, PropModeReplace, (uint8_t*)name, strlen(name));
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
#ifdef AUDIO_EKLAT
	window->snd_fd = open("/dev/snd0", O_WRONLY);
	if (window->snd_fd == -1)
		fprintf(stderr, "%s: open: %s\n", progname, strerror(errno));
#elif defined(AUDIO_PULSEAUDIO)
	static const pa_sample_spec sample_spec =
	{
		.format = PA_SAMPLE_S16LE,
		.rate = 48000,
		.channels = 2,
	};
	int error;
	window->pa = pa_simple_new(NULL, progname, PA_STREAM_PLAYBACK,
	                           NULL, "retrosef", &sample_spec,
	                           NULL, NULL, &error);
	if (!window->pa)
		fprintf(stderr, "%s: pa_simple_new: %s\n", progname,
		        pa_strerror(error));
#endif
	return 0;
}

#define COPY_SCALED_X(scale) \
static void copy_scaled_##scale(struct window *window) \
{ \
	uint32_t *dst = (uint32_t*)window->image->data; \
	uint32_t *src = (uint32_t*)g_core->video_buf; \
	for (size_t y = 0; y < g_core->system_av_info.geometry.base_height; ++y) \
	{ \
		for (size_t yy = 0; yy < scale; ++yy) \
		{ \
			uint32_t *line_src = src; \
			uint32_t *line_dst = dst; \
			for (size_t x = 0; x < g_core->system_av_info.geometry.base_width; ++x) \
			{ \
				for (size_t xx = 0; xx < scale; ++xx) \
				{ \
					*line_dst = *line_src; \
					line_dst++; \
				} \
				line_src++; \
			} \
			dst += window->image->bytes_per_line / 4; \
		} \
		src += g_core->system_av_info.geometry.base_width; \
	} \
}

COPY_SCALED_X(2);
COPY_SCALED_X(3);
COPY_SCALED_X(4);
COPY_SCALED_X(5);
COPY_SCALED_X(6);
COPY_SCALED_X(7);
COPY_SCALED_X(8);
COPY_SCALED_X(9);
COPY_SCALED_X(10);

static void copy_scaled(struct window *window)
{
	uint32_t *dst = (uint32_t*)window->image->data;
	uint32_t *src = (uint32_t*)g_core->video_buf;
	for (size_t y = 0; y < g_core->system_av_info.geometry.base_height; ++y)
	{
		for (size_t yy = 0; yy < window->scale; ++yy)
		{
			uint32_t *line_src = src;
			uint32_t *line_dst = dst;
			for (size_t x = 0; x < g_core->system_av_info.geometry.base_width; ++x)
			{
				for (size_t xx = 0; xx < window->scale; ++xx)
				{
					*line_dst = *line_src;
					line_dst++;
				}
				line_src++;
			}
			dst += window->image->bytes_per_line / 4;
		}
		src += g_core->system_av_info.geometry.base_width;
	}
}

static void copy_unscaled(struct window *window)
{
	uint8_t *dst = (uint8_t*)window->image->data;
	uint8_t *src = g_core->video_buf;
	for (size_t y = 0; y < g_core->system_av_info.geometry.base_height; ++y)
	{
		memcpy(dst, src, g_core->system_av_info.geometry.base_width * 4);
		dst += window->image->bytes_per_line;
		src += g_core->system_av_info.geometry.base_width * 4;
	}
}

static void load_memory(const char *progname, const char *path,
                        void *data, size_t size)
{
	FILE *fp = fopen(path, "rb");
	if (!fp)
	{
		if (errno != ENOENT)
			fprintf(stderr, "%s: fopen(%s): %s\n", progname, path,
			        strerror(errno));
		return;
	}
	if (fread(data, 1, size, fp) != size)
		fprintf(stderr, "%s: fread(%s) partial\n", progname, path);
	fclose(fp);
}

static void save_memory(const char *progname, const char *path,
                        const void *data, size_t size)
{
	FILE *fp = fopen(path, "wb");
	if (!fp)
	{
		fprintf(stderr, "%s: fopen(%s): %s\n", progname, path,
		        strerror(errno));
		return;
	}
	/* XXX use commit file */
	if (fwrite(data, 1, size, fp) != size)
		fprintf(stderr, "%s: fwrite(%s): %s\n", progname, path,
		        strerror(errno));
	fclose(fp);
}

static void test_save_memory(const char *progname, const char *path,
                             const void *data, size_t size,
                             uint32_t *crcp)
{
	if (!data || !size)
		return;
	/* XXX use something better than crc ? */
	uint32_t crc = crc32_z(crc32_z(0, NULL, 0), data, size);
	if (crc == *crcp)
		return;
	save_memory(progname, path, data, size);
	*crcp = crc;
}

int main(int argc, char **argv)
{
	if (argc < 3)
	{
		fprintf(stderr, "%s: missing operand\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (argc > 3)
	{
		fprintf(stderr, "%s: extra operand\n", argv[0]);
		return EXIT_FAILURE;
	}

#ifdef __eklat__
	snprintf(sysdir, sizeof(sysdir), "/lib");
#else
	snprintf(sysdir, sizeof(sysdir), "%s/.config/retroarch/system", getenv("HOME"));
#endif

	struct window window;
	struct core core;
	struct rom rom;

	g_core = &core;
	if (read_rom(argv[0], argv[2], &rom))
		return EXIT_FAILURE;
	if (load_core(argv[0], &core, argv[1], &rom))
		return EXIT_FAILURE;
	if (setup_window(argv[0], &window))
		return EXIT_FAILURE;

	if (core.sram_data && core.sram_size)
		load_memory(argv[0], rom.sram, core.sram_data, core.sram_size);
	if (core.rtc_data && core.rtc_size)
		load_memory(argv[0], rom.rtc, core.rtc_data, core.rtc_size);

	uint64_t last_frame = nanotime();
	uint64_t frame_duration = 1000000000 / core.system_av_info.timing.fps;
	uint64_t last_second = nanotime();
	uint64_t fps = 0;
	while (1)
	{
		handle_events(&window);
		core.run();
		switch (window.scale)
		{
			case 1:
				copy_unscaled(&window);
				break;
			case 2:
				copy_scaled_2(&window);
				break;
			case 3:
				copy_scaled_3(&window);
				break;
			case 4:
				copy_scaled_4(&window);
				break;
			case 5:
				copy_scaled_5(&window);
				break;
			case 6:
				copy_scaled_6(&window);
				break;
			case 7:
				copy_scaled_7(&window);
				break;
			case 8:
				copy_scaled_8(&window);
				break;
			case 9:
				copy_scaled_9(&window);
				break;
			case 10:
				copy_scaled_10(&window);
				break;
			default:
				copy_scaled(&window);
				break;
		}
		uint32_t dst_width = core.system_av_info.geometry.base_width * window.scale;
		uint32_t dst_height = core.system_av_info.geometry.base_height * window.scale;
		uint32_t dst_x = (window.width - dst_width) / 2;
		uint32_t dst_y = (window.height - dst_height) / 2;
		XShmPutImage(window.display, window.window, window.gc,
		             window.image, 0, 0, dst_x, dst_y,
		             dst_width, dst_height, False);
		XFlush(window.display);
#ifdef AUDIO_EKLAT
		if (window.snd_fd != -1)
			write(window.snd_fd, core.audio_buf, g_core->audio_pos * sizeof(*g_core->audio_buf) * 2);
#elif defined(AUDIO_PULSEAUDIO)
		if (window.pa)
			pa_simple_write(window.pa, core.audio_buf, g_core->audio_pos * sizeof(*g_core->audio_buf) * 2, NULL);
#endif
		g_core->audio_pos = 0;
		uint64_t current = nanotime();
		if (window.vsync)
		{
			if (current < last_frame + frame_duration)
			{
				uint64_t delta = frame_duration - (current - last_frame);
				struct timespec ts;
				ts.tv_sec = delta / 1000000000;
				ts.tv_nsec = delta % 1000000000;
				nanosleep(&ts, NULL);
			}
			last_frame += frame_duration;
		}
		else
		{
			last_frame = current;
		}
		fps++;
		if (current - last_second >= 1000000000)
		{
			test_save_memory(argv[0], rom.sram, core.sram_data,
			                 core.sram_size, &core.sram_crc);
			test_save_memory(argv[0], rom.rtc, core.rtc_data,
			                 core.rtc_size, &core.rtc_crc);
#if 0
			printf("fps: %" PRIu64 "\n", fps);
#endif
			char name[256];
			snprintf(name, sizeof(name), "retrosef - %s %s - %" PRIu64 " fps",
			         g_core->system_info.library_name,
			         g_core->system_info.library_version,
			         fps);
			XChangeProperty(window.display, window.window, XA_WM_NAME, XA_STRING,
			                8, PropModeReplace, (uint8_t*)name, strlen(name));
			last_second = current;
			fps = 0;
		}
	}
	return EXIT_SUCCESS;
}
