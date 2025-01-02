#ifndef REGEX_H
#define REGEX_H

#include <sys/types.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define REG_EXTEDED (1 << 0)
#define REG_ICASE   (1 << 1)
#define REG_NOSUB   (1 << 2)
#define REG_NEWLINE (1 << 3)
#define REG_NOTBOL  (1 << 4)
#define REG_NOTEOL  (1 << 5)

#define REG_NOMATCH -1

#define REG_BADBR    1
#define REG_BADPAT   2
#define REG_BADRPT   3
#define REG_EBRACE   4
#define REG_EBRACK   6
#define REG_ECOLLATE 7
#define REG_ECTYPE   8
#define REG_EESCAPE  9
#define REG_EPAREN   10
#define REG_ERANGE   11
#define REG_ESPACE   12
#define REG_ESUBREG  13

typedef ssize_t regoff_t;

typedef struct
{
	size_t re_nsub;
	char *regex;
	size_t regex_len;
	int flags;
} regex_t;

typedef struct
{
	regoff_t rm_so;
	regoff_t rm_eo;
} regmatch_t;

int regcomp(regex_t *preg, const char *regex, int flags);
int regexec(const regex_t *regex, const char *str, size_t nmatch,
            regmatch_t *matches, int flags);
size_t regerror(int err, const regex_t *regex, char *buf, size_t size);
void regfree(regex_t *regex);

#ifdef __cplusplus
}
#endif

#endif
