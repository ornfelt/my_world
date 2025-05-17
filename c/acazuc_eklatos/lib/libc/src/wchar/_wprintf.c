#include "_wprintf.h"

#include <stdlib.h>
#include <string.h>
#include <wctype.h>
#include <limits.h>
#include <wchar.h>
#include <errno.h>

#define FLAG_MINUS (1 << 0)
#define FLAG_SPACE (1 << 1)
#define FLAG_ZERO  (1 << 2)
#define FLAG_PLUS  (1 << 3)
#define FLAG_SHARP (1 << 4)
#define FLAG_HH    (1 << 5)
#define FLAG_H     (1 << 6)
#define FLAG_LL    (1 << 7)
#define FLAG_L     (1 << 8)
#define FLAG_J     (1 << 9)
#define FLAG_Z     (1 << 10)
#define FLAG_T     (1 << 11)

struct arg
{
	va_list *va_arg;
	uint32_t flags;
	int width;
	int preci;
	wchar_t type;
};

typedef void (*print_fn_t)(struct buf*, struct arg*);

static int parse_arg(struct arg *arg, const wchar_t *fmt, size_t *i);
static void print_c(struct buf *buf, struct arg *arg);
static void print_C(struct buf *buf, struct arg *arg);
static void print_d(struct buf *buf, struct arg *arg);
static void print_o(struct buf *buf, struct arg *arg);
static void print_s(struct buf *buf, struct arg *arg);
static void print_S(struct buf *buf, struct arg *arg);
static void print_u(struct buf *buf, struct arg *arg);
static void print_x(struct buf *buf, struct arg *arg);
static void print_p(struct buf *buf, struct arg *arg);
static void print_mod(struct buf *buf, struct arg *arg);
static void print_f(struct buf *buf, struct arg *arg);

static void
arg_ctr(struct arg *arg, va_list *va_arg)
{
	arg->va_arg = va_arg;
	arg->flags = 0;
	arg->width = -1;
	arg->preci = -1;
	arg->type = '\0';
}

static void
outwcs(struct buf *buf, const wchar_t *s, size_t n)
{
	switch (buf->type)
	{
		case PRINTF_BUF:
		{
			if (buf->len >= buf->size)
				break;
			size_t count = n;
			if (buf->len + count > buf->size)
				count = buf->size - buf->len;
			wmemcpy(&buf->data[buf->len], s, count);
			break;
		}
		case PRINTF_FP:
			fwrite(s, 1, n * sizeof(*s), buf->fp);
			break;
	}
	buf->len += n;
}

static void
outchar(struct buf *buf, wchar_t c)
{
	outwcs(buf, &c, 1);
}

static void
outchars(struct buf *buf, wchar_t c, size_t n)
{
	wchar_t tmp[1024];
	size_t count = n;

	if (count > sizeof(tmp) / sizeof(*tmp))
		count = sizeof(tmp) / sizeof(*tmp);
	wmemset(tmp, c, count);
	do
	{
		outwcs(buf, tmp, count);
		n -= count;
		count = n;
		if (count > sizeof(tmp) / sizeof(*tmp))
			count = sizeof(tmp) / sizeof(*tmp);
	} while (count);
}

static void
outstr(struct buf *buf, const char *s, size_t n)
{
	wchar_t tmp[1024];

	while (n && s)
	{
		const char *prv = s;
		size_t nc = mbsnrtowcs(tmp, &s, n, sizeof(tmp) / sizeof(*tmp), NULL);
		if (nc == (size_t)-1)
			break;
		outwcs(buf, tmp, nc);
		n -= s - prv;
	}
}

static long long
get_int_val(struct arg *arg)
{
	if (arg->flags & FLAG_LL)
		return va_arg(*arg->va_arg, long long);
	if (arg->flags & FLAG_L)
		return va_arg(*arg->va_arg, long);
	if (arg->flags & FLAG_HH)
		return (char)va_arg(*arg->va_arg, int);
	if (arg->flags & FLAG_H)
		return (short)va_arg(*arg->va_arg, int);
	if (arg->flags & FLAG_J)
		return va_arg(*arg->va_arg, intmax_t);
	if (arg->flags & FLAG_Z)
		return va_arg(*arg->va_arg, size_t);
	if (arg->flags & FLAG_T)
		return va_arg(*arg->va_arg, ptrdiff_t);
	return va_arg(*arg->va_arg, int);
}

