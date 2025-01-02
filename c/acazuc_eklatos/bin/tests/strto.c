#include "tests.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

void test_strtol(void)
{
	ASSERT_EQ(strtol("", NULL, 0), 0);
	ASSERT_EQ(strtol("11", NULL, 0), 11);
	ASSERT_EQ(strtol("+11", NULL, 0),  11);
	ASSERT_EQ(strtol("-11", NULL, 0), -11);
	ASSERT_EQ(strtol("011", NULL, 0), 9);
	ASSERT_EQ(strtol("+011", NULL, 0),  9);
	ASSERT_EQ(strtol("-011", NULL, 0), -9);
	ASSERT_EQ(strtol("0x11", NULL, 0), 17);
	ASSERT_EQ(strtol("+0x11", NULL, 0),  17);
	ASSERT_EQ(strtol("-0x11", NULL, 0), -17);
	ASSERT_EQ(strtol(" \t\r\n-0x0011", NULL, 0), -17);

	ASSERT_EQ(strtol("", NULL, 10), 0);
	ASSERT_EQ(strtol("1", NULL, 10), 1);
	ASSERT_EQ(strtol("9", NULL, 10), 9);
	ASSERT_EQ(strtol("10", NULL, 10), 10);
	ASSERT_EQ(strtol("11", NULL, 10), 11);

#if __SIZEOF_LONG__ == 8
	errno = 0;
	ASSERT_EQ(strtol("-9223372036854775807", NULL, 10), INT64_MIN + 1);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtol("-9223372036854775808", NULL, 10), INT64_MIN);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtol("-9223372036854775809", NULL, 10), INT64_MIN);
	ASSERT_EQ(errno, ERANGE);

	errno = 0;
	ASSERT_EQ(strtol("9223372036854775806", NULL, 10), INT64_MAX - 1);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtol("9223372036854775807", NULL, 10), INT64_MAX);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtol("9223372036854775808", NULL, 10), INT64_MAX);
	ASSERT_EQ(errno, ERANGE);

#else
	errno = 0;
	ASSERT_EQ(strtol("-2147483647", NULL, 10), INT32_MIN + 1);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtol("-2147483648", NULL, 10), INT32_MIN);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtol("-2147483649", NULL, 10), INT32_MIN);
	ASSERT_EQ(errno, ERANGE);

	errno = 0;
	ASSERT_EQ(strtol("2147483646", NULL, 10), INT32_MAX - 1);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtol("2147483647", NULL, 10), INT32_MAX);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtol("2147483648", NULL, 10), INT32_MAX);
	ASSERT_EQ(errno, ERANGE);
#endif

	ASSERT_EQ(strtol("214a", NULL, 10), 214);
	ASSERT_EQ(strtol("", NULL, 8), 0);
	ASSERT_EQ(strtol("1", NULL, 8), 1);
	ASSERT_EQ(strtol("7", NULL, 8), 7);
	ASSERT_EQ(strtol("8", NULL, 8), 0);
	ASSERT_EQ(strtol("10", NULL, 8), 8);
	ASSERT_EQ(strtol("11", NULL, 8), 9);
	ASSERT_EQ(strtol("18", NULL, 8), 1);

#if __SIZEOF_LONG__ == 8
	errno = 0;
	ASSERT_EQ(strtol("-777777777777777777777", NULL, 8), INT64_MIN + 1);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtol("-1000000000000000000000", NULL, 8), INT64_MIN);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtol("-1000000000000000000001", NULL, 8), INT64_MIN);
	ASSERT_EQ(errno, ERANGE);

	errno = 0;
	ASSERT_EQ(strtol("777777777777777777776", NULL, 8), INT64_MAX - 1);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtol("777777777777777777777", NULL, 8), INT64_MAX);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtol("1000000000000000000000", NULL, 8), INT64_MAX);
	ASSERT_EQ(errno, ERANGE);
#else
	errno = 0;
	ASSERT_EQ(strtol("-17777777777", NULL, 8), INT32_MIN + 1);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtol("-20000000000", NULL, 8), INT32_MIN);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtol("-20000000001", NULL, 8), INT32_MIN);
	ASSERT_EQ(errno, ERANGE);

	errno = 0;
	ASSERT_EQ(strtol("17777777776", NULL, 8), INT32_MAX - 1);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtol("17777777777", NULL, 8), INT32_MAX);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtol("20000000000", NULL, 8), INT32_MAX);
	ASSERT_EQ(errno, ERANGE);
