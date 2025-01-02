#ifndef NET_ETHER_H
#define NET_ETHER_H

#include <types.h>

#define ETHER_ADDR_LEN 6

#define ETHERTYPE_IP  0x0800
#define ETHERTYPE_ARP 0x0806
#define ETHERTYPE_IP6 0x86DD

/* XXX this macro name.... */
#define MAC_PRINTF(mac) (mac)[0], \
                        (mac)[1], \
                        (mac)[2], \
                        (mac)[3], \
                        (mac)[4], \
                        (mac)[5]

/* XXX this macro name.... */
#define MAC_FMT "%02" PRIx8 ":%02" PRIx8 ":%02" PRIx8 ":%02" PRIx8 ":%02" PRIx8 ":%02" PRIx8

struct ether_addr
{
	uint8_t addr[ETHER_ADDR_LEN]
} __attribute__ ((packed));

struct ether_header
{
	uint8_t ether_dhost[ETHER_ADDR_LEN];
	uint8_t ether_shost[ETHER_ADDR_LEN];
	uint16_t ether_type;
} __attribute__ ((packed));

#endif
