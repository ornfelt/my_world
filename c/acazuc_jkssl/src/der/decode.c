#include "asn1/asn1.h"
#include "der/der.h"

#include <jkssl/bio.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

struct decode_ctx
{
	struct bio *bio;
	size_t off;
	size_t limit;
};

static int der_decode(struct decode_ctx *ctx, struct asn1_object **object);

static ssize_t ctx_read(struct decode_ctx *ctx, void *data, size_t size)
{
	size_t end;
	if (__builtin_add_overflow(ctx->off, size, &end))
		return -1;
	if (end > ctx->limit)
		size = ctx->limit - ctx->off;
	ssize_t ret = bio_read(ctx->bio, data, size);
	if (ret > 0)
		ctx->off += ret;
	return ret;
}

static int decode_length(struct decode_ctx *ctx, uint32_t *length)
{
	uint8_t fb;
	if (ctx_read(ctx, &fb, 1) != 1)
		return 0;
	if (!(fb & 0x80))
	{
		if (length)
			*length = fb;
		return 1;
	}
	uint8_t lm = fb & ~0x80;
	if (!lm || lm > 4)
		return 0;
	uint8_t v[4];
	if (ctx_read(ctx, v, lm) != lm)
		return 0;
	if (length)
	{
		*length = 0;
		for (size_t i = 0; i < lm; ++i)
			*length = (*length << 8) | v[i];
	}
	return 1;
}

static int decode_oid_value(struct decode_ctx *ctx, uint32_t *value)
{
	*value = 0;
	for (size_t i = 0; i < 6; ++i)
	{
		uint8_t byte;
		if (!ctx_read(ctx, &byte, 1))
			return 0;
		*value = (*value << 7) | (byte & 0x7F);
		if (!(byte & 0x80))
			return 1;
	}
	return 0;
}

static int decode_constructed(struct decode_ctx *ctx, uint8_t tag,
                              uint32_t length, struct asn1_object **object)
{
	struct asn1_constructed *constructed = asn1_constructed_new(tag >> 6,
	                                                            tag & 0x1F);
	if (!constructed)
		return -1;
	size_t end;
	if (__builtin_add_overflow(ctx->off, length, &end))
		return -1;
	size_t prev_limit = ctx->limit;
	ctx->limit = end;
	while (ctx->off < end)
	{
		struct asn1_object *child;
		if (der_decode(ctx, &child) != 1
		 || !asn1_constructed_add1(constructed, child))
		{
			asn1_constructed_free(constructed);
			return -1;
		}
	}
	ctx->limit = prev_limit;
	*object = (struct asn1_object*)constructed;
	return 1;
}

