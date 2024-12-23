#include "ec/ec.h"

#include <jkssl/bignum.h>

#include <stdlib.h>

struct ecdsa_sig *ecdsa_sig_new(void)
{
	return calloc(1, sizeof(struct ecdsa_sig));
}

void ecdsa_sig_free(struct ecdsa_sig *sig)
{
	if (!sig)
		return;
	bignum_free(sig->r);
	bignum_free(sig->s);
	free(sig);
}

void ecdsa_sig_get0(const struct ecdsa_sig *sig,
                    const struct bignum **r,
                    const struct bignum **s)
{
	if (r)
		*r = sig->r;
	if (s)
		*s = sig->s;
}

const struct bignum *ecdsa_sig_get0_r(const struct ecdsa_sig *sig)
{
	return sig->r;
}

const struct bignum *ecdsa_sig_get0_s(const struct ecdsa_sig *sig)
{
	return sig->s;
}

int ecdsa_sig_set0(struct ecdsa_sig *sig, struct bignum *r, struct bignum *s)
{
	bignum_free(sig->r);
	bignum_free(sig->s);
	sig->r = r;
	sig->s = s;
	return 1;
}
