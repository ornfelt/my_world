#ifndef SCHED_H
#define SCHED_H

#include <bits/clone.h>

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

int sched_yield(void);
pid_t clone(int flags);

#ifdef __cplusplus
}
#endif

#endif
