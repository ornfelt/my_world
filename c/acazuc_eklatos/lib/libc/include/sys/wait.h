#ifndef SYS_WAIT_H
#define SYS_WAIT_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define WNOHANG    (1 << 0)
#define WUNTRACED  (1 << 1)
#define WCONTINUED (1 << 2)

#define WIFEXITED(wstatus)    (WTERMSIG(wstatus) == 0)
#define WEXITSTATUS(wstatus)  (((wstatus) >> 8) & 0xFF)
#define WIFSIGNALED(wstatus)  (WTERMSIG(wstatus) != 0 && WTERMSIG(wstatus) != 0x7F)
#define WTERMSIG(wstatus)     ((wstatus) & 0x7F)
#define WCOREDUMP(wstatus)    ((wstatus) & 0x80)
#define WIFSTOPPED(wstatus)   (((wstatus) & 0xFF) == 0x7F)
#define WSTOPSIG(wstatus)     WEXITSTATUS(wstatus)
#define WIFCONTINUED(wstatus) 0

struct rusage;

pid_t waitpid(pid_t pid, int *wstatus, int options);
pid_t wait(int *wstatus);
pid_t wait3(int *wstatus, int options, struct rusage *rusage);
pid_t wait4(pid_t pid, int *wstatus, int options, struct rusage *rusage);

#ifdef __cplusplus
}
#endif

#endif
