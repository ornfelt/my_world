#ifndef TESTS_H
#define TESTS_H

#include <stddef.h>
#include <stdint.h>

#define ASSERT_STR_CMP(a, b, op) \
do \
{ \
	const char *sa = a; \
	const char *sb = b; \
	if (strcmp(sa, sb) op 0) \
	{ \
		__atomic_add_fetch(&g_passed, 1, __ATOMIC_SEQ_CST); \
		break; \
	} \
	__atomic_add_fetch(&g_failed, 1, __ATOMIC_SEQ_CST); \
	printf("[%s:%d] \"%s\" " #op " \"%s\"\n", __func__, __LINE__, sa, sb); \
} while (0)

#define ASSERT_STR_LT(a, b) ASSERT_STR_CMP(a, b, <)
#define ASSERT_STR_LE(a, b) ASSERT_STR_CMP(a, b, <=)
#define ASSERT_STR_EQ(a, b) ASSERT_STR_CMP(a, b, ==)
#define ASSERT_STR_NE(a, b) ASSERT_STR_CMP(a, b, !=)
#define ASSERT_STR_GE(a, b) ASSERT_STR_CMP(a, b, >=)
#define ASSERT_STR_GT(a, b) ASSERT_STR_CMP(a, b, >)

#define ASSERT_WCS_CMP(a, b, op) \
do \
{ \
	const wchar_t *sa = a; \
	const wchar_t *sb = b; \
	if (wcscmp(sa, sb) op 0) \
	{ \
		__atomic_add_fetch(&g_passed, 1, __ATOMIC_SEQ_CST); \
		break; \
	} \
	__atomic_add_fetch(&g_failed, 1, __ATOMIC_SEQ_CST); \
	printf("[%s:%d] \"%ls\" " #op " \"%ls\"\n", __func__, __LINE__, sa, sb); \
} while (0)

#define ASSERT_WCS_LT(a, b) ASSERT_WCS_CMP(a, b, <)
#define ASSERT_WCS_LE(a, b) ASSERT_WCS_CMP(a, b, <=)
#define ASSERT_WCS_EQ(a, b) ASSERT_WCS_CMP(a, b, ==)
#define ASSERT_WCS_NE(a, b) ASSERT_WCS_CMP(a, b, !=)
#define ASSERT_WCS_GE(a, b) ASSERT_WCS_CMP(a, b, >=)
#define ASSERT_WCS_GT(a, b) ASSERT_WCS_CMP(a, b, >)

#define ASSERT_CMP(a, b, op) \
do \
{ \
	if ((a) op (b)) \
	{ \
		__atomic_add_fetch(&g_passed, 1, __ATOMIC_SEQ_CST); \
		break; \
	} \
	__atomic_add_fetch(&g_failed, 1, __ATOMIC_SEQ_CST); \
	printf("[%s:%d] %s " #op " %s (%llu " #op " %llu)\n", __func__, __LINE__, #a, #b, (unsigned long long)(uintptr_t)a, (unsigned long long)(uintptr_t)b); \
} while (0)

#define ASSERT_LT(a, b) ASSERT_CMP(a, b, <)
#define ASSERT_LE(a, b) ASSERT_CMP(a, b, <=)
#define ASSERT_EQ(a, b) ASSERT_CMP(a, b, ==)
#define ASSERT_NE(a, b) ASSERT_CMP(a, b, !=)
#define ASSERT_GE(a, b) ASSERT_CMP(a, b, >=)
#define ASSERT_GT(a, b) ASSERT_CMP(a, b, >)

extern size_t g_passed;
extern size_t g_failed;

uint64_t nanotime(void);

/* string.c */
void test_strlen(void);
void test_memchr(void);
void test_memrchr(void);
void test_memmem(void);
void test_strnlen(void);
void test_strcpy(void);
void test_strncpy(void);
void test_strlcpy(void);
void test_strcat(void);
void test_strncat(void);
void test_strlcat(void);
void test_strchr(void);
void test_strchrnul(void);
void test_strrchr(void);
void test_memcmp(void);
void test_memcpy(void);
void test_memmove(void);
void test_memset(void);
void test_stpcpy(void);
void test_stpncpy(void);
void test_memccpy(void);
void test_strstr(void);
void test_strnstr(void);
void test_strcmp(void);
void test_strncmp(void);
void test_strpbrk(void);
void test_strspn(void);
void test_strcspn(void);

/* ctype.c */
void test_isalnum(void);
void test_isalpha(void);
void test_isascii(void);
void test_isblank(void);
void test_iscntrl(void);
void test_isdigit(void);
void test_isgraph(void);
void test_islower(void);
void test_isprint(void);
void test_ispunct(void);
void test_isspace(void);
void test_isupper(void);
void test_isxdigit(void);
void test_tolower(void);
void test_toupper(void);

/* pthread.c */
void test_pthread(void);

/* strto.c */
void test_strtol(void);
void test_strtoul(void);
void test_strtoll(void);
void test_strtoull(void);

/* glob.c */
void test_wordexp(void);
void test_fnmatch(void);
void test_glob(void);

/* wcstring.c */
void test_wcslen(void);
void test_wmemchr(void);
void test_wmemrchr(void);
void test_wmemmem(void);
void test_wcsnlen(void);
void test_wcscpy(void);
void test_wcsncpy(void);
void test_wcslcpy(void);
void test_wcscat(void);
void test_wcsncat(void);
void test_wcslcat(void);
void test_wcschr(void);
void test_wcschrnul(void);
void test_wcsrchr(void);
void test_wmemcmp(void);
void test_wmemcpy(void);
void test_wmemmove(void);
void test_wmemset(void);
void test_wcpcpy(void);
void test_wcpncpy(void);
void test_wmemccpy(void);
void test_wcsstr(void);
void test_wcsnstr(void);
void test_wcscmp(void);
void test_wcsncmp(void);
void test_wcspbrk(void);
void test_wcsspn(void);
void test_wcscspn(void);

/* misc.c */
void test_pipe(void);
void test_env(void);
void test_time(void);
void test_strftime(void);
void test_printf(void);
void test_fifo(void);
void test_qsort(void);
void test_dl(void);
void test_pf_local(void);
void test_inet(void);
void test_libz(void);
void test_servent(void);
void test_hostent(void);
void test_protoent(void);
void test_cloexec(void);
void test_fetch(void);
void test_signal(void);
void test_getline(void);
void test_fgets(void);
void test_socketpair(void);
void test_basename(void);
void test_dirname(void);
void test_popen(void);
void test_fmemopen(void);
void test_bsearch(void);
void test_scanf(void);

#endif
