#include "Protocol.h"
#include "SocketPlatform.h"

namespace libnet
{

	const Protocol protocolIPv4TCP = {AF_INET, SOCK_STREAM, IPPROTO_TCP};
	const Protocol protocolIPv6TCP = {AF_INET6, SOCK_STREAM, IPPROTO_TCP};
	const Protocol protocolIPv4UDP = {AF_INET, SOCK_DGRAM, IPPROTO_UDP};
	const Protocol protocolIPv6UDP = {AF_INET6, SOCK_DGRAM, IPPROTO_UDP};

}
