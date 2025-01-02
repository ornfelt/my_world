#include <netinet/ip_icmp.h>
#include <netinet/in.h>

#include <arpa/inet.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <inttypes.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <math.h>
#include <time.h>

struct env
{
	const char *progname;
	int opt;
	char *destination;
	char *ip;
	int socket;
	struct sockaddr *addr;
	socklen_t addrlen;
	uint16_t count;
	uint16_t pcount;
};

struct packet
{
	struct ip ip;
	struct icmphdr icmp;
	uint16_t id;
	uint16_t sequence;
	char data[56];
};

static int resolve_destination(struct env *env)
{
	struct addrinfo *res;
	struct addrinfo hints;
	char tmp[16];
	int ret;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	hints.ai_protocol = IPPROTO_ICMP;
	ret = getaddrinfo(env->destination, NULL, &hints, &res);
	if (ret)
	{
		fprintf(stderr, "%s: unknown host: %s\n", env->progname,
		        gai_strerror(ret));
		return 1;
	}
	memset(tmp, 0, sizeof(tmp));
	if (!inet_ntop(AF_INET, &((struct sockaddr_in*)res->ai_addr)->sin_addr,
	               tmp, sizeof(tmp)))
	{
		fprintf(stderr, "%s: unknown host: %s\n", env->progname,
		        env->destination);
		freeaddrinfo(res);
		return 1;
	}
	env->ip = strdup(tmp);
	if (!env->ip)
	{
		fprintf(stderr, "%s: malloc: %s\n", env->progname,
		        strerror(errno));
		freeaddrinfo(res);
		return 1;
	}
	env->addrlen = res->ai_addrlen;
	env->addr = malloc(res->ai_addrlen);
	if (!env->addr)
	{
		fprintf(stderr, "%s: malloc: %s\n", env->progname,
		        strerror(errno));
		freeaddrinfo(res);
		return 1;
	}
	memcpy(env->addr, res->ai_addr, res->ai_addrlen);
	freeaddrinfo(res);
	return 0;
}

static int create_socket(struct env *env)
{
	struct timeval tv;
	int val;

	env->socket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (env->socket == -1)
	{
		fprintf(stderr, "%s: socket: %s\n", env->progname,
		        strerror(errno));
		return 1;
	}
	val = 1;
	if (setsockopt(env->socket, IPPROTO_IP, IP_HDRINCL, &val,
	               sizeof(val)) == -1)
	{
		fprintf(stderr, "%s: setsockopt(IP_HDRINCL): %s\n",
		        env->progname, strerror(errno));
		return 1;
	}
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	if (setsockopt(env->socket, SOL_SOCKET, SO_RCVTIMEO, &tv,
	               sizeof(tv)) == -1)
	{
		fprintf(stderr, "%s: setsockopt(SO_RCVTIMEO): %s\n",
		        env->progname, strerror(errno));
		return 1;
	}
	if (setsockopt(env->socket, SOL_SOCKET, SO_SNDTIMEO, &tv,
	               sizeof(tv)) == -1)
	{
		fprintf(stderr, "%s: setsockopt(SO_SNDTIMEO): %s\n",
		        env->progname, strerror(errno));
		return 1;
	}
	return 0;
}

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

