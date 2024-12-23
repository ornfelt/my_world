#ifndef JKSSL_DH_DH_H
#define JKSSL_DH_DH_H

#include "refcount.h"

struct bignum;

struct dh
{
	refcount_t refcount;
	struct bignum *p;
	struct bignum *q;
	struct bignum *g;
	struct bignum *x;
	struct bignum *y;
};

#endif
