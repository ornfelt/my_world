#ifndef STD_H
#define STD_H

#include <types.h>

struct user_regs;

#ifdef ENABLE_TRACE
# define TRACE(fmt, ...) printf("[%s:%d] " fmt "\n", __func__, __LINE__, ##__VA_ARGS__)
#else
# define TRACE(fmt, ...) do { } while (0)
#endif

#define QUOTEME_(x) #x
#define QUOTEME(x) QUOTEME_(x)

#define offsetof(T, m) ((size_t)(&((T*)0)->m))

#define NULL ((void*)0)

typedef __builtin_va_list va_list;

#define va_start(ap, lastarg) __builtin_va_start(ap, lastarg)
#define va_end(ap) __builtin_va_end(ap)
#define va_arg(ap, T) __builtin_va_arg(ap, T)
#define va_copy(dst, src) __builtin_va_copy(dst, src)

#define INT8_MIN -128
#define INT8_MAX 127
#define UINT8_MAX 255

#define INT16_MIN -32768
#define INT16_MAX 32767
#define UINT16_MAX 65535

#define INT32_MIN -2147483648
#define INT32_MAX 2147483647
#define UINT32_MAX 4294967295

#define INT64_MIN ((-9223372036854775807LL) - 1LL)
#define INT64_MAX 9223372036854775807LL
#define UINT64_MAX 18446744073709551615ULL

#define MAXPATHLEN 4096

#define __predict_true(v)  __builtin_expect(!!(v), 1)
#define __predict_false(v) __builtin_expect(!!(v), 0)

#if __SIZE_WIDTH__ == 32

#define CHAR_MIN INT8_MIN
#define CHAR_MAX INT8_MAX
#define UCHAR_MAX UINT8_MAX

#define SHRT_MIN INT16_MIN
#define SHRT_MAX INT16_MAX
#define USHRT_MAX UINT16_MAX

#define INT_MIN INT32_MIN
#define INT_MAX INT32_MAX
#define UINT_MAX UINT32_MAX

#define LONG_MIN INT32_MIN
#define LONG_MAX INT32_MAX
#define ULONG_MAX UINT32_MAX

#define LLONG_MIN INT64_MIN
#define LLONG_MAX INT64_MAX
#define ULLONG_MAX UINT64_MAX

#define SIZE_MAX UINT32_MAX

#define PRIi8 "i"
#define PRId8 "d"
#define PRIu8 "u"
#define PRIo8 "o"
#define PRIx8 "x"
#define PRIX8 "X"

#define PRIi16 "i"
#define PRId16 "d"
#define PRIu16 "u"
#define PRIo16 "o"
#define PRIx16 "x"
#define PRIX16 "X"

#define PRIi32 "li"
#define PRId32 "ld"
#define PRIu32 "lu"
#define PRIo32 "lo"
#define PRIx32 "lx"
#define PRIX32 "lX"

#define PRIi64 "lli"
#define PRId64 "lld"
#define PRIu64 "llu"
#define PRIo64 "llo"
#define PRIx64 "llx"
#define PRIX64 "llX"

#elif __SIZE_WIDTH__ == 64

#define CHAR_MIN INT8_MIN
#define CHAR_MAX INT8_MAX
#define UCHAR_MAX UINT8_MAX

#define SHRT_MIN INT16_MIN
#define SHRT_MAX INT16_MAX
#define USHRT_MAX UINT16_MAX

#define INT_MIN INT32_MIN
#define INT_MAX INT32_MAX
#define UINT_MAX UINT32_MAX

#define LONG_MIN INT64_MIN
#define LONG_MAX INT64_MAX
#define ULONG_MAX UINT64_MAX

#define LLONG_MIN INT64_MIN
#define LLONG_MAX INT64_MAX
#define ULLONG_MAX UINT64_MAX

#define SIZE_MAX UINT64_MAX

#define PRIi8 "i"
#define PRId8 "d"
#define PRIu8 "u"
#define PRIo8 "o"
#define PRIx8 "x"
#define PRIX8 "X"

#define PRIi16 "i"
#define PRId16 "d"
#define PRIu16 "u"
#define PRIo16 "o"
#define PRIx16 "x"
#define PRIX16 "X"

