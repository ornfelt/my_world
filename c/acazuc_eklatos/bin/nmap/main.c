#include "nmap.h"

#include <inttypes.h>
#include <strings.h>
#include <ifaddrs.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>

size_t epoch_micro(struct env *env)
{
	struct timespec ts;

	if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
	{
		fprintf(stderr, "%s: clock_gettime: %s\n", env->progname,
		        strerror(errno));
		exit(EXIT_FAILURE);
	}
	return ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

static int add_ip(struct env *env, const char *ip)
{
	char *dup = strdup(ip);
	if (!dup)
	{
		fprintf(stderr, "%s: strdup: %s\n", env->progname,
		        strerror(errno));
		return 1;
	}
	char **ips = realloc(env->ips, sizeof(*env->ips) * (env->ips_count + 1));
	if (!ips)
	{
		fprintf(stderr, "%s: malloc: %s\n", env->progname,
		        strerror(errno));
		free(dup);
		return 1;
	}
	ips[env->ips_count++] = dup;
	env->ips = ips;
	return 0;
}

static int parse_file(struct env *env, const char *file)
{
	FILE *fp = fopen(file, "rb");
	if (!fp)
	{
		fprintf(stderr, "%s: open(%s): %s\n", env->progname, file,
		        strerror(errno));
		return 1;
	}
	size_t n = 0;
	char *line = NULL;
	while (getline(&line, &n, fp) > 0)
	{
		size_t len = strlen(line);
		if (len && line[len - 1] == '\n')
			line[len - 1] = '\0';
		if (add_ip(env, line))
		{
			fclose(fp);
			return 1;
		}
	}
	fclose(fp);
	return 0;
}

static int parse_host(struct env *env, const char *host)
{
	return add_ip(env, host);
}

static int parse_port_part(struct env *env, const char *part)
{
	char *minus = strchr(part, '-');
	if (!minus)
	{
		char *endptr;
		errno = 0;
		unsigned long port = strtoul(part, &endptr, 10);
		if (errno || *endptr || port >= UINT16_MAX)
		{
			fprintf(stderr, "%s: invalid port\n", env->progname);
			return 1;
		}
		if (!env->ports[port])
		{
			env->ports[port] = 1;
			env->ports_count++;
		}
		return 0;
	}
	char *endptr;
	errno = 0;
	unsigned long start = strtoul(part, &endptr, 10);
	if (errno || endptr != minus || start >= UINT16_MAX)
	{
		fprintf(stderr, "%s: invalid range src port\n", env->progname);
		return 1;
	}
	errno = 0;
	unsigned long end = strtoul(minus + 1, &endptr, 10);
	if (errno || *endptr || end >= UINT16_MAX)
	{
		fprintf(stderr, "%s: invalid range dst port\n", env->progname);
		return 1;
	}
	if (start > end)
	{
		uint16_t tmp = start;
		start = end;
		end = tmp;
	}
	for (uint32_t i = start; i <= end; ++i)
	{
		if (env->ports[i])
			continue;
		env->ports[i] = 1;
		env->ports_count++;
	}
	return 0;
}

static int parse_ports(struct env *env, const char *ports)
{
	char *it;
	while ((it = strchrnul(ports, ',')))
	{
		if (it == ports + 1)
		{
			fprintf(stderr, "%s: invalid empty port\n",
			        env->progname);
			return 1;
		}
		int end = 0;
		if (*it == ',')
			*it = '\0';
		else
			end = 1;
		if (parse_port_part(env, ports))
			return 1;
		if (end)
			break;
		ports = it + 1;
	}
	return 0;
}

static int add_scan(struct env *env, const char *scan)
{
#define TEST_SCAN(name) \
do \
{ \
	if (strcasecmp(scan, #name)) \
		continue; \
	if (env->scans & SCAN_##name) \
		continue; \
	env->scans |= SCAN_##name; \
	env->scans_count++; \
	return 0; \
} while (0)

	TEST_SCAN(SYN);
	TEST_SCAN(NULL);
	TEST_SCAN(ACK);
	TEST_SCAN(FIN);
	TEST_SCAN(XMAS);
	TEST_SCAN(UDP);
	TEST_SCAN(WIN);
	TEST_SCAN(MAIM);

#undef TEST_SCAN

	fprintf(stderr, "%s: invalid scan type\n", env->progname);
	return 1;
}

static int parse_scan(struct env *env, const char *scans)
{
	char *it;
	while ((it = strchrnul(scans, ',')))
	{
		if (it == scans + 1)
		{
			fprintf(stderr, "%s: invalid empty scan\n",
			        env->progname);
			return 1;
		}
		int end = 0;
		if (*it == ',')
			*it = '\0';
		else
			end = 1;
		if (add_scan(env, scans))
			return 1;
		if (end)
			break;
		scans = it + 1;
	}
	return 0;
}

static int parse_trials(struct env *env, const char *str)
{
	errno = 0;
	char *endptr;
	unsigned long trials = strtoul(str, &endptr, 10);
	if (errno || *endptr || !trials || trials > 10)
	{
		fprintf(stderr, "%s: invalid trials count\n", env->progname);
		return 1;
	}
	env->trials = trials;
	return 0;
}

static int parse_timeout(struct env *env, const char *str)
{
	errno = 0;
	char *endptr;
	unsigned long timeout = strtoul(str, &endptr, 10);
	if (errno || *endptr || !timeout || timeout > 60)
	{
		fprintf(stderr, "%s: invalid timeout value\n", env->progname);
		return 1;
	}
	env->timeout = timeout;
	return 0;
}

static int resolve_self_ip(struct env *env)
{
	struct ifaddrs *addrs;
	char ipset = 0;

	if (getifaddrs(&addrs) == -1)
	{
		fprintf(stderr, "%s: getifaddrs: %s\n", env->progname,
		        strerror(errno));
		return 1;
	}
	for (struct ifaddrs *addr = addrs; addr; addr = addr->ifa_next)
	{
		if (!addr->ifa_addr)
			continue;
		if (addr->ifa_addr->sa_family != AF_INET)
			continue;
		if (!strcmp(addr->ifa_name, "lo"))
		{
			ipset |= 1;
			env->loopback_ip = ((struct sockaddr_in*)addr->ifa_addr)->sin_addr;
		}
		else if (ipset & 2)
		{
			fprintf(stderr, "%s: ip network collision; skipping\n",
			        env->progname);
		}
		else
		{
			ipset |= 2;
			env->local_ip = ((struct sockaddr_in*)addr->ifa_addr)->sin_addr;
		}
	}
	if (addrs)
		freeifaddrs(addrs);
	if (ipset == 3)
		return 0;
	fprintf(stderr, "%s: failed to resolve external ip\n", env->progname);
	return 1;
}

static int build_sockets(struct env *env)
{
	static const int val = 1;

	env->sock_raw = socket(AF_INET, SOCK_RAW, 0);
	if (env->sock_raw == -1)
	{
		fprintf(stderr, "%s: socket: %s\n", env->progname,
		        strerror(errno));
		return 1;
	}
	if (setsockopt(env->sock_raw, IPPROTO_IP, IP_HDRINCL, &val, 
	               sizeof(val)) == -1)
	{
		fprintf(stderr, "%s: setsockopt(IP_HDRINCL): %s\n",
		        env->progname, strerror(errno));
		return 1;
	}
	env->sock_pkt = socket(AF_PACKET, SOCK_RAW, 0);
	if (env->sock_pkt == -1)
	{
		fprintf(stderr, "%s: socket: %s\n", env->progname,
		        strerror(errno));
		return 1;
	}
	return 0;
}

static void usage(const char *progname)
{
	printf("%s: [-h] [-p ports] [-s scans] [-f file] [-H host] [-t trials] [-T timeout]\n", progname);
	printf("-h        : display this help\n");
	printf("-p ports  : ports to scan (eg: 1-10 or 1,2,3 or 1,5-15). Max is 1024 ports. Default is 1-1024\n");
	printf("-H host   : host to scan\n");
	printf("-f file   : file name containing hosts to scan. One ip per line\n");
	printf("-s scans  : SYN, NULL, FIN, XMAS, ACK, UDP, WIN, MAIM (eg: SYN,UDP or SYN). Default is SYN\n");
	printf("-r trials : number of trials per scan. Max is 10, Default to 2\n");
	printf("-t timeout: timeout in seconds of each scan. Max is 60. Default to 5\n");
}

int main(int argc, char **argv)
{
	struct env env;
	int c;

	if (getuid())
	{
		fprintf(stderr, "%s must be run as root\n", argv[0]);
		return EXIT_FAILURE;
	}
	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	srand(epoch_micro(&env));
	while ((c = getopt(argc, argv, "hp:s:f:H:t:T:")) != -1)
	{
		switch (c)
		{
			case 'h':
				usage(argv[0]);
				return EXIT_SUCCESS;
			case 'p':
				if (parse_ports(&env, optarg))
					return EXIT_FAILURE;
				break;
			case 's':
				if (parse_scan(&env, optarg))
					return EXIT_FAILURE;
				break;
			case 'f':
				if (parse_file(&env, optarg))
					return EXIT_FAILURE;
				break;
			case 'H':
				if (parse_host(&env, optarg))
					return 1;
				break;
			case 't':
				if (parse_trials(&env, optarg))
					return 1;
				break;
			case 'T':
				if (parse_timeout(&env, optarg))
					return 1;
				break;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	if (!env.ips_count)
	{
		fprintf(stderr, "%s: no ip to scan\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (!env.ports_count)
	{
		for (uint16_t i = 1; i <= 1024; ++i)
			env.ports[i] = 1;
		env.ports_count = 1024;
	}
	if (!env.scans_count)
		env.scans = SCAN_SYN;
	if (!env.trials)
		env.trials = 2;
	if (!env.timeout)
		env.timeout = 5;
	if (env.ports_count > 1024)
	{
		fprintf(stderr, "%s: invalid number of scanned ports\n",
		        argv[0]);
		return EXIT_FAILURE;
	}
	if (resolve_self_ip(&env))
		return EXIT_FAILURE;
	if (build_sockets(&env))
		return EXIT_FAILURE;
	if (build_hosts(&env))
		return EXIT_FAILURE;
	env.syn_port = 50000 + (rand() % 1000); /* XXX */
	env.null_port = 50000 + (rand() % 1000); /* XXX */
	env.ack_port = 50000 + (rand() % 1000); /* XXX */
	env.fin_port = 50000 + (rand() % 1000); /* XXX */
	env.xmas_port = 50000 + (rand() % 1000); /* XXX */
	env.udp_port = 50000 + (rand() % 1000); /* XXX */
	env.win_port = 50000 + (rand() % 1000); /* XXX */
	env.maim_port = 50000 + (rand() % 1000); /* XXX */
	printf("scan configuration:\n");
	printf("target hosts:");
	for (size_t i = 0; i < env.hosts_count; ++i)
		printf(" '%s'", env.hosts[i]->host);
	printf("\n");
	printf("number of ports to scan: %" PRIu16 "\n", env.ports_count);
	printf("scans to be performed:");
	if (env.scans & SCAN_SYN)
		printf(" SYN");
	if (env.scans & SCAN_NULL)
		printf(" NULL");
	if (env.scans & SCAN_ACK)
		printf(" ACK");
	if (env.scans & SCAN_FIN)
		printf(" FIN");
	if (env.scans & SCAN_XMAS)
		printf(" XMAS");
	if (env.scans & SCAN_UDP)
		printf(" UDP");
	if (env.scans & SCAN_WIN)
		printf(" WIN");
	if (env.scans & SCAN_MAIM)
		printf(" MAIM");
	printf("\n");
	for (int i = i; env.hosts[i]; ++i)
	{
		if (scan_host(&env, env.hosts[i]))
			return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
