#include "_scanf.h"

#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

#define FLAG_ASTERISK (1 << 0)
#define FLAG_SQUOTE   (1 << 1)
#define FLAG_M        (1 << 2)
#define FLAG_HH       (1 << 3)
#define FLAG_H        (1 << 4)
#define FLAG_LL       (1 << 5)
#define FLAG_L        (1 << 6)
#define FLAG_J        (1 << 7)
#define FLAG_Z        (1 << 8)
#define FLAG_T        (1 << 9)

struct arg
{
	va_list *va_arg;
	uint32_t flags;
	int width;
	uint8_t type;
};

static int parse_arg(struct arg *arg, const char *fmt, size_t *i);
static int scan_mod(struct buf *buf, struct arg *arg);
static int scan_d(struct buf *buf, struct arg *arg);
static int scan_i(struct buf *buf, struct arg *arg);
static int scan_o(struct buf *buf, struct arg *arg);
static int scan_u(struct buf *buf, struct arg *arg);
static int scan_x(struct buf *buf, struct arg *arg);
static int scan_f(struct buf *buf, struct arg *arg);
static int scan_s(struct buf *buf, struct arg *arg);
static int scan_c(struct buf *buf, struct arg *arg);
static int scan_p(struct buf *buf, struct arg *arg);
static int scan_n(struct buf *buf, struct arg *arg);

static int get_char(struct buf *buf)
{
	switch (buf->type)
	{
		case SCANF_STR:
			if (!*buf->str)
				return EOF;
			buf->ninput++;
			return *(buf->str++);
		case SCANF_FP:
			buf->ninput++;
			return getc(buf->fp);
	}
	return EOF;
}

static void unget_char(struct buf *buf, char c)
{
	switch (buf->type)
	{
		case SCANF_STR:
			buf->ninput--;
			buf->str--;
			break;
		case SCANF_FP:
			buf->ninput--;
			ungetc(c, buf->fp);
			break;
	}
}

static void arg_ctr(struct arg *arg, va_list *va_arg)
{
	arg->va_arg = va_arg;
	arg->flags = 0;
	arg->width = -1;
	arg->type = '\0';
}

static void skip_spaces(struct buf *buf)
{
	int c;
	do
	{
		c = get_char(buf);
	} while (isspace(c));
	if (c != EOF)
		unget_char(buf, c);
}

int scanf_buf(struct buf *buf, const char *fmt, va_list va_arg)
{
	int ret;
	va_list va_cpy;
	va_copy(va_cpy, va_arg);
	buf->nconv = 0;
	buf->ninput = 0;
	for (size_t i = 0; fmt[i]; ++i)
	{
		if (fmt[i] != '%')
		{
			if (isspace(fmt[i]))
			{
				skip_spaces(buf);
				continue;
			}
			int c = get_char(buf);
			if (c == EOF)
			{
				ret = EOF;
				goto end;
			}
			if (c != fmt[i])
				break;
			continue;
		}
		struct arg arg;
		i++;
		arg_ctr(&arg, &va_cpy);
		parse_arg(&arg, fmt, &i);
		switch (arg.type)
		{
			case '%':
				ret = scan_mod(buf, &arg);
				break;
			case 'd':
				ret = scan_d(buf, &arg);
				break;
			case 'i':
				ret = scan_i(buf, &arg);
				break;
			case 'o':
				ret = scan_o(buf, &arg);
				break;
			case 'u':
				ret = scan_u(buf, &arg);
				break;
			case 'x':
			case 'X':
				ret = scan_x(buf, &arg);
				break;
			case 'f':
			case 'e':
			case 'g':
			case 'E':
			case 'a':
				ret = scan_f(buf, &arg);
				break;
			case 's':
				ret = scan_s(buf, &arg);
				break;
			case 'c':
				ret = scan_c(buf, &arg);
				break;
			/* XXX [ */
			case 'p':
				ret = scan_p(buf, &arg);
				break;
			case 'n':
				ret = scan_n(buf, &arg);
				break;
			default:
				continue;
		}
		if (ret)
		{
			if (buf->type == SCANF_FP
			 && ferror(buf->fp))
			{
				ret = EOF;
				goto end;
			}
			break;
		}
	}
	ret = buf->nconv;

end:
	va_end(va_cpy);
	return ret;
}

static int parse_flags(struct arg *arg, char c)
{
	switch (c)
	{
		case '*':
			arg->flags |= FLAG_ASTERISK;
			break;
		case '\'':
			arg->flags |= FLAG_SQUOTE;
			break;
		case 'm':
			arg->flags |= FLAG_M;
			break;
		default:
			return 0;
	}
	return 1;
}

