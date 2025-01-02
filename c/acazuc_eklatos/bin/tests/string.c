#include "tests.h"

#include <string.h>
#include <stdio.h>

void test_strlen(void)
{
	char buf[16];

	ASSERT_EQ(memset(buf, 0, sizeof(buf)), buf);
	ASSERT_EQ(memset(&buf[3], 0xAC, 9), &buf[3]);
	ASSERT_EQ(strlen(&buf[0]), 0);
	ASSERT_EQ(strlen(&buf[2]), 0);
	ASSERT_EQ(strlen(&buf[3]), 9);
	ASSERT_EQ(strlen(&buf[4]), 8);
	ASSERT_EQ(strlen(&buf[5]), 7);
	ASSERT_EQ(strlen(&buf[6]), 6);
	ASSERT_EQ(strlen(&buf[7]), 5);
	ASSERT_EQ(strlen(&buf[8]), 4);
	ASSERT_EQ(strlen(&buf[9]), 3);
	ASSERT_EQ(strlen(&buf[10]), 2);
	ASSERT_EQ(strlen(&buf[11]), 1);
	ASSERT_EQ(strlen(&buf[12]), 0);
	ASSERT_EQ(strlen(&buf[13]), 0);
}

void test_memchr(void)
{
	char buf[16] = {0, 1, 3, 3, 4, 5, 6, 7, 8, 9, 11, 11, 12, 13, 14, 15};

	ASSERT_EQ(memchr(&buf[0] , 0 , 10), &buf[0]);
	ASSERT_EQ(memchr(&buf[1] , 11, 15), &buf[10]);
	ASSERT_EQ(memchr(&buf[2] , 11, 14), &buf[10]);
	ASSERT_EQ(memchr(&buf[3] , 11, 13), &buf[10]);
	ASSERT_EQ(memchr(&buf[4] , 11, 12), &buf[10]);
	ASSERT_EQ(memchr(&buf[5] , 11, 11), &buf[10]);
	ASSERT_EQ(memchr(&buf[6] , 11, 10), &buf[10]);
	ASSERT_EQ(memchr(&buf[7] , 11, 9 ), &buf[10]);
	ASSERT_EQ(memchr(&buf[8] , 11, 8 ), &buf[10]);
	ASSERT_EQ(memchr(&buf[9] , 11, 7 ), &buf[10]);
	ASSERT_EQ(memchr(&buf[10], 11, 6 ), &buf[10]);
	ASSERT_EQ(memchr(&buf[11], 11, 5 ), &buf[11]);
	ASSERT_EQ(memchr(&buf[12], 11, 4 ), NULL);
}

void test_memrchr(void)
{
	char buf[16] = {0, 1, 3, 3, 4, 5, 6, 7, 8, 9, 11, 11, 12, 13, 14, 15};

	ASSERT_EQ(memrchr(buf, 15, 16), &buf[15]);
	ASSERT_EQ(memrchr(buf, 3, 14), &buf[3]);
	ASSERT_EQ(memrchr(buf, 3, 13), &buf[3]);
	ASSERT_EQ(memrchr(buf, 3, 12), &buf[3]);
	ASSERT_EQ(memrchr(buf, 3, 11), &buf[3]);
	ASSERT_EQ(memrchr(buf, 3, 10), &buf[3]);
	ASSERT_EQ(memrchr(buf, 3, 9 ), &buf[3]);
	ASSERT_EQ(memrchr(buf, 3, 8 ), &buf[3]);
	ASSERT_EQ(memrchr(buf, 3, 7 ), &buf[3]);
	ASSERT_EQ(memrchr(buf, 3, 6 ), &buf[3]);
	ASSERT_EQ(memrchr(buf, 3, 5 ), &buf[3]);
	ASSERT_EQ(memrchr(buf, 3, 4 ), &buf[3]);
	ASSERT_EQ(memrchr(buf, 3, 3 ), &buf[2]);
	ASSERT_EQ(memrchr(buf, 3, 2 ), NULL);
}

