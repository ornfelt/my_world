#include "tests.h"

#include <arpa/inet.h>

#include <netinet/in.h>

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/un.h>

#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <libgen.h>
#include <stdlib.h>
#include <stdio.h>
#include <fetch.h>
#include <netdb.h>
#include <errno.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <zlib.h>

void test_pipe(void)
{
	int fds[2];
	ASSERT_NE(pipe(fds), -1);
	ASSERT_EQ(write(fds[1], "bonjour", 7), 7);
	char buf[10];
	ASSERT_EQ(read(fds[0], buf, 4), 4);
	ASSERT_EQ(memcmp(buf, "bonj", 4), 0);
	ASSERT_EQ(close(fds[1]), 0);
	ASSERT_EQ(read(fds[0], buf, 4), 3);
	ASSERT_EQ(memcmp(buf, "our", 3), 0);
	ASSERT_EQ(close(fds[0]), 0);
}

void test_env(void)
{
	char *tmp = getenv("SHELL");
	ASSERT_NE(tmp, NULL);
	tmp = getenv("TEST");
	ASSERT_EQ(tmp, NULL);
	ASSERT_EQ(putenv("TEST=oui"), 0);
	tmp = getenv("TEST");
	ASSERT_NE(tmp, NULL);
	ASSERT_STR_EQ(tmp, "oui");
	ASSERT_EQ(unsetenv("TEST"), 0);
	tmp = getenv("TEST");
	ASSERT_EQ(tmp, NULL);
	setenv("TEST", "non", 0);
	tmp = getenv("TEST");
	ASSERT_NE(tmp, NULL);
	ASSERT_STR_EQ(tmp, "non");
	ASSERT_EQ(setenv("TEST", "oui", 0), 0);
	tmp = getenv("TEST");
	ASSERT_NE(tmp, NULL);
	ASSERT_STR_EQ(tmp, "non");
	ASSERT_EQ(setenv("TEST", "oui", 1), 0);
	tmp = getenv("TEST");
	ASSERT_NE(tmp, NULL);
	ASSERT_STR_EQ(tmp, "oui");
	ASSERT_EQ(clearenv(), 0);
	tmp = getenv("SHELL");
	ASSERT_EQ(tmp, NULL);
}

void test_time(void)
{
	time_t t = 1661928356;
	struct tm tm;
	char test[32];

	ASSERT_NE(gmtime_r(&t, &tm), NULL);
	ASSERT_EQ(tm.tm_sec, 56);
	ASSERT_EQ(tm.tm_min, 45);
	ASSERT_EQ(tm.tm_hour, 6);
	ASSERT_EQ(tm.tm_mday, 31);
	ASSERT_EQ(tm.tm_mon, 7);
	ASSERT_EQ(tm.tm_year, 122);
	ASSERT_EQ(tm.tm_wday, 3);
	ASSERT_EQ(tm.tm_yday, 242);
	ASSERT_NE(ctime_r(&t, test), NULL);
	ASSERT_STR_EQ(test, "Wed Aug 31 06:45:56 2022\n");

	t = 1685691278;
	ASSERT_NE(gmtime_r(&t, &tm), NULL);
	ASSERT_EQ(tm.tm_sec, 38);
	ASSERT_EQ(tm.tm_min, 34);
	ASSERT_EQ(tm.tm_hour, 7);
	ASSERT_EQ(tm.tm_mday, 2);
	ASSERT_EQ(tm.tm_mon, 5);
	ASSERT_EQ(tm.tm_year, 123);
	ASSERT_EQ(tm.tm_wday, 5);
	ASSERT_EQ(tm.tm_yday, 152);
	ASSERT_NE(ctime_r(&t, test), NULL);
	ASSERT_STR_EQ(test, "Fri Jun  2 07:34:38 2023\n");

	t = 1709164799;
	ASSERT_NE(gmtime_r(&t, &tm), NULL);
	ASSERT_EQ(tm.tm_sec, 59);
	ASSERT_EQ(tm.tm_min, 59);
	ASSERT_EQ(tm.tm_hour, 23);
	ASSERT_EQ(tm.tm_mday, 28);
	ASSERT_EQ(tm.tm_mon, 1);
	ASSERT_EQ(tm.tm_year, 124);
	ASSERT_EQ(tm.tm_wday, 3);
	ASSERT_EQ(tm.tm_yday, 58);
	ASSERT_NE(ctime_r(&t, test), NULL);
	ASSERT_STR_EQ(test, "Wed Feb 28 23:59:59 2024\n");

	t = 1709164800;
	ASSERT_NE(gmtime_r(&t, &tm), NULL);
	ASSERT_EQ(tm.tm_sec, 0);
	ASSERT_EQ(tm.tm_min, 0);
	ASSERT_EQ(tm.tm_hour, 0);
	ASSERT_EQ(tm.tm_mday, 29);
	ASSERT_EQ(tm.tm_mon, 1);
	ASSERT_EQ(tm.tm_year, 124);
	ASSERT_EQ(tm.tm_wday, 4);
	ASSERT_EQ(tm.tm_yday, 59);
	ASSERT_NE(ctime_r(&t, test), NULL);
	ASSERT_STR_EQ(test, "Thu Feb 29 00:00:00 2024\n");
}

