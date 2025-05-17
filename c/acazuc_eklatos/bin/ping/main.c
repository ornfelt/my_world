#include <sys/socket.h>
#include <sys/types.h>

#include <arpa/inet.h>

#include <netinet/ip_icmp.h>
#include <netinet/in.h>

#include <inttypes.h>
#include <unistd.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <math.h>
#include <time.h>

#define OPT_v (1 << 0)

struct value
{
	uint64_t time;
	struct value *next;
};

struct env
{
	const char *progname;
	int opt;
	char v;
	char *destination;
	char *ip;
	int socket;
	struct sockaddr *addr;
	size_t addrlen;
	uint32_t sent;
	uint32_t count;
	uint32_t received;
	uint64_t last_sent;
	size_t total_sent;
	struct value *values;
	int has_received;
	size_t nping;
};

struct packet
{
	struct ip ip;
	struct icmphdr icmp;
	uint16_t id;
	uint16_t sequence;
	char data[56];
};

static struct env *g_env;

static int
resolve_destination(struct env *env)
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
		fprintf(stderr, "%s: unknown host: %s\n",
		        env->progname,
		        gai_strerror(ret));
		return 1;
	}
	memset(tmp, 0, sizeof(tmp));
	if (!inet_ntop(AF_INET,
	               &((struct sockaddr_in*)res->ai_addr)->sin_addr,
	               tmp,
	               sizeof(tmp)))
	{
		fprintf(stderr, "%s: unknown host: %s\n",
		        env->progname,
		        env->destination);
		freeaddrinfo(res);
		return 1;
	}
	env->ip = strdup(tmp);
	if (!env->ip)
	{
		fprintf(stderr, "%s: malloc: %s\n",
		        env->progname,
		        strerror(errno));
		freeaddrinfo(res);
		return 1;
	}
	env->addrlen = res->ai_addrlen;
	env->addr = malloc(res->ai_addrlen);
	if (!env->addr)
	{
		fprintf(stderr, "%s: malloc: %s\n",
		        env->progname,
		        strerror(errno));
		freeaddrinfo(res);
		return 1;
	}
	memcpy(env->addr, res->ai_addr, res->ai_addrlen);
	freeaddrinfo(res);
	return 0;
}

static int
create_socket(struct env *env)
{
	struct timeval tv;
	int val;

	env->socket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (env->socket == -1)
	{
		fprintf(stderr, "%s: socket: %s\n",
		        env->progname,
		        strerror(errno));
		return 1;
	}
	val = 1;
	if (setsockopt(env->socket,
	               IPPROTO_IP,
	               IP_HDRINCL,
	               &val,
	               sizeof(val)) == -1)
	{
		fprintf(stderr, "%s: setsockopt(IP_HDRINCL): %s\n",
		        env->progname,
		        strerror(errno));
		return 1;
	}
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	if (setsockopt(env->socket,
	               SOL_SOCKET,
	               SO_RCVTIMEO,
	               &tv,
	               sizeof(tv)) == -1)
	{
		fprintf(stderr, "%s: setsockopt(SO_RCVTIMEO): %s\n",
		        env->progname,
		        strerror(errno));
		return 1;
	}
	if (setsockopt(env->socket,
	               SOL_SOCKET,
	               SO_SNDTIMEO,
	               &tv,
	               sizeof(tv)) == -1)
	{
		fprintf(stderr, "%s: setsockopt(SO_SNDTIMEO): %s\n",
		        env->progname,
		        strerror(errno));
		return 1;
	}
	return 0;
}

static void
print_results(struct env *env)
{
	struct value *value;
	uint32_t min;
	uint32_t max;
	uint32_t avg;
	uint32_t total;
	uint32_t number;

	printf("--- %s ping statistics --- %" PRIu32 " packets transmitted, "
	       "%" PRIu32 " received, %" PRIu32 "%% packet loss, "
	       "time %zu ms\n",
	       env->destination, env->count, env->received,
	       env->count ? (100 - (env->received * 100 / env->count)) : 0,
	       env->total_sent / 1000);
	number = 0;
	min = UINT32_MAX;
	max = 0;
	total = 0;
	value = env->values;
	while (value)
	{
		if (value->time < min)
			min = value->time;
		if (value->time > max)
			max = value->time;
		total += value->time;
		number++;
		value = value->next;
	}
	if (min == UINT32_MAX)
		min = 0;
	avg = total / number;
	if (env->received)
		printf("rtt min/avg/max = %" PRIu32 "/%" PRIu32 "/%" PRIu32 " ms",
		       min / 1000000, number == 0 ? 0 : (avg / 1000000),
		       max / 1000000);
	printf("\n");
}

