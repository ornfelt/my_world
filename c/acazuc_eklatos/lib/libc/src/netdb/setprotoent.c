#include "_protoent.h"

#include <netdb.h>

void setprotoent(int stayopen)
{
	if (protoent_fp)
		fclose(protoent_fp);
	protoent_fp = fopen(_PATH_PROTOCOLS, "rb");
	protoent_stayopen = stayopen;
}
