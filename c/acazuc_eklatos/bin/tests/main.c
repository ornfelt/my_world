#include "tests.h"

#include <inttypes.h>
#include <libelf.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <link.h>

enum test_type
{
	TEST_SIGINT      = (1 << 0),
	TEST_SIGSEGV     = (1 << 1),
	TEST_SIGBUS      = (1 << 2),
	TEST_SIGTRAP     = (1 << 3),
	TEST_CANARY      = (1 << 4),
	TEST_SYSCALL     = (1 << 5),
	TEST_ELF_PHDR    = (1 << 6),
	TEST_MEMSET_RATE = (1 << 7),
	TEST_MALLOC      = (1 << 8),
	TEST_STRING      = (1 << 9),
	TEST_CTYPE       = (1 << 10),
	TEST_PTHREAD     = (1 << 11),
	TEST_STRTO       = (1 << 12),
	TEST_GLOB        = (1 << 13),
	TEST_WCSTRING    = (1 << 14),
	TEST_MISC        = (1 << 15),
};

static const struct
{
	const char *name;
	enum test_type type;
} test_names[] =
{
	{"sigint",      TEST_SIGINT},
	{"sigsegv",     TEST_SIGSEGV},
	{"sigbus",      TEST_SIGBUS},
	{"sigtrap",     TEST_SIGTRAP},
	{"canary",      TEST_CANARY},
	{"syscall",     TEST_SYSCALL},
	{"elf_phdr",    TEST_ELF_PHDR},
	{"memset_rate", TEST_MEMSET_RATE},
	{"malloc",      TEST_MALLOC},
	{"string",      TEST_STRING},
	{"ctype",       TEST_CTYPE},
	{"pthread",     TEST_PTHREAD},
	{"strto",       TEST_STRTO},
	{"glob",        TEST_GLOB},
	{"wcstring",    TEST_WCSTRING},
	{"misc",        TEST_MISC},
};

extern char **environ;

size_t g_passed;
size_t g_failed;

uint64_t nanotime(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000000000 + ts.tv_nsec;
}

static void __attribute__ ((noinline, constructor)) ctr(void)
{
	printf("init\n");
}

static void __attribute__ ((noinline, destructor)) dtr(void)
{
	printf("fini\n");
}

