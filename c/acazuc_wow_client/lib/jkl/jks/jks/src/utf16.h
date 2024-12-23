#ifndef JKS_UTF16_H
#define JKS_UTF16_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#define UTF16_NEXT(s) (*(str++))

static inline uint32_t utf16_codepoint_length(const char *str)
{
	if ((str[0] & 0xFC) == 0xD8)
		return 2;
	return 1;
}

static inline void utf16_encode1(char *str, uint32_t cp)
{
	UTF16_NEXT(str) = cp >> 0x8;
	UTF16_NEXT(str) = cp >> 0x0;
}

static inline uint32_t utf16_decode1(const char *str)
{
	uint32_t cp;
	cp  = (uint32_t)(UTF16_NEXT(str)) << 0x8;
	cp |= (uint32_t)(UTF16_NEXT(str)) << 0x0;
	return cp;
}

static inline bool utf16_check1(const char *str)
{
	(void)str;
	return true;
}

static inline void utf16_encode2(char *str, uint32_t cp)
{
	UTF16_NEXT(str) = 0xD8 | ((cp >> 0x12) & 0x03);
	UTF16_NEXT(str) = 0x00 | ((cp >> 0x0A) & 0xFF);
	UTF16_NEXT(str) = 0xDC | ((cp >> 0x08) & 0x03);
	UTF16_NEXT(str) = 0x00 | ((cp >> 0x00) & 0xFF);
}

static inline uint32_t utf16_decode2(const char *str)
{
	uint32_t cp;
	cp  = (uint32_t)(UTF16_NEXT(str) & 0x03) << 0x12;
	cp |= (uint32_t)(UTF16_NEXT(str) & 0xFF) << 0x0A;
	cp |= (uint32_t)(UTF16_NEXT(str) & 0x03) << 0x08;
	cp |= (uint32_t)(UTF16_NEXT(str) & 0xFF) << 0x00;
	return cp;
}

static inline bool utf16_check2(const char *str)
{
	if ((str[0] & 0xFC) != 0xD8)
		return false;
	if ((str[2] & 0xFC) != 0xDC)
		return false;
	return true;
}

static inline bool utf16_encode(char **str, uint32_t cp)
{
	if (cp < 0x10000)
	{
		utf16_encode1(*str, cp);
		*str += 2;
		return true;
	}
	if (cp < 0x100000)
	{
		utf16_encode2(*str, cp);
		*str += 4;
		return true;
	}
	return false;
}

static inline bool utf16_decode(const char **str, uint32_t *cp)
{
	switch (utf16_codepoint_length(*str))
	{
		case 1:
			*cp = utf16_decode1(*str);
			*str += 2;
			return true;
		case 2:
			*cp = utf16_decode2(*str);
			*str += 4;
			return true;
		default:
			return false;
	}
	return false;
}

static inline bool utf16_check(const char **str, const char *end)
{
	switch (utf16_codepoint_length(*str))
	{
		case 1:
			if (end - *str < 2)
				return false;
			return utf16_check1(*str);
		case 2:
			if (end - *str < 4)
				return false;
			return utf16_check2(*str);
		default:
			return false;
	}
	return false;
}

static inline bool utf16_next(const char **str, const char *end, uint32_t *cp)
{
	switch (utf16_codepoint_length(*str))
	{
		case 1:
			if (end - *str < 2)
				return false;
			if (!utf16_check1(*str))
				return false;
			*cp = utf16_decode1(*str);
			*str += 2;
			return true;
		case 2:
			if (end - *str < 4)
				return false;
			if (!utf16_check2(*str))
				return false;
			*cp = utf16_decode2(*str);
			*str += 4;
			return true;
		default:
			return false;
	}
	return false;
}

static inline bool utf16_peek_next(const char *str, const char *end, uint32_t *cp)
{
	return utf16_next(&str, end, cp);
}

static inline bool utf16_prev(const char **str, const char *begin, uint32_t *cp)
{
	const char *org = *str;
	for (size_t i = 0; i < 2; ++i)
	{
		if (*str - begin < 2)
			return false;
		*str -= 2;
		switch (utf16_codepoint_length(*str))
		{
			case 1:
				if (org - *str != 2)
					return false;
				if (!utf16_check1(*str))
					return false;
				*cp = utf16_decode1(*str);
				return true;
			case 2:
				if (org - *str != 4)
					return false;
				if (!utf16_check2(*str))
					return false;
				*cp = utf16_decode2(*str);
				return true;
		}
	}
	return false;
}

static inline bool utf16_peek_prev(const char *str, const char *begin, uint32_t *cp)
{
	return utf16_prev(&str, begin, cp);
}

static inline bool utf16_advance_step(const char **str, const char *end)
{
	switch (utf16_codepoint_length(*str))
	{
		case 1:
			if (end - *str < 2)
				return false;
			if (!utf16_check1(*str))
				return false;
			*str += 2;
			return true;
		case 2:
			if (end - *str < 4)
				return false;
			if (!utf16_check2(*str))
				return false;
			*str += 4;
			return true;
		default:
			return false;
	}
	return false;
}

static inline bool utf16_advance(const char **str, const char *end, size_t len)
{
	for (size_t i = 0; i < len; ++i)
	{
		if (!utf16_advance_step(str, end))
			return false;
	}
	return true;
}

static inline bool utf16_distance(const char *str, const char *end, size_t *distance)
{
	*distance = 0;
	while (str != end)
	{
		if (!utf16_advance_step(&str, end))
			return false;
		(*distance)++;
	}
	return true;
}

static inline const char *utf16_find_invalid(const char *str, const char *end)
{
	while (str != end)
	{
		if (!utf16_advance_step(&str, end))
			return str;
	}
	return str;
}

static inline bool utf16_is_valid(const char *str, const char *end)
{
	return utf16_find_invalid(str, end) == end;
}

#ifdef __cplusplus
}
#endif

#endif
