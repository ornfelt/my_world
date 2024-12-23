#include <jkssl/bignum.h>

#include <string.h>
#include <stddef.h>
#include <ctype.h>

int bignum_dec2bignum(struct bignum *bignum, const char *s)
{
	struct bignum_ctx *ctx;
	struct bignum tmp;
	struct bignum mul;
	size_t s_len;
	int ret = 0;

	bignum_init(&tmp);
	bignum_init(&mul);
	ctx = bignum_ctx_new();
	if (!ctx)
		goto end;
	if (!bignum_grow(&mul, 10))
		goto end;
	bignum_zero(bignum);
	s_len = strlen(s);
	for (size_t i = s[0] == '-' ? 1 : 0; i < s_len; ++i)
	{
		if (!bignum_mul(bignum, bignum, &mul, ctx))
			goto end;
		if (!isdigit(s[i]))
			goto end;
		if (!bignum_set_word(&tmp, s[i] - '0'))
			goto end;
		if (!bignum_add(bignum, bignum, &tmp, ctx))
			goto end;
	}
	bignum_set_negative(bignum, s[0] == '-');
	ret = 1;

end:
	bignum_clear(&tmp);
	bignum_clear(&mul);
	bignum_ctx_free(ctx);
	return ret;
}

int bignum_bignum2bin(const struct bignum *a, uint8_t *to)
{
	int bytes = bignum_num_bytes(a);
	for (int i = 0; i < bytes; ++i)
		to[i] = ((uint8_t*)a->data)[bytes - 1 - i];
	return bytes;
}

struct bignum *bignum_bin2bignum(const uint8_t *s, int len, struct bignum *ret)
{
	if (!ret)
	{
		ret = bignum_new();
		if (!ret)
			return NULL;
	}
	if (!bignum_resize(ret, (len + sizeof(*ret->data) - 1) / sizeof(*ret->data)))
		return NULL;
	for (int i = 0; i < len; ++i)
		((uint8_t*)ret->data)[len - 1 - i] = s[i];
	return ret;
}

static int get_val(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return 0;
}

static int do_fill(struct bignum *bignum, const char *s, int len)
{
	if (!bignum_grow(bignum, 0))
		return 0;
	for (int i = 0; i < len; ++i)
	{
		if ((s[i] < '0' || s[i] > '9')
		 && (s[i] < 'a' || s[i] > 'f')
		 && (s[i] < 'A' || s[i] > 'F'))
			return 0;
		bignum->data[bignum->len - 1] |= get_val(s[i]) << (4 * (len - 1 - i));
	}
	return 1;
}

int bignum_hex2bignum(struct bignum *bignum, const char *s)
{
	bignum_zero(bignum);
	bignum_set_negative(bignum, s[0] == '-');
	if (s[0] == '-')
		++s;
	int len = strlen(s);
	int i = len - 1;
	while (1)
	{
		if (!do_fill(bignum, s + i - 8, 8))
			return 0;
		if (i < 8)
			break;
		i -= 8;
	}
	if (i)
	{
		if (!do_fill(bignum, 0, i))
			return 0;
	}
	bignum_trunc(bignum);
	return 1;
}
