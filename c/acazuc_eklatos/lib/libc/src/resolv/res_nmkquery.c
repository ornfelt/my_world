#include <arpa/nameser.h>
#include <arpa/inet.h>

#include <resolv.h>
#include <stdlib.h>

struct ns_header
{
	uint16_t ident;
	uint16_t flags;
	uint16_t nreq;
	uint16_t nrep;
	uint16_t nauth;
	uint16_t nadd;
};

int res_nmkquery(res_state state, int op, const char *name, int class,
                 int type, const uint8_t *data, int len,
                 const uint8_t *newrr, uint8_t *buf, int buflen)
{
	(void)state;
	(void)newrr;
	(void)data;
	(void)len;
	struct ns_header *header = (struct ns_header*)&buf[0];
	if (buflen < 0 || (size_t)buflen < sizeof(*header) + 4)
		return -1;
	if (op != QUERY)
		return -1;
	header->ident = htons(rand());
	header->flags = htons((op << 11) | (1 << 8)); /* rd */
	header->nreq = htons(1);
	header->nrep = htons(0);
	header->nauth = htons(0);
	header->nadd = htons(0);
	uint8_t *ptr = &buf[sizeof(*header)];
	buflen -= sizeof(*header);
	int dn_len = dn_comp(name, ptr,buflen, NULL, NULL);
	if (dn_len == -1)
		return -1;
	ptr += dn_len;
	buflen -= dn_len;
	if (buflen < 4)
		return -1;
	*(uint16_t*)ptr = htons(class);
	ptr += 2;
	*(uint16_t*)ptr = htons(type);
	ptr += 2;
	return ptr - buf;
}