static uint16_t
ip_checksum(const void *addr, size_t len)
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

static int
ping_send(struct env *env)
{
	struct packet packet;
	struct timespec ts;

	env->count++;
	if ((env->opt & OPT_v) && !env->has_received && env->count != 1)
	{
		printf("%zu bytes from %s: type=%d code=%d\n",
		      sizeof(struct packet) - sizeof(struct ip),
		      env->ip, 11, 0);
	}
	env->has_received = 0;
	if (clock_gettime(CLOCK_MONOTONIC, &ts))
	{
		fprintf(stderr, "%s: clock_gettime: %s\n",
		        env->progname,
		        strerror(errno));
		return 1;
	}
	memcpy(packet.data, &ts, sizeof(ts));
	packet.ip.ip_v = 4;
	packet.ip.ip_hl = 5;
	packet.ip.ip_tos = 0;
	packet.ip.ip_len = sizeof(struct packet);
	packet.ip.ip_id = ICMP_ECHO;
	packet.ip.ip_off = 0;
	packet.ip.ip_ttl = 64;
	packet.ip.ip_p = IPPROTO_ICMP;
	packet.ip.ip_sum = 0;
	packet.ip.ip_src.s_addr = 0;
	if (inet_pton(AF_INET, env->ip, &packet.ip.ip_dst.s_addr) != 1)
	{
		fprintf(stderr, "%s: inet_pton: invalid address\n",
		        env->progname);
		return 1;
	}
	packet.icmp.icmp_type = ICMP_ECHO;
	packet.icmp.icmp_code = 0;
	packet.icmp.icmp_cksum = 0;
	packet.id = getpid();
	packet.sequence = htons(env->count);
	packet.icmp.icmp_cksum = ip_checksum(&packet.icmp,
	                                     sizeof(struct packet)
	                                   - sizeof(struct ip));
	if (sendto(env->socket, &packet, sizeof(packet), 0, env->addr,
	           env->addrlen) == -1)
	{
		fprintf(stderr, "%s: sendto: %s\n",
		        env->progname,
		        strerror(errno));
		return 1;
	}
	env->total_sent++;
	env->last_sent = ts.tv_sec * 1000000000 + ts.tv_nsec;
	return 0;
}

static int
value_add(struct env *env, uint64_t time)
{
	struct value *value;

	value = malloc(sizeof(*value));
	if (!value)
	{
		fprintf(stderr, "%s: malloc: %s\n",
		        env->progname,
		        strerror(errno));
		return 1;
	}
	value->time = time;
	value->next = env->values;
	env->values = value;
	return 0;
}

