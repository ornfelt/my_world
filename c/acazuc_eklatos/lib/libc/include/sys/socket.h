#ifndef SYS_SOCKET_H
#define SYS_SOCKET_H

#include <sys/types.h>
#include <sys/uio.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PF_UNSPEC 0
#define PF_UNIX   1
#define PF_LOCAL  PF_UNIX
#define PF_INET   2
#define PF_INET6  10
#define PF_PACKET 17

#define AF_UNSPEC PF_UNSPEC
#define AF_UNIX   PF_UNIX
#define AF_LOCAL  PF_LOCAL
#define AF_INET   PF_INET
#define AF_INET6  PF_INET6
#define AF_PACKET PF_PACKET

#define SOCK_STREAM 1
#define SOCK_DGRAM  2
#define SOCK_RAW    3

#define SOL_SOCKET 1

#define SO_RCVTIMEO 1
#define SO_SNDTIMEO 2

#define MSG_DONTWAIT (1 << 0)

#define SHUT_RD   0
#define SHUT_WR   1
#define SHUT_RDWR 2

typedef uint32_t socklen_t;
typedef uint16_t sa_family_t;

struct sockaddr
{
	sa_family_t sa_family;
	char sa_data[14];
};

struct msghdr
{
	void *msg_name;
	socklen_t msg_namelen;
	struct iovec *msg_iov;
	size_t msg_iovlen;
	void *msg_control;
	size_t msg_controllen;
	int msg_flags;
};

int socket(int domain, int type, int protocol);
int socketpair(int domain, int type, int protocol, int fds[2]);
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int listen(int sockfd, int backlog);
ssize_t recv(int fd, void *data, size_t count, int flags);
ssize_t recvfrom(int fd, void *data, size_t count, int flags,
                 struct sockaddr *addr, socklen_t *addrlen);
ssize_t recvmsg(int fd, struct msghdr *msg, int flags);
ssize_t send(int fd, const void *data, size_t count, int flags);
ssize_t sendto(int fd, const void *data, size_t count, int flags,
               const struct sockaddr *addr, socklen_t addrlen);
ssize_t sendmsg(int fd, const struct msghdr *msg, int flags);

int getsockopt(int fd, int level, int opt, void *val, socklen_t *len);
int setsockopt(int fd, int level, int opt, const void *val, socklen_t len);

int getpeername(int fd, struct sockaddr *addr, socklen_t *len);
int getsockname(int fd, struct sockaddr *addr, socklen_t *len);

int shutdown(int fd, int how);

#ifdef __cplusplus
}
#endif

#endif
