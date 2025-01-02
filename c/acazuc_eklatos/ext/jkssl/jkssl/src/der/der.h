#ifndef JKSSL_DER_DER_H
#define JKSSL_DER_DER_H

#include <jkssl/der.h>

#define D2I_MEM(name, ctx_t, ...) \
do \
{ \
	struct bio *bio = bio_new_mem_buf(*data, len); \
	if (!bio) \
		return NULL; \
	ctx_t *ret = d2i_##name##_bio(bio, ##__VA_ARGS__); \
	*data += bio_tell(bio); \
	bio_free(bio); \
	return ret; \
} while (0)

#define D2I_FP(name, ctx_t, ...) \
do \
{ \
	struct bio *bio = bio_new_fp(fp, BIO_NOCLOSE); \
	if (!bio) \
		return NULL; \
	ctx_t *ret = d2i_##name##_bio(bio, ##__VA_ARGS__); \
	bio_free(bio); \
	return ret; \
} while (0)

#define I2D_MEM(name, ...) \
do \
{ \
	struct bio *bio = bio_new(bio_s_mem()); \
	if (!bio) \
		return -1; \
	int ret = i2d_##name##_bio(bio, ##__VA_ARGS__); \
	if (!ret) \
		return -1; \
	struct buf_mem *mem; \
	bio_get_mem_ptr(bio, &mem); \
	*dst = mem->data; \
	size_t size = mem->size; \
	mem->data = NULL; \
	mem->size = 0; \
	bio_free(bio); \
	return size; \
} while (0)

#define I2D_FP(name, ...) \
do \
{ \
	struct bio *bio = bio_new_fp(fp, BIO_NOCLOSE); \
	if (!bio) \
		return 0; \
	int ret = i2d_##name##_bio(bio, ##__VA_ARGS__); \
	bio_free(bio); \
	return ret; \
} while (0)

#define DEF_D2I(name, st) \
struct st *d2i_##name(struct st **v, uint8_t **data, size_t len) \
{ \
	D2I_MEM(name, struct st, v); \
} \
struct st *d2i_##name##_fp(FILE *fp, struct st **v) \
{ \
	D2I_FP(name, struct st, v); \
} \
struct st *d2i_##name##_bio(struct bio *bio, struct st **v) \
{ \
	struct asn1_object *object = d2i_asn1_object_bio(bio, &object); \
	if (!object) \
		return NULL; \
	struct st *ret = a2i_##name(object, v); \
	asn1_object_free(object); \
	if (v) \
		*v = ret; \
	return ret; \
}

#define DEF_D2I_PASS(name, st) \
struct st *d2i_##name(struct st **v, \
                      uint8_t **data, \
                      size_t len, \
                      pem_password_cb *cb, \
                      void *userdata) \
{ \
	D2I_MEM(name, struct st, v, cb, userdata); \
} \
struct st *d2i_##name##_fp(FILE *fp, \
                           struct st **v, \
                           pem_password_cb *cb, \
                           void *userdata) \
{ \
	D2I_FP(name, struct st, v, cb, userdata); \
} \
struct st *d2i_##name##_bio(struct bio *bio, \
                            struct st **v, \
                            pem_password_cb *cb, \
                            void *userdata) \
{ \
	struct asn1_object *object = d2i_asn1_object_bio(bio, &object); \
	if (!object) \
		return NULL; \
	struct st *ret = a2i_##name(object, v, cb, userdata); \
	asn1_object_free(object); \
	if (v) \
		*v = ret; \
	return ret; \
}

#define DEF_I2D(name, st) \
int i2d_##name(const struct st *v, uint8_t **dst) \
{ \
	I2D_MEM(name, v); \
} \
int i2d_##name##_fp(FILE *fp, const struct st *v) \
{ \
	I2D_FP(name, v); \
} \
int i2d_##name##_bio(struct bio *bio, const struct st *v) \
{ \
	struct asn1_object *object = i2a_##name(v); \
	if (!object) \
		return 0; \
	if (!i2d_asn1_object_bio(bio, object)) \
	{ \
		asn1_object_free(object); \
		return 0; \
	} \
	asn1_object_free(object); \
	return 1; \
}

#define DEF_I2D_PASS(name, st) \
int i2d_##name(const struct st *v, \
               uint8_t **dst, \
               const struct evp_cipher *cipher, \
               pem_password_cb *cb, \
               void *userdata) \
{ \
	I2D_MEM(name, v, cipher, cb, userdata); \
} \
int i2d_##name##_fp(FILE *fp, \
                    const struct st *v, \
                    const struct evp_cipher *cipher, \
                    pem_password_cb *cb, \
                    void *userdata) \
{ \
	I2D_FP(name, v, cipher, cb, userdata); \
} \
int i2d_##name##_bio(struct bio *bio, \
                     const struct st *v, \
                     const struct evp_cipher *cipher, \
                     pem_password_cb *cb, \
                     void *userdata) \
{ \
	struct asn1_object *object = i2a_##name(v, cipher, cb, userdata); \
	if (!object) \
		return 0; \
	int ret = i2d_asn1_object_bio(bio, object); \
	asn1_object_free(object); \
	return ret; \
}

#define DER_DEF(name, st) \
DEF_D2I(name, st); \
DEF_I2D(name, st)

#define DER_DEF_PASS(name, st) \
DEF_D2I_PASS(name, st); \
DEF_I2D_PASS(name, st)

#endif
