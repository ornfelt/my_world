#include <string.h>
#include <wchar.h>
#include <stdio.h>
#include <time.h>

struct strftime_ctx
{
	wchar_t *data;
	size_t len;
	size_t size;
	const struct tm *tm;
};

static void outchar(struct strftime_ctx *ctx, wchar_t c)
{
	if (ctx->len < ctx->size)
		ctx->data[ctx->len] = c;
	ctx->len++;
}

static void outstr(struct strftime_ctx *ctx, const wchar_t *s)
{
	for (size_t i = 0; s[i]; ++i)
		outchar(ctx, s[i]);
}

static void print_digit(struct strftime_ctx *ctx, int n)
{
	outchar(ctx, L'0' + n);
}

static void print_twodigits(struct strftime_ctx *ctx, int n, wchar_t pad)
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

static void print_mod(struct strftime_ctx *ctx)
{
	outchar(ctx, L'%');
}

static void print_a(struct strftime_ctx *ctx)
{
	static const wchar_t *strings[] =
	{
		L"Sun",
		L"Mon",
		L"Tue",
		L"Wed",
		L"Thu",
		L"Fri",
		L"Sat",
	};
	if (ctx->tm->tm_wday < 0 || ctx->tm->tm_wday > 6)
		return;
	outstr(ctx, strings[ctx->tm->tm_wday]);
}

static void print_A(struct strftime_ctx *ctx)
{
	static const wchar_t *strings[] =
	{
		L"Sunday",
		L"Monday",
		L"Tuesday",
		L"Wednesday",
		L"Thursday",
		L"Friday",
		L"Saturday",
	};
	if (ctx->tm->tm_wday < 0 || ctx->tm->tm_wday > 6)
		return;
	outstr(ctx, strings[ctx->tm->tm_wday]);
}

static void print_b(struct strftime_ctx *ctx)
{
	static const wchar_t *strings[] =
	{
		L"Jan",
		L"Feb",
		L"Mar",
		L"Apr",
		L"May",
		L"Jun",
		L"Jul",
		L"Aug",
		L"Sep",
		L"Oct",
		L"Nov",
		L"Dec",
	};
	if (ctx->tm->tm_mon < 0 || ctx->tm->tm_mon > 11)
		return;
	outstr(ctx, strings[ctx->tm->tm_mon]);
}

static void print_B(struct strftime_ctx *ctx)
{
	static const wchar_t *strings[] =
	{
		L"January",
		L"February",
		L"March",
		L"April",
		L"May",
		L"June",
		L"July",
		L"August",
		L"September",
		L"October",
		L"November",
		L"December",
	};
	if (ctx->tm->tm_mon < 0 || ctx->tm->tm_mon > 11)
		return;
	outstr(ctx, strings[ctx->tm->tm_mon]);
}

static void print_c(struct strftime_ctx *ctx)
{
	print_a(ctx);
	outchar(ctx, L' ');
	print_b(ctx);
	outchar(ctx, L' ');
	print_d(ctx);
	outchar(ctx, L' ');
	print_T(ctx);
	outchar(ctx, L' ');
	print_Y(ctx);
}

static void print_C(struct strftime_ctx *ctx)
{
	int century = 19 + ctx->tm->tm_year / 100;
	print_twodigits(ctx, century, L'0');
}

static void print_d(struct strftime_ctx *ctx)
{
	if (ctx->tm->tm_mday < 1 || ctx->tm->tm_mday > 31)
		return;
	print_twodigits(ctx, ctx->tm->tm_mday, L'0');
}

static void print_D(struct strftime_ctx *ctx)
{
	print_m(ctx);
	outchar(ctx, L'/');
	print_d(ctx);
	outchar(ctx, L'/');
	print_y(ctx);
}

static void print_e(struct strftime_ctx *ctx)
{
	if (ctx->tm->tm_mday < 1 || ctx->tm->tm_mday > 31)
		return;
	print_twodigits(ctx, ctx->tm->tm_mday, L' ');
}

static void print_F(struct strftime_ctx *ctx)
{
	print_Y(ctx);
	outchar(ctx, L'-');
	print_m(ctx);
	outchar(ctx, L'-');
	print_d(ctx);
}

static void print_G(struct strftime_ctx *ctx)
{
	(void)ctx;
	/* XXX */
}

