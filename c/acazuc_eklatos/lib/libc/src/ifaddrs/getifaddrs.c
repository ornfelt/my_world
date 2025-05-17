#include <sys/socket.h>
#include <sys/ioctl.h>

#include <net/if.h>

#include <ifaddrs.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int
getifaddrs(struct ifaddrs **addrs)
{
	struct ifaddrs *base = NULL;
	struct ifaddrs *prev = NULL;
	struct ifreq ifr[1024]; /* XXX dynamic */
	struct ifconf ifc;
	int sock;
	int ret;

	ifc.ifc_len = sizeof(ifr);
	ifc.ifc_req = &ifr[0];
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == -1)
		return -1;
	ret = ioctl(sock, SIOCGIFCONF, &ifc);
	if (ret)
	{
		close(sock);
		return -1;
	}
	ifc.ifc_len /= sizeof(struct ifreq);
	for (int i = 0; i < ifc.ifc_len; ++i)
	{
		struct ifaddrs *ifaddr;

		ifaddr = calloc(1, sizeof(*ifaddr));
		if (!ifaddr)
			goto err;
		if (prev)
			prev->ifa_next = ifaddr;
		ifaddr->ifa_name = strdup(ifr[i].ifr_name);
		if (!ifaddr->ifa_name)
			goto err;
		if (ifr[i].ifr_addr.sa_family)
		{
			ifaddr->ifa_addr = malloc(sizeof(*ifaddr->ifa_addr));
			if (!ifaddr->ifa_addr)
				goto err;
			memcpy(ifaddr->ifa_addr, &ifr[i].ifr_addr,
			       sizeof(*ifaddr->ifa_addr));
		}
		else
		{
			ifaddr->ifa_addr = NULL;
		}
		ret = ioctl(sock, SIOCGIFFLAGS, &ifr[i]);
		if (ret == -1)
			goto err;
		ifaddr->ifa_flags = ifr[i].ifr_flags;
		if (!ioctl(sock, SIOCGIFNETMASK, &ifr[i]))
		{
			ifaddr->ifa_netmask = malloc(sizeof(*ifaddr->ifa_netmask));
			if (!ifaddr->ifa_netmask)
				goto err;
			memcpy(ifaddr->ifa_netmask, &ifr[i].ifr_netmask,
			       sizeof(*ifaddr->ifa_netmask));
		}
		/* XXX
		 * ifa_dstaddr
		 * ifa_data
		 */
		if (!base)
			base = ifaddr;
		prev = ifaddr;
	}
	*addrs = base;
	return 0;

err:
	close(sock);
	freeifaddrs(base);
	return -1;
}
