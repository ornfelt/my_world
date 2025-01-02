#include "tests.h"

#include <stdio.h>
#include <ctype.h>

void test_isalnum(void)
{
	ASSERT_EQ(isalnum('/'), 0);
	ASSERT_EQ(isalnum('0'), 1);
	ASSERT_EQ(isalnum('9'), 1);
	ASSERT_EQ(isalnum(':'), 0);
	ASSERT_EQ(isalnum('@'), 0);
	ASSERT_EQ(isalnum('A'), 1);
	ASSERT_EQ(isalnum('Z'), 1);
	ASSERT_EQ(isalnum('['), 0);
	ASSERT_EQ(isalnum('`'), 0);
	ASSERT_EQ(isalnum('a'), 1);
	ASSERT_EQ(isalnum('z'), 1);
	ASSERT_EQ(isalnum('{'), 0);
}

void test_isalpha(void)
{
	ASSERT_EQ(isalpha('/'), 0);
	ASSERT_EQ(isalpha('0'), 0);
	ASSERT_EQ(isalpha('9'), 0);
	ASSERT_EQ(isalpha(':'), 0);
	ASSERT_EQ(isalpha('@'), 0);
	ASSERT_EQ(isalpha('A'), 1);
	ASSERT_EQ(isalpha('Z'), 1);
	ASSERT_EQ(isalpha('['), 0);
	ASSERT_EQ(isalpha('`'), 0);
	ASSERT_EQ(isalpha('a'), 1);
	ASSERT_EQ(isalpha('z'), 1);
	ASSERT_EQ(isalpha('{'), 0);
}

void test_isascii(void)
{
	ASSERT_EQ(isascii(-1), 0);
	ASSERT_EQ(isascii('\0'), 1);
	ASSERT_EQ(isascii('0'), 1);
	ASSERT_EQ(isascii(127), 1);
	ASSERT_EQ(isascii(128), 0);
	ASSERT_EQ(isascii(255), 0);
	ASSERT_EQ(isascii(1024), 0);
}

void test_isblank(void)
{
	ASSERT_EQ(isblank(' '), 1);
	ASSERT_EQ(isblank('\t'), 1);
	ASSERT_EQ(isblank('\b'), 0);
	ASSERT_EQ(isblank('0'), 0);
	ASSERT_EQ(isblank('a'), 0);
	ASSERT_EQ(isblank('\0'), 0);
}

void test_iscntrl(void)
{
	ASSERT_EQ(iscntrl('\b'), 1);
	ASSERT_EQ(iscntrl('\x1F'), 1);
	ASSERT_EQ(iscntrl(' '), 0);
	ASSERT_EQ(iscntrl('a'), 0);
	ASSERT_EQ(iscntrl('~'), 0);
	ASSERT_EQ(iscntrl('\x7F'), 1);
}

void test_isdigit(void)
{
	ASSERT_EQ(isdigit('/'), 0);
	ASSERT_EQ(isdigit('0'), 1);
	ASSERT_EQ(isdigit('9'), 1);
	ASSERT_EQ(isdigit(':'), 0);
	ASSERT_EQ(isdigit('@'), 0);
	ASSERT_EQ(isdigit('A'), 0);
	ASSERT_EQ(isdigit('Z'), 0);
	ASSERT_EQ(isdigit('['), 0);
	ASSERT_EQ(isdigit('`'), 0);
	ASSERT_EQ(isdigit('a'), 0);
	ASSERT_EQ(isdigit('z'), 0);
	ASSERT_EQ(isdigit('{'), 0);
}

void test_isgraph(void)
{
	ASSERT_EQ(isgraph('\x1F'), 0);
	ASSERT_EQ(isgraph(' '), 0);
	ASSERT_EQ(isgraph('a'), 1);
	ASSERT_EQ(isgraph('~'), 1);
	ASSERT_EQ(isgraph('\x7F'), 0);
}

void test_islower(void)
{
	ASSERT_EQ(islower('/'), 0);
	ASSERT_EQ(islower('0'), 0);
	ASSERT_EQ(islower('9'), 0);
	ASSERT_EQ(islower(':'), 0);
	ASSERT_EQ(islower('@'), 0);
	ASSERT_EQ(islower('A'), 0);
	ASSERT_EQ(islower('Z'), 0);
	ASSERT_EQ(islower('['), 0);
	ASSERT_EQ(islower('`'), 0);
	ASSERT_EQ(islower('a'), 1);
	ASSERT_EQ(islower('z'), 1);
	ASSERT_EQ(islower('{'), 0);
}

