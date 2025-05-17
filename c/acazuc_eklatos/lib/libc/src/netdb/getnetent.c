#include "_netent.h"

#include <netdb.h>

struct netent *
getnetent(void)
{
	struct netent *netent;

	if (!netent_fp)
	{
		setnetent(1);
		if (!netent_fp)
			return NULL;
	}
	netent = next_netent();
	if (!netent_stayopen)
		endnetent();
	return netent;
}
