#ifndef JKSSL_ASN1_ASN1_H
#define JKSSL_ASN1_ASN1_H

#include <jkssl/asn1.h>

#include <sys/types.h>

#include <stdbool.h>

#define ASN1_IS_UNIVERSAL_TYPE(obj, t) (((obj)->class == ASN1_UNIVERSAL) \
                                     && ((obj)->type == (t)))
#define ASN1_IS_APPLICATION_TYPE(obj, c, t) (((obj)->class == ASN1_APPLICATION) \
                                          && ((obj)->type == (t)) \
                                          && ((obj)->constructed == (c)))
#define ASN1_IS_CONTEXT_TYPE(obj, c, t) (((obj)->class == ASN1_CONTEXT) \
                                      && ((obj)->type == (t)) \
                                      && ((obj)->constructed == (c)))
#define ASN1_IS_PRIVATE_TYPE(obj, c, t) (((obj)->class == ASN1_PRIVATE) \
                                      && ((obj)->type == (t)) \
                                      && ((obj)->constructed == (c)))

#define ASN1_IS_UNIVERSAL_STRING(obj) (((obj)->class == ASN1_UNIVERSAL) \
                                    && (((obj)->type == ASN1_UTF8_STRING) \
                                     || ((obj)->type == ASN1_PRINTABLE_STRING) \
                                     || ((obj)->type == ASN1_T61_STRING) \
                                     || ((obj)->type == ASN1_VIDEOTEX_STRING) \
                                     || ((obj)->type == ASN1_IA5_STRING) \
                                     || ((obj)->type == ASN1_GRAPHIC_STRING) \
                                     || ((obj)->type == ASN1_VISIBLE_STRING) \
                                     || ((obj)->type == ASN1_GENERAL_STRING) \
                                     || ((obj)->type == ASN1_UNIVERSAL_STRING) \
                                     || ((obj)->type == ASN1_BMP_STRING)))

#define ASN1_IS_UNIVERSAL_TIME(obj) (((obj)->class == ASN1_UNIVERSAL) \
                                 && (((obj)->type == ASN1_UTC_TIME) \
                                  || ((obj)->type == ASN1_GENERALIZED_TIME)))

struct asn1_object
{
	uint8_t class;
	uint8_t constructed;
	uint32_t type;
	uint32_t refcount;
};

/* for non-universal types */
struct asn1_primitive
{
	struct asn1_object object;
	uint8_t *data;
	uint32_t size;
};

struct asn1_constructed
{
	struct asn1_object object;
	struct asn1_object **objects;
	size_t objects_count;
};

struct asn1_integer
{
	struct asn1_object object;
	uint8_t *data;
	size_t size;
};

struct asn1_oid
{
	struct asn1_object object;
	uint32_t *values; /* XXX is it really maxed at 32 bits ? */
	size_t values_count;
};

struct asn1_null
{
	struct asn1_object object;
};

struct asn1_bit_string
{
	struct asn1_object object;
	uint8_t *data;
	size_t bits;
};

struct asn1_string
{
	struct asn1_object object;
	char *data;
	size_t size;
};

struct asn1_time
{
	struct asn1_object object;
	time_t value;
};

struct asn1_octet_string
{
	struct asn1_object object;
	uint8_t *data;
	size_t size;
};

struct asn1_boolean
{
	struct asn1_object object;
	bool value;
};

#endif
