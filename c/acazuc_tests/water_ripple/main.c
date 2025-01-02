#include <GLFW/glfw3.h>

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#define VEC3_DOT(a, b) \
	((a).x * (b).x + (a).y * (b).y + (a).z * (b).z)

#define VEC3_NORM(a) \
	sqrt(VEC3_DOT(a, a))

#define VEC3_NORMALIZE(type, r, a) \
	do \
	{ \
		type v_norm = VEC3_NORM(a); \
		(r).x = (a).x / v_norm; \
		(r).y = (a).y / v_norm; \
		(r).z = (a).z / v_norm; \
	} while (0)

#define VEC3_CROSS(r, a, b) \
	do \
	{ \
		(r).x = (a).y * (b).z - (a).z * (b).y; \
		(r).y = (a).z * (b).x - (a).x * (b).z; \
		(r).z = (a).x * (b).y - (a).y * (b).x; \
	} while (0)

#define WIDTH 1024
#define DAMPING 0.999

float data1[WIDTH * WIDTH];
float data2[WIDTH * WIDTH];
uint8_t blocks[WIDTH * WIDTH];
bool flip_flop;

struct vec3
{
	float x;
	float y;
	float z;
};

static uint64_t nanotime(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000000000 + ts.tv_nsec;
}

static void set_color(float *data, size_t x, size_t y)
{
	if (x == 0 || x == WIDTH - 1
	 || y == 0 || y == WIDTH - 1)
	{
		glColor4f(0, 0, 0, 1);
		return;
	}
	size_t i = y * WIDTH + x;
#if 1
	float v11 = data[i];
	float v01 = data[i - 1];
	float v21 = data[i + 1];
	float v10 = data[i - WIDTH];
	float v12 = data[i + WIDTH];
	struct vec3 va = {2.0, 0.0, v21 - v01};
	struct vec3 vb = {0.0, 2.0, v12 - v10};
	VEC3_NORMALIZE(float, va, va);
	VEC3_NORMALIZE(float, vb, vb);
	struct vec3 vc;
	VEC3_CROSS(vc, va, vb);
	glColor4f(vc.x, vc.y, vc.z, 1);
#elif 0
	if (blocks[i])
	{
		glColor4f(0, 1, 0, 1);
	}
	else
	{
		float v = data[i];
		if (v < 0)
			glColor4f(-v, 0, 0, 1);
		else
			glColor4f(0, 0, v, 1);
	}
#else
	if (blocks[i])
	{
		glColor4f(1, .5, .5, 1);
	}
	else
	{
		float v = .5 + .5 * data[i];
		glColor4f(v, v, v, 1);
	}
#endif
}

static void draw(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBegin(GL_POINTS);
	float *data = flip_flop ? data2 : data1;
	for (size_t y = 0; y < WIDTH; ++y)
	{
		for (size_t x = 0; x < WIDTH; ++x)
		{
			set_color(data, x, y);
			glVertex2f(x, y);
		}
	}
	glEnd();
}

static void update(void)
{
	float *src;
	float *dst;
	if (flip_flop)
	{
		src = data1;
		dst = data2;
	}
	else
	{
		src = data2;
		dst = data1;
	}
	flip_flop = !flip_flop;
	for (size_t y = 0; y < WIDTH; ++y)
	{
		for (size_t x = 0; x < WIDTH; ++x)
		{
			size_t i = y * WIDTH + x;
			if (blocks[i])
				dst[i] = 0;
			else
				dst[i] = ((src[i - 1] + src[i + 1] + src[i - WIDTH] + src[i + WIDTH]) / 2 - dst[i]) * DAMPING;
		}
	}
}

static void ripplev(int x, int y, float v)
{
	if (x < 0 || x >= WIDTH
	 || y < 0 || y >= WIDTH)
		return;
	float *data = flip_flop ? data2 : data1;
	size_t i = y * WIDTH + x;
	data[i] = v;
}

static void ripple(int x, int y, float v)
{
	ripplev(x, y, v);
	ripplev(x + 1, y, v);
	ripplev(x - 1, y, v);
	ripplev(x, y + 1, v);
	ripplev(x, y - 1, v);
}

static void blockv(int x, int y, uint8_t v)
{
	if (x < 0 || x >= WIDTH
	 || y < 0 || y >= WIDTH)
		return;
	size_t i = y * WIDTH + x;
	blocks[i] = v;
}

static void block(int x, int y, uint8_t v)
{
	if (v)
	{
		blockv(x, y, 1);
		blockv(x + 1, y, 1);
		blockv(x - 1, y, 1);
		blockv(x, y + 1, 1);
		blockv(x, y - 1, 1);
	}
	else
	{
		blockv(x, y, 0);
		blockv(x + 1, y, 0);
		blockv(x - 1, y, 0);
		blockv(x, y + 1, 0);
		blockv(x, y - 1, 0);
		blockv(x + 2, y, 0);
		blockv(x - 2, y, 0);
		blockv(x, y + 2, 0);
		blockv(x, y - 2, 0);
		blockv(x + 1, y + 1, 0);
		blockv(x + 1, y - 1, 0);
		blockv(x - 1, y + 1, 0);
		blockv(x - 1, y - 1, 0);
	}
}

static void action(int button, int action, int x, int y)
{
	if (x < 0 || x >= WIDTH
	 || y < 0 || y >= WIDTH)
		return;
	switch (button)
	{
		case GLFW_MOUSE_BUTTON_LEFT:
			if (action == GLFW_PRESS)
				ripple(x, y, -1);
			else
				ripple(x, y, 1);
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			block(x, y, 1);
			break;
		case GLFW_MOUSE_BUTTON_MIDDLE:
			block(x, y, 0);
			break;
	}
}

static void cursor_pos_callback(GLFWwindow *window, double xpos, double ypos)
{
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		action(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, xpos, ypos);
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
		action(GLFW_MOUSE_BUTTON_RIGHT, GLFW_PRESS, xpos, ypos);
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
		action(GLFW_MOUSE_BUTTON_MIDDLE, GLFW_PRESS, xpos, ypos);
}

static void mouse_button_callback(GLFWwindow *window, int button, int aaction, int mods)
{
	(void)mods;
	double xpos;
	double ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	action(button, aaction, xpos, ypos);
}

int main()
{
	for (size_t i = 0; i < WIDTH; ++i)
	{
		blocks[i] = 1;
		blocks[i * WIDTH] = 1;
		blocks[i * WIDTH + WIDTH - 1] = 1;
		blocks[WIDTH * (WIDTH - 1) + i] = 1;
	}
	glfwInit();
	GLFWwindow *window = glfwCreateWindow(WIDTH, WIDTH, "water_ripple", NULL, NULL);
	if (!window)
		return (0);
	glfwMakeContextCurrent(window);
	glfwSetCursorPosCallback(window, cursor_pos_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glViewport(0, 0, WIDTH, WIDTH);
	glMatrixMode(GL_MODELVIEW);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, WIDTH, WIDTH, 0, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0, 0, 0, 1);
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		uint64_t s = nanotime();
		update();
		uint64_t e = nanotime();
		printf("%lu\n", (e - s) / 1000);
		draw();
		glfwSwapBuffers(window);
	}
}
