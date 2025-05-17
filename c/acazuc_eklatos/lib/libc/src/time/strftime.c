#include <string.h>
#include <stdio.h>
#include <time.h>

struct strftime_ctx
{
	char *data;
	size_t len;
	size_t size;
	const struct tm *tm;
};

static void
outchar(struct strftime_ctx *ctx, char c)
{
	if (ctx->len < ctx->size)
		ctx->data[ctx->len] = c;
	ctx->len++;
}

static void
outstr(struct strftime_ctx *ctx, const char *s)
{
	for (size_t i = 0; s[i]; ++i)
		outchar(ctx, s[i]);
}

static void
print_digit(struct strftime_ctx *ctx, int n)
{
	outchar(ctx, '0' + n);
}

static void
print_twodigits(struct strftime_ctx *ctx, int n, char pad)
{
	if (n >= 10)
		print_digit(ctx, n / 10);
	else
		outchar(ctx, pad);
	print_digit(ctx, n % 10);
}

static void print_d(struct strftime_ctx *ctx);
static void print_m(struct strftime_ctx *ctx);
static void print_S(struct strftime_ctx *ctx);
static void print_T(struct strftime_ctx *ctx);
static void print_y(struct strftime_ctx *ctx);
static void print_Y(struct strftime_ctx *ctx);

static void
print_mod(struct strftime_ctx *ctx)
{
	outchar(ctx, '%');
}

static void
print_a(struct strftime_ctx *ctx)
{
	static const char *strings[] =
	{
		"Sun",
		"Mon",
		"Tue",
		"Wed",
		"Thu",
		"Fri",
		"Sat",
	};
	if (ctx->tm->tm_wday < 0 || ctx->tm->tm_wday > 6)
		return;
	outstr(ctx, strings[ctx->tm->tm_wday]);
}

static void
print_A(struct strftime_ctx *ctx)
{
	static const char *strings[] =
	{
		"Sunday",
		"Monday",
		"Tuesday",
		"Wednesday",
		"Thursday",
		"Friday",
		"Saturday",
	};
	if (ctx->tm->tm_wday < 0 || ctx->tm->tm_wday > 6)
		return;
	outstr(ctx, strings[ctx->tm->tm_wday]);
}

static void
print_b(struct strftime_ctx *ctx)
{
	static const char *strings[] =
	{
		"Jan",
		"Feb",
		"Mar",
		"Apr",
		"May",
		"Jun",
		"Jul",
		"Aug",
		"Sep",
		"Oct",
		"Nov",
		"Dec",
	};
	if (ctx->tm->tm_mon < 0 || ctx->tm->tm_mon > 11)
		return;
	outstr(ctx, strings[ctx->tm->tm_mon]);
}

static void
print_B(struct strftime_ctx *ctx)
{
	static const char *strings[] =
	{
		"January",
		"February",
		"March",
		"April",
		"May",
		"June",
		"July",
		"August",
		"September",
		"October",
		"November",
		"December",
	};
	if (ctx->tm->tm_mon < 0 || ctx->tm->tm_mon > 11)
		return;
	outstr(ctx, strings[ctx->tm->tm_mon]);
}

static void
print_c(struct strftime_ctx *ctx)
{
	print_a(ctx);
	outchar(ctx, ' ');
	print_b(ctx);
	outchar(ctx, ' ');
	print_d(ctx);
	outchar(ctx, ' ');
	print_T(ctx);
	outchar(ctx, ' ');
	print_Y(ctx);
}

static void
print_C(struct strftime_ctx *ctx)
{
	int century = 19 + ctx->tm->tm_year / 100;
	print_twodigits(ctx, century, '0');
}

static void
print_d(struct strftime_ctx *ctx)
{
	if (ctx->tm->tm_mday < 1 || ctx->tm->tm_mday > 31)
		return;
	print_twodigits(ctx, ctx->tm->tm_mday, '0');
}

static void
print_D(struct strftime_ctx *ctx)
{
	print_m(ctx);
	outchar(ctx, '/');
	print_d(ctx);
	outchar(ctx, '/');
	print_y(ctx);
}

static void
print_e(struct strftime_ctx *ctx)
{
	if (ctx->tm->tm_mday < 1 || ctx->tm->tm_mday > 31)
		return;
	print_twodigits(ctx, ctx->tm->tm_mday, ' ');
}

static void
print_F(struct strftime_ctx *ctx)
{
	print_Y(ctx);
	outchar(ctx, '-');
	print_m(ctx);
	outchar(ctx, '-');
	print_d(ctx);
}

