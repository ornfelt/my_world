#include "nmap.h"

#include <arpa/inet.h>

#include <stdlib.h>
#include <string.h>

void forge_iphdr(struct ip *ip, uint8_t proto, struct in_addr src,
                 struct in_addr dst, size_t len)
{
	memset(ip, 0, sizeof(*ip));
	ip->ip_v = 4;
	ip->ip_hl = 5;
	ip->ip_tos = 16;
	ip->ip_len = len;
	ip->ip_id = rand();
	ip->ip_off = 0;
	ip->ip_ttl = 255;
	ip->ip_p = proto;
	ip->ip_dst = dst;
	ip->ip_src = src;
	ip->ip_sum = 0;
}

static uint16_t tcp_checksum(struct tcp_packet *pkt)
{
	uint16_t len = sizeof(*pkt) - sizeof(pkt->ip) - sizeof(pkt->chain);
	struct tcpudp_pseudohdr phdr;
	phdr.src = pkt->ip.ip_src;
	phdr.dst = pkt->ip.ip_dst;
	phdr.zero = 0;
	phdr.proto = pkt->ip.ip_p;
	phdr.len = htons(len);

	uint64_t result;
	result  = ((uint16_t*)&phdr)[0];
	result += ((uint16_t*)&phdr)[1];
	result += ((uint16_t*)&phdr)[2];
	result += ((uint16_t*)&phdr)[3];
	result += ((uint16_t*)&phdr)[4];
	result += ((uint16_t*)&phdr)[5];
	uint16_t *tmp = (uint16_t*)&pkt->tcp;
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

static void forge_tcphdr_common(struct tcphdr *tcp, uint16_t sport,
                                uint16_t dport)
{
	memset(tcp, 0, sizeof(*tcp));
	tcp->th_sport = htons(sport);
	tcp->th_dport = htons(dport);
	tcp->th_seq = rand();
	tcp->th_x2 = 0;
	tcp->th_off = 5;
	tcp->th_flags = 0;
	tcp->th_win = htons(1024);
	tcp->th_sum = 0;
	tcp->th_urp = 0;
}

void forge_tcphdr_syn(struct env *env, struct tcp_packet *packet,
                      uint16_t port)
{
	forge_tcphdr_common(&packet->tcp, env->syn_port, port);
	packet->tcp.th_flags |= TH_SYN;
	packet->tcp.th_sum = tcp_checksum(packet);
}

void forge_tcphdr_null(struct env *env, struct tcp_packet *packet,
                       uint16_t port)
{
	forge_tcphdr_common(&packet->tcp, env->null_port, port);
	packet->tcp.th_sum = tcp_checksum(packet);
}

void forge_tcphdr_ack(struct env *env, struct tcp_packet *packet,
                      uint16_t port)
{
	forge_tcphdr_common(&packet->tcp, env->ack_port, port);
	packet->tcp.th_flags |= TH_ACK;
	packet->tcp.th_sum = tcp_checksum(packet);
}

void forge_tcphdr_fin(struct env *env, struct tcp_packet *packet,
                      uint16_t port)
{
	forge_tcphdr_common(&packet->tcp, env->fin_port, port);
	packet->tcp.th_flags |= TH_FIN;
	packet->tcp.th_sum = tcp_checksum(packet);
}

void forge_tcphdr_xmas(struct env *env, struct tcp_packet *packet,
                       uint16_t port)
{
	forge_tcphdr_common(&packet->tcp, env->xmas_port, port);
	packet->tcp.th_flags |= TH_FIN | TH_PUSH | TH_URG;
	packet->tcp.th_sum = tcp_checksum(packet);
}

void forge_tcphdr_win(struct env *env, struct tcp_packet *packet,
                      uint16_t port)
{
	forge_tcphdr_common(&packet->tcp, env->win_port, port);
	packet->tcp.th_flags |= TH_ACK;
	packet->tcp.th_sum = tcp_checksum(packet);
}

void forge_tcphdr_maim(struct env *env, struct tcp_packet *packet,
                      uint16_t port)
{
	forge_tcphdr_common(&packet->tcp, env->win_port, port);
	packet->tcp.th_flags |= TH_FIN | TH_ACK;
	packet->tcp.th_sum = tcp_checksum(packet);
}

static uint16_t udp_checksum(struct udp_packet *pkt)
{
	uint16_t len = sizeof(*pkt) - sizeof(pkt->ip) - sizeof(pkt->chain);
	struct tcpudp_pseudohdr phdr;
	phdr.src = pkt->ip.ip_src;
	phdr.dst = pkt->ip.ip_dst;
	phdr.zero = 0;
	phdr.proto = pkt->ip.ip_p;
	phdr.len = htons(len);

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

void forge_udphdr(struct env *env, struct udp_packet *packet, uint16_t port)
{
	memset(&packet->udp, 0, sizeof(packet->udp));
	packet->udp.uh_sport = htons(env->udp_port);
	packet->udp.uh_dport = htons(port);
	packet->udp.uh_ulen = htons(sizeof(*packet) - sizeof(packet->ip) - sizeof(packet->chain));
	packet->udp.uh_sum = udp_checksum(packet);
}
