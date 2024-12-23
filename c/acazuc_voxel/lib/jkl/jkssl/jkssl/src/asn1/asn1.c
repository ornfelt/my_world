#include "asn1/asn1.h"

#include <jkssl/bignum.h>
#include <jkssl/bio.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <time.h>

static void *asn1_object_new(uint8_t class, uint8_t constructed,
                             uint32_t type, size_t st_size)
{
	struct asn1_object *object = calloc(st_size, 1);
	if (!object)
		return NULL;
	object->class = class;
	object->constructed = constructed;
	object->type = type;
	object->refcount = 1;
	return object;
}

static void asn1_universal_free(struct asn1_object *object)
{
	switch (object->type)
	{
		case ASN1_BOOLEAN:
			asn1_boolean_free((struct asn1_boolean*)object);
			return;
		case ASN1_INTEGER:
			asn1_integer_free((struct asn1_integer*)object);
			return;
		case ASN1_BIT_STRING:
			asn1_bit_string_free((struct asn1_bit_string*)object);
			return;
		case ASN1_OCTET_STRING:
			asn1_octet_string_free((struct asn1_octet_string*)object);
			return;
		case ASN1_NULL:
			asn1_null_free((struct asn1_null*)object);
			return;
		case ASN1_OBJECT_IDENTIFIER:
			asn1_oid_free((struct asn1_oid*)object);
			return;
		case ASN1_UTF8_STRING:
		case ASN1_PRINTABLE_STRING:
		case ASN1_T61_STRING:
		case ASN1_VIDEOTEX_STRING:
		case ASN1_IA5_STRING:
		case ASN1_GRAPHIC_STRING:
		case ASN1_VISIBLE_STRING:
		case ASN1_GENERAL_STRING:
		case ASN1_UNIVERSAL_STRING:
		case ASN1_BMP_STRING:
			asn1_string_free((struct asn1_string*)object);
			return;
		case ASN1_SEQUENCE:
		case ASN1_SET:
			asn1_constructed_free((struct asn1_constructed*)object);
			return;
		case ASN1_UTC_TIME:
		case ASN1_GENERALIZED_TIME:
			asn1_time_free((struct asn1_time*)object);
			return;
		default:
			fprintf(stderr, "(free) unknown object type: 0x%x\n", object->type);
			return;
	}
}

void asn1_object_free(struct asn1_object *object)
{
	if (!object)
		return;
	switch (object->class)
	{
		case ASN1_UNIVERSAL:
			asn1_universal_free(object);
			break;
		case ASN1_APPLICATION:
		case ASN1_CONTEXT:
		case ASN1_PRIVATE:
			if (object->constructed)
				asn1_constructed_free((struct asn1_constructed*)object);
			else
				asn1_primitive_free((struct asn1_primitive*)object);
			break;
	}
}

int asn1_object_up_ref(struct asn1_object *object)
{
	return ++object->refcount;
}

static struct asn1_object *asn1_universal_dup(const struct asn1_object *object)
{
	switch (object->type)
	{
		case ASN1_BOOLEAN:
			return (struct asn1_object*)asn1_boolean_dup((struct asn1_boolean*)object);
		case ASN1_INTEGER:
			return (struct asn1_object*)asn1_integer_dup((struct asn1_integer*)object);
		case ASN1_BIT_STRING:
			return (struct asn1_object*)asn1_bit_string_dup((struct asn1_bit_string*)object);
		case ASN1_OCTET_STRING:
			return (struct asn1_object*)asn1_octet_string_dup((struct asn1_octet_string*)object);
		case ASN1_NULL:
			return (struct asn1_object*)asn1_null_dup((struct asn1_null*)object);
		case ASN1_OBJECT_IDENTIFIER:
			return (struct asn1_object*)(struct asn1_object*)asn1_oid_dup((struct asn1_oid*)object);
		case ASN1_UTF8_STRING:
		case ASN1_PRINTABLE_STRING:
		case ASN1_T61_STRING:
		case ASN1_VIDEOTEX_STRING:
		case ASN1_IA5_STRING:
		case ASN1_GRAPHIC_STRING:
		case ASN1_VISIBLE_STRING:
		case ASN1_GENERAL_STRING:
		case ASN1_UNIVERSAL_STRING:
		case ASN1_BMP_STRING:
			return (struct asn1_object*)(struct asn1_object*)asn1_string_dup((struct asn1_string*)object);
		case ASN1_SEQUENCE:
		case ASN1_SET:
			return (struct asn1_object*)asn1_constructed_dup((struct asn1_constructed*)object);
		case ASN1_UTC_TIME:
		case ASN1_GENERALIZED_TIME:
			return (struct asn1_object*)asn1_time_dup((struct asn1_time*)object);
		default:
			fprintf(stderr, "(dup) unknown object type: 0x%x\n", object->type);
			return NULL;
	}
}

