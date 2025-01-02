#include <sys/socket.h>
#include <sys/ioctl.h>

#include <netinet/ether.h>
#include <netinet/udp.h>
#include <netinet/in.h>

#include <arpa/inet.h>

#include <net/ethernet.h>
#include <net/packet.h>
#include <net/if.h>

#include <inttypes.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

#define OPT_b (1 << 0)
#define OPT_d (1 << 1)

struct env
{
	int opt;
	const char *progname;
	const char *ifname;
	struct ether_addr ether;
	uint32_t xid;
	int sock;
	struct in_addr srv;
	struct in_addr ip;
	struct in_addr gw;
	struct in_addr mask;
	struct in_addr dns;
	uint32_t lease;
	struct timespec last_run;
};

struct pkt
{
	struct ether_header ether;
	struct ip ip;
	struct udphdr udp;
	uint8_t op;
	uint8_t htype;
	uint8_t hlen;
	uint8_t hops;
	uint32_t xid;
	uint16_t secs;
	uint16_t flags;
	struct in_addr ciaddr;
	struct in_addr yiaddr;
	struct in_addr siaddr;
	struct in_addr giaddr;
	uint8_t chaddr[16];
	uint8_t sname[64];
	uint8_t file[128];
	uint8_t vend[64];
} __attribute__ ((packed));

struct udp_pseudohdr
{
	struct in_addr src;
	struct in_addr dst;
	uint8_t zero;
	uint8_t proto;
	uint16_t len;
} __attribute__ ((packed));

static uint16_t ip_checksum(const void *addr, size_t len)
{
	uint64_t result;
	uint16_t *tmp;

	tmp = (uint16_t*)addr;
	result = 0;
	while (len > 1)
	{
		result += *(tmp++);
		len -= 2;
	}
	if (len)
		result += *((uint8_t*)tmp);
	while (result > 0xFFFF)
		result = ((result >> 16) & 0xFFFF) + (result & 0xFFFF);
	return (~((uint16_t)result));
}

static uint16_t udp_checksum(struct pkt *pkt)
{
	size_t len = ntohs(pkt->udp.uh_ulen);
	struct udp_pseudohdr phdr;
	phdr.src = pkt->ip.ip_src;
	phdr.dst = pkt->ip.ip_dst;
	phdr.zero = 0;
	phdr.proto = pkt->ip.ip_p;
	phdr.len = pkt->udp.uh_ulen;

	uint64_t result;
	result  = ((uint16_t*)&phdr)[0];
	result += ((uint16_t*)&phdr)[1];
	result += ((uint16_t*)&phdr)[2];
	result += ((uint16_t*)&phdr)[3];
	result += ((uint16_t*)&phdr)[4];
	result += ((uint16_t*)&phdr)[5];
	uint16_t *tmp = (uint16_t*)&pkt->udp;
	while (len > 1)
	{
		result += *(tmp++);
		len -= 2;
	}
	if (len)
		result += *((uint8_t*)tmp);
	while (result > 0xFFFF)
		result = ((result >> 16) & 0xFFFF) + (result & 0xFFFF);
	return (~((uint16_t)result));
}

static int create_socket(struct env *env)
{
	struct timeval tv;
	struct ifreq ifreq;

	env->sock = socket(AF_PACKET, SOCK_RAW, 0);
	if (env->sock == -1)
	{
		fprintf(stderr, "%s: socket: %s\n", env->progname,
		        strerror(errno));
		return 1;
	}
	strlcpy(ifreq.ifr_name, env->ifname, sizeof(ifreq.ifr_name));
	if (ioctl(env->sock, SIOCGIFFLAGS, &ifreq) == -1)
	{
		fprintf(stderr, "%s: ioctl(SIOCGIFFLAGS): %s\n",
		        env->progname, strerror(errno));
		return 1;
	}
	if ((ifreq.ifr_flags & (IFF_UP | IFF_RUNNING)) != (IFF_UP | IFF_RUNNING))
		return 1;
	if (ioctl(env->sock, SIOCGIFHWADDR, &ifreq) == -1)
	{
		fprintf(stderr, "%s: ioctl(SIOCGIFHWADDR): %s\n",
		        env->progname, strerror(errno));
		return 1;
	}
	memcpy(env->ether.addr, ifreq.ifr_hwaddr.sa_data, ETHER_ADDR_LEN);
#if 0
	printf("%s\n", ether_ntoa(&env->ether));
#endif
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	if (setsockopt(env->sock, SOL_SOCKET, SO_RCVTIMEO, &tv,
	               sizeof(tv)) == -1)
	{
		fprintf(stderr, "%s: setsockopt(SO_RCVTIMEO): %s\n",
		        env->progname, strerror(errno));
		return 1;
	}
	if (setsockopt(env->sock, SOL_SOCKET, SO_SNDTIMEO, &tv,
	               sizeof(tv)) == -1)
	{
		fprintf(stderr, "%s: setsockopt(SO_SNDTIMEO): %s\n",
		        env->progname, strerror(errno));
		return 1;
	}
	return 0;
}

