#ifndef JKSSL_SSL_H
#define JKSSL_SSL_H

struct ssl_method;
struct ssl_ctx;
struct ssl;

struct ssl *ssl_new(struct ssl_ctx *ctx);
void ssl_free(struct ssl *ssl);
int ssl_up_ref(struct ssl *ssl);

int ssl_connect(struct ssl *ssl);

const struct ssl_method *sslv23_method(void);

struct ssl_ctx *ssl_ctx_new(const struct ssl_method *method);
void ssl_ctx_free(struct ssl_ctx *ctx);
int ssl_ctx_up_ref(struct ssl_ctx *ctx);

#endif
