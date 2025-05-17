#include "_hostent.h"

#include <arpa/inet.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

FILE *hostent_fp;
int hostent_stayopen;

struct hostent *
next_hostent(void)
{
	static char hostent_name[256];
	static struct in_addr hostent_addr;
	static char *hostent_addr_list[256];
	static struct hostent hostent;
	size_t n = 0;
	char *line = NULL;

	while (getline(&line, &n, hostent_fp) > 0)
	{
		char *tmp = strchr(line, '#');
		if (tmp)
			*tmp = '\0';
		tmp = line;
		while (isspace(*tmp))
			tmp++;
		if (!*tmp)
			continue;
		char *addr = tmp;
		while (*tmp && !isspace(*tmp))
			tmp++;
		if (!*tmp)
			continue;
		if (addr == tmp)
			continue;
		*tmp = '\0';
		tmp++;
		if (inet_pton(AF_INET, addr, &hostent_addr) != 1)
			continue;
		char *name = tmp;
		while (isgraph(*tmp) && !isspace(*tmp))
			tmp++;
		if (name == tmp)
			continue;
		*tmp = '\0';
		/* XXX aliases */
		strlcpy(hostent_name, name, sizeof(hostent_name));
		hostent.h_name = hostent_name;
		hostent.h_aliases = NULL;
		hostent.h_addrtype = AF_INET;
		hostent.h_length = sizeof(struct in_addr);
		hostent_addr_list[0] = (char*)&hostent_addr;
		hostent_addr_list[1] = NULL;
		hostent.h_addr_list = &hostent_addr_list[0];
		return &hostent;
	}
	return NULL;
}