static inline void print_pkt(struct pkt *pkt)
{
	printf("ether: %s", 
	       ether_ntoa((struct ether_addr*)&pkt->ether.ether_shost));
	printf(" -> %s (0x%04" PRIx16 ")\n",
	       ether_ntoa((struct ether_addr*)&pkt->ether.ether_dhost), 
	       ntohs(pkt->ether.ether_type));
	printf("ip: %s", inet_ntoa(pkt->ip.ip_src));
	printf(" -> %s (0x%02" PRIx8")\n", inet_ntoa(pkt->ip.ip_dst),
	       pkt->ip.ip_p);
	printf("udp: %" PRIu16 " -> %" PRIu16 "\n",
	       ntohs(pkt->udp.uh_sport), ntohs(pkt->udp.uh_dport));
	printf("DHCP:\n");
	printf(" op    : 0x%02" PRIx8 "\n", pkt->op);
	printf(" htype : 0x%02" PRIx8 "\n", pkt->htype);
	printf(" hlen  : 0x%02" PRIx8 "\n", pkt->hlen);
	printf(" hops  : 0x%02" PRIx8 "\n", pkt->hops);
	printf(" xid   : 0x%08" PRIx32 "\n", pkt->xid);
	printf(" secs  : 0x%04" PRIx16 "\n", pkt->secs);
	printf(" flags : 0x%04" PRIx16 "\n", pkt->flags);
	printf(" ciaddr: %s\n", inet_ntoa(pkt->ciaddr));
	printf(" yiaddr: %s\n", inet_ntoa(pkt->yiaddr));
	printf(" siaddr: %s\n", inet_ntoa(pkt->siaddr));
	printf(" giaddr: %s\n", inet_ntoa(pkt->giaddr));
	printf(" chaddr: %s\n",
	       ether_ntoa((struct ether_addr*)&pkt->chaddr[0]));
	printf(" vendor: ");
	for (size_t i = 0; i < sizeof(pkt->vend); ++i)
	{
		printf("%02" PRIx8 " ", pkt->vend[i]);
		if (i % 8 == 7)
		{
			printf("\n");
			if (i != sizeof(pkt->vend) - 1)
				printf("         ");
		}
	}
}

