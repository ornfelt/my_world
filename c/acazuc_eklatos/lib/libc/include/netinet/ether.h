#ifndef NETINET_ETHER_H
#define NETINET_ETHER_H

#ifdef __cplusplus
extern "C" {
#endif

struct ether_addr;

char *ether_ntoa(const struct ether_addr *addr);

#ifdef __cplusplus
}
#endif

#endif