#endif

	ASSERT_EQ(strtol("", NULL, 16), 0);
	ASSERT_EQ(strtol("1", NULL, 16), 1);
	ASSERT_EQ(strtol("f", NULL, 16), 15);
	ASSERT_EQ(strtol("F", NULL, 16), 15);
	ASSERT_EQ(strtol("fg", NULL, 16), 15);
	ASSERT_EQ(strtol("10", NULL, 16), 16);
	ASSERT_EQ(strtol("11", NULL, 16), 17);

#if __SIZEOF_LONG__ == 8
	errno = 0;
	ASSERT_EQ(strtol("-7FFFFFFFFFFFFFFF", NULL, 16), INT64_MIN + 1);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtol("-8000000000000000", NULL, 16), INT64_MIN);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtol("-8000000000000001", NULL, 16), INT64_MIN);
	ASSERT_EQ(errno, ERANGE);

	errno = 0;
	ASSERT_EQ(strtol("7FFFFFFFFFFFFFFE", NULL, 16), INT64_MAX - 1);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtol("7FFFFFFFFFFFFFFF", NULL, 16), INT64_MAX);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtol("8000000000000000", NULL, 16), INT64_MAX);
	ASSERT_EQ(errno, ERANGE);
#else
	errno = 0;
	ASSERT_EQ(strtol("-7FFFFFFF", NULL, 16), INT32_MIN + 1);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtol("-80000000", NULL, 16), INT32_MIN);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtol("-80000001", NULL, 16), INT32_MIN);
	ASSERT_EQ(errno, ERANGE);

	errno = 0;
	ASSERT_EQ(strtol("7FFFFFFE", NULL, 16), INT32_MAX - 1);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtol("7FFFFFFF", NULL, 16), INT32_MAX);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtol("80000000", NULL, 16), INT32_MAX);
	ASSERT_EQ(errno, ERANGE);
#endif
}

void test_strtoul(void)
{
	ASSERT_EQ(strtoul("", NULL, 0), 0);
	ASSERT_EQ(strtoul("11", NULL, 0), 11);
	ASSERT_EQ(strtoul("+11", NULL, 0), +11);
	ASSERT_EQ(strtoul("-11", NULL, 0), (unsigned long)-11L);
	ASSERT_EQ(strtoul("011", NULL, 0), 9);
	ASSERT_EQ(strtoul("+011", NULL, 0), 9);
	ASSERT_EQ(strtoul("-011", NULL, 0), (unsigned long)-011L);
	ASSERT_EQ(strtoul("0x11", NULL, 0), 17);
	ASSERT_EQ(strtoul("+0x11", NULL, 0), 17);
	ASSERT_EQ(strtoul("-0x11", NULL, 0), (unsigned long)-0x11L);
	ASSERT_EQ(strtoul(" \t\r\n-0x0011", NULL, 0), (unsigned long)-0x11L);

	ASSERT_EQ(strtoul("", NULL, 10), 0);
	ASSERT_EQ(strtoul("1", NULL, 10), 1);
	ASSERT_EQ(strtoul("9", NULL, 10), 9);
	ASSERT_EQ(strtoul("10", NULL, 10), 10);
	ASSERT_EQ(strtoul("11", NULL, 10), 11);

#if __SIZEOF_LONG__ == 8
	errno = 0;
	ASSERT_EQ(strtoul("18446744073709551614", NULL, 10), UINT64_MAX - 1);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtoul("18446744073709551615", NULL, 10), UINT64_MAX);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtoul("18446744073709551616", NULL, 10), UINT64_MAX);
	ASSERT_EQ(errno, ERANGE);
#else
	errno = 0;
	ASSERT_EQ(strtoul("4294967294", NULL, 10), UINT32_MAX - 1);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtoul("4294967295", NULL, 10), UINT32_MAX);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtoul("4294967296", NULL, 10), UINT32_MAX);
	ASSERT_EQ(errno, ERANGE);
#endif

	ASSERT_EQ(strtoul("", NULL, 8), 0);
	ASSERT_EQ(strtoul("1", NULL, 8), 1);
	ASSERT_EQ(strtoul("7", NULL, 8), 7);
	ASSERT_EQ(strtoul("8", NULL, 8), 0);
	ASSERT_EQ(strtoul("10", NULL, 8), 8);
	ASSERT_EQ(strtoul("11", NULL, 8), 9);
	ASSERT_EQ(strtoul("18", NULL, 8), 1);

#if __SIZEOF_LONG__ == 8
	errno = 0;
	ASSERT_EQ(strtoul("1777777777777777777776", NULL, 8), UINT64_MAX - 1);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtoul("1777777777777777777777", NULL, 8), UINT64_MAX);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtoul("2000000000000000000000", NULL, 8), UINT64_MAX);
	ASSERT_EQ(errno, ERANGE);
