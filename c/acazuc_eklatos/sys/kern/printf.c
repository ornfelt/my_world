#include <std.h>
#include <tty.h>
#include <uio.h>

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
	uint8_t type;
};

struct buf
{
	enum
	{
		PRINTF_BUF,
		PRINTF_TTY,
		PRINTF_UIO,
	} type;
	union
	{
		struct
		{
			char *data;
			size_t size;
		} buf;
		struct
		{
			char buf[PAGE_SIZE];
			size_t buf_pos;
			struct uio *uio;
		} uio;
	};
	size_t len;
};

typedef int (*print_fn_t)(struct buf *, struct arg *);

static int parse_arg(struct arg *arg, const char *fmt, size_t *i);
static int print_c(struct buf *buf, struct arg *arg);
static int print_d(struct buf *buf, struct arg *arg);
static int print_o(struct buf *buf, struct arg *arg);
static int print_s(struct buf *buf, struct arg *arg);
static int print_u(struct buf *buf, struct arg *arg);
static int print_x(struct buf *buf, struct arg *arg);
static int print_p(struct buf *buf, struct arg *arg);
static int print_mod(struct buf *buf, struct arg *arg);

static struct tty *g_ttys[16];
static size_t g_ttys_nb;

early_printf_t g_early_printf;

int printf_addtty(struct tty *tty)
{
	if (g_ttys_nb >= sizeof(g_ttys) / sizeof(*g_ttys))
		return 1;
	g_ttys[g_ttys_nb++] = tty;
	return 0;
}

static void arg_ctr(struct arg *arg, va_list *va_arg)
{
	arg->va_arg = va_arg;
	arg->flags = 0;
	arg->width = -1;
	arg->preci = -1;
	arg->type = '\0';
}

static int outstr(struct buf *buf, const char *s, size_t n)
{
	switch (buf->type)
	{
		case PRINTF_BUF:
		{
			if (buf->len >= buf->buf.size)
				break;
			size_t count = n;
			if (buf->len + count > buf->buf.size)
				count = buf->buf.size - buf->len;
			memcpy(&buf->buf.data[buf->len], s, count);
			break;
		}
		case PRINTF_TTY:
		{
			if (!g_ttys_nb)
			{
				if (g_early_printf)
					g_early_printf(s, n);
				break;
			}
			struct uio uio;
			struct iovec iov;
			for (size_t i = 0; i < g_ttys_nb; ++i)
			{
				uio_fromkbuf(&uio, &iov, (void*)s, n, 0);
				tty_write(g_ttys[i], &uio);
			}
			break;
		}
		case PRINTF_UIO:
		{
			size_t count = n;
			if (buf->uio.uio->off > 0)
			{
				if (count <= (size_t)buf->uio.uio->off)
				{
					buf->uio.uio->off -= count;
					break;
				}
				s += buf->uio.uio->off;
				count -= buf->uio.uio->off;
				buf->uio.uio->off = 0;
			}
			while (count)
			{
				size_t avail = sizeof(buf->uio.buf) - buf->uio.buf_pos;
				if (count < avail)
				{
					memcpy(&buf->uio.buf[buf->uio.buf_pos], s, count);
					buf->uio.buf_pos += count;
					break;
				}
				memcpy(&buf->uio.buf[buf->uio.buf_pos], s, avail);
				s += avail;
				count -= avail;
				ssize_t ret = uio_copyin(buf->uio.uio, buf->uio.buf,
				                         sizeof(buf->uio.buf));
				buf->uio.uio->off = 0;
				buf->uio.buf_pos = 0;
				if (ret < 0)
					return ret;
			}
			break;
		}
	}
	buf->len += n;
	return 0;
}

static int outchar(struct buf *buf, char c)
{
	return outstr(buf, &c, 1);
}

static int outchars(struct buf *buf, char c, size_t n)
{
	char tmp[PAGE_SIZE];
	size_t count = n > sizeof(tmp) ? sizeof(tmp) : n;
	memset(tmp, c, count);
	do
	{
		int ret = outstr(buf, tmp, count);
		if (ret)
			return ret;
		n -= count;
		count = n > sizeof(tmp) ? sizeof(tmp) : n;
	} while (count);
	return 0;
}

