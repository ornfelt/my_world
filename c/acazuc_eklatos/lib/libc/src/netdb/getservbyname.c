#include "_servent.h"

#include <string.h>
#include <netdb.h>

struct servent *getservbyname(const char *name, const char *proto)
{
	if (!name)
		return NULL;
	setservent(0);
	if (!servent_fp)
		return NULL;
	struct servent *servent;
	while (1)
	{
		servent = next_servent();
		if (!servent)
			break;
		if (strcmp(servent->s_name, name))
			continue;
		if (proto && strcmp(servent->s_proto, proto))
			continue;
		break;
	}
	if (!servent_stayopen)
		endservent();
	return servent;
}
