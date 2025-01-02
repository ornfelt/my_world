#include <vector>
#include <iostream>
#include <unistd.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <cmath>

#define BASE_FREQUENCY 50
#define RANGE_FREQUENCY 5
#define RENDER_WIDTH 1280
#define RENDER_HEIGHT 720

std::vector<double> g_in(RENDER_WIDTH);
std::vector<double> g_mod(RENDER_WIDTH);
std::vector<double> g_dem(RENDER_WIDTH);
double g_base_frequency = BASE_FREQUENCY;

void mod(std::vector<double> &in, std::vector<double> &out)
{
	double pos = 0;
	for (size_t i = 0; i < in.size(); ++i)
	{
		out[i] = sin(M_PI * 2 * g_base_frequency * i / double(in.size())) * (in[i] / 2 + .5);
	}
}

void dem(std::vector<double> &in, std::vector<double> &out)
{
	for (size_t i = 0; i < in.size(); ++i)
	{
		double ref = sin(M_PI * 2 * g_base_frequency * i / double(in.size()));
		double a = ((in[i] / ref) - .5) * 2;
		out[i] = a;
	}
}

void draw(std::vector<double> &in, std::vector<double> &mod, std::vector<double> &dem)
{
	glBegin(GL_POINTS);
	long prev;
	long value;
	glColor4f(1, 0, 0, 1);
	for (size_t i = 0; i < RENDER_WIDTH; ++i)
	{
		value = RENDER_HEIGHT * .25 - RENDER_HEIGHT * .25 * in[i];
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
		value = RENDER_HEIGHT * .25 - RENDER_HEIGHT * .25 * dem[i];
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
	glColor4f(0, 0, 1, 1);
	for (size_t i = 0; i < RENDER_WIDTH; ++i)
	{
		value = RENDER_HEIGHT * .75 - RENDER_HEIGHT * .25 * mod[i];
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
	for (size_t i = 0; i < g_in.size(); ++i)
		g_in[i] = std::min(1., std::max(-1., g_in[i]));
	mod(g_in, g_mod);
	dem(g_mod, g_dem);
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
	{
		g_in[x] = (y - (RENDER_HEIGHT * .25)) / (RENDER_HEIGHT * .25);
	}
	else
	{
		if (x > last_x)
		{
			for (int i = last_x; i <= x; ++i)
				g_in[i] = -(last_y + (y - last_y) * ((i - last_x) / (float)(x - last_x)) - (RENDER_HEIGHT * .25)) / (RENDER_HEIGHT * .25);
		}
		else
		{
			for (int i = x; i <= last_x; ++i)
				g_in[i] = -(y + (last_y - y) * ((i - x) / (float)(last_x - x)) - (RENDER_HEIGHT * .25)) / (RENDER_HEIGHT * .25);
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

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_UP && action != GLFW_RELEASE)
		g_base_frequency++;
	else if (key == GLFW_KEY_DOWN && action != GLFW_RELEASE)
		g_base_frequency--;
	std::cout << "base frequency: " << g_base_frequency << std::endl;
	run();
	(void)mods;
	(void)window;
	(void)scancode;
}


int main()
{
	srand(time(nullptr));
	for (size_t i = 0; i < g_in.size(); ++i)
	{
		int osef = 6;
		for (int j = 0; j < osef; ++j)
			g_in[i] += sin(i / static_cast<double>(g_in.size()) * 1 * pow(2, j) * M_PI * 2) / osef;
		//g_in[i] = sin(i / double(g_in.size()) * M_PI * 4);
		//g_in[i] += (i % 300) / 300.;
		//g_in[i] += i < RENDER_WIDTH / 2 ? 1 : -1;
		//g_in[i] += rand() / (float)RAND_MAX;
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
	glfwSetKeyCallback(window, key_callback);
	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glfwWaitEvents();
		if (g_changed)
			run();
		draw(g_in, g_mod, g_dem);
		glfwSwapBuffers(window);
	}
	return EXIT_SUCCESS;
}