static long long int get_int_val(struct arg *arg)
{
	if (arg->flags & FLAG_LL)
		return va_arg(*arg->va_arg, long long);
	if (arg->flags & FLAG_L)
		return va_arg(*arg->va_arg, long);
	if (arg->flags & FLAG_HH)
		return (char)va_arg(*arg->va_arg, long);
	if (arg->flags & FLAG_H)
		return (short)va_arg(*arg->va_arg, long);
	if (arg->flags & FLAG_J)
		return va_arg(*arg->va_arg, intmax_t);
	if (arg->flags & FLAG_Z)
		return va_arg(*arg->va_arg, size_t);
	if (arg->flags & FLAG_T)
		return va_arg(*arg->va_arg, ptrdiff_t);
	return va_arg(*arg->va_arg, int);
}

static unsigned long long int get_uint_val(struct arg *arg)
{
	if (arg->flags & FLAG_LL)
		return va_arg(*arg->va_arg, unsigned long long);
	if (arg->flags & FLAG_L)
		return va_arg(*arg->va_arg, unsigned long);
	if (arg->flags & FLAG_HH)
		return (unsigned char)va_arg(*arg->va_arg, unsigned long);
	if (arg->flags & FLAG_H)
		return (unsigned short)va_arg(*arg->va_arg, unsigned long);
	if (arg->flags & FLAG_J)
		return va_arg(*arg->va_arg, uintmax_t);
	if (arg->flags & FLAG_Z)
		return va_arg(*arg->va_arg, size_t);
	 if (arg->flags & FLAG_T)
		return va_arg(*arg->va_arg, ptrdiff_t);
	return va_arg(*arg->va_arg, unsigned);
}

static ssize_t printf_buf(struct buf *buf, const char *fmt, va_list va_arg)
{
	va_list va_cpy;
	va_copy(va_cpy, va_arg);
	buf->len = 0;
	size_t first_nonfmt = 0;
	size_t i;
	for (i = 0; fmt[i]; ++i)
	{
		if (fmt[i] != '%')
			continue;
		if (first_nonfmt != i)
			outstr(buf, &fmt[first_nonfmt], i - first_nonfmt);
		struct arg arg;
		i++;
		arg_ctr(&arg, &va_cpy);
		parse_arg(&arg, fmt, &i);
		int ret;
		switch (arg.type)
		{
			case 'c':
				ret = print_c(buf, &arg);
				break;
			case 'd':
			case 'i':
				ret = print_d(buf, &arg);
				break;
			case 'o':
				ret = print_o(buf, &arg);
				break;
			case 's':
				ret = print_s(buf, &arg);
				break;
			case 'u':
				ret = print_u(buf, &arg);
				break;
			case 'x':
			case 'X':
				ret = print_x(buf, &arg);
				break;
			case 'p':
				ret = print_p(buf, &arg);
				break;
			case '%':
				ret = print_mod(buf, &arg);
				break;
			default:
				continue;
		}
		if (ret)
		{
			va_end(va_cpy);
			return ret;
		}
		first_nonfmt = i + 1;
	}
	if (first_nonfmt != i)
		outstr(buf, &fmt[first_nonfmt], i - first_nonfmt);
	va_end(va_cpy);
	return buf->len;
}

ssize_t vprintf(const char *fmt, va_list va_arg)
{
	struct buf buf;
	buf.type = PRINTF_TTY;
	return printf_buf(&buf, fmt, va_arg);
}

ssize_t printf(const char *fmt, ...)
{
	va_list va_arg;
	va_start(va_arg, fmt);
	ssize_t ret = vprintf(fmt, va_arg);
	va_end(va_arg);
	return ret;
}

ssize_t vsnprintf(char *d, size_t n, const char *fmt, va_list va_arg)
{
	struct buf buf;
	buf.type = PRINTF_BUF;
	buf.buf.data = d;
	buf.buf.size = n;
	ssize_t ret = printf_buf(&buf, fmt, va_arg);
	if (n)
	{
		if (buf.len < n)
			d[buf.len] = '\0';
		else
			d[n - 1] = '\0';
	}
	return ret;
}

ssize_t snprintf(char *d, size_t n, const char *fmt, ...)
{
	va_list va_arg;
	va_start(va_arg, fmt);
	ssize_t ret = vsnprintf(d, n, fmt, va_arg);
	va_end(va_arg);
	return ret;
}

