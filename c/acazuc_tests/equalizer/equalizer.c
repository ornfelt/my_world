#include <GLFW/glfw3.h>

#include <sys/queue.h>

#include <inttypes.h>
#include <stdbool.h>
#include <complex.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#define RENDER_SIZE 512

#define BUF_WIDTH (RENDER_SIZE * 8)

#define FILTER_SIZE 20

struct filter;

typedef void (*update_fn_t)(struct filter *filter);
typedef void (*filter_fn_t)(struct filter *filter, float *samples, size_t size);

struct filter
{
	update_fn_t update;
	filter_fn_t filter;
	uint32_t sampling;
	float frequency;
	float strength;
	float width;
	TAILQ_ENTRY(filter) chain;
};

struct bilinear_filter
{
	struct filter filter;
	float back_samples[3];
	float prev_samples[2];
	float a0;
	float a1;
	float a2;
	float b0;
	float b1;
	float b2;
};

struct delay_filter
{
	struct filter filter;
	float *buf;
	size_t len;
	size_t pos;
};

struct fft_ctx
{
	size_t size;
	complex float *factors;
};

static int offset = 0;

static complex float buf[4][BUF_WIDTH];

TAILQ_HEAD(, filter) filters = TAILQ_HEAD_INITIALIZER(filters);

struct filter *clicked_filter;

FILE *fp;
uint64_t started;
uint64_t last_read;
bool paused;
float x_axis_base = 50;

struct fft_ctx fft_ctx;

static uint64_t nanotime(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000000000 + ts.tv_nsec;
}

int fft_ctx_init(struct fft_ctx *ctx, size_t size)
{
	ctx->size = size;
	ctx->factors = malloc(sizeof(*ctx->factors) * size);
	if (!ctx->factors)
		return 1;
	float base = M_PI / size;
	for (size_t i = 0; i < size; ++i)
	{
		float p = base * i;
		ctx->factors[i] = cosf(p) + I * sinf(p);
	}
	return 0;
}

void fft_rec(const struct fft_ctx *ctx, complex float *in, complex float *out, size_t step)
{
	if (step >= ctx->size)
		return;
	fft_rec(ctx, out       , in       , step * 2);
	fft_rec(ctx, out + step, in + step, step * 2);
	complex float *ina = &in[0];
	complex float *inb = &in[ctx->size / 2];
	complex float *out_step = &out[step];
	for (size_t i = 0; i < ctx->size; i += step * 2)
	{
		complex float t = ctx->factors[i] * out_step[i];
		ina[i / 2] = out[i] + t;
		inb[i / 2] = out[i] - t;
	}
}

void fft(const struct fft_ctx *ctx, complex float *out, const complex float *in, complex float *tmp)
{
	memcpy(tmp, in, sizeof(*tmp) * ctx->size);
	memcpy(out, in, sizeof(*out) * ctx->size);
	fft_rec(ctx, out, tmp, 1);
	float factor = 100.0f / ctx->size;
	for (size_t i = 0; i < ctx->size; ++i)
		out[i] *= factor;
}

void bilinear_filter(struct filter *filter, float *samples, size_t size)
{
	struct bilinear_filter *bili = (struct bilinear_filter*)filter;
	float *dst = samples;
	for (ssize_t i = 0; i < size; ++i)
	{
		bili->back_samples[2] = bili->back_samples[1];
		bili->back_samples[1] = bili->back_samples[0];
		bili->back_samples[0] = *dst;
		float pcm =
			(bili->b0 * bili->back_samples[0]) +
			(bili->b1 * bili->back_samples[1]) +
			(bili->b2 * bili->back_samples[2]) -
			(bili->a1 * bili->prev_samples[0]) -
			(bili->a2 * bili->prev_samples[1]);
		*dst = *dst * (1 - filter->strength) + pcm * filter->strength;
		bili->prev_samples[1] = bili->prev_samples[0];
		bili->prev_samples[0] = pcm;
		++dst;
	}
}

