#ifndef JKSSL_DSA_DSA_H
#define JKSSL_DSA_DSA_H

#include "refcount.h"

struct bignum;

struct dsa
{
	refcount_t refcount;
	struct bignum *p;
	struct bignum *q;
	struct bignum *g;
	struct bignum *y;
	struct bignum *x;
};

struct dsa_sig
{
	struct bignum *r;
	struct bignum *s;
};

#endif