#else
	errno = 0;
	ASSERT_EQ(strtoul("37777777776", NULL, 8), UINT32_MAX - 1);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtoul("37777777777", NULL, 8), UINT32_MAX);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtoul("40000000000", NULL, 8), UINT32_MAX);
	ASSERT_EQ(errno, ERANGE);
#endif

	ASSERT_EQ(strtoul("", NULL, 16), 0);
	ASSERT_EQ(strtoul("1", NULL, 16), 1);
	ASSERT_EQ(strtoul("f", NULL, 16), 15);
	ASSERT_EQ(strtoul("F", NULL, 16), 15);
	ASSERT_EQ(strtoul("fg", NULL, 16), 15);
	ASSERT_EQ(strtoul("10", NULL, 16), 16);
	ASSERT_EQ(strtoul("11", NULL, 16), 17);

#if __SIZEOF_LONG__ == 8
	errno = 0;
	ASSERT_EQ(strtoul("FFFFFFFFFFFFFFFE", NULL, 16), UINT64_MAX - 1);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtoul("FFFFFFFFFFFFFFFF", NULL, 16), UINT64_MAX);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtoul("10000000000000000", NULL, 16), UINT64_MAX);
	ASSERT_EQ(errno, ERANGE);
#else
	errno = 0;
	ASSERT_EQ(strtoul("FFFFFFFE", NULL, 16), UINT32_MAX - 1);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtoul("FFFFFFFF", NULL, 16), UINT32_MAX);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtoul("100000000", NULL, 16), UINT32_MAX);
	ASSERT_EQ(errno, ERANGE);
#endif
}

void test_strtoll(void)
{
	ASSERT_EQ(strtoll("", NULL, 0), 0);
	ASSERT_EQ(strtoll("11", NULL, 0), 11);
	ASSERT_EQ(strtoll("+11", NULL, 0), +11);
	ASSERT_EQ(strtoll("-11", NULL, 0), -11);
	ASSERT_EQ(strtoll("011", NULL, 0), 9);
	ASSERT_EQ(strtoll("+011", NULL, 0), 9);
	ASSERT_EQ(strtoll("-011", NULL, 0), -9);
	ASSERT_EQ(strtoll("0x11", NULL, 0), 17);
	ASSERT_EQ(strtoll("+0x11", NULL, 0), 17);
	ASSERT_EQ(strtoll("-0x11", NULL, 0), -17);
	ASSERT_EQ(strtoll(" \t\r\n-0x0011", NULL, 0), -17);

	ASSERT_EQ(strtoll("", NULL, 10), 0);
	ASSERT_EQ(strtoll("1", NULL, 10), 1);
	ASSERT_EQ(strtoll("9", NULL, 10), 9);
	ASSERT_EQ(strtoll("10", NULL, 10), 10);
	ASSERT_EQ(strtoll("11", NULL, 10), 11);

	errno = 0;
	ASSERT_EQ(strtoll("-9223372036854775807", NULL, 10), INT64_MIN + 1);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtoll("-9223372036854775808", NULL, 10), INT64_MIN);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtoll("-9223372036854775809", NULL, 10), INT64_MIN);
	ASSERT_EQ(errno, ERANGE);

	errno = 0;
	ASSERT_EQ(strtoll("9223372036854775806", NULL, 10), INT64_MAX - 1);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtoll("9223372036854775807", NULL, 10), INT64_MAX);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtoll("9223372036854775808", NULL, 10), INT64_MAX);
	ASSERT_EQ(errno, ERANGE);

	ASSERT_EQ(strtoll("214a", NULL, 10), 214);
	ASSERT_EQ(strtoll("", NULL, 8), 0);
	ASSERT_EQ(strtoll("1", NULL, 8), 1);
	ASSERT_EQ(strtoll("7", NULL, 8), 7);
	ASSERT_EQ(strtoll("8", NULL, 8), 0);
	ASSERT_EQ(strtoll("10", NULL, 8), 8);
	ASSERT_EQ(strtoll("11", NULL, 8), 9);
	ASSERT_EQ(strtoll("18", NULL, 8), 1);

	errno = 0;
	ASSERT_EQ(strtoll("-777777777777777777777", NULL, 8), INT64_MIN + 1);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtoll("-1000000000000000000000", NULL, 8), INT64_MIN);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtoll("-1000000000000000000001", NULL, 8), INT64_MIN);
	ASSERT_EQ(errno, ERANGE);

	errno = 0;
	ASSERT_EQ(strtoll("777777777777777777776", NULL, 8), INT64_MAX - 1);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtoll("777777777777777777777", NULL, 8), INT64_MAX);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtoll("1000000000000000000000", NULL, 8), INT64_MAX);
	ASSERT_EQ(errno, ERANGE);

	ASSERT_EQ(strtoll("", NULL, 16), 0);
	ASSERT_EQ(strtoll("1", NULL, 16), 1);
	ASSERT_EQ(strtoll("f", NULL, 16), 15);
	ASSERT_EQ(strtoll("F", NULL, 16), 15);
	ASSERT_EQ(strtoll("fg", NULL, 16), 15);
	ASSERT_EQ(strtoll("10", NULL, 16), 16);
	ASSERT_EQ(strtoll("11", NULL, 16), 17);

	errno = 0;
	ASSERT_EQ(strtoll("-7FFFFFFFFFFFFFFF", NULL, 16), INT64_MIN + 1);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtoll("-8000000000000000", NULL, 16), INT64_MIN);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtoll("-8000000000000001", NULL, 16), INT64_MIN);
	ASSERT_EQ(errno, ERANGE);

	errno = 0;
	ASSERT_EQ(strtoll("7FFFFFFFFFFFFFFE", NULL, 16), INT64_MAX - 1);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtoll("7FFFFFFFFFFFFFFF", NULL, 16), INT64_MAX);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtoll("8000000000000000", NULL, 16), INT64_MAX);
	ASSERT_EQ(errno, ERANGE);
}

