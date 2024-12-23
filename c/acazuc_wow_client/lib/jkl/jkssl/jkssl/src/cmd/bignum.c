#include "cmd/cmd.h"

#include <jkssl/bignum.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static int do_execute1(int argc, char **argv, int (*fn)(struct bignum *r,
                                                        const struct bignum *a,
                                                        const struct bignum *b,
                                                        struct bignum_ctx *ctx))
{
	struct bignum_ctx *ctx;
	struct bignum a;
	struct bignum b;
	struct bignum r;
	int ret = EXIT_FAILURE;

	if (argc < 4)
		return EXIT_FAILURE;
	bignum_init(&a);
	bignum_init(&b);
	bignum_init(&r);
	ctx = bignum_ctx_new();
	if (!ctx)
		goto end;
	if (!bignum_dec2bignum(&a, argv[1])
	 || !bignum_dec2bignum(&b, argv[3])
	 || !fn(&r, &a, &b, ctx)
	 || !bignum_print(&r, stdout))
		goto end;
	ret = EXIT_SUCCESS;

end:
	bignum_clear(&a);
	bignum_clear(&b);
	bignum_clear(&r);
	bignum_ctx_free(ctx);
	return ret;
}

static int do_execute2(int argc, char **argv, int (*fn)(struct bignum *r,
                                                        const struct bignum *a,
                                                        uint32_t b,
                                                        struct bignum_ctx *ctx))
{
	struct bignum_ctx *ctx;
	struct bignum a;
	struct bignum r;
	uint64_t b;
	int ret = EXIT_FAILURE;

	if (argc < 4)
		return EXIT_FAILURE;
	b = atol(argv[3]);
	bignum_init(&a);
	bignum_init(&r);
	ctx = bignum_ctx_new();
	if (!ctx)
		goto end;
	if (!bignum_dec2bignum(&a, argv[1])
	 || !fn(&r, &a, b, ctx)
	 || !bignum_print(&r, stdout))
		goto end;
	ret = EXIT_SUCCESS;

end:
	bignum_clear(&a);
	bignum_clear(&r);
	bignum_ctx_free(ctx);
	return ret;
}

static int do_execute3(int argc, char **argv, int (*fn)(struct bignum *r,
                                                        const struct bignum *a,
                                                        struct bignum_ctx *ctx))
{
	struct bignum_ctx *ctx;
	struct bignum a;
	struct bignum r;
	int ret = EXIT_FAILURE;

	if (argc < 3)
		return EXIT_FAILURE;
	bignum_init(&a);
	bignum_init(&r);
	ctx = bignum_ctx_new();
	if (!ctx)
		goto end;
	if (!bignum_dec2bignum(&a, argv[1])
	 || !fn(&r, &a, ctx)
	 || !bignum_print(&r, stdout))
		goto end;
	ret = EXIT_SUCCESS;

end:
	bignum_clear(&a);
	bignum_clear(&r);
	bignum_ctx_free(ctx);
	return ret;
}

static int do_execute4(int argc, char **argv, int (*fn)(struct bignum *r,
                                                        const struct bignum *a,
                                                        const struct bignum *b,
                                                        const struct bignum *c,
                                                        struct bignum_ctx *ctx))
{
	struct bignum_ctx *ctx;
	struct bignum a;
	struct bignum b;
	struct bignum c;
	struct bignum r;
	int ret = EXIT_FAILURE;

	if (argc < 5)
		return EXIT_FAILURE;
	bignum_init(&a);
	bignum_init(&b);
	bignum_init(&c);
	bignum_init(&r);
	ctx = bignum_ctx_new();
	if (!ctx)
		goto end;
	if (!bignum_dec2bignum(&a, argv[1])
	 || !bignum_dec2bignum(&b, argv[3])
	 || !bignum_dec2bignum(&c, argv[4])
	 || !fn(&r, &a, &b, &c, ctx)
	 || !bignum_print(&r, stdout))
		goto end;
	ret = EXIT_SUCCESS;

end:
	bignum_clear(&a);
	bignum_clear(&b);
	bignum_clear(&c);
	bignum_clear(&r);
	bignum_ctx_free(ctx);
	return ret;
}

static int do_execute_prime(char *v)
{
	struct bignum_ctx *ctx;
	struct bignum a;
	int is_prime;
	int ret = EXIT_FAILURE;

	bignum_init(&a);
	ctx = bignum_ctx_new();
	if (!ctx)
		goto end;
	if (!bignum_dec2bignum(&a, v))
		goto end;
	is_prime = bignum_check_prime(&a, ctx, NULL);
	if (is_prime == -1)
		goto end;
	if (is_prime)
		printf("1\n");
	else
		printf("0\n");
	ret = EXIT_SUCCESS;

end:
	bignum_clear(&a);
		bignum_ctx_free(ctx);
	return ret;
}

int cmd_bignum(int argc, char **argv)
{
	if (argc < 3)
		return EXIT_FAILURE;
	if (!strcmp(argv[2], "+"))
		return do_execute1(argc, argv, &bignum_add);
	if (!strcmp(argv[2], "-"))
		return do_execute1(argc, argv, &bignum_sub);
	if (!strcmp(argv[2], "_"))
		return do_execute1(argc, argv, &bignum_mul);
	if (!strcmp(argv[2], "/"))
		return do_execute1(argc, argv, &bignum_div);
	if (!strcmp(argv[2], "%"))
		return do_execute1(argc, argv, &bignum_mod);
	if (!strcmp(argv[2], "^"))
		return do_execute1(argc, argv, &bignum_exp);
	if (!strcmp(argv[2], ">>"))
		return do_execute2(argc, argv, &bignum_rshift);
	if (!strcmp(argv[2], "<<"))
		return do_execute2(argc, argv, &bignum_lshift);
	if (!strcmp(argv[2], ">>1"))
		return do_execute3(argc, argv, &bignum_rshift1);
	if (!strcmp(argv[2], "<<1"))
		return do_execute3(argc, argv, &bignum_lshift1);
	if (!strcmp(argv[2], "is_prime"))
		return do_execute_prime(argv[1]);
	if (!strcmp(argv[2], "%+"))
		return do_execute4(argc, argv, &bignum_mod_add);
	if (!strcmp(argv[2], "%-"))
		return do_execute4(argc, argv, &bignum_mod_sub);
	if (!strcmp(argv[2], "%*"))
		return do_execute4(argc, argv, &bignum_mod_mul);
	return EXIT_FAILURE;
}
