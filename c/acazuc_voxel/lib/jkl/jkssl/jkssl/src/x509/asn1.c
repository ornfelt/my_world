#include "asn1/asn1.h"
#include "x509/x509.h"
#include "der/der.h"
#include "pem/pem.h"

#include <jkssl/bio.h>

#include <stdlib.h>
#include <string.h>

struct x509_algor *a2i_x509_algor(const struct asn1_object *object,
                                  struct x509_algor **algorp)
{
	if (!ASN1_IS_UNIVERSAL_TYPE(object, ASN1_SEQUENCE))
		return NULL;
	struct asn1_constructed *sequence = (struct asn1_constructed*)object;
	if (sequence->objects_count != 2
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[0], ASN1_OBJECT_IDENTIFIER))
		return NULL;
	struct x509_algor *algor = x509_algor_new();
	struct asn1_oid *oid = asn1_oid_dup((const struct asn1_oid*)sequence->objects[0]);
	struct asn1_object *params = asn1_object_dup(sequence->objects[1]);
	if (!algor || !oid || !params)
		goto err;
	if (!x509_algor_set0(algor, oid, params))
		goto err;
	if (algorp)
	{
		x509_algor_free(*algorp);
		*algorp = algor;
	}
	return algor;

err:
	x509_algor_free(algor);
	asn1_oid_free(oid);
	asn1_object_free(params);
	return NULL;
}

struct asn1_object *i2a_x509_algor(struct x509_algor *algor)
{
	struct asn1_constructed *sequence = asn1_sequence_new();
	struct asn1_object *ret = NULL;
	if (!sequence
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)algor->oid)
	 || !asn1_constructed_add1(sequence, algor->params))
		goto end;
	ret = (struct asn1_object*)sequence;

end:
	if (!ret)
		asn1_constructed_free(sequence);
	return ret;
}

DER_DEF(x509_algor, x509_algor);

struct x509_name_entry *a2i_x509_name_entry(const struct asn1_object *object,
                                            struct x509_name_entry **entryp)
{
	if (!ASN1_IS_UNIVERSAL_TYPE(object, ASN1_SET))
		return NULL;
	struct asn1_constructed *set = (struct asn1_constructed*)object;
	if (set->objects_count != 1
	 || !ASN1_IS_UNIVERSAL_TYPE(set->objects[0], ASN1_SEQUENCE))
		return NULL;
	const struct asn1_constructed *sequence = (const struct asn1_constructed*)set->objects[0];
	if (sequence->objects_count != 2
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[0], ASN1_OBJECT_IDENTIFIER)
	 || !ASN1_IS_UNIVERSAL_STRING(sequence->objects[1]))
		return NULL;
	struct x509_name_entry *name_entry = x509_name_entry_new();
	if (!name_entry)
		return NULL;
	name_entry->type = asn1_oid_dup((const struct asn1_oid*)sequence->objects[0]);
	name_entry->value = (struct asn1_string*)asn1_object_dup(sequence->objects[1]);
	if (!name_entry->type
	 || !name_entry->value)
		goto err;
	if (entryp)
	{
		x509_name_entry_free(*entryp);
		*entryp = name_entry;
	}
	return name_entry;

err:
	x509_name_entry_free(name_entry);
	return NULL;
}

struct asn1_object *i2a_x509_name_entry(struct x509_name_entry *entry)
{
	struct asn1_constructed *set = asn1_set_new();
	struct asn1_constructed *sequence = asn1_sequence_new();
	struct asn1_object *ret = NULL;
	if (!set
	 || !sequence
	 || !entry->type
	 || !entry->value
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)entry->type)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)entry->value)
	 || !asn1_constructed_add1(set, (struct asn1_object*)sequence))
		goto end;
	ret = (struct asn1_object*)set;

end:
	if (!ret)
		asn1_constructed_free(set);
	asn1_constructed_free(sequence);
	return ret;
}

DER_DEF(x509_name_entry, x509_name_entry);

