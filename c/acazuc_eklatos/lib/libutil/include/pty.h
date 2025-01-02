#ifndef PTY_H
#define PTY_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct termios;
struct winsize;

int openpty(int *mainfd, int *childfd, char *name,
            const struct termios *termios, const struct winsize *winsize);
pid_t forkpty(int *mainfd, int *childfd, const struct termios *termios,
              const struct winsize *winsize);

#ifdef __cplusplus
}
#endif

#endif