static void print_g(struct strftime_ctx *ctx)
{
	(void)ctx;
	/* XXX */
}

static void print_h(struct strftime_ctx *ctx)
{
	print_b(ctx);
}

static void print_H(struct strftime_ctx *ctx)
{
	if (ctx->tm->tm_hour < 0 || ctx->tm->tm_hour > 23)
		return;
	print_twodigits(ctx, ctx->tm->tm_hour, L'0');
}

static void print_I(struct strftime_ctx *ctx)
{
	if (ctx->tm->tm_hour < 0 || ctx->tm->tm_hour > 23)
		return;
	print_twodigits(ctx, ctx->tm->tm_hour % 12, L'0');
}

static void print_j(struct strftime_ctx *ctx)
{
	if (ctx->tm->tm_yday < 0 || ctx->tm->tm_yday > 366)
		return;
	int yday = ctx->tm->tm_yday + 1;
	if (yday >= 100)
		outchar(ctx, L'0' + yday / 100);
	else
		outchar(ctx, L'0');
	if (ctx->tm->tm_yday >= 10)
		outchar(ctx, L'0' + (yday / 10) % 10);
	else
		outchar(ctx, L'0');
	outchar(ctx, L'0' + yday % 10);
}

static void print_k(struct strftime_ctx *ctx)
{
	if (ctx->tm->tm_hour < 0 || ctx->tm->tm_hour > 23)
		return;
	print_twodigits(ctx, ctx->tm->tm_hour, L' ');
}

static void print_l(struct strftime_ctx *ctx)
{
	if (ctx->tm->tm_hour < 0 || ctx->tm->tm_hour > 23)
		return;
	print_twodigits(ctx, ctx->tm->tm_hour % 12, L' ');
}

static void print_m(struct strftime_ctx *ctx)
{
	if (ctx->tm->tm_mon < 0 || ctx->tm->tm_mon > 11)
		return;
	print_twodigits(ctx, ctx->tm->tm_mon + 1, L'0');
}

static void print_M(struct strftime_ctx *ctx)
{
	if (ctx->tm->tm_min < 0 || ctx->tm->tm_min > 59)
		return;
	print_twodigits(ctx, ctx->tm->tm_min, L'0');
}

static void print_n(struct strftime_ctx *ctx)
{
	outchar(ctx, L'\n');
}

static void print_p(struct strftime_ctx *ctx)
{
	if (ctx->tm->tm_hour < 12)
		outstr(ctx, L"AM");
	else
		outstr(ctx, L"PM");
}

static void print_P(struct strftime_ctx *ctx)
{
	if (ctx->tm->tm_hour < 12)
		outstr(ctx, L"am");
	else
		outstr(ctx, L"pm");
}

static void print_r(struct strftime_ctx *ctx)
{
	print_I(ctx);
	outchar(ctx, L':');
	print_M(ctx);
	outchar(ctx, L':');
	print_S(ctx);
	outchar(ctx, L' ');
	print_p(ctx);
}

static void print_R(struct strftime_ctx *ctx)
{
	print_H(ctx);
	outchar(ctx, L':');
	print_M(ctx);
}

static void print_s(struct strftime_ctx *ctx)
{
	wchar_t str[64];
	struct tm tm;
	memcpy(&tm, ctx->tm, sizeof(tm));
	swprintf(str, sizeof(str), L"%lld", (unsigned long long)mktime(&tm));
	outstr(ctx, str);
}

static void print_S(struct strftime_ctx *ctx)
{
	print_twodigits(ctx, ctx->tm->tm_sec, L'0');
}

static void print_t(struct strftime_ctx *ctx)
{
	outchar(ctx, L'\t');
}

static void print_T(struct strftime_ctx *ctx)
{
	print_H(ctx);
	outchar(ctx, L':');
	print_M(ctx);
	outchar(ctx, L':');
	print_S(ctx);
}

static void print_u(struct strftime_ctx *ctx)
{
	if (ctx->tm->tm_wday < 0 || ctx->tm->tm_wday > 6)
		return;
	if (ctx->tm->tm_wday == 0)
		print_digit(ctx, 7);
	else
		print_digit(ctx, ctx->tm->tm_wday);
}

static void print_U(struct strftime_ctx *ctx)
{
	(void)ctx;
	/* XXX */
}

static void print_V(struct strftime_ctx *ctx)
{
	(void)ctx;
	/* XXX */
}

