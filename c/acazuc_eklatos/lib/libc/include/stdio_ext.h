#ifndef STDIO_EXT_H
#define STDIO_EXT_H

#include <sys/types.h>

#define FSETLOCKING_INTERNAL 0
#define FSETLOCKING_BYCALLER 1
#define FSETLOCKING_QUERY    2

#ifdef __cplusplus
extern "C" {
#endif

typedef struct FILE FILE;

size_t __fbufsize(FILE *fp);
size_t __fpending(FILE *fp);
size_t __freadahead(FILE *fp);
int __flfb(FILE *fp);
int __freadable(FILE *fp);
int __fwritable(FILE *fp);
int __freading(FILE *fp);
int __fwriting(FILE *fp);
int __fsetlocking(FILE *fp, int type);
void _flushlfb(void);
void __fpurge(FILE *fp);
void __fseterr(FILE *fp);

#ifdef __cplusplus
}
#endif

#endif
