#ifndef POLL_H
#define POLL_H

#include <signal.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define POLLIN   (1 << 0)
#define POLLPRI  (1 << 1)
#define POLLOUT  (1 << 2)
#define POLLERR  (1 << 3)
#define POLLHUP  (1 << 4)
#define POLLNVAL (1 << 5)

typedef unsigned long nfds_t;

struct pollfd
{
	int fd;
	short events;
	short revents;
};

int poll(struct pollfd *fds, nfds_t nfds, int timeout);
int ppoll(struct pollfd *fds, nfds_t nfds, const struct timespec *timeout,
          const sigset_t *sigmask);

#ifdef __cplusplus
}
#endif

#endif
