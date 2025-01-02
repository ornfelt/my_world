#include "asn1/asn1.h"
#include "der/der.h"

#include <jkssl/bio.h>

#include <inttypes.h>
#include <time.h>

static size_t asn1_length(const struct asn1_object *object);
static int asn1_encode(struct bio *bio, const struct asn1_object *object);

static uint32_t der_len_len(uint32_t len)
{
	if (len < 0x7F)
		return 1;
	if (len < 0x100)
		return 2;
	if (len < 0x10000)
		return 3;
	if (len < 0x1000000)
		return 4;
	return 5;
}

static uint32_t der_encode_len(uint8_t *data, uint32_t len)
{
	uint64_t res;
	uint8_t tmp;

	res = 0;
	if (len < 0x7F)
	{
		data[0] = len;
		return 1;
	}
	if (len < 0x100)
		tmp = 1;
	else if (len < 0x10000)
		tmp = 2;
	else if (len < 0x1000000)
		tmp = 3;
	else
		tmp = 4;
	data[res++] = 0x80 | tmp;
	if (tmp >= 4)
		data[res++] = len >> 24;
	if (tmp >= 3)
		data[res++] = len >> 16;
	if (tmp >= 2)
		data[res++] = len >> 8;
	data[res++] = len;
	return res;
}

static size_t constructed_length(const struct asn1_object *object)
{
	const struct asn1_constructed *constructed = (struct asn1_constructed*)object;
	size_t sum = 0;
	for (size_t i = 0; i < constructed->objects_count; ++i)
	{
		size_t len = asn1_length(constructed->objects[i]);
		sum += len + der_len_len(len) + 1;
	}
	return sum;
}

static size_t primitive_length(const struct asn1_object *object)
{
	const struct asn1_primitive *primitive = (struct asn1_primitive*)object;
	return primitive->size;
}

static size_t universal_length(const struct asn1_object *object)
{
	switch (object->type)
	{
		case ASN1_BOOLEAN:
			return 1;
		case ASN1_INTEGER:
		{
			struct asn1_integer *integer = (struct asn1_integer*)object;
			if (!integer->size)
				return 1;
			if (integer->data[0] & 0x80)
				return integer->size + 1;
			return integer->size;
		}
		case ASN1_BIT_STRING:
		{
			struct asn1_bit_string *bitstring = (struct asn1_bit_string*)object;
			return 1 + (bitstring->bits + 7) / 8;
		}
		case ASN1_OCTET_STRING:
		{
			struct asn1_octet_string *octetstring = (struct asn1_octet_string*)object;
			return octetstring->size;
		}
		case ASN1_NULL:
			return 0;
		case ASN1_OBJECT_IDENTIFIER:
		{
			struct asn1_oid *oid = (struct asn1_oid*)object;
			size_t sum = 1;
			for (size_t i = 2; i < oid->values_count; ++i)
			{
				if (oid->values[i] < 0x80)
					sum += 1;
				else if (oid->values[i] < 0x4000)
					sum += 2;
				else if (oid->values[i] < 0x200000)
					sum += 3;
				else if (oid->values[i] < 0x10000000)
					sum += 4;
				else
					sum += 5;
			}
			return sum;
		}
		case ASN1_UTF8_STRING:
		case ASN1_NUMERIC_STRING:
		case ASN1_PRINTABLE_STRING:
		case ASN1_T61_STRING:
		case ASN1_VIDEOTEX_STRING:
		case ASN1_IA5_STRING:
		case ASN1_GRAPHIC_STRING:
		case ASN1_VISIBLE_STRING:
		case ASN1_GENERAL_STRING:
		case ASN1_UNIVERSAL_STRING:
		case ASN1_BMP_STRING:
		{
			struct asn1_string *string = (struct asn1_string*)object;
			return string->size;
		}
		case ASN1_UTC_TIME:
			return 13;
		case ASN1_GENERALIZED_TIME:
			return 15;
		case ASN1_SEQUENCE:
		case ASN1_SET:
			return constructed_length(object);
		default:
			fprintf(stderr, "(length) unknown asn1 type: 0x%" PRIx32 "\n",
			        object->type);
			return 0;
	}
}

