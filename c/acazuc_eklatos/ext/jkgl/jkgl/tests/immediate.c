#include <stdlib.h>
#include <stdio.h>

#include "common.h"
#include "gl.h"

int main(int argc, char **argv)
{
	struct window window;

	(void)argc;
	if (setup_window(argv[0], &window))
		return EXIT_FAILURE;
	while (1)
	{
		handle_events(&window);

		GL_CALL(glViewport, 0, 0, window.width, window.height);

		GL_CALL(glDisable, GL_DEPTH_TEST);
		GL_CALL(glDisable, GL_LIGHTING);
		GL_CALL(glClearDepth, 1);
		GL_CALL(glClearColor, 0, 0, 0, 1);
		GL_CALL(glClear, GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		GL_CALL(glMatrixMode, GL_PROJECTION);
		GL_CALL(glLoadIdentity);
		GL_CALL(glOrtho, 0, window.width, 0, window.height, -1, 1);
		GL_CALL(glMatrixMode, GL_MODELVIEW);
		GL_CALL(glLoadIdentity);

		GL_CALL(glBegin, GL_LINE_LOOP);
		GL_CALL(glColor4f, 1, 1, 1, 1);
		GL_CALL(glVertex2f, 10, 10);
		GL_CALL(glColor4f, 1, 0, 0, 1);
		GL_CALL(glVertex2f, 40, 10);
		GL_CALL(glColor4f, 0, 1, 0, 1);
		GL_CALL(glVertex2f, 40, 40);
		GL_CALL(glColor4f, 0, 0, 1, 1);
		GL_CALL(glVertex2f, 10, 40);
		GL_CALL(glEnd);

		GL_CALL(glBegin, GL_LINE_STRIP);
		GL_CALL(glColor4f, 1, 1, 1, 1);
		GL_CALL(glVertex2f, 10, 50);
		GL_CALL(glColor4f, 1, 0, 0, 1);
		GL_CALL(glVertex2f, 40, 50);
		GL_CALL(glColor4f, 0, 1, 0, 1);
		GL_CALL(glVertex2f, 40, 80);
		GL_CALL(glColor4f, 0, 0, 1, 1);
		GL_CALL(glVertex2f, 10, 80);
		GL_CALL(glEnd);

		GL_CALL(glBegin, GL_LINES);
		GL_CALL(glColor4f, 1, 1, 1, 1);
		GL_CALL(glVertex2f, 10, 90);
		GL_CALL(glColor4f, 1, 0, 0, 1);
		GL_CALL(glVertex2f, 40, 90);
		GL_CALL(glColor4f, 0, 1, 0, 1);
		GL_CALL(glVertex2f, 40, 130);
		GL_CALL(glColor4f, 0, 0, 1, 1);
		GL_CALL(glVertex2f, 10, 130);
		GL_CALL(glEnd);

		GL_CALL(glBegin, GL_TRIANGLES);
		GL_CALL(glColor4f, 1, 1, 1, 1);
		GL_CALL(glVertex2f, 50, 10);
		GL_CALL(glColor4f, 1, 0, 0, 1);
		GL_CALL(glVertex2f, 80, 10);
		GL_CALL(glColor4f, 0, 1, 0, 1);
		GL_CALL(glVertex2f, 80, 40);
		GL_CALL(glColor4f, 1, 1, 1, 1);
		GL_CALL(glVertex2f, 50, 10);
		GL_CALL(glColor4f, 0, 1, 0, 1);
		GL_CALL(glVertex2f, 80, 40);
		GL_CALL(glColor4f, 0, 0, 1, 1);
		GL_CALL(glVertex2f, 50, 40);
		GL_CALL(glEnd);

		GL_CALL(glBegin, GL_TRIANGLE_STRIP);
		GL_CALL(glColor4f, 1, 0, 0, 1);
		GL_CALL(glVertex2f, 80, 50);
		GL_CALL(glColor4f, 1, 1, 1, 1);
		GL_CALL(glVertex2f, 50, 50);
		GL_CALL(glColor4f, 0, 1, 0, 1);
		GL_CALL(glVertex2f, 80, 80);
		GL_CALL(glColor4f, 0, 0, 1, 1);
		GL_CALL(glVertex2f, 50, 80);
		GL_CALL(glEnd);

		GL_CALL(glBegin, GL_TRIANGLE_FAN);
		GL_CALL(glColor4f, 0, 1, 0, 1);
		GL_CALL(glVertex2f, 80, 90);
		GL_CALL(glColor4f, 1, 0, 0, 1);
		GL_CALL(glVertex2f, 80, 120);
		GL_CALL(glColor4f, 1, 1, 1, 1);
		GL_CALL(glVertex2f, 50, 120);
		GL_CALL(glColor4f, 0, 0, 1, 1);
		GL_CALL(glVertex2f, 50, 90);
		GL_CALL(glColor4f, 0, 1, 1, 1);
		GL_CALL(glVertex2f, 50, 140);
		GL_CALL(glColor4f, 1, 0, 1, 1);
		GL_CALL(glVertex2f, 80, 140);
		GL_CALL(glEnd);

		GL_CALL(glBegin, GL_QUADS);
		GL_CALL(glColor4f, 1, 1, 1, 1);
		GL_CALL(glVertex2f, 90, 10);
		GL_CALL(glColor4f, 1, 0, 0, 1);
		GL_CALL(glVertex2f, 120, 10);
		GL_CALL(glColor4f, 0, 1, 0, 1);
		GL_CALL(glVertex2f, 120, 40);
		GL_CALL(glColor4f, 0, 0, 1, 1);
		GL_CALL(glVertex2f, 90, 40);
		GL_CALL(glEnd);

		GL_CALL(glBegin, GL_QUAD_STRIP);
		GL_CALL(glColor4f, 1, 1, 1, 1);
		GL_CALL(glVertex2f, 130, 10);
		GL_CALL(glColor4f, 1, 0, 0, 1);
		GL_CALL(glVertex2f, 160, 10);
		GL_CALL(glColor4f, 0, 1, 0, 1);
		GL_CALL(glVertex2f, 160, 40);
		GL_CALL(glColor4f, 0, 0, 1, 1);
		GL_CALL(glVertex2f, 130, 40);
		GL_CALL(glColor4f, 0, 1, 1, 1);
		GL_CALL(glVertex2f, 130, 70);
		GL_CALL(glColor4f, 1, 0, 1, 1);
		GL_CALL(glVertex2f, 160, 70);
		GL_CALL(glEnd);

		swap_buffers(&window);
	}
	return EXIT_SUCCESS;
}
