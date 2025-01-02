#include <X11/Xlib.h>

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	(void)argc;
	Display *display = XOpenDisplay(NULL);
	if (!display)
	{
		fprintf(stderr, "%s: failed to open display\n", argv[0]);
		return EXIT_FAILURE;
	}
	uint8_t map[256];
	int nmap = XGetPointerMapping(display, map, 256);
	for (int i = 0; i < nmap; ++i)
		printf("map[%d] = %u\n", i, map[i]);
	printf("\n");
	int accel_numerator;
	int accel_denominator;
	int threshold;
	XGetPointerControl(display, &accel_numerator, &accel_denominator,
	                   &threshold);
	printf("accel_numerator: %d\n", accel_numerator);
	printf("accel_denominator: %d\n", accel_denominator);
	printf("threshold: %d\n", threshold);
	return EXIT_SUCCESS;
}
