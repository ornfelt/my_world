#include <std.h>

int atoi(const char *s)
{
	char is_neg = 0;
	size_t i = 0;
	while (s[i] && isspace(s[i]))
		i++;
	if (s[i] == '-')
		is_neg = 1;
	if (s[i] == '+' || s[i] == '-')
		i++;
	int result = 0;
	while (s[i] == '0')
		i++;
	while (s[i])
	{
		if (s[i] >= '0' && s[i++] <= '9')
			result = result * 10 + s[i - 1] - '0';
		else
			return is_neg ? -result : result;
	}
	return is_neg ? -result : result;
}

int atoin(const char *s, size_t n)
{
	char is_neg = 0;
	size_t i = 0;
	while (s[i] && i < n && isspace(s[i]))
		i++;
	if (i >= n)
		return 0;
	if (s[i] == '-')
		is_neg = 1;
	if (s[i] == '+' || s[i] == '-')
		i++;
	if (i >= n)
		return 0;
	while (s[i] == '0' && i < n)
		i++;
	if (i >= n)
		return 0;
	int result = 0;
	while (s[i] && i < n)
	{
		if (s[i] >= '0' && s[i++] <= '9')
			result = result * 10 + s[i - 1] - '0';
		else
			return is_neg ? -result : result;
	}
	return is_neg ? -result : result;
}
