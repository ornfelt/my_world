#ifndef JKSSL_PEM_PEM_H
#define JKSSL_PEM_PEM_H

#include <jkssl/pem.h>

struct evp_cipher;
struct bio;

struct pem_write_ctx
{
	const struct evp_cipher *evp_cipher;
	uint8_t *data;
	size_t len;
	pem_password_cb *password_cb;
	void *password_userdata;
	const char *text;
	struct bio *bio;
};

struct pem_read_ctx
{
	struct bio *bio;
	uint8_t *data;
	size_t len;
	uint8_t *salt_iv;
	int salt_iv_len;
	const struct evp_cipher *evp_cipher;
	pem_password_cb *password_cb;
	void *password_userdata;
	const char *text;
	char matched_text[65];
};

int pem_get_key(const struct evp_cipher *evp_cipher, uint8_t *key,
                const uint8_t *salt_iv, const char *pass);
int pem_write(struct pem_write_ctx *ctx);
int pem_read(struct pem_read_ctx *ctx);

#define PEM_WRITE_FP(name, ...) \
do \
{ \
	struct bio *bio = bio_new_fp(fp, BIO_NOCLOSE); \
	if (!bio) \
		return 0; \
	int ret = pem_write_bio_##name(bio, ##__VA_ARGS__); \
	bio_free(bio); \
	return ret; \
} while (0)

#define PEM_WRITE_BIO(name, txt, ctx, cipher, cb, userdata) \
do \
{ \
	struct pem_write_ctx pem_ctx; \
	int ret; \
	pem_ctx.bio = bio; \
	pem_ctx.evp_cipher = cipher; \
	pem_ctx.password_cb = cb; \
	pem_ctx.password_userdata = userdata; \
	pem_ctx.text = txt; \
	pem_ctx.len = i2d_##name(ctx, &pem_ctx.data); \
	if (pem_ctx.len == (size_t)-1) \
		return 0; \
	ret = pem_write(&pem_ctx); \
	free(pem_ctx.data); \
	return ret; \
} while (0)

#define PEM_READ_FP(name, ctx_t, ...) \
do \
{ \
	struct bio *bio = bio_new_fp(fp, BIO_NOCLOSE); \
	if (!bio) \
		return NULL; \
	ctx_t *ret = pem_read_bio_##name(bio, ##__VA_ARGS__); \
	bio_free(bio); \
	return ret; \
} while (0)

#define PEM_READ_BIO(name, ctx_t, txt, ctx, cb, userdata) \
do \
{ \
	struct pem_read_ctx pem_ctx; \
	ctx_t *ret; \
	uint8_t *data; \
	pem_ctx.bio = bio; \
	pem_ctx.password_cb = cb; \
	pem_ctx.password_userdata = userdata; \
	pem_ctx.text = txt; \
	if (!pem_read(&pem_ctx)) \
		return NULL; \
	data = pem_ctx.data; \
	ret = d2i_##name(ctx, &data, pem_ctx.len); \
	free(pem_ctx.data); \
	return ret; \
} while (0)

#define PEM_DEF_READ(name, st, txt) \
struct st *pem_read_bio_##name(struct bio *bio, struct st **v) \
{ \
	PEM_READ_BIO(name, struct st, txt, v, NULL, NULL); \
} \
struct st *pem_read_##name(FILE *fp, struct st **v) \
{ \
	PEM_READ_FP(name, struct st, v); \
}

#define PEM_DEF_READ_PASS(name, st, txt) \
struct st *pem_read_bio_##name(struct bio *bio, \
                               struct st **v, \
                               pem_password_cb *cb, \
                               void *userdata) \
{ \
	PEM_READ_BIO(name, struct st, txt, v, cb, userdata); \
} \
struct st *pem_read_##name(FILE *fp, \
                           struct st **v, \
                           pem_password_cb *cb, \
                           void *userdata) \
{ \
	PEM_READ_FP(name, struct st, v, cb, userdata); \
}

#define PEM_DEF_WRITE(name, st, txt) \
int pem_write_bio_##name(struct bio *bio, \
                         const struct st *v) \
{ \
	PEM_WRITE_BIO(name, txt, v, NULL, NULL, NULL); \
} \
int pem_write_##name(FILE *fp, const struct st *v) \
{ \
	PEM_WRITE_FP(name, v); \
}

#define PEM_DEF_WRITE_PASS(name, st, txt) \
int pem_write_bio_##name(struct bio *bio, \
                         const struct st *v, \
                         const struct evp_cipher *cipher, \
                         pem_password_cb *cb, \
                         void *userdata) \
{ \
	PEM_WRITE_BIO(name, txt, v, cipher, cb, userdata); \
} \
int pem_write_##name(FILE *fp, \
                     const struct st *v, \
                     const struct evp_cipher *cipher, \
                     pem_password_cb *cb, \
                     void *userdata) \
{ \
	PEM_WRITE_FP(name, v, cipher, cb, userdata); \
}

#define PEM_DEF_RPWP(name, st, txt) \
PEM_DEF_READ_PASS(name, st, txt); \
PEM_DEF_WRITE_PASS(name, st, txt)

#define PEM_DEF_RPWC(name, st, txt) \
PEM_DEF_READ_PASS(name, st, txt); \
PEM_DEF_WRITE(name, st, txt)

#define PEM_DEF_RCWC(name, st, txt) \
PEM_DEF_READ(name, st, txt); \
PEM_DEF_WRITE(name, st, txt)

#endif
