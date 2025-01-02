#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#define RENDER_SIZE 1000

static int offset = 0;

static double p1[RENDER_SIZE];
static double p2[RENDER_SIZE];

static void draww()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBegin(GL_LINE_STRIP);
	glColor4f(1, 0, 0, 1);
	for (int i = 0; i < RENDER_SIZE; ++i)
	{
		p1[i] = sin(i / (RENDER_SIZE / 2.) * M_PI);
		glVertex2f(i, RENDER_SIZE / 2 - p1[i] * RENDER_SIZE / 2);
	}
	glEnd();
	glBegin(GL_LINE_STRIP);
	glColor4f(0, 1, 0, 1);
	for (int i = 0; i < RENDER_SIZE; ++i)
	{
		p2[i] = sin((i + offset) / (RENDER_SIZE / 2.) * M_PI);
		glVertex2f(i, RENDER_SIZE / 2 - p2[i] * RENDER_SIZE  / 2);
	}
	glEnd();
	glBegin(GL_LINE_STRIP);
	glColor4f(0, 0, 1, 1);
	for (int i = 0; i < RENDER_SIZE; ++i)
	{
		glVertex2f(i, RENDER_SIZE / 2 - p1[i] * p2[i] * RENDER_SIZE / 2);
	}
	glEnd();
	double total = 0;
	for (int i = 0; i < RENDER_SIZE; ++i)
	{
		total += p1[i] * p2[i];
	}
	glBegin(GL_LINES);
	glColor4f(1, 1, 1, 1);
	glVertex2f(0, RENDER_SIZE / 2 - total / RENDER_SIZE * RENDER_SIZE / 2);
	glVertex2f(RENDER_SIZE, RENDER_SIZE / 2 - total / RENDER_SIZE * RENDER_SIZE / 2);
	glEnd();
}

static void keyListener(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	(void)key;
	(void)window;
	if (action != 2 && action != 1)
		return;
	if (key == GLFW_KEY_LEFT)
		offset++;
	else if (key == GLFW_KEY_RIGHT)
		offset--;
	draww();
}

int main()
{
	glfwInit();
	GLFWwindow *window = glfwCreateWindow(RENDER_SIZE, RENDER_SIZE, "osef", NULL, NULL);
	if (!window)
		return (0);
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, keyListener);
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
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		draww();
		glfwSwapBuffers(window);
	}
}
