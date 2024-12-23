#include <jkssl/ssl.h>

#include <stdlib.h>

struct ssl *ssl_new(void)
{
	struct ssl *ssl = calloc(sizeof(*ssl), 1);
	if (!ssl)
		return NULL;
	refcount_init(&ssl->refcount, 1);
	return ssl;
}

void ssl_free(struct ssl *ssl)
{
	if (!ssl)
		return;
	if (refcount_dec(&ssl->refcount))
		return;
	free(ssl);
}

int ssl_up_ref(struct ssl *ssl)
{
	return refcount_inc(&ssl->refcount);
}
