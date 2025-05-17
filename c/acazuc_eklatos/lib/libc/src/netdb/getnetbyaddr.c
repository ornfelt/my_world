#include "_netent.h"

#include <netdb.h>

struct netent *
getnetbyaddr(uint32_t net, int type)
{
	struct netent *netent;

	if (type != AF_INET)
		return NULL;
	setnetent(0);
	if (!netent_fp)
		return NULL;
	while (1)
	{
		netent = next_netent();
		if (!netent)
			break;
		if (netent->n_addrtype != type)
			continue;
		if (netent->n_net != net)
			continue;
		break;
	}
	if (!netent_stayopen)
		endnetent();
	return netent;
}