struct filter *bilinear_new(update_fn_t update_fn, uint32_t sampling,
                            uint32_t cutoff, float resonance)
{
	struct bilinear_filter *filter = calloc(1, sizeof(*filter));
	filter->filter.update = update_fn;
	filter->filter.filter = bilinear_filter;
	filter->filter.sampling = sampling;
	filter->filter.frequency = cutoff;
	filter->filter.width = resonance;
	filter->filter.strength = 1;
	filter->filter.update(&filter->filter);
	return &filter->filter;
}

void lowpass_update(struct filter *filter)
{
	struct bilinear_filter *bili = (struct bilinear_filter*)filter;
	float w0 = 2 * M_PI * filter->frequency / filter->sampling;
	float alpha = sinf(w0) / (2 * filter->width);
	float cw0 = cosf(w0);
	bili->a0 = 1 + alpha;
	bili->a1 = (-2 * cw0) / bili->a0;
	bili->a2 = (1 - alpha) / bili->a0;
	bili->b0 = ((1 - cw0) / 2) / bili->a0;
	bili->b1 = (1 - cw0) / bili->a0;
	bili->b2 = ((1 - cw0) / 2) / bili->a0;
}

struct filter *lowpass_new(uint32_t sampling, uint32_t cutoff, float resonance)
{
	return bilinear_new(lowpass_update, sampling, cutoff, resonance);
}

void highpass_update(struct filter *filter)
{
	struct bilinear_filter *bili = (struct bilinear_filter*)filter;
	float w0 = 2 * M_PI * filter->frequency / filter->sampling;
	float alpha = sinf(w0) / (2 * filter->width);
	float cw0 = cosf(w0);
	bili->a0 = 1 + alpha;
	bili->a1 = (-2 * cw0) / bili->a0;
	bili->a2 = (1 - alpha) / bili->a0;
	bili->b0 = ((1 + cw0) / 2) / bili->a0;
	bili->b1 = (-1 - cw0) / bili->a0;
	bili->b2 = ((1 + cw0) / 2) / bili->a0;
}

struct filter *highpass_new(uint32_t sampling, uint32_t cutoff, float resonance)
{
	return bilinear_new(highpass_update, sampling, cutoff, resonance);
}

void bandpass_update(struct filter *filter)
{
	struct bilinear_filter *bili = (struct bilinear_filter*)filter;
	float w0 = 2 * M_PI * filter->frequency / filter->sampling;
	float sw0 = sinf(w0);
	float alpha = sw0 * sinhf((log(2.0) / 2.0) * filter->width * (w0 / sw0));
	float cw0 = cosf(w0);
	bili->a0 = 1 + alpha;
	bili->a1 = (-2 * cw0) / bili->a0;
	bili->a2 = (1 - alpha) / bili->a0;
	bili->b0 = alpha / bili->a0;
	bili->b1 = 0;
	bili->b2 = -alpha / bili->a0;
}

struct filter *bandpass_new(uint32_t sampling, uint32_t frequency, float width)
{
	return bilinear_new(bandpass_update, sampling, frequency, width);
}

void allpass_update(struct filter *filter)
{
	struct bilinear_filter *bili = (struct bilinear_filter*)filter;
	float w0 = 2 * M_PI * filter->frequency / filter->sampling;
	float sw0 = sinf(w0);
	float alpha = sw0 * sinhf((log(2.0) / 2.0) * filter->width * (w0 / sw0));
	float cw0 = cosf(w0);
	bili->a0 = 1 + alpha;
	bili->a1 = (-2 * cw0) / bili->a0;
	bili->a2 = (1 - alpha) / bili->a0;
	bili->b0 = (1 - alpha) / bili->a0;
	bili->b1 = (-2 * cw0) / bili->a0;
	bili->b2 = (1 + alpha) / bili->a0;
}

struct filter *allpass_new(uint32_t sampling, uint32_t frequency, float width)
{
	return bilinear_new(allpass_update, sampling, frequency, width);
}

void notch_update(struct filter *filter)
{
	struct bilinear_filter *bili = (struct bilinear_filter*)filter;
	float w0 = 2 * M_PI * filter->frequency / filter->sampling;
	float sw0 = sinf(w0);
	float alpha = sw0 * sinhf((log(2.0) / 2.0) * filter->width * (w0 / sw0));
	float cw0 = cosf(w0);
	bili->a0 = 1 + alpha;
	bili->a1 = (-2 * cw0) / bili->a0;
	bili->a2 = (1 - alpha) / bili->a0;
	bili->b0 = 1 / bili->a0;
	bili->b1 = (-2 * cw0) / bili->a0;
	bili->b2 = 1 / bili->a0;
}