static unsigned long long
get_uint_val(struct arg *arg)
{
	if (arg->flags & FLAG_LL)
		return va_arg(*arg->va_arg, unsigned long long);
	if (arg->flags & FLAG_L)
		return va_arg(*arg->va_arg, unsigned long);
	if (arg->flags & FLAG_HH)
		return (unsigned char)va_arg(*arg->va_arg, unsigned);
	if (arg->flags & FLAG_H)
		return (unsigned short)va_arg(*arg->va_arg, unsigned);
	if (arg->flags & FLAG_J)
		return va_arg(*arg->va_arg, uintmax_t);
	if (arg->flags & FLAG_Z)
		return va_arg(*arg->va_arg, size_t);
	if (arg->flags & FLAG_T)
		return va_arg(*arg->va_arg, ptrdiff_t);
	return va_arg(*arg->va_arg, unsigned);
}

int
wprintf_buf(struct buf *buf, const wchar_t *fmt, va_list va_arg)
{
	va_list va_cpy;
	va_copy(va_cpy, va_arg);
	buf->len = 0;
	size_t first_nonfmt = 0;
	size_t i;
	for (i = 0; fmt[i]; ++i)
	{
		if (fmt[i] != L'%')
			continue;
		if (first_nonfmt != i)
			outwcs(buf, &fmt[first_nonfmt], i - first_nonfmt);
		struct arg arg;
		i++;
		arg_ctr(&arg, &va_cpy);
		parse_arg(&arg, fmt, &i);
		switch (arg.type)
		{
			case L'c':
				print_c(buf, &arg);
				break;
			case L'C':
				print_C(buf, &arg);
				break;
			case L'd':
			case L'i':
				print_d(buf, &arg);
				break;
			case L'o':
				print_o(buf, &arg);
				break;
			case L's':
				print_s(buf, &arg);
				break;
			case L'S':
				print_S(buf, &arg);
				break;
			case L'u':
				print_u(buf, &arg);
				break;
			case L'x':
			case L'X':
				print_x(buf, &arg);
				break;
			case L'p':
				print_p(buf, &arg);
				break;
			case L'%':
				print_mod(buf, &arg);
				break;
			case L'f':
				print_f(buf, &arg);
				break;
			default:
				continue;
		}
		first_nonfmt = i + 1;
	}
	if (first_nonfmt != i)
		outwcs(buf, &fmt[first_nonfmt], i - first_nonfmt);
	va_end(va_cpy);
	return buf->len;
}

static int
parse_flags(struct arg *arg, wchar_t c)
{
	switch (c)
	{
		case L'-':
			arg->flags |= FLAG_MINUS;
			break;
		case L'+':
			arg->flags |= FLAG_PLUS;
			break;
		case L'0':
			arg->flags |= FLAG_ZERO;
			break;
		case L'#':
			arg->flags |= FLAG_SHARP;
			break;
		case L' ':
			arg->flags |= FLAG_SPACE;
			break;
		default:
			return 0;
	}
	return 1;
}

static int
parse_preci(struct arg *arg, const wchar_t *fmt, size_t *i)
{
	wchar_t *endptr;

	if (fmt[*i] != L'.')
		return 1;
	(*i)++;
	if (fmt[*i] == L'*')
	{
		arg->preci = va_arg(*arg->va_arg, int);
		(*i)++;
		return 1;
	}
	errno = 0;
	arg->preci = wcstoul(&fmt[*i], &endptr, 10);
	if (errno)
		return 0;
	*i += endptr - &fmt[*i];
	return 1;
}

static void
parse_length(struct arg *arg, const wchar_t *fmt, size_t *i)
{
	if (fmt[*i] == L'h')
	{
		if (fmt[*i + 1] == L'h')
		{
			arg->flags |= FLAG_HH;
			(*i)++;
		}
		else
		{
			arg->flags |= FLAG_H;
		}
		(*i)++;
	}
	else if (fmt[*i] == L'l')
	{
		if (fmt[*i + 1] == L'l')
		{
			arg->flags |= FLAG_LL;
			(*i)++;
		}
		else
		{
			arg->flags |= FLAG_L;
		}
		(*i)++;
	}
	else if (fmt[*i] == L'j')
	{
		arg->flags |= FLAG_J;
		(*i)++;
	}
	else if (fmt[*i] == L'z')
	{
		arg->flags |= FLAG_Z;
		(*i)++;
	}
	else if (fmt[*i] == L't')
	{
		arg->flags |= FLAG_T;
		(*i)++;
	}
}

