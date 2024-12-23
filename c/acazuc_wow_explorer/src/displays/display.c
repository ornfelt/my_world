#include "display.h"

void display_delete(struct display *display)
{
	if (!display)
		return;
	if (display->dtr)
		display->dtr(display);
	free(display->name);
	free(display);
}
