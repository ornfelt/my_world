#include <arpa/inet.h>

#include <stdlib.h>
#include <netdb.h>
#include <errno.h>

int getaddrinfo(const char *node, const char *service,
                       const struct addrinfo *hints, struct addrinfo **result)
{
	struct in_addr in_addr;
	uint16_t port;

	(void)hints; /* XXX */
	if (!node && !service)
		return EAI_NONAME;
	if (node)
	{
		struct hostent *hostent;
		hostent = gethostbyname(node);
		if (!hostent)
			return EAI_NONAME;
		if (hostent->h_addrtype != AF_INET)
			return EAI_NONAME;
		in_addr = *(struct in_addr*)hostent->h_addr_list[0];
	}
	else
	{
		in_addr.s_addr = INADDR_ANY;
	}
	if (service)
	{
		struct servent *servent;
		servent = getservbyname(service, NULL);
		if (servent)
		{
			port = servent->s_port;
		}
		else
		{
			char *endptr;
			errno = 0;
			unsigned long tmp = strtoul(service, &endptr, 0);
			if (errno || *endptr || tmp > UINT16_MAX)
				return EAI_NONAME;
			port = ntohs(tmp);
		}
	}
	else
	{
		port = 0;
	}
	struct addrinfo *addr = calloc(1, sizeof(*addr));
	if (!addr)
		return EAI_MEMORY;
	addr->ai_family = AF_INET;
	addr->ai_addrlen = sizeof(struct sockaddr_in);
	addr->ai_addr = calloc(addr->ai_addrlen, 1);
	if (!addr->ai_addr)
	{
		free(addr);
		return EAI_MEMORY;
	}
	struct sockaddr_in *sin = (struct sockaddr_in*)addr->ai_addr;
	sin->sin_family = AF_INET;
	sin->sin_addr = in_addr;
	sin->sin_port = port;
	*result = addr;
	return 0;
}
