#include "_servent.h"

#include <string.h>
#include <netdb.h>

struct servent *getservbyport(int port, const char *proto)
{
	setservent(0);
	if (!servent_fp)
		return NULL;
	struct servent *servent;
	while (1)
	{
		servent = next_servent();
		if (!servent)
			break;
		if (port != servent->s_port)
			continue;
		if (proto && strcmp(servent->s_proto, proto))
			continue;
		break;
	}
	if (!servent_stayopen)
		endservent();
	return servent;
}