static void
print_G(struct strftime_ctx *ctx)
{
	(void)ctx;
	/* XXX */
}

static void
print_g(struct strftime_ctx *ctx)
{
	(void)ctx;
	/* XXX */
}

static void
print_h(struct strftime_ctx *ctx)
{
	print_b(ctx);
}

static void
print_H(struct strftime_ctx *ctx)
{
	if (ctx->tm->tm_hour < 0 || ctx->tm->tm_hour > 23)
		return;
	print_twodigits(ctx, ctx->tm->tm_hour, '0');
}

static void
print_I(struct strftime_ctx *ctx)
{
	if (ctx->tm->tm_hour < 0 || ctx->tm->tm_hour > 23)
		return;
	print_twodigits(ctx, ctx->tm->tm_hour % 12, '0');
}

static void
print_j(struct strftime_ctx *ctx)
{
	if (ctx->tm->tm_yday < 0 || ctx->tm->tm_yday > 366)
		return;
	int yday = ctx->tm->tm_yday + 1;
	if (yday >= 100)
		outchar(ctx, '0' + yday / 100);
	else
		outchar(ctx, '0');
	if (ctx->tm->tm_yday >= 10)
		outchar(ctx, '0' + (yday / 10) % 10);
	else
		outchar(ctx, '0');
	outchar(ctx, '0' + yday % 10);
}

static void
print_k(struct strftime_ctx *ctx)
{
	if (ctx->tm->tm_hour < 0 || ctx->tm->tm_hour > 23)
		return;
	print_twodigits(ctx, ctx->tm->tm_hour, ' ');
}

static void
print_l(struct strftime_ctx *ctx)
{
	if (ctx->tm->tm_hour < 0 || ctx->tm->tm_hour > 23)
		return;
	print_twodigits(ctx, ctx->tm->tm_hour % 12, ' ');
}

static void
print_m(struct strftime_ctx *ctx)
{
	if (ctx->tm->tm_mon < 0 || ctx->tm->tm_mon > 11)
		return;
	print_twodigits(ctx, ctx->tm->tm_mon + 1, '0');
}

static void
print_M(struct strftime_ctx *ctx)
{
	if (ctx->tm->tm_min < 0 || ctx->tm->tm_min > 59)
		return;
	print_twodigits(ctx, ctx->tm->tm_min, '0');
}

static void
print_n(struct strftime_ctx *ctx)
{
	outchar(ctx, '\n');
}

static void
print_p(struct strftime_ctx *ctx)
{
	if (ctx->tm->tm_hour < 12)
		outstr(ctx, "AM");
	else
		outstr(ctx, "PM");
}

static void
print_P(struct strftime_ctx *ctx)
{
	if (ctx->tm->tm_hour < 12)
		outstr(ctx, "am");
	else
		outstr(ctx, "pm");
}

static void
print_r(struct strftime_ctx *ctx)
{
	print_I(ctx);
	outchar(ctx, ':');
	print_M(ctx);
	outchar(ctx, ':');
	print_S(ctx);
	outchar(ctx, ' ');
	print_p(ctx);
}

static void
print_R(struct strftime_ctx *ctx)
{
	print_H(ctx);
	outchar(ctx, ':');
	print_M(ctx);
}

static void
print_s(struct strftime_ctx *ctx)
{
	char str[64];
	struct tm tm;

	memcpy(&tm, ctx->tm, sizeof(tm));
	snprintf(str, sizeof(str), "%lld", (unsigned long long)mktime(&tm));
	outstr(ctx, str);
}

static void
print_S(struct strftime_ctx *ctx)
{
	print_twodigits(ctx, ctx->tm->tm_sec, '0');
}

static void
print_t(struct strftime_ctx *ctx)
{
	outchar(ctx, '\t');
}

static void
print_T(struct strftime_ctx *ctx)
{
	print_H(ctx);
	outchar(ctx, ':');
	print_M(ctx);
	outchar(ctx, ':');
	print_S(ctx);
}

static void
print_u(struct strftime_ctx *ctx)
{
	if (ctx->tm->tm_wday < 0 || ctx->tm->tm_wday > 6)
		return;
	if (ctx->tm->tm_wday == 0)
		print_digit(ctx, 7);
	else
		print_digit(ctx, ctx->tm->tm_wday);
}

static void
print_U(struct strftime_ctx *ctx)
{
	(void)ctx;
	/* XXX */
}

