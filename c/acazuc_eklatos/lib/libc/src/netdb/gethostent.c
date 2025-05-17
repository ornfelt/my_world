#include "_hostent.h"

#include <netdb.h>

struct hostent *
gethostent(void)
{
	struct hostent *hostent;

	if (!hostent_fp)
	{
		sethostent(1);
		if (!hostent_fp)
			return NULL;
	}
	hostent = next_hostent();
	if (!hostent_stayopen)
		endhostent();
	return hostent;
}
