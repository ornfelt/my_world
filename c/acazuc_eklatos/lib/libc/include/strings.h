#ifndef STRINGS_H
#define STRINGS_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int strcasecmp(const char *s1, const char *s2);
int strncasecmp(const char *s1, const char *s2, size_t n);
char *strcasestr(const char *s1, const char *s2);

#ifdef __cplusplus
}
#endif

#endif