#define PRIi32 "i"
#define PRId32 "d"
#define PRIu32 "u"
#define PRIo32 "o"
#define PRIx32 "x"
#define PRIX32 "X"

#define PRIi64 "li"
#define PRId64 "ld"
#define PRIu64 "lu"
#define PRIo64 "lo"
#define PRIx64 "lx"
#define PRIX64 "lX"

#else
# error "unknown arch"
#endif

struct thread;
struct tty;
struct uio;

void _panic(const char *file, size_t line, const char *fn, const char *fmt,
           ...)  __attribute__((format(printf, 4, 5), noreturn));
#define panic(...) _panic(__FILE__, __LINE__, __func__, __VA_ARGS__)

void arch_disable_interrupts(void);
void arch_enable_interrupts(void);
void arch_wait_for_interrupt(void);
void arch_print_regs(const struct user_regs *regs);
int arch_waitq_sleep(void);
void arch_waitq_leave(void);
int arch_shutdown(void);
int arch_reboot(void);
int arch_suspend(void);
int arch_hibernate(void);

ssize_t vprintf(const char *fmt, va_list va_arg);
ssize_t printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
ssize_t vsnprintf(char *d, size_t n, const char *fmt, va_list va_arg);
ssize_t snprintf(char *d, size_t n, const char *fmt, ...) __attribute__((format(printf, 3, 4)));
ssize_t vuprintf(struct uio *uio, const char *fmt, va_list va_arg);
ssize_t uprintf(struct uio *uio, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
int printf_addtty(struct tty *tty);

typedef void (*early_printf_t)(const char *s, size_t n);
extern early_printf_t g_early_printf;

void print_stack_trace(void);

#define M_ZERO (1 << 0)

void *malloc(size_t size, uint32_t flags);
void free(void *ptr);
void *realloc(void *ptr, size_t size, uint32_t flags);
int alloc_print(struct uio *uio);
void alloc_init(void);

int atoi(const char *s);
int atoin(const char *s, size_t n);

void *memset(void *d, int c, size_t n);
void *memcpy(void *d, const void *s, size_t n);
void *memccpy(void *d, const void *s, int c, size_t n);
void *memmove(void *d, const void *s, size_t n);
void *memchr(const void *s, int c, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);

size_t strlen(const char *s);
size_t strnlen(const char *s, size_t maxlen);
char *strcpy(char *d, const char *s);
char *strncpy(char *d, const char *s, size_t n);
size_t strlcpy(char *d, const char *s, size_t n);
char *strcat(char *d, const char *s);
char *strncat(char *d, const char *s, size_t n);
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

char *strerror(int errnum);

int isalnum(int c);
int isalpha(int c);
int iscntrl(int c);
int isdigit(int c);
int isgraph(int c);
int islower(int c);
int isprint(int c);
int ispunct(int c);
int isspace(int c);
int isupper(int c);
int isxdigit(int c);

int isascii(int c);
int isblank(int c);

int toupper(int c);
int tolower(int c);

#define memset __builtin_memset
#define memcpy __builtin_memcpy
#define memmove __builtin_memmove
#define memchr __builtin_memchr
#define memcmp __builtin_memcmp

#define strlen __builtin_strlen
#define strnlen __builtin_strnlen
#define strcpy __builtin_strcpy
#define strncpy __builtin_strncpy
#define strcat __builtin_strcat
#define strncat __builtin_strncat
#define strchr __builtin_strchr
#define strrchr __builtin_strrchr
#define strstr __builtin_strstr
#define strcmp __builtin_strcmp
#define strncmp __builtin_strncmp
#define strdup __builtin_strdup
#define strndup __builtin_strndup

#define isalnum __builtin_isalnum
#define isalpha __builtin_isalpha
#define iscntrl __builtin_iscntrl
#define isdigit __builtin_isdigit
#define isgraph __builtin_isgraph
#define islower __builtin_islower
#define isprint __builtin_isprint
#define ispunct __builtin_ispunct
#define isspace __builtin_isspace
#define isupper __builtin_isupper
#define isxdigit __builtin_isxdigit

#define isascii __builtin_isascii
#define isblank __builtin_isblank

#define tolower __builtin_tolower
#define toupper __builtin_toupper

#define assert(expression, ...) \
do \
{ \
	if (!__predict_true(expression)) \
		panic(__VA_ARGS__); \
} while (0)

#endif
