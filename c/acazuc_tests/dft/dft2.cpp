#include <vector>
#include <iostream>
#include <unistd.h>
#include <complex>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#define IMAGE_WIDTH 50
#define IMAGE_HEIGHT 50

#define DFT_SCALE 30
#define RESULT_BRIGHTNESS 1

#define SCALE 4

#define RENDER_WIDTH (IMAGE_WIDTH * 3 * SCALE)
#define RENDER_HEIGHT (IMAGE_HEIGHT * 4 * SCALE)

using Precision = double;
using Pixel = std::complex<Precision>;
using ImageRow = std::vector<Pixel>;
using ImageData = std::vector<std::vector<Pixel>>;

std::vector<ImageData> g_in(3, ImageData(IMAGE_HEIGHT, ImageRow(IMAGE_WIDTH)));
std::vector<ImageData> g_out(3, ImageData(IMAGE_HEIGHT, ImageRow(IMAGE_WIDTH)));
std::vector<ImageData> g_re(3, ImageData(IMAGE_HEIGHT, ImageRow(IMAGE_WIDTH)));

void dft2(ImageData &in, ImageData &out)
{
	for (size_t v = 0; v < in.size(); ++v)
	{
		std::fill(out[v].begin(), out[v].end(), Pixel(0, 0));
		Precision basey = v / (Precision)in.size();
		for (size_t u = 0; u < in[0].size(); ++u)
		{
			Precision basex = u / (Precision)in[0].size();
			Precision tmpy = 0;
			for (size_t y = 0; y < in.size(); ++y)
			{
				Precision tmpx = 0;
				for (size_t x = 0; x < in[0].size(); ++x)
				{
					Precision tmp = 2 * M_PI * (tmpx + tmpy);
					out[v][u] = Pixel(out[v][u] + in[y][x].real() * Pixel(cos(tmp), sin(tmp)));
					//out[v][u].real(out[v][u].real() + in[y][x].real() * cos(tmp));
					//out[v][u].imag(out[v][u].imag() + in[y][x].real() * sin(tmp));
					tmpx += basex;
				}
				tmpy += basey;
			}
		}
	}
}

void dft2_re(ImageData &in, ImageData &out)
{
	for (size_t v = 0; v < in.size(); ++v)
	{
		std::fill(out[v].begin(), out[v].end(), Pixel(0, 0));
		for (size_t u = 0; u < in[0].size(); ++u)
		{
			for (size_t y = 0; y < in.size(); ++y)
			{
				for (size_t x = 0; x < in[0].size(); ++x)
				{
					Precision tmp = 2 * M_PI * (v * y / (Precision)in.size() + u * x / (Precision)in[0].size());
					out[v][u].real(out[v][u].real() + 2 * (in[y][x].real() * cos(tmp) + in[y][x].imag() * sin(tmp)));
				}
			}
			out[v][u].real(out[v][u].real() / in.size() / in[0].size() / 2);
		}
	}
}

void filter_lpf(ImageData &img, Precision threshold)
{
	for (size_t v = 0; v < img.size(); ++v)
	{
		for (size_t u = 0; u < img[v].size(); ++u)
		{
			Precision factor;
			if (u > threshold && v > threshold)
				factor = 0;
			else
				factor = 1;
			img[v][u].real(img[v][u].real() * factor);
			img[v][u].imag(img[v][u].imag() * factor);
		}
	}
}

void filter_hpf(ImageData &img, Precision threshold)
{
	for (size_t v = 0; v < img.size(); ++v)
	{
		for (size_t u = 0; u < img[v].size(); ++u)
		{
			Precision factor;
			if (u < threshold || v < threshold)
				factor = 0;
			else
				factor = 1;
			img[v][u].real(img[v][u].real() * factor);
			img[v][u].imag(img[v][u].imag() * factor);
		}
	}
}

