#include <stdlib.h>
#include <stdio.h>

uintptr_t __stack_chk_guard;

void __stack_chk_fail(void)
{
	fprintf(stderr, "***stack smash detected***\n");
	abort();
}

void __chk_fail(void)
{
	fprintf(stderr, "***buffer overflow detected***\n");
	abort();
}
