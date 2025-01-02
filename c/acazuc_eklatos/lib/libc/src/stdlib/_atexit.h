#ifndef _ATEXIT_H
#define _ATEXIT_H

#include <stdlib.h>

extern void (*g_atexit_fn[ATEXIT_MAX])();
extern size_t g_atexit_fn_nb;

#endif