void draw()
{
	glBegin(GL_POINTS);
	for (int i = 0; i < 3; ++i)
	{
		float r = i == 0 ? 1 : 0;
		float g = i == 1 ? 1 : 0;
		float b = i == 2 ? 1 : 0;
		for (size_t y = 0; y < IMAGE_HEIGHT * SCALE; ++y)
		{
			for (size_t x = 0; x < IMAGE_WIDTH * SCALE; ++x)
			{
				Precision val = g_in[i][y / SCALE][x / SCALE].real();
				glColor4f(val * r, val * g, val * b, 1);
				glVertex2f(x, IMAGE_HEIGHT * SCALE * i + y);
			}
		}
		for (size_t y = 0; y < IMAGE_HEIGHT * SCALE; ++y)
		{
			for (size_t x = 0; x < IMAGE_WIDTH * SCALE; ++x)
			{
				Precision val = sqrt(g_out[i][y / SCALE][x / SCALE].real() * g_out[i][y / SCALE][x / SCALE].real() + g_out[i][y / SCALE][x / SCALE].imag() * g_out[i][y / SCALE][x / SCALE].imag()) / DFT_SCALE;
				glColor4f(val * r, val * g, val * b, 1);
				glVertex2f(IMAGE_WIDTH * SCALE + x, IMAGE_HEIGHT * SCALE * i + y);
			}
		}
		for (size_t y = 0; y < IMAGE_HEIGHT * SCALE; ++y)
		{
			for (size_t x = 0; x < IMAGE_WIDTH * SCALE; ++x)
			{
				Precision val = g_re[i][y / SCALE][x / SCALE].real() * RESULT_BRIGHTNESS;
				glColor4f(val * r, val * g, val * b, 1);
				glVertex2f(IMAGE_WIDTH * SCALE * 2 + x, IMAGE_HEIGHT * SCALE * i + y);
			}
		}
	}
	for (size_t y = 0; y < IMAGE_HEIGHT * SCALE; ++y)
	{
		for (size_t x = 0; x < IMAGE_WIDTH * SCALE; ++x)
		{
			Precision r = g_in[0][y / SCALE][x / SCALE].real();
			Precision g = g_in[1][y / SCALE][x / SCALE].real();
			Precision b = g_in[2][y / SCALE][x / SCALE].real();
			glColor4f(r, g, b, 1);
			glVertex2f(x, IMAGE_HEIGHT * SCALE * 3 + y);
		}
	}
	for (size_t y = 0; y < IMAGE_HEIGHT * SCALE; ++y)
	{
		for (size_t x = 0; x < IMAGE_WIDTH * SCALE; ++x)
		{
			Precision r = sqrt(g_out[0][y / SCALE][x / SCALE].real() * g_out[0][y / SCALE][x / SCALE].real() + g_out[0][y / SCALE][x / SCALE].imag() * g_out[0][y / SCALE][x / SCALE].imag()) / DFT_SCALE;
			Precision g = sqrt(g_out[1][y / SCALE][x / SCALE].real() * g_out[1][y / SCALE][x / SCALE].real() + g_out[1][y / SCALE][x / SCALE].imag() * g_out[1][y / SCALE][x / SCALE].imag()) / DFT_SCALE;
			Precision b = sqrt(g_out[2][y / SCALE][x / SCALE].real() * g_out[2][y / SCALE][x / SCALE].real() + g_out[2][y / SCALE][x / SCALE].imag() * g_out[2][y / SCALE][x / SCALE].imag()) / DFT_SCALE;
			glColor4f(r, g, b, 1);
			glVertex2f(IMAGE_WIDTH * SCALE + x, IMAGE_HEIGHT * SCALE * 3 + y);
		}
	}
	for (size_t y = 0; y < IMAGE_HEIGHT * SCALE; ++y)
	{
		for (size_t x = 0; x < IMAGE_WIDTH * SCALE; ++x)
		{
			Precision r = g_re[0][y / SCALE][x / SCALE].real() * RESULT_BRIGHTNESS;
			Precision g = g_re[1][y / SCALE][x / SCALE].real() * RESULT_BRIGHTNESS;
			Precision b = g_re[2][y / SCALE][x / SCALE].real() * RESULT_BRIGHTNESS;
			glColor4f(r, g, b, 1);
			glVertex2f(IMAGE_WIDTH * SCALE * 2 + x, IMAGE_HEIGHT * SCALE * 3 + y);
		}
	}
	glEnd();
}

void run()
{
	for (int i = 0; i < 3; ++i)
	{
		dft2(g_in[i], g_out[i]);
		//filter_lpf(g_out[i], 0);
		filter_hpf(g_out[i], 0);//IMAGE_WIDTH + IMAGE_HEIGHT - 10);
		dft2_re(g_out[i], g_re[i]);
	}
}

int main()
{
	srand(time(nullptr));
	for (int i = 0; i < 3; ++i)
	{
		for (size_t y = 0; y < g_in[0].size(); ++y)
		{
			for (size_t x = 0; x < g_in[0][y].size(); ++x)
			{
				g_in[i][y][x] = Pixel(0, 0);
				g_in[i][y][x] = rand() / (Precision)RAND_MAX * .25 + .75 * (std::sin(y / (Precision)g_in[0].size() * 2 * M_PI * (i + 1) * 1) / 2 + std::cos(x / (Precision)g_in[0][y].size() * 2 * M_PI * (i + 1) * 2) / 2);
			}
		}
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
	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glfwWaitEvents();
		draw();
		glfwSwapBuffers(window);
	}
	return EXIT_SUCCESS;
}