static void __attribute__ ((noinline)) test_syscall(void)
{
	ASSERT_EQ(unlink((char*)1), -1);
	ASSERT_EQ(errno, EFAULT);
	struct timespec ts[20];
	for (size_t i = 0; i < sizeof(ts) / sizeof(*ts); ++i)
		clock_gettime(CLOCK_MONOTONIC, &ts[i]);
	uint64_t sum = 0;
	for (size_t i = 1; i < sizeof(ts) / sizeof(*ts); ++i)
	{
		struct timespec diff;
		struct timespec *a = &ts[i];
		struct timespec *b = &ts[i - 1];
		diff.tv_sec = a->tv_sec - b->tv_sec;
		if (a->tv_nsec >= b->tv_nsec)
		{
			diff.tv_nsec = a->tv_nsec - b->tv_nsec;
		}
		else
		{
			diff.tv_nsec = 1000000000 - (b->tv_nsec - a->tv_nsec);
			diff.tv_sec--;
		}
		sum += diff.tv_sec * 1000000000 + diff.tv_nsec;
	}
	printf("clock_gettime duration: %" PRIu64 "ns\n",
	       sum / (sizeof(ts) / sizeof(*ts) - 1));
	uint64_t begin;
	uint64_t end;
	uint64_t diff;
	size_t count = 1000000;
#define TEST_SYSCALL(syscall, ...) \
	begin = nanotime(); \
	for (size_t i = 0; i < count; ++i) \
		syscall(__VA_ARGS__); \
	end = nanotime(); \
	diff = end - begin; \
	printf(#syscall " duration: %" PRIu64  "ns\n", diff / count);
	TEST_SYSCALL(getpid);
	TEST_SYSCALL(write, 234234, NULL, 0);
	struct timespec tmp_ts;
	TEST_SYSCALL(clock_gettime, CLOCK_MONOTONIC, &tmp_ts);
}

static void __attribute__ ((noinline)) test_canary(void)
{
	uint8_t test[64];
	test[74] = rand();
	test[64] = '\0';
	test[63] = rand();
	printf("%s\n", test);
}

static int elf_phdr_cb(struct dl_phdr_info *info, size_t size, void *data)
{
	(void)size;
	(void)data;
	printf("elf '%s'\n", info->dlpi_name);
	for (size_t i = 0; i < info->dlpi_phnum; ++i)
		printf("%p-%p: %s PT_%s\n",
		       (void*)(info->dlpi_phdr[i].p_vaddr + info->dlpi_addr),
		       (void*)(info->dlpi_phdr[i].p_vaddr + info->dlpi_addr + info->dlpi_phdr[i].p_memsz),
		       elf_phdr_flags_str(info->dlpi_phdr[i].p_flags),
		       elf_phdr_type_str(info->dlpi_phdr[i].p_type));
	return 0;
}

static void __attribute__ ((noinline)) test_elf_phdr(void)
{
	dl_iterate_phdr(elf_phdr_cb, NULL);
}

static void __attribute__ ((noinline)) test_sigint(void)
{
	raise(SIGINT);
}

static void __attribute__ ((noinline)) test_sigsegv(void)
{
	*(uint8_t*)NULL = 0;
}

static void __attribute__ ((noinline)) test_sigbus(void)
{
	((char*)"salut")[0] = 0;
}

static void __attribute__ ((noinline)) test_sigtrap(void)
{
#if defined(__i386__) || defined(__x86_64__)
	__asm__ volatile ("int $0x3");
#elif defined(__arm__) || defined(__aarch64__) || defined(__riscv) || defined(__mips__)
	fprintf(stderr, "unsupported SIGTRAP\n");
	abort();
#else
# error "unknown arch"
#endif
}

static void __attribute__ ((noinline)) test_memset_rate(void)
{
	static const size_t n = 1024 * 1024 * 64;
	uint64_t s = nanotime();
	void *ptr = malloc(n);
	uint64_t e = nanotime();
	ASSERT_NE(ptr, NULL);
	printf("malloc: %lu us\n", (unsigned long)(e - s) / 1000);
	s = nanotime();
	memset(ptr, 0, n);
	e = nanotime();
	write(2342, ptr, n);
	printf("memset: %lu us\n", (unsigned long)(e - s) / 1000);
}

static inline void timespec_diff(struct timespec *d, const struct timespec *a,
                                 const struct timespec *b)
{
	d->tv_sec = a->tv_sec - b->tv_sec;
	if (a->tv_nsec >= b->tv_nsec)
	{
		d->tv_nsec = a->tv_nsec - b->tv_nsec;
	}
	else
	{
		d->tv_nsec = 1000000000 - (b->tv_nsec - a->tv_nsec);
		d->tv_sec--;
	}
}

static void __attribute__ ((noinline)) test_malloc(void)
{
	#define N 10000000
	static void *v[N];
	struct timespec a, b, tsa, tsf;
	clock_gettime(CLOCK_MONOTONIC, &a);
	for (size_t i = 0; i < N; ++i)
		v[i] = malloc(16);
	clock_gettime(CLOCK_MONOTONIC, &b);
	timespec_diff(&tsa, &b, &a);
	clock_gettime(CLOCK_MONOTONIC, &a);
	for (size_t i = 0; i < N; ++i)
		free(v[i]);
	clock_gettime(CLOCK_MONOTONIC, &b);
	timespec_diff(&tsf, &b, &a);
	printf("alloc: %" PRId64 ".%09" PRId64 "\n", tsa.tv_sec, tsa.tv_nsec);
	printf("free : %" PRId64 ".%09" PRId64 "\n", tsf.tv_sec, tsf.tv_nsec);
}

void test_atexit(void)
{
	printf("atexit ok\n");
}

int main(int argc, char **argv)
{
	int tests;

	if (argc > 1)
	{
		tests = 0;
		for (int i = 1; i < argc; ++i)
		{
			int found = 0;
			for (size_t j = 0; j < sizeof(test_names) / sizeof(*test_names); ++j)
			{
				if (!strcmp(argv[i], test_names[j].name))
				{
					tests |= test_names[j].type;
					found = 1;
					break;
				}
			}
			if (!found)
			{
				fprintf(stderr, "invalid test: %s\n", argv[i]);
				return EXIT_FAILURE;
			}
		}
	}
	else
	{
		tests = TEST_STRING
		      | TEST_CTYPE
		      | TEST_PTHREAD
		      | TEST_STRTO
		      | TEST_GLOB
		      | TEST_WCSTRING
		      | TEST_MISC;
	}

	srand(time(NULL));

	if (tests & TEST_SIGINT)
		test_sigint();
	if (tests & TEST_SIGSEGV)
		test_sigsegv();
	if (tests & TEST_SIGBUS)
		test_sigbus();
	if (tests & TEST_SIGTRAP)
		test_sigtrap();
	if (tests & TEST_CANARY)
		test_canary();
	if (tests & TEST_SYSCALL)
		test_syscall();
	if (tests & TEST_ELF_PHDR)
		test_elf_phdr();
	if (tests & TEST_MEMSET_RATE)
		test_memset_rate();
	if (tests & TEST_MALLOC)
		test_malloc();
	if (tests & TEST_STRING)
	{
		test_strlen();
		test_memchr();
		test_memrchr();
		test_memmem();
		test_strnlen();
		test_strcpy();
		test_strncpy();
		test_strlcpy();
		test_strcat();
		test_strncat();
		test_strlcat();
		test_strchr();
		test_strchrnul();
		test_strrchr();
		test_memcmp();
		test_memcpy();
		test_memmove();
		test_memset();
		test_stpcpy();
		test_stpncpy();
		test_memccpy();
		test_strstr();
		test_strnstr();
		test_strcmp();
		test_strncmp();
		test_strpbrk();
		test_strspn();
		test_strcspn();
	}
	if (tests & TEST_CTYPE)
	{
		test_isalnum();
		test_isalpha();
		test_isascii();
		test_isblank();
		test_iscntrl();
		test_isdigit();
		test_isgraph();
		test_islower();
		test_isprint();
		test_ispunct();
		test_isspace();
		test_isupper();
		test_isxdigit();
		test_tolower();
		test_toupper();
	}
	if (tests & TEST_PTHREAD)
	{
		test_pthread();
	}
	if (tests & TEST_STRTO)
	{
		test_strtol();
		test_strtoul();
		test_strtoll();
		test_strtoull();
	}
	if (tests & TEST_GLOB)
	{
		test_wordexp();
		test_fnmatch();
		test_glob();
	}
	if (tests & TEST_WCSTRING)
	{
		test_wcslen();
		test_wmemchr();
		test_wmemrchr();
		test_wmemmem();
		test_wcsnlen();
		test_wcscpy();
		test_wcsncpy();
		test_wcslcpy();
		test_wcscat();
		test_wcsncat();
		test_wcslcat();
		test_wcschr();
		test_wcschrnul();
		test_wcsrchr();
		test_wmemcmp();
		test_wmemcpy();
		test_wmemmove();
		test_wmemset();
		test_wcpcpy();
		test_wcpncpy();
		test_wmemccpy();
		test_wcsstr();
		test_wcsnstr();
		test_wcscmp();
		test_wcsncmp();
		test_wcspbrk();
		test_wcsspn();
		test_wcscspn();
	}
	if (tests & TEST_MISC)
	{
		test_pipe();
		test_env();
		test_time();
		test_strftime();
		test_printf();
		test_fifo();
		test_qsort();
		test_dl();
		test_pf_local();
		test_inet();
		test_libz();
		test_servent();
		test_hostent();
		test_protoent();
		test_cloexec();
		test_fetch();
		test_signal();
		test_getline();
		test_fgets();
		test_socketpair();
		test_basename();
		test_dirname();
		test_popen();
		test_fmemopen();
		test_bsearch();
		test_scanf();
	}

	ASSERT_EQ(atexit(test_atexit), 0);
	printf("passed: %zu\n", g_passed);
	printf("failed: %zu\n", g_failed);
	return 0;
}
