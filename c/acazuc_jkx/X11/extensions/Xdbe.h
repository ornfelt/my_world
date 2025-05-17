#ifndef X11_EXTENSIONS_XDBE_H
#define X11_EXTENSIONS_XDBE_H

#include <X11/Xlib.h>

#ifdef __cplusplus
extern "C" {
#endif

Status XdbeQueryExtension(Display *display, int *major, int *minor);

#ifdef __cplusplus
}
#endif

#endif
