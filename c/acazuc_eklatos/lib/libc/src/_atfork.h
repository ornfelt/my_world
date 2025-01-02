#ifndef _ATFORK_H
#define _ATFORK_H

#include <sys/types.h>

#define ATFORK_MAX 1024

extern void (*g_atfork_prepare[ATFORK_MAX])();
extern void (*g_atfork_parent[ATFORK_MAX])();
extern void (*g_atfork_child[ATFORK_MAX])();
extern size_t g_atfork_prepare_nb;
extern size_t g_atfork_parent_nb;
extern size_t g_atfork_child_nb;

#endif
