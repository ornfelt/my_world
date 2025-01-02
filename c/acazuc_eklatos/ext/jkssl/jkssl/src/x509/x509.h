#ifndef JKSSL_X509_X509_H
#define JKSSL_X509_X509_H

#include "refcount.h"

#include <jkssl/x509.h>
#include <jkssl/asn1.h>

#include <stdbool.h>

struct x509_algor
{
	struct asn1_oid *oid;
	struct asn1_object *params;
};

struct x509_name_entry
{
	struct asn1_oid *type;
	struct asn1_string *value;
};

struct x509_name
{
	struct x509_name_entry **entries;
	size_t entries_count;
};

struct x509_extension
{
	struct asn1_oid *oid;
	bool critical;
	struct asn1_octet_string *object;
};

struct x509
{
	refcount_t refcount;
	uint8_t version;
	struct asn1_integer *serial;
	struct x509_name *issuer;
	struct x509_name *subject;
	struct asn1_time *not_before;
	struct asn1_time *not_after;
	struct evp_pkey *pkey;
	struct asn1_bit_string *issuer_uid;
	struct asn1_bit_string *subject_uid;
	struct x509_extension **extensions;
	size_t extensions_count;
	struct x509_algor *signature;
	struct x509_algor *sigalg;
	struct asn1_bit_string *signature_value;
};

#endif