static void
print_V(struct strftime_ctx *ctx)
{
	(void)ctx;
	/* XXX */
}

static void
print_w(struct strftime_ctx *ctx)
{
	if (ctx->tm->tm_wday < 0 || ctx->tm->tm_wday > 6)
		return;
	print_digit(ctx, ctx->tm->tm_wday);
}

static void
print_W(struct strftime_ctx *ctx)
{
	(void)ctx;
	/* XXX */
}

static void
print_x(struct strftime_ctx *ctx)
{
	print_m(ctx);
	outchar(ctx, '/');
	print_d(ctx);
	outchar(ctx, '/');
	print_y(ctx);
}

static void
print_X(struct strftime_ctx *ctx)
{
	print_T(ctx);
}

static void
print_y(struct strftime_ctx *ctx)
{
	print_twodigits(ctx, ctx->tm->tm_year % 100, '0');
}

static void
print_Y(struct strftime_ctx *ctx)
{
	int year = 1900 + ctx->tm->tm_year;
	print_digit(ctx, year / 1000);
	print_digit(ctx, (year / 100) % 10);
	print_digit(ctx, (year / 10) % 10);
	print_digit(ctx, year % 10);
}

static void
print_z(struct strftime_ctx *ctx)
{
	(void)ctx;
	/* XXX */
}

static void
print_Z(struct strftime_ctx *ctx)
{
	(void)ctx;
	/* XXX */
}

size_t
strftime(char *s, size_t max, const char *format, const struct tm *tm)
{
	struct strftime_ctx ctx;
	ctx.data = s;
	ctx.len = 0;
	ctx.size = max;
	ctx.tm = tm;
	for (size_t i = 0; format[i]; ++i)
	{
		if (format[i] != '%')
		{
			outchar(&ctx, format[i]);
			continue;
		}
		i++;
		switch (format[i])
		{
			case '%':
				print_mod(&ctx);
				break;
			case 'a':
				print_a(&ctx);
				break;
			case 'A':
				print_A(&ctx);
				break;
			case 'b':
				print_b(&ctx);
				break;
			case 'B':
				print_B(&ctx);
				break;
			case 'c':
				print_c(&ctx);
				break;
			case 'C':
				print_C(&ctx);
				break;
			case 'd':
				print_d(&ctx);
				break;
			case 'D':
				print_D(&ctx);
				break;
			case 'e':
				print_e(&ctx);
				break;
			case 'F':
				print_F(&ctx);
				break;
			case 'g':
				print_g(&ctx);
				break;
			case 'G':
				print_G(&ctx);
				break;
			case 'h':
				print_h(&ctx);
				break;
			case 'H':
				print_H(&ctx);
				break;
			case 'I':
				print_I(&ctx);
				break;
			case 'j':
				print_j(&ctx);
				break;
			case 'k':
				print_k(&ctx);
				break;
			case 'l':
				print_l(&ctx);
				break;
			case 'm':
				print_m(&ctx);
				break;
			case 'M':
				print_M(&ctx);
				break;
			case 'n':
				print_n(&ctx);
				break;
			case 'p':
				print_p(&ctx);
				break;
			case 'P':
				print_P(&ctx);
				break;
			case 'r':
				print_r(&ctx);
				break;
			case 'R':
				print_R(&ctx);
				break;
			case 's':
				print_s(&ctx);
				break;
			case 'S':
				print_S(&ctx);
				break;
			case 't':
				print_t(&ctx);
				break;
			case 'T':
				print_T(&ctx);
				break;
			case 'u':
				print_u(&ctx);
				break;
			case 'U':
				print_U(&ctx);
				break;
			case 'V':
				print_V(&ctx);
				break;
			case 'w':
				print_w(&ctx);
				break;
			case 'W':
				print_W(&ctx);
				break;
			case 'x':
				print_x(&ctx);
				break;
			case 'X':
				print_X(&ctx);
				break;
			case 'y':
				print_y(&ctx);
				break;
			case 'Y':
				print_Y(&ctx);
				break;
			case 'z':
				print_z(&ctx);
				break;
			case 'Z':
				print_Z(&ctx);
				break;
		}
	}
	if (ctx.size)
	{
		if (ctx.len < ctx.size)
			ctx.data[ctx.len] = '\0';
		else
			ctx.data[ctx.size - 1] = '\0';
	}
	if (ctx.len + 1 > ctx.size)
		return 0;
	return ctx.len + 1;
}