static size_t asn1_length(const struct asn1_object *object)
{
	if (object->class == ASN1_UNIVERSAL)
		return universal_length(object);
	if (object->constructed)
		return constructed_length(object);
	return primitive_length(object);
}

static int encode_tag(struct bio *bio, const struct asn1_object *object)
{
	uint8_t tag = (object->class << 6)
	            | ((!!object->constructed) << 5)
	            | object->type;
	return bio_write(bio, &tag, 1) == 1;
}

static int encode_length(struct bio *bio, const struct asn1_object *object)
{
	size_t length = asn1_length(object);
	uint8_t data[5];
	size_t len = der_encode_len(data, length);
	return bio_write(bio, data, len) == (ssize_t)len;
}

static int encode_constructed(struct bio *bio, const struct asn1_object *object)
{
	const struct asn1_constructed *constructed = (struct asn1_constructed*)object;
	for (size_t i = 0; i < constructed->objects_count; ++i)
	{
		if (!asn1_encode(bio, constructed->objects[i]))
			return 0;
	}
	return 1;
}

static int encode_primitive(struct bio *bio, const struct asn1_object *object)
{
	const struct asn1_primitive *primitive = (struct asn1_primitive*)object;
	return bio_write(bio, primitive->data, primitive->size) == primitive->size;
}