static int
parse_width(struct arg *arg, const wchar_t *fmt, size_t *i)
{
	wchar_t *endptr;

	if (fmt[*i] == L'*')
	{
		arg->width = va_arg(*arg->va_arg, int);
		(*i)++;
		return 1;
	}
	errno = 0;
	arg->width = wcstoul(&fmt[*i], &endptr, 10);
	if (errno)
		return 0;
	*i += endptr - &fmt[*i];
	return 1;
}

static int
parse_arg(struct arg *arg, const wchar_t *fmt, size_t *i)
{
	while (parse_flags(arg, fmt[*i]))
		(*i)++;
	if (!parse_width(arg, fmt, i))
		return 0;
	if (!parse_preci(arg, fmt, i))
		return 0;
	parse_length(arg, fmt, i);
	arg->type = fmt[*i];
	if (arg->flags & FLAG_MINUS)
		arg->flags &= ~FLAG_ZERO;
	return 1;
}

static void
print_str(struct buf *buf,
          struct arg *arg,
          const wchar_t *prefix,
          const char *s,
          size_t len)
{
	size_t prefix_len = prefix ? wcslen(prefix) : 0;
	size_t pad_len;

	if (arg->width > 0 && (size_t)arg->width > len + prefix_len)
		pad_len = arg->width - len - prefix_len;
	else
		pad_len = 0;
	if (pad_len && !(arg->flags & (FLAG_ZERO | FLAG_MINUS)))
		outchars(buf, L' ', pad_len);
	if (prefix)
		outwcs(buf, prefix, wcslen(prefix));
	if (pad_len && (arg->flags & FLAG_ZERO))
		outchars(buf, L'0', pad_len);
	outstr(buf, s, len);
	if (pad_len && (arg->flags & FLAG_MINUS))
		outchars(buf, L' ', pad_len);
}

static void
print_wcs(struct buf *buf,
          struct arg *arg,
          const wchar_t *prefix,
          const wchar_t *s,
          size_t len)
{
	size_t prefix_len = prefix ? wcslen(prefix) : 0;
	size_t pad_len;

	if (arg->width > 0 && (size_t)arg->width > len + prefix_len)
		pad_len = arg->width - len - prefix_len;
	else
		pad_len = 0;
	if (pad_len && !(arg->flags & (FLAG_ZERO | FLAG_MINUS)))
		outchars(buf, L' ', pad_len);
	if (prefix)
		outwcs(buf, prefix, wcslen(prefix));
	if (pad_len && (arg->flags & FLAG_ZERO))
		outchars(buf, L'0', pad_len);
	outwcs(buf, s, len);
	if (pad_len && (arg->flags & FLAG_MINUS))
		outchars(buf, L' ', pad_len);
}

static void
print_nbr(struct buf *buf,
          struct arg *arg,
          const wchar_t *prefix,
          const wchar_t *s,
          size_t len)
{
	size_t prefix_len = prefix ? wcslen(prefix) : 0;
	size_t preci_len;
	size_t pad_len;

	preci_len = arg->preci >= 0 && (size_t)arg->preci > len
	          ? arg->preci - len : 0;
	if (preci_len && prefix && prefix[0] == L'0' && prefix[1] == L'\0')
		preci_len--;
	if (arg->width > 0 && (size_t)arg->width > len + prefix_len + preci_len)
		pad_len = arg->width - len - prefix_len - preci_len;
	else
		pad_len = 0;
	if (pad_len && !(arg->flags & (FLAG_ZERO | FLAG_MINUS)))
		outchars(buf, L' ', pad_len);
	if (prefix)
		outwcs(buf, prefix, wcslen(prefix));
	if (pad_len && (arg->flags & FLAG_ZERO))
		outchars(buf, L'0', pad_len);
	outchars(buf, L'0', preci_len);
	outwcs(buf, s, len);
	if (pad_len && (arg->flags & FLAG_MINUS))
		outchars(buf, L' ', pad_len);
}

static void
ulltowcs(wchar_t *d, unsigned long long int n, const wchar_t *base)
{
	size_t size;
	size_t base_len;
	size_t i;
	unsigned long long int nb;

	if (!n)
	{
		wcscpy(d, L"0");
		return;
	}
	nb = n;
	base_len = wcslen(base);
	size = 1;
	while (n > 0)
	{
		size++;
		n /= base_len;
	}
	i = 2;
	while (nb > 0)
	{
		d[size - i] = base[nb % base_len];
		nb /= base_len;
		++i;
	}
	d[size - 1] = L'\0';
}