static int run_packet(struct env *env)
{
	struct packet packet;
	int finished = 0;
	uint32_t oldip = 0;
	struct timespec ts_recv;
	struct timespec ts_sent;

	env->count++;
	env->pcount++;
	if (env->count > 30)
		return 1;
	printf("%2d ", env->count);
	fflush(stdout);
	for (size_t i = 0; i < 3; ++i)
	{
		memset(&packet.data, 0, sizeof(packet.data));
		memcpy(&packet.data[0], &env->pcount, sizeof(env->pcount));
		packet.ip.ip_v = 4;
		packet.ip.ip_hl = 5;
		packet.ip.ip_tos = 0;
		packet.ip.ip_len = sizeof(struct packet);
		packet.ip.ip_id = ICMP_ECHO;
		packet.ip.ip_off = 0;
		packet.ip.ip_ttl = env->count;
		packet.ip.ip_p = IPPROTO_ICMP;
		packet.ip.ip_sum = 0;
		packet.ip.ip_src.s_addr = 0;
		if (inet_pton(AF_INET, env->ip, &packet.ip.ip_dst.s_addr) != 1)
		{
			fprintf(stderr, "%s:  inet_pton: invalid address\n",
			        env->progname);
			return 1;
		}
		packet.icmp.icmp_type = ICMP_ECHO;
		packet.icmp.icmp_code = 0;
		packet.id = getpid();
		packet.sequence = htons(env->pcount);
		packet.icmp.icmp_cksum = 0;
		packet.icmp.icmp_cksum = ip_checksum(&packet.icmp,
		                                     sizeof(struct packet)
		                                   - sizeof(struct ip));
		if (sendto(env->socket, &packet, sizeof(packet), 0,
		           env->addr, env->addrlen) == -1)
		{
			fprintf(stderr, "%s: sendto: %s\n", env->progname,
			        strerror(errno));
			return -1;
		}
		if (clock_gettime(CLOCK_MONOTONIC, &ts_sent))
		{
			fprintf(stderr, "%s: clock_gettime: %s\n", env->progname,
			        strerror(errno));
			return -1;
		}
		memset(&packet, 0, sizeof(packet));
		if (recvfrom(env->socket, &packet, sizeof(packet), 0, env->addr,
		             &env->addrlen) == -1)
		{
			if (errno != EAGAIN && errno != EWOULDBLOCK)
			{
				fprintf(stderr, "%s: recvfrom: %s\n",
				        env->progname, strerror(errno));
				return -1;
			}
			printf(" *");
			fflush(stdout);
			continue;
		}
		if (clock_gettime(CLOCK_MONOTONIC, &ts_recv))
		{
			fprintf(stderr, "%s: clock_gettime: %s\n", env->progname,
			        strerror(errno));
			return -1;
		}
		if (packet.icmp.icmp_type != ICMP_TIME_EXCEEDED
		 && packet.icmp.icmp_type != ICMP_ECHOREPLY)
			continue;
		if (packet.icmp.icmp_type == ICMP_ECHOREPLY
		 && (ntohs(packet.sequence) != env->pcount || packet.id != getpid()))
			continue;
		if (packet.ip.ip_src.s_addr != oldip)
		{
			oldip = packet.ip.ip_src.s_addr;
			printf(" %s", inet_ntoa(packet.ip.ip_src));
		}
		uint32_t recv_ms = ts_recv.tv_sec * 1000
		                 + ts_recv.tv_nsec / 1000000;
		uint32_t sent_ms = ts_sent.tv_sec * 1000
		                 + ts_sent.tv_nsec / 1000000;
		printf("  %" PRIu32 " ms", recv_ms - sent_ms);
		fflush(stdout);
		if (packet.icmp.icmp_type == ICMP_ECHOREPLY)
			finished = 1;
	}
	printf("\n");
	if (finished)
		return 1;
	return 0;
}

static void usage(const char *progname)
{
	printf("%s [-h] HOST\n", progname);
	printf("-h: display this help\n");
}

int main(int argc, char **argv)
{
	struct env env;
	int c;

	if (getuid() != 0)
	{
		fprintf(stderr, "%s must be run as root\n", argv[0]);
		return EXIT_FAILURE;
	}
	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	while ((c = getopt(argc, argv, "h")) != -1)
	{
		switch (c)
		{
			case 'h':
				usage(argv[0]);
				return EXIT_SUCCESS;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	if (optind != argc - 1)
	{
		usage(argv[0]);
		return EXIT_FAILURE;
	}
	env.destination = argv[optind];
	if (resolve_destination(&env)
	 || create_socket(&env))
		return EXIT_FAILURE;
	printf("traceroute to %s (%s), 30 hops max, 60 byte packets\n",
	       env.destination, env.ip);
	while (1)
	{
		switch (run_packet(&env))
		{
			case -1:
				return EXIT_FAILURE;
			case 0:
				break;
			case 1:
				return EXIT_SUCCESS;
		}
	}
	return EXIT_SUCCESS;
}
