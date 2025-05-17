#include "_servent.h"

#include <netdb.h>

struct servent *
getservent(void)
{
	struct servent *servent;

	if (!servent_fp)
	{
		setservent(1);
		if (!servent_fp)
			return NULL;
	}
	servent = next_servent();
	if (!servent_stayopen)
		endservent();
	return servent;
}