static int parse_vendor(struct env *env, struct pkt *pkt, uint8_t *code)
{
	size_t n = 4;
	while (1)
	{

/* XXX i'm soooo lazy.... come on... */
#define GETB(b) \
do \
{ \
	if (n + 1 > sizeof(pkt->vend)) \
		return -1;  \
	(b) = pkt->vend[n++]; \
} while (0)

#define GETL(l) \
do \
{ \
	if (n + 4 > sizeof(pkt->vend)) \
		return -1; \
	(l)  = pkt->vend[n++] << 24; \
	(l) |= pkt->vend[n++] << 16; \
	(l) |= pkt->vend[n++] <<  8; \
	(l) |= pkt->vend[n++] <<  0; \
} while (0)

#define GETLEN(len) \
do \
{ \
	GETB(len); \
	if (n + len >= sizeof(pkt->vend)) \
		return -1; \
} while (0)

		uint8_t opt;
		GETB(opt);
		switch (opt)
		{
			case 0x01:
			{
				uint8_t len;
				GETLEN(len);
				if (len != 4)
				{
					fprintf(stderr, "%s: invalid submask len\n",
					        env->progname);
					return 1;
				}
				GETL(env->mask.s_addr);
				env->mask.s_addr = htonl(env->mask.s_addr);
				break;
			}
			case 0x03:
			{
				uint8_t len;
				GETLEN(len);
				if (len != 4)
				{
					fprintf(stderr, "%s: invalid router len\n",
					        env->progname);
					return 1;
				}
				GETL(env->gw.s_addr);
				env->gw.s_addr = htonl(env->gw.s_addr);
				break;
			}
			case 0x06:
			{
				uint8_t len;
				GETLEN(len);
				if (len != 4)
				{
					fprintf(stderr, "%s: invalid DNS len\n",
					        env->progname);
					return 1;
				}
				GETL(env->dns.s_addr);
				env->dns.s_addr = htonl(env->dns.s_addr);
				break;
			}
			case 0x33:
			{
				uint8_t len;
				GETLEN(len);
				if (len != 4)
				{
					fprintf(stderr, "%s: invalid lease len\n",
					        env->progname);
					return 1;
				}
				GETL(env->lease);
				break;
			}
			case 0x35:
			{
				uint8_t len;
				GETLEN(len);
				if (len != 1)
				{
					fprintf(stderr, "%s: invalid DHCP code len\n",
					        env->progname);
					return 1;
				}
				GETB(*code);
				break;
			}
			case 0x36:
			{
				uint8_t len;
				GETLEN(len);
				if (len != 4)
				{
					fprintf(stderr, "%s: invalid DHCP server len\n",
					        env->progname);
					return 1;
				}
				GETL(env->srv.s_addr);
				env->srv.s_addr = htonl(env->srv.s_addr);
				break;
			}
			case 0xFF:
				return 0;
			default:
				fprintf(stderr, "%s: unknown vendor code: 0x%02" PRIx8 "\n",
				        env->progname, opt);
				return 1;
		}

#undef GETB
#undef GETLEN

	}
}

static void forge_net_headers(struct env *env, struct pkt *pkt)
{
	memcpy(pkt->ether.ether_shost, env->ether.addr, ETHER_ADDR_LEN);
	memset(pkt->ether.ether_dhost, 0xFF, ETHER_ADDR_LEN);
	pkt->ether.ether_type = htons(ETHERTYPE_IP);
	pkt->ip.ip_v = 4;
	pkt->ip.ip_hl = 5;
	pkt->ip.ip_tos = 0;
	pkt->ip.ip_len = htons(sizeof(*pkt) - sizeof(pkt->ether));
	pkt->ip.ip_id = htons(getpid()); /* XXX safer id ? */
	pkt->ip.ip_off = htons(0);
	pkt->ip.ip_ttl = 64;
	pkt->ip.ip_p = IPPROTO_UDP;
	pkt->ip.ip_sum = 0;
	pkt->ip.ip_src.s_addr = htonl(0x00000000);
	pkt->ip.ip_dst.s_addr = htonl(0xFFFFFFFF);
	pkt->ip.ip_sum = ip_checksum(&pkt->ip, sizeof(pkt->ip));
	pkt->udp.uh_sport = htons(68);
	pkt->udp.uh_dport = htons(67);
	pkt->udp.uh_ulen = htons(sizeof(*pkt) - sizeof(pkt->ether) - sizeof(pkt->ip));
	pkt->udp.uh_sum = 0;
}