void test_isprint(void)
{
	ASSERT_EQ(isprint('\x1F'), 0);
	ASSERT_EQ(isprint(' '), 1);
	ASSERT_EQ(isprint('a'), 1);
	ASSERT_EQ(isprint('~'), 1);
	ASSERT_EQ(isprint('\x7F'), 0);
}

void test_ispunct(void)
{
	ASSERT_EQ(ispunct(' '), 0);
	ASSERT_EQ(ispunct('!'), 1);
	ASSERT_EQ(ispunct('/'), 1);
	ASSERT_EQ(ispunct('0'), 0);
	ASSERT_EQ(ispunct('9'), 0);
	ASSERT_EQ(ispunct(':'), 1);
	ASSERT_EQ(ispunct('@'), 1);
	ASSERT_EQ(ispunct('A'), 0);
	ASSERT_EQ(ispunct('Z'), 0);
	ASSERT_EQ(ispunct('['), 1);
	ASSERT_EQ(ispunct('`'), 1);
	ASSERT_EQ(ispunct('a'), 0);
	ASSERT_EQ(ispunct('z'), 0);
	ASSERT_EQ(ispunct('{'), 1);
	ASSERT_EQ(ispunct('~'), 1);
	ASSERT_EQ(ispunct('\x7F'), 0);
}

void test_isspace(void)
{
	ASSERT_EQ(isspace(' '), 1);
	ASSERT_EQ(isspace('\t'), 1);
	ASSERT_EQ(isspace('\n'), 1);
	ASSERT_EQ(isspace('\v'), 1);
	ASSERT_EQ(isspace('\f'), 1);
	ASSERT_EQ(isspace('\r'), 1);
	ASSERT_EQ(isspace('a'), 0);
	ASSERT_EQ(isspace('A'), 0);
	ASSERT_EQ(isspace('0'), 0);
	ASSERT_EQ(isspace('@'), 0);
}

void test_isupper(void)
{
	ASSERT_EQ(isupper('/'), 0);
	ASSERT_EQ(isupper('0'), 0);
	ASSERT_EQ(isupper('9'), 0);
	ASSERT_EQ(isupper(':'), 0);
	ASSERT_EQ(isupper('@'), 0);
	ASSERT_EQ(isupper('A'), 1);
	ASSERT_EQ(isupper('Z'), 1);
	ASSERT_EQ(isupper('['), 0);
	ASSERT_EQ(isupper('`'), 0);
	ASSERT_EQ(isupper('a'), 0);
	ASSERT_EQ(isupper('z'), 0);
	ASSERT_EQ(isupper('{'), 0);
}

void test_isxdigit(void)
{
	ASSERT_EQ(isxdigit('/'), 0);
	ASSERT_EQ(isxdigit('0'), 1);
	ASSERT_EQ(isxdigit('9'), 1);
	ASSERT_EQ(isxdigit(':'), 0);
	ASSERT_EQ(isxdigit('@'), 0);
	ASSERT_EQ(isxdigit('A'), 1);
	ASSERT_EQ(isxdigit('F'), 1);
	ASSERT_EQ(isxdigit('G'), 0);
	ASSERT_EQ(isxdigit('['), 0);
	ASSERT_EQ(isxdigit('`'), 0);
	ASSERT_EQ(isxdigit('a'), 1);
	ASSERT_EQ(isxdigit('f'), 1);
	ASSERT_EQ(isxdigit('g'), 0);
	ASSERT_EQ(isxdigit('{'), 0);
}

void test_tolower(void)
{
	ASSERT_EQ(tolower('0'), '0');
	ASSERT_EQ(tolower('@'), '@');
	ASSERT_EQ(tolower('A'), 'a');
	ASSERT_EQ(tolower('Z'), 'z');
	ASSERT_EQ(tolower('['), '[');
	ASSERT_EQ(tolower('a'), 'a');
	ASSERT_EQ(tolower('z'), 'z');
}

void test_toupper(void)
{
	ASSERT_EQ(toupper('0'), '0');
	ASSERT_EQ(toupper('@'), '@');
	ASSERT_EQ(toupper('A'), 'A');
	ASSERT_EQ(toupper('Z'), 'Z');
	ASSERT_EQ(toupper('['), '[');
	ASSERT_EQ(toupper('a'), 'A');
	ASSERT_EQ(toupper('z'), 'Z');
}
