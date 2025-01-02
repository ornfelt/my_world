#include "tests.h"

#include <wchar.h>
#include <stdio.h>

void test_wcslen(void)
{
	wchar_t buf[16];

	ASSERT_EQ(wmemset(buf, 0, 16), buf);
	ASSERT_EQ(wmemset(&buf[3], 0xAC, 9), &buf[3]);
	ASSERT_EQ(wcslen(&buf[0]), 0);
	ASSERT_EQ(wcslen(&buf[2]), 0);
	ASSERT_EQ(wcslen(&buf[3]), 9);
	ASSERT_EQ(wcslen(&buf[4]), 8);
	ASSERT_EQ(wcslen(&buf[5]), 7);
	ASSERT_EQ(wcslen(&buf[6]), 6);
	ASSERT_EQ(wcslen(&buf[7]), 5);
	ASSERT_EQ(wcslen(&buf[8]), 4);
	ASSERT_EQ(wcslen(&buf[9]), 3);
	ASSERT_EQ(wcslen(&buf[10]), 2);
	ASSERT_EQ(wcslen(&buf[11]), 1);
	ASSERT_EQ(wcslen(&buf[12]), 0);
	ASSERT_EQ(wcslen(&buf[13]), 0);
}

void test_wmemchr(void)
{
	wchar_t buf[16] = {0, 1, 3, 3, 4, 5, 6, 7, 8, 9, 11, 11, 12, 13, 14, 15};

	ASSERT_EQ(wmemchr(&buf[0] , 0 , 10), &buf[0]);
	ASSERT_EQ(wmemchr(&buf[1] , 11, 15), &buf[10]);
	ASSERT_EQ(wmemchr(&buf[2] , 11, 14), &buf[10]);
	ASSERT_EQ(wmemchr(&buf[3] , 11, 13), &buf[10]);
	ASSERT_EQ(wmemchr(&buf[4] , 11, 12), &buf[10]);
	ASSERT_EQ(wmemchr(&buf[5] , 11, 11), &buf[10]);
	ASSERT_EQ(wmemchr(&buf[6] , 11, 10), &buf[10]);
	ASSERT_EQ(wmemchr(&buf[7] , 11, 9 ), &buf[10]);
	ASSERT_EQ(wmemchr(&buf[8] , 11, 8 ), &buf[10]);
	ASSERT_EQ(wmemchr(&buf[9] , 11, 7 ), &buf[10]);
	ASSERT_EQ(wmemchr(&buf[10], 11, 6 ), &buf[10]);
	ASSERT_EQ(wmemchr(&buf[11], 11, 5 ), &buf[11]);
	ASSERT_EQ(wmemchr(&buf[12], 11, 4 ), NULL);
}

void test_wmemrchr(void)
{
	wchar_t buf[16] = {0, 1, 3, 3, 4, 5, 6, 7, 8, 9, 11, 11, 12, 13, 14, 15};

	ASSERT_EQ(wmemrchr(buf, 15, 16), &buf[15]);
	ASSERT_EQ(wmemrchr(buf, 3, 14), &buf[3]);
	ASSERT_EQ(wmemrchr(buf, 3, 13), &buf[3]);
	ASSERT_EQ(wmemrchr(buf, 3, 12), &buf[3]);
	ASSERT_EQ(wmemrchr(buf, 3, 11), &buf[3]);
	ASSERT_EQ(wmemrchr(buf, 3, 10), &buf[3]);
	ASSERT_EQ(wmemrchr(buf, 3, 9 ), &buf[3]);
	ASSERT_EQ(wmemrchr(buf, 3, 8 ), &buf[3]);
	ASSERT_EQ(wmemrchr(buf, 3, 7 ), &buf[3]);
	ASSERT_EQ(wmemrchr(buf, 3, 6 ), &buf[3]);
	ASSERT_EQ(wmemrchr(buf, 3, 5 ), &buf[3]);
	ASSERT_EQ(wmemrchr(buf, 3, 4 ), &buf[3]);
	ASSERT_EQ(wmemrchr(buf, 3, 3 ), &buf[2]);
	ASSERT_EQ(wmemrchr(buf, 3, 2 ), NULL);
}