static int encode_universal(struct bio *bio, const struct asn1_object *object)
{
	switch (object->type)
	{
		case ASN1_BOOLEAN:
		{
			const struct asn1_boolean *boolean = (struct asn1_boolean*)object;
			uint8_t v = boolean->value ? 0xFF : 0;
			return bio_write(bio, &v, 1) == 1;
		}
		case ASN1_INTEGER:
		{
			const struct asn1_integer *integer = (struct asn1_integer*)object;
			if (!integer->size)
				return bio_write(bio, "\x00", 1) == 1;
			/* XXX negative */
			if ((integer->data[0] & 0x80) && !bio_write(bio, "\x00", 1))
				return 0;
			return bio_write(bio, integer->data,
			                 integer->size) == (ssize_t)integer->size;
		}
		case ASN1_BIT_STRING:
		{
			const struct asn1_bit_string *bitstring = (struct asn1_bit_string*)object;
			uint8_t unused_bits = (8 - (bitstring->bits % 8)) % 8;
			if (!bio_write(bio, &unused_bits, 1))
				return 0;
			uint32_t bytes = (bitstring->bits + 7) / 8;
			if (!bytes)
				return 1;
			return bio_write(bio, bitstring->data, bytes) == bytes;
		}
		case ASN1_OCTET_STRING:
		{
			const struct asn1_octet_string *octetstring = (struct asn1_octet_string*)object;
			if (!octetstring->size)
				return 1;
			return bio_write(bio, octetstring->data,
			                 octetstring->size) == (ssize_t)octetstring->size;
		}
		case ASN1_NULL:
			return 1;
		case ASN1_OBJECT_IDENTIFIER:
		{
			const struct asn1_oid *oid = (struct asn1_oid*)object;
			uint8_t first = 0;
			if (oid->values_count > 0)
				first += oid->values[0] * 40;
			if (oid->values_count > 1)
				first += oid->values[1];
			if (!bio_write(bio, &first, 1))
				return 0;
			for (size_t i = 2; i < oid->values_count; ++i)
			{
				uint8_t tmp[5];
				size_t n = 0;
				uint32_t v = oid->values[i];
				if (v >= 0x10000000)
					tmp[n++] = 0x80 | ((v >> 28) & 0x7F);
				if (v >= 0x200000)
					tmp[n++] = 0x80 | ((v >> 21) & 0x7F);
				if (v >= 0x4000)
					tmp[n++] = 0x80 | ((v >> 14) & 0x7F);
				if (v >= 0x80)
					tmp[n++] = 0x80 | ((v >> 7) & 0x7F);
				tmp[n++] = 0x80 | ((v >> 0) & 0x7F);
				tmp[n - 1] &= ~0x80;
				if (!bio_write(bio, tmp, n))
					return 0;
			}
			return 1;
		}
		case ASN1_UTF8_STRING:
		case ASN1_NUMERIC_STRING:
		case ASN1_PRINTABLE_STRING:
		case ASN1_T61_STRING:
		case ASN1_VIDEOTEX_STRING:
		case ASN1_IA5_STRING:
		case ASN1_GRAPHIC_STRING:
		case ASN1_VISIBLE_STRING:
		case ASN1_GENERAL_STRING:
		case ASN1_UNIVERSAL_STRING:
		case ASN1_BMP_STRING:
		{
			const struct asn1_string *string = (struct asn1_string*)object;
			if (!string->size)
				return 1;
			return bio_write(bio, string->data,
			                 string->size) == (ssize_t)string->size;
		}
		case ASN1_UTC_TIME:
		{
			const struct asn1_time *utc_time = (struct asn1_time*)object;
			struct tm tm;
#ifdef _WIN32
			if (gmtime_s(&tm, &utc_time->value))
				return 0;
#else
			if (!gmtime_r(&utc_time->value, &tm))
				return 0;
#endif
			if (tm.tm_year < 50 || tm.tm_year >= 150)
				return 0;
			char data[13];
			data[0] = '0' + ((tm.tm_year / 10) % 10);
			data[1] = '0' + (tm.tm_year % 10);
			data[2] = '0' + (tm.tm_mon + 1) / 10;
			data[3] = '0' + (tm.tm_mon + 1) % 10;
			data[4] = '0' + tm.tm_mday / 10;
			data[5] = '0' + tm.tm_mday % 10;
			data[6] = '0' + tm.tm_hour / 10;
			data[7] = '0' + tm.tm_hour % 10;
			data[8] = '0' + tm.tm_min / 10;
			data[9] = '0' + tm.tm_min % 10;
			data[10] = '0' + tm.tm_sec / 10;
			data[11] = '0' + tm.tm_sec % 10;
			data[12] = 'Z';
			return bio_write(bio, data, 13) == 13;
		}
		case ASN1_GENERALIZED_TIME:
		{
			const struct asn1_time *generalized_time = (struct asn1_time*)object;
			struct tm tm;
#ifdef _WIN32
			if (gmtime_s(&tm, &generalized_time->value))
				return 0;
#else
			if (!gmtime_r(&generalized_time->value, &tm))
				return 0;
#endif
			char data[15];
			data[0] = '0' + ((tm.tm_year / 1000) % 10);
			data[1] = '0' + ((tm.tm_year / 100) % 10);
			data[2] = '0' + ((tm.tm_year / 10) % 10);
			data[3] = '0' + (tm.tm_year % 10);
			data[4] = '0' + (tm.tm_mon + 1) / 10;
			data[5] = '0' + (tm.tm_mon + 1) % 10;
			data[6] = '0' + tm.tm_mday / 10;
			data[7] = '0' + tm.tm_mday % 10;
			data[8] = '0' + tm.tm_hour / 10;
			data[9] = '0' + tm.tm_hour % 10;
			data[10] = '0' + tm.tm_min / 10;
			data[11] = '0' + tm.tm_min % 10;
			data[12] = '0' + tm.tm_sec / 10;
			data[13] = '0' + tm.tm_sec % 10;
			data[14] = 'Z';
			return bio_write(bio, data, 15) == 15;
		}
		case ASN1_SEQUENCE:
		case ASN1_SET:
			return encode_constructed(bio, object);
		default:
			fprintf(stderr, "(data) unknown asn1 type: 0x%" PRIx32 "\n",
			        object->type);
			return 0;
	}
}

static int asn1_encode(struct bio *bio, const struct asn1_object *object)
{
	if (!encode_tag(bio, object)
	 || !encode_length(bio, object))
		return 0;
	if (object->class == ASN1_UNIVERSAL)
		return encode_universal(bio, object);
	if (object->constructed)
		return encode_constructed(bio, object);
	return encode_primitive(bio, object);
}

int i2d_asn1_object(const struct asn1_object *object, uint8_t **dst)
{
	I2D_MEM(asn1_object, object);
}

int i2d_asn1_object_fp(FILE *fp, const struct asn1_object *object)
{
	I2D_FP(asn1_object, object);
}

int i2d_asn1_object_bio(struct bio *bio, const struct asn1_object *object)
{
	return asn1_encode(bio, object);
}
