#include <X11/Xlib-xcb.h>
#include <X11/Xlibint.h>

xcb_connection_t *
XGetXCBConnection(Display *display)
{
	return display->conn;
}