void test_memmem(void)
{
	const char *hs = "ouiouinonoui";

	ASSERT_EQ(memmem(hs, 12, "nun", 3), NULL);
	ASSERT_EQ(memmem(hs, 12, "non", 3), &hs[6]);
	ASSERT_EQ(memmem(hs, 12, hs, 12), hs);
	ASSERT_EQ(memmem(hs, 12, "noui", 4), &hs[8]);
	ASSERT_EQ(memmem(hs, 12, "nouo", 3), &hs[8]);
	ASSERT_EQ(memmem(hs, 11, "noui", 4), NULL);
}

void test_strnlen(void)
{
	ASSERT_EQ(strnlen("ouioui", 1), 1);
	ASSERT_EQ(strnlen("ouioui", 3), 3);
	ASSERT_EQ(strnlen("ouioui", 4), 4);
	ASSERT_EQ(strnlen("ouioui", 5), 5);
	ASSERT_EQ(strnlen("ouioui", 6), 6);
	ASSERT_EQ(strnlen("ouioui", 7), 6);
}

void test_strcpy(void)
{
	char dst[16];

	ASSERT_EQ(memset(dst, 0, sizeof(dst)), dst);
	ASSERT_EQ(strcpy(dst, "oui"), dst);
	ASSERT_STR_EQ(dst, "oui");

	ASSERT_EQ(memset(dst, 0, sizeof(dst)), dst);
	ASSERT_EQ(strcpy(dst, "0123456789"), dst);
	ASSERT_STR_EQ(dst, "0123456789");

	ASSERT_EQ(memset(dst, 0, sizeof(dst)), dst);
	ASSERT_EQ(strcpy(dst, "012345678"), dst);
	ASSERT_STR_EQ(dst, "012345678");

	ASSERT_EQ(memset(dst, 0, sizeof(dst)), dst);
	ASSERT_EQ(strcpy(dst, "01234567"), dst);
	ASSERT_STR_EQ(dst, "01234567");

	ASSERT_EQ(memset(dst, 0, sizeof(dst)), dst);
	ASSERT_EQ(strcpy(dst, "0123456"), dst);
	ASSERT_STR_EQ(dst, "0123456");
}

void test_strncpy(void)
{
	char dst[16];

	ASSERT_EQ(memset(dst, 0, sizeof(dst)), dst);
	ASSERT_EQ(strncpy(dst, "0123456789", 0), dst);
	ASSERT_STR_EQ(dst, "");

	ASSERT_EQ(memset(dst, 0, sizeof(dst)), dst);
	ASSERT_EQ(strncpy(dst, "0123456789", 6), dst);
	ASSERT_STR_EQ(dst, "012345");

	ASSERT_EQ(memset(dst, 0, sizeof(dst)), dst);
	ASSERT_EQ(strncpy(dst, "0123456789", 7), dst);
	ASSERT_STR_EQ(dst, "0123456");

	ASSERT_EQ(memset(dst, 0, sizeof(dst)), dst);
	ASSERT_EQ(strncpy(dst, "0123456789", 8), dst);
	ASSERT_STR_EQ(dst, "01234567");

	ASSERT_EQ(memset(dst, 0, sizeof(dst)), dst);
	ASSERT_EQ(strncpy(dst, "0123456789", 9), dst);
	ASSERT_STR_EQ(dst, "012345678");

	ASSERT_EQ(memset(dst, 0, sizeof(dst)), dst);
	ASSERT_EQ(strncpy(dst, "0123456789", 10), dst);
	ASSERT_STR_EQ(dst, "0123456789");

	ASSERT_EQ(memset(dst, 0, sizeof(dst)), dst);
	ASSERT_EQ(strncpy(dst, "0123456789", 11), dst);
	ASSERT_STR_EQ(dst, "0123456789");
}

