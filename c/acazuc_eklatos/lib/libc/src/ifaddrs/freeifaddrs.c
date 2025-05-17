#include <ifaddrs.h>
#include <stdlib.h>

void
freeifaddrs(struct ifaddrs *addrs)
{
	while (addrs)
	{
		struct ifaddrs *next;

		next = addrs->ifa_next;
		free(addrs->ifa_name);
		free(addrs->ifa_addr);
		free(addrs->ifa_netmask);
		free(addrs->ifa_dstaddr);
		free(addrs->ifa_data);
		free(addrs);
		addrs = next;
	}
}
