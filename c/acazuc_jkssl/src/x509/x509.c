#include "x509/x509.h"
#include "asn1/asn1.h"
#include "oid/oid.h"

#include <jkssl/bio.h>
#include <jkssl/evp.h>
#include <jkssl/oid.h>

#include <inttypes.h>
#include <stdlib.h>
#include <time.h>

struct x509 *x509_new(void)
{
	struct x509 *x509 = calloc(sizeof(*x509), 1);
	if (!x509)
		return NULL;
	refcount_init(&x509->refcount, 1);
	return x509;
}

void x509_free(struct x509 *x509)
{
	if (!x509)
		return;
	if (refcount_dec(&x509->refcount))
		return;
	asn1_integer_free(x509->serial);
	x509_name_free(x509->issuer);
	x509_name_free(x509->subject);
	asn1_time_free(x509->not_before);
	asn1_time_free(x509->not_after);
	evp_pkey_free(x509->pkey);
	asn1_bit_string_free(x509->issuer_uid);
	asn1_bit_string_free(x509->subject_uid);
	for (size_t i = 0; i < x509->extensions_count; ++i)
		x509_extension_free(x509->extensions[i]);
	free(x509->extensions);
	x509_algor_free(x509->signature);
	x509_algor_free(x509->sigalg);
	asn1_bit_string_free(x509->signature_value);
	free(x509);
}

int x509_up_ref(struct x509 *x509)
{
	return refcount_inc(&x509->refcount);
}

int x509_get_version(const struct x509 *x509)
{
	return x509->version;
}

struct asn1_integer *x509_get_serial_number(struct x509 *x509)
{
	return x509->serial;
}

const struct asn1_integer *x509_get0_serial_number(const struct x509 *x509)
{
	return x509->serial;
}

int x509_add_ext(struct x509 *x509, struct x509_extension *extension)
{
	struct x509_extension **extensions = realloc(x509->extensions,
	                                             sizeof(*extensions) * (x509->extensions_count + 1));
	if (!extensions)
		return 0;
	x509->extensions = extensions;
	x509->extensions[x509->extensions_count++] = extension;
	return 1;
}

struct x509_name *x509_get_subject_name(struct x509 *x509)
{
	return x509->subject;
}

struct x509_name *x509_get_issuer_name(struct x509 *x509)
{
	return x509->issuer;
}

const struct asn1_time *x509_get0_not_before(const struct x509 *x509)
{
	return x509->not_before;
}

const struct asn1_time *x509_get0_not_after(const struct x509 *x509)
{
	return x509->not_after;
}

int x509_print(struct bio *bio, struct x509 *x509)
{
	char oid_buf[128];
	bio_printf(bio, "Certificate:\n");
	bio_printf(bio, "    Data:\n");
	bio_printf(bio, "        Version: %d (0x%x)\n",
	           x509->version + 1, x509->version);
	uint64_t serial;
	if (asn1_integer_get_uint64(x509->serial, &serial))
	{
		if (serial < 0x100000000)
			bio_printf(bio, "        Serial Number: %" PRIu32 " (0x08%" PRIx32 ")\n",
			           (uint32_t)serial, (uint32_t)serial);
		else
			bio_printf(bio, "        Serial Number: %" PRIu64 " (0x%016" PRIx64 ")\n",
			           serial, serial);
	}
	else
	{
		bio_printf(bio, "        Serial Number:\n");
		bio_printf(bio, "            ");
		for (size_t i = 0; i < x509->serial->size; ++i)
		{
			if (i)
				bio_printf(bio, ":");
			bio_printf(bio, "%02x", x509->serial->data[i]);
		}
		bio_printf(bio, "\n");
	}
	bio_printf(bio, "        Signature Algorithm: %s\n",
	           oid2txt(x509->sigalg->oid, oid_buf, sizeof(oid_buf)));
	bio_printf(bio, "        Issuer: ");
	x509_name_print(bio, x509->issuer);
	bio_printf(bio, "\n");
	bio_printf(bio, "        Validity\n");
	bio_printf(bio, "            Not Before: ");
	asn1_time_print(bio, x509->not_before);
	bio_printf(bio, "\n");
	bio_printf(bio, "            Not After : ");
	asn1_time_print(bio, x509->not_after);
	bio_printf(bio, "\n");
	bio_printf(bio, "        Subject: ");
	x509_name_print(bio, x509->subject);
	bio_printf(bio, "\n");
	bio_printf(bio, "        Subject Public Key Info:\n");
	/* XXX decode public key */
	if (x509->extensions_count)
		bio_printf(bio, "        X509v3 extensions:\n");
	for (size_t i = 0; i < x509->extensions_count; ++i)
	{
		struct x509_extension *ext = x509->extensions[i];
		bio_printf(bio, "            X509v3 %s:%s\n",
		           oid2txt(ext->oid, oid_buf, sizeof(oid_buf)),
		           ext->critical ? " critical" : "");
	}
	bio_printf(bio, "    Signature Algorithm: %s\n",
	           oid2txt(x509->sigalg->oid, oid_buf, sizeof(oid_buf)));
	for (size_t i = 0; i < (x509->signature_value->bits + 7) / 8; ++i)
	{
		if (i % 18 == 0)
			bio_printf(bio, "         ");
		bio_printf(bio, "%02x", x509->signature_value->data[i]);
		if (i != (x509->signature_value->bits + 7) / 8 - 1)
		{
			bio_printf(bio, ":");
			if (i % 18 == 17)
				bio_printf(bio, "\n");
		}
	}
	bio_printf(bio, "\n");
	return 1;
}