struct asn1_object *asn1_object_dup(const struct asn1_object *object)
{
	if (!object)
		return NULL;
	switch (object->class)
	{
		case ASN1_UNIVERSAL:
			return asn1_universal_dup(object);
		case ASN1_APPLICATION:
		case ASN1_CONTEXT:
		case ASN1_PRIVATE:
			if (object->constructed)
				return (struct asn1_object*)asn1_constructed_dup((struct asn1_constructed*)object);
			return (struct asn1_object*)asn1_primitive_dup((struct asn1_primitive*)object);
	}
	return NULL;
}

struct asn1_constructed *asn1_constructed_new(enum asn1_class class,
                                              uint32_t type)
{
	return asn1_object_new(class, 1, type,
	                       sizeof(struct asn1_constructed));
}

struct asn1_constructed *asn1_sequence_new(void)
{
	return asn1_constructed_new(ASN1_UNIVERSAL, ASN1_SEQUENCE);
}

struct asn1_constructed *asn1_set_new(void)
{
	return asn1_constructed_new(ASN1_UNIVERSAL, ASN1_SET);
}

void asn1_constructed_free(struct asn1_constructed *constructed)
{
	if (!constructed)
		return;
	if (--constructed->object.refcount)
		return;
	for (uint32_t i = 0; i < constructed->objects_count; ++i)
		asn1_object_free(constructed->objects[i]);
}

struct asn1_constructed *asn1_constructed_dup(const struct asn1_constructed *constructed)
{
	struct asn1_constructed *dup = asn1_constructed_new(constructed->object.class,
	                                                    constructed->object.type);
	if (!dup)
		return NULL;
	if (!constructed->objects_count)
		return dup;
	dup->objects = malloc(sizeof(*dup->objects) * constructed->objects_count);
	if (!dup->objects)
	{
		asn1_constructed_free(dup);
		return NULL;
	}
	for (size_t i = 0; i < constructed->objects_count; ++i)
	{
		struct asn1_object *obj = asn1_object_dup(constructed->objects[i]);
		if (!obj)
		{
			asn1_constructed_free(dup);
			return NULL;
		}
		dup->objects[dup->objects_count++] = obj;
	}
	return dup;
}

int asn1_constructed_add1(struct asn1_constructed *constructed,
                          struct asn1_object *object)
{
	struct asn1_object **objects = realloc(constructed->objects,
	                                      sizeof(*objects) * (constructed->objects_count + 1));
	if (!objects)
		return 0;
	objects[constructed->objects_count] = object;
	constructed->objects = objects;
	constructed->objects_count++;
	object->refcount++;
	return 1;
}

struct asn1_primitive *asn1_primitive_new(enum asn1_class class,
                                          uint32_t type)
{
	return asn1_object_new(class, 1, type,
	                       sizeof(struct asn1_primitive));
}

void asn1_primitive_free(struct asn1_primitive *primitive)
{
	if (!primitive)
		return;
	if (--primitive->object.refcount)
		return;
	free(primitive->data);
}

