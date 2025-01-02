#ifndef X11_EXTENSIONS_XSHM_H
#define X11_EXTENSIONS_XSHM_H

#include <X11/Xlib.h>

typedef uint32_t ShmSeg;

typedef struct
{
	ShmSeg shmseg;
	int shmid;
	char *shmaddr;
	Bool readOnly;
} XShmSegmentInfo;

Status XShmQueryExtension(Display *display);
Status XShmQueryVersion(Display *display, int *major, int *minor, Bool *pixmaps);
Bool XShmAttach(Display *display, XShmSegmentInfo *shminfo);
Bool XShmDetach(Display *display, XShmSegmentInfo *shminfo);
XImage *XShmCreateImage(Display *display, Visual *visual, unsigned depth,
                        int format, char *data, XShmSegmentInfo *shminfo,
                        unsigned width, unsigned height);
Bool XShmPutImage(Display *display, Drawable drawable, GC gc, XImage *image,
                  int src_x, int src_y, int dst_x, int dst_y, unsigned width,
                  unsigned height, Bool send_event);
Bool XShmGetImage(Display *display, Drawable drawable, XImage *image,
                  int x, int y, unsigned long plane_mask);

#endif