void test_wmemmem(void)
{
	const wchar_t *hs = L"ouiouinonoui";

	ASSERT_EQ(wmemmem(hs, 12, L"nun", 3), NULL);
	ASSERT_EQ(wmemmem(hs, 12, L"non", 3), &hs[6]);
	ASSERT_EQ(wmemmem(hs, 12, hs, 12), hs);
	ASSERT_EQ(wmemmem(hs, 12, L"noui", 4), &hs[8]);
	ASSERT_EQ(wmemmem(hs, 12, L"nouo", 3), &hs[8]);
	ASSERT_EQ(wmemmem(hs, 11, L"noui", 4), NULL);
}

void test_wcsnlen(void)
{
	ASSERT_EQ(wcsnlen(L"ouioui", 1), 1);
	ASSERT_EQ(wcsnlen(L"ouioui", 3), 3);
	ASSERT_EQ(wcsnlen(L"ouioui", 4), 4);
	ASSERT_EQ(wcsnlen(L"ouioui", 5), 5);
	ASSERT_EQ(wcsnlen(L"ouioui", 6), 6);
	ASSERT_EQ(wcsnlen(L"ouioui", 7), 6);
}

void test_wcscpy(void)
{
	wchar_t dst[16];

	ASSERT_EQ(wmemset(dst, 0, 16), dst);
	ASSERT_EQ(wcscpy(dst, L"oui"), dst);
	ASSERT_WCS_EQ(dst, L"oui");

	ASSERT_EQ(wmemset(dst, 0, 16), dst);
	ASSERT_EQ(wcscpy(dst, L"0123456789"), dst);
	ASSERT_WCS_EQ(dst, L"0123456789");

	ASSERT_EQ(wmemset(dst, 0, 16), dst);
	ASSERT_EQ(wcscpy(dst, L"012345678"), dst);
	ASSERT_WCS_EQ(dst, L"012345678");

	ASSERT_EQ(wmemset(dst, 0, 16), dst);
	ASSERT_EQ(wcscpy(dst, L"01234567"), dst);
	ASSERT_WCS_EQ(dst, L"01234567");

	ASSERT_EQ(wmemset(dst, 0, 16), dst);
	ASSERT_EQ(wcscpy(dst, L"0123456"), dst);
	ASSERT_WCS_EQ(dst, L"0123456");
}

void test_wcsncpy(void)
{
	wchar_t dst[16];

	ASSERT_EQ(wmemset(dst, 0, 16), dst);
	ASSERT_EQ(wcsncpy(dst, L"0123456789", 0), dst);
	ASSERT_WCS_EQ(dst, L"");

	ASSERT_EQ(wmemset(dst, 0, 16), dst);
	ASSERT_EQ(wcsncpy(dst, L"0123456789", 6), dst);
	ASSERT_WCS_EQ(dst, L"012345");

	ASSERT_EQ(wmemset(dst, 0, 16), dst);
	ASSERT_EQ(wcsncpy(dst, L"0123456789", 7), dst);
	ASSERT_WCS_EQ(dst, L"0123456");

	ASSERT_EQ(wmemset(dst, 0, 16), dst);
	ASSERT_EQ(wcsncpy(dst, L"0123456789", 8), dst);
	ASSERT_WCS_EQ(dst, L"01234567");

	ASSERT_EQ(wmemset(dst, 0, 16), dst);
	ASSERT_EQ(wcsncpy(dst, L"0123456789", 9), dst);
	ASSERT_WCS_EQ(dst, L"012345678");

	ASSERT_EQ(wmemset(dst, 0, 16), dst);
	ASSERT_EQ(wcsncpy(dst, L"0123456789", 10), dst);
	ASSERT_WCS_EQ(dst, L"0123456789");

	ASSERT_EQ(wmemset(dst, 0, 16), dst);
	ASSERT_EQ(wcsncpy(dst, L"0123456789", 11), dst);
	ASSERT_WCS_EQ(dst, L"0123456789");
}

void test_wcslcpy(void)
{
	wchar_t dst[16];

	ASSERT_EQ(wcslcpy(dst, L"oui",  16), 3);
	ASSERT_WCS_EQ(dst, L"oui");

	ASSERT_EQ(wcslcpy(dst, L"012345678", 7), 9);
	ASSERT_WCS_EQ(dst, L"012345");

	ASSERT_EQ(wcslcpy(dst, L"012345678", 8), 9);
	ASSERT_WCS_EQ(dst, L"0123456");

	ASSERT_EQ(wcslcpy(dst, L"012345678", 9), 9);
	ASSERT_WCS_EQ(dst, L"01234567");

	ASSERT_EQ(wcslcpy(dst, L"012345678", 10), 9);
	ASSERT_WCS_EQ(dst, L"012345678");
}

