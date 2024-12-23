#include "asn1/asn1.h"
#include "der/der.h"
#include "pem/pem.h"

#include <jkssl/bio.h>
#include <jkssl/dh.h>

#include <stdlib.h>

struct dh_ctx *a2i_dh_params(const struct asn1_object *object,
                             struct dh_ctx **ctx)
{
	if (!ASN1_IS_UNIVERSAL_TYPE(object, ASN1_SEQUENCE))
		return NULL;
	struct asn1_constructed *sequence = (struct asn1_constructed*)object;
	if (sequence->objects_count != 2
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[0], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[1], ASN1_INTEGER))
		return NULL;
	struct asn1_integer *p = (struct asn1_integer*)sequence->objects[0];
	struct asn1_integer *g = (struct asn1_integer*)sequence->objects[1];
	if (!asn1_integer_get_bignum(p, &(*ctx)->p)
	 || !asn1_integer_get_bignum(g, &(*ctx)->g))
		return NULL;
	return *ctx;
}

struct asn1_object *i2a_dh_params(struct dh_ctx *ctx)
{
	struct asn1_constructed *sequence = asn1_sequence_new();
	struct asn1_integer *p = asn1_integer_new();
	struct asn1_integer *g = asn1_integer_new();
	struct asn1_object *ret = NULL;
	if (!sequence
	 || !p
	 || !g
	 || !asn1_integer_set_bignum(p, ctx->p)
	 || !asn1_integer_set_bignum(g, ctx->g)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)p)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)g))
		goto end;
	ret = (struct asn1_object*)sequence;

end:
	if (!ret)
		asn1_constructed_free(sequence);
	asn1_integer_free(p);
	asn1_integer_free(g);
	return ret;
}

DER_DEF(dh_params, dh_ctx);
PEM_DEF_RPWC(dh_params, dh_ctx, "DH PARAMETERS");
