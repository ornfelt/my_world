#include "_netent.h"

#include <netdb.h>

struct netent *getnetent(void)
{
	if (!netent_fp)
	{
		setnetent(1);
		if (!netent_fp)
			return NULL;
	}
	struct netent *netent = next_netent();
	if (!netent_stayopen)
		endnetent();
	return netent;
}