struct x509_name *a2i_x509_name(const struct asn1_object *object,
                                struct x509_name **namep)
{
	if (!ASN1_IS_UNIVERSAL_TYPE(object, ASN1_SEQUENCE))
		return NULL;
	struct asn1_constructed *sequence = (struct asn1_constructed*)object;
	if (sequence->objects_count < 1)
		return NULL;
	struct x509_name *name = x509_name_new();
	if (!name)
		return NULL;
	for (size_t i = 0; i < sequence->objects_count; ++i)
	{
		if (!ASN1_IS_UNIVERSAL_TYPE(sequence->objects[i], ASN1_SET))
			goto err;
		struct x509_name_entry *name_entry = NULL;
		if (!a2i_x509_name_entry(sequence->objects[i], &name_entry)
		 || !x509_name_add0_entry(name, name_entry))
		{
			x509_name_entry_free(name_entry);
			goto err;
		}
	}
	if (namep)
	{
		x509_name_free(*namep);
		*namep = name;
	}
	return name;

err:
	x509_name_free(name);
	return NULL;
}

struct asn1_object *i2a_x509_name(struct x509_name *name)
{
	struct asn1_constructed *sequence = asn1_sequence_new();
	struct asn1_object *ret = NULL;
	if (!sequence)
		goto end;
	for (size_t i = 0; i < name->entries_count; ++i)
	{
		struct asn1_object *name_entry = i2a_x509_name_entry(name->entries[i]);
		if (!name_entry
		 || !asn1_constructed_add1(sequence, name_entry))
		{
			asn1_object_free(name_entry);
			goto end;
		}
	}
	ret = (struct asn1_object*)sequence;

end:
	if (!ret)
		asn1_constructed_free(sequence);
	return ret;
}

DER_DEF(x509_name, x509_name);

struct x509_extension *a2i_x509_extension(const struct asn1_object *object,
                                          struct x509_extension **extensionp)
{
	if (!ASN1_IS_UNIVERSAL_TYPE(object, ASN1_SEQUENCE))
		return NULL;
	struct asn1_constructed *sequence = (struct asn1_constructed*)object;
	if (sequence->objects_count < 2
	 || sequence->objects_count > 3
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[0], ASN1_OBJECT_IDENTIFIER))
		return NULL;
	const struct asn1_object *obj;
	bool critical = false;
	if (ASN1_IS_UNIVERSAL_TYPE(sequence->objects[1], ASN1_BOOLEAN))
	{
		if (sequence->objects_count != 3)
			return NULL;
		critical = ((const struct asn1_boolean*)sequence->objects[1])->value;
		obj = sequence->objects[2];
	}
	else
	{
		obj = sequence->objects[1];
	}
	if (!ASN1_IS_UNIVERSAL_TYPE(obj, ASN1_OCTET_STRING))
		return NULL;
	struct asn1_oid *oid = asn1_oid_dup((struct asn1_oid*)sequence->objects[0]);
	struct asn1_octet_string *os = asn1_octet_string_dup((struct asn1_octet_string*)obj);
	if (!oid || !os)
	{
		asn1_oid_free(oid);
		asn1_octet_string_free(os);
		return NULL;
	}
	struct x509_extension *extension = x509_extension_create(NULL,
	                                                         oid,
	                                                         critical,
	                                                         os);
	if (!extension)
	{
		asn1_oid_free(oid);
		asn1_octet_string_free(os);
		return NULL;
	}
	if (extensionp)
	{
		x509_extension_free(*extensionp);
		*extensionp = extension;
	}
	return extension;
}

struct asn1_object *i2a_x509_extension(struct x509_extension *extension)
{
	struct asn1_constructed *sequence = asn1_sequence_new();
	struct asn1_boolean *critical = NULL;
	struct asn1_object *ret = NULL;
	if (extension->critical)
	{
		critical = asn1_boolean_new();
		if (!critical)
			goto end;
		critical->value = true;
	}
	if (!sequence
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)extension->oid)
	 || (critical && !asn1_constructed_add1(sequence, (struct asn1_object*)critical))
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)extension->object))
		goto end;
	ret = (struct asn1_object*)sequence;

