#include "nmap.h"

#include <arpa/inet.h>

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

static struct in_addr get_send_ip(struct env *env, struct host *host)
{
	uint32_t ip = ntohl(((struct sockaddr_in*)host->addr)->sin_addr.s_addr);
	if ((ip & 0xFF000000) == 0x7F000000 || ip == 0)
		return env->loopback_ip;
	return env->local_ip;
}

static void set_tcp_result(struct port_result *result, int type,
                           struct tcp_packet *pkt)
{
	switch (type)
	{
		case SCAN_SYN:
			if (!pkt)
				result->status_syn = FILTERED;
			else if ((pkt->tcp.th_flags & (TH_SYN | TH_ACK)) == (TH_SYN | TH_ACK))
				result->status_syn = OPEN;
			else
				result->status_syn = CLOSED;
			break;
		case SCAN_FIN:
			if (pkt)
				result->status_fin = CLOSED;
			else
				result->status_fin = OPEN_FILTERED;
			break;
		case SCAN_XMAS:
			if (pkt)
				result->status_xmas = CLOSED;
			else
				result->status_xmas = OPEN_FILTERED;
			break;
		case SCAN_NULL:
			if (pkt)
				result->status_null = CLOSED;
			else
				result->status_null = OPEN_FILTERED;
			break;
		case SCAN_ACK:
			if (pkt)
				result->status_ack = UNFILTERED;
			else
				result->status_ack = FILTERED;
			break;
		case SCAN_WIN:
			if (pkt)
			{
				if (pkt->tcp.th_win)
					result->status_win = OPEN;
				else
					result->status_win = CLOSED;
			}
			else
			{
				result->status_win = FILTERED;
			}
			break;
		case SCAN_MAIM:
			if (pkt)
				result->status_maim = CLOSED;
			else
				result->status_maim = OPEN_FILTERED;
			break;
	}
}

static int scan_tcp(struct env *env, struct host *host, struct ip *ip,
                    void (*forge_tcphdr)(struct env *env,
                                         struct tcp_packet *packet,
                                         uint16_t port),
                    uint16_t port, int type)
{
	struct tcp_packet *recv_packet = NULL;
	struct tcp_packet packet;
	uint32_t sequence;

	packet.ip = *ip;
	forge_tcphdr(env, &packet, port);
	sequence = packet.tcp.th_seq;
	packet_flush_tcp(host, port);
	for (size_t retry = 0; retry < env->trials && !recv_packet; ++retry)
	{
		if (sendto(env->sock_raw, &packet,
		           sizeof(packet) - sizeof(packet.chain), 0,
		           host->addr, host->addrlen) == -1)
		{
			fprintf(stderr, "%s: send: %s\n", env->progname,
			        strerror(errno));
			return 1;
		}
		for (size_t i = 0; i < 100; ++i)
		{
			recv_packet = packet_get_tcp(env, host, port, sequence,
			                             type);
			if (recv_packet)
				break;
			usleep(10000 * env->timeout);
		}
	}
	set_tcp_result(&host->results[port], type, recv_packet);
	packet_flush_tcp(host, port);
	free(recv_packet);
	return 0;
}

static int scan_udp(struct env *env, struct host *host, struct ip *ip,
                    uint16_t port)
{
	struct icmp_packet *recv_packet = NULL;
	struct udp_packet packet;

	packet.ip = *ip;
	forge_udphdr(env, &packet, port);
	packet_flush_icmp(host, port);
	for (size_t retry = 0; retry < env->trials && !recv_packet; ++retry)
	{
		if (sendto(env->sock_raw, &packet,
		           sizeof(packet) - sizeof(packet.chain), 0,
		           host->addr, host->addrlen) == -1)
		{
			fprintf(stderr, "%s: send: %s\n", env->progname,
			        strerror(errno));
			return 1;
		}
		for (size_t i = 0; i < 100; ++i)
		{
			recv_packet = packet_get_icmp(env, host, port);
			if (recv_packet)
				break;
			usleep(10000 * env->timeout);
		}
	}
	packet_flush_icmp(host, port);
	if (recv_packet)
		host->results[port].status_udp = CLOSED;
	else
		host->results[port].status_udp = OPEN_FILTERED;
	free(recv_packet);
	return 0;
}

static int scan_port(struct env *env, struct host *host, uint16_t port)
{
	struct ip ip;
	struct in_addr src;
	struct in_addr dst;

	if (inet_pton(AF_INET, host->ip, &dst) != 1)
	{
		fprintf(stderr, "%s: inet_pton failed\n", env->progname);
		return 1;
	}
	src = get_send_ip(env, host);
	host->scanning[port] = 1;
	if (env->scans & SCAN_TCP)
	{
		forge_iphdr(&ip, IPPROTO_TCP, src, dst,
		            sizeof(struct tcp_packet)
		          - sizeof(((struct tcp_packet*)NULL)->chain));
		if (env->scans & SCAN_SYN)
		{
			if (scan_tcp(env, host, &ip, forge_tcphdr_syn,
			             port, SCAN_SYN))
				return 1;
		}
		if (env->scans & SCAN_FIN)
		{
			if (scan_tcp(env, host, &ip, forge_tcphdr_fin,
			             port, SCAN_FIN))
				return 1;
		}
		if (env->scans & SCAN_NULL)
		{
			if (scan_tcp(env, host, &ip, forge_tcphdr_null,
			             port, SCAN_NULL))
				return 1;
		}
		if (env->scans & SCAN_XMAS)
		{
			if (scan_tcp(env, host, &ip, forge_tcphdr_xmas,
			             port, SCAN_XMAS))
				return 1;
		}
		if (env->scans & SCAN_ACK)
		{
			if (scan_tcp(env, host, &ip, forge_tcphdr_ack,
			             port, SCAN_ACK))
				return 1;
		}
		if (env->scans & SCAN_WIN)
		{
			if (scan_tcp(env, host, &ip, forge_tcphdr_win,
			             port, SCAN_WIN))
				return 1;
		}
		if (env->scans & SCAN_MAIM)
		{
			if (scan_tcp(env, host, &ip, forge_tcphdr_maim,
			             port, SCAN_MAIM))
				return 1;
		}
	}
	if (env->scans & SCAN_UDP)
	{
		forge_iphdr(&ip, IPPROTO_UDP, src, dst,
		            sizeof(struct udp_packet)
		          - sizeof(((struct udp_packet*)NULL)->chain));
		if (scan_udp(env, host, &ip, port))
			return 1;
	}
	host->scanning[port] = 0;
	return 0;
}

int scan_host(struct env *env, struct host *host)
{
	size_t start;
	size_t duration;
	printf("\nscanning %s", host->ip);
	if (strcmp(host->ip, host->host))
		printf(" (%s)", host->host);
	printf("\n");
	start = epoch_micro(env);
	for (uint32_t i = 0; i < 65536; ++i)
	{
		if (!env->ports[i])
			continue;
		if (scan_port(env, host, i))
			return 1;
	}
	duration = epoch_micro(env) - start;
	printf("scan took %lu seconds\n", (unsigned long)(duration / 1000000));
	print_result(env, host);
	return 0;
}
