#ifndef GLOB_H
#define GLOB_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GLOB_ERR         (1 << 0)
#define GLOB_MARK        (1 << 1)
#define GLOB_NOSORT      (1 << 2)
#define GLOB_DOOFFS      (1 << 3)
#define GLOB_NOCHECK     (1 << 4)
#define GLOB_APPEND      (1 << 5)
#define GLOB_NOESCAPE    (1 << 6)

#define GLOB_NOSPACE 1
#define GLOB_ABORTED 2
#define GLOB_NOMATCH 3

typedef struct
{
	size_t gl_pathc;
	char **gl_pathv;
	size_t gl_offs;
} glob_t;

int glob(const char *pattern, int flags, int (*errfn)(const char *path,
                                                      int err),
         glob_t *globp);
void globfree(glob_t *globp);

#ifdef __cplusplus
}
#endif

#endif