void test_wcscat(void)
{
	wchar_t dst[16];

	wcscpy(dst, L"ou");
	wcscat(dst, L"testio");
	ASSERT_WCS_EQ(dst, L"outestio");
}

void test_wcsncat(void)
{
	wchar_t dst[16];

	ASSERT_EQ(wcscpy(dst, L"ou"), dst);
	ASSERT_EQ(wcsncat(dst, L"testiop", 6), dst);
	ASSERT_WCS_EQ(dst, L"outestio");
}

void test_wcslcat(void)
{
	wchar_t dst[16];

	ASSERT_EQ(wcscpy(dst, L"012345"), dst);
	ASSERT_EQ(wcslcat(dst, L"01234567", 16), 14);
	ASSERT_WCS_EQ(dst, L"01234501234567");

	ASSERT_EQ(wcscpy(dst, L"012345"), dst);
	ASSERT_EQ(wcslcat(dst, L"012345678", 16), 15);
	ASSERT_WCS_EQ(dst, L"012345012345678");

	ASSERT_EQ(wcscpy(dst, L"012345"), dst);
	ASSERT_EQ(wcslcat(dst, L"0123456789", 16), 16);
	ASSERT_WCS_EQ(dst, L"012345012345678");

	ASSERT_EQ(wcscpy(dst, L"012345"), dst);
	ASSERT_EQ(wcslcat(dst, L"01234567890", 16), 17);
	ASSERT_WCS_EQ(dst, L"012345012345678");
}

void test_wcschr(void)
{
	wchar_t *str = L"0123456789";
	ASSERT_EQ(wcschr(str, L'a'), NULL);
	ASSERT_EQ(wcschr(str, L'0'), &str[0]);
	ASSERT_EQ(wcschr(str, L'1'), &str[1]);
	ASSERT_EQ(wcschr(str, L'2'), &str[2]);
	ASSERT_EQ(wcschr(str, L'3'), &str[3]);
	ASSERT_EQ(wcschr(str, L'4'), &str[4]);
	ASSERT_EQ(wcschr(str, L'5'), &str[5]);
	ASSERT_EQ(wcschr(str, L'6'), &str[6]);
	ASSERT_EQ(wcschr(str, L'7'), &str[7]);
	ASSERT_EQ(wcschr(str, L'8'), &str[8]);
	ASSERT_EQ(wcschr(str, L'9'), &str[9]);
	ASSERT_EQ(wcschr(str,   0 ), &str[10]);
	ASSERT_EQ(wcschr(&str[3], L'3'), &str[3]);
}

void test_wcschrnul(void)
{
	wchar_t *str = L"0123456789";
	ASSERT_EQ(wcschrnul(str, L'a'), &str[10]);
	ASSERT_EQ(wcschrnul(str, L'0'), &str[0]);
	ASSERT_EQ(wcschrnul(str, L'1'), &str[1]);
	ASSERT_EQ(wcschrnul(str, L'2'), &str[2]);
	ASSERT_EQ(wcschrnul(str, L'3'), &str[3]);
	ASSERT_EQ(wcschrnul(str, L'4'), &str[4]);
	ASSERT_EQ(wcschrnul(str, L'5'), &str[5]);
	ASSERT_EQ(wcschrnul(str, L'6'), &str[6]);
	ASSERT_EQ(wcschrnul(str, L'7'), &str[7]);
	ASSERT_EQ(wcschrnul(str, L'8'), &str[8]);
	ASSERT_EQ(wcschrnul(str, L'9'), &str[9]);
	ASSERT_EQ(wcschrnul(str,   0 ), &str[10]);
	ASSERT_EQ(wcschrnul(&str[3], '3'), &str[3]);
}

