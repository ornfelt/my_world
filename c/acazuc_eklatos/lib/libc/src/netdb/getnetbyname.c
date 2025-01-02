#include "_netent.h"

#include <string.h>
#include <netdb.h>

struct netent *getnetbyname(const char *name)
{
	if (!name)
		return NULL;
	setnetent(0);
	if (!netent_fp)
		return NULL;
	struct netent *netent;
	while (1)
	{
		netent = next_netent();
		if (!netent)
			break;
		if (strcmp(netent->n_name, name))
			continue;
		break;
	}
	if (!netent_stayopen)
		endnetent();
	return netent;
}
