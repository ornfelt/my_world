#include "_protoent.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

FILE *protoent_fp;
int protoent_stayopen;

struct protoent *
next_protoent(void)
{
	static char protoent_name[256];
	static struct protoent protoent;
	size_t n = 0;
	char *line = NULL;

	while (getline(&line, &n, protoent_fp) > 0)
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
		char *endptr;
		errno = 0;
		unsigned long proto = strtoul(tmp, &endptr, 10);
		if (errno || (*endptr && !isspace(*endptr)) || proto > INT_MAX)
			continue;
		*endptr = '\0';
		/* XXX aliases */
		strlcpy(protoent_name, name, sizeof(protoent_name));
		protoent.p_name = protoent_name;
		protoent.p_aliases = NULL;
		protoent.p_proto = proto;
		return &protoent;
	}
	return NULL;
}