void test_wcsrchr(void)
{
	wchar_t *str = L"0123456720";
	ASSERT_EQ(wcsrchr(str, L'a'), NULL);
	ASSERT_EQ(wcsrchr(str, L'0'), &str[9]);
	ASSERT_EQ(wcsrchr(str, L'1'), &str[1]);
	ASSERT_EQ(wcsrchr(str, L'2'), &str[8]);
	ASSERT_EQ(wcsrchr(str, L'3'), &str[3]);
	ASSERT_EQ(wcsrchr(str, L'4'), &str[4]);
	ASSERT_EQ(wcsrchr(str, L'5'), &str[5]);
	ASSERT_EQ(wcsrchr(str, L'6'), &str[6]);
	ASSERT_EQ(wcsrchr(str, L'7'), &str[7]);
	ASSERT_EQ(wcsrchr(str, L'8'), NULL);
	ASSERT_EQ(wcsrchr(str, L'9'), NULL);
	ASSERT_EQ(wcsrchr(str,   0 ), &str[10]);
	ASSERT_EQ(wcsrchr(&str[3], L'3'), &str[3]);
}

void test_wmemcmp(void)
{
	ASSERT_EQ(wmemcmp(L"1", L"1", 1), 0);
	ASSERT_EQ(wmemcmp(L"1", L"0", 1), 1);
	ASSERT_EQ(wmemcmp(L"9", L"0", 1), 9);
	ASSERT_EQ(wmemcmp(L"0", L"9", 1), -9);
	ASSERT_EQ(wmemcmp(L"0", L"9", 0), 0);
	ASSERT_EQ(wmemcmp(L"01", L"01", 3), 0);
	ASSERT_EQ(wmemcmp(L"01", L"02", 1), 0);
}

void test_wmemcpy(void)
{
	wchar_t buf[16];

	ASSERT_EQ(wmemcpy(buf, L"salut", 6), buf);
	ASSERT_WCS_EQ(buf, L"salut");
	ASSERT_EQ(wmemcpy(buf, L"bonjour", 2), buf);
	ASSERT_WCS_EQ(buf, L"bolut");
	ASSERT_EQ(wmemcpy(buf, L"ouais", 0), buf);
	ASSERT_WCS_EQ(buf, L"bolut");
}

void test_wmemmove(void)
{
	wchar_t buf[16];

	ASSERT_EQ(wmemmove(buf, L"salut", 6), buf);
	ASSERT_WCS_EQ(buf, L"salut");
	ASSERT_EQ(wmemmove(buf, &buf[1], 5), buf);
	ASSERT_WCS_EQ(buf, L"alut");
	ASSERT_EQ(wmemmove(&buf[1], buf, 5), &buf[1]);
	ASSERT_WCS_EQ(buf, L"aalut");
}

void test_wmemset(void)
{
	wchar_t buf[15] = {0};

	ASSERT_EQ(wmemset(buf, L'1', 2), buf);
	ASSERT_EQ(buf[0], L'1');
	ASSERT_EQ(buf[1], L'1');
	ASSERT_EQ(buf[2], 0);
	ASSERT_EQ(wmemset(&buf[1], L'2', 3), &buf[1]);
	ASSERT_EQ(buf[0], L'1');
	ASSERT_EQ(buf[1], L'2');
	ASSERT_EQ(buf[2], L'2');
	ASSERT_EQ(buf[3], L'2');
	ASSERT_EQ(buf[4], 0);
	ASSERT_EQ(wmemset(&buf[2], L'5', 0), &buf[2]);
	ASSERT_EQ(buf[2], L'2');
}

void test_wcpcpy(void)
{
	wchar_t buf[15] = {0};
	wchar_t *it;

	ASSERT_WCS_EQ(buf, L"");
	it = wcpcpy(buf, L"test");
	ASSERT_EQ(it, &buf[4]);
	ASSERT_WCS_EQ(buf, L"test");
	it = wcpcpy(it, L"oui");
	ASSERT_EQ(it, &buf[7]);
	ASSERT_WCS_EQ(buf, L"testoui");
	it = wcpcpy(it, L"");
	ASSERT_EQ(it, &buf[7]);
	ASSERT_WCS_EQ(buf, L"testoui");
}

void test_wcpncpy(void)
{
	wchar_t buf[15] = {0};
	wchar_t *it;

	buf[0] = L'\0';
	wmemset(&buf[1], 1, 14);
	ASSERT_WCS_EQ(buf, L"");
	it = wcpncpy(buf, L"test", 3);
	ASSERT_EQ(it, &buf[3]);
	buf[3] = '\0';
	ASSERT_WCS_EQ(buf, L"tes");
	it = wcpncpy(it, L"oui", 5);
	ASSERT_EQ(it, &buf[6]);
	ASSERT_EQ(it[0], '\0');
	ASSERT_EQ(it[1], '\0');
	ASSERT_WCS_EQ(buf, L"tesoui");
	it = wcpncpy(it, L"", 3);
	ASSERT_EQ(it, &buf[6]);
	ASSERT_EQ(it[0], L'\0');
	ASSERT_EQ(it[1], L'\0');
	ASSERT_EQ(it[2], L'\0');
	ASSERT_WCS_EQ(buf, L"tesoui");
}

