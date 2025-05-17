#include "_protoent.h"

#include <netdb.h>

struct protoent *
getprotoent(void)
{
	struct protoent *protoent;

	if (!protoent_fp)
	{
		setprotoent(1);
		if (!protoent_fp)
			return NULL;
	}
	protoent = next_protoent();
	if (!protoent_stayopen)
		endprotoent();
	return protoent;
}
