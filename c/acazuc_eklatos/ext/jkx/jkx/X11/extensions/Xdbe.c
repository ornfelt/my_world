#include <X11/extensions/Xdbe.h>
#include <X11/Xlibint.h>

#include <xcb/dbe.h>

#include <stdlib.h>

Status XdbeQueryExtension(Display *display, int *major, int *minor)
{
	REPLY_REQ(display, dbe_query_version,
	          XCB_DBE_MAJOR_VERSION,
	          XCB_DBE_MINOR_VERSION);
	if (error)
	{
		free(error);
		return False;
	}
	if (!reply)
		return False;
	if (major)
		*major = reply->major_version;
	if (minor)
		*minor = reply->minor_version;
	free(reply);
	return True;
}
