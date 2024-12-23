#ifndef JKSSL_BIGNUM_H
#define JKSSL_BIGNUM_H

#include <stdint.h>
#include <stdio.h>

#define BIGNUM_RAND_BOT_ANY 0
#define BIGNUM_RAND_BOT_ODD 1
#define BIGNUM_RAND_TOP_ANY 0
#define BIGNUM_RAND_TOP_ONE 1
#define BIGNUM_RAND_TOP_TWO 2
#define BIGNUM_PRIME_CHECKS_AUTO 0

#define BIGNUM_BITS_PER_WORD (8 * sizeof(bignum_word_t))

#if defined(__GNUC__) && defined(__x86_64__)

typedef uint64_t bignum_word_t;
typedef int64_t bignum_sword_t;
typedef unsigned __int128 bignum_dword_t;
typedef __int128 bignum_sdword_t;

#else

typedef uint32_t bignum_word_t;
typedef int32_t bignum_sword_t;
typedef uint64_t bignum_dword_t;
typedef int64_t bignum_sdword_t;

#endif

struct bignum
{
	bignum_word_t *data;
	uint32_t cap;
	uint32_t len;
	int sign;
};

struct bignum_ctx_entry
{
	struct bignum *bignum;
	int used;
};

struct bignum_ctx
{
	struct bignum_ctx_entry *entries;
	uint32_t len;
};

struct bignum_gencb;

typedef int (*bignum_gencb_cb_t)(int a, int b, struct bignum_gencb *gencb);

struct bignum_gencb
{
	void *arg;
	bignum_gencb_cb_t callback;
};

int bignum_print(const struct bignum *bignum, FILE *fp);
int bignum_printhex(const struct bignum *bignum, FILE *fp);
int bignum_printbin(const struct bignum *bignum, FILE *fp);
int bignum_dec2bignum(struct bignum *bignum, const char *s);
int bignum_hex2bignum(struct bignum *bignum, const char *s);
int bignum_bignum2bin(const struct bignum *a, uint8_t *to);
struct bignum *bignum_bin2bignum(const uint8_t *s, int len, struct bignum *ret);

struct bignum *bignum_new(void);
void bignum_free(struct bignum *bignum);
void bignum_init(struct bignum *bignum);
void bignum_clear(struct bignum *bignum);
int bignum_grow(struct bignum *bignum, bignum_word_t a);
int bignum_grow_front(struct bignum *bignum, bignum_word_t a);
int bignum_reserve(struct bignum *bignum, uint32_t len);
int bignum_resize(struct bignum *bignum, uint32_t len);
int bignum_copy(struct bignum *dst, const struct bignum *src);
struct bignum *bignum_dup(const struct bignum *bignum);
void bignum_move(struct bignum *a, struct bignum *b);
void bignum_swap(struct bignum *a, struct bignum *b);

int bignum_cmp(const struct bignum *a, const struct bignum *b);
int bignum_ucmp(const struct bignum *a, const struct bignum *b);

static inline void bignum_trunc(struct bignum *bignum)
{
	while (bignum->len > 1 && !bignum->data[bignum->len - 1])
		--bignum->len;
}

static inline void bignum_zero(struct bignum *bignum)
{
	bignum->len = 0;
	bignum->sign = 0;
}

static inline int bignum_one(struct bignum *bignum)
{
	if (!bignum_resize(bignum, 1))
		return 0;
	bignum->data[0] = 1;
	bignum->sign = 0;
	return 1;
}

static inline int bignum_is_word(const struct bignum *bignum,
                                 bignum_word_t word)
{
	if (!bignum->len)
		return !word;
	if (bignum->sign)
		return 0;
	if (bignum->data[0] != word)
		return 0;
	for (uint32_t i = 1; i < bignum->len; ++i)
	{
		if (bignum->data[i])
			return 0;
	}
	return 1;
}

static inline int bignum_is_zero(const struct bignum *bignum)
{
	return bignum_is_word(bignum, 0);
}

static inline int bignum_is_one(const struct bignum *bignum)
{
	return bignum_is_word(bignum, 1);
}

static inline int bignum_is_negative(const struct bignum *bignum)
{
	return bignum->sign;
}

static inline void bignum_set_negative(struct bignum *bignum, int neg)
{
	bignum->sign = !!neg;
}

static inline bignum_word_t bignum_word_one(void)
{
	return 1;
}

static inline int bignum_set_word(struct bignum *bignum, bignum_word_t word)
{
	if (!bignum_resize(bignum, 1))
		return 0;
	bignum->data[0] = word;
	return 1;
}

int bignum_is_bit_set(const struct bignum *a, uint32_t b);
int bignum_set_bit(struct bignum *bignum, uint32_t n);
int bignum_clear_bit(struct bignum *bignum, uint32_t n);
int bignum_num_bits(const struct bignum *bignum);
int bignum_num_bytes(const struct bignum *bignum);

int bignum_lshift(struct bignum *r, const struct bignum *a, uint32_t n,
                  struct bignum_ctx *ctx);
int bignum_ulshift(struct bignum *r, const struct bignum *a, uint32_t n,
                   struct bignum_ctx *ctx);
int bignum_rshift(struct bignum *r, const struct bignum *a, uint32_t n,
                  struct bignum_ctx *ctx);
int bignum_urshift(struct bignum *r, const struct bignum *a, uint32_t n,
                   struct bignum_ctx *ctx);
