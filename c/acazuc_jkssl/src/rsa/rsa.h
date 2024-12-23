#ifndef JKSSL_RSA_RSA_H
#define JKSSL_RSA_RSA_H

#include "refcount.h"

struct bignum;

struct rsa
{
	refcount_t refcount;
	struct bignum *p;
	struct bignum *q;
	struct bignum *d;
	struct bignum *phi;
	struct bignum *e;
	struct bignum *n;
	struct bignum *dmq;
	struct bignum *dmp;
	struct bignum *coef;
};

#endif