end:
	if (!ret)
		asn1_constructed_free(sequence);
	asn1_boolean_free(critical);
	return ret;
}

DER_DEF(x509_extension, x509_extension);

struct x509_pubkey *a2i_x509_pubkey(const struct asn1_object *object,
                                    struct x509_pubkey **pubkeyp)
{
	if (!ASN1_IS_UNIVERSAL_TYPE(object, ASN1_SEQUENCE))
		return NULL;
	struct asn1_constructed *sequence = (struct asn1_constructed*)object;
	if (sequence->objects_count != 2
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[0], ASN1_SEQUENCE)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[1], ASN1_BIT_STRING))
		return NULL;
	struct x509_pubkey *pubkey = x509_pubkey_new();
	if (!pubkey)
		return NULL;
	pubkey->pubkey = asn1_bit_string_dup((const struct asn1_bit_string*)sequence->objects[1]);
	if (!pubkey->pubkey
	 || !a2i_x509_algor(sequence->objects[0], &pubkey->tbs_sigalg))
	{
		x509_pubkey_free(pubkey);
		return NULL;
	}
	if (pubkeyp)
	{
		x509_pubkey_free(*pubkeyp);
		*pubkeyp = pubkey;
	}
	return pubkey;
}

struct asn1_object *i2a_x509_pubkey(struct x509_pubkey *pubkey)
{
	if (!pubkey->tbs_sigalg
	 || !pubkey->pubkey)
		return NULL;
	struct asn1_constructed *sequence = asn1_sequence_new();
	struct asn1_object *tbs_sigalg = i2a_x509_algor(pubkey->tbs_sigalg);
	struct asn1_object *ret = NULL;
	if (!sequence
	 || !asn1_constructed_add1(sequence, tbs_sigalg)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)pubkey->pubkey))
		goto end;
	ret = (struct asn1_object*)sequence;

end:
	if (!ret)
		asn1_constructed_free(sequence);
	asn1_object_free(tbs_sigalg);
	return ret;
}

DER_DEF(x509_pubkey, x509_pubkey);

static int a2i_version(struct x509 *x509,
                       const struct asn1_constructed *sequence)
{
	uint64_t v;
	if (sequence->objects_count != 1
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[0], ASN1_INTEGER)
	 || !asn1_integer_get_uint64((const struct asn1_integer*)sequence->objects[0], &v)
	 || v > X509_VERSION_3)
		return 0;
	x509->version = v;
	return 1;
}

static int a2i_serial(struct x509 *x509,
                      const struct asn1_integer *serial)
{
	struct asn1_integer *dup = asn1_integer_dup(serial);
	if (!dup)
		return 0;
	asn1_integer_free(x509->serial);
	x509->serial = dup;
	return 1;
}

static int a2i_validity(struct x509 *x509,
                        const struct asn1_constructed *sequence)
{
	if (sequence->objects_count != 2
	 || !ASN1_IS_UNIVERSAL_TIME(sequence->objects[0])
	 || !ASN1_IS_UNIVERSAL_TIME(sequence->objects[1]))
		return 0;
	struct asn1_time *not_before = asn1_time_dup((const struct asn1_time*)sequence->objects[0]);
	struct asn1_time *not_after = asn1_time_dup((const struct asn1_time*)sequence->objects[1]);
	if (!not_before || !not_after)
	{
		asn1_time_free(not_before);
		asn1_time_free(not_after);
		return 0;
	}
	asn1_time_free(x509->not_before);
	asn1_time_free(x509->not_after);
	x509->not_before = not_before;
	x509->not_after = not_after;
	return 1;
}

static int a2i_x509_extensions(struct x509 *x509,
                               const struct asn1_constructed *sequence)
{
	if (sequence->objects_count != 1
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[0], ASN1_SEQUENCE))
		return 0;
	for (size_t i = 0; i < x509->extensions_count; ++i)
		x509_extension_free(x509->extensions[i]);
	free(x509->extensions);
	x509->extensions = NULL;
	x509->extensions_count = 0;
	struct asn1_constructed *list = (struct asn1_constructed*)sequence->objects[0];
	for (size_t i = 0; i < list->objects_count; ++i)
	{
		struct x509_extension *extension = NULL;
		if (!a2i_x509_extension(list->objects[i], &extension)
		 || !x509_add_ext(x509, extension))
		{
			x509_extension_free(extension);
			return 0;
		}
	}
	return 1;
}

