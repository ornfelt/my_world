#include "ServerSocket.h"
#include <cstring>

namespace libnet
{

	ServerSocket::ServerSocket()
	: protocol(nullptr)
	, bound(false)
	{
	}

	ServerSocket::~ServerSocket()
	{
		close();
	}

	bool ServerSocket::open(const Protocol *protocol)
	{
		if (this->protocol)
			return false;
		if ((this->sockfd = ::socket(protocol->domain, protocol->type, protocol->protocol)) == INVALID_SOCKET)
			return false;
		int flag = 1;
		::setsockopt(this->sockfd, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&flag, sizeof(flag));
		this->protocol = protocol;
		return true;
	}

	bool ServerSocket::close()
	{
		if (!this->protocol)
			return false;
		closesocket(this->sockfd);
		this->protocol = nullptr;
		this->bound = false;
		return true;
	}

	bool ServerSocket::shutdown()
	{
		if (!this->protocol)
			return false;
#ifdef LIBNET_PLATFORM_WINDOWS
		::shutdown(this->sockfd, SD_BOTH);
#elif defined LIBNET_PLATFORM_LINUX
		::shutdown(this->sockfd, SHUT_RDWR);
#else
#error Platform not supported
#endif
		return true;
	}

	bool ServerSocket::bind(uint16_t port)
	{
		if (!this->protocol)
			return false;
		std::memset(&this->sockaddr_u, 0, sizeof(this->sockaddr_u));
		SOCKLEN_T socklen;
		switch (this->protocol->domain)
		{
			case AF_INET:
				this->sockaddr_u.sockaddr_in.sin_family = AF_INET;
				this->sockaddr_u.sockaddr_in.sin_addr.s_addr = INADDR_ANY;
				this->sockaddr_u.sockaddr_in.sin_port = htons(port);
				socklen = sizeof(this->sockaddr_u.sockaddr_in);
				break;
			case AF_INET6:
				this->sockaddr_u.sockaddr_in6.sin6_family = AF_INET6;
				std::memset(&this->sockaddr_u.sockaddr_in6.sin6_addr, 0, sizeof(this->sockaddr_u.sockaddr_in6.sin6_addr));
				this->sockaddr_u.sockaddr_in6.sin6_port = htons(port);
				socklen = sizeof(this->sockaddr_u.sockaddr_in6);
				break;
			default:
				return false;
		}
		if (::bind(this->sockfd, &this->sockaddr_u.sockaddr, socklen) == SOCKET_ERROR)
			return false;
		listen(this->sockfd, 255);
		this->bound = true;
		return true;
	}

	Socket *ServerSocket::accept()
	{
		struct sockaddr sockaddr;
		SOCKLEN_T socklen;
		SOCKET newsockfd;
		if (!this->bound)
			return nullptr;
		socklen = sizeof(sockaddr);
		if ((newsockfd = ::accept(this->sockfd, &sockaddr, &socklen)) == INVALID_SOCKET)
			return nullptr;
		return new Socket(newsockfd, sockaddr, this->protocol);
	}

	bool ServerSocket::setNagle(bool active)
	{
		if (!this->protocol)
			return false;
		int flag = active ? 1 : 0;
		return setsockopt(this->sockfd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag)) == 0;
	}

	bool ServerSocket::setBlocking(bool blocking)
	{
		if (!this->protocol)
			return false;
#ifdef LIBNET_PLATFORM_WINDOWS
		u_long mode = blocking ? 0 : 1;
		return ioctlsocket(this->sockfd, FIONBIO, &mode) == 0;
#elif defined LIBNET_PLATFORM_LINUX
		int flags = fcntl(this->sockfd, F_GETFL, 0);
		if (flags == -1)
			return false;
		flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
		return fcntl(this->sockfd, F_SETFL, flags) == 0;
#else
#error Platform not supported
#endif
	}

}
