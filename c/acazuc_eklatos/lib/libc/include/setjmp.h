#ifndef SETJMP_H
#define SETJMP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	intptr_t data[5];
	int val;
} jmp_buf;

typedef struct
{
	jmp_buf buf; /* XXX */
} sigjmp_buf;

int setjmp(jmp_buf env);
void longjmp(jmp_buf env, int val) __attribute__((noreturn));
int sigsetjmp(sigjmp_buf env, int savesigs);
void siglongjmp(sigjmp_buf env, int val) __attribute__((noreturn));

#ifdef __cplusplus
}
#endif

#endif
