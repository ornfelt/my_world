#include "_protoent.h"

#include <netdb.h>

struct protoent *getprotobynumber(int proto)
{
	setprotoent(0);
	if (!protoent_fp)
		return NULL;
	struct protoent *protoent;
	while (1)
	{
		protoent = next_protoent();
		if (!protoent)
			break;
		if (protoent->p_proto != proto)
			continue;
		break;
	}
	if (!protoent_stayopen)
		endprotoent();
	return protoent;
}
