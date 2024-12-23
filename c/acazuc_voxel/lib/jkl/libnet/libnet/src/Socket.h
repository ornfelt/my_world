#ifndef LIBNET_SOCKET_H
# define LIBNET_SOCKET_H

# include "SocketPlatform.h"
# include "Protocol.h"
# include "Buffer.h"
# include <string>

namespace libnet
{

	class Socket
	{

		protected:
			union
			{
				struct sockaddr sockaddr;
				struct sockaddr_in sockaddr_in;
				struct sockaddr_in6 sockaddr_in6;
			} sockaddr_u;
			const Protocol *protocol;
			SOCKET sockfd;
			bool waitingConnection;
			bool connected;

		public:
			Socket(SOCKET sockfd, struct sockaddr sockaddr, const Protocol *protocol);
			Socket();
			virtual ~Socket();
			bool open(const Protocol *protocol);
			bool close();
			bool shutdown();
			bool connect(const std::string &host, uint16_t port);
			int getConnectionStatus();
			int32_t send(const void *data, int32_t len);
			int32_t send(Buffer &buffer);
			int32_t recv(void *data, int32_t len);
			int32_t recv(Buffer &buffer);
			bool setNagle(bool active);
			bool setBlocking(bool blocking);
			bool setRecvTimeout(uint64_t timeout);
			bool setSendTimeout(uint64_t timeout);
			size_t getIp(void *data);
			SOCKET getSockfd() {return this->sockfd;};

	};

}

#endif
