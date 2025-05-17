#include "_servent.h"

#include <netdb.h>

void
endservent(void)
{
	if (!servent_fp)
		return;
	fclose(servent_fp);
	servent_fp = NULL;
}
