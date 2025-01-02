#include "_servent.h"

#include <arpa/inet.h>

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

FILE *servent_fp;
int servent_stayopen;

struct servent *next_servent(void)
{
	static char servent_name[256];
	static char servent_proto[256];
	static struct servent servent;
	size_t n = 0;
	char *line = NULL;
	while (getline(&line, &n, servent_fp) > 0)
	{
		char *tmp = strchr(line, '#');
		if (tmp)
			*tmp = '\0';
		tmp = line;
		while (isspace(*tmp))
			tmp++;
		if (!*tmp)
			continue;
		char *name = tmp;
		while (isgraph(*tmp) && !isspace(*tmp))
			tmp++;
		if (name == tmp)
			continue;
		*tmp = '\0';
		tmp++;
		while (isspace(*tmp))
			continue;
		if (!*tmp)
			continue;
		char *endptr;
		errno = 0;
		unsigned long port = strtoul(tmp, &endptr, 10);
		if (errno || *endptr != '/' || port > UINT16_MAX)
			continue;
		tmp = endptr + 1;
		if (!*tmp)
			continue;
		char *proto = tmp;
		while (isgraph(*tmp) && !isspace(*tmp))
			tmp++;
		*tmp = '\0';
		/* XXX aliases */
		strlcpy(servent_name, name, sizeof(servent_name));
		strlcpy(servent_proto, proto, sizeof(servent_proto));
		servent.s_name = servent_name;
		servent.s_aliases = NULL;
		servent.s_port = ntohs(port);
		servent.s_proto = servent_proto;
		return &servent;
	}
	return NULL;
}
