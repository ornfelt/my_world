#ifndef JKSSL_ASN1_H
#define JKSSL_ASN1_H

#include <sys/types.h>

#include <stdint.h>
#include <stddef.h>

struct asn1_octet_string;
struct asn1_constructed;
struct asn1_bit_string;
struct asn1_integer;
struct asn1_boolean;
struct asn1_string;
struct asn1_object;
struct asn1_time;
struct asn1_null;
struct asn1_oid;
struct bignum;
struct bio;

enum asn1_class
{
	ASN1_UNIVERSAL   = 0,
	ASN1_APPLICATION = 1,
	ASN1_CONTEXT     = 2,
	ASN1_PRIVATE     = 3,
};

enum asn1_pc
{
	ASN1_PRIMITIVE   = 0,
	ASN1_CONSTRUCTED = 1,
};

enum asn1_universal_type
{
	ASN1_EOC               = 0,
	ASN1_BOOLEAN           = 1,
	ASN1_INTEGER           = 2,
	ASN1_BIT_STRING        = 3,
	ASN1_OCTET_STRING      = 4,
	ASN1_NULL              = 5,
	ASN1_OBJECT_IDENTIFIER = 6,
	ASN1_OBJECT_DESCRIPTOR = 7,
	ASN1_EXTERNAL          = 8,
	ASN1_REAL              = 9,
	ASN1_ENUMERATED        = 10,
	ASN1_EMBEDDED_PDV      = 11,
	ASN1_UTF8_STRING       = 12,
	ASN1_RELATIVE_OID      = 13,
	ASN1_TIME              = 14,
	ASN1_SEQUENCE          = 16,
	ASN1_SET               = 17,
	ASN1_NUMERIC_STRING    = 18,
	ASN1_PRINTABLE_STRING  = 19,
	ASN1_T61_STRING        = 20,
	ASN1_VIDEOTEX_STRING   = 21,
	ASN1_IA5_STRING        = 22,
	ASN1_UTC_TIME          = 23,
	ASN1_GENERALIZED_TIME  = 24,
	ASN1_GRAPHIC_STRING    = 25,
	ASN1_VISIBLE_STRING    = 26,
	ASN1_GENERAL_STRING    = 27,
	ASN1_UNIVERSAL_STRING  = 28,
	ASN1_CHARACTER_STRING  = 29,
	ASN1_BMP_STRING        = 30,
	ASN1_DATE              = 31,
	ASN1_TIME_OF_DAY       = 32,
	ASN1_DATE_TIME         = 33,
	ASN1_DURATION          = 34,
	ASN1_OID_IRI           = 35,
	ASN1_RELATIVE_OID_IRI  = 36,
};

void asn1_object_free(struct asn1_object *object);
int asn1_object_up_ref(struct asn1_object *object);
struct asn1_object *asn1_object_dup(const struct asn1_object *object);
uint32_t asn1_object_get_type(const struct asn1_object *object);
int asn1_object_get_class(const struct asn1_object *object);
int asn1_object_get_constructed(const struct asn1_object *object);

struct asn1_constructed *asn1_constructed_new(enum asn1_class class,
                                              uint32_t type);
struct asn1_constructed *asn1_sequence_new(void);
struct asn1_constructed *asn1_set_new(void);
void asn1_constructed_free(struct asn1_constructed *constructed);
struct asn1_constructed *asn1_constructed_dup(const struct asn1_constructed *constructed);
int asn1_constructed_add1(struct asn1_constructed *constructed,
                          struct asn1_object *object);
int asn1_constructed_get0(const struct asn1_constructed *constructed,
                          struct asn1_object ***objects,
                          size_t *size);

struct asn1_primitive *asn1_primitive_new(enum asn1_class class,
                                          uint32_t type);
void asn1_primitive_free(struct asn1_primitive *primitive);
struct asn1_primitive *asn1_primitive_dup(const struct asn1_primitive *primitive);
int asn1_primitive_get(const struct asn1_primitive *primitive,
                       uint8_t *data, size_t *size);

struct asn1_integer *asn1_integer_new(void);
void asn1_integer_free(struct asn1_integer *integer);
struct asn1_integer *asn1_integer_dup(const struct asn1_integer *integer);
int asn1_integer_set_uint64(struct asn1_integer *integer, uint64_t v);
int asn1_integer_set_bignum(struct asn1_integer *integer,
                            struct bignum *bignum);
int asn1_integer_set0_data(struct asn1_integer *integer, uint8_t *data,
                           size_t size);
int asn1_integer_set1_data(struct asn1_integer *integer, const uint8_t *data,
                           size_t size);
