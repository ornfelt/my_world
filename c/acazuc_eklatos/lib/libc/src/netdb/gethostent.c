#include "_hostent.h"

#include <netdb.h>

struct hostent *gethostent(void)
{
	if (!hostent_fp)
	{
		sethostent(1);
		if (!hostent_fp)
			return NULL;
	}
	struct hostent *hostent = next_hostent();
	if (!hostent_stayopen)
		endhostent();
	return hostent;
}
