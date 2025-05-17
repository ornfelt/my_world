#include "nmap.h"

#include <arpa/inet.h>

#include <net/ethernet.h>

#include <sys/socket.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

static void
recv_packets(struct env *env, struct host *host)
{
	while (1)
	{
		char buf[1024];
		ssize_t ret;

		ret = recv(env->sock_pkt, buf, sizeof(buf), MSG_DONTWAIT);
		if (ret == -1)
		{
			if (errno == EAGAIN)
				return;
			fprintf(stderr, "%s: recv: %s\n",
			        env->progname,
			        strerror(errno));
			exit(EXIT_FAILURE);
		}
		if ((size_t)ret < sizeof(struct ether_header) + sizeof(struct ip))
			continue;
		struct ether_header *ether = (struct ether_header*)&buf[0];
		if (ether->ether_type != htons(ETHERTYPE_IP))
			continue;
		struct ip *ip = (struct ip*)&ether[1];
		if (ip->ip_src.s_addr != ((struct sockaddr_in*)host->addr)->sin_addr.s_addr)
			continue;
		switch (ip->ip_p)
		{
			case IPPROTO_TCP:
			{
				if ((size_t)ret < sizeof(struct ether_header) + sizeof(struct ip) + sizeof(struct tcphdr))
					continue;
				struct tcphdr *tcp = (struct tcphdr*)&ip[1];
				if (!env->ports[ntohs(tcp->th_sport)])
					continue;
				if (ntohs(tcp->th_dport) != env->syn_port
				 && ntohs(tcp->th_dport) != env->null_port
				 && ntohs(tcp->th_dport) != env->ack_port
				 && ntohs(tcp->th_dport) != env->fin_port
				 && ntohs(tcp->th_dport) != env->xmas_port)
					continue;
				struct tcp_packet *pkt = malloc(sizeof(*pkt));
				if (!pkt)
				{
					fprintf(stderr, "%s: malloc: %s\n",
					        env->progname, strerror(errno));
					exit(EXIT_FAILURE);
				}
				memcpy(&pkt->ip, ip, sizeof(*ip));
				memcpy(&pkt->tcp, tcp, sizeof(*tcp));
				TAILQ_INSERT_TAIL(&host->packets_tcp, pkt, chain);
				break;
			}
			case IPPROTO_ICMP:
			{
				if ((size_t)ret < sizeof(struct ether_header)
				                + sizeof(struct ip)
				                + sizeof(struct icmphdr)
				                + sizeof(struct ip)
				                + sizeof(struct udphdr))
					continue;
				struct icmphdr *icmp = (struct icmphdr*)&ip[1];
				if (icmp->icmp_type != ICMP_DEST_UNREACH
				 || icmp->icmp_code != 3)
					continue;
				struct icmp_packet *pkt = malloc(sizeof(*pkt));
				if (!pkt)
				{
					fprintf(stderr, "%s: malloc: %s\n",
					        env->progname, strerror(errno));
					exit(EXIT_FAILURE);
				}
				memcpy(&pkt->ip, ip, sizeof(*ip));
				memcpy(&pkt->icmp, icmp, sizeof(*icmp));
				memcpy(&pkt->data, &icmp[1], sizeof(pkt->data));
				TAILQ_INSERT_TAIL(&host->packets_icmp, pkt, chain);
				break;
			}
		}
	}
}

void
packet_flush_tcp(struct host *host, uint16_t port)
{
	struct tcp_packet *pkt;
	struct tcp_packet *nxt;

	TAILQ_FOREACH_SAFE(pkt, &host->packets_tcp, chain, nxt)
	{
		if (pkt->tcp.th_sport != htons(port))
			continue;
		TAILQ_REMOVE(&host->packets_tcp, pkt, chain);
		free(pkt);
	}
}

void
packet_flush_icmp(struct host *host, uint16_t port)
{
	struct icmp_packet *pkt;
	struct icmp_packet *nxt;

	TAILQ_FOREACH_SAFE(pkt, &host->packets_icmp, chain, nxt)
	{
		uint16_t tmp;

		if (pkt->icmp.icmp_type != ICMP_DEST_UNREACH
		 || pkt->icmp.icmp_code != 3)
		{
			TAILQ_REMOVE(&host->packets_icmp, pkt, chain);
			free(pkt);
			continue;
		}
		memcpy(&tmp, pkt->data + sizeof(struct ip) + 2, sizeof(tmp));
		if (tmp == htons(port))
		{
			TAILQ_REMOVE(&host->packets_icmp, pkt, chain);
			free(pkt);
		}
	}
}

static int
tcp_finished(struct tcp_packet *pkt, int type)
{
	switch (type)
	{
		case SCAN_SYN:
			if ((pkt->tcp.th_flags & (TH_SYN | TH_ACK)) == (TH_SYN | TH_ACK))
				return 1;
			if ((pkt->tcp.th_flags & (TH_RST | TH_ACK)) == (TH_RST | TH_ACK))
				return 1;
			return 0;
		case SCAN_FIN:
			if (pkt->tcp.th_flags & TH_RST)
				return 1;
			return 0;
		case SCAN_XMAS:
			if (pkt->tcp.th_flags & TH_RST)
				return 1;
			return 0;
		case SCAN_NULL:
			if (pkt->tcp.th_flags & TH_RST)
				return 1;
			return 0;
		case SCAN_ACK:
			if (pkt->tcp.th_flags & TH_RST)
				return 1;
			return 0;
		case SCAN_WIN:
			if (pkt->tcp.th_flags & TH_RST)
				return 1;
			return 0;
		case SCAN_MAIM:
			if (pkt->tcp.th_flags & TH_RST)
				return 1;
			return 0;
	}
	return 0;
}

struct tcp_packet *
packet_get_tcp(struct env *env,
               struct host *host,
               uint16_t port,
               uint32_t sequence,
               int type)
{
	struct tcp_packet *pkt;

	recv_packets(env, host);
	TAILQ_FOREACH(pkt, &host->packets_tcp, chain)
	{
		if ((pkt->tcp.th_ack == sequence
		  || ntohl(pkt->tcp.th_ack) == ntohl(sequence) + 1)
		 && pkt->tcp.th_sport == htons(port)
		 && tcp_finished(pkt, type))
		{
			TAILQ_REMOVE(&host->packets_tcp, pkt, chain);
			return pkt;
		}
	}
	return NULL;
}

struct icmp_packet *
packet_get_icmp(struct env *env, struct host *host, uint16_t port)
{
	struct icmp_packet *pkt;

	recv_packets(env, host);
	TAILQ_FOREACH(pkt, &host->packets_icmp, chain)
	{
		uint16_t tmp;

		memcpy(&tmp, &pkt->data[4 + sizeof(struct ip) + 2], sizeof(tmp));
		if (tmp == htons(port))
		{
			TAILQ_REMOVE(&host->packets_icmp, pkt, chain);
			return pkt;
		}
	}
	return NULL;
}
