#include "_atexit.h"

void (*g_atexit_fn[ATEXIT_MAX])();
size_t g_atexit_fn_nb;
