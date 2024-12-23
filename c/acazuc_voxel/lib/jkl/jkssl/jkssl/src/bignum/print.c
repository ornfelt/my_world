#include <jkssl/bignum.h>

#include <stdio.h>

static int print(struct bignum *tmp1, struct bignum *tmp2, struct bignum *div,
                 FILE *fp, struct bignum_ctx *ctx)
{
	bignum_set_negative(tmp1, 0);
	if (!tmp1->len || (tmp1->len == 1 && !tmp1->data[0]))
		return 1;
	if (!bignum_mod(tmp2, tmp1, div, ctx))
		return 0;
	char c = tmp2->data[0];
	if (tmp1->len > 1 || tmp1->data[0] > 9)
	{
		if (!bignum_div(tmp1, tmp1, div, ctx))
			return 0;
		print(tmp1, tmp2, div, fp, ctx);
	}
	fprintf(fp, "%c", '0' + c);
	return 1;
}

int bignum_print(const struct bignum *bignum, FILE *fp)
{
	struct bignum_ctx *ctx;
	struct bignum tmp1;
	struct bignum tmp2;
	struct bignum div;
	int ret = 0;

	if (bignum_is_negative(bignum))
		fprintf(fp, "-");
	if (!bignum->len || (bignum->len == 1 && !bignum->data[0]))
	{
		fprintf(fp, "0");
		return 1;
	}
	bignum_init(&div);
	bignum_init(&tmp1);
	bignum_init(&tmp2);
	ctx = bignum_ctx_new();
	if (!ctx
	 || !bignum_grow(&div, 10)
	 || !bignum_copy(&tmp1, bignum))
		goto end;
	ret = print(&tmp1, &tmp2, &div, fp, ctx);

end:
	bignum_clear(&div);
	bignum_clear(&tmp1);
	bignum_clear(&tmp2);
	bignum_ctx_free(ctx);
	return ret;
}

int bignum_printbin(const struct bignum *bignum, FILE *fp)
{
	char buffer[1024];
	uint32_t buffer_pos;
	uint32_t i;
	uint32_t total;
	uint32_t s8;

	buffer_pos = 0;
	total = bignum_num_bits(bignum);
	s8 = sizeof(*bignum->data) * 8;
	i = 0;
	while (i < total)
	{
		buffer[buffer_pos++] = bignum->data[i / s8] & (0x1 << (s8 - (i % s8))) ? 1 : 0;
		if (buffer_pos == sizeof(buffer))
		{
			fprintf(fp, "%s", buffer);
			buffer_pos = 0;
		}
		++i;
	}
	if (buffer_pos)
		fprintf(fp, "%s", buffer);
	return 1;
}

static void print_hex(bignum_word_t data, FILE *fp, int first)
{
	for (uint32_t i = 0; i < 8 * sizeof(bignum_word_t); i += 4)
	{
		uint8_t v = (data >> (sizeof(bignum_word_t) * 8 - 4 - i)) & 0xf;
		if (v || !first)
		{
			first = 0;
			if (v >= 10)
				fprintf(fp, "%c", 'a' + v - 10);
			else
				fprintf(fp, "%c", '0' + v);
		}
	}
}

int bignum_printhex(const struct bignum *bignum, FILE *fp)
{
	if (!bignum->len || (bignum->len == 1 && !bignum->data[0]))
	{
		fprintf(fp, "0");
		return 1;
	}
	if (bignum_is_negative(bignum))
		fprintf(fp, "-");
	if (!bignum->len)
	{
		fprintf(fp, "0");
		return 1;
	}
	uint32_t i = bignum->len - 1;
	while (1)
	{
		print_hex(bignum->data[i], fp, i == bignum->len - 1);
		if (!i)
			break;
		--i;
	}
	return 1;
}
