#ifndef WAIT_H
#define WAIT_H

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

#endif
