#include <vector>
#include <iostream>
#include <unistd.h>
#include <complex>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#define COUNT 1024
#define FREQ 5

#define RENDER_WIDTH 1024
#define RENDER_HEIGHT 1024

using type = float;
#define fn_cos cosf
#define fn_sin sinf

#include "dft.h"

std::vector<std::complex<type>> g_in(COUNT);
std::vector<std::complex<type>> g_out(COUNT);
std::vector<std::complex<type>> g_re(COUNT);
std::vector<std::complex<type>> g_fft(COUNT);
std::vector<std::complex<type>> g_fft_re(COUNT);
float g_scale = 8;

static uint64_t nanotime(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000000000 + ts.tv_nsec;
}


void draw(std::vector<std::complex<type>> &in, std::vector<std::complex<type>> &out, std::vector<std::complex<type>> &re, std::vector<std::complex<type>> &fft, std::vector<std::complex<type>> &fft_re)
{
	glBegin(GL_POINTS);
	long prev;
	long value;
	glColor4f(1, 0, 0, 1);
	for (size_t i = 0; i < RENDER_WIDTH; ++i)
	{
		value = RENDER_HEIGHT / 2 + RENDER_HEIGHT / g_scale * in[i].real();
		value = std::min((long)RENDER_HEIGHT, std::max((long)0, value));
		glVertex2f(i, value);
		if (i)
		{
			if (value > prev)
			{
				for (int j = prev; j <= value; ++j)
					glVertex2f(i, j);
			}
			else
			{
				for (int j = value; j <= prev; ++j)
					glVertex2f(i, j);
			}
		}
		prev = value;
	}
	glColor4f(0, 1, 0, 1);
	for (size_t i = 0; i < RENDER_WIDTH; ++i)
	{
		value = RENDER_HEIGHT / 2 + RENDER_HEIGHT / g_scale * re[i].real();// / 960;
		value = std::min((long)RENDER_HEIGHT, std::max((long)0, value));
		glVertex2f(i, value);
		if (i)
		{
			if (value > prev)
			{
				for (int j = prev; j <= value; ++j)
					glVertex2f(i, j);
			}
			else
			{
				for (int j = value; j <= prev; ++j)
					glVertex2f(i, j);
			}
		}
		prev = value;
	}
	glColor4f(1, 0, 1, 1);
	for (size_t i = 0; i < RENDER_WIDTH; ++i)
	{
		value = RENDER_HEIGHT / 2 + RENDER_HEIGHT / g_scale * fft_re[i].real();// / 960;
		value = std::min((long)RENDER_HEIGHT, std::max((long)0, value));
		glVertex2f(i, value);
		if (i)
		{
			if (value > prev)
			{
				for (int j = prev; j <= value; ++j)
					glVertex2f(i, j);
			}
			else
			{
				for (int j = value; j <= prev; ++j)
					glVertex2f(i, j);
			}
		}
		prev = value;
	}
	glColor4f(0, 1, 1, 1);
	for (size_t i = 0; i < RENDER_WIDTH; ++i)
	{
		std::complex<type> &tmp = out[i];
		value = 1 * sqrt(tmp.real() * tmp.real() + tmp.imag() * tmp.imag());
		value = std::min((long)RENDER_HEIGHT, std::max((long)0, value));
		glVertex2f(i, value);
		if (i)
		{
			if (value > prev)
			{
				for (int j = prev; j <= value; ++j)
					glVertex2f(i, j);
			}
			else
			{
				for (int j = value; j <= prev; ++j)
					glVertex2f(i, j);
			}
		}
		prev = value;
	}
	glColor4f(1, 1, 0, 1);
	for (size_t i = 0; i < RENDER_WIDTH; ++i)
	{
		std::complex<type> &tmp = fft[i];
		value = 1 * sqrt(tmp.real() * tmp.real() + tmp.imag() * tmp.imag());
		value = std::min((long)RENDER_HEIGHT, std::max((long)0, value));
		glVertex2f(i, value);
		if (i)
		{
			if (value > prev)
			{
				for (int j = prev; j <= value; ++j)
					glVertex2f(i, j);
			}
			else
			{
				for (int j = value; j <= prev; ++j)
					glVertex2f(i, j);
			}
		}
		prev = value;
	}
	glEnd();
}

void run()
{
	uint64_t s, e;
	s = nanotime();
	dft(g_in, g_out);
	e = nanotime();
	printf("dft: %lu\n", (e - s) / 1000);
	idft(g_out, g_re);
	s = nanotime();
	fft(g_in, g_fft);
	e = nanotime();
	printf("fft: %lu\n", (e - s) / 1000);
	idft(g_fft, g_fft_re);
}

static bool g_changed = false;
int last_y = -1;
int last_x = -1;

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	if (state != GLFW_PRESS)
		return;
	int x = xpos;
	if (x < 0 || x > RENDER_WIDTH)
		return;
	if (x == last_x)
		return;
	int y = ypos;
	if (y < 0 || y > RENDER_HEIGHT)
		return;
	if (last_x == -1)
		g_in[x] = (y - (RENDER_HEIGHT / 2)) / RENDER_HEIGHT * g_scale;
	else
	{
		if (x > last_x)
		{
			for (int i = last_x; i <= x; ++i)
				g_in[i] = (last_y + (y - last_y) * ((i - last_x) / (float)(x - last_x)) - (RENDER_HEIGHT / 2)) / RENDER_HEIGHT * g_scale;
		}
		else
		{
			for (int i = x; i <= last_x; ++i)
				g_in[i] = (y + (last_y - y) * ((i - x) / (float)(last_x - x)) - (RENDER_HEIGHT / 2)) / RENDER_HEIGHT * g_scale;
		}
	}
	last_x = x;
	last_y = y;
	g_changed = true;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE && g_changed)
		run();
	last_x = -1;
}

int main()
{
	srand(time(nullptr));
	for (size_t i = 0; i < g_in.size(); ++i)
	{
		g_in[i] = std::complex<type>(0, 0);
		int osef = 20;
		for (int j = 0; j < osef; ++j)
			g_in[i].real(g_in[i].real() + sin(i / static_cast<type>(g_in.size()) * FREQ * pow(2, j) * M_PI * 2) / osef);
		g_in[i].real(g_in[i].real() + (i % 300) / 300.);
		g_in[i].real(g_in[i].real() + (i < RENDER_WIDTH / 2 ? 1 : -1));
		g_in[i].real(g_in[i].real() + rand() / (float)RAND_MAX);
	}
	run();
	glfwInit();
	GLFWwindow *window = glfwCreateWindow(RENDER_WIDTH, RENDER_HEIGHT, "osef", NULL, NULL);
	if (!window)
		return 0;
	glfwMakeContextCurrent(window);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glViewport(0, 0, RENDER_WIDTH, RENDER_HEIGHT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, RENDER_WIDTH, RENDER_HEIGHT, 0, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(.1, .1, .1, 1);
	glfwSetCursorPosCallback(window, cursor_pos_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glfwWaitEvents();
		draw(g_in, g_out, g_re, g_fft, g_fft_re);
		glfwSwapBuffers(window);
	}
	return EXIT_SUCCESS;
}
