#include <arpa/nameser.h>
#include <arpa/inet.h>

int
ns_initparse(const uint8_t *data, int len, ns_msg *msg)
{
	if (len < 12)
		return -1;
	msg->ident = ntohs(*(uint16_t*)&data[0]);
	msg->flags = ntohs(*(uint16_t*)&data[2]);
	msg->counts[0] = ntohs(*(uint16_t*)&data[4]);
	msg->counts[1] = ntohs(*(uint16_t*)&data[6]);
	msg->counts[2] = ntohs(*(uint16_t*)&data[8]);
	msg->counts[3] = ntohs(*(uint16_t*)&data[10]);
	msg->msg = data;
	msg->eom = data + len;
	return 0;
}
