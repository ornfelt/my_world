#include "_hostent.h"

#include <arpa/inet.h>

#include <netdb.h>

struct hostent *gethostbyaddr(const void *addr, socklen_t len, int type)
{
	if (type != AF_INET)
		return NULL;
	if (len < sizeof(struct in_addr))
		return NULL;
	sethostent(0);
	if (!hostent_fp)
		return NULL;
	struct hostent *hostent;
	while (1)
	{
		hostent = next_hostent();
		if (!hostent)
			break;
		if (hostent->h_addrtype != AF_INET)
			continue;
		for (size_t i = 0; hostent->h_addr_list[i]; ++i)
		{
			struct in_addr *haddr = (struct in_addr*)hostent->h_addr_list[i];
			if (haddr->s_addr == ((struct in_addr*)addr)->s_addr)
				goto end;
		}
	}
end:
	if (!hostent_stayopen)
		endhostent();
	return hostent;
}
