#ifndef SYS_SELECT_H
#define SYS_SELECT_H

#include <signal.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FD_SETSIZE 1024
#define FDSET_WORDS (FD_SETSIZE / FDSET_BPW)
#define FDSET_BPW (8)

#define FD_ZERO(fds) \
do \
{ \
	for (size_t _fds_i = 0; _fds_i < FDSET_WORDS; ++_fds_i) \
		(fds)->values[_fds_i] = 0; \
} while (0)

#define FD_CLR(fd, fds) \
do \
{ \
	(fds)->values[(fd) / FDSET_BPW] &= ~(1 << ((fd) % FDSET_BPW)); \
} while (0)

#define FD_SET(fd, fds) \
do \
{ \
	(fds)->values[(fd) / FDSET_BPW] |= 1 << ((fd) % FDSET_BPW); \
} while (0)

#define FD_ISSET(fd, fds) \
	((fds)->values[(fd) / FDSET_BPW] & (1 << ((fd) % FDSET_BPW)))

typedef struct
{
	uint8_t values[FDSET_WORDS];
} fd_set;

int select(int nfds, fd_set *rfds, fd_set *wfds, fd_set *efds,
           struct timeval *timeout);
int pselect(int nfds, fd_set *rfds, fd_set *wfds, fd_set *efds,
            const struct timespec *timeout, const sigset_t *sigmask);

#ifdef __cplusplus
}
#endif

#endif
