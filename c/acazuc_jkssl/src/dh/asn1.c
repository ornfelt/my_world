#include "asn1/asn1.h"
#include "der/der.h"
#include "pem/pem.h"
#include "dh/dh.h"

#include <jkssl/bio.h>
#include <jkssl/dh.h>

#include <stdlib.h>

ASN1_OBJ_ALLOC_DEF(dh, struct dh, dh_new);

struct dh *a2i_dh_params(const struct asn1_object *object,
                         struct dh **dhp)
{
	struct dh *dh = NULL;
	int allocated = 0;

	if (!get_dh(&dh, dhp, &allocated))
		return NULL;
	if (!ASN1_IS_UNIVERSAL_TYPE(object, ASN1_SEQUENCE))
		goto err;
	struct asn1_constructed *sequence = (struct asn1_constructed*)object;
	if (sequence->objects_count != 2
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[0], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[1], ASN1_INTEGER))
		goto err;
	struct asn1_integer *p = (struct asn1_integer*)sequence->objects[0];
	struct asn1_integer *g = (struct asn1_integer*)sequence->objects[1];
	if (!asn1_integer_get_bignum(p, &dh->p)
	 || !asn1_integer_get_bignum(g, &dh->g))
		goto err;
	if (dhp)
		*dhp = dh;
	return dh;

err:
	if (allocated)
		dh_free(dh);
	return NULL;
}

struct asn1_object *i2a_dh_params(const struct dh *dh)
{
	struct asn1_constructed *sequence = asn1_sequence_new();
	struct asn1_integer *p = asn1_integer_new();
	struct asn1_integer *g = asn1_integer_new();
	struct asn1_object *ret = NULL;

	if (!sequence
	 || !p
	 || !g
	 || !asn1_integer_set_bignum(p, dh->p)
	 || !asn1_integer_set_bignum(g, dh->g)
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

DER_DEF(dh_params, dh);
PEM_DEF_RPWC(dh_params, dh, "DH PARAMETERS");
