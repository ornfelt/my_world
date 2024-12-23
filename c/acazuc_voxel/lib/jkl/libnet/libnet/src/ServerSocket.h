#ifndef LIBNET_SERVER_SOCKET_H
# define LIBNET_SERVER_SOCKET_H

# include "SocketPlatform.h"
# include "Protocol.h"
# include "Socket.h"
# include <cstdint>

namespace libnet
{

	class ServerSocket
	{

		private:
			union
			{
				struct sockaddr sockaddr;
				struct sockaddr_in sockaddr_in;
				struct sockaddr_in6 sockaddr_in6;
			} sockaddr_u;
			const Protocol *protocol;
			SOCKET sockfd;
			bool bound;

		public:
			ServerSocket();
			~ServerSocket();
			bool open(const Protocol *protocol);
			bool close();
			bool shutdown();
			bool bind(uint16_t port);
			Socket *accept();
			bool setNagle(bool active);
			bool setBlocking(bool blocking);
			inline SOCKET getSockfd() {return this->sockfd;};

	};

}

#endif