void test_strlcpy(void)
{
	char dst[16];

	ASSERT_EQ(strlcpy(dst, "oui",  16), 3);
	ASSERT_STR_EQ(dst, "oui");

	ASSERT_EQ(strlcpy(dst, "012345678", 7), 9);
	ASSERT_STR_EQ(dst, "012345");

	ASSERT_EQ(strlcpy(dst, "012345678", 8), 9);
	ASSERT_STR_EQ(dst, "0123456");

	ASSERT_EQ(strlcpy(dst, "012345678", 9), 9);
	ASSERT_STR_EQ(dst, "01234567");

	ASSERT_EQ(strlcpy(dst, "012345678", 10), 9);
	ASSERT_STR_EQ(dst, "012345678");
}

void test_strcat(void)
{
	char dst[16];

	strcpy(dst, "ou");
	strcat(dst, "testio");
	ASSERT_STR_EQ(dst, "outestio");
}

void test_strncat(void)
{
	char dst[16];

	ASSERT_EQ(strcpy(dst, "ou"), dst);
	ASSERT_EQ(strncat(dst, "testiop", 6), dst);
	ASSERT_STR_EQ(dst, "outestio");
}

void test_strlcat(void)
{
	char dst[16];

	ASSERT_EQ(strcpy(dst, "012345"), dst);
	ASSERT_EQ(strlcat(dst, "01234567", 16), 14);
	ASSERT_STR_EQ(dst, "01234501234567");

	ASSERT_EQ(strcpy(dst, "012345"), dst);
	ASSERT_EQ(strlcat(dst, "012345678", 16), 15);
	ASSERT_STR_EQ(dst, "012345012345678");

	ASSERT_EQ(strcpy(dst, "012345"), dst);
	ASSERT_EQ(strlcat(dst, "0123456789", 16), 16);
	ASSERT_STR_EQ(dst, "012345012345678");

	ASSERT_EQ(strcpy(dst, "012345"), dst);
	ASSERT_EQ(strlcat(dst, "01234567890", 16), 17);
	ASSERT_STR_EQ(dst, "012345012345678");
}

void test_strchr(void)
{
	char *str = "0123456789";

	ASSERT_EQ(strchr(str, 'a'), NULL);
	ASSERT_EQ(strchr(str, '0'), &str[0]);
	ASSERT_EQ(strchr(str, '1'), &str[1]);
	ASSERT_EQ(strchr(str, '2'), &str[2]);
	ASSERT_EQ(strchr(str, '3'), &str[3]);
	ASSERT_EQ(strchr(str, '4'), &str[4]);
	ASSERT_EQ(strchr(str, '5'), &str[5]);
	ASSERT_EQ(strchr(str, '6'), &str[6]);
	ASSERT_EQ(strchr(str, '7'), &str[7]);
	ASSERT_EQ(strchr(str, '8'), &str[8]);
	ASSERT_EQ(strchr(str, '9'), &str[9]);
	ASSERT_EQ(strchr(str,  0 ), &str[10]);
	ASSERT_EQ(strchr(&str[3], '3'), &str[3]);
}

void test_strchrnul(void)
{
	char *str = "0123456789";

	ASSERT_EQ(strchrnul(str, 'a'), &str[10]);
	ASSERT_EQ(strchrnul(str, '0'), &str[0]);
	ASSERT_EQ(strchrnul(str, '1'), &str[1]);
	ASSERT_EQ(strchrnul(str, '2'), &str[2]);
	ASSERT_EQ(strchrnul(str, '3'), &str[3]);
	ASSERT_EQ(strchrnul(str, '4'), &str[4]);
	ASSERT_EQ(strchrnul(str, '5'), &str[5]);
	ASSERT_EQ(strchrnul(str, '6'), &str[6]);
	ASSERT_EQ(strchrnul(str, '7'), &str[7]);
	ASSERT_EQ(strchrnul(str, '8'), &str[8]);
	ASSERT_EQ(strchrnul(str, '9'), &str[9]);
	ASSERT_EQ(strchrnul(str,  0 ), &str[10]);
	ASSERT_EQ(strchrnul(&str[3], '3'), &str[3]);
}