ssize_t vuprintf(struct uio *uio, const char *fmt, va_list va_arg)
{
	if (!uio)
		return vprintf(fmt, va_arg);
	struct buf buf;
	buf.type = PRINTF_UIO;
	buf.uio.uio = uio;
	buf.uio.buf_pos = 0;
	ssize_t ret = printf_buf(&buf, fmt, va_arg);
	if (ret < 0)
		return ret;
	if (buf.uio.buf_pos)
	{
		ssize_t n = uio_copyin(buf.uio.uio, buf.uio.buf, buf.uio.buf_pos);
		buf.uio.uio->off = 0;
		if (n < 0)
			return n;
		ret += n;
	}
	return ret;
}

ssize_t uprintf(struct uio *uio, const char *fmt, ...)
{
	va_list va_arg;
	va_start(va_arg, fmt);
	ssize_t ret = vuprintf(uio, fmt, va_arg);
	va_end(va_arg);
	return ret;
}

static int parse_flags(struct arg *arg, char c)
{
	if (c == '-')
		arg->flags |= FLAG_MINUS;
	else if (c == '+')
		arg->flags |= FLAG_PLUS;
	else if (c == '0')
		arg->flags |= FLAG_ZERO;
	else if (c == '#')
		arg->flags |= FLAG_SHARP;
	else if (c == ' ')
		arg->flags |= FLAG_SPACE;
	else
		return 0;
	return 1;
}

static int parse_preci(struct arg *arg, const char *fmt, size_t *i)
{
	size_t start;
	size_t end;

	if (fmt[*i] != '.')
		return 1;
	(*i)++;
	if (fmt[*i] == '*')
	{
		arg->preci = va_arg(*arg->va_arg, int);
		(*i)++;
		return 1;
	}
	start = *i;
	while (isdigit(fmt[*i]))
		(*i)++;
	end = *i;
	if (end == start)
		arg->preci = 0;
	else
		arg->preci = atoin(&fmt[start], end - start);
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
	size_t start;
	size_t end;

	if (fmt[*i] == '*')
	{
		arg->width = va_arg(*arg->va_arg, int);
		(*i)++;
		return 1;
	}
	start = *i;
	while (isdigit(fmt[*i]))
		(*i)++;
	end = *i;
	if (end == start)
		return 1;
	arg->width = atoin(&fmt[start], end - start);
	return 1;
}

static int parse_arg(struct arg *arg, const char *fmt, size_t *i)
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

static int print_str(struct buf *buf, struct arg *arg, const char *prefix,
                     const char *s, size_t len)
{
	size_t prefix_len = prefix ? strlen(prefix) : 0;
	size_t pad_len;
	int ret;

	if (arg->width > 0 && (size_t)arg->width > len + prefix_len)
		pad_len = arg->width - len - prefix_len;
	else
		pad_len = 0;
	if (pad_len && !(arg->flags & (FLAG_ZERO | FLAG_MINUS)))
	{
		ret = outchars(buf, ' ', pad_len);
		if (ret)
			return ret;
	}
	if (prefix)
	{
		ret = outstr(buf, prefix, strlen(prefix));
		if (ret)
			return ret;
	}
	if (pad_len && (arg->flags & FLAG_ZERO))
	{
		ret = outchars(buf, '0', pad_len);
		if (ret)
			return ret;
	}
	ret = outstr(buf, s, len);
	if (ret)
		return ret;
	if (pad_len && (arg->flags & FLAG_MINUS))
	{
		ret = outchars(buf, ' ', pad_len);
		if (ret)
			return ret;
	}
	return 0;
}

static int print_nbr(struct buf *buf, struct arg *arg, const char *prefix,
                     const char *s, size_t len)
{
	size_t prefix_len = prefix ? strlen(prefix) : 0;
	size_t preci_len;
	size_t pad_len;
	int ret;

	preci_len = arg->preci >= 0 && (size_t)arg->preci > len
	          ? arg->preci - len : 0;
	if (preci_len && prefix && prefix[0] == '0' && prefix[1] == '\0')
		preci_len--;
	if (arg->width > 0 && (size_t)arg->width > len + prefix_len + preci_len)
		pad_len = arg->width - len - prefix_len - preci_len;
	else
		pad_len = 0;
	if (pad_len && !(arg->flags & (FLAG_ZERO | FLAG_MINUS)))
	{
		ret = outchars(buf, ' ', pad_len);
		if (ret)
			return ret;
	}
	if (prefix)
	{
		ret = outstr(buf, prefix, strlen(prefix));
		if (ret)
			return ret;
	}
	if (pad_len && (arg->flags & FLAG_ZERO))
	{
		ret = outchars(buf, '0', pad_len);
		if (ret)
			return ret;
	}
	ret = outchars(buf, '0', preci_len);
	if (ret)
		return ret;
	ret = outstr(buf, s, len);
	if (ret)
		return ret;
	if (pad_len && (arg->flags & FLAG_MINUS))
	{
		ret = outchars(buf, ' ', pad_len);
		if (ret)
			return ret;
	}
	return 0;
}