void test_strftime(void)
{
	time_t t = 1661928356;
	struct tm tm;
	char buf[4096];

	ASSERT_NE(gmtime_r(&t, &tm), NULL);
	ASSERT_NE(strftime(buf, sizeof(buf), "char %a %A %b %B %c %C %d %D %e %F %h %H %I %j %k %l %m %M %n %p %P %r %R %s %S %t %T %u %w %x %X %y %Y", &tm), 0);
	ASSERT_STR_EQ(buf, "char Wed Wednesday Aug August Wed Aug 31 06:45:56 2022 20 31 08/31/22 31 2022-08-31 Aug 06 06 243  6  6 08 45 \n AM am 06:45:56 AM 06:45 1661928356 56 	 06:45:56 3 3 08/31/22 06:45:56 22 2022");
}

void test_printf(void)
{
	char buf[4096];
#define TEST(expected, fmt, ...) \
do \
{ \
	ASSERT_EQ(snprintf(buf, sizeof(buf), fmt, ##__VA_ARGS__), (int)strlen(expected)); \
	ASSERT_STR_EQ(buf, expected); \
} while (0)

	TEST("test", "test");
	TEST("t0t", "t%dt", 0);
	TEST("t 0t", "t% 2dt", 0);
	TEST("t00t", "t%02dt", 0);
	TEST("t 0t", "t%2dt", 0);
	TEST("t1337t", "t%.1dt", 1337);
	TEST("t01337t", "t%.5dt", 1337);
	TEST("t 01337t", "t%6.5dt", 1337);
	TEST("9", "%d", 9);
	TEST("10", "%d", 10);
	TEST("-1", "%d", -1);
	TEST("-9", "%d", -9);
	TEST("-10", "%d", -10);
	TEST("0", "%x", 0);
	TEST("9", "%x", 9);
	TEST("a", "%x", 10);
	TEST("f", "%x", 15);
	TEST("10", "%x", 16);
	TEST("0x10", "%#x", 16);
	TEST(" 0010", "%5.4x", 16);
	TEST("      0x10", "%#10x", 16);
	TEST("      0x10", "%#10.1x", 16);
	TEST("   0x00010", "%#10.5x", 16);
	TEST("0x10", "%#2x", 16);
	TEST("0x10", "%#2.1x", 16);
	TEST("A", "%X", 10);
	TEST("F", "%X", 15);
	TEST("10", "%X", 16);
	TEST("0X10", "%#X", 16);
	TEST("0", "%o", 0);
	TEST("7", "%o", 7);
	TEST("10", "%o", 8);
	TEST("11", "%o", 9);
	TEST("010", "%#o", 8);
	TEST(" 010", "%#4.3o", 8);
	TEST("       010", "%#10o", 8);
	TEST("       010", "%#10.1o", 8);
	TEST("     00010", "%#10.5o", 8);
	TEST("test", "%s", "test");
	TEST(" test", "%5s", "test");
	TEST("   te", "%5.2s", "test");

#undef TEST
}

void test_fifo(void)
{
	ASSERT_EQ(mkfifo("/fifo_test", 0777), 0);
	ASSERT_EQ(mkfifo("/fifo_test", 0777), -1);
	ASSERT_EQ(errno, EEXIST);
	int fdr = open("/fifo_test", O_RDONLY);
	ASSERT_NE(fdr, -1);
	int fdw = open("/fifo_test", O_WRONLY);
	ASSERT_NE(fdw, -1);
	char buf[5];
	ASSERT_EQ(write(fdw, "test", 4), 4);
	ASSERT_EQ(close(fdw), 0);
	ASSERT_EQ(close(fdw), -1);
	ASSERT_EQ(errno, EBADF);
	ASSERT_EQ(read(fdr, buf, 5), 4);
	ASSERT_EQ(memcmp(buf, "test", 4), 0);
	ASSERT_EQ(close(fdr), 0);
	ASSERT_EQ(close(fdr), -1);
	ASSERT_EQ(errno, EBADF);
	ASSERT_EQ(unlink("/fifo_test"), 0);
}

static int cmp_str(const void *a, const void *b)
{
	return strcmp(*(char**)a, *(char**)b);
}

void test_qsort(void)
{
	char *items[] =
	{
		"yy",
		"a",
		"5",
		"yx",
		"y",
		"4",
		"3",
		"4",
		"4",
		"5",
		"yx",
	};
	qsort(items, sizeof(items) / sizeof(*items), sizeof(*items), cmp_str);
	for (size_t i = 1; i < sizeof(items) / sizeof(*items); ++i)
		ASSERT_STR_LE(items[i - 1], items[i]);
}

void test_dl(void)
{
	/* XXX it should be working with /lib/libc.so too */
	void *handle = dlopen("/lib/libc.so.1", RTLD_NOW);
	ASSERT_NE(handle, NULL);
	typedef void (*puts_fn_t)(const char *s);
	puts_fn_t puts_fn = dlsym(handle, "puts");
	ASSERT_NE(puts_fn, NULL);
	ASSERT_EQ((void*)puts_fn, (void*)puts);
	dlclose(handle);
}

void test_pf_local(void)
{
	unlink("/tmp/pf_local");
	int ssock = socket(PF_LOCAL, SOCK_STREAM, 0);
	ASSERT_NE(ssock, -1);
	struct sockaddr_un sun;
	sun.sun_family = PF_LOCAL;
	strcpy(sun.sun_path, "/tmp/pf_local");
	ASSERT_EQ(bind(ssock, (struct sockaddr*)&sun, sizeof(sun)), 0);
	ASSERT_EQ(listen(ssock, 128), 0);
	int child = fork();
	ASSERT_NE(child, -1);
	if (!child)
	{
		int csock = socket(PF_LOCAL, SOCK_STREAM, 0);
		ASSERT_NE(csock, -1);
		ASSERT_EQ(connect(csock, (struct sockaddr*)&sun, sizeof(sun)), 0);
		uint8_t buf[128];
		for (size_t i = 0; i < 128; ++i)
			buf[i] = i;
		ASSERT_EQ(send(csock, buf, 64, 0), 64);
		ASSERT_EQ(recv(csock, buf, 65, 0), 64);
		for (size_t i = 0; i < 64; ++i)
			ASSERT_EQ(buf[i], i + 1);
		exit(EXIT_SUCCESS);
	}
	socklen_t sunlen = sizeof(sun);
	int csock = accept(ssock, (struct sockaddr*)&sun, &sunlen);
	ASSERT_NE(csock, -1);
	uint8_t buf[128];
	ASSERT_EQ(recv(csock, buf, 65, 0), 64);
	for (size_t i = 0; i < 64; ++i)
	{
		ASSERT_EQ(buf[i], i);
		buf[i] = i + 1;
	}
	ASSERT_EQ(send(csock, buf, 64, 0), 64);
	int status;
	ASSERT_EQ(waitpid(child, &status, 0), child);
	ASSERT_EQ(recv(csock, buf, 64, 0), 0);
	ASSERT_EQ(send(csock, buf, 64, 0), -1);
	ASSERT_EQ(errno, EPIPE);
	close(csock);
	close(ssock);
	unlink("/tmp/pf_local");
}

void test_inet(void)
{
	struct in_addr in;
	char buf[256];

	ASSERT_EQ(inet_pton(0xFF, NULL, NULL), -1);
	ASSERT_EQ(errno, EAFNOSUPPORT);

	ASSERT_EQ(inet_pton(AF_INET, "1.2.3.4", &in), 1);
	ASSERT_EQ(in.s_addr, 0x04030201);

	ASSERT_EQ(inet_pton(AF_INET, "1", &in), 0);

	ASSERT_EQ(inet_pton(AF_INET, "1.2.3.255", &in), 1);
	ASSERT_EQ(in.s_addr, 0xFF030201);

	ASSERT_EQ(inet_pton(AF_INET, "1.2.3.256", &in), 0);

	ASSERT_EQ(inet_ntop(0xFF, NULL, NULL, 0), 0);
	ASSERT_EQ(errno, EAFNOSUPPORT);

	in.s_addr = 0x04030201;
	ASSERT_NE(inet_ntop(AF_INET, &in, buf, sizeof(buf)), 0);
	ASSERT_STR_EQ(buf, "1.2.3.4");

	ASSERT_EQ(inet_ntop(AF_INET, &in, buf, 1), 0);
	ASSERT_EQ(errno, ENOSPC);
}

void test_libz(void)
{
	ASSERT_EQ(adler32(0, NULL, 0), 1);
	ASSERT_EQ(adler32(1, (uint8_t*)"test", 4), 0x045D01C1);
	ASSERT_EQ(adler32(1, (uint8_t*)"abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26 * 4 + 20), 0xA4F22A17);
	ASSERT_EQ(adler32(adler32(adler32(adler32(1, (uint8_t*)"t", 1), (uint8_t*)"e", 1), (uint8_t*)"s", 1), (uint8_t*)"t", 1), 0x045D01C1);

	ASSERT_EQ(crc32(0, NULL, 0), 0);
	ASSERT_EQ(crc32(0, (uint8_t*)"test", 4), 0xD87F7E0C);
	ASSERT_EQ(crc32(0, (uint8_t*)"abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26 * 4 + 20), 0x5C45CDB1);
	ASSERT_EQ(crc32(crc32(crc32(crc32(0, (uint8_t*)"t", 1), (uint8_t*)"e", 1), (uint8_t*)"s", 1), (uint8_t*)"t", 1), 0xD87F7E0C);
}

void test_servent(void)
{
	struct servent *servent;

	servent = getservbyname("http", NULL);
	ASSERT_NE(servent, NULL);
	ASSERT_STR_EQ(servent->s_name, "http");
	ASSERT_EQ(servent->s_port, htons(80));
	ASSERT_STR_EQ(servent->s_proto, "tcp");

	servent = getservbyname("http", "tcp");
	ASSERT_NE(servent, NULL);
	ASSERT_STR_EQ(servent->s_name, "http");
	ASSERT_EQ(servent->s_port, htons(80));
	ASSERT_STR_EQ(servent->s_proto, "tcp");

	servent = getservbyname("http", "udp");
	ASSERT_EQ(servent, NULL);

	servent = getservbyname("osef", NULL);
	ASSERT_EQ(servent, NULL);

	servent = getservbyport(htons(80), NULL);
	ASSERT_NE(servent, NULL);
	ASSERT_STR_EQ(servent->s_name, "http");
	ASSERT_EQ(servent->s_port, htons(80));
	ASSERT_STR_EQ(servent->s_proto, "tcp");

	servent = getservbyport(htons(80), "tcp");
	ASSERT_NE(servent, NULL);
	ASSERT_STR_EQ(servent->s_name, "http");
	ASSERT_EQ(servent->s_port, htons(80));
	ASSERT_STR_EQ(servent->s_proto, "tcp");

	servent = getservbyport(htons(80), "udp");
	ASSERT_EQ(servent, NULL);

	servent = getservbyport(htons(1337), NULL);
	ASSERT_EQ(servent, NULL);
}

void test_hostent(void)
{
	struct hostent *hostent;

	hostent = gethostbyname("localhost");
	ASSERT_NE(hostent, NULL);
	ASSERT_STR_EQ(hostent->h_name, "localhost");
	ASSERT_EQ(hostent->h_addrtype, AF_INET);
	ASSERT_EQ(hostent->h_length, sizeof(struct in_addr));
	ASSERT_EQ(((struct in_addr*)hostent->h_addr_list[0])->s_addr, 0x0100007F);

	/* XXX make this faster */
#if 0
	hostent = gethostbyname("doesntexists");
	ASSERT_EQ(hostent, NULL);
#endif

	struct in_addr addr;
	addr.s_addr = 0x0100007F;
	hostent = gethostbyaddr(&addr, sizeof(addr), AF_INET);
	ASSERT_NE(hostent, NULL);
	ASSERT_STR_EQ(hostent->h_name, "localhost");
	ASSERT_EQ(hostent->h_addrtype, AF_INET);
	ASSERT_EQ(hostent->h_length, sizeof(struct in_addr));
	ASSERT_EQ(((struct in_addr*)hostent->h_addr_list[0])->s_addr, 0x0100007F);

	hostent = gethostbyaddr(&addr, 1, AF_INET);
	ASSERT_EQ(hostent, NULL);

	hostent = gethostbyaddr(&addr, sizeof(addr), AF_LOCAL);
	ASSERT_EQ(hostent, NULL);

	addr.s_addr = 0x0200007F;
	hostent = gethostbyaddr(&addr, sizeof(addr), AF_INET);
	ASSERT_EQ(hostent, NULL);
}

void test_protoent(void)
{
	struct protoent *protoent;

	protoent = getprotobyname("tcp");
	ASSERT_NE(protoent, NULL);
	ASSERT_STR_EQ(protoent->p_name, "tcp");
	ASSERT_EQ(protoent->p_proto, IPPROTO_TCP);
	protoent = getprotobyname("nope");
	ASSERT_EQ(protoent, NULL);

	protoent = getprotobynumber(IPPROTO_UDP);
	ASSERT_NE(protoent, NULL);
	ASSERT_STR_EQ(protoent->p_name, "udp");
	ASSERT_EQ(protoent->p_proto, IPPROTO_UDP);
	protoent = getprotobynumber(253);
	ASSERT_EQ(protoent, NULL);
}

void test_cloexec(void)
{
	int fd1 = open("/bin/ls", O_RDONLY);
	ASSERT_NE(fd1, -1);
	int fd2 = open("/bin/ls", O_RDONLY | O_CLOEXEC);
	ASSERT_NE(fd2, -1);
	int fd3 = open("/bin/ls", O_RDONLY);
	ASSERT_NE(fd3, -1);
	ASSERT_EQ(fcntl(fd1, F_GETFD, 0), 0);
	ASSERT_EQ(fcntl(fd2, F_GETFD, 0), FD_CLOEXEC);
	ASSERT_EQ(fcntl(fd3, F_GETFD, 0), 0);
	ASSERT_EQ(fcntl(fd3, F_SETFD, FD_CLOEXEC), 0);
	ASSERT_EQ(fcntl(fd3, F_GETFD, 0), FD_CLOEXEC);
	ASSERT_EQ(close(fd1), 0);
	ASSERT_EQ(close(fd2), 0);
	ASSERT_EQ(close(fd3), 0);
}

void test_fetch(void)
{
	struct url *url;

	url = fetchMakeURL("pouet", "ouais", 29, ":3", "user", "pwd");
	ASSERT_NE(url, NULL);
	ASSERT_STR_EQ(url->scheme, "pouet");
	ASSERT_STR_EQ(url->host, "ouais");
	ASSERT_STR_EQ(url->doc, ":3");
	ASSERT_STR_EQ(url->user, "user");
	ASSERT_STR_EQ(url->pwd, "pwd");
	ASSERT_EQ(url->port, 29);
	fetchFreeURL(url);

	url = fetchMakeURL("a", "a", UINT16_MAX, "a", "a", "a");
	ASSERT_NE(url, NULL);
	fetchFreeURL(url);

	url = fetchMakeURL("a", "a", UINT16_MAX + 1, "a", "a", "a");
	ASSERT_EQ(url, NULL);
	fetchFreeURL(url);

	url = fetchMakeURL("a", "a", 0, "a", "a", "a");
	ASSERT_NE(url, NULL);
	fetchFreeURL(url);

	url = fetchMakeURL("a", "a", -1, "a", "a", "a");
	ASSERT_EQ(url, NULL);
	fetchFreeURL(url);

	url = fetchParseURL("http://user:pwd@host:80/doc");
	ASSERT_NE(url, NULL);
	ASSERT_STR_EQ(url->scheme, "http");
	ASSERT_STR_EQ(url->host, "host");
	ASSERT_STR_EQ(url->doc, "/doc");
	ASSERT_STR_EQ(url->user, "user");
	ASSERT_STR_EQ(url->pwd, "pwd");
	ASSERT_EQ(url->port, 80);
	fetchFreeURL(url);

	url = fetchParseURL("http://user@host:80/doc");
	ASSERT_NE(url, NULL);
	ASSERT_STR_EQ(url->scheme, "http");
	ASSERT_STR_EQ(url->host, "host");
	ASSERT_STR_EQ(url->doc, "/doc");
	ASSERT_STR_EQ(url->user, "user");
	ASSERT_STR_EQ(url->pwd, "");
	ASSERT_EQ(url->port, 80);
	fetchFreeURL(url);

	url = fetchParseURL("http://user@host/doc");
	ASSERT_NE(url, NULL);
	ASSERT_STR_EQ(url->scheme, "http");
	ASSERT_STR_EQ(url->host, "host");
	ASSERT_STR_EQ(url->doc, "/doc");
	ASSERT_STR_EQ(url->user, "user");
	ASSERT_STR_EQ(url->pwd, "");
	ASSERT_EQ(url->port, 0);
	fetchFreeURL(url);

	url = fetchParseURL("http://host/doc");
	ASSERT_NE(url, NULL);
	ASSERT_STR_EQ(url->scheme, "http");
	ASSERT_STR_EQ(url->host, "host");
	ASSERT_STR_EQ(url->doc, "/doc");
	ASSERT_STR_EQ(url->user, "");
	ASSERT_STR_EQ(url->pwd, "");
	ASSERT_EQ(url->port, 0);
	fetchFreeURL(url);

	url = fetchParseURL("http://host");
	ASSERT_NE(url, NULL);
	ASSERT_STR_EQ(url->scheme, "http");
	ASSERT_STR_EQ(url->host, "host");
	ASSERT_STR_EQ(url->doc, "/");
	ASSERT_STR_EQ(url->user, "");
	ASSERT_STR_EQ(url->pwd, "");
	ASSERT_EQ(url->port, 0);
	fetchFreeURL(url);
}

static volatile int signal_mark;
static volatile int siginfo_mark;
static volatile void *altstack;
static volatile size_t altstack_size;

static void sigint_handler(int signum)
{
	ASSERT_EQ(signal_mark, 0);
	ASSERT_EQ(signum, SIGINT);
	signal_mark = 1;
}

static void siginfo_handler(int signum, siginfo_t *siginfo, void *ctx)
{
	ASSERT_EQ(siginfo_mark, 0);
	ASSERT_EQ(signum, SIGINT);
	ASSERT_NE(siginfo, NULL);
	ASSERT_NE(ctx, NULL);
	ASSERT_EQ(siginfo->si_signo, SIGINT);
	stack_t ss;
	ASSERT_EQ(sigaltstack(NULL, &ss), 0);
	ASSERT_EQ(ss.ss_flags, SS_ONSTACK);
	ASSERT_GE((uintptr_t)siginfo, (uintptr_t)altstack);
	ASSERT_LE((uintptr_t)siginfo + sizeof(*siginfo), (uintptr_t)altstack + altstack_size);
	siginfo_mark = 1;
}

void test_signal(void)
{
	ASSERT_EQ(signal(SIGINT, sigint_handler), SIG_DFL);
	ASSERT_EQ(raise(SIGINT), 0);
	ASSERT_EQ(signal_mark, 1);

	altstack_size = (SIGSTKSZ + 4095) & ~4095;
	altstack = mmap(NULL, altstack_size, PROT_READ | PROT_WRITE,
	                MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	ASSERT_NE(altstack, MAP_FAILED);
	stack_t ss;
	ss.ss_sp = (void*)altstack;
	ss.ss_flags = 0;
	ss.ss_size = altstack_size;
	ASSERT_EQ(sigaltstack(&ss, NULL), 0);
	ss.ss_sp = NULL;
	ASSERT_EQ(sigaltstack(NULL, &ss), 0);
	ASSERT_EQ(ss.ss_sp, altstack);
	struct sigaction act;
	act.sa_sigaction = siginfo_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_SIGINFO | SA_RESTORER | SA_ONSTACK;
	act.sa_restorer = sigreturn;
	ASSERT_EQ(sigaction(SIGINT, &act, NULL), 0);
	act.sa_restorer = NULL;
	ASSERT_EQ(sigaction(SIGINT, NULL, &act), 0);
	ASSERT_EQ(act.sa_restorer, sigreturn);
	ASSERT_EQ(raise(SIGINT), 0);
	ASSERT_EQ(siginfo_mark, 1);
	ASSERT_EQ(sigaltstack(NULL, &ss), 0);
	ASSERT_EQ(ss.ss_flags, 0);

	ASSERT_EQ(signal(SIGINT, SIG_DFL), (void*)siginfo_handler);
}

void test_getline(void)
{
	FILE *fp;
	char *line = NULL;
	size_t len = 0;

	fp = fopen("/tmp/test.txt", "w");
	fprintf(fp, "a\nbb\nccc\n");
	fclose(fp);
	fp = fopen("/tmp/test.txt", "r");
	ASSERT_EQ(getline(&line, &len, fp), 2);
	ASSERT_STR_EQ(line, "a\n");
	ASSERT_EQ(getline(&line, &len, fp), 3);
	ASSERT_STR_EQ(line, "bb\n");
	ASSERT_EQ(getline(&line, &len, fp), 4);
	ASSERT_STR_EQ(line, "ccc\n");
	ASSERT_EQ(getline(&line, &len, fp), -1);
	fclose(fp);

	fp = fopen("/tmp/test.txt", "w");
	fprintf(fp, "a\nbb\nccc");
	fclose(fp);
	fp = fopen("/tmp/test.txt", "r");
	ASSERT_EQ(getline(&line, &len, fp), 2);
	ASSERT_STR_EQ(line, "a\n");
	ASSERT_EQ(getline(&line, &len, fp), 3);
	ASSERT_STR_EQ(line, "bb\n");
	ASSERT_EQ(getline(&line, &len, fp), 3);
	ASSERT_STR_EQ(line, "ccc");
	ASSERT_EQ(getline(&line, &len, fp), -1);
	fclose(fp);
}

void test_fgets(void)
{
	FILE *fp;
	char buf[10];

	fp = fopen("/tmp/test.txt", "w");
	fprintf(fp, "a\nbb\nccc\n");
	fclose(fp);
	fp = fopen("/tmp/test.txt", "r");
	ASSERT_NE(fgets(buf, 2, fp), NULL);
	ASSERT_STR_EQ(buf, "a");
	ASSERT_NE(fgets(buf, 2, fp), NULL);
	ASSERT_STR_EQ(buf, "\n");
	ASSERT_NE(fgets(buf, 2, fp), NULL);
	ASSERT_STR_EQ(buf, "b");
	ASSERT_NE(fgets(buf, 2, fp), NULL);
	ASSERT_STR_EQ(buf, "b");
	ASSERT_NE(fgets(buf, 2, fp), NULL);
	ASSERT_STR_EQ(buf, "\n");
	ASSERT_NE(fgets(buf, 10, fp), NULL);
	ASSERT_STR_EQ(buf, "ccc\n");
	ASSERT_EQ(fgets(buf, 10, fp), NULL);
	fclose(fp);

	fp = fopen("/tmp/test.txt", "w");
	fprintf(fp, "a\nbb\nccc");
	fclose(fp);
	fp = fopen("/tmp/test.txt", "r");
	ASSERT_NE(fgets(buf, 2, fp), NULL);
	ASSERT_STR_EQ(buf, "a");
	ASSERT_NE(fgets(buf, 2, fp), NULL);
	ASSERT_STR_EQ(buf, "\n");
	ASSERT_NE(fgets(buf, 2, fp), NULL);
	ASSERT_STR_EQ(buf, "b");
	ASSERT_NE(fgets(buf, 2, fp), NULL);
	ASSERT_STR_EQ(buf, "b");
	ASSERT_NE(fgets(buf, 2, fp), NULL);
	ASSERT_STR_EQ(buf, "\n");
	ASSERT_NE(fgets(buf, 10, fp), NULL);
	ASSERT_STR_EQ(buf, "ccc");
	ASSERT_EQ(fgets(buf, 10, fp), NULL);
	fclose(fp);
}

void test_socketpair(void)
{
	int fds[2];
	ASSERT_NE(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), -1);
	ASSERT_EQ(send(fds[1], "bonjour", 7, 0), 7);
	char buf[10];
	ASSERT_EQ(recv(fds[0], buf, 4, 0), 4);
	ASSERT_EQ(memcmp(buf, "bonj", 4), 0);
	ASSERT_EQ(close(fds[1]), 0);
	ASSERT_EQ(recv(fds[0], buf, 4, 0), 3);
	ASSERT_EQ(memcmp(buf, "our", 3), 0);
	ASSERT_EQ(close(fds[0]), 0);
}

void test_basename(void)
{
	char path[MAXPATHLEN];
	char *ret;

	ASSERT_STR_EQ(basename(NULL), ".");

	strlcpy(path, "", sizeof(path));
	ret = basename(path);
	ASSERT_STR_EQ(ret, ".");

	strlcpy(path, "/", sizeof(path));
	ret = basename(path);
	ASSERT_STR_EQ(ret, "/");

	strlcpy(path, "//", sizeof(path));
	ret = basename(path);
	ASSERT_STR_EQ(ret, "/");

	strlcpy(path, "test", sizeof(path));
	ret = basename(path);
	ASSERT_STR_EQ(ret, "test");

	strlcpy(path, "test/oui", sizeof(path));
	ret = basename(path);
	ASSERT_STR_EQ(ret, "oui");

	strlcpy(path, "test/oui/non", sizeof(path));
	ret = basename(path);
	ASSERT_STR_EQ(ret, "non");

	strlcpy(path, "/test/oui/non", sizeof(path));
	ret = basename(path);
	ASSERT_STR_EQ(ret, "non");

	strlcpy(path, "test/oui/non/", sizeof(path));
	ret = basename(path);
	ASSERT_STR_EQ(ret, "non");

	strlcpy(path, "test/oui/non//", sizeof(path));
	ret = basename(path);
	ASSERT_STR_EQ(ret, "non");

	strlcpy(path, "test///oui////non", sizeof(path));
	ret = basename(path);
	ASSERT_STR_EQ(ret, "non");
}

void test_dirname(void)
{
	char path[MAXPATHLEN];
	char *ret;

	ASSERT_STR_EQ(dirname(NULL), ".");

	strlcpy(path, "", sizeof(path));
	ret = dirname(path);
	ASSERT_STR_EQ(ret, ".");

	strlcpy(path, "/", sizeof(path));
	ret = dirname(path);
	ASSERT_STR_EQ(ret, "/");

	strlcpy(path, "//", sizeof(path));
	ret = dirname(path);
	ASSERT_STR_EQ(ret, "/");

	strlcpy(path, "test", sizeof(path));
	ret = dirname(path);
	ASSERT_STR_EQ(ret, ".");

	strlcpy(path, "/test", sizeof(path));
	ret = dirname(path);
	ASSERT_STR_EQ(ret, "/");

	strlcpy(path, "/test/oui", sizeof(path));
	ret = dirname(path);
	ASSERT_STR_EQ(ret, "/test");

	strlcpy(path, "test/oui", sizeof(path));
	ret = dirname(path);
	ASSERT_STR_EQ(ret, "test");

	strlcpy(path, "test///oui", sizeof(path));
	ret = dirname(path);
	ASSERT_STR_EQ(ret, "test");

	strlcpy(path, "test/oui/", sizeof(path));
	ret = dirname(path);
	ASSERT_STR_EQ(ret, "test");

	strlcpy(path, "test/oui/non", sizeof(path));
	ret = dirname(path);
	ASSERT_STR_EQ(ret, "test/oui");

	strlcpy(path, "test/oui///non", sizeof(path));
	ret = dirname(path);
	ASSERT_STR_EQ(ret, "test/oui");

	strlcpy(path, "test/oui/non///", sizeof(path));
	ret = dirname(path);
	ASSERT_STR_EQ(ret, "test/oui");
}

void test_popen(void)
{
	char buf[10];
	FILE *fp = popen("echo salut", "r");
	ASSERT_NE(fp, NULL);
	ASSERT_EQ(fread(buf, 1, 6, fp), 6);
	buf[6] = '\0';
	ASSERT_STR_EQ(buf, "salut\n");
	pclose(fp);
}

void test_fmemopen(void)
{
	char buf[15];
	char membuf[50] = "ouais";
	FILE *fp;

	fp = fmemopen(membuf, sizeof(membuf), "a+");
	ASSERT_NE(fp, NULL);
	setbuf(fp, NULL);
	ASSERT_EQ(ftell(fp), 5);
	ASSERT_EQ(fputs("bonjour", fp), 7);
	ASSERT_EQ(ftell(fp), 12);
	ASSERT_EQ(fflush(fp), 0);
	ASSERT_EQ(ftell(fp), 12);
	ASSERT_STR_EQ(membuf, "ouaisbonjour");
	ASSERT_EQ(fseek(fp, 0, SEEK_SET), 0);
	ASSERT_EQ(fread(buf, 1, sizeof(buf), fp), 12);
	buf[12] = '\0';
	ASSERT_STR_EQ(buf, "ouaisbonjour");
	ASSERT_EQ(fclose(fp), 0);

	fp = fmemopen(NULL, 6, "w+");
	ASSERT_NE(fp, NULL);
	setbuf(fp, NULL);
	ASSERT_EQ(fputs("bonjour", fp), 7);
	ASSERT_EQ(fflush(fp), 0);
	ASSERT_EQ(fseek(fp, 0, SEEK_SET), 0);
	ASSERT_EQ(fread(buf, 1, sizeof(buf), fp), 6);
	buf[6] = '\0';
	ASSERT_STR_EQ(buf, "bonjou");
	ASSERT_EQ(fclose(fp), 0);
}

static int byte_cmp(const void *a, const void *b)
{
	return (int)*(uint8_t*)a - (int)*(uint8_t*)b;
}

void test_bsearch(void)
{
	static const uint8_t buf[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};

	for (size_t i = 0; i < sizeof(buf); ++i)
	{
		for (uint8_t j = 0; j < sizeof(buf); ++j)
		{
			void *ptr = bsearch(&j, buf, i, 1, byte_cmp);
			if (j >= i)
				ASSERT_EQ(ptr, NULL);
			else
				ASSERT_EQ(ptr, &buf[j]);
		}
	}
}

void test_scanf(void)
{
	int ret;
	int i[10];
	char c[10];
	void *p;

	ret = sscanf("salut", "salut");
	ASSERT_EQ(ret, 0);
	ret = sscanf("salut", "salutt");
	ASSERT_EQ(ret, EOF);
	ret = sscanf("salut", "salu");
	ASSERT_EQ(ret, 0);
	ret = sscanf("salut7", "salut%d", &i[0]);
	ASSERT_EQ(ret, 1);
	ASSERT_EQ(i[0], 7);
	ret = sscanf("salut10a4", "salut%da%d", &i[0], &i[1]);
	ASSERT_EQ(ret, 2);
	ASSERT_EQ(i[0], 10);
	ASSERT_EQ(i[1], 4);
	ret = sscanf("salut0x1fg2", "salut%ig%i", &i[0], &i[1]);
	ASSERT_EQ(ret, 2);
	ASSERT_EQ(i[0], 31);
	ASSERT_EQ(i[1], 2);
	ret = sscanf("salut01784", "salut%i8%d", &i[0], &i[1]);
	ASSERT_EQ(ret, 2);
	ASSERT_EQ(i[0], 15);
	ASSERT_EQ(i[1], 4);
	ret = sscanf("133713381339 2", "%d %d", &i[0], &i[1]);
	ASSERT_EQ(ret, 2);
	ASSERT_EQ(i[0], 569395163);
	ASSERT_EQ(i[1], 2);
	ret = sscanf("salut20", "salut%i%n", &i[0], &i[1]);
	ASSERT_EQ(ret, 1);
	ASSERT_EQ(i[0], 20);
	ASSERT_EQ(i[1], 7);
	ret = sscanf("salut20", "salut%*i%n", &i[0]);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(i[0], 7);
	ret = sscanf("salut      \t\t    20", "salut %d", &i[0]);
	ASSERT_EQ(ret, 1);
	ASSERT_EQ(i[0], 20);
	ret = sscanf("salutt22", "salut%c%d", &c[0], &i[0]);
	ASSERT_EQ(ret, 2);
	ASSERT_EQ(c[0], 't');
	ASSERT_EQ(i[0], 22);
	c[2] = 5;
	ret = sscanf("salutuia", "salut%2ca", &c[0]);
	ASSERT_EQ(ret, 1);
	ASSERT_EQ(c[0], 'u');
	ASSERT_EQ(c[1], 'i');
	ASSERT_EQ(c[2], 5);
	ret = sscanf("salutopa", "salut%2sa", &c[0]);
	ASSERT_EQ(ret, 1);
	ASSERT_EQ(c[0], 'o');
	ASSERT_EQ(c[1], 'p');
	ASSERT_EQ(c[2], '\0');
	ret = sscanf("saluto r", "salut%4sr", &c[0]);
	ASSERT_EQ(ret, 1);
	ASSERT_EQ(c[0], 'o');
	ASSERT_EQ(c[1], '\0');
	ret = sscanf("salut0o", "salut%p%c", &p, &c[0]);
	ASSERT_EQ(ret, 2);
	ASSERT_EQ(p, NULL);
	ASSERT_EQ(c[0], 'o');
	ret = sscanf("salut1p", "salut%p%c", &p, &c[0]);
	ASSERT_EQ(ret, 2);
	ASSERT_EQ(p, (void*)1);
	ASSERT_EQ(c[0], 'p');
	ret = sscanf("salut0x1i", "salut%p%c", &p, &c[0]);
	ASSERT_EQ(ret, 2);
	ASSERT_EQ(p, (void*)1);
	ASSERT_EQ(c[0], 'i');
	ret = sscanf("salut(nil)a", "salut%p%c", &p, &c[0]);
	ASSERT_EQ(ret, 2);
	ASSERT_EQ(p, NULL);
	ASSERT_EQ(c[0], 'a');
	ret = sscanf("salut(nl)", "salut%p", &p);
	ASSERT_EQ(ret, 0);
	ret = sscanf("salutt", "salut%d", &i[0]);
	ASSERT_EQ(ret, 0);
	ret = sscanf("salut128", "salut%o8", &i[0]);
	ASSERT_EQ(ret, 1);
	ASSERT_EQ(i[0], 10);
	ret = sscanf("salut0x1eg", "salut%x%c", &i[0], &c[0]);
	ASSERT_EQ(ret, 2);
	ASSERT_EQ(i[0], 30);
	ASSERT_EQ(c[0], 'g');
	ret = sscanf("salut01du", "salut%x%c", &i[0], &c[0]);
	ASSERT_EQ(ret, 2);
	ASSERT_EQ(i[0], 29);
	ASSERT_EQ(c[0], 'u');
	ret = sscanf("salut0xi", "salut%x%c", &i[0], &c[0]);
	ASSERT_EQ(ret, 2);
	ASSERT_EQ(i[0], 0);
	ASSERT_EQ(c[0], 'i');
	ret = sscanf("salut0o", "salut%x%c", &i[0], &c[0]);
	ASSERT_EQ(ret, 2);
	ASSERT_EQ(i[0], 0);
	ASSERT_EQ(c[0], 'o');
}
