#include "_hostent.h"

#include <arpa/nameser.h>
#include <arpa/inet.h>

#include <string.h>
#include <resolv.h>
#include <netdb.h>

static struct hostent *
resolve_dns(const char *name)
{
	struct __res_state state;
	uint8_t buf[4096];
	int ret;
	int len;
	ns_msg msg;

	ret = res_ninit(&state);
	if (ret)
		return NULL;
	len = res_nquery(&state, name, ns_c_in, ns_t_a, buf, sizeof(buf));
	res_nclose(&state);
	if (len < 0)
		return NULL;
	ret = ns_initparse(buf, len, &msg);
	if (ret < 0)
		return NULL;
	if (ns_msg_getflag(msg, ns_f_rcode))
		return NULL;
	for (int sect = 0; sect < ns_s_max; ++sect)
	{
		int count;

		count = ns_msg_count(msg, sect);
		for (int i = 0; i < count; ++i)
		{
			static char hostent_name[256];
			static struct in_addr hostent_addr;
			static char *hostent_addr_list[256];
			static struct hostent hostent;
			ns_rr rr;

			if (ns_parserr(&msg, sect, i, &rr))
				return NULL;
			if (sect == ns_s_qd)
				continue;
			if (ns_rr_type(rr) != ns_t_a)
				continue;
			if (ns_rr_rdlen(rr) != 4)
				continue;
			/* XXX multiple addr */
			hostent_addr = *(struct in_addr*)ns_rr_rdata(rr);
			strlcpy(hostent_name, name, sizeof(hostent_name));
			hostent.h_name = hostent_name;
			hostent.h_aliases = NULL;
			hostent.h_addrtype = AF_INET;
			hostent.h_length = sizeof(struct in_addr);
			hostent_addr_list[0] = (char*)&hostent_addr;
			hostent_addr_list[1] = NULL;
			hostent.h_addr_list = &hostent_addr_list[0];
			return &hostent;
		}
	}
	return NULL;
}

struct hostent *
gethostbyname(const char *name)
{
	static char ip_hostent_name[256];
	static struct in_addr ip_hostent_addr;
	static char *ip_hostent_addr_list[256];
	static struct hostent ip_hostent;
	struct hostent *hostent;

	if (!name)
		return NULL;
	sethostent(0);
	if (!hostent_fp)
		return resolve_dns(name);
	while (1)
	{
		hostent = next_hostent();
		if (!hostent)
			break;
		if (strcmp(hostent->h_name, name))
			continue;
		break;
	}
	if (!hostent_stayopen)
		endhostent();
	if (hostent)
		return hostent;
	if (inet_aton(name, &ip_hostent_addr) != 1)
		return resolve_dns(name);
	strlcpy(ip_hostent_name, name, sizeof(ip_hostent_name));
	ip_hostent.h_name = ip_hostent_name;
	ip_hostent.h_aliases = NULL;
	ip_hostent.h_addrtype = AF_INET;
	ip_hostent.h_length = sizeof(struct in_addr);
	ip_hostent_addr_list[0] = (char*)&ip_hostent_addr;
	ip_hostent_addr_list[1] = NULL;
	ip_hostent.h_addr_list = &ip_hostent_addr_list[0];
	return &ip_hostent;
}