int bignum_lshift1(struct bignum *r, const struct bignum *a,
                   struct bignum_ctx *ctx);
int bignum_ulshift1(struct bignum *r, const struct bignum *a,
                    struct bignum_ctx *ctx);
int bignum_rshift1(struct bignum *r, const struct bignum *a,
                   struct bignum_ctx *ctx);
int bignum_urshift1(struct bignum *r, const struct bignum *a,
                    struct bignum_ctx *ctx);

int bignum_rand(struct bignum *bignum, uint32_t bits, int top, int bottom);
int bignum_rand_range(struct bignum *bignum, const struct bignum *range,
                      int top, int bottom);

int bignum_add(struct bignum *r, const struct bignum *a,
               const struct bignum *b, struct bignum_ctx *ctx);
int bignum_uadd(struct bignum *r, const struct bignum *a,
                const struct bignum *b, struct bignum_ctx *ctx);
int bignum_sub(struct bignum *r, const struct bignum *a,
               const struct bignum *b, struct bignum_ctx *ctx);
int bignum_usub(struct bignum *r, const struct bignum *a,
                const struct bignum *b, struct bignum_ctx *ctx);
int bignum_mul(struct bignum *r, const struct bignum *a,
               const struct bignum *b, struct bignum_ctx *ctx);
int bignum_mul_word(struct bignum *r, const struct bignum *a, bignum_word_t b);
int bignum_umul(struct bignum *r, const struct bignum *a,
                const struct bignum *b, struct bignum_ctx *ctx);
int bignum_sqr(struct bignum *r, const struct bignum *a,
               struct bignum_ctx *ctx);
int bignum_usqr(struct bignum *r, const struct bignum *a,
                struct bignum_ctx *ctx);
int bignum_div_mod(struct bignum *dv, struct bignum *rm,
                   const struct bignum *a, const struct bignum *b,
                   struct bignum_ctx *ctx);
int bignum_udiv_mod(struct bignum *dv, struct bignum *rm,
                    const struct bignum *a, const struct bignum *b,
                    struct bignum_ctx *ctx);
int bignum_div(struct bignum *r, const struct bignum *a,
               const struct bignum *b, struct bignum_ctx *ctx);
int bignum_udiv(struct bignum *r, const struct bignum *a,
                const struct bignum *b, struct bignum_ctx *ctx);
int bignum_mod(struct bignum *r, const struct bignum *a,
               const struct bignum *b, struct bignum_ctx *ctx);
int bignum_umod(struct bignum *r, const struct bignum *a,
                const struct bignum *b, struct bignum_ctx *ctx);
int bignum_mod_word(bignum_word_t *r, const struct bignum *a, bignum_word_t b);
int bignum_exp(struct bignum *r, const struct bignum *a,
               const struct bignum *p, struct bignum_ctx *ctx);
int bignum_uexp(struct bignum *r, const struct bignum *a,
                const struct bignum *p, struct bignum_ctx *ctx);
int bignum_mod_exp(struct bignum *r, const struct bignum *a,
                   const struct bignum *p, const struct bignum *m,
                   struct bignum_ctx *ctx);
int bignum_umod_exp(struct bignum *r, const struct bignum *a,
                    const struct bignum *p, const struct bignum *m,
                    struct bignum_ctx *ctx);
int bignum_gcd(struct bignum *r, const struct bignum *a,
               const struct bignum *b, struct bignum_ctx *ctx);
int bignum_mod_inverse(struct bignum *r, const struct bignum *a,
                       const struct bignum *b, struct bignum_ctx *ctx);
int bignum_nnmod(struct bignum *r, const struct bignum *a,
                 const struct bignum *m, struct bignum_ctx *ctx);
int bignum_mod_add(struct bignum *r, const struct bignum *a,
                   const struct bignum *b, const struct bignum *m,
                   struct bignum_ctx *ctx);
int bignum_mod_sub(struct bignum *r, const struct bignum *a,
                   const struct bignum *b, const struct bignum *m,
                   struct bignum_ctx *ctx);
int bignum_mod_mul(struct bignum *r, const struct bignum *a,
                   const struct bignum *b, const struct bignum *m,
                   struct bignum_ctx *ctx);
int bignum_mod_sqr(struct bignum *r, const struct bignum *a,
                   const struct bignum *m, struct bignum_ctx *ctx);

int bignum_check_prime(const struct bignum *bignum, struct bignum_ctx *ctx,
                       struct bignum_gencb *gencb);
int bignum_generate_prime(struct bignum *bignum, size_t bits, int safe,
                          const struct bignum *add, const struct bignum *rem,
                          struct bignum_gencb *gencb,
                          struct bignum_ctx *bn_ctx);

struct bignum_ctx *bignum_ctx_new(void);
void bignum_ctx_free(struct bignum_ctx *ctx);
struct bignum *bignum_ctx_get(struct bignum_ctx *ctx);
void bignum_ctx_release(struct bignum_ctx *ctx, struct bignum *bignum);

struct bignum_gencb *bignum_gencb_new(void);
void bignum_gencb_free(struct bignum_gencb *gencb);
void bignum_gencb_set(struct bignum_gencb *gencb, bignum_gencb_cb_t callback,
                      void *arg);
void *bignum_gencb_get_arg(struct bignum_gencb *gencb);
int bignum_gencb_call(struct bignum_gencb *gencb, int a, int b);

#endif