static void print_w(struct strftime_ctx *ctx)
{
	if (ctx->tm->tm_wday < 0 || ctx->tm->tm_wday > 6)
		return;
	print_digit(ctx, ctx->tm->tm_wday);
}

static void print_W(struct strftime_ctx *ctx)
{
	(void)ctx;
	/* XXX */
}

static void print_x(struct strftime_ctx *ctx)
{
	print_m(ctx);
	outchar(ctx, L'/');
	print_d(ctx);
	outchar(ctx, L'/');
	print_y(ctx);
}

static void print_X(struct strftime_ctx *ctx)
{
	print_T(ctx);
}

static void print_y(struct strftime_ctx *ctx)
{
	print_twodigits(ctx, ctx->tm->tm_year % 100, L'0');
}

static void print_Y(struct strftime_ctx *ctx)
{
	int year = 1900 + ctx->tm->tm_year;
	print_digit(ctx, year / 1000);
	print_digit(ctx, (year / 100) % 10);
	print_digit(ctx, (year / 10) % 10);
	print_digit(ctx, year % 10);
}

static void print_z(struct strftime_ctx *ctx)
{
	(void)ctx;
	/* XXX */
}

static void print_Z(struct strftime_ctx *ctx)
{
	(void)ctx;
	/* XXX */
}

size_t wcsftime(wchar_t *s, size_t max, const wchar_t *format,
                const struct tm *tm)
{
	struct strftime_ctx ctx;
	ctx.data = s;
	ctx.len = 0;
	ctx.size = max;
	ctx.tm = tm;
	for (size_t i = 0; format[i]; ++i)
	{
		if (format[i] != L'%')
		{
			outchar(&ctx, format[i]);
			continue;
		}
		i++;
		switch (format[i])
		{
			case L'%':
				print_mod(&ctx);
				break;
			case L'a':
				print_a(&ctx);
				break;
			case L'A':
				print_A(&ctx);
				break;
			case L'b':
				print_b(&ctx);
				break;
			case L'B':
				print_B(&ctx);
				break;
			case L'c':
				print_c(&ctx);
				break;
			case L'C':
				print_C(&ctx);
				break;
			case L'd':
				print_d(&ctx);
				break;
			case L'D':
				print_D(&ctx);
				break;
			case L'e':
				print_e(&ctx);
				break;
			case L'F':
				print_F(&ctx);
				break;
			case L'g':
				print_g(&ctx);
				break;
			case L'G':
				print_G(&ctx);
				break;
			case L'h':
				print_h(&ctx);
				break;
			case L'H':
				print_H(&ctx);
				break;
			case L'I':
				print_I(&ctx);
				break;
			case L'j':
				print_j(&ctx);
				break;
			case L'k':
				print_k(&ctx);
				break;
			case L'l':
				print_l(&ctx);
				break;
			case L'm':
				print_m(&ctx);
				break;
			case L'M':
				print_M(&ctx);
				break;
			case L'n':
				print_n(&ctx);
				break;
			case L'p':
				print_p(&ctx);
				break;
			case L'P':
				print_P(&ctx);
				break;
			case L'r':
				print_r(&ctx);
				break;
			case L'R':
				print_R(&ctx);
				break;
			case L's':
				print_s(&ctx);
				break;
			case L'S':
				print_S(&ctx);
				break;
			case L't':
				print_t(&ctx);
				break;
			case L'T':
				print_T(&ctx);
				break;
			case L'u':
				print_u(&ctx);
				break;
			case L'U':
				print_U(&ctx);
				break;
			case L'V':
				print_V(&ctx);
				break;
			case L'w':
				print_w(&ctx);
				break;
			case L'W':
				print_W(&ctx);
				break;
			case L'x':
				print_x(&ctx);
				break;
			case L'X':
				print_X(&ctx);
				break;
			case L'y':
				print_y(&ctx);
				break;
			case L'Y':
				print_Y(&ctx);
				break;
			case L'z':
				print_z(&ctx);
				break;
			case L'Z':
				print_Z(&ctx);
				break;
		}
	}
	if (ctx.size)
	{
		if (ctx.len < ctx.size)
			ctx.data[ctx.len] = L'\0';
		else
			ctx.data[ctx.size - 1] = L'\0';
	}
	if (ctx.len + 1 > ctx.size)
		return 0;
	return ctx.len + 1;
}
