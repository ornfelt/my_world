#include <X11/extensions/XShm.h>
#include <X11/Xlibint.h>

#include <xcb/shm.h>

#include <stdlib.h>

Status XShmQueryExtension(Display *display)
{
	int major_opcode;
	int first_event;
	int first_error;
	return XQueryExtension(display, "MIT-SHM", &major_opcode, &first_event,
	                       &first_error);
}

Status XShmQueryVersion(Display *display, int *major, int *minor, Bool *pixmaps)
{
	REPLY_REQ(display, shm_query_version);
	if (error)
	{
		xlib_handle_error(display, error);
		free(error);
		return False;
	}
	if (!reply)
		return False;
	if (major)
		*major = reply->major_version;
	if (minor)
		*minor = reply->minor_version;
	if (pixmaps)
		*pixmaps = reply->shared_pixmaps;
	free(reply);
	return True;
}

Bool XShmAttach(Display *display, XShmSegmentInfo *shminfo)
{
	shminfo->shmseg = xcb_generate_id(display->conn);
	return XID_REQ(display, shminfo->shmseg, shm_attach,
	               shminfo->shmseg,
	               shminfo->shmid,
	               shminfo->readOnly) == Success;
}

Bool XShmDetach(Display *display, XShmSegmentInfo *shminfo)
{
	return REQUEST(display, shm_detach,
	                 shminfo->shmseg) == Success;
}

XImage *XShmCreateImage(Display *display,
                        Visual *visual,
                        unsigned depth,
                        int format,
                        char *data,
                        XShmSegmentInfo *shminfo,
                        unsigned width,
                        unsigned height)
{
	XImage *image = XCreateImage(display, visual, depth, format, 0, data,
	                             width, height, 32, width * 4); /* XXX */
	if (!image)
		return NULL;
	image->obdata = (XPointer)shminfo;
	return image;
}

Bool XShmPutImage(Display *display,
                  Drawable drawable,
                  GC gc,
                  XImage *image,
                  int src_x,
                  int src_y,
                  int dst_x,
                  int dst_y,
                  unsigned width,
                  unsigned height,
                  Bool send_event)
{
	XFlushGC(gc);
	XShmSegmentInfo *shminfo = (XShmSegmentInfo*)image->obdata;
	return REQUEST(display, shm_put_image,
	                 drawable,
	                 gc->gc,
	                 image->width,
	                 image->height,
	                 src_x,
	                 src_y,
	                 width,
	                 height,
	                 dst_x,
	                 dst_y,
	                 image->depth,
	                 image->format,
	                 send_event,
	                 shminfo->shmseg,
	                 0) == Success; /* XXX not always 0 */
}

Bool XShmGetImage(Display *display,
                  Drawable drawable,
                  XImage *image,
                  int x,
                  int y,
                  unsigned long plane_mask)
{
	XShmSegmentInfo *shminfo = (XShmSegmentInfo*)image->obdata;
	REPLY_REQ(display, shm_get_image,
	          drawable,
	          x,
	          y,
	          image->width,
	          image->height,
	          plane_mask,
	          image->format,
	          shminfo->shmseg,
	          0); /* XXX not always 0 */
	if (error)
	{
		xlib_handle_error(display, error);
		free(error);
		return False;
	}
	if (!reply)
		return False;
	image->depth = reply->depth;
	/* XXX use visual & size from reply */
	free(reply);
	return True;
}
