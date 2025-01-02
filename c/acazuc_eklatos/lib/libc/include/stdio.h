#ifndef STDIO_H
#define STDIO_H

#include <sys/types.h>
#include <sys/queue.h>

#include <stdio_ext.h>
#include <stddef.h>
#include <stdarg.h>
#include <fcntl.h>

#ifdef __cplusplus
extern "C" {
#endif

#define _POSIX_VERSION 200809 /* XXX move somewhere else */

#ifndef SEEK_SET
#define SEEK_SET 0
#endif
#ifndef SEEK_CUR
#define SEEK_CUR 1
#endif
#ifndef SEEK_END
#define SEEK_END 2
#endif

#define EOF -1

#define _IONBF 0
#define _IOLBF 1
#define _IOFBF 2

#define BUFSIZ 4096

#define P_tmpdir "/tmp"
#define L_tmpnam 20

typedef struct FILE FILE;
typedef off_t fpos_t;

typedef ssize_t (cookie_read_function_t)(void *cookie, char *buf, size_t size);
typedef ssize_t (cookie_write_function_t)(void *cookie, const char *buf,
                                          size_t size);
typedef int (cookie_seek_function_t)(void *cookie, off_t off, int whence);
typedef int (cookie_close_function_t)(void *cookie);

typedef struct
{
	cookie_read_function_t *read;
	cookie_write_function_t *write;
	cookie_seek_function_t *seek;
	cookie_close_function_t *close;
} cookie_io_functions_t;

int printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
int vprintf(const char *fmt, va_list va_arg);
int snprintf(char *d, size_t n, const char *fmt, ...) __attribute__((format(printf, 3, 4)));
int vsnprintf(char *d, size_t n, const char *fmt, va_list va_arg);
int fprintf(FILE *fp, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
int vfprintf(FILE *fp, const char *fmt, va_list va_arg);
int sprintf(char *d, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
int vsprintf(char *d, const char *fmt, va_list va_arg);

#if defined(_FORTIFY_SOURCE) && _FORTIFY_SOURCE > 0
#define printf(...) __builtin___printf_chk(_FORTIFY_SOURCE - 1, ##__VA_ARGS__)
#define vprintf(...) __builtin___vprintf_chk(_FORTIFY_SOURCE - 1, ##__VA_ARGS__)
#define fprintf(fp, ...) __builtin___fprintf_chk(fp, _FORTIFY_SOURCE - 1, ##__VA_ARGS__)
#define vfprintf(fp, ...) __builtin___vfprintf_chk(fp, _FORTIFY_SOURCE - 1, ##__VA_ARGS__)
#define snprintf(d, n, ...) __builtin___snprintf_chk(d, n, _FORTIFY_SOURCE - 1, __builtin_object_size(d, _FORTIFY_SOURCE > 1), ##__VA_ARGS__)
#define vsnprintf(d, n, fmt, va_arg) __builtin___vsnprintf_chk(d, n, _FORTIFY_SOURCE - 1, __builtin_object_size(d, _FORTIFY_SOURCE > 1), fmt, va_arg)
#define sprintf(d, ...) __builtin___sprintf_chk(d, _FORTIFY_SOURCE - 1, __builtin_object_size(d, _FORTIFY_SOURCE > 1), ##__VA_ARGS__)
#define vsprintf(d, fmt, va_arg) __builtin___vsprintf_chk(d, _FORTIFY_SOURCE - 1, __builtin_object_size(d, _FORTIFY_SOURCE > 1), fmt, va_arg)
#endif

int dprintf(int fd, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
int vdprintf(int fd, const char *fmt, va_list va_arg);

int scanf(const char *fmt, ...) __attribute__((format(scanf, 1, 2)));
int vscanf(const char *fmt, va_list ap);
int sscanf(const char *str, const char *fmt, ...) __attribute__((format(scanf, 2, 3)));
int vsscanf(const char *str, const char *fmt, va_list ap);
int fscanf(FILE *fp, const char *fmt, ...) __attribute__((format(scanf, 2, 3)));
int vfscanf(FILE *fp, const char *fmt, va_list ap);

int fputc(int c, FILE *fp);
int fputc_unlocked(int c, FILE *fp);
int fputs(const char *s, FILE *fp);
int fputs_unlocked(const char *s, FILE *fp);
int putc(int c, FILE *fp);
int putc_unlocked(int c, FILE *fp);
int putchar(int c);
int putchar_unlocked(int c);
int puts(const char *s);
int puts_unlock(const char *s);
int putw(int w, FILE *fp);
int putw_unlocked(int w, FILE *fp);

int fgetc(FILE *fp);
int fgetc_unlocked(FILE *fp);
char *fgets(char *s, int size, FILE *fp);
char *fgets_unlocked(char *s, int size, FILE *fp);
int getc(FILE *fp);
int getc_unlocked(FILE *fp);
int getchar(void);
int getchar_unlocked(void);
int ungetc(int c, FILE *fp);
int ungetc_unlocked(int c, FILE *fp);

void perror(const char *s);

FILE *fopen(const char *pathname, const char *mode);
FILE *fdopen(int fd, const char *mode);
FILE *freopen(const char *pathname, const char *mode, FILE *fp);
FILE *fopencookie(void *cookie, const char *mode, cookie_io_functions_t *funcs);
FILE *funopen(const void *cookie, int (*readfn)(void *, char *, int),
              int (*writefn)(void *, const char *, int),
              off_t (*seekfn)(void *, off_t, int), int (*closefn)(void *));
FILE *fropen(void *cookie, int (*readfn)(void *, char *, int));
FILE *fwopen(void *cookie, int (*writefn)(void *, const char *, int));
FILE *fmemopen(void *buf, size_t size, const char *mode);
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *fp);
size_t fread_unlocked(void *ptr, size_t size, size_t nmemb, FILE *fp);
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *fp);
size_t fwrite_unlocked(const void *ptr, size_t size, size_t nmemb, FILE *fp);
int fseek(FILE *fp, long offset, int whence);
int fseeko(FILE *fp, off_t offset, int whence);
long ftell(FILE *fp);
off_t ftello(FILE *fp);
void rewind(FILE *fp);
int fgetpos(FILE *fp, fpos_t *pos);
int fsetpos(FILE *fp, const fpos_t *pos);
int fflush(FILE *fp);
int fflush_unlocked(FILE *fp);
int fpurge(FILE *fp);
void clearerr(FILE *fp);
void clearerr_unlocked(FILE *fp);
int feof(FILE *fp);
int feof_unlocked(FILE *fp);
int ferror(FILE *fp);
int ferror_unlocked(FILE *fp);
int fileno(FILE *fp);
int fileno_unlocked(FILE *fp);
int fclose(FILE *fp);
int fcloseall(void);
void flockfile(FILE *fp);
int ftrylockfile(FILE *fp);
void funlockfile(FILE *fp);

int remove(const char *pathname);
int rename(const char *oldpath, const char *newpath);
int renameat(int olddirfd, const char *oldpath, int newdirfd,
             const char *newpath);

void setbuf(FILE *fp, char *buf);
int setvbuf(FILE *fp, char *buf, int mode, size_t size);

ssize_t getline(char **lineptr, size_t *n, FILE *fp);
ssize_t getdelim(char **lineptr, size_t *n, int delim, FILE *fp);

FILE *tmpfile(void);

char *tmpnam(char *s);

FILE *popen(const char *cmd, const char *type);
int pclose(FILE *fp);

extern FILE *stdout;
extern FILE *stdin;
extern FILE *stderr;

#ifdef __cplusplus
}
#endif

#endif