struct asn1_primitive *asn1_primitive_dup(const struct asn1_primitive *primitive)
{
	struct asn1_primitive *dup = asn1_primitive_new(primitive->object.class,
	                                                primitive->object.type);
	if (!dup)
		return NULL;
	dup->size = primitive->size;
	if (!dup->size)
		return dup;
	dup->data = malloc(dup->size);
	if (!dup->data)
	{
		asn1_primitive_free(dup);
		return NULL;
	}
	memcpy(dup->data, primitive->data, dup->size);
	return dup;
}

struct asn1_integer *asn1_integer_new(void)
{
	return asn1_object_new(ASN1_UNIVERSAL, 0, ASN1_INTEGER,
	                       sizeof(struct asn1_integer));
}

void asn1_integer_free(struct asn1_integer *integer)
{
	if (!integer)
		return;
	if (--integer->object.refcount)
		return;
	free(integer->data);
	free(integer);
}

struct asn1_integer *asn1_integer_dup(const struct asn1_integer *integer)
{
	struct asn1_integer *dup = asn1_integer_new();
	if (!dup)
		return NULL;
	dup->size = integer->size;
	if (!dup->size)
		return dup;
	dup->data = malloc(dup->size);
	if (!dup->data)
	{
		asn1_integer_free(dup);
		return NULL;
	}
	memcpy(dup->data, integer->data, dup->size);
	return dup;
}

int asn1_integer_set_uint64(struct asn1_integer *integer, uint64_t v)
{
	uint8_t *data = malloc(8);
	if (!data)
		return 0;
	size_t size = 0;
	if (v > 0xFFFFFFFFFFFFFF)
		data[size++] = v >> 56;
	if (v > 0xFFFFFFFFFFFF)
		data[size++] = v >> 48;
	if (v > 0xFFFFFFFFFF)
		data[size++] = v >> 40;
	if (v > 0xFFFFFFFF)
		data[size++] = v >> 32;
	if (v > 0xFFFFFF)
		data[size++] = v >> 24;
	if (v > 0xFFFF)
		data[size++] = v >> 16;
	if (v > 0xFF)
		data[size++] = v >> 8;
	data[size++] = v;
	free(integer->data);
	integer->data = data;
	integer->size = size;
	return 1;
}

int asn1_integer_set_bignum(struct asn1_integer *integer,
                            struct bignum *bignum)
{
	size_t size = bignum_num_bytes(bignum);
	if (!size)
	{
		free(integer->data);
		integer->data = NULL;
		integer->size = 0;
		return 1;
	}
	uint8_t *data = malloc(size);
	if (!data
	 || !bignum_bignum2bin(bignum, data))
		return 0;
	free(integer->data);
	integer->data = data;
	integer->size = size;
	return 1;
}

int asn1_integer_set0_data(struct asn1_integer *integer, uint8_t *data,
                           size_t size)
{
	free(integer->data);
	integer->data = data;
	integer->size = size;
	return 1;
}

int asn1_integer_set1_data(struct asn1_integer *integer,
                           const uint8_t *data, size_t size)
{
	if (!size)
	{
		free(integer->data);
		integer->data = NULL;
		integer->size = 0;
		return 1;
	}
	uint8_t *tmp = malloc(size);
	if (!tmp)
		return 0;
	memcpy(tmp, data, size);
	free(integer->data);
	integer->data = tmp;
	integer->size = size;
	return 1;
}

int asn1_integer_get_uint64(const struct asn1_integer *integer, uint64_t *v)
{
	if (integer->size > 8)
		return 0;
	*v = 0;
	for (size_t i = 0; i < integer->size; ++i)
		*v = (*v << 8) | integer->data[i];
	return 1;
}

