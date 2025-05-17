#include "_netent.h"

#include <arpa/inet.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

FILE *netent_fp;
int netent_stayopen;

struct netent *
next_netent(void)
{
	static char netent_name[256];
	static struct netent netent;
	size_t n = 0;
	char *line = NULL;

	while (getline(&line, &n, netent_fp) > 0)
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
		while (*tmp && !isspace(*tmp))
			tmp++;
		if (!*tmp)
			continue;
		if (name == tmp)
			continue;
		*tmp = '\0';
		tmp++;
		char *addr = tmp;
		while (*tmp && !isspace(*tmp))
			tmp++;
		if (!*tmp)
			continue;
		if (name == tmp)
			continue;
		*tmp = '\0';
		if (inet_pton(AF_INET, addr, &netent.n_net) != 1)
			continue;
		strlcpy(netent_name, name, sizeof(netent_name));
		netent.n_name = netent_name;
		netent.n_aliases = NULL;
		netent.n_addrtype = AF_INET;
		return &netent;
	}
	return NULL;
}