static int
ping_receive(struct env *env)
{
	struct msghdr msghdr;
	struct iovec iovec;
	struct packet packet;
	struct timespec ts;
	int got;
	uint64_t time;
	char ip[16];

	memset(&msghdr, 0, sizeof(msghdr));
	memset(&iovec, 0, sizeof(iovec));
	iovec.iov_base = &packet;
	iovec.iov_len = sizeof(packet);
	msghdr.msg_name = env->addr;
	msghdr.msg_namelen = env->addrlen;
	msghdr.msg_iov = &iovec;
	msghdr.msg_iovlen = 1;
	got = recvmsg(env->socket, &msghdr, 0);
	if (got == -1)
	{
		if (errno == EAGAIN)
			return 0;
		fprintf(stderr, "%s: recvmsg: %s\n",
		        env->progname,
		        strerror(errno));
		return 1;
	}
	if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1)
	{
		fprintf(stderr, "%s: clock_gettime: %s\n",
		        env->progname,
		        strerror(errno));
		return 1;
	}
	time = (ts.tv_sec * 1000000000 + ts.tv_nsec) - env->last_sent;
	got -= sizeof(packet.ip);
	memset(ip, 0, sizeof(ip));
	if (packet.id == getpid()
	 && packet.sequence == ntohs(env->count)
	 && packet.icmp.icmp_type == ICMP_ECHOREPLY)
	{
		env->has_received = 1;
		env->received++;
		printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%" PRIu64 " ms\n",
		       got, inet_ntop(AF_INET, &packet.ip.ip_src.s_addr, ip, sizeof(ip)),
		       ntohs(packet.sequence), packet.ip.ip_ttl, time / 1000000);
		if (value_add(env, time))
			return 1;
	}
	else if (env->opt & OPT_v)
	{
		if (packet.icmp.icmp_type == ICMP_DEST_UNREACH
		 || packet.icmp.icmp_type == ICMP_TIME_EXCEEDED
		 || packet.icmp.icmp_type == ICMP_PARAMPROB)
			printf("%d bytes from %s: type=%d code=%d\n", got,
			       inet_ntop(AF_INET, &packet.ip.ip_src.s_addr, ip, sizeof(ip)),
			       packet.icmp.icmp_type, packet.icmp.icmp_code);
	}
	return 0;
}

static void
sigint_handler(int sig)
{
	(void)sig;
	print_results(g_env);
	exit(EXIT_SUCCESS);
}

static void
usage(const char *progname)
{
	printf("%s [-v] [-h] [-c count] HOST\n", progname);
	printf("-v      : verbose non-answered packets\n");
	printf("-h      : display this help\n");
	printf("-c count: send n ping requests\n");
}

int
main(int argc, char **argv)
{
	struct env env;
	int c;

	g_env = &env;
	if (getuid() != 0)
	{
		fprintf(stderr, "%s must be run as root\n", argv[0]);
		return EXIT_FAILURE;
	}
	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	env.nping = SIZE_MAX;
	while ((c = getopt(argc, argv, "hvc:")) != -1)
	{
		switch (c)
		{
			case 'v':
				env.opt |= OPT_v;
				break;
			case 'c':
			{
				char *endptr;
				errno = 0;
				env.nping = strtoul(optarg, &endptr, 0);
				if (errno || *endptr)
				{
					fprintf(stderr, "%s: invalid count\n",
					        argv[0]);
					return EXIT_FAILURE;
				}
				break;
			}
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
	printf("PING %s (%s) %zu data bytes\n", env.destination, env.ip,
	       sizeof(struct packet) - sizeof(struct ip) - sizeof(struct icmphdr));
	signal(SIGINT, sigint_handler);
	for (size_t i = 0; i < env.nping; ++i)
	{
		struct timespec started;
		struct timespec ended;
		struct timespec diff;

		if (clock_gettime(CLOCK_MONOTONIC, &started) == -1)
		{
			fprintf(stderr, "%s: clock_gettime: %s\n", argv[0],
			        strerror(errno));
			return EXIT_FAILURE;
		}
		if (ping_send(&env)
		 || ping_receive(&env))
			return EXIT_FAILURE;
		if (clock_gettime(CLOCK_MONOTONIC, &ended) == -1)
		{
			fprintf(stderr, "%s: clock_gettime: %s\n", argv[0],
			        strerror(errno));
			return EXIT_FAILURE;
		}
		if (ended.tv_sec - started.tv_sec > 1)
			continue;
		if (i == env.nping - 1)
			break;
		if (ended.tv_nsec < started.tv_nsec)
		{
			diff.tv_sec = ended.tv_sec - started.tv_sec - 1;
			diff.tv_nsec = 1000000000 - (started.tv_nsec - ended.tv_nsec);
		}
		else
		{
			diff.tv_sec = ended.tv_sec - started.tv_sec;
			diff.tv_nsec = ended.tv_nsec - started.tv_nsec;
		}
		if (diff.tv_sec > 0)
			diff.tv_sec--;
		else
			diff.tv_nsec = 999999999 - diff.tv_nsec;
		nanosleep(&diff, NULL);
	}
	print_results(&env);
	return EXIT_SUCCESS;
}