int asn1_integer_get_bignum(const struct asn1_integer *integer,
                                   struct bignum **bignum)
{
	struct bignum *ret = bignum_bin2bignum(integer->data, integer->size,
	                                       *bignum);
	if (!ret)
		return 0;
	*bignum = ret;
	return 1;
}

struct asn1_oid *asn1_oid_new(void)
{
	return asn1_object_new(ASN1_UNIVERSAL, 0, ASN1_OBJECT_IDENTIFIER,
	                       sizeof(struct asn1_oid));
}

void asn1_oid_free(struct asn1_oid *oid)
{
	if (!oid)
		return;
	if (--oid->object.refcount)
		return;
	free(oid->values);
	free(oid);
}

struct asn1_oid *asn1_oid_dup(const struct asn1_oid *oid)
{
	struct asn1_oid *dup = asn1_oid_new();
	if (!dup)
		return NULL;
	dup->values_count = oid->values_count;
	if (!dup->values_count)
		return dup;
	dup->values = malloc(sizeof(*dup->values) * dup->values_count);
	if (!dup->values)
	{
		asn1_oid_free(dup);
		return NULL;
	}
	memcpy(dup->values, oid->values,
	       sizeof(*dup->values) * dup->values_count);
	return dup;
}

int asn1_oid_cmp(const struct asn1_oid *oid1,
                        const struct asn1_oid *oid2)
{
	return asn1_oid_cmp_uint32(oid1, oid2->values, oid2->values_count);
}

int asn1_oid_cmp_uint32(const struct asn1_oid *oid,
                        const uint32_t *data,
                        size_t size)
{
	if (oid->values_count != size)
		return 1;
	for (size_t i = 0; i < oid->values_count; ++i)
	{
		if (oid->values[i] != data[i])
			return 1;
	}
	return 0;
}

int asn1_oid_set0(struct asn1_oid *oid, uint32_t *values, size_t count)
{
	if (count < 2)
		return 0;
	free(oid->values);
	oid->values = values;
	oid->values_count = count;
	return 1;
}

int asn1_oid_set1(struct asn1_oid *oid, const uint32_t *values,
                  size_t count)
{
	if (count < 2)
		return 0;
	if (values[0] > 6)
		return 0;
	if (values[1] > 39)
		return 0;
	uint32_t *data = malloc(sizeof(*data) * count);
	if (!data)
		return 0;
	memcpy(data, values, sizeof(*data) * count);
	free(oid->values);
	oid->values = data;
	oid->values_count = count;
	return 1;
}

int asn1_oid_get0(const struct asn1_oid *oid, uint32_t **values,
                  size_t *count)
{
	*values = oid->values;
	*count = oid->values_count;
	return 1;
}

int asn1_oid_get1(const struct asn1_oid *oid, uint32_t *values,
                  size_t *count)
{
	if (!values)
	{
		*count = oid->values_count;
		return 1;
	}
	if (*count > oid->values_count)
		*count = oid->values_count;
	memcpy(values, oid->values, sizeof(*values) * *count);
	return 1;
}

struct asn1_null *asn1_null_new(void)
{
	return asn1_object_new(ASN1_UNIVERSAL, 0, ASN1_NULL,
	                       sizeof(struct asn1_null));
}

void asn1_null_free(struct asn1_null *null)
{
	if (!null)
		return;
	if (--null->object.refcount)
		return;
	free(null);
}

struct asn1_null *asn1_null_dup(const struct asn1_null *null)
{
	(void)null;
	return asn1_null_new();
}

struct asn1_bit_string *asn1_bit_string_new(void)
{
	return asn1_object_new(ASN1_UNIVERSAL, 0, ASN1_BIT_STRING,
	                       sizeof(struct asn1_bit_string));
}

void asn1_bit_string_free(struct asn1_bit_string *bitstring)
{
	if (!bitstring)
		return;
	if (--bitstring->object.refcount)
		return;
	free(bitstring->data);
	free(bitstring);
}