static void
lltowcs(wchar_t *d, long long n, const wchar_t *base)
{
	if (n < 0)
	{
		d[0] = L'-';
		if (n == LLONG_MIN)
			ulltowcs(&d[1], (unsigned long long)LLONG_MAX + 1, base);
		else
			ulltowcs(&d[1], -n, base);
	}
	else
	{
		ulltowcs(d, n, base);
	}
}

static void
print_c(struct buf *buf, struct arg *arg)
{
	wint_t v;

	v = btowc(va_arg(*arg->va_arg, int));
	print_wcs(buf, arg, NULL, (wchar_t*)&v, 1);
}

static void
print_C(struct buf *buf, struct arg *arg)
{
	wchar_t v;

	v = va_arg(*arg->va_arg, wchar_t);
	print_wcs(buf, arg, NULL, &v, 1);
}

static void
print_d(struct buf *buf, struct arg *arg)
{
	long long val;
	wchar_t str[64];

	val = get_int_val(arg);
	lltowcs(str, val, L"0123456789");
	print_nbr(buf, arg, NULL, str, wcslen(str));
}

static void
print_o(struct buf *buf, struct arg *arg)
{
	wchar_t str[64];
	unsigned long long val;
	const wchar_t *prefix;

	val = get_uint_val(arg);
	ulltowcs(str, val, L"01234567");
	if (val && (arg->flags & FLAG_SHARP))
		prefix = L"0";
	else
		prefix = NULL;
	print_nbr(buf, arg, prefix, str, wcslen(str));
}

static void
print_s(struct buf *buf, struct arg *arg)
{
	char *str;
	size_t len;

	if (arg->flags & FLAG_L)
	{
		print_S(buf, arg);
		return;
	}
	str = va_arg(*arg->va_arg, char*);
	if (!str)
		str = "(null)";
	len = strlen(str);
	if (arg->preci >= 0 && (size_t)arg->preci < len)
		len = arg->preci;
	print_str(buf, arg, NULL, str, len);
}

static void
print_S(struct buf *buf, struct arg *arg)
{
	wchar_t *str;
	size_t len;

	str = va_arg(*arg->va_arg, wchar_t*);
	if (!str)
		str = L"(null)";
	len = wcslen(str);
	if (arg->preci >= 0 && (size_t)arg->preci < len)
		len = arg->preci;
	print_wcs(buf, arg, NULL, str, len);
}

static void
print_u(struct buf *buf, struct arg *arg)
{
	wchar_t str[64];
	unsigned long long val;

	val = get_uint_val(arg);
	ulltowcs(str, val, L"0123456789");
	print_nbr(buf, arg, NULL, str, wcslen(str));
}

static wchar_t *
get_x_chars(struct arg *arg)
{
	if (arg->type == L'X')
		return L"0123456789ABCDEF";
	return L"0123456789abcdef";
}

static void
print_x(struct buf *buf, struct arg *arg)
{
	wchar_t str[64];
	unsigned long long val;
	const wchar_t *prefix;

	val = get_uint_val(arg);
	ulltowcs(str, val, get_x_chars(arg));
	if (val && (arg->flags & FLAG_SHARP))
		prefix = (arg->type == L'X' ? L"0X" : L"0x");
	else
		prefix = NULL;
	print_nbr(buf, arg, prefix, str, wcslen(str));
}

static void
print_p(struct buf *buf, struct arg *arg)
{
	wchar_t str[64];
	unsigned long long val;
	const wchar_t *prefix;

	arg->flags |= FLAG_L;
	val = get_uint_val(arg);
	if (!val)
	{
		arg->preci = -1;
		print_wcs(buf, arg, NULL, L"(nil)", 5);
		return;
	}
	arg->flags |= FLAG_SHARP;
	arg->flags &= ~(FLAG_LL | FLAG_H | FLAG_HH | FLAG_Z | FLAG_J | FLAG_T);
	ulltowcs(str, val, get_x_chars(arg));
	if (val && (arg->flags & FLAG_SHARP))
		prefix = (arg->type == L'X' ? L"0X" : L"0x");
	else
		prefix = NULL;
	print_nbr(buf, arg, prefix, str, wcslen(str));
}

static void
print_mod(struct buf *buf, struct arg *arg)
{
	(void)arg;
	outchar(buf, '%');
}

static void
print_f(struct buf *buf, struct arg *arg)
{
	(void)buf;
	(void)arg;
}