static void ulltoa(char *d, unsigned long long int n, const char *base)
{
	size_t size;
	size_t base_len;
	size_t i;
	unsigned long long int nb;

	if (!n)
	{
		strcpy(d, "0");
		return;
	}
	nb = n;
	base_len = strlen(base);
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
	d[size - 1] = '\0';
}

static void lltoa(char *d, long long n, const char *base)
{
	if (n < 0)
	{
		d[0] = '-';
		if (n == LLONG_MIN)
			ulltoa(&d[1], (unsigned long long)LLONG_MAX + 1, base);
		else
			ulltoa(&d[1], -n, base);
	}
	else
	{
		ulltoa(d, n, base);
	}
}

static int print_c(struct buf *buf, struct arg *arg)
{
	uint8_t v;

	v = va_arg(*arg->va_arg, int);
	return print_str(buf, arg, NULL, (char*)&v, 1);
}

static int print_d(struct buf *buf, struct arg *arg)
{
	long long int val;
	char str[64];

	val = get_int_val(arg);
	lltoa(str, val, "0123456789");
	return print_nbr(buf, arg, NULL, str, strlen(str));
}

static int print_o(struct buf *buf, struct arg *arg)
{
	char str[64];
	unsigned long long int val;
	const char *prefix;

	val = get_uint_val(arg);
	ulltoa(str, val, "01234567");
	if (val && (arg->flags & FLAG_SHARP))
		prefix = "0";
	else
		prefix = NULL;
	return print_nbr(buf, arg, prefix, str, strlen(str));
}

static int print_s(struct buf *buf, struct arg *arg)
{
	char *str;
	size_t len;

	str = va_arg(*arg->va_arg, char*);
	if (!str)
		str = "(null)";
	len = strlen(str);
	if (arg->preci >= 0 && (size_t)arg->preci < len)
		len = arg->preci;
	return print_str(buf, arg, NULL, str, len);
}

static int print_u(struct buf *buf, struct arg *arg)
{
	char str[64];
	unsigned long long int val;

	val = get_uint_val(arg);
	ulltoa(str, val, "0123456789");
	return print_nbr(buf, arg, NULL, str, strlen(str));
}

static char *get_x_chars(struct arg *arg)
{
	if (arg->type == 'X')
		return "0123456789ABCDEF";
	return "0123456789abcdef";
}

static int print_x(struct buf *buf, struct arg *arg)
{
	char str[64];
	unsigned long long int val;
	const char *prefix;

	val = get_uint_val(arg);
	ulltoa(str, val, get_x_chars(arg));
	if (val && (arg->flags & FLAG_SHARP))
		prefix = (arg->type == 'X' ? "0X" : "0x");
	else
		prefix = NULL;
	return print_nbr(buf, arg, prefix, str, strlen(str));
}

static int print_p(struct buf *buf, struct arg *arg)
{
	char str[64];
	unsigned long long int val;
	const char *prefix;

	arg->flags |= FLAG_L;
	val = get_uint_val(arg);
	if (!val)
	{
		arg->preci = -1;
		return print_str(buf, arg, NULL, "(nil)", 5);
	}
	arg->flags |= FLAG_SHARP;
	arg->flags &= ~(FLAG_LL | FLAG_H | FLAG_HH | FLAG_Z | FLAG_J | FLAG_T);
	ulltoa(str, val, get_x_chars(arg));
	if (val && (arg->flags & FLAG_SHARP))
		prefix = (arg->type == 'X' ? "0X" : "0x");
	else
		prefix = NULL;
	return print_nbr(buf, arg, prefix, str, strlen(str));
}

static int print_mod(struct buf *buf, struct arg *arg)
{
	(void)arg;
	return outchar(buf, '%');
}
