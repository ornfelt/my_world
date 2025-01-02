#include <arpa/inet.h>

#include <net/if.h>

#include <netinet/ether.h>

#include <sys/ioctl.h>
#include <sys/param.h>

#include <inttypes.h>
#include <stdbool.h>
#include <ifaddrs.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

struct env
{
	const char *progname;
	int sock;
};

static const char *flags_str(uint16_t flags)
{
	static char buf[1024];

	buf[0] = '\0';

#define TEST_FLAG(f) \
do \
{ \
	if (!(flags & (IFF_##f))) \
		break; \
	if (buf[0]) \
		strlcat(buf, ", ", sizeof(buf)); \
	strlcat(buf, #f, sizeof(buf)); \
} while (0)

	TEST_FLAG(LOOPBACK);
	TEST_FLAG(UP);
	TEST_FLAG(BROADCAST);
	TEST_FLAG(PROMISC);
	TEST_FLAG(MULTICAST);
	TEST_FLAG(ALLMULTI);
	TEST_FLAG(RUNNING);

#undef TEST_FLAG

	return buf;
}

static void print_netif_stats(struct env *env, const char *name)
{
	char path[MAXPATHLEN];
	char *line = NULL;
	size_t size = 0;
	FILE *fp = NULL;

	snprintf(path, sizeof(path), "/sys/net/%s", name);
	fp = fopen(path, "rb");
	if (!fp)
	{
		fprintf(stderr, "%s: open(%s): %s\n", env->progname,
		        path, strerror(errno));
		goto end;
	}
	while ((getline(&line, &size, fp)) > 0)
	{
		if (!strncmp(line, "rx_packets:", 11))
		{
			char *endptr;
			errno = 0;
			uint64_t rx_packets = strtoull(&line[11], &endptr, 10);
			if (errno || (*endptr && *endptr != '\n'))
			{
				fprintf(stderr, "%s: invalid rx packets format\n",
				        env->progname);
				break;
			}
			printf("\tRX packets %" PRIu64 "\n", rx_packets);
		}
		else if (!strncmp(line, "rx_bytes:", 9))
		{
			char *endptr;
			errno = 0;
			uint64_t rx_bytes = strtoull(&line[9], &endptr, 10);
			if (errno || (*endptr && *endptr != '\n'))
			{
				fprintf(stderr, "%s: invalid rx bytes format\n",
				        env->progname);
				break;
			}
			printf("\tRX bytes %" PRIu64 "\n", rx_bytes);
		}
		else if (!strncmp(line, "rx_errors:", 10))
		{
			char *endptr;
			errno = 0;
			uint64_t rx_errors = strtoull(&line[10], &endptr, 10);
			if (errno || (*endptr && *endptr != '\n'))
			{
				fprintf(stderr, "%s: invalid rx errors format\n",
				        env->progname);
				break;
			}
			printf("\tRX errors %" PRIu64 "\n", rx_errors);
		}
		else if (!strncmp(line, "tx_packets:", 11))
		{
			char *endptr;
			errno = 0;
			uint64_t tx_packets = strtoull(&line[11], &endptr, 10);
			if (errno || (*endptr && *endptr != '\n'))
			{
				fprintf(stderr, "%s: invalid tx packets format\n",
				        env->progname);
				break;
			}
			printf("\tTX packets %" PRIu64 "\n", tx_packets);
		}
		else if (!strncmp(line, "tx_bytes:", 9))
		{
			char *endptr;
			errno = 0;
			uint64_t tx_bytes = strtoull(&line[9], &endptr, 10);
			if (errno || (*endptr && *endptr != '\n'))
			{
				fprintf(stderr, "%s: invalid tx bytes format\n",
				        env->progname);
				break;
			}
			printf("\tTX bytes %" PRIu64 "\n", tx_bytes);
		}
		else if (!strncmp(line, "tx_errors:", 10))
		{
			char *endptr;
			errno = 0;
			uint64_t tx_errors = strtoull(&line[10], &endptr, 10);
			if (errno || (*endptr && *endptr != '\n'))
			{
				fprintf(stderr, "%s: invalid tx errors format\n",
				        env->progname);
				break;
			}
			printf("\tTX errors %" PRIu64 "\n", tx_errors);
		}
	}

end:
	if (fp)
		fclose(fp);
}

static int print_netif(struct env *env, struct ifaddrs *addr,
                       const char *name)
{
	for (; addr; addr = addr->ifa_next)
	{
		if (strcmp(addr->ifa_name, name))
			continue;
		struct ifreq ifr;
		printf("%s: flags=%u<%s>\n", addr->ifa_name, addr->ifa_flags,
		       flags_str(addr->ifa_flags));
		if (addr->ifa_addr)
		{
			switch (addr->ifa_addr->sa_family)
			{
				case AF_INET:
				{
					char buf[17];
					printf("\tinet %s\n",
					       inet_ntop(AF_INET,
					                 &((struct sockaddr_in*)addr->ifa_addr)->sin_addr,
					                 buf, sizeof(buf)));
					break;
				}
				case AF_INET6:
				{
					char buf[65];
					printf("\tinet6 %s\n",
					       inet_ntop(AF_INET6,
					                 &((struct sockaddr_in6*)addr->ifa_addr)->sin6_addr,
					                 buf, sizeof(buf)));
					break;
				}
				default:
					fprintf(stderr, "%s: unknown interface addr family\n",
					        env->progname);
					return 1;
			}
		}
		if (addr->ifa_netmask)
		{
			switch (addr->ifa_netmask->sa_family)
			{
				case AF_INET:
				{
					char buf[17];
					printf("\tnetmask %s\n",
					       inet_ntop(AF_INET,
					                 &((struct sockaddr_in*)addr->ifa_netmask)->sin_addr,
					                 buf, sizeof(buf)));
					break;
				}
				case AF_INET6:
				{
					struct in6_addr *in6 = &((struct sockaddr_in6*)addr->ifa_addr)->sin6_addr;
					uint32_t prefix_len = 0;
					for (ssize_t i = 127; i >= 0; --i)
					{
						if (!(in6->s6_addr[i / 8] & (1 << i % 8)))
							break;
					}
					printf("prefixlen %" PRIu32 "\n", prefix_len);
					break;
				}
				default:
					fprintf(stderr, "%s: unknown interface netmask family\n",
					        env->progname);
					return 1;
			}
		}
		if (addr->ifa_dstaddr)
		{
			switch (addr->ifa_dstaddr->sa_family)
			{
				case AF_INET:
					printf("\tdstaddr %s\n", inet_ntoa(((struct sockaddr_in*)addr->ifa_dstaddr)->sin_addr));
					break;
				default:
					fprintf(stderr, "%s: unknown interface dstaddr family\n",
					        env->progname);
					return 1;
			}
		}
		strlcpy(ifr.ifr_name, addr->ifa_name, sizeof(ifr.ifr_name));
		if (!ioctl(env->sock, SIOCGIFHWADDR, &ifr))
		{
			printf("\tether %s\n",
			       ether_ntoa((struct ether_addr*)ifr.ifr_hwaddr.sa_data));
		}
		print_netif_stats(env, addr->ifa_name);
	}
	return 0;
}

static int print_interfaces(struct env *env)
{
	struct ifaddrs *addrs;
	if (getifaddrs(&addrs) == -1)
	{
		fprintf(stderr, "%s: getifaddrs: %s\n", env->progname,
		        strerror(errno));
		return 1;
	}
	for (struct ifaddrs *addr = addrs; addr; addr = addr->ifa_next)
	{
		bool found = false;
		struct ifaddrs *tmp = addrs;
		while (tmp != addr)
		{
			if (!strcmp(tmp->ifa_name, addr->ifa_name))
			{
				found = true;
				break;
			}
			tmp = tmp->ifa_next;
		}
		if (found)
			continue;
		if (addr != addrs)
			printf("\n");
		print_netif(env, addr, tmp->ifa_name);
	}
	freeifaddrs(addrs);
	return 0;
}

int main(int argc, char **argv)
{
	struct env env;

	(void)argc; /* XXX */
	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	env.sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (env.sock == -1)
	{
		fprintf(stderr, "%s: socket: %s\n", argv[0],
		        strerror(errno));
		return EXIT_FAILURE;
	}
	if (print_interfaces(&env))
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}