void test_strrchr(void)
{
	char *str = "0123456720";

	ASSERT_EQ(strrchr(str, 'a'), NULL);
	ASSERT_EQ(strrchr(str, '0'), &str[9]);
	ASSERT_EQ(strrchr(str, '1'), &str[1]);
	ASSERT_EQ(strrchr(str, '2'), &str[8]);
	ASSERT_EQ(strrchr(str, '3'), &str[3]);
	ASSERT_EQ(strrchr(str, '4'), &str[4]);
	ASSERT_EQ(strrchr(str, '5'), &str[5]);
	ASSERT_EQ(strrchr(str, '6'), &str[6]);
	ASSERT_EQ(strrchr(str, '7'), &str[7]);
	ASSERT_EQ(strrchr(str, '8'), NULL);
	ASSERT_EQ(strrchr(str, '9'), NULL);
	ASSERT_EQ(strrchr(str,  0 ), &str[10]);
	ASSERT_EQ(strrchr(&str[3], '3'), &str[3]);
}

void test_memcmp(void)
{
	ASSERT_EQ(memcmp("1", "1", 1), 0);
	ASSERT_EQ(memcmp("1", "0", 1), 1);
	ASSERT_EQ(memcmp("9", "0", 1), 9);
	ASSERT_EQ(memcmp("0", "9", 1), -9);
	ASSERT_EQ(memcmp("0", "9", 0), 0);
	ASSERT_EQ(memcmp("01", "01", 3), 0);
	ASSERT_EQ(memcmp("01", "02", 1), 0);
}

void test_memcpy(void)
{
	char buf[16];

	ASSERT_EQ(memcpy(buf, "salut", 6), buf);
	ASSERT_STR_EQ(buf, "salut");
	ASSERT_EQ(memcpy(buf, "bonjour", 2), buf);
	ASSERT_STR_EQ(buf, "bolut");
	ASSERT_EQ(memcpy(buf, "ouais", 0), buf);
	ASSERT_STR_EQ(buf, "bolut");
}

void test_memmove(void)
{
	char buf[16];

	ASSERT_EQ(memmove(buf, "salut", 6), buf);
	ASSERT_STR_EQ(buf, "salut");
	ASSERT_EQ(memmove(buf, &buf[1], 5), buf);
	ASSERT_STR_EQ(buf, "alut");
	ASSERT_EQ(memmove(&buf[1], buf, 5), &buf[1]);
	ASSERT_STR_EQ(buf, "aalut");
}

void test_memset(void)
{
	char buf[15] = {0};

	ASSERT_EQ(memset(buf, '1', 2), buf);
	ASSERT_EQ(buf[0], '1');
	ASSERT_EQ(buf[1], '1');
	ASSERT_EQ(buf[2], 0);
	ASSERT_EQ(memset(&buf[1], '2', 3), &buf[1]);
	ASSERT_EQ(buf[0], '1');
	ASSERT_EQ(buf[1], '2');
	ASSERT_EQ(buf[2], '2');
	ASSERT_EQ(buf[3], '2');
	ASSERT_EQ(buf[4], 0);
	ASSERT_EQ(memset(&buf[2], '5', 0), &buf[2]);
	ASSERT_EQ(buf[2], '2');
}

void test_stpcpy(void)
{
	char buf[15] = {0};
	char *it;

	ASSERT_STR_EQ(buf, "");
	it = stpcpy(buf, "test");
	ASSERT_EQ(it, &buf[4]);
	ASSERT_STR_EQ(buf, "test");
	it = stpcpy(it, "oui");
	ASSERT_EQ(it, &buf[7]);
	ASSERT_STR_EQ(buf, "testoui");
	it = stpcpy(it, "");
	ASSERT_EQ(it, &buf[7]);
	ASSERT_STR_EQ(buf, "testoui");
}

