#ifndef NETINET_IP_ICMP_H
#define NETINET_IP_ICMP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ICMP_ECHOREPLY     0
#define ICMP_DEST_UNREACH  3
#define ICMP_ECHO          8
#define ICMP_TIME_EXCEEDED 11
#define ICMP_PARAMPROB     12

struct icmphdr
{
	uint8_t icmp_type;
	uint8_t icmp_code;
	uint16_t icmp_cksum;
};

#ifdef __cplusplus
}
#endif

#endif