void test_wmemccpy(void)
{
	wchar_t buf[15] = {0};

	ASSERT_EQ(wmemccpy(buf, L"test", L's', 10), &buf[3]);
	ASSERT_WCS_EQ(buf, L"tes");
	ASSERT_EQ(wmemccpy(buf, L"test", L'q', 5), NULL);
	ASSERT_WCS_EQ(buf, L"test");
	ASSERT_EQ(wmemccpy(buf, L"bonjour", L'\0', 6), NULL);
	ASSERT_WCS_EQ(buf, L"bonjou");
}

void test_wcsstr(void)
{
	const wchar_t buf[] = L"bonjouroui";
	ASSERT_EQ(wcsstr(buf, L"our"), &buf[4]);
	ASSERT_EQ(wcsstr(buf, L"uo"), NULL);
	ASSERT_EQ(wcsstr(buf, L""), buf);
}

void test_wcsnstr(void)
{
	const wchar_t buf[] = L"bonjouroui";
	ASSERT_EQ(wcsnstr(buf, L"our", 7), &buf[4]);
	ASSERT_EQ(wcsnstr(buf, L"our", 6), NULL);
	ASSERT_EQ(wcsnstr(buf, L"uo", sizeof(buf) / sizeof(*buf)), NULL);
	ASSERT_EQ(wcsnstr(buf, L"", sizeof(buf) / sizeof(*buf)), buf);
}

void test_wcscmp(void)
{
	ASSERT_EQ(wcscmp(L"00", L"01"), -1);
	ASSERT_EQ(wcscmp(L"00", L"00"), 0);
	ASSERT_EQ(wcscmp(L"01", L"00"), 1);
	ASSERT_EQ(wcscmp(L"00", L"0"), L'0');
	ASSERT_EQ(wcscmp(L"0", L"00"), -L'0');
}

void test_wcsncmp(void)
{
	ASSERT_EQ(wcsncmp(L"00", L"01", 2), -1);
	ASSERT_EQ(wcsncmp(L"00", L"01", 1), 0);
	ASSERT_EQ(wcsncmp(L"00", L"00", 2), 0);
	ASSERT_EQ(wcsncmp(L"01", L"00", 2), 1);
	ASSERT_EQ(wcsncmp(L"01", L"00", 1), 0);
	ASSERT_EQ(wcsncmp(L"0", L"1", 0), 0);
	ASSERT_EQ(wcsncmp(L"00", L"0", 2), L'0');
	ASSERT_EQ(wcsncmp(L"00", L"0", 1), 0);
	ASSERT_EQ(wcsncmp(L"0", L"00", 2), -L'0');
	ASSERT_EQ(wcsncmp(L"0", L"00", 1), 0);
}

void test_wcspbrk(void)
{
	const wchar_t buf[] = L"bonjour";

	ASSERT_EQ(wcspbrk(buf, L"jt"), &buf[3]);
	ASSERT_EQ(wcspbrk(buf, L""), NULL);
	ASSERT_EQ(wcspbrk(buf, L"t"), NULL);
}

void test_wcsspn(void)
{
	ASSERT_EQ(wcsspn(L"bonjour", L"o"), 0);
	ASSERT_EQ(wcsspn(L"bonjour", L"ob"), 2);
	ASSERT_EQ(wcsspn(L"bonjour", L"nobj"), 5);
	ASSERT_EQ(wcsspn(L"bonjour", L""), 0);
	ASSERT_EQ(wcsspn(L"bonjour", L"y"), 0);
}

void test_wcscspn(void)
{
	ASSERT_EQ(wcscspn(L"bonjour", L"y"), 7);
	ASSERT_EQ(wcscspn(L"bonjour", L"b"), 0);
	ASSERT_EQ(wcscspn(L"bonjour", L"o"), 1);
	ASSERT_EQ(wcscspn(L"bonjour", L"jo"), 1);
	ASSERT_EQ(wcscspn(L"bonjour", L""), 7);
}