static int send_discovery(struct env *env)
{
	struct pkt pkt;
	forge_net_headers(env, &pkt);
	pkt.op = 1;
	pkt.htype = 1;
	pkt.hlen = 6;
	pkt.hops = 0;
	pkt.xid = env->xid;
	pkt.secs = 0;
	pkt.flags = 0;
	pkt.ciaddr.s_addr = htonl(0x00000000);
	pkt.yiaddr.s_addr = htonl(0x00000000);
	pkt.siaddr.s_addr = htonl(0x00000000);
	pkt.giaddr.s_addr = htonl(0x00000000);
	memcpy(pkt.chaddr, env->ether.addr, ETHER_ADDR_LEN);
	memset(&pkt.chaddr[ETHER_ADDR_LEN], 0, sizeof(pkt.chaddr) - ETHER_ADDR_LEN);
	memset(pkt.sname, 0, sizeof(pkt.sname));
	memset(pkt.file, 0, sizeof(pkt.file));
	/* cookie */
	pkt.vend[0x0] = 0x63;
	pkt.vend[0x1] = 0x82;
	pkt.vend[0x2] = 0x53;
	pkt.vend[0x3] = 0x63;
	/* DHCP discovery */
	pkt.vend[0x4] = 0x35;
	pkt.vend[0x5] = 0x01;
	pkt.vend[0x6] = 0x01;
	/* request list */
	pkt.vend[0x7] = 0x37;
	pkt.vend[0x8] = 0x03;
	pkt.vend[0x9] = 0x01; /* netmask */
	pkt.vend[0xA] = 0x03; /* router */
	pkt.vend[0xB] = 0x06; /* dns */
	/* end mark */
	pkt.vend[0xC] = 0xFF;
	memset(&pkt.vend[0xD], 0, sizeof(pkt.vend) - 0xD);
	pkt.udp.uh_sum = udp_checksum(&pkt);
	ssize_t ret = send(env->sock, &pkt, sizeof(pkt), 0);
	if (ret == -1)
	{
		fprintf(stderr, "%s: send: %s\n", env->progname,
		        strerror(errno));
		return 1;
	}
	return 0;
}

static int recv_offer(struct env *env)
{
	struct pkt pkt;
	ssize_t ret;
	uint8_t dhcp_code = 0;

	ret = recv(env->sock, &pkt, sizeof(pkt), 0);
	if (ret < 0)
	{
		fprintf(stderr, "%s: recv: %s\n", env->progname,
		        strerror(errno));
		return -1;
	}
	if ((size_t)ret < sizeof(pkt) - sizeof(pkt.vend))
		return 0;
	if (pkt.ether.ether_type != ntohs(ETHERTYPE_IP))
		return 0;
	if (pkt.ip.ip_p != IPPROTO_UDP)
		return 0;
	/* XXX more ip header check */
	if (pkt.udp.uh_sport != htons(67) || pkt.udp.uh_dport != htons(68))
		return 0;
	if (pkt.op != 0x2
	 || pkt.htype != 1
	 || pkt.hlen != 6
	 || pkt.hops != 0
	 || pkt.xid != env->xid
	 || memcmp(pkt.chaddr, env->ether.addr, ETHER_ADDR_LEN)
	 || memcmp(&pkt.vend[0], "\x63\x82\x53\x63", 4))
	{
		fprintf(stderr, "%s: invalid DHCP offer\n", env->progname);
		return -1;
	}
	if (parse_vendor(env, &pkt, &dhcp_code))
		return -1;
	if (dhcp_code != 2)
	{
		fprintf(stderr, "%s: invalid DHCP response code\n",
		        env->progname);
		return -1;
	}
	env->ip.s_addr = pkt.yiaddr.s_addr;
	return 1;
}

