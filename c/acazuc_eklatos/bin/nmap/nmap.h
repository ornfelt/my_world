#ifndef NMAP_H
#define NMAP_H

#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/in.h>

#include <sys/queue.h>

#include <limits.h>
#include <stdint.h>
#include <stddef.h>

#define SCAN_SYN  (1 << 0)
#define SCAN_NULL (1 << 1)
#define SCAN_ACK  (1 << 2)
#define SCAN_FIN  (1 << 3)
#define SCAN_XMAS (1 << 4)
#define SCAN_UDP  (1 << 5)
#define SCAN_WIN  (1 << 6)
#define SCAN_MAIM (1 << 7)
#define SCAN_TCP  (SCAN_SYN | SCAN_NULL | SCAN_ACK | SCAN_FIN | SCAN_XMAS | SCAN_WIN | SCAN_MAIM)
#define SCAN_ALL  (SCAN_TCP | SCAN_UDP)

struct icmp_packet;
struct tcp_packet;

struct env
{
	const char *progname;
	struct host **hosts;
	size_t hosts_count;
	char **ips;
	size_t ips_count;
	char ports[USHRT_MAX + 1];
	uint16_t ports_count;
	struct in_addr local_ip;
	struct in_addr loopback_ip;
	uint16_t syn_port;
	uint16_t null_port;
	uint16_t ack_port;
	uint16_t fin_port;
	uint16_t xmas_port;
	uint16_t udp_port;
	uint16_t win_port;
	uint16_t maim_port;
	uint8_t scans;
	uint8_t scans_count;
	uint8_t trials;
	uint8_t timeout;
	int sock_pkt;
	int sock_raw;
};

enum port_status
{
	OPEN,
	FILTERED,
	CLOSED,
	UNFILTERED,
	OPEN_FILTERED,
};

struct port_result
{
	enum port_status status_syn;
	enum port_status status_null;
	enum port_status status_ack;
	enum port_status status_fin;
	enum port_status status_xmas;
	enum port_status status_udp;
	enum port_status status_win;
	enum port_status status_maim;
};

struct host
{
	char *host;
	char *ip;
	struct sockaddr *addr;
	size_t addrlen;
	struct port_result results[USHRT_MAX + 1];
	char scanning[USHRT_MAX + 1];
	TAILQ_HEAD(, tcp_packet) packets_tcp;
	TAILQ_HEAD(, icmp_packet) packets_icmp;
	char ended;
};

struct tcp_packet
{
	struct ip ip;
	struct tcphdr tcp;
	TAILQ_ENTRY(tcp_packet) chain;
};

struct udp_packet
{
	struct ip ip;
	struct udphdr udp;
	TAILQ_ENTRY(udp_packet) chain;
};

struct icmp_packet
{
	struct ip ip;
	struct icmphdr icmp;
	char data[sizeof(struct ip) + sizeof(struct udphdr)];
	TAILQ_ENTRY(icmp_packet) chain;
};

struct tcpudp_pseudohdr
{
	struct in_addr src;
	struct in_addr dst;
	uint8_t zero;
	uint8_t proto;
	uint16_t len;
} __attribute__ ((packed));

int scan_host(struct env *env, struct host *host);
int build_hosts(struct env *env);
void forge_iphdr(struct ip *ip, uint8_t proto, struct in_addr src,
                 struct in_addr dst, size_t len);
void forge_tcphdr_syn(struct env *env, struct tcp_packet *packet, uint16_t port);
void forge_tcphdr_null(struct env *env, struct tcp_packet *packet, uint16_t port);
void forge_tcphdr_ack(struct env *env, struct tcp_packet *packet, uint16_t port);
void forge_tcphdr_fin(struct env *env, struct tcp_packet *packet, uint16_t port);
void forge_tcphdr_xmas(struct env *env, struct tcp_packet *packet, uint16_t port);
void forge_tcphdr_win(struct env *env, struct tcp_packet *packet, uint16_t port);
void forge_tcphdr_maim(struct env *env, struct tcp_packet *packet, uint16_t port);
void forge_udphdr(struct env *env, struct udp_packet *packet, uint16_t port);
size_t epoch_micro(struct env *env);
void print_result(struct env *env, struct host *host);
void packet_flush_tcp(struct host *host, uint16_t port);
void packet_flush_icmp(struct host *host, uint16_t port);
struct tcp_packet *packet_get_tcp(struct env *env, struct host *host,
                                  uint16_t port, uint32_t sequence, int type);
struct icmp_packet *packet_get_icmp(struct env *env, struct host *host,
                                    uint16_t port);

#endif
