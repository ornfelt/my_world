#include "_netent.h"

#include <netdb.h>

void setnetent(int stayopen)
{
	if (netent_fp)
		fclose(netent_fp);
	netent_fp = fopen(_PATH_NETWORKS, "rb");
	netent_stayopen = stayopen;
}