struct filter *notch_new(uint32_t sampling, uint32_t frequency, float width)
{
	return bilinear_new(notch_update, sampling, frequency, width);
}

void delay_update(struct filter *filter)
{
	struct delay_filter *delay = (struct delay_filter*)filter;
	size_t len = filter->width * 44100 / 5; /* ranges from 0 to 1 second */
	delay->buf = realloc(delay->buf, sizeof(float) * len + 1);
	if (len > delay->len)
	{
		for (size_t i = delay->len; i < len; ++i)
			delay->buf[i] = 0;
	}
	delay->len = len;
}

void delay_filter(struct filter *filter, float *samples, size_t size)
{
	struct delay_filter *delay = (struct delay_filter*)filter;
	float *dst = samples;
	for (size_t i = 0; i < size; ++i)
	{
		if (delay->pos >= delay->len)
			delay->pos = 0;
		float *bufp = &delay->buf[delay->pos];
		float tmp = *bufp;
		*bufp = *dst;
		*dst = *dst * (1 - filter->strength) + tmp * filter->strength;
		dst++;
		delay->pos++;
	}
}

struct filter *delay_new(void)
{
	struct delay_filter *filter = calloc(1, sizeof(*filter));
	filter->filter.update = delay_update;
	filter->filter.filter = delay_filter;
	delay_update(&filter->filter);
	return &filter->filter;
}

float lognf(float v, float b)
{
	return logf(v) / logf(b);
}

float expnf(float v, float b)
{
	return expf(v * logf(b));
}

size_t coord_to_freq(size_t coord, size_t size)
{
	return (expnf(1 + coord / (float)RENDER_SIZE, x_axis_base) - x_axis_base) / (expnf(2, x_axis_base) - x_axis_base) * size;
}

size_t freq_to_coord(size_t freq, size_t size)
{
	return (lognf(freq / (float)size * (expnf(2, x_axis_base) - x_axis_base) + x_axis_base, x_axis_base) - 1) * RENDER_SIZE;
}

void draw_buf(const complex float *buf, ssize_t size, ssize_t avg_width, int loga)
{
	uint32_t prev_min = 0;
	uint32_t prev_max = 0;
	for (ssize_t i = 0; i < RENDER_SIZE; ++i)
	{
		float sum = 0;
		float count = 0;
		ssize_t first_idx = i - avg_width / 2;
		ssize_t last_idx = i + avg_width / 2;
		if (first_idx < 0)
			first_idx = 0;
		if (last_idx >= size)
			last_idx = size - 1;
		size_t first = coord_to_freq(first_idx, size);
		size_t last = coord_to_freq(last_idx, size);
		if (first < 0)
			first = 0;
		if (last <= first)
			last = first + 1;
		for (size_t n = first; n < last; ++n)
		{
			complex float c = buf[n];
			float v = i ? sqrtf(creal(c) * creal(c) + cimag(c) * cimag(c)) : 0;
			float f = avg_width ? 1 - fabsf(((n - first) / (float)(last - first) - .5) * 2.0) : 1;
			sum += v * f;
			count += f;
		}
		sum /= count;
		if (loga)
			sum = (log10f(10 + sum) - 1) * 10;
		if (sum < 0)
			sum = 0;
		if (sum > 1)
			sum = 1;
		uint32_t max = RENDER_SIZE - RENDER_SIZE * sum;
		uint32_t min = max;
		if (i)
		{
			if (prev_max < min)
				min = prev_max;
			if (prev_min > max)
				max = prev_min;
		}
		prev_min = min;
		prev_max = max;
		if (i > 1)
		{
			glVertex2f(i, min);
			glVertex2f(i, max + 1);
		}
	}
}

void get_filter_pos(struct filter *filter, float *x, float *y)
{
	*x = freq_to_coord(filter->frequency, BUF_WIDTH / 2);
	*y = RENDER_SIZE / 2 - (RENDER_SIZE / 2) * filter->strength;
}