static int send_request(struct env *env)
{
	struct pkt pkt;
	forge_net_headers(env, &pkt);
	pkt.op = 1;
	pkt.htype = 1;
	pkt.hlen = 6;
	pkt.hops = 0;
	pkt.xid = env->xid;
	pkt.secs = 0;
	pkt.flags = 0;
	pkt.ciaddr.s_addr = htonl(0x00000000);
	pkt.yiaddr.s_addr = htonl(0x00000000);
	pkt.siaddr.s_addr = env->srv.s_addr;
	pkt.giaddr.s_addr = htonl(0x00000000);
	memcpy(pkt.chaddr, env->ether.addr, ETHER_ADDR_LEN);
	memset(&pkt.chaddr[ETHER_ADDR_LEN], 0, sizeof(pkt.chaddr) - ETHER_ADDR_LEN);
	memset(pkt.sname, 0, sizeof(pkt.sname));
	memset(pkt.file, 0, sizeof(pkt.file));
	/* cookie */
	pkt.vend[0x0] = 0x63;
	pkt.vend[0x1] = 0x82;
	pkt.vend[0x2] = 0x53;
	pkt.vend[0x3] = 0x63;
	/* DHCP request */
	pkt.vend[0x4] = 0x35;
	pkt.vend[0x5] = 0x01;
	pkt.vend[0x6] = 0x03;
	/* ip request */
	pkt.vend[0x7] = 0x32;
	pkt.vend[0x8] = 0x04;
	*(uint32_t*)&pkt.vend[0x9] = env->ip.s_addr;
	/* DHCP server */
	pkt.vend[0x0D] = 0x36;
	pkt.vend[0x0E] = 0x04;
	*(uint32_t*)&pkt.vend[0x0F] = env->srv.s_addr;
	/* oh, end mark */
	pkt.vend[0x13] = 0xFF;
	memset(&pkt.vend[0x14], 0, sizeof(pkt.vend) - 0x14);
	pkt.udp.uh_sum = udp_checksum(&pkt);
	ssize_t ret = send(env->sock, &pkt, sizeof(pkt), 0);
	if (ret == -1)
	{
		fprintf(stderr, "%s: send: %s\n", env->progname,
		        strerror(errno));
		return 1;
	}
	return 0;
}

static int recv_ack(struct env *env)
{
	struct pkt pkt;
	ssize_t ret;
	uint8_t dhcp_code = 0;

	ret = recv(env->sock, &pkt, sizeof(pkt), 0);
	if (ret < 0)
	{
		fprintf(stderr, "%s: recv: %s\n", env->progname,
		        strerror(errno));
		return -1;
	}
	if ((size_t)ret < sizeof(pkt) - sizeof(pkt.vend))
		return 0;
	if (pkt.ether.ether_type != ntohs(ETHERTYPE_IP))
		return 0;
	if (pkt.ip.ip_p != IPPROTO_UDP)
		return 0;
	/* XXX more ip header check */
	if (pkt.udp.uh_sport != htons(67) || pkt.udp.uh_dport != htons(68))
		return 0;
	if (pkt.op != 0x2
	 || pkt.htype != 1
	 || pkt.hlen != 6
	 || pkt.hops != 0
	 || pkt.xid != env->xid
	 || memcmp(pkt.chaddr, env->ether.addr, ETHER_ADDR_LEN)
	 || memcmp(&pkt.vend[0], "\x63\x82\x53\x63", 4))
	{
		fprintf(stderr, "%s: invalid DHCP ack\n", env->progname);
		return -1;
	}
	if (parse_vendor(env, &pkt, &dhcp_code))
		return -1;
	if (dhcp_code != 5)
	{
		fprintf(stderr, "%s: invalid DHCP ack code\n",
		        env->progname);
		return -1;
	}
	return 1;
}

static int update_if(struct env *env)
{
	struct ifreq ifreq;
	struct sockaddr_in *sin;

	strlcpy(ifreq.ifr_name, env->ifname, sizeof(ifreq.ifr_name));
	sin = (struct sockaddr_in*)&ifreq.ifr_addr;
	sin->sin_family = AF_INET;
	sin->sin_port = 0;
	sin->sin_addr.s_addr = env->ip.s_addr;
	if (ioctl(env->sock, SIOCSIFADDR, &ifreq) == -1)
	{
		fprintf(stderr, "%s: ioctl(SIOCSIFADDR): %s\n",
		        env->progname, strerror(errno));
		return 1;
	}
	sin = (struct sockaddr_in*)&ifreq.ifr_netmask;
	sin->sin_family = AF_INET;
	sin->sin_port = 0;
	sin->sin_addr.s_addr = env->mask.s_addr;
	if (ioctl(env->sock, SIOCSIFNETMASK, &ifreq) == -1)
	{
		fprintf(stderr, "%s: ioctl(SIOCSIFNETMASK): %s\n",
		        env->progname, strerror(errno));
		return 1;
	}
	sin = (struct sockaddr_in*)&ifreq.ifr_netmask;
	sin->sin_family = AF_INET;
	sin->sin_port = 0;
	sin->sin_addr.s_addr = env->gw.s_addr;
	if (ioctl(env->sock, SIOCSGATEWAY, sin) == -1)
	{
		fprintf(stderr, "%s: ioctl(SIOCSGATEWAY): %s\n",
		        env->progname, strerror(errno));
		return 1;
	}
#if 0
	printf("DHCP %s: ", env->ifname);
	printf("%s", inet_ntoa(env->ip));
	printf("/%s", inet_ntoa(env->mask));
	printf(" -> %s\n", inet_ntoa(env->gw));
	printf("DNS: %s\n", inet_ntoa(env->dns));
#endif
	FILE *fp = fopen("/tmp/named_server", "w");
	if (fp)
	{
		fprintf(fp, "%s\n", inet_ntoa(env->dns));
		fclose(fp);
		/* XXX SIGHUP named */
	}
	else
	{
		fprintf(stderr, "%s: fopen(%s): %s\n", env->progname,
		        "/var/run/dns", strerror(errno));
	}
	return 0;
}

