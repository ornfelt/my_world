#include <arpa/nameser.h>

int ns_msg_getflag(ns_msg msg, enum ns_flag flag)
{
	switch (flag)
	{
		case ns_f_qr:
			return (msg.flags >> 15) & 0x1;
		case ns_f_opcode:
			return (msg.flags >> 11) & 0xF;
		case ns_f_aa:
			return (msg.flags >> 10) & 0x1;
		case ns_f_tc:
			return (msg.flags >> 9) & 0x1;
		case ns_f_rd:
			return (msg.flags >> 8) & 0x1;
		case ns_f_ra:
			return (msg.flags >> 7) & 0x1;
		case ns_f_z:
			return (msg.flags >> 4) & 0x7;
		case ns_f_rcode:
			return (msg.flags >> 0) & 0xF;
	}
	return -1;
}