int x509_print_fp(FILE *fp, struct x509 *x509)
{
	struct bio *bio = bio_new_fp(fp, BIO_NOCLOSE);
	if (!bio)
		return 0;
	int ret = x509_print(bio, x509);
	bio_vfree(bio);
	return ret;
}

struct x509_algor *x509_algor_new(void)
{
	return calloc(sizeof(struct x509_algor), 1);
}

void x509_algor_free(struct x509_algor *algor)
{
	if (!algor)
		return;
	asn1_oid_free(algor->oid);
	asn1_object_free(algor->params);
	free(algor);
}

int x509_algor_set0(struct x509_algor *algor, struct asn1_oid *oid,
                    struct asn1_object *params)
{
	/* XXX test that params are ok given the oid */
	asn1_oid_free(algor->oid);
	asn1_object_free(algor->params);
	algor->oid = oid;
	algor->params = params;
	return 1;
}

void x509_algor_get0(struct x509_algor *algor,
                     const struct asn1_oid **oid,
                     const struct asn1_object **params)
{
	*oid = algor->oid;
	*params = algor->params;
}

struct x509_name *x509_name_new(void)
{
	return calloc(sizeof(struct x509_name), 1);
}

void x509_name_free(struct x509_name *name)
{
	if (!name)
		return;
	for (size_t i = 0; i < name->entries_count; ++i)
		x509_name_entry_free(name->entries[i]);
	free(name);
}

int x509_name_add0_entry(struct x509_name *name,
                         struct x509_name_entry *entry)
{
	for (size_t i = 0; i < name->entries_count; ++i)
	{
		if (!asn1_oid_cmp(name->entries[i]->type, entry->type))
			return 0;
	}
	/* XXX test if oid is valid and string is corresponding to type */
	struct x509_name_entry **entries = realloc(name->entries,
	                                           sizeof(*entries) * (name->entries_count + 1));
	if (!entries)
		return 0;
	name->entries = entries;
	name->entries[name->entries_count++] = entry;
	return 1;
}

int x509_name_print(struct bio *bio, struct x509_name *name)
{
	char oid_buf[128];
	for (size_t i = 0; i < name->entries_count; ++i)
	{
		char *s;
		if (!asn1_oid_cmp_uint32(name->entries[i]->type, oid_commonName, sizeof(oid_commonName) / sizeof(*oid_commonName)))
			s = "CN";
		else if (!asn1_oid_cmp_uint32(name->entries[i]->type, oid_localityName, sizeof(oid_localityName) / sizeof(*oid_localityName)))
			s = "L";
		else if (!asn1_oid_cmp_uint32(name->entries[i]->type, oid_countryName, sizeof(oid_countryName) / sizeof(*oid_countryName)))
			s = "C";
		else if (!asn1_oid_cmp_uint32(name->entries[i]->type, oid_stateOrProvinceName, sizeof(oid_stateOrProvinceName) / sizeof(*oid_stateOrProvinceName)))
			s = "ST";
		else if (!asn1_oid_cmp_uint32(name->entries[i]->type, oid_organizationName, sizeof(oid_organizationName) / sizeof(*oid_organizationName)))
			s = "O";
		else if (!asn1_oid_cmp_uint32(name->entries[i]->type, oid_organizationalUnitName, sizeof(oid_organizationalUnitName) / sizeof(*oid_organizationalUnitName)))
			s = "OU";
		else if (!asn1_oid_cmp_uint32(name->entries[i]->type, oid_emailAddress, sizeof(oid_emailAddress) / sizeof(*oid_emailAddress)))
			s = "emailAddress";
		else
			s = oid2txt(name->entries[i]->type,
			            oid_buf, sizeof(oid_buf));
		bio_printf(bio, "%s", s);
		bio_printf(bio, " = ");
		bio_printf(bio, "%.*s", (int)name->entries[i]->value->size,
		           name->entries[i]->value->data);
		if (i != name->entries_count - 1)
			bio_printf(bio, ", ");
	}
	return 1;
}

struct x509_name_entry *x509_name_entry_new(void)
{
	return calloc(sizeof(struct x509_name_entry), 1);
}

void x509_name_entry_free(struct x509_name_entry *entry)
{
	if (!entry)
		return;
	asn1_oid_free(entry->type);
	asn1_string_free(entry->value);
	free(entry);
}

struct asn1_oid *x509_name_entry_get_object(struct x509_name_entry *entry)
{
	return entry->type;
}

struct asn1_string *x509_name_entry_get_data(struct x509_name_entry *entry)
{
	return entry->value;
}

struct x509_extension *x509_extension_new(void)
{
	return calloc(sizeof(struct x509_extension), 1);
}

void x509_extension_free(struct x509_extension *extension)
{
	if (!extension)
		return;
	asn1_oid_free(extension->oid);
	asn1_octet_string_free(extension->object);
	free(extension);
}

struct x509_extension *x509_extension_create(struct x509_extension **extensionp,
                                             struct asn1_oid *oid,
                                             bool critical,
                                             struct asn1_octet_string *object)
{
	struct x509_extension *extension = NULL;
	if (extensionp)
		extension = *extensionp;
	if (!extension)
	{
		extension = x509_extension_new();
		if (!extension)
			return NULL;
	}
	asn1_oid_free(extension->oid);
	asn1_octet_string_free(extension->object);
	extension->oid = oid;
	extension->critical = critical;
	extension->object = object;
	if (extensionp)
		*extensionp = extension;
	return extension;
}
