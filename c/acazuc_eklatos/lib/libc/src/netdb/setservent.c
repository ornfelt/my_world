#include "_servent.h"

#include <netdb.h>

void
setservent(int stayopen)
{
	if (servent_fp)
		fclose(servent_fp);
	servent_fp = fopen(_PATH_SERVICES, "rb");
	servent_stayopen = stayopen;
}
