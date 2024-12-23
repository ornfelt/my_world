#include "sha/sha.h"

#include <jkssl/bignum.h>
#include <jkssl/srp.h>
#include <jkssl/evp.h>

#include <stdlib.h>
#include <string.h>

struct bignum *srp_calc_server_key(const struct bignum *A,
                                   const struct bignum *v,
                                   const struct bignum *u,
                                   const struct bignum *b,
                                   const struct bignum *n)
{
	struct bignum_ctx *ctx = NULL;
	struct bignum *ret = NULL;

	ctx = bignum_ctx_new();
	ret = bignum_new();
	if (!ctx
	 || !ret
	 || !bignum_mod_exp(ret, v, u, n, ctx)
	 || !bignum_mod_mul(ret, ret, A, n, ctx)
	 || !bignum_mod_exp(ret, ret, b, n, ctx))
		goto err;
	bignum_ctx_free(ctx);
	return ret;

err:
	bignum_free(ret);
	bignum_ctx_free(ctx);
	return NULL;
}

struct bignum *srp_calc_B(const struct bignum *b, const struct bignum *n,
                          const struct bignum *g, const struct bignum *v)
{
	struct bignum_ctx *ctx = NULL;
	struct bignum *ret = NULL;

	ctx = bignum_ctx_new();
	ret = bignum_new();
	if (!ctx
	 || !ret
	 || !bignum_mod_exp(ret, g, b, n, ctx)
	 || !bignum_add(ret, ret, v, ctx)
	 || !bignum_mod(ret, ret, n, ctx))
		goto err;
	bignum_ctx_free(ctx);
	return ret;

err:
	bignum_free(ret);
	bignum_ctx_free(ctx);
	return NULL;
}

struct bignum *srp_calc_u(const struct bignum *A, const struct bignum *B,
                          const struct bignum *n)
{
	size_t n_size;
	size_t A_size;
	size_t B_size;
	uint8_t *AB_data = NULL;
	uint8_t md[SHA_DIGEST_LENGTH];

	n_size = bignum_num_bytes(n);
	if (!n_size
	 || bignum_ucmp(A, n) > 0
	 || bignum_ucmp(B, n) > 0)
	AB_data = malloc(n_size * 2);
	if (!AB_data)
		goto err;
	memset(AB_data, 0, n_size * 2);
	A_size = bignum_num_bytes(A);
	B_size = bignum_num_bytes(B);
	if (!bignum_bignum2bin(A, AB_data + n_size - A_size)
	 || !bignum_bignum2bin(B, AB_data + n_size * 2 - B_size)
	 || !sha1(AB_data, n_size * 2, md))
		goto err;
	struct bignum *ret = bignum_bin2bignum(AB_data, n_size * 2, NULL);
	free(AB_data);
	return ret;

err:
	free(AB_data);
	return NULL;
}

struct bignum *srp_calc_client_key(const struct bignum *n,
                                   const struct bignum *B,
                                   const struct bignum *g,
                                   const struct bignum *x,
                                   const struct bignum *a,
                                   const struct bignum *u)
{
	struct bignum_ctx *ctx = NULL;
	struct bignum *ret = NULL;
	struct bignum *tmp = NULL;

	ctx = bignum_ctx_new();
	ret = bignum_new();
	tmp = bignum_new();
	if (!ctx
	 || !ret
	 || !tmp
	 || !bignum_mod_exp(ret, g, x, n, ctx)
	 || bignum_cmp(B, ret) < 0
	 || !bignum_sub(ret, B, ret, ctx)
	 || !bignum_mul(tmp, u, x, ctx)
	 || !bignum_add(tmp, tmp, a, ctx)
	 || !bignum_mod_exp(ret, ret, tmp, n, ctx))
		goto err;
	bignum_ctx_free(ctx);
	bignum_free(tmp);
	return ret;

err:
	bignum_free(ret);
	bignum_free(tmp);
	bignum_ctx_free(ctx);
	return NULL;
}

struct bignum *srp_calc_x(const struct bignum *s, const char *user,
                          const char *pass)
{
	uint8_t sha_a[SHA_DIGEST_LENGTH];
	uint8_t sha_b[SHA_DIGEST_LENGTH];
	uint8_t *s_data = NULL;
	size_t s_size;
	struct evp_md_ctx *ctx = NULL;
	struct bignum *ret = NULL;

	s_size = bignum_num_bytes(s);
	if (!s_size)
		goto err;
	ctx = evp_md_ctx_new();
	s_data = malloc(s_size);
	ret = bignum_new();
	if (!ctx
	 || !s_data
	 || !ret
	 || !bignum_bignum2bin(s, s_data)
	 || !evp_digest_init(ctx, evp_sha1())
	 || !evp_digest_update(ctx, (uint8_t*)user, strlen(user))
	 || !evp_digest_update(ctx, (uint8_t*)":", 1)
	 || !evp_digest_update(ctx, (uint8_t*)pass, strlen(pass))
	 || !evp_digest_final(ctx, sha_a)
	 || !evp_md_ctx_reset(ctx)
	 || !evp_digest_update(ctx, s_data, s_size)
	 || !evp_digest_update(ctx, sha_a, sizeof(sha_a))
	 || !evp_digest_final(ctx, sha_b)
	 || !bignum_bin2bignum(sha_b, sizeof(sha_b), ret))
		goto err;
	free(s_data);
	evp_md_ctx_free(ctx);
	return ret;

err:
	free(s_data);
	bignum_free(ret);
	evp_md_ctx_free(ctx);
	return NULL;
}

struct bignum *srp_calc_A(const struct bignum *a, const struct bignum *n,
                          const struct bignum *g)
{
	struct bignum_ctx *ctx = NULL;
	struct bignum *ret = NULL;

	ctx = bignum_ctx_new();
	ret = bignum_new();
	if (!ctx
	 || !ret
	 || !bignum_mod_exp(ret, g, a, n, ctx))
		goto err;
	bignum_ctx_free(ctx);
	return ret;

err:
	bignum_free(ret);
	bignum_ctx_free(ctx);
	return NULL;
}