void test_strtoull(void)
{
	ASSERT_EQ(strtoull("", NULL, 0), 0);
	ASSERT_EQ(strtoull("11", NULL, 0), 11);
	ASSERT_EQ(strtoull("+11", NULL, 0), +11);
	ASSERT_EQ(strtoull("-11", NULL, 0), (unsigned long long)-11L);
	ASSERT_EQ(strtoull("011", NULL, 0), 9);
	ASSERT_EQ(strtoull("+011", NULL, 0), 9);
	ASSERT_EQ(strtoull("-011", NULL, 0), (unsigned long long)-011L);
	ASSERT_EQ(strtoull("0x11", NULL, 0), 17);
	ASSERT_EQ(strtoull("+0x11", NULL, 0), 17);
	ASSERT_EQ(strtoull("-0x11", NULL, 0), (unsigned long long)-0x11L);
	ASSERT_EQ(strtoull(" \t\r\n-0x0011", NULL, 0), (unsigned long long)-0x11L);

	ASSERT_EQ(strtoull("", NULL, 10), 0);
	ASSERT_EQ(strtoull("1", NULL, 10), 1);
	ASSERT_EQ(strtoull("9", NULL, 10), 9);
	ASSERT_EQ(strtoull("10", NULL, 10), 10);
	ASSERT_EQ(strtoull("11", NULL, 10), 11);

	errno = 0;
	ASSERT_EQ(strtoull("18446744073709551614", NULL, 10), UINT64_MAX - 1);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtoull("18446744073709551615", NULL, 10), UINT64_MAX);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtoull("18446744073709551616", NULL, 10), UINT64_MAX);
	ASSERT_EQ(errno, ERANGE);

	ASSERT_EQ(strtoull("", NULL, 8), 0);
	ASSERT_EQ(strtoull("1", NULL, 8), 1);
	ASSERT_EQ(strtoull("7", NULL, 8), 7);
	ASSERT_EQ(strtoull("8", NULL, 8), 0);
	ASSERT_EQ(strtoull("10", NULL, 8), 8);
	ASSERT_EQ(strtoull("11", NULL, 8), 9);
	ASSERT_EQ(strtoull("18", NULL, 8), 1);

	errno = 0;
	ASSERT_EQ(strtoull("1777777777777777777776", NULL, 8), UINT64_MAX - 1);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtoull("1777777777777777777777", NULL, 8), UINT64_MAX);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtoull("2000000000000000000000", NULL, 8), UINT64_MAX);
	ASSERT_EQ(errno, ERANGE);

	ASSERT_EQ(strtoull("", NULL, 16), 0);
	ASSERT_EQ(strtoull("1", NULL, 16), 1);
	ASSERT_EQ(strtoull("f", NULL, 16), 15);
	ASSERT_EQ(strtoull("F", NULL, 16), 15);
	ASSERT_EQ(strtoull("fg", NULL, 16), 15);
	ASSERT_EQ(strtoull("10", NULL, 16), 16);
	ASSERT_EQ(strtoull("11", NULL, 16), 17);

	errno = 0;
	ASSERT_EQ(strtoull("FFFFFFFFFFFFFFFE", NULL, 16), UINT64_MAX - 1);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtoull("FFFFFFFFFFFFFFFF", NULL, 16), UINT64_MAX);
	ASSERT_EQ(errno, 0);

	errno = 0;
	ASSERT_EQ(strtoull("10000000000000000", NULL, 16), UINT64_MAX);
	ASSERT_EQ(errno, ERANGE);
}
