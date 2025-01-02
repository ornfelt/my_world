#include "_atfork.h"

void (*g_atfork_prepare[ATFORK_MAX])();
void (*g_atfork_parent[ATFORK_MAX])();
void (*g_atfork_child[ATFORK_MAX])();
size_t g_atfork_prepare_nb;
size_t g_atfork_parent_nb;
size_t g_atfork_child_nb;
