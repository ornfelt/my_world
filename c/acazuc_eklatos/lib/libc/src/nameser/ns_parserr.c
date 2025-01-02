#include <arpa/nameser.h>
#include <arpa/inet.h>

#include <resolv.h>

static int nextrr(ns_msg *msg, ns_sect sect, ns_rr *rr, const uint8_t **ptr)
{
	int ret;

	ret = dn_expand(msg->msg, msg->eom, *ptr, rr->name, sizeof(rr->name));
	if (ret < 0)
		return -1;
	*ptr += ret;
	if (*ptr + 4 > msg->eom)
		return -1;
	rr->type = ntohs(*(uint16_t*)&(*ptr)[0]);
	rr->class = ntohs(*(uint16_t*)&(*ptr)[2]);
	*ptr += 4;
	if (sect == ns_s_qd)
		return 0;
	if (*ptr + 6 > msg->eom)
		return -1;
	rr->ttl = ntohl(*(uint32_t*)&(*ptr)[0]);
	rr->rdlen = ntohs(*(uint16_t*)&(*ptr)[4]);
	*ptr += 6;
	if (*ptr + rr->rdlen > msg->eom)
		return -1;
	rr->rdata = *ptr;
	*ptr += rr->rdlen;
	return 0;
}

int ns_parserr(ns_msg *msg, ns_sect sect, int id, ns_rr *rr)
{
	const uint8_t *ptr;

	ptr = &msg->msg[12];
	for (ns_sect s = 0; s < sect; ++s)
	{
		for (int i = 0; i < msg->counts[s]; ++i)
		{
			if (nextrr(msg, s, rr, &ptr) < 0)
				return -1;
		}
	}
	for (int i = 0; i < id; ++i)
	{
		if (nextrr(msg, sect, rr, &ptr) < 0)
			return -1;
	}
	return nextrr(msg, sect, rr, &ptr);
}
