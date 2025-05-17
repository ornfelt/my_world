#include "_hostent.h"

#include <netdb.h>

void
sethostent(int stayopen)
{
	if (hostent_fp)
		fclose(hostent_fp);
	hostent_fp = fopen(_PATH_HOSTS, "rb");
	hostent_stayopen = stayopen;
}