static int decode_universal(struct decode_ctx *ctx, uint8_t tag,
                            uint32_t length, struct asn1_object **object)
{
	switch (tag & 0x1F)
	{
		case ASN1_BOOLEAN:
		{
			if (tag & 0x20)
				return -1;
			if (length != 1)
				return -1;
			uint8_t v;
			if (!ctx_read(ctx, &v, 1))
				return -1;
			struct asn1_boolean *boolean = asn1_boolean_new();
			if (!boolean)
				return -1;
			boolean->value = (v != 0);
			*object = (struct asn1_object*)boolean;
			return 1;
		}
		case ASN1_INTEGER:
		{
			if (tag & 0x20)
				return -1;
			struct asn1_integer *integer = asn1_integer_new();
			if (!integer)
				return -1;
			if (length)
			{
				/* XXX be more restrictive about length ? */
				uint8_t *data = malloc(length);
				if (!data
				 || ctx_read(ctx, data, length) != length
				 || !asn1_integer_set0_data(integer, data, length))
				{
					free(data);
					asn1_integer_free(integer);
					return -1;
				}
			}
			*object = (struct asn1_object*)integer;
			return 1;
		}
		case ASN1_BIT_STRING:
		{
			if (tag & 0x20)
				return -1;
			if (length < 1) /* XXX really the case for empty bit string ? */
				return -1;
			uint8_t unused_bits;
			if (!ctx_read(ctx, &unused_bits, 1))
				return -1;
			if (unused_bits >= 8) /* XXX really ? */
				return -1;
			length--;
			struct asn1_bit_string *bitstring = asn1_bit_string_new();
			if (!bitstring)
				return -1;
			if (length)
			{
				/* XXX be more restrictive about length ? */
				uint8_t *data = malloc(length);
				if (!data
				 || ctx_read(ctx, data, length) != length
				 || !asn1_bit_string_set0(bitstring, data,
				                          length * 8 - unused_bits))
				{
					free(data);
					asn1_bit_string_free(bitstring);
					return -1;
				}
			}
			*object = (struct asn1_object*)bitstring;
			return 1;
		}
		case ASN1_OCTET_STRING:
		{
			if (tag & 0x20)
				return -1;
			if (length < 1) /* XXX really the case for empty octet string ? */
				return -1;
			struct asn1_octet_string *octetstring = asn1_octet_string_new();
			if (!octetstring)
				return -1;
			if (length)
			{
				/* XXX be more restrictive about length ? */
				uint8_t *data = malloc(length);
				if (!data
				 || ctx_read(ctx, data, length) != length
				 || !asn1_octet_string_set0(octetstring, data,
				                            length))
				{
					free(data);
					asn1_octet_string_free(octetstring);
					return -1;
				}
			}
			*object = (struct asn1_object*)octetstring;
			return 1;
		}
		case ASN1_NULL:
		{
			if (tag & 0x20)
				return -1;
			if (length)
				return -1;
			struct asn1_null *null = asn1_null_new();
			if (!null)
				return -1;
			*object = (struct asn1_object*)null;
			return 1;
		}
		case ASN1_OBJECT_IDENTIFIER:
		{
			if (tag & 0x20)
				return -1;
			if (length < 1)
				return -1;
			uint8_t first;
			if (!ctx_read(ctx, &first, 1))
				return -1;
			length--;
			size_t values_count = 2;
			uint32_t *values = malloc(sizeof(*values) * 2);
			if (!values)
				return -1;
			values[0] = first / 40;
			values[1] = first % 40;
			size_t end;
			if (__builtin_add_overflow(ctx->off, length, &end))
				return -1;
			size_t prev_limit = ctx->limit;
			ctx->limit = end;
			while (ctx->off < end)
			{
				uint32_t *tmp = realloc(values, sizeof(*tmp) * (values_count + 1));
				if (!tmp)
				{
					free(values);
					return -1;
				}
				values = tmp;
				if (!decode_oid_value(ctx, &values[values_count]))
				{
					free(values);
					return -1;
				}
				values_count++;
			}
			struct asn1_oid *oid = asn1_oid_new();
			if (!oid)
			{
				free(values);
				return -1;
			}
			if (!asn1_oid_set0(oid, values, values_count))
			{
				free(values);
				asn1_oid_free(oid);
				return -1;
			}
			ctx->limit = prev_limit;
			*object = (struct asn1_object*)oid;
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
			if (tag & 0x20)
				return -1;
			struct asn1_string *string = asn1_string_new(tag & 0x1F);
			if (!string)
				return -1;
			/* XXX be more restrictive about length ? */
			char *data = malloc(length + 1);
			if (!data
			 || ctx_read(ctx, data, length) != length
			 || (data[length] = '\0')
			 || !asn1_string_set0(string, data, length))
			{
				free(data);
				asn1_string_free(string);
				return -1;
			}
			*object = (struct asn1_object*)string;
			return 1;
		}
		case ASN1_SEQUENCE:
		{
			if (!(tag & 0x20))
				return -1;
			return decode_constructed(ctx, tag, length, object);
		}
		case ASN1_SET:
		{
			if (!(tag & 0x20))
				return -1;
			return decode_constructed(ctx, tag, length, object);
		}
		case ASN1_UTC_TIME:
		{
			if (tag & 0x20)
				return -1;
			if (length != 13)
				return -1;
			char data[13];
			if (ctx_read(ctx, data, 13) != 13)
				return -1;
			for (size_t i = 0; i < 12; ++i)
			{
				if (!isdigit(data[i]))
					return -1;
			}
			if (data[12] != 'Z')
				return -1;
			struct tm tm;
			memset(&tm, 0, sizeof(tm));
			tm.tm_year = (data[0] - '0') * 10
			           + (data[1] - '0') * 1;
			if (tm.tm_year < 50)
				tm.tm_year += 100;
			tm.tm_mon = (data[2] - '0') * 10
			          + (data[3] - '0') * 1 - 1;
			tm.tm_mday = (data[4] - '0') * 10
			           + (data[5] - '0') * 1;
			tm.tm_hour = (data[6] - '0') * 10
			           + (data[7] - '0') * 1;
			tm.tm_min = (data[8] - '0') * 10
			          + (data[9] - '0') * 1;
			tm.tm_sec = (data[10] - '0') * 10
			          + (data[11] - '0') * 1;
			time_t t = mktime(&tm);
			if (t == -1)
				return -1;
			struct asn1_time *utc_time = asn1_utc_time_new();
			if (!utc_time)
				return -1;
			utc_time->value = t;
			*object = (struct asn1_object*)utc_time;
			return 1;
		}
		case ASN1_GENERALIZED_TIME:
		{
			if (tag & 0x20)
				return -1;
			if (length != 15)
				return -1;
			char data[15];
			if (ctx_read(ctx, data, 15) != 15)
				return -1;
			for (size_t i = 0; i < 14; ++i)
			{
				if (!isdigit(data[i]))
					return -1;
			}
			if (data[14] != 'Z')
				return -1;
			struct tm tm;
			memset(&tm, 0, sizeof(tm));
			tm.tm_year = (data[0] - '0') * 1000
			           + (data[1] - '0') * 100
			           + (data[2] - '0') * 10
			           + (data[3] - '0') * 1;
			tm.tm_mon = (data[4] - '0') * 10
			          + (data[5] - '0') * 1 - 1;
			tm.tm_mday = (data[6] - '0') * 10
			           + (data[7] - '0') * 1;
			tm.tm_hour = (data[8] - '0') * 10
			           + (data[9] - '0') * 1;
			tm.tm_min = (data[10] - '0') * 10
			          + (data[11] - '0') * 1;
			tm.tm_sec = (data[12] - '0') * 10
			          + (data[13] - '0') * 1;
			time_t t = mktime(&tm);
			if (t == -1)
				return -1;
			struct asn1_time *generalized_time = asn1_generalized_time_new();
			if (!generalized_time)
				return -1;
			generalized_time->value = t;
			*object = (struct asn1_object*)generalized_time;
			return 1;
		}
		case 0x1F: /* XXX long form */
			fprintf(stderr, "unhandled long form\n");
			return -1;
		default:
			fprintf(stderr, "unknown tag: 0x%x\n", tag & 0x1F);
			return -1;
	}
}