void test_stpncpy(void)
{
	char buf[15] = {0};
	char *it;

	buf[0] = '\0';
	memset(&buf[1], 1, sizeof(buf) - 1);
	ASSERT_STR_EQ(buf, "");
	it = stpncpy(buf, "test", 3);
	ASSERT_EQ(it, &buf[3]);
	buf[3] = '\0';
	ASSERT_STR_EQ(buf, "tes");
	it = stpncpy(it, "oui", 5);
	ASSERT_EQ(it, &buf[6]);
	ASSERT_EQ(it[0], '\0');
	ASSERT_EQ(it[1], '\0');
	ASSERT_STR_EQ(buf, "tesoui");
	it = stpncpy(it, "", 3);
	ASSERT_EQ(it, &buf[6]);
	ASSERT_EQ(it[0], '\0');
	ASSERT_EQ(it[1], '\0');
	ASSERT_EQ(it[2], '\0');
	ASSERT_STR_EQ(buf, "tesoui");
}

void test_memccpy(void)
{
	char buf[15] = {0};

	ASSERT_EQ(memccpy(buf, "test", 's', 10), &buf[3]);
	ASSERT_STR_EQ(buf, "tes");
	ASSERT_EQ(memccpy(buf, "test", 'q', 5), NULL);
	ASSERT_STR_EQ(buf, "test");
	ASSERT_EQ(memccpy(buf, "bonjour", '\0', 6), NULL);
	ASSERT_STR_EQ(buf, "bonjou");
}

void test_strstr(void)
{
	const char buf[] = "bonjouroui";
	ASSERT_EQ(strstr(buf, "our"), &buf[4]);
	ASSERT_EQ(strstr(buf, "uo"), NULL);
	ASSERT_EQ(strstr(buf, ""), buf);
}

void test_strnstr(void)
{
	const char buf[] = "bonjouroui";
	ASSERT_EQ(strnstr(buf, "our", 7), &buf[4]);
	ASSERT_EQ(strnstr(buf, "our", 6), NULL);
	ASSERT_EQ(strnstr(buf, "uo", sizeof(buf)), NULL);
	ASSERT_EQ(strnstr(buf, "", sizeof(buf)), buf);
}

void test_strcmp(void)
{
	ASSERT_EQ(strcmp("00", "01"), -1);
	ASSERT_EQ(strcmp("00", "00"), 0);
	ASSERT_EQ(strcmp("01", "00"), 1);
	ASSERT_EQ(strcmp("00", "0"), '0');
	ASSERT_EQ(strcmp("0", "00"), -'0');
}

void test_strncmp(void)
{
	ASSERT_EQ(strncmp("00", "01", 2), -1);
	ASSERT_EQ(strncmp("00", "01", 1), 0);
	ASSERT_EQ(strncmp("00", "00", 2), 0);
	ASSERT_EQ(strncmp("01", "00", 2), 1);
	ASSERT_EQ(strncmp("01", "00", 1), 0);
	ASSERT_EQ(strncmp("0", "1", 0), 0);
	ASSERT_EQ(strncmp("00", "0", 2), '0');
	ASSERT_EQ(strncmp("00", "0", 1), 0);
	ASSERT_EQ(strncmp("0", "00", 2), -'0');
	ASSERT_EQ(strncmp("0", "00", 1), 0);
}

void test_strpbrk(void)
{
	const char buf[] = "bonjour";

	ASSERT_EQ(strpbrk(buf, "jt"), &buf[3]);
	ASSERT_EQ(strpbrk(buf, ""), NULL);
	ASSERT_EQ(strpbrk(buf, "t"), NULL);
}

void test_strspn(void)
{
	ASSERT_EQ(strspn("bonjour", "o"), 0);
	ASSERT_EQ(strspn("bonjour", "ob"), 2);
	ASSERT_EQ(strspn("bonjour", "nobj"), 5);
	ASSERT_EQ(strspn("bonjour", ""), 0);
	ASSERT_EQ(strspn("bonjour", "y"), 0);
}

void test_strcspn(void)
{
	ASSERT_EQ(strcspn("bonjour", "y"), 7);
	ASSERT_EQ(strcspn("bonjour", "b"), 0);
	ASSERT_EQ(strcspn("bonjour", "o"), 1);
	ASSERT_EQ(strcspn("bonjour", "jo"), 1);
	ASSERT_EQ(strcspn("bonjour", ""), 7);
}
