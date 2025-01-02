#ifndef STDLIB_H
#define STDLIB_H

#include <stdint.h>
#include <stddef.h>
#include <alloca.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

#define PATH_MAX   4096
#define ATEXIT_MAX 32
#define RAND_MAX   0x7FFFFFFF
#define MB_CUR_MAX 1

typedef struct
{
	int quot;
	int rem;
} div_t;

typedef struct
{
	long quot;
	long rem;
} ldiv_t;

typedef struct
{
	long long quot;
	long long rem;
} lldiv_t;

void *malloc(size_t size) __attribute__((malloc, alloc_size(1)));
void free(void *ptr);
void *realloc(void *ptr, size_t size) __attribute__((alloc_size(2)));
void *calloc(size_t nmemb, size_t size) __attribute__((malloc, alloc_size(1, 2)));

int atoi(const char *s);
long atol(const char *s);
long long atoll(const char *s);
double atof(const char *nptr);

unsigned long strtoul(const char *nptr, char **endptr, int base);
unsigned long long strtoull(const char *nptr, char **endptr, int base);
long strtol(const char *nptr, char **endptr, int base);
long long strtoll(const char *nptr, char **endptr, int base);

double strtod(const char *nptr, char **endptr);
float strtof(const char *nptr, char **endptr);
long double strtold(const char *nptr, char **endptr);

void exit(int status) __attribute__ ((noreturn));
void _Exit(int status) __attribute__ ((noreturn));
void abort(void) __attribute__ ((noreturn));

char *getenv(const char *name);
int setenv(const char *name, const char *value, int overwrite);
int unsetenv(const char *name);
int putenv(char *string);
int clearenv(void);

int getsubopt(char **optionp, char * const *tokens, char **valuep);

void qsort(void *base, size_t nmemb, size_t size,
           int (*cmp)(const void *, const void *));

char *realpath(const char *path, char *resolved_path);

int rand(void);
void srand(unsigned int seed);
double drand48(void);
double erand48(uint16_t x[3]);
long lrand48(void);
long nrand48(uint16_t x[3]);
long mrand48(void);
long jrand48(uint16_t x[3]);
void srand48(long seed);
uint16_t *seed48(uint16_t v[3]);
void lcong48(uint16_t param[7]);
long random(void);
void srandom(unsigned seed);
char *initstate(unsigned seed, char *state, size_t n);
char *setstate(char *state);

int system(const char *command);

int abs(int j);
long labs(long j);
long long llabs(long long j);

void *bsearch(const void *key, const void *base, size_t nmemb, size_t size,
              int (*compar)(const void *, const void *));

size_t mbstowcs(wchar_t *d, const char *s, size_t n);
int wctomb(char *s, wchar_t wc);
int mbtowc(wchar_t *wc, const char *s, size_t n);
int mblen(const char *s, size_t n);

char *mktemp(char *tmplt);
char *mkdtemp(char *tmplt);
int mkstemp(char *tmplt);

int atexit(void (*function)(void));

int getloadavg(double loadavg[], int nelem);

int posix_openpt(int flags);
int grantpt(int fd);
int unlockpt(int fd);
char *ptsname(int fd);

div_t div(int num, int dem);
ldiv_t ldiv(long num, long dem);
lldiv_t lldiv(long long num, long long dem);

const char *getprogname(void);

#ifdef __cplusplus
}
#endif

#endif
