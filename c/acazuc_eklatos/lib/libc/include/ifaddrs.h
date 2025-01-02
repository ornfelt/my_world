#ifndef IFADDRS_H
#define IFADDRS_H

#ifdef __cplusplus
extern "C" {
#endif

struct sockaddr;

struct ifaddrs
{
	struct ifaddrs *ifa_next;
	char *ifa_name;
	unsigned int ifa_flags;
	struct sockaddr *ifa_addr;
	struct sockaddr *ifa_netmask;
	struct sockaddr *ifa_dstaddr;
	void *ifa_data;
};

int getifaddrs(struct ifaddrs **addrs);
void freeifaddrs(struct ifaddrs *addrs);

#ifdef __cplusplus
}
#endif

#endif
