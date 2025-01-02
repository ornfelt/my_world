#ifndef RESOURCE_H
#define RESOURCE_H

#include <types.h>
#include <time.h>

#define PRIO_PROCESS 0
#define PRIO_PGRP    1
#define PRIO_USER    2

#define RLIMIT_AS      0
#define RLIMIT_CORE    1
#define RLIMIT_CPU     2
#define RLIMIT_DATA    3
#define RLIMIT_FSIZE   4
#define RLIMIT_MEMLOCK 5
#define RLIMIT_NOFILE  6
#define RLIMIT_NPROC   7
#define RLIMIT_RSS     8
#define RLIMIT_STACK   9

#define RLIM_INFINITY ((rlim_t)-1)

#define RUSAGE_SELF      0
#define RUSAGE_CHILDREN -1
#define RUSAGE_THREAD   -2

typedef int64_t rlim_t;

struct rlimit
{
	rlim_t rlim_cur;
	rlim_t rlim_max;
};

struct rusage
{
	struct timeval ru_utime;
	struct timeval ru_stime;
	long ru_maxrss;
	long ru_ixrss;
	long ru_idrss;
	long ru_isrss;
	long ru_minflt;
	long ru_majflt;
	long ru_nswap;
	long ru_inblock;
	long ru_outblock;
	long ru_msgsnd;
	long ru_msgrcv;
	long ru_nsignals;
	long ru_nvcsw;
	long ru_nivcsw;
};

#endif