static int der_decode(struct decode_ctx *ctx, struct asn1_object **object)
{
	uint8_t tag;
	if (!ctx_read(ctx, &tag, 1))
		return 0;
	uint32_t length;
	if (!decode_length(ctx, &length))
		return -1;
	if (!(tag & 0xC0))
		return decode_universal(ctx, tag, length, object);
	if (tag & 0x20)
		return decode_constructed(ctx, tag, length, object);
	struct asn1_primitive *primitive = asn1_primitive_new(tag >> 6, tag & 0x1F);
	if (!primitive)
		return -1;
	primitive->size = length;
	if (primitive->size)
	{
		primitive->data = malloc(length);
		if (!primitive->data
		 || ctx_read(ctx, primitive->data, length) != length)
		{
			asn1_primitive_free(primitive);
			return -1;
		}
	}
	*object = (struct asn1_object*)primitive;
	return 1;
}

struct asn1_object *d2i_asn1_object(struct asn1_object **object,
                                    uint8_t **data, size_t len)
{
	D2I_MEM(asn1_object, struct asn1_object, object);
}

struct asn1_object *d2i_asn1_object_fp(FILE *fp,
                                       struct asn1_object **object)
{
	D2I_FP(asn1_object, struct asn1_object, object);
}

struct asn1_object *d2i_asn1_object_bio(struct bio *bio,
                                        struct asn1_object **objectp)
{
	struct decode_ctx ctx;
	ctx.bio = bio;
	ctx.off = 0;
	ctx.limit = (size_t)-1;
	struct asn1_object *object = objectp ? *objectp : NULL;
	if (der_decode(&ctx, &object) != 1)
		return NULL;
	if (objectp)
		*objectp = object;
	return object;
}
