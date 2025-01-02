#ifndef NETINET_UDP_H
#define NETINET_UDP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct udphdr
{
	uint16_t uh_sport;
	uint16_t uh_dport;
	uint16_t uh_ulen;
	uint16_t uh_sum;
};

#ifdef __cplusplus
}
#endif

#endif
