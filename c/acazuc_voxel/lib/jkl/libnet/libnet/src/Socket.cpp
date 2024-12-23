#include "Socket.h"
#include <cstring>

namespace libnet
{

	Socket::Socket(SOCKET sockfd, struct sockaddr sockaddr, const Protocol *protocol)
	: protocol(protocol)
	, sockfd(sockfd)
	, waitingConnection(false)
	, connected(true)
	{
		this->sockaddr_u.sockaddr = sockaddr;
	}

	Socket::Socket()
	: protocol(nullptr)
	, waitingConnection(false)
	, connected(false)
	{
	}

	Socket::~Socket()
	{
		close();
	}

	bool Socket::open(const Protocol *protocol)
	{
		if (this->protocol)
			return false;
		if ((this->sockfd = ::socket(protocol->domain, protocol->type, protocol->protocol)) == INVALID_SOCKET)
			return false;
		this->protocol = protocol;
		return true;
	}

	bool Socket::close()
	{
		if (!this->protocol)
			return false;
		closesocket(this->sockfd);
		this->protocol = nullptr;
		this->connected = false;
		this->waitingConnection = false;
		return true;
	}

	bool Socket::shutdown()
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
		this->connected = false;
		this->waitingConnection = false;
		return true;
	}

	bool Socket::connect(const std::string &host, uint16_t port)
	{
		if (!this->protocol)
			return false;
		std::string service(std::to_string(port));
		struct addrinfo *result = nullptr;
		struct addrinfo hints;
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = this->protocol->domain;
		hints.ai_socktype = this->protocol->type;
		hints.ai_protocol = this->protocol->protocol;
		int rett = getaddrinfo(host.c_str(), service.c_str(), &hints, &result);
		if (rett)
			return false;
		if (!result)
			return false;
		switch (result->ai_family)
		{
			case AF_INET:
				std::memcpy(&this->sockaddr_u.sockaddr_in, result->ai_addr, sizeof(struct sockaddr_in));
				break;
			case AF_INET6:
				std::memcpy(&this->sockaddr_u.sockaddr_in6, result->ai_addr, sizeof(struct sockaddr_in6));
				break;
			default:
				return false;
		}
		int ret = ::connect(this->sockfd, result->ai_addr, result->ai_addrlen);
		freeaddrinfo(result);
		if (ret == SOCKET_ERROR)
		{
#ifdef LIBNET_PLATFORM_WINDOWS
			if (WSAGetLastError() != WSAEWOULDBLOCK)
				return false;
#elif defined LIBNET_PLATFORM_LINUX
			if (errno != EINPROGRESS)
				return false;
#else
#error Platform not supported
#endif
			this->waitingConnection = true;
		}
		else
		{
			this->connected = true;
		}
		return true;
	}

	int Socket::getConnectionStatus()
	{
		if (this->connected)
			return 1;
		if (!this->waitingConnection)
			return -1;
		struct timeval tv;
		fd_set fdset;
		int ret;
		tv.tv_sec = 0;
		tv.tv_usec = 1;
		FD_ZERO(&fdset);
		FD_SET(this->sockfd, &fdset);
		if ((ret = select(this->sockfd + 1, NULL, &fdset, &fdset, &tv)))
		{
			this->waitingConnection = false;
			if (ret == SOCKET_ERROR)
				return -1;
			int err;
			socklen_t len = sizeof(err);
			if (getsockopt(this->sockfd, SOL_SOCKET, SO_ERROR, (char*)&err, &len) == SOCKET_ERROR)
				return -1;
#ifdef LIBNET_PLATFORM_WINDOWS
			if (err && err != WSAEINPROGRESS)
				return -1;
#elif defined LIBNET_PLATFORM_LINUX
			if (err && err != EINPROGRESS)
				return -1;
#else
#error Unsupported platform
#endif
			this->connected = true;
			return 1;
		}
		return 0;
	}

	int32_t Socket::send(const void *data, int32_t len)
	{
		if (!this->connected)
			throw std::exception();
		if (!len)
			return 0;
		int32_t written = ::send(this->sockfd, reinterpret_cast<const char*>(data), len, 0);
		if (written == SOCKET_ERROR)
		{
#ifdef LIBNET_PLATFORM_WINDOWS
			if (WSAGetLastError() != WSAEWOULDBLOCK)
				throw std::exception();
#elif defined LIBNET_PLATFORM_LINUX
			if (errno != EWOULDBLOCK && errno != EAGAIN)
				throw std::exception();
#else
#error Platform not supported
#endif
			written = -1;
		}
		return written;
	}

	int32_t Socket::send(Buffer &buffer)
	{
		buffer.flip();
		if (!buffer.getRemaining())
		{
			buffer.clear();
			return 0;
		}
		int32_t written = send(buffer.getData().data() + buffer.getPosition(), buffer.getRemaining());
		if (written > 0)
			buffer.setPosition(buffer.getPosition() + written);
		if (buffer.getRemaining())
		{
			if (buffer.getPosition())
				std::memmove(buffer.getData().data(), buffer.getData().data() + buffer.getPosition(), buffer.getRemaining());
			buffer.setPosition(buffer.getRemaining());
			buffer.setLimit(buffer.getData().size());
		}
		else
		{
			buffer.clear();
		}
		return written;
	}

	int32_t Socket::recv(void *data, int32_t len)
	{
		if (!this->connected)
			throw std::exception();
		if (!len)
			return -1;
		int32_t readed = ::recv(this->sockfd, reinterpret_cast<char*>(data), len, 0);
		if (readed == SOCKET_ERROR)
		{
#ifdef LIBNET_PLATFORM_WINDOWS
			if (WSAGetLastError() != WSAEWOULDBLOCK)
				throw std::exception();
#elif defined LIBNET_PLATFORM_LINUX
			if (errno != EWOULDBLOCK && errno != EAGAIN)
				throw std::exception();
#else
#error Platform not supported
#endif
			readed = -1;
		}
		return readed;
	}

	int32_t Socket::recv(Buffer &buffer)
	{
		if (buffer.getRemaining())
		{
			if (buffer.getPosition())
				std::memmove(buffer.getData().data(), buffer.getData().data() + buffer.getPosition(), buffer.getRemaining());
			buffer.setPosition(buffer.getRemaining());
			buffer.setLimit(buffer.getData().size());
		}
		else
		{
			buffer.clear();
		}
		int32_t readed = recv(buffer.getData().data() + buffer.getPosition(), buffer.getRemaining());
		if (readed > 0)
		{
			if (buffer.isCrypted())
				buffer.crypt(buffer.getPosition(), readed);
			buffer.setPosition(buffer.getPosition() + readed);
		}
		buffer.flip();
		return readed;
	}

	bool Socket::setNagle(bool active)
	{
		if (!this->protocol)
			return false;
		int flag = active ? 1 : 0;
		return setsockopt(this->sockfd, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(const_cast<const int*>(&flag)), sizeof(flag)) == 0;
	}

	bool Socket::setBlocking(bool blocking)
	{
		if (!this->protocol)
			return false;
#ifdef LIBNET_PLATFORM_WINDOWS
		u_long mode = blocking ? 0 : 1;
		return ioctlsocket(this->sockfd, FIONBIO, &mode) == 0;
#elif defined LIBNET_PLATFORM_LINUX
		int flags = fcntl(this->sockfd, F_GETFL, 0);
		if (flags < 0)
			return false;
		flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
		return fcntl(this->sockfd, F_SETFL, flags) == 0;
#else
#error Platform not supported
#endif
	}

	bool Socket::setRecvTimeout(uint64_t timeout)
	{
		if (!this->protocol)
			return false;
#ifdef LIBNET_PLATFORM_LINUX
		struct timeval tv;
		tv.tv_sec = timeout / 1000;
		tv.tv_usec = timeout % 1000;
		return setsockopt(this->sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == 0;
#elif defined LIBNET_PLATFORM_WINDOWS
		int time = timeout;
		return setsockopt(this->sockfd, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(const_cast<const int*>(&time)), sizeof(time)) == 0;
#else
#error Platform not supported
#endif
	}

	bool Socket::setSendTimeout(uint64_t timeout)
	{
		if (!this->protocol)
			return false;
#ifdef LIBNET_PLATFORM_LINUX
		struct timeval tv;
		tv.tv_sec = timeout / 1000;
		tv.tv_usec = timeout % 1000;
		return setsockopt(this->sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) == 0;
#elif defined LIBNET_PLATFORM_WINDOWS
		int time = timeout;
		return setsockopt(this->sockfd, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(const_cast<const int*>(&time)), sizeof(time)) == 0;
#else
#error Platform not supported
#endif
	}

	size_t Socket::getIp(void *data)
	{
		switch (this->sockaddr_u.sockaddr.sa_family)
		{
			case AF_INET:
				std::memcpy(data, &this->sockaddr_u.sockaddr_in.sin_addr, 4);
				return 4;
			case AF_INET6:
				std::memcpy(data, &this->sockaddr_u.sockaddr_in6.sin6_addr, 16);
				return 16;
		}
		return -1;
	}

}