struct asn1_bit_string *asn1_bit_string_dup(const struct asn1_bit_string *bitstring)
{
	struct asn1_bit_string *dup = asn1_bit_string_new();
	if (!dup)
		return NULL;
	dup->bits = bitstring->bits;
	if (!dup->bits)
		return dup;
	dup->data = malloc((dup->bits + 7) / 8);
	if (!dup->data)
	{
		asn1_bit_string_free(dup);
		return NULL;
	}
	memcpy(dup->data, bitstring->data, (dup->bits + 7) / 8);
	return dup;
}

int asn1_bit_string_set0(struct asn1_bit_string *bitstring,
                         uint8_t *data, size_t bits)
{
	free(bitstring->data);
	bitstring->data = data;
	bitstring->bits = bits;
	return 1;
}

int asn1_bit_string_set1(struct asn1_bit_string *bitstring,
                         const uint8_t *data, size_t bits)
{
	uint8_t *tmp = malloc((bits + 7) / 8);
	if (!tmp)
		return 0;
	memcpy(tmp, data, (bits + 7) / 8);
	free(bitstring->data);
	bitstring->data = tmp;
	bitstring->bits = bits;
	return 1;
}

int asn1_bit_string_get(const struct asn1_bit_string *bitstring,
                        uint8_t *data, size_t *bits)
{
	if (!data)
	{
		*bits = bitstring->bits;
		return 0;
	}
	if (*bits > bitstring->bits)
		*bits = bitstring->bits;
	memcpy(data, bitstring->data, (*bits + 7) / 8);
	return 0;
}

struct asn1_string *asn1_string_new(enum asn1_universal_type type)
{
	return asn1_object_new(ASN1_UNIVERSAL, 0, type,
	                       sizeof(struct asn1_string));
}

struct asn1_string *asn1_utf8_string_new(void)
{
	return asn1_string_new(ASN1_UTF8_STRING);
}

struct asn1_string *asn1_printable_string_new(void)
{
	return asn1_string_new(ASN1_PRINTABLE_STRING);
}

struct asn1_string *asn1_t61_string_new(void)
{
	return asn1_string_new(ASN1_T61_STRING);
}

struct asn1_string *asn1_videotex_string_new(void)
{
	return asn1_string_new(ASN1_VIDEOTEX_STRING);
}

struct asn1_string *asn1_ia5_string_new(void)
{
	return asn1_string_new(ASN1_IA5_STRING);
}

struct asn1_string *asn1_graphic_string_new(void)
{
	return asn1_string_new(ASN1_GRAPHIC_STRING);
}

struct asn1_string *asn1_visible_string_new(void)
{
	return asn1_string_new(ASN1_VISIBLE_STRING);
}

struct asn1_string *asn1_general_string_new(void)
{
	return asn1_string_new(ASN1_GENERAL_STRING);
}

struct asn1_string *asn1_univeral_string_new(void)
{
	return asn1_string_new(ASN1_UNIVERSAL_STRING);
}

struct asn1_string *asn1_BMP_string_new(void)
{
	return asn1_string_new(ASN1_BMP_STRING);
}

void asn1_string_free(struct asn1_string *string)
{
	if (!string)
		return;
	if (--string->object.refcount)
		return;
	free(string->data);
	free(string);
}

struct asn1_string *asn1_string_dup(const struct asn1_string *string)
{
	struct asn1_string *dup = asn1_string_new(string->object.type);
	if (!dup)
		return NULL;
	dup->size = string->size;
	dup->data = malloc(dup->size + 1);
	if (!dup->data)
	{
		asn1_string_free(dup);
		return NULL;
	}
	memcpy(dup->data, string->data, dup->size + 1);
	return dup;
}

int asn1_string_set0_data(struct asn1_string *string, char *data, size_t size)
{
	free(string->data);
	string->data = data;
	string->size = size;
	return 1;
}