static int a2i_certificate(struct x509 *x509,
                           const struct asn1_constructed *sequence)
{
	if (sequence->objects_count < 7
	 || !ASN1_IS_CONTEXT_TYPE(sequence->objects[0], ASN1_CONSTRUCTED, 0)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[1], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[2], ASN1_SEQUENCE)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[3], ASN1_SEQUENCE)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[4], ASN1_SEQUENCE)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[5], ASN1_SEQUENCE)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[6], ASN1_SEQUENCE)
	 || !a2i_version(x509, (const struct asn1_constructed*)sequence->objects[0])
	 || !a2i_serial(x509, (const struct asn1_integer*)sequence->objects[1])
	 || !a2i_x509_algor(sequence->objects[2], &x509->signature)
	 || !a2i_x509_name(sequence->objects[3], &x509->issuer)
	 || !a2i_validity(x509, (const struct asn1_constructed*)sequence->objects[4])
	 || !a2i_x509_name(sequence->objects[5], &x509->subject)
	 || !a2i_x509_pubkey(sequence->objects[6], &x509->pubkey))
		return 0;
	if (sequence->objects_count <= 7)
		return 1;
	size_t n = 7;
	if (ASN1_IS_CONTEXT_TYPE(sequence->objects[n], ASN1_PRIMITIVE, 1))
	{
		if (x509->version < X509_VERSION_2)
			return 0;
		n++;
		/* XXX issuer uid */
		return 0;
	}
	if (ASN1_IS_CONTEXT_TYPE(sequence->objects[n], ASN1_PRIMITIVE, 2))
	{
		if (x509->version < X509_VERSION_2)
			return 0;
		n++;
		/* XXX subject uid */
		return 0;
	}
	if (ASN1_IS_CONTEXT_TYPE(sequence->objects[n], ASN1_CONSTRUCTED, 3))
	{
		if (x509->version < X509_VERSION_3)
			return 0;
		if (!a2i_x509_extensions(x509, (const struct asn1_constructed*)sequence->objects[n]))
			return 0;
		n++;
	}
	if (n != sequence->objects_count)
		return 0;
	return 1;
}

static int a2i_signature_value(struct x509 *x509,
                               const struct asn1_bit_string *bitstring)
{
	struct asn1_bit_string *dup = asn1_bit_string_dup(bitstring);
	if (!dup)
		return 0;
	asn1_bit_string_free(x509->signature_value);
	x509->signature_value = dup;
	return 1;
}

struct x509 *a2i_x509(const struct asn1_object *object,
                      struct x509 **x509)
{
	if (!ASN1_IS_UNIVERSAL_TYPE(object, ASN1_SEQUENCE))
		return NULL;
	const struct asn1_constructed *sequence = (const struct asn1_constructed*)object;
	if (sequence->objects_count != 3
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[0], ASN1_SEQUENCE)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[1], ASN1_SEQUENCE)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[2], ASN1_BIT_STRING)
	 || !a2i_certificate(*x509, (const struct asn1_constructed*)sequence->objects[0])
	 || !a2i_x509_algor(sequence->objects[1], &(*x509)->sigalg)
	 || !a2i_signature_value(*x509, (const struct asn1_bit_string*)sequence->objects[2]))
		return NULL;
	return *x509;
}

static struct asn1_object *i2a_version(struct x509 *x509)
{
	struct asn1_constructed *sequence = asn1_constructed_new(ASN1_CONTEXT, 0);
	struct asn1_integer *integer = asn1_integer_new();
	struct asn1_object *ret = NULL;
	if (!sequence
	 || !integer
	 || !asn1_integer_set_uint64(integer, x509->version)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)integer))
		goto end;
	ret = (struct asn1_object*)sequence;