void draw_filter(struct filter *filter)
{
	glColor4f(0.5 + 0.5 * (filter == clicked_filter), 0.5 - 0.5 * (filter == clicked_filter), 0.5 + 0.5 * filter->width / 5, 1);
	glBegin(GL_LINES);
	for (ssize_t i = 0; i <= FILTER_SIZE * 2; ++i)
	{
		float s = sinf(acos((i / (float)(FILTER_SIZE * 2) - 0.5) * 2)) * FILTER_SIZE;
		float x, y;
		get_filter_pos(filter, &x, &y);
		glVertex2f(x - s, y - FILTER_SIZE + i);
		glVertex2f(x + s, y - FILTER_SIZE + i);
	}
	glEnd();
}

void get_samples(float *samples)
{
	if (fp)
	{
		uint64_t cur = nanotime();
		uint64_t interval = 1000000000ull * BUF_WIDTH / 44100;
		uint64_t next = last_read + interval;
		if (cur < next)
		{
			uint64_t delta = next - cur;
			struct timespec ts;
			ts.tv_sec = delta / 1000000000;
			ts.tv_nsec = delta % 1000000000;
			nanosleep(&ts, NULL);
		}
		last_read = next;
		float tmp[BUF_WIDTH * 2];
		size_t rd = fread(tmp, sizeof(*tmp), sizeof(tmp) / sizeof(*tmp), fp);
		for (size_t i = rd; i < sizeof(tmp) / sizeof(*tmp); ++i)
			tmp[i] = 0;
		for (size_t i = 0; i < BUF_WIDTH; ++i)
			samples[i] = tmp[i * 2];
		return;
	}
#if 1
	for (size_t i = 0; i < BUF_WIDTH; ++i)
		samples[i] = rand() / (float)RAND_MAX;
#endif
#if 0
	size_t nfreq = 0;
	for (size_t i = 0; i < BUF_WIDTH; ++i)
		samples[i] = 0;
	for (size_t freq = 1; freq < BUF_WIDTH / 2; freq = ceil(freq * 1.5))
	{
		for (size_t i = 0; i < BUF_WIDTH; ++i)
			samples[i] += .75 + .25 * cosf(i / (float)BUF_WIDTH * M_PI * 2 * freq);
		nfreq++;
	}
	for (size_t i = 0; i < BUF_WIDTH; ++i)
		samples[i] /= nfreq;
#endif
}

void draw(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	static float samples[BUF_WIDTH];
	if (!paused)
		get_samples(samples);
	for (size_t i = 0; i < BUF_WIDTH; ++i)
		buf[0][i] = samples[i];
	fft(&fft_ctx, buf[3], buf[0], buf[2]);
	glBegin(GL_LINES);
	glColor4f(1.0, 1.0, 1.0, 0.2);
	draw_buf(buf[0], BUF_WIDTH, 0, 0);
	if (!paused)
	{
		struct filter *filter;
		TAILQ_FOREACH(filter, &filters, chain)
			filter->filter(filter, samples, BUF_WIDTH);
	}
	if (fp)
		fwrite(samples, sizeof(samples), 1, stdout);
	for (size_t i = 0; i < BUF_WIDTH; ++i)
		buf[0][i] = samples[i];
	fft(&fft_ctx, buf[1], buf[0], buf[2]);
	glColor4f(1.0, 1.0, 1.0, 0.5);
	draw_buf(buf[0], BUF_WIDTH, 0, 0);
	glColor4f(1.0, 0.5, 0.5, 1);
	draw_buf(buf[3], BUF_WIDTH / 2, 20, 1);
	glColor4f(0.5, 1.0, 0.5, 1);
	draw_buf(buf[1], BUF_WIDTH / 2, 20, 1);
	glEnd();
	struct filter *filter;
	TAILQ_FOREACH(filter, &filters, chain)
		draw_filter(filter);
}

struct filter *get_filter(double x, double y)
{
	struct filter *filter;
	TAILQ_FOREACH(filter, &filters, chain)
	{
		float fx, fy;
		get_filter_pos(filter, &fx, &fy);
		float dx = x - fx;
		float dy = y - fy;
		if (sqrtf(dx * dx + dy * dy) < FILTER_SIZE)
			return filter;
	}
	return NULL;
}