struct asn1_time *asn1_time_new(enum asn1_universal_type type)
{
	return asn1_object_new(ASN1_UNIVERSAL, 0, type,
	                       sizeof(struct asn1_time));
}

struct asn1_time *asn1_utc_time_new(void)
{
	return asn1_time_new(ASN1_UTC_TIME);
}

struct asn1_time *asn1_generalized_time_new(void)
{
	return asn1_time_new(ASN1_GENERALIZED_TIME);
}

void asn1_time_free(struct asn1_time *time)
{
	if (!time)
		return;
	if (--time->object.refcount)
		return;
	free(time);
}

struct asn1_time *asn1_time_dup(const struct asn1_time *time)
{
	struct asn1_time *dup = asn1_time_new(time->object.type);
	if (!dup)
		return NULL;
	dup->value = time->value;
	return dup;
}

int asn1_time_print(struct bio *bio, const struct asn1_time *time)
{
	struct tm tm;
#ifdef _WIN32
	if (gmtime_s(&tm, &time->value))
		return 0;
#else
	if (!gmtime_r(&time->value, &tm))
		return 0;
#endif
	char buf[64];
	if (!strftime(buf, sizeof(buf), "%b %d %H:%M:%S %Y GMT", &tm))
		return 0;
	return bio_printf(bio, "%s", buf) != -1;
}

struct asn1_octet_string *asn1_octet_string_new(void)
{
	return asn1_object_new(ASN1_UNIVERSAL, 0, ASN1_OCTET_STRING,
	                       sizeof(struct asn1_octet_string));
}

void asn1_octet_string_free(struct asn1_octet_string *octetstring)
{
	if (!octetstring)
		return;
	if (--octetstring->object.refcount)
		return;
	free(octetstring->data);
	free(octetstring);
}

struct asn1_octet_string *asn1_octet_string_dup(const struct asn1_octet_string *octetstring)
{
	struct asn1_octet_string *dup = asn1_octet_string_new();
	if (!dup)
		return NULL;
	dup->size = octetstring->size;
	if (!dup->size)
		return dup;
	dup->data = malloc(dup->size);
	if (!dup->data)
	{
		asn1_octet_string_free(dup);
		return NULL;
	}
	memcpy(dup->data, octetstring->data, dup->size);
	return dup;
}

int asn1_octet_string_set0(struct asn1_octet_string *octetstring,
                           uint8_t *data, size_t size)
{
	free(octetstring->data);
	octetstring->data = data;
	octetstring->size = size;
	return 1;
}

int asn1_octet_string_set1(struct asn1_octet_string *octetstring,
                           const uint8_t *data, size_t size)
{
	if (!size)
	{
		free(octetstring->data);
		octetstring->data = NULL;
		octetstring->size = 0;
		return 1;
	}
	uint8_t *tmp = malloc(size);
	if (!tmp)
		return 0;
	memcpy(tmp, data, size);
	free(octetstring->data);
	octetstring->data = tmp;
	octetstring->size = size;
	return 1;
}

int asn1_octet_string_get(const struct asn1_octet_string *octetstring,
                          uint8_t *data, size_t *size)
{
	if (!data)
	{
		*size = octetstring->size;
		return 0;
	}
	if (*size > octetstring->size)
		*size = octetstring->size;
	memcpy(data, octetstring->data, *size);
	return 0;
}

struct asn1_boolean *asn1_boolean_new(void)
{
	return asn1_object_new(ASN1_UNIVERSAL, 0, ASN1_BOOLEAN,
	                       sizeof(struct asn1_boolean));
}

void asn1_boolean_free(struct asn1_boolean *boolean)
{
	if (!boolean)
		return;
	if (--boolean->object.refcount)
		return;
	free(boolean);
}

struct asn1_boolean *asn1_boolean_dup(const struct asn1_boolean *boolean)
{
	struct asn1_boolean *dup = asn1_boolean_new();
	if (!dup)
		return NULL;
	dup->value = boolean->value;
	return dup;
}
