#include "_servent.h"

#include <netdb.h>

struct servent *getservent(void)
{
	if (!servent_fp)
	{
		setservent(1);
		if (!servent_fp)
			return NULL;
	}
	struct servent *servent = next_servent();
	if (!servent_stayopen)
		endservent();
	return servent;
}