static int run_request(struct env *env)
{
	if (clock_gettime(CLOCK_REALTIME, &env->last_run) == -1)
	{
		fprintf(stderr, "%s: clock_gettime: %s\n", env->progname,
		        strerror(errno));
		return 1;
	}
	if (create_socket(env)
	 || send_discovery(env)) /* harder, better, faster, stronger */
		return 1;
	while (1)
	{
		switch (recv_offer(env))
		{
			case -1:
				return 1;
			case 0:
				break;
			case 1:
				goto request;
		}
	}
request:
	if (send_request(env))
		return 1;
	while (1)
	{
		switch (recv_ack(env))
		{
			case -1:
				return 1;
			case 0:
				break;
			case 1:
				goto update_ip;
		}
	}
update_ip:
	if (update_if(env))
		return 1;
	return 0;
}

static int test_lease(struct env *env)
{
	struct timespec ts;
	if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
	{
		fprintf(stderr, "%s: clock_gettime: %s\n", env->progname,
		        strerror(errno));
		return 1;
	}
	if (ts.tv_sec - env->last_run.tv_sec < env->lease - env->lease / 10)
		return 0;
	return run_request(env);
}

static void usage(const char *progname)
{
	printf("%s [-h] [-b] [-d] INTERFACE\n", progname);
	printf("-h: display this help\n");
	printf("-b: run immediatly as a background process\n");
	printf("-d: never daemonize\n");
}

int main(int argc, char **argv)
{
	struct env env;
	int c;

	srand(time(NULL));
	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	env.xid = rand();
	while ((c = getopt(argc, argv, "bdh")) != -1)
	{
		switch (c)
		{
			case 'b':
				env.opt |= OPT_b;
				break;
			case 'd':
				env.opt |= OPT_d;
				break;
			case 'h':
				usage(argv[0]);
				return EXIT_SUCCESS;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	switch (argc - optind)
	{
		case 0:
			fprintf(stderr, "%s: missing operand\n", argv[0]);
			return EXIT_FAILURE;
		case 1:
			break;
		default:
			fprintf(stderr, "%s: extra operand\n", argv[0]);
			return EXIT_FAILURE;
	}
	env.ifname = argv[optind];
	if (strlen(env.ifname) >= IFNAMSIZ)
	{
		fprintf(stderr, "%s: interface name too long\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (env.opt & OPT_b)
	{
		if (daemon(0, 1) == -1)
		{
			fprintf(stderr, "%s: daemon: %s\n", argv[0], strerror(errno));
			return EXIT_FAILURE;
		}
	}
	if (run_request(&env))
		return EXIT_FAILURE;
	if (!(env.opt & OPT_d))
	{
		if (daemon(0, 1) == -1)
		{
			fprintf(stderr, "%s: daemon: %s\n", argv[0], strerror(errno));
			return EXIT_FAILURE;
		}
	}
	while (1)
	{
		if (test_lease(&env))
			return EXIT_FAILURE;
		sleep(60);
	}
	return EXIT_SUCCESS;
}
