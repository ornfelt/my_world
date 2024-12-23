#ifndef JKS_UTF8_H
#define JKS_UTF8_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#define UTF8_NEXT(s) (*(str++))

static inline uint32_t utf8_codepoint_length(const char *str)
{
	if ((*str & 0x80) == 0x00)
		return 1;
	if ((*str & 0xE0) == 0xC0)
		return 2;
	if ((*str & 0xF0) == 0xE0)
		return 3;
	if ((*str & 0xF8) == 0xF0)
		return 4;
	return 0;
}

static inline void utf8_encode1(char *str, uint32_t cp)
{
	UTF8_NEXT(str) = cp & 0x7F;
}

static inline uint32_t utf8_decode1(const char *str)
{
	return UTF8_NEXT(str);
}

static inline bool utf8_check1(const char *str)
{
	(void)str;
	return true;
}

static inline void utf8_encode2(char *str, uint32_t cp)
{
	UTF8_NEXT(str) = 0xC0 | ((cp >> 0x6) & 0x1F);
	UTF8_NEXT(str) = 0x80 | ((cp >> 0x0) & 0x3F);
}

static inline uint32_t utf8_decode2(const char *str)
{
	uint32_t cp;
	cp  = (uint32_t)(UTF8_NEXT(str) & 0x1F) << 0x6;
	cp |= (uint32_t)(UTF8_NEXT(str) & 0x3F) << 0x0;
	return cp;
}

static inline bool utf8_check2(const char *str)
{
	if ((str[0] & 0x1F) < 2)
		return false;
	if ((str[1] & 0xC0) != 0x80)
		return false;
	return true;
}

static inline void utf8_encode3(char *str, uint32_t cp)
{
	UTF8_NEXT(str) = 0xE0 | ((cp >> 0xC) & 0x0F);
	UTF8_NEXT(str) = 0x80 | ((cp >> 0x6) & 0x3F);
	UTF8_NEXT(str) = 0x80 | ((cp >> 0x0) & 0x3F);
}

static inline uint32_t utf8_decode3(const char *str)
{
	uint32_t cp;
	cp  = (uint32_t)(UTF8_NEXT(str) & 0x0F) << 0xC;
	cp |= (uint32_t)(UTF8_NEXT(str) & 0x3F) << 0x6;
	cp |= (uint32_t)(UTF8_NEXT(str) & 0x3F) << 0x0;
	return cp;
}

static inline bool utf8_check3(const char *str)
{
	if (!(str[0] & 0xF) && !(str[1] & 0x20))
		return false;
	if ((str[1] & 0xC0) != 0x80)
		return false;
	if ((str[2] & 0xC0) != 0x80)
		return false;
	return true;
}

static inline void utf8_encode4(char *str, uint32_t cp)
{
	UTF8_NEXT(str) = 0xF0 | ((cp >> 0x12) & 0x07);
	UTF8_NEXT(str) = 0x80 | ((cp >> 0x0C) & 0x3F);
	UTF8_NEXT(str) = 0x80 | ((cp >> 0x06) & 0x3F);
	UTF8_NEXT(str) = 0x80 | ((cp >> 0x00) & 0x3F);
}

static inline uint32_t utf8_decode4(const char *str)
{
	uint32_t cp;
	cp  = (uint32_t)(UTF8_NEXT(str) & 0x07) << 0x12;
	cp |= (uint32_t)(UTF8_NEXT(str) & 0x3F) << 0x0C;
	cp |= (uint32_t)(UTF8_NEXT(str) & 0x3F) << 0x06;
	cp |= (uint32_t)(UTF8_NEXT(str) & 0x3F) << 0x00;
	return cp;
}

static inline bool utf8_check4(const char *str)
{
	if (!(str[0] & 0x7) && !(str[1] & 0x30))
		return false;
	if ((str[1] & 0xC0) != 0x80)
		return false;
	if ((str[2] & 0xC0) != 0x80)
		return false;
	if ((str[3] & 0xC0) != 0x80)
		return false;
	return true;
}

static inline bool utf8_encode(char **str, uint32_t cp)
{
	if (cp < 0x80)
	{
		utf8_encode1(*str, cp);
		*str += 1;
		return true;
	}
	else if (cp < 0x800)
	{
		utf8_encode2(*str, cp);
		*str += 2;
		return true;
	}
	else if (cp < 0x10000)
	{
		utf8_encode3(*str, cp);
		*str += 3;
		return true;
	}
	else if (cp < 0x10FFFF)
	{
		utf8_encode4(*str, cp);
		*str += 4;
		return true;
	}
	return false;
}

