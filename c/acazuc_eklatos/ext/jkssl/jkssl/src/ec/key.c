#include "ec/ec.h"

#include <jkssl/bignum.h>

#include <stdlib.h>

struct ec_key *ec_key_new(void)
{
	struct ec_key *key = calloc(1, sizeof(*key));
	if (!key)
		return NULL;
	refcount_init(&key->refcount, 1);
	return key;
}

void ec_key_free(struct ec_key *key)
{
	if (!key)
		return;
	if (refcount_dec(&key->refcount))
		return;
	ec_group_free(key->group);
	bignum_free(key->priv);
	ec_point_free(key->pub);
	free(key);
}

int ec_key_up_ref(struct ec_key *key)
{
	return refcount_inc(&key->refcount);
}

const struct ec_group *ec_key_get0_group(const struct ec_key *key)
{
	return key->group;
}

int ec_key_set_group(struct ec_key *key, const struct ec_group *group)
{
	struct ec_group *dup = ec_group_dup(group);
	if (!dup)
		return 0;
	ec_group_free(key->group);
	key->group = dup;
	return 1;
}

const struct bignum *ec_key_get0_private_key(const struct ec_key *key)
{
	return key->priv;
}

int ec_key_set_private_key(struct ec_key *key, const struct bignum *pkey)
{
	if (!pkey)
		return 0;
	struct bignum *dup = bignum_dup(pkey);
	if (!dup)
		return 0;
	bignum_free(key->priv);
	key->priv = dup;
	return 1;
}

const struct ec_point *ec_key_get0_public_key(const struct ec_key *key)
{
	return key->pub;
}

int ec_key_set_public_key(struct ec_key *key, const struct ec_point *pkey)
{
	struct ec_point *dup = ec_point_dup(pkey);
	if (!dup)
		return 0;
	ec_point_free(key->pub);
	key->pub = dup;
	return 1;
}

int ec_key_generate_key(struct ec_key *key)
{
	struct bignum *priv = NULL;
	struct ec_point *pub = NULL;
	struct bignum_ctx *bn_ctx = NULL;

	if (!key->group || !key->group->n)
		return 0;
	priv = bignum_new();
	pub = ec_point_new(key->group);
	bn_ctx = bignum_ctx_new();
	if (!priv
	 || !pub
	 || !bn_ctx
	 || !bignum_rand_range(priv, key->group->n,
	                       BIGNUM_RAND_TOP_ONE,
	                       BIGNUM_RAND_BOT_ODD)
	 || !ec_point_mul(key->group, pub, priv, NULL, NULL, bn_ctx))
		goto err;
	bignum_ctx_free(bn_ctx);
	bignum_free(key->priv);
	ec_point_free(key->pub);
	key->priv = priv;
	key->pub = pub;
	return 1;

err:
	bignum_ctx_free(bn_ctx);
	bignum_free(priv);
	ec_point_free(pub);
	return 0;
}

int ec_key_check_key(const struct ec_key *key)
{
	struct bignum_ctx *bn_ctx = NULL;
	int ret = -1;

	if (!key->group)
		return 0;
	bn_ctx = bignum_ctx_new();
	if (!bn_ctx)
		goto end;
	if (key->pub)
	{
		switch (ec_point_is_on_curve(key->group, key->pub, bn_ctx))
		{
			case -1:
				goto end;
			case 0:
				ret = 0;
				goto end;
		}
		if (key->priv)
		{
			if (!ec_point_mul(key->group, key->pub, key->priv,
			                  NULL, NULL, bn_ctx))
				goto end;
		}
	}
	ret = 1;

end:
	bignum_ctx_free(bn_ctx);
	return ret;
}

void ec_key_set_asn1_flag(struct ec_key *key, int flag)
{
	ec_group_set_asn1_flag(key->group, flag);
}

int ec_key_get_asn1_flag(struct ec_key *key)
{
	return ec_group_get_asn1_flag(key->group);
}

void ec_key_set_conv_form(struct ec_key *key, enum point_conversion_form form)
{
	ec_group_set_point_conversion_form(key->group, form);
}

enum point_conversion_form ec_key_get_conv_form(const struct ec_key *key)
{
	return ec_group_get_point_conversion_form(key->group);
}
