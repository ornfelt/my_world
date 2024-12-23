#ifndef LIBRENDER_EVENTS_H
# define LIBRENDER_EVENTS_H

namespace librender
{

	struct KeyEvent
	{
		int key;
		int scancode;
		int mods;
	};

	struct CharEvent
	{
		char *charcode;
	};

	struct MouseEvent
	{
		int button;
		int mods;
	};

	struct ScrollEvent
	{
		int xScroll;
		int yScroll;
	};

}

#endif
