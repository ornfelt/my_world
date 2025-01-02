#ifndef STRING_H
#define STRING_H

#include <strings.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_FORTIFY_SOURCE) && _FORTIFY_SOURCE > 0

static inline void *memset(void *d, int c, size_t n)
{
	return __builtin___memset_chk(d, c, n, __builtin_object_size(d, 0));
}

static inline void *memcpy(void *d, const void *s, size_t n)
{
	return __builtin___memcpy_chk(d, s, n, __builtin_object_size(d, 0));
}

static inline void *memmove(void *d, const void *s, size_t n)
{
	return __builtin___memmove_chk(d, s, n, __builtin_object_size(d, 0));
}

static inline char *strcpy(char *d, const char *s)
{
	return __builtin___strcpy_chk(d, s, __builtin_object_size(d, _FORTIFY_SOURCE > 1));
}

static inline char *stpcpy(char *d, const char *s)
{
	return __builtin___stpcpy_chk(d, s, __builtin_object_size(d, _FORTIFY_SOURCE > 1));
}

static inline char *strncpy(char *d, const char *s, size_t n)
{
	return __builtin___strncpy_chk(d, s, n, __builtin_object_size(d, _FORTIFY_SOURCE > 1));
}

static inline char *strcat(char *d, const char *s)
{
	return __builtin___strcat_chk(d, s, __builtin_object_size(d, _FORTIFY_SOURCE > 1));
}

static inline char *strncat(char *d, const char *s, size_t n)
{
	return __builtin___strncat_chk(d, s, n, __builtin_object_size(d, _FORTIFY_SOURCE > 1));
}

#else

void *memset(void *d, int c, size_t n);
void *memcpy(void *d, const void *s, size_t n);
void *memmove(void *d, const void *s, size_t n);
char *strcpy(char *d, const char *s);
char *strncpy(char *d, const char *s, size_t n);
char *stpcpy(char *d, const char *s);
char *strcat(char *d, const char *s);
char *strncat(char *d, const char *s, size_t n);

#endif

void *memccpy(void *d, const void *s, int c, size_t n);
void *memchr(const void *s, int c, size_t n);
void *memrchr(const void *s, int c, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
void *memmem(const void *haystack, size_t haystacklen,
             const void *needle, size_t needlelen);

size_t strlen(const char *s);
size_t strnlen(const char *s, size_t maxlen);
size_t strlcpy(char *d, const char *s, size_t n);
size_t strlcat(char *d, const char *s, size_t n);
char *strchr(const char *s, int c);
char *strchrnul(const char *s, int c);
char *strrchr(const char *s, int c);
char *strstr(const char *s1, const char *s2);
char *strnstr(const char *s1, const char *s2, size_t n);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strdup(const char *s);
char *strndup(const char *s, size_t n);
char *strpbrk(const char *s, const char *accept);
char *strtok(char *str, const char *delim);
char *strtok_r(char *str, const char *delim, char **saveptr);
size_t strspn(const char *s, const char *accept);
size_t strcspn(const char *s, const char *reject);
int strcoll(const char *s1, const char *s2);
char *stpncpy(char *d, const char *s, size_t n);

char *strerror(int errnum);
char *strsignal(int signum);

size_t strxfrm(char *dst, const char *src, size_t n);

#ifdef __cplusplus
}
#endif

#endif