static inline bool utf8_decode(const char **str, uint32_t *cp)
{
	switch (utf8_codepoint_length(*str))
	{
		case 1:
			*cp = utf8_decode1(*str);
			*str += 1;
			return true;
		case 2:
			*cp = utf8_decode2(*str);
			*str += 2;
			return true;
		case 3:
			*cp = utf8_decode3(*str);
			*str += 3;
			return true;
		case 4:
			*cp = utf8_decode4(*str);
			*str += 4;
			return true;
		default:
			return false;
	}
	return false;
}

static inline bool utf8_check(const char **str, const char *end)
{
	switch (utf8_codepoint_length(*str))
	{
		case 1:
			if (end - *str < 1)
				return false;
			return utf8_check1(*str);
		case 2:
			if (end - *str < 2)
				return false;
			return utf8_check2(*str);
		case 3:
			if (end - *str < 3)
				return false;
			return utf8_check3(*str);
		case 4:
			if (end - *str < 4)
				return false;
			return utf8_check4(*str);
		default:
			return false;
	}
	return false;
}

static inline bool utf8_next(const char **str, const char *end, uint32_t *cp)
{
	switch (utf8_codepoint_length(*str))
	{
		case 1:
			if (end - *str < 1)
				return false;
			if (!utf8_check1(*str))
				return false;
			*cp = utf8_decode1(*str);
			*str += 1;
			return true;
		case 2:
			if (end - *str < 2)
				return false;
			if (!utf8_check2(*str))
				return false;
			*cp = utf8_decode2(*str);
			*str += 2;
			return true;
		case 3:
			if (end - *str < 3)
				return false;
			if (!utf8_check3(*str))
				return false;
			*cp = utf8_decode3(*str);
			*str += 3;
			return true;
		case 4:
			if (end - *str < 4)
				return false;
			if (!utf8_check4(*str))
				return false;
			*cp = utf8_decode4(*str);
			*str += 4;
			return true;
		default:
			return false;
	}
	return false;
}

static inline bool utf8_peek_next(const char *str, const char *end, uint32_t *cp)
{
	return utf8_next(&str, end, cp);
}

static inline bool utf8_prev(const char **str, const char *begin, uint32_t *cp)
{
	const char *org = *str;
	for (size_t i = 0; i < 4; ++i)
	{
		if (*str - begin < 1)
			return false;
		(*str)--;
		switch (utf8_codepoint_length(*str))
		{
			case 1:
				if (org - *str != 1)
					return false;
				if (!utf8_check1(*str))
					return false;
				*cp = utf8_decode1(*str);
				return true;
			case 2:
				if (org - *str != 2)
					return false;
				if (!utf8_check2(*str))
					return false;
				*cp = utf8_decode2(*str);
				return true;
			case 3:
				if (org - *str != 3)
					return false;
				if (!utf8_check3(*str))
					return false;
				*cp = utf8_decode3(*str);
				return true;
			case 4:
				if (org - *str != 4)
					return false;
				if (!utf8_check4(*str))
					return false;
				*cp = utf8_decode4(*str);
				return true;
		}
	}
	return false;
}

static inline bool utf8_peek_prev(const char *str, const char *begin, uint32_t *cp)
{
	return utf8_prev(&str, begin, cp);
}

static inline bool utf8_advance_step(const char **str, const char *end)
{
	switch (utf8_codepoint_length(*str))
	{
		case 1:
			if (end - *str < 1)
				return false;
			if (!utf8_check1(*str))
				return false;
			*str += 1;
			return true;
		case 2:
			if (end - *str < 2)
				return false;
			if (!utf8_check2(*str))
				return false;
			*str += 2;
			return true;
		case 3:
			if (end - *str < 3)
				return false;
			if (!utf8_check3(*str))
				return false;
			*str += 3;
			return true;
		case 4:
			if (end - *str < 4)
				return false;
			if (!utf8_check4(*str))
				return false;
			*str += 4;
			return true;
		default:
			return false;
	}
	return false;
}

static inline bool utf8_advance(const char **str, const char *end, size_t len)
{
	for (size_t i = 0; i < len; ++i)
	{
		if (!utf8_advance_step(str, end))
			return false;
	}
	return true;
}

static inline bool utf8_distance(const char *str, const char *end, size_t *distance)
{
	*distance = 0;
	while (str != end)
	{
		if (!utf8_advance_step(&str, end))
			return false;
		(*distance)++;
	}
	return true;
}

static inline const char *utf8_find_invalid(const char *str, const char *end)
{
	while (str != end)
	{
		if (!utf8_advance_step(&str, end))
			return str;
	}
	return str;
}

static inline bool utf8_is_valid(const char *str, const char *end)
{
	return utf8_find_invalid(str, end) == end;
}

#ifdef __cplusplus
}
#endif

#endif
