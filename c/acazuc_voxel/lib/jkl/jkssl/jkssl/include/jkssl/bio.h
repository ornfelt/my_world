#ifndef JKSSL_BIO_H
#define JKSSL_BIO_H

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>

#define BIO_CLOSE   0
#define BIO_NOCLOSE (1 << 0)

enum bio_ctrl
{
	BIO_CTRL_RESET,
	BIO_CTRL_SEEK,
	BIO_CTRL_TELL,
	BIO_CTRL_FLUSH,
	BIO_CTRL_EOF,
	BIO_CTRL_CIPHER_GET,
	BIO_CTRL_CIPHER_SET,
	BIO_CTRL_MEM_PTR_GET,
};

struct bio_method;
struct evp_cipher;
struct bio;

struct bio_cipher_set_ctx
{
	const struct evp_cipher *cipher;
	const uint8_t *key;
	const uint8_t *iv;
	int enc;
};

struct buf_mem
{
	void *data;
	size_t size;
};

struct bio *bio_new(const struct bio_method *method);
int bio_free(struct bio *bio);
void bio_vfree(struct bio *bio);
void bio_free_all(struct bio *bio);
int bio_up_ref(struct bio *bio);
struct bio *bio_push(struct bio *bio, struct bio *next);
struct bio *bio_pop(struct bio *bio);
void bio_set_next(struct bio *bio, struct bio *next);

long bio_ctrl(struct bio *bio, enum bio_ctrl cmd, long arg1, void *arg2);

#define bio_reset(bio) bio_ctrl(bio, BIO_CTRL_RESET, 0, NULL)
#define bio_seek(bio, ofs) bio_ctrl(bio, BIO_CTRL_SEEK, ofs, NULL)
#define bio_tell(bio) bio_ctrl(bio, BIO_CTRL_TELL, 0, NULL)
#define bio_flush(bio) bio_ctrl(bio, BIO_CTRL_FLUSH, 0, NULL)
#define bio_eof(bio) bio_ctrl(bio, BIO_CTRL_EOF, 0, NULL)
#define bio_get_cipher_ctx(bio, ctx) bio_ctrl(bio, BIO_CTRL_CIPHER_GET, 0, ctx)
#define bio_get_mem_ptr(bio, ptr) bio_ctrl(bio, BIO_CTRL_MEM_PTR_GET, 0, ptr)

static inline int bio_set_cipher(struct bio *bio, const struct evp_cipher *cipher,
                                 const uint8_t *key, const uint8_t *iv, int enc)
{
	struct bio_cipher_set_ctx ctx;
	ctx.cipher = cipher;
	ctx.key = key;
	ctx.iv = iv;
	ctx.enc = enc;
	return bio_ctrl(bio, BIO_CTRL_CIPHER_SET, 0, &ctx);
}

ssize_t bio_read(struct bio *bio, void *data, size_t size);
ssize_t bio_write(struct bio *bio, const void *data, size_t size);
ssize_t bio_gets(struct bio *bio, char *data, size_t size);
ssize_t bio_puts(struct bio *bio, const char *line);

int bio_printf(struct bio *bio, const char *fmt, ...);
int bio_vprintf(struct bio *bio, const char *fmt, va_list ap);

struct bio *bio_new_mem_buf(const void *buf, ssize_t size);
struct bio *bio_new_file(const char *filename, const char *mode);
struct bio *bio_new_fp(FILE *fp, int flags);
struct bio *bio_new_fd(int fd, int flags);

const struct bio_method *bio_s_mem(void);
const struct bio_method *bio_s_file(void);
const struct bio_method *bio_s_fd(void);
const struct bio_method *bio_s_null(void);

const struct bio_method *bio_f_base64(void);
const struct bio_method *bio_f_buffer(void);
const struct bio_method *bio_f_cipher(void);
const struct bio_method *bio_f_md(void);
const struct bio_method *bio_f_null(void);

#endif
