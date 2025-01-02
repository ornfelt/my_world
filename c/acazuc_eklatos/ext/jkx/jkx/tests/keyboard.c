#include <X11/Xlib.h>

#include <stdlib.h>
#include <stdio.h>

static const char *mod_names[] =
{
	"Shift",
	"Lock",
	"Control",
	"Mod1",
	"Mod2",
	"Mod3",
	"Mod4",
	"Mod5",
};

int main(int argc, char **argv)
{
	(void)argc;
	Display *display = XOpenDisplay(NULL);
	if (!display)
	{
		fprintf(stderr, "%s: failed to open display\n", argv[0]);
		return EXIT_FAILURE;
	}
	int min_keycode;
	int max_keycode;
	XDisplayKeycodes(display, &min_keycode, &max_keycode);
	int keysyms_per_keycode;
	KeySym *syms = XGetKeyboardMapping(display, min_keycode,
	                                   max_keycode - min_keycode,
	                                   &keysyms_per_keycode);
	if (!syms)
	{
		fprintf(stderr, "%s: failed to get keyboard syms\n", argv[0]);
		return EXIT_FAILURE;
	}
	for (int i = min_keycode; i <= max_keycode; ++i)
	{
		printf("%3d: ", i);
		for (int j = 0; j < keysyms_per_keycode; ++j)
			printf("%08x ", syms[i * keysyms_per_keycode + j]);
		printf("\n");
	}
	printf("\n");
	XModifierKeymap *modmap = XGetModifierMapping(display);
	if (!modmap)
	{
		fprintf(stderr, "%s: failed to get modifier mapping\n", argv[0]);
		return EXIT_FAILURE;
	}
	for (int i = 0; i < 8; ++i)
	{
		printf("%-7s: ", mod_names[i]);
		for (int j = 0; j < modmap->max_keypermod; ++j)
		{
			printf("%3u ", modmap->modifiermap[i * modmap->max_keypermod + j]);
		}
		printf("\n");
	}
	printf("\n");
	XFreeModifiermap(modmap);
	XKeyboardState state;
	if (!XGetKeyboardControl(display, &state))
	{
		fprintf(stderr, "%s: failed to get keyboard control\n", argv[0]);
		return EXIT_FAILURE;
	}
	printf("key click percent: %d\n", state.key_click_percent);
	printf("bell percent: %d\n", state.bell_percent);
	printf("bell pitch: %u\n", state.bell_pitch);
	printf("bell duration: %u\n", state.bell_duration);
	printf("led mask: 0x%lx\n", state.led_mask);
	printf("global auto repeat: %d\n", state.global_auto_repeat);
	printf("auto repeats:");
	for (uint8_t i = 0; i < 32; ++i)
		printf(" %02x", (uint8_t)state.auto_repeats[i]);
	printf("\n");
	return EXIT_SUCCESS;
}
