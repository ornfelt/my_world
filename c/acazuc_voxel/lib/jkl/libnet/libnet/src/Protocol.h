#ifndef LIBNET_PROTOCOL_H
# define LIBNET_PROTOCOL_H

namespace libnet
{

	struct Protocol
	{
		int domain;
		int type;
		int protocol;
	};

	extern const Protocol protocolIPv4TCP;
	extern const Protocol protocolIPv6TCP;
	extern const Protocol protocolIPv4UDP;
	extern const Protocol protocolIPv6UDP;

}

#endif
