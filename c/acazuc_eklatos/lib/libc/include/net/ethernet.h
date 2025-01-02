#ifndef NET_ETHERNET_H
#define NET_ETHERNET_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ETHER_ADDR_LEN 6

#define ETHERTYPE_IP  0x0800
#define ETHERTYPE_ARP 0x0806

struct ether_addr
{
	uint8_t addr[ETHER_ADDR_LEN];
} __attribute__ ((packed));

struct ether_header
{
	uint8_t ether_dhost[ETHER_ADDR_LEN];
	uint8_t ether_shost[ETHER_ADDR_LEN];
	uint16_t ether_type;
} __attribute__ ((packed));

#ifdef __cplusplus
}
#endif

#endif
