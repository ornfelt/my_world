#include <vector>
#include <iostream>
#include <unistd.h>
#include <complex>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#define COUNT 8192

#define RENDER_WIDTH 1024
#define RENDER_HEIGHT 1024

using type = float;
#define fn_cos cosf
#define fn_sin sinf

#include "dft.h"

std::vector<std::complex<type>> g_in(COUNT);
std::vector<std::complex<type>> g_out(COUNT);
float g_scale = 8;

static uint64_t nanotime(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000000000 + ts.tv_nsec;
}

void draw(std::vector<std::complex<type>> &in, std::vector<std::complex<type>> &out)
{
	glBegin(GL_POINTS);
	long prev;
	long value;
	glColor4f(0, 1, 0, 1);
	for (size_t i = 0; i < RENDER_WIDTH; ++i)
	{
		value = RENDER_HEIGHT / 2 + RENDER_HEIGHT / g_scale * in[i / (float)RENDER_WIDTH * in.size()].real();
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
		std::complex<type> &tmp = out[i / (float)RENDER_WIDTH * out.size() / 10];
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
	fft(g_in, g_out);
}

int main(int argc, char **argv)
{
	if (argc < 1)
	{
		fprintf(stderr, "dfta <file>\n");
		return EXIT_FAILURE;
	}
	FILE *fp = fopen(argv[1], "r");
	if (!fp)
	{
		fprintf(stderr, "failed to open file\n");
		return EXIT_FAILURE;
	}
	srand(time(nullptr));
	glfwInit();
	GLFWwindow *window = glfwCreateWindow(RENDER_WIDTH, RENDER_HEIGHT, "osef", NULL, NULL);
	if (!window)
		return 0;
	glfwSwapInterval(1);
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
	float buf[COUNT];
	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glfwPollEvents();
		if (fread(buf, 4, COUNT, fp) != COUNT)
		{
			printf("eof\n");
			break;
		}
		for (size_t i = 0; i < COUNT; ++i)
			g_in[i] = std::complex<type>(buf[i], 0);
		run();
		draw(g_in, g_out);
		glfwSwapBuffers(window);
		usleep(1000000 / (44100 / COUNT));
	}
	return EXIT_SUCCESS;
}