void filter_update(struct filter *filter)
{
	filter->update(filter);
}

void cursor_position_callback(GLFWwindow *window, double x, double y)
{
	if (!clicked_filter)
		return;
	clicked_filter->strength = -((y - (RENDER_SIZE / 2)) / (RENDER_SIZE / 2));
	if (clicked_filter->strength < -1)
		clicked_filter->strength = -1;
	else if (clicked_filter->strength > 1)
		clicked_filter->strength = 1;
	clicked_filter->frequency = coord_to_freq(x, BUF_WIDTH / 2);
	if (clicked_filter->frequency <= 0)
		clicked_filter->frequency = 1;
	else if (clicked_filter->frequency >= BUF_WIDTH / 2)
		clicked_filter->frequency = BUF_WIDTH / 2 - 1;
	filter_update(clicked_filter);
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
	if (button != GLFW_MOUSE_BUTTON_LEFT)
		return;
	if (action == GLFW_RELEASE)
	{
		clicked_filter = NULL;
		return;
	}
	double x, y;
	glfwGetCursorPos(window, &x, &y);
	clicked_filter = get_filter(x, y);
}

void scroll_callback(GLFWwindow *window, double x, double y)
{
	double cx, cy;
	glfwGetCursorPos(window, &cx, &cy);
	struct filter *filter = get_filter(cx, cy);
	if (!filter)
		return;
	if (y > 0)
		filter->width -= 0.05;
	else if (y < 0)
		filter->width += 0.05;
	if (filter->width < 0.01)
		filter->width = 0.01;
	else if (filter->width > 5)
		filter->width = 5;
	filter_update(filter);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
		paused = !paused;
	if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
	{
		if (fp)
			rewind(fp);
	}
}

int main(int argc, char **argv)
{
	fft_ctx_init(&fft_ctx, BUF_WIDTH);
	if (argc > 1)
	{
		fp = fopen(argv[1], "rb");
		if (!fp)
		{
			fprintf(stderr, "%s: failed to open file\n", argv[0]);
			return EXIT_FAILURE;
		}
	}
	started = nanotime();
	last_read = started;
	glfwInit();
	GLFWwindow *window = glfwCreateWindow(RENDER_SIZE, RENDER_SIZE, "equalizer", NULL, NULL);
	if (!window)
	{
		fprintf(stderr, "%s: failed to create window\n", argv[0]);
		return EXIT_FAILURE;
	}
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwMakeContextCurrent(window);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glViewport(0, 0, RENDER_SIZE, RENDER_SIZE);
	glMatrixMode(GL_MODELVIEW);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, RENDER_SIZE, RENDER_SIZE, 0, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0, 0, 0, 1);

#if 0
	struct filter *lpf;
	lpf = lowpass_new(BUF_WIDTH, BUF_WIDTH * 0.25, 0.5);
	TAILQ_INSERT_TAIL(&filters, lpf, chain);
#endif

#if 0
	struct filter *hpf;
	hpf = highpass_new(BUF_WIDTH, BUF_WIDTH * 0.25, 1);
	TAILQ_INSERT_TAIL(&filters, hpf, chain);
#endif

#if 0
	struct filter *bpf;
	bpf = bandpass_init(BUF_WIDTH, BUF_WIDTH * 0.25, 0.1);
	TAILQ_INSERT_TAIL(&filters, bpf, chain);
#endif

#if 0
	struct filter *apf;
	apf = allpass_new(BUF_WIDTH, BUF_WIDTH * 0.25, 0.1);
	TAILQ_INSERT_TAIL(&filters, apf, chain);
#endif

#if 1
	struct filter *notch;
	notch = notch_new(BUF_WIDTH, BUF_WIDTH * 0.25, 0.1);
	TAILQ_INSERT_TAIL(&filters, notch, chain);
#endif

#if 0
	struct filter *delay;
	delay = delay_new();
	TAILQ_INSERT_TAIL(&filters, delay, chain);
#endif

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		draw();
		glfwSwapBuffers(window);
	}
	return EXIT_SUCCESS;
}
