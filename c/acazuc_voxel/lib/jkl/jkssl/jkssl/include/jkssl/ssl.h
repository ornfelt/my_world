#ifndef JKSSL_SSL_H
#define JKSSL_SSL_H

#include "refcount.h"

struct ssl
{
	refcount_t refcount;
};

struct ssl *ssl_new(void);
void ssl_free(struct ssl *ssl);
int ssl_up_ref(struct ssl *ssl);

#endif