static void parse_length(struct arg *arg, const char *fmt, size_t *i)
{
	if (fmt[*i] == 'h')
	{
		if (fmt[*i + 1] == 'h')
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
	else if (fmt[*i] == 'l')
	{
		if (fmt[*i + 1] == 'l')
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
	else if (fmt[*i] == 'j')
	{
		arg->flags |= FLAG_J;
		(*i)++;
	}
	else if (fmt[*i] == 'z')
	{
		arg->flags |= FLAG_Z;
		(*i)++;
	}
	else if (fmt[*i] == 't')
	{
		arg->flags |= FLAG_T;
		(*i)++;
	}
}

static int parse_width(struct arg *arg, const char *fmt, size_t *i)
{
	char *endptr;

	errno = 0;
	arg->width = strtoul(&fmt[*i], &endptr, 10);
	if (errno)
		return 0;
	*i += endptr - &fmt[*i];
	return 1;
}

static int parse_arg(struct arg *arg, const char *fmt, size_t *i)
{
	while (parse_flags(arg, fmt[*i]))
		(*i)++;
	if (!parse_width(arg, fmt, i))
		return 0;
	parse_length(arg, fmt, i);
	arg->type = fmt[*i];
	return 1;
}

static void set_val(struct arg *arg, unsigned long long val)
{
	if (arg->flags & FLAG_LL)
	{
		*va_arg(*arg->va_arg, unsigned long long*) = val;
		return;
	}
	if (arg->flags & FLAG_L)
	{
		*va_arg(*arg->va_arg, unsigned long*) = val;
		return;
	}
	if (arg->flags & FLAG_HH)
	{
		*va_arg(*arg->va_arg, unsigned char*) = val;
		return;
	}
	if (arg->flags & FLAG_H)
	{
		*va_arg(*arg->va_arg, unsigned short*) = val;
		return;
	}
	if (arg->flags & FLAG_Z)
	{
		*va_arg(*arg->va_arg, size_t*) = val;
		return;
	}
	if (arg->flags & FLAG_T)
	{
		*va_arg(*arg->va_arg, ptrdiff_t*) = val;
		return;
	}
	*va_arg(*arg->va_arg, int*) = val;
}

static int scan_mod(struct buf *buf, struct arg *arg)
{
	int c;

	(void)arg;
	skip_spaces(buf);
	c = get_char(buf);
	if (c == EOF || c != '%')
		return 1;
	return 0;
}

static unsigned long long get_decimal(struct buf *buf, struct arg *arg)
{
	unsigned long long val = 0;
	int c;

	while ((c = get_char(buf)) != EOF)
	{
		if (!isdigit(c))
		{
			if (c == '\'' && (arg->flags & FLAG_SQUOTE))
				continue;
			unget_char(buf, c);
			break;
		}
		val = val * 10 + c - '0';
	}
	return val;
}

static unsigned long long get_octal(struct buf *buf)
{
	unsigned long long val = 0;
	int c;

	while ((c = get_char(buf)) != EOF)
	{
		if (c < '0' || c >= '8')
		{
			unget_char(buf, c);
			break;
		}
		val = val * 8 + c - '0';
	}
	return val;
}

static unsigned long long get_hexadecimal(struct buf *buf)
{
	unsigned long long val = 0;
	int c;

	while ((c = get_char(buf)) != EOF)
	{
		if (c >= '0' && c <= '9')
		{
			val = val * 16 + c - '0';
		}
		else if (c >= 'a' && c <= 'f')
		{
			val = val * 16 + c - 'a' + 10;
		}
		else if (c >= 'A' && c <= 'F')
		{
			val = val * 16 + c - 'A' + 10;
		}
		else
		{
			unget_char(buf, c);
			break;
		}
	}
	return val;
}

static unsigned long long get_number(struct buf *buf, struct arg *arg)
{
	int c = get_char(buf);
	if (c == EOF)
		return 0;
	if (c == '0')
	{
		c = get_char(buf);
		if (c == EOF)
			return 0;
		if (c == 'x' || c == 'X')
			return get_hexadecimal(buf);
		if (c >= '0' && c <= '7')
		{
			unget_char(buf, c);
			return get_octal(buf);
		}
		unget_char(buf, c);
		return 0;
	}
	unget_char(buf, c);
	return get_decimal(buf, arg);
}

static int scan_d(struct buf *buf, struct arg *arg)
{
	unsigned long long val;
	int c;
	int neg = 0;
	int prev_ninput;

	skip_spaces(buf);
	c = get_char(buf);
	if (c == EOF)
		return 1;
	if (c == '-')
		neg = 1;
	else
		unget_char(buf, c);
	prev_ninput = buf->ninput;
	val = get_decimal(buf, arg);
	if (buf->ninput == prev_ninput)
		return 1;
	if (neg)
		val = -val;
	if (!(arg->flags & FLAG_ASTERISK))
	{
		set_val(arg, val);
		buf->nconv++;
	}
	return 0;
}

static int scan_i(struct buf *buf, struct arg *arg)
{
	unsigned long long val;
	int c;
	int neg = 0;
	int prev_ninput;

	skip_spaces(buf);
	c = get_char(buf);
	if (c == EOF)
		return 1;
	if (c == '-')
		neg = 1;
	else
		unget_char(buf, c);
	prev_ninput = buf->ninput;
	val = get_number(buf, arg);
	if (buf->ninput == prev_ninput)
		return 1;
	if (neg)
		val = -val;
	if (!(arg->flags & FLAG_ASTERISK))
	{
		set_val(arg, val);
		buf->nconv++;
	}
	return 0;
}

static int scan_o(struct buf *buf, struct arg *arg)
{
	unsigned long long val;
	int prev_ninput;

	skip_spaces(buf);
	prev_ninput = buf->ninput;
	val = get_octal(buf);
	if (buf->ninput == prev_ninput)
		return 1;
	if (!(arg->flags & FLAG_ASTERISK))
	{
		set_val(arg, val);
		buf->nconv++;
	}
	return 0;
}

static int scan_u(struct buf *buf, struct arg *arg)
{
	unsigned long long val;
	int prev_ninput;

	skip_spaces(buf);
	prev_ninput = buf->ninput;
	val = get_decimal(buf, arg);
	if (buf->ninput == prev_ninput)
		return 1;
	if (!(arg->flags & FLAG_ASTERISK))
	{
		set_val(arg, val);
		buf->nconv++;
	}
	return 0;
}

static int scan_x(struct buf *buf, struct arg *arg)
{
	unsigned long long val;
	int prev_ninput;
	int c;

	skip_spaces(buf);
	prev_ninput = buf->ninput;
	c = get_char(buf);
	if (c == '0')
	{
		c = get_char(buf);
		if (c != 'x' && c != 'X')
			unget_char(buf, c);
	}
	else
	{
		unget_char(buf, c);
	}
	val = get_hexadecimal(buf);
	if (buf->ninput == prev_ninput)
		return 1;
	if (!(arg->flags & FLAG_ASTERISK))
	{
		set_val(arg, val);
		buf->nconv++;
	}
	return 0;
}

static int scan_f(struct buf *buf, struct arg *arg)
{
	skip_spaces(buf);
	(void)buf;
	(void)arg;
	/* XXX */
	return 0;
}

static int scan_ms(struct buf *buf, struct arg *arg)
{
	skip_spaces(buf);
	(void)buf;
	(void)arg;
	/* XXX */
	return 0;
}

static int scan_s(struct buf *buf, struct arg *arg)
{
	char *base = NULL;
	int c;

	if (arg->flags & FLAG_M)
		return scan_ms(buf, arg);
	skip_spaces(buf);
	if (!(arg->flags & FLAG_ASTERISK))
		base = va_arg(*arg->va_arg, char*);
	for (int i = 0;; i++)
	{
		if (arg->width && i == arg->width)
			break;
		c = get_char(buf);
		if (c == EOF)
			return 1;
		if (isspace(c))
			break;
		if (!(arg->flags & FLAG_ASTERISK))
			*(base++) = c;
	}
	if (!(arg->flags & FLAG_ASTERISK))
	{
		*(base++) = '\0';
		buf->nconv++;
	}
	return 0;
}

static int scan_c(struct buf *buf, struct arg *arg)
{
	char *base = NULL;
	int nchars;
	int c;

	nchars = arg->width;
	if (!nchars)
		nchars = 1;
	if (!(arg->flags & FLAG_ASTERISK))
		base = va_arg(*arg->va_arg, char*);
	for (int i = 0; i < nchars; ++i)
	{
		c = get_char(buf);
		if (c == EOF)
			return 1;
		if (!(arg->flags & FLAG_ASTERISK))
			*(base++) = c;
	}
	if (!(arg->flags & FLAG_ASTERISK))
		buf->nconv++;
	return 0;
}

static int scan_p(struct buf *buf, struct arg *arg)
{
	unsigned long long val;
	int prev_ninput;
	int c;

	c = get_char(buf);
	if (c == '(')
	{
		c = get_char(buf);
		if (c != 'n')
			return 1;
		c = get_char(buf);
		if (c != 'i')
			return 1;
		c = get_char(buf);
		if (c != 'l')
			return 1;
		c = get_char(buf);
		if (c != ')')
			return 1;
		if (!(arg->flags & FLAG_ASTERISK))
		{
			*va_arg(*arg->va_arg, void**) = NULL;
			buf->nconv++;
		}
		return 0;
	}
	unget_char(buf, c);
	prev_ninput = buf->ninput;
	val = get_number(buf, arg);
	if (buf->ninput == prev_ninput)
		return 1;
	if (!(arg->flags & FLAG_ASTERISK))
	{
		*va_arg(*arg->va_arg, void**) = (void*)val;
		buf->nconv++;
	}
	return 0;
}

static int scan_n(struct buf *buf, struct arg *arg)
{
	if (!(arg->flags & FLAG_ASTERISK))
		*va_arg(*arg->va_arg, int*) = buf->ninput;
	return 0;
}
