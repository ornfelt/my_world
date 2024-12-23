#ifndef JKSSL_EC_EC_H
#define JKSSL_EC_EC_H

#include <jkssl/ec.h>

struct ec_point;
struct bignum;

struct ec_group
{
	struct ec_point *g;
	struct bignum *p;
	struct bignum *a;
	struct bignum *b;
};

struct ec_point
{
	int infinity;
	struct bignum *x;
	struct bignum *y;
};

#endif