int asn1_integer_get_uint64(const struct asn1_integer *integer, uint64_t *v);
int asn1_integer_get_bignum(const struct asn1_integer *integer,
                            struct bignum **bignum);

struct asn1_oid *asn1_oid_new(void);
void asn1_oid_free(struct asn1_oid *oid);
struct asn1_oid *asn1_oid_dup(const struct asn1_oid *iod);
int asn1_oid_cmp(const struct asn1_oid *oid1, const struct asn1_oid *oid2);
int asn1_oid_cmp_uint32(const struct asn1_oid *oid, const uint32_t *data,
                        size_t size);
int asn1_oid_set0(struct asn1_oid *oid, uint32_t *values, size_t count);
int asn1_oid_set1(struct asn1_oid *oid, const uint32_t *values, size_t count);
int asn1_oid_get0(const struct asn1_oid *oid, uint32_t **values, size_t *count);
int asn1_oid_get1(const struct asn1_oid *oid, uint32_t *values, size_t *count);

struct asn1_null *asn1_null_new(void);
void asn1_null_free(struct asn1_null *null);
struct asn1_null *asn1_null_dup(const struct asn1_null *null);

struct asn1_bit_string *asn1_bit_string_new(void);
void asn1_bit_string_free(struct asn1_bit_string *bitstring);
struct asn1_bit_string *asn1_bit_string_dup(const struct asn1_bit_string *bitstring);
int asn1_bit_string_set0(struct asn1_bit_string *bitstring, uint8_t *data,
                         size_t bits);
int asn1_bit_string_set1(struct asn1_bit_string *bitstring,
                         const uint8_t *data, size_t bits);
int asn1_bit_string_get(const struct asn1_bit_string *bitstring,
                        uint8_t *data, size_t *bits);

struct asn1_string *asn1_string_new(enum asn1_universal_type type);
struct asn1_string *asn1_utf8_string_new(void);
struct asn1_string *asn1_numeric_string_new(void);
struct asn1_string *asn1_printable_string_new(void);
struct asn1_string *asn1_t61_string_new(void);
struct asn1_string *asn1_videotex_string_new(void);
struct asn1_string *asn1_ia5_string_new(void);
struct asn1_string *asn1_graphic_string_new(void);
struct asn1_string *asn1_visible_string_new(void);
struct asn1_string *asn1_general_string_new(void);
struct asn1_string *asn1_universal_string_new(void);
struct asn1_string *asn1_bmp_string_new(void);
void asn1_string_free(struct asn1_string *string);
struct asn1_string *asn1_string_dup(const struct asn1_string *string);
int asn1_string_set0(struct asn1_string *string,
                     char *data, size_t size);
int asn1_string_set1(struct asn1_string *string,
                     char *data, size_t size);
int asn1_string_get(const struct asn1_string *string,
                    char *data, size_t *size);

struct asn1_time *asn1_time_new(enum asn1_universal_type type);
struct asn1_time *asn1_utc_time_new(void);
struct asn1_time *asn1_generalized_time_new(void);
void asn1_time_free(struct asn1_time *time);
struct asn1_time *asn1_time_dup(const struct asn1_time *time);
int asn1_time_print(struct bio *bio, const struct asn1_time *time);
time_t asn1_time_get(const struct asn1_time *time);

struct asn1_octet_string *asn1_octet_string_new(void);
void asn1_octet_string_free(struct asn1_octet_string *octetstring);
struct asn1_octet_string *asn1_octet_string_dup(const struct asn1_octet_string *octetstring);
int asn1_octet_string_set0(struct asn1_octet_string *octetstring, uint8_t *data,
                           size_t size);
int asn1_octet_string_set1(struct asn1_octet_string *octetstring,
                           const uint8_t *data, size_t size);
int asn1_octet_string_get(const struct asn1_octet_string *octetstring,
                          uint8_t *data, size_t *size);

struct asn1_boolean *asn1_boolean_new(void);
void asn1_boolean_free(struct asn1_boolean *boolean);
struct asn1_boolean *asn1_boolean_dup(const struct asn1_boolean *boolean);
int asn1_boolean_get(const struct asn1_boolean *boolean);
void asn1_boolean_set(struct asn1_boolean *boolean, int value);

int asn1_decode_spki(const struct asn1_object *object,
                     struct asn1_oid **algorithm,
                     struct asn1_object **parameters,
                     struct asn1_bit_string **pubkey);
struct asn1_object *asn1_encode_spki(struct asn1_oid *algorithm,
                                     struct asn1_object *parameters,
                                     struct asn1_bit_string *pubkey);

#endif
