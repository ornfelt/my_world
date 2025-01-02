#include "_protoent.h"

#include <string.h>
#include <netdb.h>

struct protoent *getprotobyname(const char *name)
{
	if (!name)
		return NULL;
	setprotoent(0);
	if (!protoent_fp)
		return NULL;
	struct protoent *protoent;
	while (1)
	{
		protoent = next_protoent();
		if (!protoent)
			break;
		if (strcmp(protoent->p_name, name))
			continue;
		break;
	}
	if (!protoent_stayopen)
		endprotoent();
	return protoent;
}
