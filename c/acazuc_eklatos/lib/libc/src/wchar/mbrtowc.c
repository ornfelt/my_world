#include <wchar.h>
#include <errno.h>

static int utf8_codepoint_length(const char *s)
{
	if ((*s & 0x80) == 0x00)
		return 1;
	if ((*s & 0xE0) == 0xC0)
		return 2;
	if ((*s & 0xF0) == 0xE0)
		return 3;
	if ((*s & 0xF8) == 0xF0)
		return 4;
	return -1;
}

static int utf8_check1(const char *s)
{
	(void)s;
	return 0;
}

static int utf8_check2(const char *s)
{
	return !(s[0] & 0x1E)
	    || (s[1] & 0xC0) != 0x80;
}

static int utf8_check3(const char *s)
{
	return !(s[0] & 0xF)
	    || (s[1] & 0xC0) != 0x80
	    || (s[2] & 0xC0) != 0x80;
}

static int utf8_check4(const char *s)
{
	return (!(s[0] & 0x7) && !(s[1] & 0x30))
	    || (s[1] & 0xC0) != 0x80
	    || (s[2] & 0xC0) != 0x80
	    || (s[3] & 0xC0) != 0x80;
}

size_t mbrtowc(wchar_t *wc, const char *s, size_t n, mbstate_t *ps)
{
	if (!n)
		return (size_t)-2;
	int wclen = utf8_codepoint_length(s);
	if (wclen < 0)
	{
		errno = EILSEQ;
		return (size_t)-1;
	}
	if (n < (size_t)wclen)
		return (size_t)-2;
	switch (wclen)
	{
		case 1:
			if (utf8_check1(s))
				break;
			*wc = *s;
			return 1;
		case 2:
			if (utf8_check2(s))
				break;
			*wc = (wchar_t)(s[0] & 0x1F) << 0x6
			    | (wchar_t)(s[1] & 0x3F) << 0x0;
			return 2;
		case 3:
			if (utf8_check3(s))
				break;
			*wc = (wchar_t)(s[0] & 0x0F) << 0xC
			    | (wchar_t)(s[1] & 0x3F) << 0x6
			    | (wchar_t)(s[2] & 0x3F) << 0x0;
			return 3;
		case 4:
			if (utf8_check4(s))
				break;
			*wc = (wchar_t)(s[0] & 0x07) << 0x12
			    | (wchar_t)(s[1] & 0x3F) << 0x0C
			    | (wchar_t)(s[2] & 0x3F) << 0x06
			    | (wchar_t)(s[3] & 0x3F) << 0x00;
			return 4;
	}
	errno = EILSEQ;
	return (size_t)-1;
}