end:
	if (!ret)
		asn1_constructed_free(sequence);
	return ret;
}

static struct asn1_object *i2a_validity(struct x509 *x509)
{
	struct asn1_constructed *sequence = asn1_sequence_new();
	struct asn1_object *ret = NULL;
	if (!sequence
	 || !x509->not_before
	 || !x509->not_after
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)x509->not_before)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)x509->not_after))
		goto end;
	ret = (struct asn1_object*)sequence;

end:
	if (!ret)
		asn1_constructed_free(sequence);
	return ret;
}

static struct asn1_object *i2a_x509_extensions(struct x509 *x509)
{
	struct asn1_constructed *extensions = asn1_constructed_new(ASN1_CONTEXT, 3);
	struct asn1_constructed *sequence = asn1_sequence_new();
	struct asn1_object *ret = NULL;
	if (!extensions
	 || !sequence)
		goto end;
	for (size_t i = 0; i < x509->extensions_count; ++i)
	{
		struct asn1_object *extension = i2a_x509_extension(x509->extensions[i]);
		if (!extension
		 || !asn1_constructed_add1(sequence, extension))
		{
			asn1_object_free(extension);
			goto end;
		}
	}
	if (!asn1_constructed_add1(extensions, (struct asn1_object*)sequence))
		goto end;
	ret = (struct asn1_object*)extensions;

end:
	if (!ret)
		asn1_constructed_free(extensions);
	asn1_constructed_free(sequence);
	return ret;
}

static struct asn1_object *i2a_certificate(struct x509 *x509)
{
	struct asn1_constructed *sequence = asn1_sequence_new();
	struct asn1_object *version = i2a_version(x509);
	struct asn1_object *signature = i2a_x509_algor(x509->signature);
	struct asn1_object *issuer = i2a_x509_name(x509->issuer);
	struct asn1_object *validity = i2a_validity(x509);
	struct asn1_object *subject = i2a_x509_name(x509->subject);
	struct asn1_object *subject_pki = i2a_x509_pubkey(x509->pubkey);
	struct asn1_object *extensions = NULL;
	struct asn1_object *ret = NULL;
	if (!sequence
	 || !version
	 || !x509->serial
	 || !signature
	 || !issuer
	 || !validity
	 || !subject
	 || !subject_pki
	 || !asn1_constructed_add1(sequence, version)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)x509->serial)
	 || !asn1_constructed_add1(sequence, signature)
	 || !asn1_constructed_add1(sequence, issuer)
	 || !asn1_constructed_add1(sequence, validity)
	 || !asn1_constructed_add1(sequence, subject)
	 || !asn1_constructed_add1(sequence, subject_pki))
		goto end;
	/* XXX issuer uid */
	/* XXX subject uid */
	if (x509->extensions_count)
	{
		extensions = i2a_x509_extensions(x509);
		if (!extensions
		 || !asn1_constructed_add1(sequence, extensions))
			goto end;
	}
	ret = (struct asn1_object*)sequence;

end:
	if (!ret)
		asn1_constructed_free(sequence);
	asn1_object_free(version);
	asn1_object_free(signature);
	asn1_object_free(issuer);
	asn1_object_free(validity);
	asn1_object_free(subject);
	asn1_object_free(extensions);
	return ret;
}

struct asn1_object *i2a_x509(struct x509 *x509)
{
	struct asn1_constructed *sequence = asn1_sequence_new();
	struct asn1_object *certificate = i2a_certificate(x509);
	struct asn1_object *sigalg = i2a_x509_algor(x509->sigalg);
	struct asn1_object *ret = NULL;
	if (!sequence
	 || !certificate
	 || !sigalg
	 || !x509->signature_value
	 || !asn1_constructed_add1(sequence, certificate)
	 || !asn1_constructed_add1(sequence, sigalg)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)x509->signature_value))
		goto end;
	ret = (struct asn1_object*)sequence;

end:
	if (!ret)
		asn1_constructed_free(sequence);
	return ret;
}

DER_DEF(x509, x509);
PEM_DEF_RCWC(x509, x509, "CERTIFICATE");
